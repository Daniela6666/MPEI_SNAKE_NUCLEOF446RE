#include "snake.h"
#include "graficos.h"
#include "botones.h"
#include "system_config.h"

// Cuerpo de la serpiente: arreglo circular. Los segmentos van desde cola hasta cabeza, y son longitud en total.
static Posicion cuerpo[CAPACIDAD_CUERPO];
// Índice dentro de cuerpo del segmento de la cabeza (el más nuevo).
static uint32_t cabeza = 0U;
// Índice dentro de cuerpo del segmento de la cola (el más viejo, el primero que se apaga al moverse).
static uint32_t cola = 0U;
// Cantidad de segmentos de la serpiente. Crece 1 por cada manzana comida.
static uint32_t longitud = 0U;
// Dirección del último paso hecho: uno de los DIRECCION_*. Empieza en derecha.
static uint32_t direccion_actual = DIRECCION_DERECHA;
// Dirección pedida por el último botón de A a D presionado: uno de los DIRECCION_*. Empieza en quieto.
static uint32_t direccion_pedida = DIRECCION_QUIETO;
// Vale 0 hasta que se presiona un botón de dirección válido. Mientras vale 0, la serpiente está quieta.
static uint32_t en_marcha = 0U;
// Posición de la manzana roja en el tablero.
static Posicion manzana;
// Puntos acumulados: sube 1 por cada manzana comida. Todavía no se muestra en pantalla.
static uint32_t puntaje = 0U;
// Vale 1 cuando la serpiente chocó o llenó el tablero. Al terminar, la serpiente se queda quieta hasta reiniciar.
static uint32_t juego_terminado = 0U;

// Devuelve la dirección contraria a la recibida: arriba con abajo, y derecha con izquierda.
static uint32_t direccion_opuesta(uint32_t direccion)
{
    // Si la dirección es arriba, la contraria es abajo.
    if (direccion == DIRECCION_ARRIBA) {
        return DIRECCION_ABAJO;
    }
    // Si la dirección es abajo, la contraria es arriba.
    if (direccion == DIRECCION_ABAJO) {
        return DIRECCION_ARRIBA;
    }
    // Si la dirección es derecha, la contraria es izquierda.
    if (direccion == DIRECCION_DERECHA) {
        return DIRECCION_IZQUIERDA;
    }
    // Si la dirección es izquierda, la contraria es derecha.
    if (direccion == DIRECCION_IZQUIERDA) {
        return DIRECCION_DERECHA;
    }
    // Sin dirección (quieto) no hay contraria: se devuelve quieto.
    return DIRECCION_QUIETO;
}

// Devuelve 1 si algún segmento del cuerpo está en la posición (x, y), y 0 si ninguno está.
static uint32_t celda_ocupada(uint32_t x, uint32_t y)
{
    // Se recorren los segmentos desde la cola: son longitud segmentos en total.
    for (uint32_t i = 0U; i < longitud; i++) {
        // Índice real del segmento i dentro del arreglo circular: la vuelta al principio la hace el módulo.
        uint32_t indice = (cola + i) % CAPACIDAD_CUERPO;
        // Si el segmento está en la posición buscada, la celda está ocupada.
        if (cuerpo[indice].x == x && cuerpo[indice].y == y) {
            return 1U;
        }
    }
    // Ningún segmento está en esa posición: la celda está libre.
    return 0U;
}

