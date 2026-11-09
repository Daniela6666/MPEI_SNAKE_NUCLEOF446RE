#include "panel.h"

// Configura un pin del panel (puerto C) como salida push-pull, sin resistencia, con velocidad alta.
static void configurar_salida_del_panel(uint32_t pin)
{
    // Datos de configuración del pin del panel.
    ConfiguracionPin configuracion;
    // Modo salida: el microcontrolador maneja el pin y saca el nivel que se le escriba.
    configuracion.modo = MODO_SALIDA;
    // Número del pin dentro del puerto C.
    configuracion.pin = pin;
    // Sin pull-up ni pull-down: el pin lo domina solo la salida.
    configuracion.resistencia = RESISTENCIA_NINGUNA;
    // Velocidad alta: los flancos de CLK y LAT tienen que ser rápidos.
    configuracion.velocidad = VELOCIDAD_ALTA;
    // AF0: no se usa, el pin no es de función alterna.
    configuracion.funcion_alterna = 0U;
    // Se escribe la configuración en el puerto C.
    configurar_pin(GPIOC, configuracion);
}

// Desplaza un bit por los seis colores. datos lleva en sus bits 0 a 5 los valores de R1, G1, B1, R2, G2 y B2.
static void desplazar_bit(uint32_t datos)
{
    // R1 sale por PC0: se toma el bit 0 de datos (el desplazamiento es el número del pin).
    escribir_pin(GPIOC, PIN_PANEL_R1, (datos >> PIN_PANEL_R1) & 1U);
    // G1 sale por PC1: se toma el bit 1 de datos.
    escribir_pin(GPIOC, PIN_PANEL_G1, (datos >> PIN_PANEL_G1) & 1U);
    // B1 sale por PC2: se toma el bit 2 de datos.
    escribir_pin(GPIOC, PIN_PANEL_B1, (datos >> PIN_PANEL_B1) & 1U);
    // R2 sale por PC3: se toma el bit 3 de datos.
    escribir_pin(GPIOC, PIN_PANEL_R2, (datos >> PIN_PANEL_R2) & 1U);
    // G2 sale por PC4: se toma el bit 4 de datos.
    escribir_pin(GPIOC, PIN_PANEL_G2, (datos >> PIN_PANEL_G2) & 1U);
    // B2 sale por PC5: se toma el bit 5 de datos.
    escribir_pin(GPIOC, PIN_PANEL_B2, (datos >> PIN_PANEL_B2) & 1U);
    // CLK en 1: el panel toma el bit de los pines de color en este flanco de subida.
    escribir_pin(GPIOC, PIN_PANEL_CLK, 1U);
    // CLK en 0: deja CLK listo para el siguiente bit.
    escribir_pin(GPIOC, PIN_PANEL_CLK, 0U);
}

// Configura los pines del panel. Primero se dejan en un nivel seguro y después pasan a salida.
void configurar_panel(void)
{
    // Se enciende el reloj del puerto C antes de escribir sus registros.
    habilitar_reloj_puerto(GPIOC);
    // OE apagado antes de ser salida: el panel no debe mostrar basura mientras se configura.
    escribir_pin(GPIOC, PIN_PANEL_OE, NIVEL_OE_APAGADO);
    // CLK en 0 antes de ser salida: el reset deja ODR en 0, pero se escribe por claridad.
    escribir_pin(GPIOC, PIN_PANEL_CLK, 0U);
    // LAT en 0 antes de ser salida: el panel no debe cargar datos mientras se configura.
    escribir_pin(GPIOC, PIN_PANEL_LAT, 0U);
    // R1 como salida.
    configurar_salida_del_panel(PIN_PANEL_R1);
    // G1 como salida.
    configurar_salida_del_panel(PIN_PANEL_G1);
    // B1 como salida.
    configurar_salida_del_panel(PIN_PANEL_B1);
    // R2 como salida.
    configurar_salida_del_panel(PIN_PANEL_R2);
    // G2 como salida.
    configurar_salida_del_panel(PIN_PANEL_G2);
    // B2 como salida.
    configurar_salida_del_panel(PIN_PANEL_B2);
    // OE como salida.
    configurar_salida_del_panel(PIN_PANEL_OE);
    // LAT como salida.
    configurar_salida_del_panel(PIN_PANEL_LAT);
    // CLK como salida.
    configurar_salida_del_panel(PIN_PANEL_CLK);
    // A como salida (selección de fase).
    configurar_salida_del_panel(PIN_PANEL_A);
    // B como salida (selección de fase).
    configurar_salida_del_panel(PIN_PANEL_B);
}

// Envía una fase completa. Solo el bit bit_encendido lleva datos; los demás bits van en 0.
void enviar_fase_con_un_bit(uint32_t fase, uint32_t bit_encendido, uint32_t datos)
{
    // Se desplazan los 64 bits de la fase, uno por uno, con CLK.
    for (uint32_t bit = 0U; bit < PANEL_BITS_POR_FASE; bit++) {
        // Si es el bit elegido, se desplazan los datos.
        if (bit == bit_encendido) {
            desplazar_bit(datos);
        // Si no es el bit elegido, se desplaza 0 para que ese píxel quede apagado.
        } else {
            desplazar_bit(0U);
        }
    }
    // OE apagado mientras cambian LAT y la fase: así no se ve basura durante el cambio.
    escribir_pin(GPIOC, PIN_PANEL_OE, NIVEL_OE_APAGADO);
    // LAT en 1: el latch pasa los 64 bits desplazados a la salida. Va después de desplazar todos los bits.
    escribir_pin(GPIOC, PIN_PANEL_LAT, 1U);
    // LAT en 0: termina el pulso de latch.
    escribir_pin(GPIOC, PIN_PANEL_LAT, 0U);
    // A toma el bit 0 de la fase (selección de fila dentro del escaneo 1/4).
    escribir_pin(GPIOC, PIN_PANEL_A, fase & 1U);
    // B toma el bit 1 de la fase.
    escribir_pin(GPIOC, PIN_PANEL_B, (fase >> 1U) & 1U);
    // OE encendido: el panel muestra la fase recién cargada.
    escribir_pin(GPIOC, PIN_PANEL_OE, NIVEL_OE_ENCENDIDO);
}
