#include "system_config.h"
#include "panel.h"

int main(void)
{
    // Primero el reloj, antes de usar cualquier periférico: deja el sistema a 84 MHz.
    configurar_reloj();
    // Se enciende la base de tiempo de 1 ms (SysTick).
    iniciar_systick();

    // Se configuran los pines del panel como salidas.
    configurar_panel();
    // Según la fórmula de docs/mapeo_pixeles.md, el bit 0 de la fase 0 corresponde al píxel x = 7, y = 0.
    // Se envía fase 0, bit 0 con datos solo en G1 (verde): debe encenderse ese píxel en verde.
    enviar_fase_con_un_bit(0U, 0U, (1U << PIN_PANEL_G1));

    while (1) {
        // El panel conserva lo enviado mientras OE esté encendido; no hace falta repetir el envío.
    }
}
