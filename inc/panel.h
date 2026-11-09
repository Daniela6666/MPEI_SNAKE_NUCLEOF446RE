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

// Nivel de OE que enciende el panel: 0 (OE activo en bajo). Confirmado en la placa (2026-11-09).
// Si otro panel no enciende, se intercambian este valor y NIVEL_OE_APAGADO.
#define NIVEL_OE_ENCENDIDO 0U
// Nivel de OE que apaga el panel: 1, el opuesto de NIVEL_OE_ENCENDIDO. Confirmado en la placa (2026-11-09).
#define NIVEL_OE_APAGADO 1U

// Configura los pines del panel como salidas y los deja en un estado seguro.
void configurar_panel(void);
// Envía una fase completa: 64 bits, de los cuales solo el bit bit_encendido lleva datos; el resto va en 0.
void enviar_fase_con_un_bit(uint32_t fase, uint32_t bit_encendido, uint32_t datos);

#endif
