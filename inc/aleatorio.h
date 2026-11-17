#ifndef ALEATORIO_H
#define ALEATORIO_H

#include <stdint.h>

// Pone la semilla del generador. Si la semilla queda en 0, se usa 1 en su lugar.
void sembrar_aleatorio(uint32_t semilla);
// Devuelve el siguiente número pseudoaleatorio de 16 bits (LFSR de Fibonacci).
uint32_t siguiente_aleatorio(void);

#endif
