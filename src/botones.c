#include "botones.h"

// Número del pin del botón A del shield: PA10, activo en bajo (presionado = 0).
#define PIN_BOTON_A 10U
// Número del pin del botón B del shield: PB3, activo en bajo (presionado = 0).
#define PIN_BOTON_B 3U
// Número del pin del botón C del shield: PB5, activo en bajo (presionado = 0).
#define PIN_BOTON_C 5U
// Número del pin del botón D del shield: PB4, activo en bajo (presionado = 0).
#define PIN_BOTON_D 4U
// Número del pin del botón E del shield: PB10, activo en bajo (presionado = 0).
#define PIN_BOTON_E 10U
// Número del pin del botón F del shield: PA8, activo en bajo (presionado = 0).
#define PIN_BOTON_F 8U

// Configura un pin como entrada con pull-up interno. El pin queda en 1 mientras el botón no está presionado.
static void configurar_entrada_con_pull_up(RegistrosGPIO *puerto, uint32_t pin)
{
    // Datos de configuración del pin, que se escriben de una sola vez con configurar_pin.
    ConfiguracionPin configuracion;
    // El botón es una entrada.
    configuracion.modo = MODO_ENTRADA;
    // Número del pin dentro del puerto.
    configuracion.pin = pin;
    // Pull-up interno: el pin queda en 1 mientras el botón no está presionado.
    configuracion.resistencia = RESISTENCIA_PULL_UP;
    // Velocidad baja.
    configuracion.velocidad = VELOCIDAD_BAJA;
    // AF0: no se usa, el pin no es de función alterna.
    configuracion.funcion_alterna = 0U;
    // Se escribe la configuración en el puerto indicado.
    configurar_pin(puerto, configuracion);
}

void configurar_botones(void)
{
    // Botón A (PA10) en el puerto A.
    configurar_entrada_con_pull_up(GPIOA, PIN_BOTON_A);
    // Botón B (PB3) en el puerto B. PB3 arranca en función alterna de depuración; por eso se configura explícitamente como entrada.
    configurar_entrada_con_pull_up(GPIOB, PIN_BOTON_B);
    // Botón C (PB5) en el puerto B.
    configurar_entrada_con_pull_up(GPIOB, PIN_BOTON_C);
    // Botón D (PB4) en el puerto B. PB4 arranca en función alterna de depuración; por eso se configura explícitamente como entrada.
    configurar_entrada_con_pull_up(GPIOB, PIN_BOTON_D);
    // Botón E (PB10) en el puerto B.
    configurar_entrada_con_pull_up(GPIOB, PIN_BOTON_E);
    // Botón F (PA8) en el puerto A.
    configurar_entrada_con_pull_up(GPIOA, PIN_BOTON_F);
}

uint32_t leer_botones(void)
{
    // Valor que se va armando: cada botón presionado pone su bit en 1.
    uint32_t botones = 0U;
    // Los botones son activos en bajo: leer_pin devuelve 0 cuando el botón está presionado.
    if (leer_pin(GPIOA, PIN_BOTON_A) == 0U) {
        // Se pone en 1 el bit del botón A (|= no cambia los demás bits).
        botones |= BOTON_A;
    }
    // Botón B (PB3): presionado = 0.
    if (leer_pin(GPIOB, PIN_BOTON_B) == 0U) {
        // Se pone en 1 el bit del botón B.
        botones |= BOTON_B;
    }
    // Botón C (PB5): presionado = 0.
    if (leer_pin(GPIOB, PIN_BOTON_C) == 0U) {
        // Se pone en 1 el bit del botón C.
        botones |= BOTON_C;
    }
    // Botón D (PB4): presionado = 0.
    if (leer_pin(GPIOB, PIN_BOTON_D) == 0U) {
        // Se pone en 1 el bit del botón D.
        botones |= BOTON_D;
    }
    // Botón E (PB10): presionado = 0.
    if (leer_pin(GPIOB, PIN_BOTON_E) == 0U) {
        // Se pone en 1 el bit del botón E.
        botones |= BOTON_E;
    }
    // Botón F (PA8): presionado = 0.
    if (leer_pin(GPIOA, PIN_BOTON_F) == 0U) {
        // Se pone en 1 el bit del botón F.
        botones |= BOTON_F;
    }
    // Se devuelve el valor con un bit por cada botón presionado.
    return botones;
}
