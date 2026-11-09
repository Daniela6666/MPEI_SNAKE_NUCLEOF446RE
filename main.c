#include "registros_f446.h"
#include "gpio_config.h"
#include "system_config.h"

// Número del pin del LED de la placa (PA5).
#define PIN_LED 5U
// Botón A del shield: pin PA10, activo en bajo (presionado = 0).
#define PIN_BOTON_A 10U
// Botón B del shield: pin PB3, activo en bajo (presionado = 0).
#define PIN_BOTON_B 3U
// Botón C del shield: pin PB5, activo en bajo (presionado = 0).
#define PIN_BOTON_C 5U
// Botón D del shield: pin PB4, activo en bajo (presionado = 0).
#define PIN_BOTON_D 4U
// Botón E del shield: pin PB10, activo en bajo (presionado = 0).
#define PIN_BOTON_E 10U
// Botón F del shield: pin PA8, activo en bajo (presionado = 0).
#define PIN_BOTON_F 8U

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

    // Configuración del botón A (PA10) como entrada.
    ConfiguracionPin configuracion_boton_A;
    // El botón es una entrada.
    configuracion_boton_A.modo = MODO_ENTRADA;
    // Número del pin del botón A.
    configuracion_boton_A.pin = PIN_BOTON_A;
    // Pull-up interno: el pin queda en 1 mientras el botón no está presionado.
    configuracion_boton_A.resistencia = RESISTENCIA_PULL_UP;
    // Velocidad baja.
    configuracion_boton_A.velocidad = VELOCIDAD_BAJA;
    // AF0: no se usa, el pin no es de función alterna.
    configuracion_boton_A.funcion_alterna = 0U;
    // Se escribe la configuración del botón A en el puerto A.
    configurar_pin(GPIOA, configuracion_boton_A);

    // Configuración del botón B (PB3) como entrada.
    ConfiguracionPin configuracion_boton_B;
    // El botón es una entrada.
    configuracion_boton_B.modo = MODO_ENTRADA;
    // Número del pin del botón B.
    configuracion_boton_B.pin = PIN_BOTON_B;
    // Pull-up interno: el pin queda en 1 mientras el botón no está presionado.
    configuracion_boton_B.resistencia = RESISTENCIA_PULL_UP;
    // Velocidad baja.
    configuracion_boton_B.velocidad = VELOCIDAD_BAJA;
    // AF0: no se usa, el pin no es de función alterna.
    configuracion_boton_B.funcion_alterna = 0U;
    // PB3 arranca en función alterna de depuración; por eso se configura explícitamente como entrada.
    configurar_pin(GPIOB, configuracion_boton_B);

    // Configuración del botón C (PB5) como entrada.
    ConfiguracionPin configuracion_boton_C;
    // El botón es una entrada.
    configuracion_boton_C.modo = MODO_ENTRADA;
    // Número del pin del botón C.
    configuracion_boton_C.pin = PIN_BOTON_C;
    // Pull-up interno: el pin queda en 1 mientras el botón no está presionado.
    configuracion_boton_C.resistencia = RESISTENCIA_PULL_UP;
    // Velocidad baja.
    configuracion_boton_C.velocidad = VELOCIDAD_BAJA;
    // AF0: no se usa, el pin no es de función alterna.
    configuracion_boton_C.funcion_alterna = 0U;
    // Se escribe la configuración del botón C en el puerto B.
    configurar_pin(GPIOB, configuracion_boton_C);

    // Configuración del botón D (PB4) como entrada.
    ConfiguracionPin configuracion_boton_D;
    // El botón es una entrada.
    configuracion_boton_D.modo = MODO_ENTRADA;
    // Número del pin del botón D.
    configuracion_boton_D.pin = PIN_BOTON_D;
    // Pull-up interno: el pin queda en 1 mientras el botón no está presionado.
    configuracion_boton_D.resistencia = RESISTENCIA_PULL_UP;
    // Velocidad baja.
    configuracion_boton_D.velocidad = VELOCIDAD_BAJA;
    // AF0: no se usa, el pin no es de función alterna.
    configuracion_boton_D.funcion_alterna = 0U;
    // PB4 arranca en función alterna de depuración; por eso se configura explícitamente como entrada.
    configurar_pin(GPIOB, configuracion_boton_D);

    // Configuración del botón E (PB10) como entrada.
    ConfiguracionPin configuracion_boton_E;
    // El botón es una entrada.
    configuracion_boton_E.modo = MODO_ENTRADA;
    // Número del pin del botón E.
    configuracion_boton_E.pin = PIN_BOTON_E;
    // Pull-up interno: el pin queda en 1 mientras el botón no está presionado.
    configuracion_boton_E.resistencia = RESISTENCIA_PULL_UP;
    // Velocidad baja.
    configuracion_boton_E.velocidad = VELOCIDAD_BAJA;
    // AF0: no se usa, el pin no es de función alterna.
    configuracion_boton_E.funcion_alterna = 0U;
    // Se escribe la configuración del botón E en el puerto B.
    configurar_pin(GPIOB, configuracion_boton_E);

    // Configuración del botón F (PA8) como entrada.
    ConfiguracionPin configuracion_boton_F;
    // El botón es una entrada.
    configuracion_boton_F.modo = MODO_ENTRADA;
    // Número del pin del botón F.
    configuracion_boton_F.pin = PIN_BOTON_F;
    // Pull-up interno: el pin queda en 1 mientras el botón no está presionado.
    configuracion_boton_F.resistencia = RESISTENCIA_PULL_UP;
    // Velocidad baja.
    configuracion_boton_F.velocidad = VELOCIDAD_BAJA;
    // AF0: no se usa, el pin no es de función alterna.
    configuracion_boton_F.funcion_alterna = 0U;
    // Se escribe la configuración del botón F en el puerto A.
    configurar_pin(GPIOA, configuracion_boton_F);

    while (1) {
        // Los botones son activos en bajo: presionado = pin en 0. El LED se enciende si CUALQUIERA de los seis está presionado, y se apaga si ninguno lo está.
        if (leer_pin(GPIOA, PIN_BOTON_A) == 0U ||
            leer_pin(GPIOB, PIN_BOTON_B) == 0U ||
            leer_pin(GPIOB, PIN_BOTON_C) == 0U ||
            leer_pin(GPIOB, PIN_BOTON_D) == 0U ||
            leer_pin(GPIOB, PIN_BOTON_E) == 0U ||
            leer_pin(GPIOA, PIN_BOTON_F) == 0U) {
            // Algún botón está presionado: el LED PA5 se enciende (es activo en alto).
            escribir_pin(GPIOA, PIN_LED, 1U);
        } else {
            // Ningún botón está presionado: el LED PA5 se apaga.
            escribir_pin(GPIOA, PIN_LED, 0U);
        }
    }
}