// Pone la manzana en una celda libre y la enciende en rojo. Empieza en la celda que indican los milisegundos y,
// si está ocupada, pasa a la siguiente (circular).
// Aviso: prueba como máximo CAPACIDAD_CUERPO celdas y cada prueba recorre el cuerpo. Solo tarda mucho si el tablero está casi lleno.
static void colocar_manzana(void)
{
    // Celda de partida: los milisegundos desde el arranque, cortados al tamaño del tablero.
    uint32_t celda_inicial = obtener_milisegundos() % CAPACIDAD_CUERPO;
    // Se prueban todas las celdas del tablero una sola vez, empezando por la celda de partida.
    for (uint32_t i = 0U; i < CAPACIDAD_CUERPO; i++) {
        // Celda que se prueba en esta vuelta: si pasa del final del tablero, vuelve al principio.
        uint32_t celda = (celda_inicial + i) % CAPACIDAD_CUERPO;
        // Columna x de la celda: el resto de dividir la celda entre el ancho del tablero.
        uint32_t x = celda % PANEL_ANCHO;
        // Fila y de la celda: el cociente de dividir la celda entre el ancho del tablero.
        uint32_t y = celda / PANEL_ANCHO;
        // Si la celda no tiene ningún segmento del cuerpo, la manzana va ahí.
        if (celda_ocupada(x, y) == 0U) {
            // Se guarda la posición de la manzana.
            manzana.x = x;
            manzana.y = y;
            // Se enciende la manzana en rojo.
            encender_pixel(x, y, COLOR_ROJO);
            // La manzana ya está colocada: no hace falta seguir buscando.
            return;
        }
    }
    // Ninguna celda está libre: la serpiente llena el tablero, así que la partida termina.
    juego_terminado = 1U;
}

void iniciar_juego(void)
{
    // Se borra la pantalla para quitar lo que quedó de la partida anterior.
    borrar_pantalla();
    // La serpiente empieza con LONGITUD_INICIAL segmentos.
    longitud = LONGITUD_INICIAL;
    // La cola es el primer segmento del arreglo: índice 0.
    cola = 0U;
    // La cabeza es el último de los segmentos iniciales: índice LONGITUD_INICIAL - 1.
    cabeza = LONGITUD_INICIAL - 1U;
    // Columna del centro del panel. Los segmentos se ponen a su izquierda y en el centro.
    uint32_t x = PANEL_ANCHO / 2U;
    // Fila del centro del panel.
    uint32_t y = PANEL_ALTO / 2U;
    // Segmento 0 (cola): dos columnas a la izquierda del centro.
    cuerpo[0].x = x - 2U;
    cuerpo[0].y = y;
    // Segmento 1: una columna a la izquierda del centro.
    cuerpo[1].x = x - 1U;
    cuerpo[1].y = y;
    // Segmento 2 (cabeza): el centro.
    cuerpo[2].x = x;
    cuerpo[2].y = y;
    // Se encienden los tres segmentos en verde.
    encender_pixel(x - 2U, y, COLOR_VERDE);
    encender_pixel(x - 1U, y, COLOR_VERDE);
    encender_pixel(x, y, COLOR_VERDE);
    // La serpiente mira a la derecha, pero no se mueve hasta que se presione un botón válido.
    direccion_actual = DIRECCION_DERECHA;
    // No hay dirección pedida todavía.
    direccion_pedida = DIRECCION_QUIETO;
    // La serpiente está quieta hasta el primer botón válido.
    en_marcha = 0U;
    // El puntaje vuelve a 0.
    puntaje = 0U;
    // La partida está en curso.
    juego_terminado = 0U;
    // Se coloca la primera manzana en una celda libre.
    colocar_manzana();
}

void pedir_direccion(uint32_t botones)
{
    // Si el botón A está presionado, la dirección pedida es arriba.
    if ((botones & BOTON_A) != 0U) {
        direccion_pedida = DIRECCION_ARRIBA;
    }
    // Si el botón B está presionado, la dirección pedida es derecha.
    if ((botones & BOTON_B) != 0U) {
        direccion_pedida = DIRECCION_DERECHA;
    }
    // Si el botón C está presionado, la dirección pedida es abajo.
    if ((botones & BOTON_C) != 0U) {
        direccion_pedida = DIRECCION_ABAJO;
    }
    // Si el botón D está presionado, la dirección pedida es izquierda.
    if ((botones & BOTON_D) != 0U) {
        direccion_pedida = DIRECCION_IZQUIERDA;
    }
}

