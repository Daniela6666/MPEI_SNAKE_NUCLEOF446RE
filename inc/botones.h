#ifndef BOTONES_H
#define BOTONES_H

#include "gpio_config.h"

// Bit del botón A en el valor que devuelve leer_botones (1 = presionado).
#define BOTON_A (1U << 0)
// Bit del botón B en el valor que devuelve leer_botones (1 = presionado).
#define BOTON_B (1U << 1)
// Bit del botón C en el valor que devuelve leer_botones (1 = presionado).
#define BOTON_C (1U << 2)
// Bit del botón D en el valor que devuelve leer_botones (1 = presionado).
#define BOTON_D (1U << 3)
// Bit del botón E en el valor que devuelve leer_botones (1 = presionado).
#define BOTON_E (1U << 4)
// Bit del botón F en el valor que devuelve leer_botones (1 = presionado).
#define BOTON_F (1U << 5)

// Configura los seis botones del shield como entradas con pull-up interno.
void configurar_botones(void);
// Devuelve un valor con un bit por cada botón presionado (BOTON_A a BOTON_F). Un bit en 1 = botón presionado.
uint32_t leer_botones(void);

#endif
