#include "registros_f446.h"
#include "gpio_config.h"
#include "system_config.h"

// Número del pin del LED de la placa (PA5).
#define PIN_LED 5U

int main(void)
{
    // Primero el reloj, antes de usar cualquier periférico: deja el sistema a 84 MHz.
    configurar_reloj();
    // Se enciende la base de tiempo de 1 ms (SysTick).
    iniciar_systick();

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

    while (1) {
        // Se invierte el LED y se espera 500 ms: 500 ms encendido y 500 ms apagado = 1 Hz.
        invertir_pin(GPIOA, PIN_LED);
        esperar_milisegundos(500U);
    }
}
