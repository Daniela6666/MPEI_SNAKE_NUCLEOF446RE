#ifndef GRAFICOS_H
#define GRAFICOS_H

#include "panel.h"

// Ancho de un dígito de la fuente en columnas (3 bits por fila).
#define FUENTE_ANCHO 3U
// Alto de un dígito de la fuente en filas (5 filas por dígito).
#define FUENTE_ALTO 5U
// Cantidad de dígitos de la fuente (los dígitos 0 a 9).
#define FUENTE_CANTIDAD_DIGITOS 10U

// Pone en negro todos los píxeles del tablero.
void borrar_pantalla(void);
// Dibuja solo el contorno de un rectángulo de ancho por alto píxeles. (x, y) es la esquina superior izquierda.
void dibujar_rectangulo(uint32_t x, uint32_t y, uint32_t ancho, uint32_t alto, uint32_t color);
// Dibuja el dígito (0 a 9) de 3x5 con la esquina superior izquierda en (x, y). Solo enciende los píxeles del dígito; los demás no se tocan.
void dibujar_digito(uint32_t x, uint32_t y, uint32_t digito, uint32_t color);

#endif
