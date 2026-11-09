#include "gpio_config.h"

// Enciende el reloj del puerto. Sin reloj, los registros del puerto no responden.
void habilitar_reloj_puerto(RegistrosGPIO *puerto)
{
    // Si el puerto es A, se pone en 1 el bit 0 de RCC->AHB1ENR (|= no cambia los demás bits).
    if (puerto == GPIOA) {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    // Si el puerto es B, se pone en 1 el bit 1 de RCC->AHB1ENR (|= no cambia los demás bits).
    } else if (puerto == GPIOB) {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    // Si el puerto es C, se pone en 1 el bit 2 de RCC->AHB1ENR.
    } else if (puerto == GPIOC) {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    }
    // Variable que solo sirve para leer el registro; el valor leído no se usa.
    volatile uint32_t lectura_descartada;
    // Dos lecturas después de habilitar el reloj, como en el ejemplo del profesor, por una errata del chip:
    // el periférico necesita unos ciclos antes de poder usarse (no verificado en el RM0390).
    lectura_descartada = RCC->AHB1ENR;
    lectura_descartada = RCC->AHB1ENR;
    // El valor leído no se usa: (void) le dice al compilador que se descarta a propósito (evita la advertencia).
    (void)lectura_descartada;
}

// Configura un pin: modo, velocidad, resistencia y función alterna.
void configurar_pin(RegistrosGPIO *puerto, ConfiguracionPin configuracion)
{
    // Primero se enciende el reloj del puerto para poder escribir sus registros.
    habilitar_reloj_puerto(puerto);

    // Los paréntesis en ~(...) son obligatorios. Sin ellos, ~0x3U << posición se evalúa como (~0x3U) << posición
    // y la máscara borra también la configuración de todos los pines con número menor.
    // Esta es la corrección al WRITE_REG_FIELD del profesor, que usaba ~MASK sin paréntesis.
    // MODER: borra los 2 bits del pin (máscara ~(0x3U << posición)) y pone el modo nuevo en esos bits.
    puerto->MODER = (puerto->MODER & ~(0x3U << (configuracion.pin * 2U))) | (configuracion.modo << (configuracion.pin * 2U));
    // OSPEEDR: borra los 2 bits de velocidad del pin y pone la velocidad nueva.
    puerto->OSPEEDR = (puerto->OSPEEDR & ~(0x3U << (configuracion.pin * 2U))) | (configuracion.velocidad << (configuracion.pin * 2U));
    // PUPDR: borra los 2 bits de resistencia del pin y pone la resistencia nueva.
    puerto->PUPDR = (puerto->PUPDR & ~(0x3U << (configuracion.pin * 2U))) | (configuracion.resistencia << (configuracion.pin * 2U));

    // Pines 0 a 7: la función alterna está en AFRL.
    if (configuracion.pin < 8U) {
        // AFRL: borra los 4 bits de función alterna del pin y pone la función nueva.
        puerto->AFRL = (puerto->AFRL & ~(0xFU << (configuracion.pin * 4U))) | (configuracion.funcion_alterna << (configuracion.pin * 4U));
    // Pines 8 a 15: la función alterna está en AFRH; la posición se calcula restando 8.
    } else {
        // AFRH: borra los 4 bits de función alterna del pin y pone la función nueva.
        puerto->AFRH = (puerto->AFRH & ~(0xFU << ((configuracion.pin - 8U) * 4U))) | (configuracion.funcion_alterna << ((configuracion.pin - 8U) * 4U));
    }
}

// Lee el estado de un pin de entrada. Devuelve 1 si está en 1 y 0 si está en 0.
uint32_t leer_pin(RegistrosGPIO *puerto, uint32_t pin)
{
    // IDR tiene el estado real de cada pin. (1U << pin) crea una máscara con solo ese bit.
    // Si el bit del pin es 1, la expresión vale distinto de 0 y se devuelve 1U; si no, se devuelve 0U.
    return (puerto->IDR & (1U << pin)) ? 1U : 0U;
}

// Pone un pin de salida en 1 o en 0.
void escribir_pin(RegistrosGPIO *puerto, uint32_t pin, uint32_t estado)
{
    // Si estado es distinto de 0, se pone en 1 el bit del pin en ODR (|= deja los demás bits igual).
    if (estado != 0U) {
        puerto->ODR |= (1U << pin);
    // Si estado es 0, se pone en 0 el bit del pin en ODR (&= ~ deja los demás bits igual).
    } else {
        puerto->ODR &= ~(1U << pin);
    }
}

// Invierte un pin de salida: si estaba en 1 pasa a 0, y si estaba en 0 pasa a 1.
void invertir_pin(RegistrosGPIO *puerto, uint32_t pin)
{
    // ^= con la máscara del pin invierte solo ese bit de ODR y deja los demás igual.
    puerto->ODR ^= (1U << pin);
}
