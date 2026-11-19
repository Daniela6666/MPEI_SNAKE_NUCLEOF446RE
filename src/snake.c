#include "snake.h"
#include "graficos.h"
#include "botones.h"
#include "system_config.h"
#include "aleatorio.h"

// Cuerpo de la serpiente: arreglo circular. Los segmentos van desde cola hasta cabeza, y son longitud en total.
static Posicion cuerpo[CAPACIDAD_CUERPO];
// Índice dentro de cuerpo del segmento de la cabeza (el más nuevo).
static uint32_t cabeza = 0U;
// Índice dentro de cuerpo del segmento de la cola (el más viejo, el primero que se apaga al moverse).
static uint32_t cola = 0U;
// Cantidad de segmentos de la serpiente. Crece 1 por cada manzana comida y baja 2 por cada veneno comido.
static uint32_t longitud = 0U;
// Dirección del último paso hecho: uno de los DIRECCION_*. Empieza en derecha.
static uint32_t direccion_actual = DIRECCION_DERECHA;
// Dirección pedida por el último botón de A a D presionado: uno de los DIRECCION_*. Empieza en quieto.
static uint32_t direccion_pedida = DIRECCION_QUIETO;
// Vale 0 hasta que se presiona un botón de dirección válido. Mientras vale 0, la serpiente está quieta.
static uint32_t en_marcha = 0U;
// Posición de la manzana roja en el tablero.
static Posicion manzana;
// Puntos acumulados: sube 1 por cada manzana comida y baja 2 por cada veneno comido (sin bajar de 0).
static uint32_t puntaje = 0U;
// Vale 1 cuando la serpiente chocó o llenó el tablero. Lo pone terminar_partida, que además muestra el puntaje. Al terminar, la serpiente se queda quieta hasta reiniciar.
static uint32_t juego_terminado = 0U;
// Posición del veneno azul en el tablero. Solo es válida mientras veneno_visible vale 1.
static Posicion veneno;
// Vale 1 mientras el veneno está en el tablero; vale 0 cuando está escondido, esperando a reaparecer.
static uint32_t veneno_visible = 0U;
// Pasos del juego que faltan para que el veneno desaparezca (si está visible) o aparezca (si no está visible).
static uint32_t pasos_veneno = 0U;
// Vale 1 cuando la manzana tiene una posición válida. Sirve para no comparar con una posición vieja al buscar celda libre.
static uint32_t manzana_colocada = 0U;
// Vale 1 después de sembrar el generador aleatorio con el primer botón de dirección. Solo se siembra una vez por arranque.
static uint32_t aleatorio_sembrado = 0U;

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

// Devuelve 1 si la celda (x, y) está libre: sin cuerpo, sin manzana y sin veneno visible. Si no, devuelve 0.
static uint32_t celda_libre(uint32_t x, uint32_t y)
{
    // Si algún segmento del cuerpo está en la celda, no está libre.
    if (celda_ocupada(x, y) == 1U) {
        return 0U;
    }
    // Si la manzana tiene posición válida y está en la celda, no está libre.
    if (manzana_colocada == 1U && manzana.x == x && manzana.y == y) {
        return 0U;
    }
    // Si el veneno está visible y está en la celda, no está libre.
    if (veneno_visible == 1U && veneno.x == x && veneno.y == y) {
        return 0U;
    }
    // Ninguna de las tres cosas está en la celda: está libre.
    return 1U;
}

