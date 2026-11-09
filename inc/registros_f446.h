#ifndef REGISTROS_F446_H
#define REGISTROS_F446_H

#include <stdint.h>

// Direcciones base de los buses. Fuente: RM0390 (manual de referencia STM32F446),
// sección 2.2.2, tabla 1 «STM32F446xx register boundary addresses».
#define PERIFERICOS_BASE (0x40000000U)
// Bus AHB1: aquí están los puertos GPIO y el RCC.
#define AHB1_BASE (PERIFERICOS_BASE + 0x20000U)

// Direcciones base de cada periférico. Fuente: RM0390, sección 2.2.2, tabla 1.
// Puerto A de GPIO (0x40020000).
#define GPIOA_BASE (AHB1_BASE)
// Puerto B de GPIO (0x40020400). Fuente: RM0390, sección 2.2.2, tabla 1.
#define GPIOB_BASE (AHB1_BASE + 0x400U)
// Puerto C de GPIO (0x40020800).
#define GPIOC_BASE (AHB1_BASE + 0x800U)
// Controlador de reloj RCC (0x40023800).
#define RCC_BASE (AHB1_BASE + 0x3800U)
// Interfaz de la memoria FLASH (0x40023C00). Fuente: RM0390, sección 2.2.2, tabla 1, fila «Flash interface register».
#define FLASH_BASE (AHB1_BASE + 0x3C00U)
// Registros SysTick del núcleo Cortex-M4 (0xE000E010). Fuente: PM0214, sección «SysTick timer (STK)» (número de sección y tabla por verificar por Daniela); coincide con core_cm4.h de CMSIS de ARM.
#define SYSTICK_BASE (0xE000E010U)

// Mapa de los registros del puerto: cada campo está 4 bytes después del anterior,
// por eso el orden no se puede cambiar. Fuente: RM0390, capítulo 7, sección 7.4, tabla 26.
typedef struct {
    // Desplazamiento 0x00. Modo de cada pin: 2 bits por pin (00 entrada, 01 salida, 10 alterna, 11 analógico).
    volatile uint32_t MODER;
    // Desplazamiento 0x04. Tipo de salida de cada pin: 1 bit por pin (0 push-pull, 1 open-drain).
    volatile uint32_t OTYPER;
    // Desplazamiento 0x08. Velocidad de cambio de cada pin: 2 bits por pin.
    volatile uint32_t OSPEEDR;
    // Desplazamiento 0x0C. Resistencia pull-up o pull-down de cada pin: 2 bits por pin.
    volatile uint32_t PUPDR;
    // Desplazamiento 0x10. Lectura del estado de los pines de entrada: 1 bit por pin.
    volatile uint32_t IDR;
    // Desplazamiento 0x14. Valor que se saca por cada pin de salida: 1 bit por pin.
    volatile uint32_t ODR;
    // Desplazamiento 0x18. Pone (bits 0 a 15) o borra (bits 16 a 31) bits de ODR de forma atómica.
    volatile uint32_t BSRR;
    // Desplazamiento 0x1C. Bloquea la configuración de los pines hasta el próximo reinicio.
    volatile uint32_t LCKR;
    // Desplazamiento 0x20. Función alterna de los pines 0 a 7: 4 bits por pin.
    volatile uint32_t AFRL;
    // Desplazamiento 0x24. Función alterna de los pines 8 a 15: 4 bits por pin.
    volatile uint32_t AFRH;
} RegistrosGPIO;

