#ifndef SNAKE_H
#define SNAKE_H

#include "panel.h"

// Posición de un segmento del cuerpo o de la manzana, en píxeles del tablero.
typedef struct {
    // Columna x del píxel, de 0 a PANEL_ANCHO - 1.
    uint8_t x;
    // Fila y del píxel, de 0 a PANEL_ALTO - 1.
    uint8_t y;
} Posicion;

// Cantidad de posiciones del arreglo circular del cuerpo: una por cada píxel del tablero, así la serpiente puede llenarlo.
#define CAPACIDAD_CUERPO (PANEL_ANCHO * PANEL_ALTO)
// Cantidad de segmentos con la que empieza la serpiente.
#define LONGITUD_INICIAL 3U
// Dirección sin movimiento: la serpiente está quieta hasta que se presiona A, B, C o D.
#define DIRECCION_QUIETO 0U
// Dirección arriba: y disminuye (el botón A).
#define DIRECCION_ARRIBA 1U
// Dirección derecha: x aumenta (el botón B).
#define DIRECCION_DERECHA 2U
// Dirección abajo: y aumenta (el botón C).
#define DIRECCION_ABAJO 3U
// Dirección izquierda: x disminuye (el botón D).
#define DIRECCION_IZQUIERDA 4U

// Borra la pantalla y pone la serpiente, la manzana, el puntaje y el estado de arranque.
void iniciar_juego(void);
// Guarda la dirección que piden los botones A a D. Si no hay ninguno de A a D presionado, no cambia la pedida.
void pedir_direccion(uint32_t botones);
// Hace un paso del juego: mueve la serpiente una celda o marca el fin de la partida.
void avanzar_serpiente(void);

#endif
