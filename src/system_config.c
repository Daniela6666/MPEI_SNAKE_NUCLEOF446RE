#include "system_config.h"

// Estados de espera de la FLASH (LATENCY = 2). Con VDD de 2.7 a 3.6 V, 2 estados de espera son necesarios para 60 < HCLK ≤ 90 MHz. Fuente: RM0390, sección 3.4.1, tabla 5 «Number of wait states according to CPU clock (HCLK) frequency».
#define FLASH_ESTADOS_DE_ESPERA 2U
// Divisor M del PLL: 16 MHz de la HSI / 16 = 1 MHz, que es la entrada del VCO. Fuente: RM0390, sección 6.3.2 (RCC_PLLCFGR).
#define PLL_DIVISOR_ENTRADA_M 16U
// Multiplicador N del PLL: 1 MHz × 336 = 336 MHz en el VCO. Fuente: RM0390, sección 6.3.2 (RCC_PLLCFGR).
#define PLL_MULTIPLICADOR_N 336U
// Código del divisor P del PLL: 01 = dividir entre 4, 336 MHz / 4 = 84 MHz. Fuente: RM0390, sección 6.3.2 (00 = /2, 01 = /4, 10 = /6, 11 = /8).
#define PLL_CODIGO_DIVISOR_SALIDA_P 1U
// Código HPRE = 0xxx: el AHB no se divide, HCLK = SYSCLK = 84 MHz. Fuente: RM0390, sección 6.3.3 (RCC_CFGR, HPRE).
#define AHB_SIN_DIVISION 0U
// Código PPRE1 = 100: el APB1 se divide entre 2, APB1 = 42 MHz (el máximo permitido es 45 MHz). Fuente: máximo de 45 MHz en RM0390, sección 6.2 «Clocks»; código del campo en la sección 6.3.3 (RCC_CFGR, PPRE1).
#define APB1_DIVIDIR_ENTRE_2 4U
// Código PPRE2 = 0xx: el APB2 no se divide, APB2 = 84 MHz (el máximo permitido es 90 MHz). Fuente: máximo de 90 MHz en RM0390, sección 6.2 «Clocks»; código del campo en la sección 6.3.3 (RCC_CFGR, PPRE2).
#define APB2_SIN_DIVISION 0U
// Código SW y SWS = 10: el PLL es la fuente del reloj del sistema. Fuente: RM0390, sección 6.3.3 (RCC_CFGR, SW y SWS).
#define RELOJ_SISTEMA_PLL 2U

// Cuenta milisegundos desde el arranque. Es volatile porque la interrupción la cambia sin que main lo sepa.
// Es static porque solo este archivo la usa.
static volatile uint32_t contador_milisegundos = 0U;

// Temporizador de software que SysTick_Handler actualiza cada milisegundo.
typedef struct {
    // Milisegundos desde el último vencimiento. SysTick_Handler lo suma cada 1 ms y lo pone en 0 al vencer.
    uint32_t contador;
    // Milisegundos entre un vencimiento y el siguiente. Si vale 0, el temporizador está detenido.
    uint32_t periodo;
    // Vale 1 cuando el temporizador venció y todavía no se consultó. main la lee y la baja a 0.
    uint32_t bandera;
} TemporizadorSoftware;

// Temporizador de movimiento: una sola instancia, arranca detenido (periodo 0).
// Es volatile porque SysTick_Handler la cambia sin que main lo sepa.
static volatile TemporizadorSoftware temporizador_movimiento = { 0U, 0U, 0U };

void configurar_reloj(void)
{
    // Se escribe solo el campo LATENCY: la máscara va entre paréntesis en su definición para que ~ afecte solo a esos bits, y no se sobrescribe FLASH->ACR completo.
    // Primero la latencia de la FLASH, porque la FLASH es más lenta que 84 MHz y debe esperar más ciclos antes de subir la frecuencia.
    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY_MASCARA) | (FLASH_ESTADOS_DE_ESPERA << FLASH_ACR_LATENCY_POSICION);
    // El manual (RM0390, sección 3.4.1) recomienda releer LATENCY y esperar a que quede escrito antes de cambiar el reloj.
    while (((FLASH->ACR & FLASH_ACR_LATENCY_MASCARA) >> FLASH_ACR_LATENCY_POSICION) != FLASH_ESTADOS_DE_ESPERA) { }

    // PLLSRC queda en 0 (HSI de 16 MHz, valor tras el reset). PLLQ y PLLR no se tocan y conservan su valor de reset.
    // El PLL está apagado, por eso se puede escribir PLLCFGR. Solo se escriben los campos PLLM, PLLN y PLLP, no el registro completo.
    // Campo PLLM: divisor de entrada del PLL.
    RCC->PLLCFGR = (RCC->PLLCFGR & ~RCC_PLLCFGR_PLLM_MASCARA) | (PLL_DIVISOR_ENTRADA_M << RCC_PLLCFGR_PLLM_POSICION);
    // Campo PLLN: multiplicador del VCO.
    RCC->PLLCFGR = (RCC->PLLCFGR & ~RCC_PLLCFGR_PLLN_MASCARA) | (PLL_MULTIPLICADOR_N << RCC_PLLCFGR_PLLN_POSICION);
    // Campo PLLP: divisor de salida del PLL que da el reloj del sistema.
    RCC->PLLCFGR = (RCC->PLLCFGR & ~RCC_PLLCFGR_PLLP_MASCARA) | (PLL_CODIGO_DIVISOR_SALIDA_P << RCC_PLLCFGR_PLLP_POSICION);

    // Divisores de los buses, antes de subir la frecuencia para que ningún bus pase de su máximo.
    // Campo HPRE: divisor del AHB.
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_HPRE_MASCARA) | (AHB_SIN_DIVISION << RCC_CFGR_HPRE_POSICION);
    // Campo PPRE1: divisor del APB1 (42 MHz; TIM3 recibirá 84 MHz).
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_PPRE1_MASCARA) | (APB1_DIVIDIR_ENTRE_2 << RCC_CFGR_PPRE1_POSICION);
    // Campo PPRE2: divisor del APB2.
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_PPRE2_MASCARA) | (APB2_SIN_DIVISION << RCC_CFGR_PPRE2_POSICION);

    // Se enciende el PLL con |= para no cambiar los demás bits de CR.
    RCC->CR |= RCC_CR_PLLON;
    // Se espera a que el PLL enganche: PLLRDY = 1 indica que el PLL está listo.
    while ((RCC->CR & RCC_CR_PLLRDY) == 0U) { }

    // Campo SW: se elige el PLL como fuente del reloj del sistema.
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_MASCARA) | (RELOJ_SISTEMA_PLL << RCC_CFGR_SW_POSICION);
    // Se espera a que el hardware confirme el cambio: SWS muestra la fuente que está usando de verdad.
    while ((RCC->CFGR & RCC_CFGR_SWS_MASCARA) != (RELOJ_SISTEMA_PLL << RCC_CFGR_SWS_POSICION)) { }
}

