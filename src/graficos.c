#include "graficos.h"

// Fuente propia de 3x5 para los dígitos 0 a 9. Cada dígito son 5 bytes, uno por fila, de arriba abajo.
// Solo se usan los bits 0 a 2 de cada byte: el bit 2 es la columna izquierda y el bit 0 la columna derecha.
// Se escribe en binario con literales 0b... (extensión de GCC, que acepta arm-none-eabi-gcc).
// Es const, así que el arreglo va en FLASH (.rodata) y no ocupa RAM.
static const uint8_t fuente_digitos[FUENTE_CANTIDAD_DIGITOS][FUENTE_ALTO] = {
    // Dígito 0: 111 101 101 101 111.
    { 0b111, 0b101, 0b101, 0b101, 0b111 },
    // Dígito 1: 010 110 010 010 111.
    { 0b010, 0b110, 0b010, 0b010, 0b111 },
    // Dígito 2: 111 001 111 100 111.
    { 0b111, 0b001, 0b111, 0b100, 0b111 },
    // Dígito 3: 111 001 111 001 111.
    { 0b111, 0b001, 0b111, 0b001, 0b111 },
    // Dígito 4: 101 101 111 001 001.
    { 0b101, 0b101, 0b111, 0b001, 0b001 },
    // Dígito 5: 111 100 111 001 111.
    { 0b111, 0b100, 0b111, 0b001, 0b111 },
    // Dígito 6: 111 100 111 101 111.
    { 0b111, 0b100, 0b111, 0b101, 0b111 },
    // Dígito 7: 111 001 001 001 001.
    { 0b111, 0b001, 0b001, 0b001, 0b001 },
    // Dígito 8: 111 101 111 101 111.
    { 0b111, 0b101, 0b111, 0b101, 0b111 },
    // Dígito 9: 111 101 111 001 111.
    { 0b111, 0b101, 0b111, 0b001, 0b111 },
};

// Pone en negro todos los píxeles del tablero.
void borrar_pantalla(void)
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

// Dibuja el contorno de un rectángulo: la fila de arriba, la de abajo, la columna izquierda y la derecha.
// Si el ancho o el alto son 0, no se dibuja nada: con alto = 0, alto - 1U daría una vuelta de uint32_t (0xFFFFFFFF).
void dibujar_rectangulo(uint32_t x, uint32_t y, uint32_t ancho, uint32_t alto, uint32_t color)
{
    // Ancho o alto en 0: no hay rectángulo que dibujar, se sale sin tocar el tablero.
    if (ancho == 0U || alto == 0U) {
        return;
    }
    // Recorre las columnas del rectángulo (de x hasta x + ancho - 1) para pintar la fila de arriba y la de abajo.
    for (uint32_t columna = x; columna < x + ancho; columna++) {
        // Fila de arriba del rectángulo.
        encender_pixel(columna, y, color);
        // Fila de abajo del rectángulo (y + alto - 1).
        encender_pixel(columna, y + alto - 1U, color);
    }
    // Recorre las filas del rectángulo (de y hasta y + alto - 1) para pintar la columna izquierda y la derecha.
    for (uint32_t fila = y; fila < y + alto; fila++) {
        // Columna izquierda del rectángulo.
        encender_pixel(x, fila, color);
        // Columna derecha del rectángulo (x + ancho - 1).
        encender_pixel(x + ancho - 1U, fila, color);
    }
    // Los píxeles que caen fuera del tablero los ignora encender_pixel, así que un rectángulo que se sale no escribe fuera del arreglo.
}

// Dibuja un dígito de 3x5 con la esquina superior izquierda en (x, y). Solo enciende los píxeles que valen 1 en la fuente.
void dibujar_digito(uint32_t x, uint32_t y, uint32_t digito, uint32_t color)
{
    // Si el número no es un dígito de la fuente (0 a 9), no se dibuja nada.
    if (digito >= FUENTE_CANTIDAD_DIGITOS) {
        return;
    }
    // Recorre las filas del dígito, de arriba abajo.
    for (uint32_t fila = 0U; fila < FUENTE_ALTO; fila++) {
        // Recorre las columnas del dígito, de izquierda a derecha.
        for (uint32_t columna = 0U; columna < FUENTE_ANCHO; columna++) {
            // El desplazamiento toma el bit de esta columna: la columna 0 (izquierda) es el bit 2 y la columna 2 (derecha) es el bit 0.
            // Se desplaza el byte de la fila y con & 1U se mira solo ese bit. Si vale 1, el píxel se enciende.
            if (((fuente_digitos[digito][fila] >> (FUENTE_ANCHO - 1U - columna)) & 1U) == 1U) {
                // Enciende el píxel del dígito en su posición dentro del tablero.
                encender_pixel(x + columna, y + fila, color);
            }
        }
    }
}
