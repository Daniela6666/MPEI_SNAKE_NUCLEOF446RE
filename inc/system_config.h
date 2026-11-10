#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include "registros_f446.h"

// Frecuencia del reloj del sistema (SYSCLK y HCLK) después de configurar_reloj: 84 MHz.
#define FRECUENCIA_RELOJ_HZ 84000000U
// Interrupciones de SysTick por segundo: 1000 = una interrupción por milisegundo (base de tiempo de 1 ms).
#define INTERRUPCIONES_SYSTICK_POR_SEGUNDO 1000U

// Configura el PLL a 84 MHz, la latencia de la FLASH y los divisores de los buses. Se llama antes que cualquier periférico.
void configurar_reloj(void);
// Enciende SysTick para que interrumpa cada 1 ms (base de tiempo).
void iniciar_systick(void);
// Espera la cantidad de milisegundos indicada. Mientras espera, la interrupción de SysTick sigue contando.
void esperar_milisegundos(uint32_t milisegundos);
// Devuelve los milisegundos desde el arranque. Sirve como semilla para la posición de la manzana.
uint32_t obtener_milisegundos(void);
// Deja el temporizador de movimiento contando desde 0 y con el periodo indicado, en milisegundos.
void iniciar_temporizador_movimiento(uint32_t periodo_ms);
// Devuelve 1 y baja la bandera si el temporizador de movimiento venció desde la última consulta; devuelve 0 si no.
uint32_t temporizador_movimiento_vencio(void);

#endif