void avanzar_serpiente(void)
{
    // Paso 1: si la partida terminó, no se hace nada: la serpiente queda congelada.
    if (juego_terminado == 1U) {
        return;
    }
    // Paso 2: si se pidió una dirección que no es la opuesta a la actual, se cambia la dirección.
    // La opuesta se ignora para que la serpiente no se vuelva sobre sí misma.
    if (direccion_pedida != DIRECCION_QUIETO && direccion_pedida != direccion_opuesta(direccion_actual)) {
        // La dirección del último paso pasa a ser la pedida.
        direccion_actual = direccion_pedida;
        // Con una dirección válida, la serpiente empieza a moverse.
        en_marcha = 1U;
    }
    // Paso 3: antes del primer botón válido, la serpiente sigue quieta.
    if (en_marcha == 0U) {
        return;
    }

    // Paso 4: se calcula la cabeza nueva, un paso desde la cabeza actual en la dirección actual.
    // La cabeza actual es el segmento del índice cabeza.
    uint32_t nueva_x = cuerpo[cabeza].x;
    uint32_t nueva_y = cuerpo[cabeza].y;
    // Arriba: y disminuye. Si y ya es 0 (primera fila), la cabeza saldría del tablero y la partida termina.
    if (direccion_actual == DIRECCION_ARRIBA) {
        if (nueva_y == 0U) {
            juego_terminado = 1U;
            return;
        }
        nueva_y = nueva_y - 1U;
    }
    // Abajo: y aumenta. Si y ya es la última fila, la partida termina.
    if (direccion_actual == DIRECCION_ABAJO) {
        if (nueva_y == PANEL_ALTO - 1U) {
            juego_terminado = 1U;
            return;
        }
        nueva_y = nueva_y + 1U;
    }
    // Izquierda: x disminuye. Si x ya es 0 (primera columna), la partida termina.
    if (direccion_actual == DIRECCION_IZQUIERDA) {
        if (nueva_x == 0U) {
            juego_terminado = 1U;
            return;
        }
        nueva_x = nueva_x - 1U;
    }
    // Derecha: x aumenta. Si x ya es la última columna, la partida termina.
    if (direccion_actual == DIRECCION_DERECHA) {
        if (nueva_x == PANEL_ANCHO - 1U) {
            juego_terminado = 1U;
            return;
        }
        nueva_x = nueva_x + 1U;
    }

    // Paso 5: si la cabeza nueva cae sobre el cuerpo, la partida termina. Esto incluye la cola,
    // porque la cola todavía no se movió en este paso.
    if (celda_ocupada(nueva_x, nueva_y) == 1U) {
        juego_terminado = 1U;
        return;
    }

    // Paso 6: la cabeza avanza una posición en el arreglo circular y guarda la posición nueva.
    cabeza = (cabeza + 1U) % CAPACIDAD_CUERPO;
    cuerpo[cabeza].x = nueva_x;
    cuerpo[cabeza].y = nueva_y;
    // Se enciende el píxel de la nueva cabeza en verde.
    encender_pixel(nueva_x, nueva_y, COLOR_VERDE);

    // Paso 7: si la cabeza llegó a la manzana, la serpiente crece: la cola no se mueve, suma 1 segmento
    // y 1 punto, y aparece otra manzana.
    if (nueva_x == manzana.x && nueva_y == manzana.y) {
        // Un segmento más.
        longitud++;
        // Un punto más.
        puntaje++;
        // Nueva manzana en una celda libre.
        colocar_manzana();
    } else {
        // Si no hubo manzana, la cola se mueve: se apaga el píxel de la cola en negro para que no quede un segmento viejo.
        encender_pixel(cuerpo[cola].x, cuerpo[cola].y, COLOR_NEGRO);
        // El índice de la cola avanza una posición en el arreglo circular.
        cola = (cola + 1U) % CAPACIDAD_CUERPO;
    }
}
