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
// Puerto C de GPIO (0x40020800).
#define GPIOC_BASE (AHB1_BASE + 0x800U)
// Controlador de reloj RCC (0x40023800).
#define RCC_BASE (AHB1_BASE + 0x3800U)

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
    // Desplazamiento 0x00. Control del reloj (HSI, HSE, PLL). No se usa todavía (paso 2).
    volatile uint32_t CR;
    // Desplazamiento 0x04. Configuración del PLL. No se usa todavía (paso 2).
    volatile uint32_t PLLCFGR;
    // Desplazamiento 0x08. Configuración del reloj del sistema y de los buses. No se usa todavía (paso 2).
    volatile uint32_t CFGR;
    // Desplazamiento 0x0C. Interrupciones del reloj. No se usa todavía (paso 2).
    volatile uint32_t CIR;
    // Desplazamiento 0x10. Reinicia los puertos AHB1. No se usa todavía (paso 2).
    volatile uint32_t AHB1RSTR;
    // Desplazamiento 0x14. Reinicia los periféricos AHB2. No se usa todavía (paso 2).
    volatile uint32_t AHB2RSTR;
    // Desplazamiento 0x18. Reinicia los periféricos AHB3. No se usa todavía (paso 2).
    volatile uint32_t AHB3RSTR;
    // Desplazamiento 0x1C. Reservado por el fabricante, no se usa.
    uint32_t RESERVADO_0;
    // Desplazamiento 0x20. Reinicia los periféricos del bus APB1. No se usa todavía (paso 2).
    volatile uint32_t APB1RSTR;
    // Desplazamiento 0x24. Reinicia los periféricos del bus APB2. No se usa todavía (paso 2).
    volatile uint32_t APB2RSTR;
    // Desplazamientos 0x28 y 0x2C. Reservados por el fabricante, no se usan.
    uint32_t RESERVADO_1[2];
    // Desplazamiento 0x30. Habilita el reloj de cada puerto GPIO (bit 0 = puerto A, bit 2 = puerto C).
    volatile uint32_t AHB1ENR;
} RegistrosRCC;

// Punteros a los periféricos: se usan como GPIOA->ODR, RCC->AHB1ENR, etc.
// Cada puntero apunta a la dirección base del periférico.
#define GPIOA ((RegistrosGPIO *)GPIOA_BASE)
// Puntero al puerto C de GPIO (botón azul PC13).
#define GPIOC ((RegistrosGPIO *)GPIOC_BASE)
// Puntero al controlador de reloj RCC.
#define RCC ((RegistrosRCC *)RCC_BASE)

// Bit 0 de RCC_AHB1ENR: 1 enciende el reloj del puerto A. Fuente: RM0390, sección 6.3.10.
#define RCC_AHB1ENR_GPIOAEN (1U << 0)
// Bit 2 de RCC_AHB1ENR: 1 enciende el reloj del puerto C. Fuente: RM0390, sección 6.3.10.
#define RCC_AHB1ENR_GPIOCEN (1U << 2)

#endif