// Busca una celda libre al azar y guarda su posición en *destino. Devuelve 1 si la encontró y 0 si el tablero está lleno.
// Empieza en una celda que sale del LFSR y, si está ocupada, pasa a la siguiente (circular).
// Aviso: prueba como máximo CAPACIDAD_CUERPO celdas y cada prueba recorre el cuerpo. Solo tarda mucho si el tablero está casi lleno.
static uint32_t buscar_celda_libre(Posicion *destino)
{
    // Celda de partida: un número aleatorio cortado al tamaño del tablero.
    uint32_t celda_inicial = siguiente_aleatorio() % CAPACIDAD_CUERPO;
    // Se prueban todas las celdas del tablero una sola vez, empezando por la celda de partida.
    for (uint32_t i = 0U; i < CAPACIDAD_CUERPO; i++) {
        // Celda que se prueba en esta vuelta: si pasa del final del tablero, vuelve al principio.
        uint32_t celda = (celda_inicial + i) % CAPACIDAD_CUERPO;
        // Columna x de la celda: el resto de dividir la celda entre el ancho del tablero.
        uint32_t x = celda % PANEL_ANCHO;
        // Fila y de la celda: el cociente de dividir la celda entre el ancho del tablero.
        uint32_t y = celda / PANEL_ANCHO;
        // Si la celda está libre, se guarda su posición en el destino.
        if (celda_libre(x, y) == 1U) {
            destino->x = x;
            destino->y = y;
            return 1U;
        }
    }
    // Ninguna celda está libre: el tablero está lleno.
    return 0U;
}

// Termina la partida: marca juego_terminado, borra el tablero y muestra el puntaje en blanco, centrado en el panel.
static void terminar_partida(void)
{
    // Marca la partida como terminada: a partir de aquí avanzar_serpiente no mueve la serpiente.
    juego_terminado = 1U;
    // Borra todo el tablero para que solo quede el puntaje.
    borrar_pantalla();
    // Dibuja el puntaje centrado en blanco: la esquina izquierda va en (ancho del panel - ancho del número) / 2 y la superior en (alto del panel - alto de la fuente) / 2.
    dibujar_numero((PANEL_ANCHO - NUMERO_ANCHO) / 2U, (PANEL_ALTO - FUENTE_ALTO) / 2U, puntaje, COLOR_BLANCO);
}

// Pone la manzana en una celda libre y la enciende en rojo. Si no hay celda libre, la serpiente llenó el tablero y la partida termina.
static void colocar_manzana(void)
{
    // Si se encontró una celda libre, la manzana va ahí.
    if (buscar_celda_libre(&manzana) == 1U) {
        // La manzana ya tiene posición válida.
        manzana_colocada = 1U;
        // Se enciende la manzana en rojo.
        encender_pixel(manzana.x, manzana.y, COLOR_ROJO);
    } else {
        // Ninguna celda está libre: la serpiente llena el tablero, así que la partida termina y se muestra el puntaje.
        terminar_partida();
    }
}

// Pone el veneno en una celda libre y lo enciende en azul. Si no hay celda libre, el veneno no aparece esta vez.
static void colocar_veneno(void)
{
    // Si se encontró una celda libre, el veneno va ahí.
    if (buscar_celda_libre(&veneno) == 1U) {
        // El veneno está visible durante VENENO_DURACION_PASOS pasos del juego.
        veneno_visible = 1U;
        pasos_veneno = VENENO_DURACION_PASOS;
        // Se enciende el veneno en azul.
        encender_pixel(veneno.x, veneno.y, COLOR_AZUL);
    } else {
        // No hay celda libre: el veneno no aparece y se vuelve a intentar después de VENENO_ESPERA_PASOS pasos.
        veneno_visible = 0U;
        pasos_veneno = VENENO_ESPERA_PASOS;
    }
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
    // El veneno no está en el tablero al empezar. Su primera aparición ocurre tras VENENO_ESPERA_PASOS pasos.
    veneno_visible = 0U;
    pasos_veneno = VENENO_ESPERA_PASOS;
    // La manzana todavía no tiene posición válida: así colocar_manzana no compara con la posición de la partida anterior.
    manzana_colocada = 0U;
    // Se coloca la primera manzana en una celda libre.
    colocar_manzana();
}

