#include "panel.h"

// Divisor de TIM3: 84 MHz / (83 + 1) = 1 MHz, es decir, el contador avanza una cuenta cada 1 µs.
#define TIM3_PRESCALER 83U
// Valor de recarga de TIM3: 1 MHz / (999 + 1) = 1 kHz, es decir, una interrupción cada 1 ms.
#define TIM3_AUTORRECARGA 999U

// Imagen del tablero: un color por píxel, indexado [y][x]. Los bits 0 a 2 son R, G y B.
// volatile porque la interrupción de TIM3 lee este arreglo mientras main lo escribe.
// Está en .bss, así que empieza en 0 (COLOR_NEGRO).
static volatile uint8_t framebuffer[PANEL_ALTO][PANEL_ANCHO];
// Fase que se dibuja en la siguiente interrupción de TIM3. Solo la usa la interrupción, por eso no necesita volatile.
static uint32_t fase_actual = 0U;

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

// Dibuja una fase completa con los colores del framebuffer. Fórmula de docs/mapeo_pixeles.md:
//   bloque = bit / 16, j = bit % 16
//   si j < 8:  x = 8 * bloque + (7 - j), y = fase        (de derecha a izquierda)
//   si j >= 8: x = 8 * bloque + (j - 8), y = fase + 4    (de izquierda a derecha)
// R2, G2 y B2 usan la fila y + 8.
static void dibujar_fase(uint32_t fase)
{
    // Se desplazan los 64 bits de la fase, uno por uno, con CLK.
    for (uint32_t bit = 0U; bit < PANEL_BITS_POR_FASE; bit++) {
        // Bloque de 16 bits al que pertenece el bit (0 a 3).
        uint32_t bloque = bit / PANEL_BITS_POR_BLOQUE;
        // Posición del bit dentro de su bloque (0 a 15).
        uint32_t j = bit % PANEL_BITS_POR_BLOQUE;
        // Columna del píxel de la mitad de arriba que corresponde a este bit.
        uint32_t x;
        // Fila del píxel de la mitad de arriba que corresponde a este bit.
        uint32_t y;
        // Mitad j < 8: las columnas van de derecha a izquierda y la fila es la de la fase.
        if (j < PANEL_COLUMNAS_POR_BLOQUE) {
            x = PANEL_COLUMNAS_POR_BLOQUE * bloque + (PANEL_COLUMNAS_POR_BLOQUE - 1U - j);
            y = fase;
        // Mitad j >= 8: las columnas van de izquierda a derecha y la fila es la de la fase + 4.
        } else {
            x = PANEL_COLUMNAS_POR_BLOQUE * bloque + (j - PANEL_COLUMNAS_POR_BLOQUE);
            y = fase + PANEL_SEPARACION_FILAS;
        }
        // datos: bits 0 a 2 = R1, G1, B1 (fila y); bits 3 a 5 = R2, G2, B2 (fila y + 8), desplazados 3 posiciones.
        uint32_t datos = framebuffer[y][x] | (framebuffer[y + PANEL_FILAS_MITAD_SUPERIOR][x] << PANEL_DESPLAZAMIENTO_MITAD_INFERIOR);
        // Se desplazan los 6 bits de color de este píxel.
        desplazar_bit(datos);
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

// Guarda el color de un píxel en el framebuffer. Si x o y están fuera del tablero, no hace nada:
// escribir fuera del arreglo dañaría otra zona de la memoria.
void encender_pixel(uint32_t x, uint32_t y, uint32_t color)
{
    // Solo se escribe si x está dentro de las columnas y y dentro de las filas del tablero.
    if (x < PANEL_ANCHO && y < PANEL_ALTO) {
        // Se guarda el color en 3 bits (R = bit 0, G = bit 1, B = bit 2). Cabe en uint8_t sin perder datos.
        framebuffer[y][x] = (uint8_t)color;
    }
}

// Enciende TIM3 para que interrumpa cada 1 ms. Cada interrupción dibuja una fase del panel.
void iniciar_refresco_panel(void)
{
    // (a) Se enciende el reloj de TIM3 en el bus APB1. |= no cambia los demás bits de RCC->APB1ENR.
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    // Variable que solo sirve para leer el registro; el valor leído no se usa.
    volatile uint32_t lectura_descartada;
    // Dos lecturas después de habilitar el reloj, como en habilitar_reloj_puerto: el periférico necesita unos ciclos antes de usarse.
    lectura_descartada = RCC->APB1ENR;
    lectura_descartada = RCC->APB1ENR;
    // El valor leído no se usa: (void) le dice al compilador que se descarta a propósito (evita la advertencia).
    (void)lectura_descartada;
    // (b) Divisor y valor de recarga: el contador cuenta de 0 a 999 con un reloj de 1 MHz, es decir, 1 ms.
    TIM3->PSC = TIM3_PRESCALER;
    TIM3->ARR = TIM3_AUTORRECARGA;
    // (c) UG carga PSC y ARR en el momento; sin esto, el divisor nuevo esperaría a la primera actualización.
    TIM3->EGR = TIM_EGR_UG;
    // (d) Se limpia el UIF que dejó UG: se escribe 0 en ese bit y 1 en los demás (rc_w0 no cambia con 1).
    TIM3->SR = ~TIM_SR_UIF;
    // (e) Se habilita la interrupción de actualización de TIM3 (UIE).
    TIM3->DIER |= TIM_DIER_UIE;
    // (f) Se habilita la IRQ 29 en el NVIC. Escribir 0 en los demás bits no cambia nada, por eso no hace falta leer ISER0.
    NVIC->ISER0 = (1U << NUMERO_INTERRUPCION_TIM3);
    // (g) Se enciende el contador de TIM3 (CEN). A partir de aquí llegan interrupciones cada 1 ms.
    TIM3->CR1 |= TIM_CR1_CEN;
}

// Manejador de la interrupción de TIM3. El nombre es el de la tabla de vectores de crt0.S, así que reemplaza al alias débil.
// Se ejecuta cada 1 ms y dibuja una fase (0, 1, 2, 3, 0...): cada fase se repite 250 veces por segundo.
void TIM3_IRQHandler(void)
{
    // Se limpia la causa de la interrupción (UIF) escribiendo 0 en ese bit; si no, la interrupción se repetiría sin fin.
    TIM3->SR = ~TIM_SR_UIF;
    // Se dibuja la fase actual con los colores que hay en el framebuffer.
    dibujar_fase(fase_actual);
    // Se pasa a la siguiente fase. % PANEL_FASES hace que después de la fase 3 vuelva a la 0.
    fase_actual = (fase_actual + 1U) % PANEL_FASES;
}
