#include "registros_f446.h"
#include "gpio_config.h"

// Número del pin del LED de la placa (PA5).
#define PIN_LED 5U
// Número del pin del botón azul de la placa (PC13). Es activo en bajo: leer 0 significa presionado.
#define PIN_BOTON_AZUL 13U

int main(void)
{
    // Configuración del LED de la placa (PA5).
    ConfiguracionPin configuracion_led;
    // El LED es una salida.
    configuracion_led.modo = MODO_SALIDA;
    // Número del pin del LED.
    configuracion_led.pin = PIN_LED;
    // Sin pull-up ni pull-down en el LED.
    configuracion_led.resistencia = RESISTENCIA_NINGUNA;
    // Velocidad baja.
    configuracion_led.velocidad = VELOCIDAD_BAJA;
    // AF0: no se usa, el pin no es de función alterna.
    configuracion_led.funcion_alterna = 0U;
    // Se escribe la configuración del LED en el puerto A.
    configurar_pin(GPIOA, configuracion_led);

    // Configuración del botón azul (PC13).
    ConfiguracionPin configuracion_boton;
    // El botón es una entrada.
    configuracion_boton.modo = MODO_ENTRADA;
    // Número del pin del botón.
    configuracion_boton.pin = PIN_BOTON_AZUL;
    // Pull-up interno: el pin queda en 1 mientras el botón no está presionado.
    configuracion_boton.resistencia = RESISTENCIA_PULL_UP;
    // Velocidad baja.
    configuracion_boton.velocidad = VELOCIDAD_BAJA;
    // AF0: no se usa, el pin no es de función alterna.
    configuracion_boton.funcion_alterna = 0U;
    // Se escribe la configuración del botón en el puerto C.
    configurar_pin(GPIOC, configuracion_boton);

    while (1) {
        // El botón es activo en bajo: leer 0 significa presionado; el LED se enciende mientras se mantiene presionado.
        if (leer_pin(GPIOC, PIN_BOTON_AZUL) == 0U) {
            // Botón presionado: se pone en 1 el pin PA5 y el LED enciende.
            escribir_pin(GPIOA, PIN_LED, 1U);
        } else {
            // Botón suelto: se pone en 0 el pin PA5 y el LED se apaga.
            escribir_pin(GPIOA, PIN_LED, 0U);
        }
    }
}
