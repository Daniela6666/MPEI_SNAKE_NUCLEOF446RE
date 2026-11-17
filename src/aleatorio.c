#include "aleatorio.h"

// Cantidad de desplazamientos del LFSR que se hacen en cada llamada a siguiente_aleatorio.
#define DESPLAZAMIENTOS_POR_LLAMADA 16U
// Semilla de reemplazo: si la semilla pedida da 0, el LFSR usa 1 en su lugar.
#define SEMILLA_DE_REEMPLAZO 1U

// Estado del LFSR de 16 bits. Nunca puede valer 0: un LFSR en 0 se queda en 0 para siempre.
// Empieza en 1 para que el generador funcione aunque nunca se haya sembrado.
static uint16_t estado_lfsr = 1U;

void sembrar_aleatorio(uint32_t semilla)
{
    // Se guardan solo los 16 bits bajos de la semilla, que es el tamaño del estado.
    estado_lfsr = (uint16_t)(semilla & 0xFFFFU);
    // Si la semilla quedó en 0, el LFSR se quedaría en 0 para siempre: se usa la semilla de reemplazo.
    if (estado_lfsr == 0U) {
        estado_lfsr = SEMILLA_DE_REEMPLAZO;
    }
}

uint32_t siguiente_aleatorio(void)
{
    // Se hacen varios desplazamientos por llamada: con uno solo, cada número sería el anterior corrido un bit
    // y las posiciones de la manzana y del veneno quedarían muy parecidas entre sí.
    for (uint32_t i = 0U; i < DESPLAZAMIENTOS_POR_LLAMADA; i++) {
        // Bit nuevo: XOR de los bits 0, 2, 3 y 5 del estado (taps 16, 14, 13 y 11 del polinomio x^16 + x^14 + x^13 + x^11 + 1).
        uint16_t bit = ((estado_lfsr >> 0) ^ (estado_lfsr >> 2) ^ (estado_lfsr >> 3) ^ (estado_lfsr >> 5)) & 1U;
        // El estado se desplaza una posición a la derecha y el bit nuevo entra por la posición 15.
        estado_lfsr = (estado_lfsr >> 1) | (bit << 15);
    }
    // Se devuelve el estado ya desplazado como número aleatorio.
    return estado_lfsr;
}