void pedir_direccion(uint32_t botones)
{
    // Si hay algún botón de A a D presionado y el generador todavía no se sembró, se siembra ahora, una sola vez por arranque.
    // Se siembra aquí y no al vencer el temporizador: en el instante en que se presiona el botón, los milisegundos son distintos en cada arranque;
    // al vencer el temporizador siempre serían múltiplos de 200 más una constante.
    if (((botones & (BOTON_A | BOTON_B | BOTON_C | BOTON_D)) != 0U) && (aleatorio_sembrado == 0U)) {
        // La semilla son los milisegundos desde el arranque, en el instante de la primera pulsación.
        sembrar_aleatorio(obtener_milisegundos());
        // Se marca que ya se sembró para no volver a sembrar en esta partida ni en las siguientes.
        aleatorio_sembrado = 1U;
    }
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
            terminar_partida();
            return;
        }
        nueva_y = nueva_y - 1U;
    }
    // Abajo: y aumenta. Si y ya es la última fila, la partida termina.
    if (direccion_actual == DIRECCION_ABAJO) {
        if (nueva_y == PANEL_ALTO - 1U) {
            terminar_partida();
            return;
        }
        nueva_y = nueva_y + 1U;
    }
    // Izquierda: x disminuye. Si x ya es 0 (primera columna), la partida termina.
    if (direccion_actual == DIRECCION_IZQUIERDA) {
        if (nueva_x == 0U) {
            terminar_partida();
            return;
        }
        nueva_x = nueva_x - 1U;
    }
    // Derecha: x aumenta. Si x ya es la última columna, la partida termina.
    if (direccion_actual == DIRECCION_DERECHA) {
        if (nueva_x == PANEL_ANCHO - 1U) {
            terminar_partida();
            return;
        }
        nueva_x = nueva_x + 1U;
    }

    // Paso 5: si la cabeza nueva cae sobre el cuerpo, la partida termina. Esto incluye la cola,
    // porque la cola todavía no se movió en este paso.
    if (celda_ocupada(nueva_x, nueva_y) == 1U) {
        terminar_partida();
        return;
    }

    // Paso 5b: se mira si la cabeza nueva cae sobre el veneno visible. Se calcula antes de mover la cabeza.
    uint32_t come_veneno = (veneno_visible == 1U && nueva_x == veneno.x && nueva_y == veneno.y);
    // Si se come veneno con VENENO_SEGMENTOS_PERDIDOS segmentos o menos, la serpiente muere: la partida termina y la serpiente no se mueve.
    if (come_veneno == 1U && longitud <= VENENO_SEGMENTOS_PERDIDOS) {
        terminar_partida();
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
        // Si además se comió veneno, la serpiente pierde VENENO_SEGMENTOS_PERDIDOS segmentos más desde la cola.
        if (come_veneno == 1U) {
            // Cada segmento perdido: se apaga su píxel, la cola avanza y la longitud baja 1.
            for (uint32_t i = 0U; i < VENENO_SEGMENTOS_PERDIDOS; i++) {
                encender_pixel(cuerpo[cola].x, cuerpo[cola].y, COLOR_NEGRO);
                cola = (cola + 1U) % CAPACIDAD_CUERPO;
                longitud--;
            }
            // El puntaje baja VENENO_PUNTOS_PERDIDOS, pero nunca por debajo de 0.
            if (puntaje >= VENENO_PUNTOS_PERDIDOS) {
                puntaje = puntaje - VENENO_PUNTOS_PERDIDOS;
            } else {
                puntaje = 0U;
            }
            // El veneno se consume: deja de estar visible y empieza la espera para reaparecer.
            // Su píxel queda cubierto por la cabeza verde, así que no hace falta apagarlo.
            veneno_visible = 0U;
            pasos_veneno = VENENO_ESPERA_PASOS;
        }
    }

    // Paso 8: el tiempo del veneno avanza un paso. Solo si la partida sigue y en este paso no se comió veneno.
    if (juego_terminado == 0U && come_veneno == 0U) {
        // El contador no baja de 0: solo se resta si todavía vale más de 0.
        if (pasos_veneno > 0U) {
            pasos_veneno--;
        }
        // Si el contador llegó a 0, toca cambiar el estado del veneno.
        if (pasos_veneno == 0U) {
            // Si el veneno estaba visible, desaparece: se apaga su píxel y empieza la espera para reaparecer.
            if (veneno_visible == 1U) {
                encender_pixel(veneno.x, veneno.y, COLOR_NEGRO);
                veneno_visible = 0U;
                pasos_veneno = VENENO_ESPERA_PASOS;
            } else {
                // Si el veneno estaba escondido, reaparece en una celda libre.
                colocar_veneno();
            }
        }
    }
}
