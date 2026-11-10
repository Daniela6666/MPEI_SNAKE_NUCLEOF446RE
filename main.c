#include "system_config.h"
#include "panel.h"
#include "botones.h"
#include "snake.h"

// Tiempo entre un paso de la serpiente y el siguiente, en milisegundos. La serpiente avanza una celda cada 200 ms.
#define PERIODO_MOVIMIENTO_MS 200U

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
    // La serpiente aparece en el centro, quieta, y se coloca la primera manzana.
    iniciar_juego();

    while (1) {
        // Los botones se leen en cada vuelta del bucle, no solo cuando vence el temporizador, para no perder pulsaciones cortas.
        uint32_t botones = leer_botones();
        // F reinicia la partida (mientras se mantiene presionada); si no, los botones A a D piden una dirección.
        if ((botones & BOTON_F) != 0U) {
            iniciar_juego();
        } else {
            pedir_direccion(botones);
        }

        // La bandera del temporizador se consume siempre, aunque haya pausa, para que no quede un vencimiento viejo pendiente.
        if (temporizador_movimiento_vencio() == 1U) {
            // E pausa el juego solo mientras se mantiene presionada: si no está presionada, la serpiente avanza un paso.
            if ((botones & BOTON_E) == 0U) {
                avanzar_serpiente();
            }
        }
    }
}