// Mapa de los registros del RCC (reloj). Cada campo está en su desplazamiento del manual.
// Fuente: RM0390, capítulo 6, sección 6.3, «RCC registers», mapa de registros.
typedef struct {
    // Desplazamiento 0x00. Control del reloj (HSI, HSE, PLL). Se usa en el paso 2 (configurar_reloj).
    volatile uint32_t CR;
    // Desplazamiento 0x04. Configuración del PLL. Se usa en el paso 2 (configurar_reloj).
    volatile uint32_t PLLCFGR;
    // Desplazamiento 0x08. Configuración del reloj del sistema y de los buses. Se usa en el paso 2 (configurar_reloj).
    volatile uint32_t CFGR;
    // Desplazamiento 0x0C. Interrupciones del reloj. No se usa todavía.
    volatile uint32_t CIR;
    // Desplazamiento 0x10. Reinicia los puertos AHB1. No se usa todavía.
    volatile uint32_t AHB1RSTR;
    // Desplazamiento 0x14. Reinicia los periféricos AHB2. No se usa todavía.
    volatile uint32_t AHB2RSTR;
    // Desplazamiento 0x18. Reinicia los periféricos AHB3. No se usa todavía.
    volatile uint32_t AHB3RSTR;
    // Desplazamiento 0x1C. Reservado por el fabricante, no se usa.
    uint32_t RESERVADO_0;
    // Desplazamiento 0x20. Reinicia los periféricos del bus APB1. No se usa todavía.
    volatile uint32_t APB1RSTR;
    // Desplazamiento 0x24. Reinicia los periféricos del bus APB2. No se usa todavía.
    volatile uint32_t APB2RSTR;
    // Desplazamientos 0x28 y 0x2C. Reservados por el fabricante, no se usan.
    uint32_t RESERVADO_1[2];
    // Desplazamiento 0x30. Habilita el reloj de cada puerto GPIO (bit 0 = puerto A, bit 1 = puerto B, bit 2 = puerto C).
    volatile uint32_t AHB1ENR;
} RegistrosRCC;

// Solo se define ACR: los demás registros de la FLASH no se usan todavía.
typedef struct {
    // Desplazamiento 0x00. Control de acceso a la FLASH: estados de espera (LATENCY, bits 3:0). Fuente: RM0390, sección 3.8.1.
    volatile uint32_t ACR;
} RegistrosFLASH;

// Mapa de los registros de SysTick, mismo criterio que RegistrosGPIO.
typedef struct {
    // Desplazamiento 0x00. Control y estado: habilitar, interrupción y fuente de reloj (bits 0 a 2).
    volatile uint32_t CTRL;
    // Desplazamiento 0x04. Valor de recarga del contador, de 24 bits (bits 23:0).
    volatile uint32_t LOAD;
    // Desplazamiento 0x08. Valor actual del contador; escribir cualquier valor lo pone en 0.
    volatile uint32_t VAL;
    // Desplazamiento 0x0C. Calibración del contador. No se usa.
    volatile uint32_t CALIB;
} RegistrosSysTick;

// Punteros a los periféricos: se usan como GPIOA->ODR, RCC->AHB1ENR, etc.
// Cada puntero apunta a la dirección base del periférico.
#define GPIOA ((RegistrosGPIO *)GPIOA_BASE)
// Puntero al puerto B de GPIO (botones B, C, D y E del shield).
#define GPIOB ((RegistrosGPIO *)GPIOB_BASE)
// Puntero al puerto C de GPIO (botón azul PC13).
#define GPIOC ((RegistrosGPIO *)GPIOC_BASE)
// Puntero al controlador de reloj RCC.
#define RCC ((RegistrosRCC *)RCC_BASE)
// Puntero a la interfaz de la FLASH (se usa para FLASH->ACR).
#define FLASH ((RegistrosFLASH *)FLASH_BASE)
// Puntero a los registros de SysTick (se usa para SysTick->CTRL, LOAD y VAL).
#define SysTick ((RegistrosSysTick *)SYSTICK_BASE)

// Bit 0 de RCC_AHB1ENR: 1 enciende el reloj del puerto A. Fuente: RM0390, sección 6.3.10.
#define RCC_AHB1ENR_GPIOAEN (1U << 0)
// Bit 1 de RCC_AHB1ENR: 1 enciende el reloj del puerto B. Fuente: RM0390, sección 6.3.10.
#define RCC_AHB1ENR_GPIOBEN (1U << 1)
// Bit 2 de RCC_AHB1ENR: 1 enciende el reloj del puerto C. Fuente: RM0390, sección 6.3.10.
#define RCC_AHB1ENR_GPIOCEN (1U << 2)

// Bit 24 de RCC_CR: 1 enciende el PLL. Fuente: RM0390, sección 6.3.1.
#define RCC_CR_PLLON (1U << 24)
// Bit 25 de RCC_CR: 1 indica que el PLL ya enganchó y está listo. Fuente: RM0390, sección 6.3.1.
#define RCC_CR_PLLRDY (1U << 25)

