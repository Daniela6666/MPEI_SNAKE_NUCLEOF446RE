#ifndef PANEL_H
#define PANEL_H

#include "gpio_config.h"

// Pin R1 del panel (HUB75 pin 1), conectado a PC0. Fuente: docs/conexiones.md.
#define PIN_PANEL_R1 0U
// Pin G1 del panel (HUB75 pin 2), conectado a PC1. Fuente: docs/conexiones.md.
#define PIN_PANEL_G1 1U
// Pin B1 del panel (HUB75 pin 3), conectado a PC2. Fuente: docs/conexiones.md.
#define PIN_PANEL_B1 2U
// Pin R2 del panel (HUB75 pin 5), conectado a PC3. Fuente: docs/conexiones.md.
#define PIN_PANEL_R2 3U
// Pin G2 del panel (HUB75 pin 6), conectado a PC4. Fuente: docs/conexiones.md.
#define PIN_PANEL_G2 4U
// Pin B2 del panel (HUB75 pin 7), conectado a PC5. Fuente: docs/conexiones.md.
#define PIN_PANEL_B2 5U
// Pin OE del panel (HUB75 pin 15), conectado a PC6. Fuente: docs/conexiones.md.
#define PIN_PANEL_OE 6U
// Pin LAT del panel (HUB75 pin 14), conectado a PC7. Fuente: docs/conexiones.md.
#define PIN_PANEL_LAT 7U
// Pin CLK del panel (HUB75 pin 13), conectado a PC8. Fuente: docs/conexiones.md.
#define PIN_PANEL_CLK 8U
// Pin A del panel (HUB75 pin 9), conectado a PC10. Fuente: docs/conexiones.md.
#define PIN_PANEL_A 10U
// Pin B de selección de fase del panel (HUB75 pin 10), conectado a PC11. Fuente: docs/conexiones.md.
#define PIN_PANEL_B 11U

// Bits que se desplazan por fase y por módulo: 4 bloques de 16. Fuente: docs/mapeo_pixeles.md.
#define PANEL_BITS_POR_FASE 64U
// Ancho del tablero en píxeles (columnas x). Etapa 1: un solo panel de 32 columnas. En la etapa 2 cambia aquí.
#define PANEL_ANCHO 32U
// Alto del tablero en píxeles (filas y). Etapa 1: un solo panel de 16 filas. En la etapa 2 cambia aquí.
#define PANEL_ALTO 16U
// Número de fases del escaneo 1/4: cada fase enciende 4 filas (0, 1, 2, 3). Fuente: docs/mapeo_pixeles.md.
#define PANEL_FASES 4U
// Bits que se desplazan en cada bloque de 16 bits. Fuente: docs/mapeo_pixeles.md (bloque = bit / 16).
#define PANEL_BITS_POR_BLOQUE 16U
// Columnas que ocupa cada mitad de un bloque (8 para la mitad j < 8 y 8 para la mitad j >= 8). Fuente: docs/mapeo_pixeles.md.
#define PANEL_COLUMNAS_POR_BLOQUE 8U
// Separación en filas entre la mitad j < 8 y la mitad j >= 8 de un bloque: y = fase + 4. Fuente: docs/mapeo_pixeles.md.
#define PANEL_SEPARACION_FILAS 4U
// Filas que hay entre la mitad de arriba y la de abajo del panel: R2, G2 y B2 suman 8 a y. Fuente: docs/mapeo_pixeles.md.
#define PANEL_FILAS_MITAD_SUPERIOR 8U
// Posición del bit de los colores de abajo (R2, G2, B2) dentro de datos: R2 es el bit 3. Fuente: desplazar_bit en src/panel.c.
#define PANEL_DESPLAZAMIENTO_MITAD_INFERIOR 3U

// Color negro: los tres bits de color en 0 (el pixel está apagado).
#define COLOR_NEGRO 0U
// Color rojo: solo el bit R (bit 0) en 1.
#define COLOR_ROJO 1U
// Color verde: solo el bit G (bit 1) en 1.
#define COLOR_VERDE 2U
// Color azul: solo el bit B (bit 2) en 1.
#define COLOR_AZUL 4U
// Color amarillo: bits R (bit 0) y G (bit 1) en 1, es decir, rojo + verde.
#define COLOR_AMARILLO 3U
// Color magenta: bits R (bit 0) y B (bit 2) en 1, es decir, rojo + azul.
#define COLOR_MAGENTA 5U
// Color cian: bits G (bit 1) y B (bit 2) en 1, es decir, verde + azul.
#define COLOR_CIAN 6U
// Color blanco: los tres bits R, G y B en 1.
#define COLOR_BLANCO 7U

// Nivel de OE que enciende el panel: 0 (OE activo en bajo). Confirmado en la placa (2026-11-09).
// Si otro panel no enciende, se intercambian este valor y NIVEL_OE_APAGADO.
#define NIVEL_OE_ENCENDIDO 0U
// Nivel de OE que apaga el panel: 1, el opuesto de NIVEL_OE_ENCENDIDO. Confirmado en la placa (2026-11-09).
#define NIVEL_OE_APAGADO 1U

// Configura los pines del panel como salidas y los deja en un estado seguro.
void configurar_panel(void);
// Enciende TIM3 para que interrumpa cada 1 ms y el panel se refresque solo.
void iniciar_refresco_panel(void);
// Guarda el color (uno de los COLOR_*) del píxel (x, y) en el framebuffer.
void encender_pixel(uint32_t x, uint32_t y, uint32_t color);

#endif
