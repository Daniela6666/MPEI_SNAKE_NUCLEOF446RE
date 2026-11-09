#include "system_config.h"
#include "panel.h"

// Pone en negro todos los píxeles del tablero.
static void limpiar_pantalla(void)
{
    // Recorre todas las filas del tablero.
    for (uint32_t y = 0U; y < PANEL_ALTO; y++) {
        // Recorre todas las columnas de la fila.
        for (uint32_t x = 0U; x < PANEL_ANCHO; x++) {
            // Pone el píxel (x, y) en negro.
            encender_pixel(x, y, COLOR_NEGRO);
        }
    }
}

// Pinta 1 de cada 4 píxeles (x e y pares) con el color dado. Es parcial para limitar la corriente de la fuente de 3 A.
static void rellenar_parcial(uint32_t color)
{
    // Recorre todas las filas del tablero.
    for (uint32_t y = 0U; y < PANEL_ALTO; y++) {
        // Recorre todas las columnas de la fila.
        for (uint32_t x = 0U; x < PANEL_ANCHO; x++) {
            // Solo se pintan los píxeles con x par e y par: queda 1 de cada 4 encendido.
            if ((x % 2U) == 0U && (y % 2U) == 0U) {
                // Pone el píxel (x, y) en el color pedido.
                encender_pixel(x, y, color);
            }
        }
    }
}

// Dibuja un marco completo alrededor del tablero: la fila de arriba, la de abajo, la columna izquierda y la derecha.
static void dibujar_marco(uint32_t color)
{
    // Recorre las columnas para pintar la fila de arriba (y = 0) y la de abajo (y = PANEL_ALTO - 1).
    for (uint32_t x = 0U; x < PANEL_ANCHO; x++) {
        // Fila de arriba.
        encender_pixel(x, 0U, color);
        // Fila de abajo.
        encender_pixel(x, PANEL_ALTO - 1U, color);
    }
    // Recorre las filas para pintar la columna de la izquierda (x = 0) y la de la derecha (x = PANEL_ANCHO - 1).
    for (uint32_t y = 0U; y < PANEL_ALTO; y++) {
        // Columna de la izquierda.
        encender_pixel(0U, y, color);
        // Columna de la derecha.
        encender_pixel(PANEL_ANCHO - 1U, y, color);
    }
}

// Dibuja dos diagonales de esquina a esquina con el color dado.
static void dibujar_diagonales(uint32_t color)
{
    // Recorre las filas: en cada una se pintan dos píxeles, uno de cada diagonal.
    for (uint32_t y = 0U; y < PANEL_ALTO; y++) {
        // x1 crece de 0 a PANEL_ANCHO - 1 a medida que y crece de 0 a PANEL_ALTO - 1: es la diagonal de arriba a la izquierda hacia abajo a la derecha.
        uint32_t x1 = (y * (PANEL_ANCHO - 1U)) / (PANEL_ALTO - 1U);
        // x2 es el espejo de x1: es la diagonal de arriba a la derecha hacia abajo a la izquierda.
        uint32_t x2 = (PANEL_ANCHO - 1U) - x1;
        // Pone el píxel de la primera diagonal en la fila y.
        encender_pixel(x1, y, color);
        // Pone el píxel de la segunda diagonal en la fila y.
        encender_pixel(x2, y, color);
    }
}

int main(void)
{
    // Primero el reloj, antes de usar cualquier periférico: deja el sistema a 84 MHz.
    configurar_reloj();
    // Se enciende la base de tiempo de 1 ms (SysTick), que usa esperar_milisegundos.
    iniciar_systick();

    // Se configuran los pines del panel como salidas.
    configurar_panel();
    // Se enciende TIM3: desde aquí, una interrupción cada 1 ms dibuja una fase del framebuffer en el panel.
    iniciar_refresco_panel();

    while (1) {
        // Dibujo 1: se debe ver 1 de cada 4 píxeles en rojo durante 2 s.
        limpiar_pantalla();
        rellenar_parcial(COLOR_ROJO);
        esperar_milisegundos(2000U);

        // Dibujo 2: se debe ver 1 de cada 4 píxeles en verde durante 2 s.
        limpiar_pantalla();
        rellenar_parcial(COLOR_VERDE);
        esperar_milisegundos(2000U);

        // Dibujo 3: se debe ver 1 de cada 4 píxeles en azul durante 2 s.
        limpiar_pantalla();
        rellenar_parcial(COLOR_AZUL);
        esperar_milisegundos(2000U);

        // Dibujo 4: se debe ver un marco rojo completo alrededor del panel durante 2 s.
        limpiar_pantalla();
        dibujar_marco(COLOR_ROJO);
        esperar_milisegundos(2000U);

        // Dibujo 5: se deben ver dos diagonales verdes de esquina a esquina durante 2 s.
        limpiar_pantalla();
        dibujar_diagonales(COLOR_VERDE);
        esperar_milisegundos(2000U);
    }
}