// Posición del campo PLLM (divisor de entrada del PLL) en RCC_PLLCFGR, bits 5:0. Fuente: RM0390, sección 6.3.2.
#define RCC_PLLCFGR_PLLM_POSICION 0U
// Máscara de los 6 bits de PLLM, ya desplazada a su posición. Fuente: RM0390, sección 6.3.2.
#define RCC_PLLCFGR_PLLM_MASCARA (0x3FU << RCC_PLLCFGR_PLLM_POSICION)
// Posición del campo PLLN (multiplicador del VCO) en RCC_PLLCFGR, bits 14:6. Fuente: RM0390, sección 6.3.2.
#define RCC_PLLCFGR_PLLN_POSICION 6U
// Máscara de los 9 bits de PLLN, ya desplazada a su posición. Fuente: RM0390, sección 6.3.2.
#define RCC_PLLCFGR_PLLN_MASCARA (0x1FFU << RCC_PLLCFGR_PLLN_POSICION)
// Posición del campo PLLP (divisor de salida del PLL) en RCC_PLLCFGR, bits 17:16. Fuente: RM0390, sección 6.3.2.
#define RCC_PLLCFGR_PLLP_POSICION 16U
// Máscara de los 2 bits de PLLP, ya desplazada a su posición. Fuente: RM0390, sección 6.3.2.
#define RCC_PLLCFGR_PLLP_MASCARA (0x3U << RCC_PLLCFGR_PLLP_POSICION)

// Posición del campo SW (fuente del reloj del sistema) en RCC_CFGR, bits 1:0. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_SW_POSICION 0U
// Máscara de los 2 bits de SW, ya desplazada a su posición. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_SW_MASCARA (0x3U << RCC_CFGR_SW_POSICION)
// Posición del campo SWS (fuente que el hardware está usando de verdad) en RCC_CFGR, bits 3:2. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_SWS_POSICION 2U
// Máscara de los 2 bits de SWS, ya desplazada a su posición. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_SWS_MASCARA (0x3U << RCC_CFGR_SWS_POSICION)
// Posición del campo HPRE (divisor del AHB) en RCC_CFGR, bits 7:4. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_HPRE_POSICION 4U
// Máscara de los 4 bits de HPRE, ya desplazada a su posición. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_HPRE_MASCARA (0xFU << RCC_CFGR_HPRE_POSICION)
// Posición del campo PPRE1 (divisor del APB1) en RCC_CFGR, bits 12:10. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_PPRE1_POSICION 10U
// Máscara de los 3 bits de PPRE1, ya desplazada a su posición. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_PPRE1_MASCARA (0x7U << RCC_CFGR_PPRE1_POSICION)
// Posición del campo PPRE2 (divisor del APB2) en RCC_CFGR, bits 15:13. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_PPRE2_POSICION 13U
// Máscara de los 3 bits de PPRE2, ya desplazada a su posición. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_PPRE2_MASCARA (0x7U << RCC_CFGR_PPRE2_POSICION)

// Posición del campo LATENCY (estados de espera de la FLASH) en FLASH_ACR, bits 3:0. Fuente: RM0390, sección 3.8.1.
#define FLASH_ACR_LATENCY_POSICION 0U
// Máscara de los 4 bits de LATENCY, ya desplazada a su posición. Fuente: RM0390, sección 3.8.1.
#define FLASH_ACR_LATENCY_MASCARA (0xFU << FLASH_ACR_LATENCY_POSICION)

// Bit 0 de SysTick CTRL: 1 enciende el contador. Fuente: PM0214, sección SysTick (por verificar por Daniela).
#define SYSTICK_CTRL_ENABLE (1U << 0)
// Bit 1 de SysTick CTRL: 1 genera la interrupción al llegar el contador a 0. Fuente: PM0214, sección SysTick (por verificar por Daniela).
#define SYSTICK_CTRL_TICKINT (1U << 1)
// Bit 2 de SysTick CTRL: 1 usa el reloj del procesador, 0 usa AHB/8. Fuente: PM0214, sección SysTick (por verificar por Daniela).
#define SYSTICK_CTRL_CLKSOURCE (1U << 2)

#endif
