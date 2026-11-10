#include "system_config.h"
#include "panel.h"
#include "botones.h"

// Tiempo entre un paso del punto y el siguiente, en milisegundos. El punto avanza un píxel cada 200 ms.
#define PERIODO_MOVIMIENTO_MS 200U
// Columna x donde nace el punto: el centro del panel.
#define POSICION_INICIAL_X (PANEL_ANCHO / 2U)
// Fila y donde nace el punto: el centro del panel.
#define POSICION_INICIAL_Y (PANEL_ALTO / 2U)
// Dirección sin movimiento: el punto está quieto hasta que se presiona A, B, C o D.
#define DIRECCION_QUIETO 0U
// Dirección arriba: y disminuye (el botón A).
#define DIRECCION_ARRIBA 1U
// Dirección derecha: x aumenta (el botón B).
#define DIRECCION_DERECHA 2U
// Dirección abajo: y aumenta (el botón C).
#define DIRECCION_ABAJO 3U
// Dirección izquierda: x disminuye (el botón D).
#define DIRECCION_IZQUIERDA 4U

// Columna actual del punto verde.
static uint32_t punto_x;
// Fila actual del punto verde.
static uint32_t punto_y;
// Dirección en la que avanza el punto: uno de los DIRECCION_*.
static uint32_t direccion;

// Apaga el punto en su posición actual, lo pone en el centro, lo deja quieto y lo enciende en verde.
static void reiniciar_punto(void)
{
    // Se apaga el píxel de la posición actual para que no quede un punto viejo en el panel.
    encender_pixel(punto_x, punto_y, COLOR_NEGRO);
    // El punto vuelve a la posición inicial (el centro).
    punto_x = POSICION_INICIAL_X;
    punto_y = POSICION_INICIAL_Y;
    // El punto queda quieto hasta que se presione una dirección.
    direccion = DIRECCION_QUIETO;
    // Se enciende el punto en su posición inicial, en verde.
    encender_pixel(punto_x, punto_y, COLOR_VERDE);
}

// Cambia la dirección según los botones presionados. Si hay dos presionados, gana el último en este orden,
// porque el programa no sabe cuál se presionó primero.
static void actualizar_direccion(uint32_t botones)
{
    // Si el botón A está presionado, la dirección es arriba.
    if ((botones & BOTON_A) != 0U) {
        direccion = DIRECCION_ARRIBA;
    }
    // Si el botón B está presionado, la dirección es derecha.
    if ((botones & BOTON_B) != 0U) {
        direccion = DIRECCION_DERECHA;
    }
    // Si el botón C está presionado, la dirección es abajo.
    if ((botones & BOTON_C) != 0U) {
        direccion = DIRECCION_ABAJO;
    }
    // Si el botón D está presionado, la dirección es izquierda.
    if ((botones & BOTON_D) != 0U) {
        direccion = DIRECCION_IZQUIERDA;
    }
}

// Avanza el punto un píxel en la dirección actual. En el borde se detiene y conserva la dirección.
static void mover_punto(void)
{
    // La posición nueva empieza igual a la actual; solo cambia si la dirección lo permite.
    uint32_t nueva_x = punto_x;
    uint32_t nueva_y = punto_y;
    // Arriba: se resta 1 a y solo si no está en la primera fila (0).
    if (direccion == DIRECCION_ARRIBA && punto_y > 0U) {
        nueva_y = punto_y - 1U;
    }
    // Abajo: se suma 1 a y solo si no está en la última fila.
    if (direccion == DIRECCION_ABAJO && punto_y < PANEL_ALTO - 1U) {
        nueva_y = punto_y + 1U;
    }
    // Izquierda: se resta 1 a x solo si no está en la primera columna (0).
    if (direccion == DIRECCION_IZQUIERDA && punto_x > 0U) {
        nueva_x = punto_x - 1U;
    }
    // Derecha: se suma 1 a x solo si no está en la última columna.
    if (direccion == DIRECCION_DERECHA && punto_x < PANEL_ANCHO - 1U) {
        nueva_x = punto_x + 1U;
    }
    // Si la posición cambió, se mueve el punto. Si no cambió (borde o quieto), no se hace nada.
    if (nueva_x != punto_x || nueva_y != punto_y) {
        // Primero se enciende el píxel nuevo en verde.
        encender_pixel(nueva_x, nueva_y, COLOR_VERDE);
        // Después se apaga el píxel viejo, para no dejar dos puntos en el panel.
        encender_pixel(punto_x, punto_y, COLOR_NEGRO);
        // Se guarda la posición nueva como la actual.
        punto_x = nueva_x;
        punto_y = nueva_y;
    }
}

int main(void)
{
    // Primero el reloj, antes de usar cualquier periférico: deja el sistema a 84 MHz.
    configurar_reloj();
    // Se enciende la base de tiempo de 1 ms (SysTick), que usa el temporizador de movimiento.
    iniciar_systick();

    // Se configuran los pines del panel como salidas.
    configurar_panel();
    // Se enciende TIM3: desde aquí, una interrupción cada 1 ms dibuja una fase del framebuffer en el panel.
    iniciar_refresco_panel();

    // Se configuran los seis botones como entradas con pull-up.
    configurar_botones();
    // El temporizador de movimiento cuenta desde 0 con un periodo de 200 ms.
    iniciar_temporizador_movimiento(PERIODO_MOVIMIENTO_MS);
    // El punto aparece en el centro, quieto y en verde.
    reiniciar_punto();

    while (1) {
        // Los botones se leen en cada vuelta del bucle, no solo cuando vence el temporizador, para no perder pulsaciones cortas.
        uint32_t botones = leer_botones();
        // F reinicia el punto (mientras se mantiene presionada); si no, los botones A a D cambian la dirección.
        if ((botones & BOTON_F) != 0U) {
            reiniciar_punto();
        } else {
            actualizar_direccion(botones);
        }

        // La bandera del temporizador se consume siempre, aunque haya pausa, para que no quede un vencimiento viejo pendiente.
        if (temporizador_movimiento_vencio() == 1U) {
            // E pausa el movimiento solo mientras se mantiene presionada: si no está presionada, el punto avanza.
            if ((botones & BOTON_E) == 0U) {
                mover_punto();
            }
        }
    }
}
