#ifndef GPIO_CONFIG_H
#define GPIO_CONFIG_H

#include "registros_f446.h"

// Valores para los campos de configuración de un pin.
// Fuente: RM0390, sección 7.4, descripción de los registros GPIOx_MODER, GPIOx_PUPDR y GPIOx_OSPEEDR.
// Modo entrada: MODER = 00.
#define MODO_ENTRADA 0U
// Modo salida: MODER = 01.
#define MODO_SALIDA 1U
// Sin pull-up ni pull-down: PUPDR = 00.
#define RESISTENCIA_NINGUNA 0U
// Pull-up interno: PUPDR = 01 (el pin queda en 1 si nadie lo baja).
#define RESISTENCIA_PULL_UP 1U
// Velocidad baja: OSPEEDR = 00.
#define VELOCIDAD_BAJA 0U

// Datos para configurar un pin de una sola vez.
typedef struct {
    // Modo del pin: MODO_ENTRADA o MODO_SALIDA.
    uint32_t modo;
    // Número del pin dentro del puerto (0 a 15).
    uint32_t pin;
    // Resistencia interna: RESISTENCIA_NINGUNA o RESISTENCIA_PULL_UP.
    uint32_t resistencia;
    // Velocidad del pin: VELOCIDAD_BAJA.
    uint32_t velocidad;
    // Función alterna (AF0 a AF15). Usar 0U cuando el pin no es de función alterna.
    uint32_t funcion_alterna;
} ConfiguracionPin;

// Enciende el reloj del puerto indicado (puerto A, B o C).
void habilitar_reloj_puerto(RegistrosGPIO *puerto);
// Escribe en los registros del puerto la configuración de un pin.
void configurar_pin(RegistrosGPIO *puerto, ConfiguracionPin configuracion);
// Devuelve 1 si el pin está en 1 y 0 si está en 0.
uint32_t leer_pin(RegistrosGPIO *puerto, uint32_t pin);
// Pone el pin en 1 si estado es distinto de 0, o en 0 si estado es 0.
void escribir_pin(RegistrosGPIO *puerto, uint32_t pin, uint32_t estado);
// Cambia el pin de 0 a 1 o de 1 a 0.
void invertir_pin(RegistrosGPIO *puerto, uint32_t pin);

#endif
