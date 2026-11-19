#ifndef GRAFICOS_H
#define GRAFICOS_H

#include "panel.h"

// Ancho de un dígito de la fuente en columnas (3 bits por fila).
#define FUENTE_ANCHO 3U
// Alto de un dígito de la fuente en filas (5 filas por dígito).
#define FUENTE_ALTO 5U
// Cantidad de dígitos de la fuente (los dígitos 0 a 9).
#define FUENTE_CANTIDAD_DIGITOS 10U
// Columnas vacías entre dos dígitos al dibujar un número.
#define FUENTE_SEPARACION 1U
// Cantidad de dígitos que dibuja dibujar_numero: siempre 4, con ceros a la izquierda.
#define NUMERO_CANTIDAD_DIGITOS 4U
// Ancho total en columnas de un número de NUMERO_CANTIDAD_DIGITOS dígitos: los dígitos más las separaciones entre ellos.
#define NUMERO_ANCHO (NUMERO_CANTIDAD_DIGITOS * FUENTE_ANCHO + (NUMERO_CANTIDAD_DIGITOS - 1U) * FUENTE_SEPARACION)

// Pone en negro todos los píxeles del tablero.
void borrar_pantalla(void);
// Dibuja solo el contorno de un rectángulo de ancho por alto píxeles. (x, y) es la esquina superior izquierda.
void dibujar_rectangulo(uint32_t x, uint32_t y, uint32_t ancho, uint32_t alto, uint32_t color);
// Dibuja el dígito (0 a 9) de 3x5 con la esquina superior izquierda en (x, y). Solo enciende los píxeles del dígito; los demás no se tocan.
void dibujar_digito(uint32_t x, uint32_t y, uint32_t digito, uint32_t color);
// Dibuja el número con la esquina superior izquierda en (x, y), con NUMERO_CANTIDAD_DIGITOS dígitos y ceros a la izquierda. Si valor es mayor que 9999, muestra solo sus últimos 4 dígitos. No borra lo que haya debajo.
void dibujar_numero(uint32_t x, uint32_t y, uint32_t valor, uint32_t color);

#endif
