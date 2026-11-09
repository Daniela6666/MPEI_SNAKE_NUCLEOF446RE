#include "system_config.h"
#include "panel.h"
#include "graficos.h"

// Columna donde empieza el primer dígito de cada fila.
#define COLUMNA_INICIAL_DIGITOS 6U
// Separación en columnas entre el inicio de un dígito y el siguiente: ancho 3 + 1 columna de separación.
#define SEPARACION_DIGITOS 4U
// Fila donde empieza la fila de dígitos de arriba.
#define FILA_DIGITOS_ARRIBA 3U
// Fila donde empieza la fila de dígitos de abajo.
#define FILA_DIGITOS_ABAJO 9U
// Cantidad de dígitos en cada fila: la fila de arriba muestra 0 a 4 y la de abajo 5 a 9.
#define DIGITOS_POR_FILA 5U
// Columna donde empieza la primera barra de colores.
#define COLUMNA_INICIAL_BARRAS 2U
// Separación en columnas entre el inicio de una barra y la siguiente: ancho 3 + 1 columna de separación.
#define SEPARACION_BARRAS 4U
// Ancho de cada barra en columnas.
#define ANCHO_BARRA 3U
// Alto de cada barra en filas.
#define ALTO_BARRA 12U
// Fila donde empiezan todas las barras.
#define FILA_BARRAS 2U
// Cantidad de barras: una por cada uno de los 7 colores de prueba.
#define CANTIDAD_BARRAS 7U
// Tiempo que se mantiene cada pantalla de prueba, en milisegundos.
#define TIEMPO_PANTALLA_MS 3000U

// Dibuja el marco blanco alrededor del tablero y los dígitos 0 a 9 en dos filas.
static void mostrar_marco_y_digitos(void)
{
    // Se borra el tablero antes de dibujar la nueva pantalla.
    borrar_pantalla();
    // Marco blanco: contorno del tablero completo, desde la esquina (0, 0) con el tamaño del panel.
    dibujar_rectangulo(0U, 0U, PANEL_ANCHO, PANEL_ALTO, COLOR_BLANCO);
    // Recorre las posiciones de los dígitos de una fila (0 a 4).
    for (uint32_t i = 0U; i < DIGITOS_POR_FILA; i++) {
        // Columna de este dígito: la columna inicial más la separación por cada dígito anterior.
        uint32_t posicion_x = COLUMNA_INICIAL_DIGITOS + SEPARACION_DIGITOS * i;
        // Fila de arriba: el dígito i (0 a 4) en verde.
        dibujar_digito(posicion_x, FILA_DIGITOS_ARRIBA, i, COLOR_VERDE);
        // Fila de abajo: el dígito i + 5 (5 a 9) en amarillo.
        dibujar_digito(posicion_x, FILA_DIGITOS_ABAJO, i + DIGITOS_POR_FILA, COLOR_AMARILLO);
    }
}

// Dibuja siete barras verticales, una de cada color de prueba: rojo, verde, azul, amarillo, magenta, cian y blanco.
static void mostrar_siete_colores(void)
{
    // Se borra el tablero antes de dibujar la nueva pantalla.
    borrar_pantalla();
    // Color de cada barra, de izquierda a derecha.
    const uint32_t colores_de_prueba[CANTIDAD_BARRAS] = { COLOR_ROJO, COLOR_VERDE, COLOR_AZUL, COLOR_AMARILLO, COLOR_MAGENTA, COLOR_CIAN, COLOR_BLANCO };
    // Recorre las siete barras.
    for (uint32_t i = 0U; i < CANTIDAD_BARRAS; i++) {
        // Dibuja el contorno de la barra i en su color, con la columna y fila de inicio de las barras.
        dibujar_rectangulo(COLUMNA_INICIAL_BARRAS + SEPARACION_BARRAS * i, FILA_BARRAS, ANCHO_BARRA, ALTO_BARRA, colores_de_prueba[i]);
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
        // Pantalla 1: se debe ver un marco blanco alrededor del panel, los dígitos 0 a 4 en verde en la fila de arriba y los dígitos 5 a 9 en amarillo en la fila de abajo, durante 3 s.
        mostrar_marco_y_digitos();
        esperar_milisegundos(TIEMPO_PANTALLA_MS);

        // Pantalla 2: se deben ver siete barras contorneadas, de izquierda a derecha en rojo, verde, azul, amarillo, magenta, cian y blanco, durante 3 s.
        mostrar_siete_colores();
        esperar_milisegundos(TIEMPO_PANTALLA_MS);
    }
}