void iniciar_systick(void)
{
    // Se apaga el contador mientras se configura: ENABLE = 0 detiene la cuenta.
    SysTick->CTRL &= ~SYSTICK_CTRL_ENABLE;
    // 84 000 000 / 1000 = 84 000 ciclos por milisegundo. El contador cuenta desde LOAD hasta 0, o sea LOAD + 1 ciclos, por eso se resta 1.
    // LOAD = 83 999: cabe en los 24 bits (máximo 16 777 215).
    SysTick->LOAD = (FRECUENCIA_RELOJ_HZ / INTERRUPCIONES_SYSTICK_POR_SEGUNDO) - 1U;
    // Se pone el contador en 0 para que empiece desde LOAD en la primera cuenta.
    SysTick->VAL = 0U;
    // CLKSOURCE = 1: el contador usa el reloj del procesador (84 MHz), no AHB/8.
    SysTick->CTRL |= SYSTICK_CTRL_CLKSOURCE;
    // TICKINT = 1: al llegar a 0 se genera la interrupción. SysTick es una excepción del núcleo y no necesita habilitarse en el NVIC.
    SysTick->CTRL |= SYSTICK_CTRL_TICKINT;
    // ENABLE = 1: se enciende el contador al final, cuando ya está todo configurado.
    SysTick->CTRL |= SYSTICK_CTRL_ENABLE;
}

// El nombre es el de la tabla de vectores de crt0.S (posición 0x3C). Al definirla aquí reemplaza al alias débil que apunta a Default_Handler.
// Se ejecuta cada 1 ms.
void SysTick_Handler(void)
{
    // Suma 1 milisegundo al contador.
    contador_milisegundos++;
    // Suma 1 milisegundo al contador del temporizador de movimiento.
    temporizador_movimiento.contador++;
    // Si el periodo es 0, el temporizador está detenido y no vence. Si no, se compara con >= y no con ==:
    // así, aunque el contador haya pasado del periodo (por ejemplo, al cambiar el periodo), igual vence.
    if (temporizador_movimiento.periodo != 0U && temporizador_movimiento.contador >= temporizador_movimiento.periodo) {
        // Se reinicia el contador para contar otra vez desde 0 hasta el periodo.
        temporizador_movimiento.contador = 0U;
        // Se avisa a main que el temporizador venció: la bandera queda en 1 hasta que main la consulte.
        temporizador_movimiento.bandera = 1U;
    }
}

void esperar_milisegundos(uint32_t milisegundos)
{
    // Se guarda el instante en que empieza la espera.
    uint32_t instante_inicial = contador_milisegundos;
    // La resta funciona aunque el contador de 32 bits dé la vuelta. main espera aquí y la interrupción sigue contando.
    while ((contador_milisegundos - instante_inicial) < milisegundos) { }
}

void iniciar_temporizador_movimiento(uint32_t periodo_ms)
{
    // Primero se pone el contador en 0 para que la cuenta empiece desde el principio.
    temporizador_movimiento.contador = 0U;
    // Se baja la bandera para que no quede un vencimiento viejo pendiente.
    temporizador_movimiento.bandera = 0U;
    // Se guarda el periodo nuevo. Va al final porque, al escribirlo, el temporizador empieza a poder vencer.
    temporizador_movimiento.periodo = periodo_ms;
}

uint32_t temporizador_movimiento_vencio(void)
{
    // Si la bandera está en 1, el temporizador venció desde la última consulta.
    if (temporizador_movimiento.bandera != 0U) {
        // Se baja la bandera para consumir el aviso: la siguiente consulta devuelve 0 si no vuelve a vencer.
        // La interrupción podría poner la bandera en 1 justo entre el if y esta línea, y ese vencimiento se perdería.
        // Con un periodo de cientos de ms no importa: el siguiente vencimiento se detecta igual.
        temporizador_movimiento.bandera = 0U;
        // Devuelve 1: sí venció.
        return 1U;
    }
    // La bandera está en 0: no venció desde la última consulta.
    return 0U;
}
