/* adaptado de startup_stm32f401xe.s (ST) y del ejemplo 24-ADC-1 del profesor */
/**
  ******************************************************************************
  * @file      startup_stm32f401xe.s
  * @author    MCD Application Team
  * @brief     STM32F401xExx Devices vector table for GCC based toolchains. 
  *            This module performs:
  *                - Set the initial SP
  *                - Set the initial PC == Reset_Handler,
  *                - Set the vector table entries with the exceptions ISR address
  *                - Branches to main in the C library (which eventually
  *                  calls main()).
  *            After Reset the Cortex-M4 processor is in Thread mode,
  *            priority is Privileged, and the Stack is set to Main.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

  .syntax unified   /* sintaxis unificada de ARM (la misma para ARM y Thumb) */
  .cpu cortex-m4    /* instrucciones del Cortex-M4 */
  .fpu softvfp      /* sin FPU: coincide con -mfloat-abi=soft del Makefile */
  .thumb            /* el codigo es Thumb (los Cortex-M solo ejecutan Thumb) */

.global  g_pfnVectors    /* la tabla de vectores es visible para el linker */
.global  Default_Handler /* manejador por defecto */
.global  Reset_Handler   /* manejador de reset: ENTRY del linker script */

/**
 * Reset_Handler: es lo primero que se ejecuta despues de un reset.
 * Prepara la pila, copia .data de FLASH a RAM, pone .bss en cero
 * y llama a main().
*/
    .section  .text.Reset_Handler
  .type  Reset_Handler, %function
Reset_Handler:
  /* Pila: SP = _estack (final de la RAM, 0x20020000; la pila crece hacia abajo) */
  ldr   sp, =_estack

  /* Copiar el valor inicial de .data desde la FLASH (_sidata) hasta la RAM (_sdata a _edata) */
  ldr r0, =_sdata               /* r0 = direccion de inicio de .data en RAM */
  ldr r1, =_edata               /* r1 = direccion de fin de .data en RAM */
  ldr r2, =_sidata              /* r2 = direccion del valor inicial en FLASH */
  movs r3, #0                   /* r3 = desplazamiento en bytes, empieza en 0 */
  b LoopCopyDataInit            /* saltar a la condicion del bucle */

CopyDataInit:
  ldr r4, [r2, r3]              /* leer una palabra (4 bytes) de la FLASH */
  str r4, [r0, r3]              /* escribirla en la RAM */
  adds r3, r3, #4               /* avanzar 4 bytes */

LoopCopyDataInit:
  adds r4, r0, r3               /* r4 = direccion actual en RAM */
  cmp r4, r1                    /* comparar con el fin de .data */
  bcc CopyDataInit              /* si todavia es menor, seguir copiando */

  /* Poner en cero .bss (de _sbss a _ebss), porque en C las globales sin valor inicial valen 0 */
  ldr r2, =_sbss                /* r2 = direccion de inicio de .bss */
  ldr r4, =_ebss                /* r4 = direccion de fin de .bss */
  movs r3, #0                   /* r3 = valor cero */
  b LoopFillZerobss             /* saltar a la condicion del bucle */

FillZerobss:
  str  r3, [r2]                 /* escribir un cero (4 bytes) */
  adds r2, r2, #4               /* avanzar 4 bytes */

LoopFillZerobss:
  cmp r2, r4                    /* comparar con el fin de .bss */
  bcc FillZerobss               /* si todavia es menor, seguir borrando */

  /* Llamar a main() */
  bl  main

  /* Si main retorna, quedarse en un bucle infinito */
  b .
  .size  Reset_Handler, .-Reset_Handler

/**
 * Default_Handler: se ejecuta si llega una interrupcion que no tiene manejador.
 * Entra en un bucle infinito para poder examinar el sistema con el depurador.
*/
    .section  .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
  b  Infinite_Loop
  .size  Default_Handler, .-Default_Handler

/**
 * Tabla de vectores del STM32F446 (RM0390, seccion 10.1.3, tabla 38).
 * El linker la deja al inicio de la FLASH (0x08000000), que es donde el procesador la busca.
 * Cada entrada es una palabra de 4 bytes con la direccion del manejador (0 = reservado).
*/
   .section  .isr_vector,"a",%progbits
  .type  g_pfnVectors, %object

g_pfnVectors:
  .word  _estack                          /* 0x00: valor inicial del puntero de pila (SP) */
  .word  Reset_Handler                    /* 0x04: Reset */
  .word  NMI_Handler                      /* 0x08: NMI (interrupcion no enmascarable) */
  .word  HardFault_Handler                /* 0x0C: HardFault (todas las clases de falla) */
  .word  MemManage_Handler                /* 0x10: MemManage (gestion de memoria) */
  .word  BusFault_Handler                 /* 0x14: BusFault (falla de bus) */
  .word  UsageFault_Handler               /* 0x18: UsageFault (instruccion indefinida o estado ilegal) */
  .word  0                                /* 0x1C: reservado */
  .word  0                                /* 0x20: reservado */
  .word  0                                /* 0x24: reservado */
  .word  0                                /* 0x28: reservado */
  .word  SVC_Handler                      /* 0x2C: SVCall (llamada al sistema) */
  .word  DebugMon_Handler                 /* 0x30: Debug Monitor */
  .word  0                                /* 0x34: reservado */
  .word  PendSV_Handler                   /* 0x38: PendSV */
  .word  SysTick_Handler                  /* 0x3C: SysTick (temporizador del sistema) */

  /* Interrupciones externas: IRQ 0 a 96 (la direccion en la tabla es 0x40 + 4 * numero de IRQ) */
  .word  WWDG_IRQHandler                  /* IRQ 0: Window Watchdog */
  .word  PVD_IRQHandler                   /* IRQ 1: PVD por la linea EXTI */
  .word  TAMP_STAMP_IRQHandler            /* IRQ 2: Tamper y TimeStamp por la linea EXTI */
  .word  RTC_WKUP_IRQHandler              /* IRQ 3: RTC Wakeup por la linea EXTI */
  .word  FLASH_IRQHandler                 /* IRQ 4: FLASH */
  .word  RCC_IRQHandler                   /* IRQ 5: RCC */
  .word  EXTI0_IRQHandler                 /* IRQ 6: EXTI linea 0 */
  .word  EXTI1_IRQHandler                 /* IRQ 7: EXTI linea 1 */
  .word  EXTI2_IRQHandler                 /* IRQ 8: EXTI linea 2 */
  .word  EXTI3_IRQHandler                 /* IRQ 9: EXTI linea 3 */
  .word  EXTI4_IRQHandler                 /* IRQ 10: EXTI linea 4 */
  .word  DMA1_Stream0_IRQHandler          /* IRQ 11: DMA1 stream 0 */
  .word  DMA1_Stream1_IRQHandler          /* IRQ 12: DMA1 stream 1 */
  .word  DMA1_Stream2_IRQHandler          /* IRQ 13: DMA1 stream 2 */
  .word  DMA1_Stream3_IRQHandler          /* IRQ 14: DMA1 stream 3 */
  .word  DMA1_Stream4_IRQHandler          /* IRQ 15: DMA1 stream 4 */
  .word  DMA1_Stream5_IRQHandler          /* IRQ 16: DMA1 stream 5 */
  .word  DMA1_Stream6_IRQHandler          /* IRQ 17: DMA1 stream 6 */
  .word  ADC_IRQHandler                   /* IRQ 18: ADC1, ADC2 y ADC3 */
  .word  CAN1_TX_IRQHandler               /* IRQ 19: CAN1 TX */
  .word  CAN1_RX0_IRQHandler              /* IRQ 20: CAN1 RX0 */
  .word  CAN1_RX1_IRQHandler              /* IRQ 21: CAN1 RX1 */
  .word  CAN1_SCE_IRQHandler              /* IRQ 22: CAN1 SCE */
  .word  EXTI9_5_IRQHandler               /* IRQ 23: EXTI lineas 9 a 5 */
  .word  TIM1_BRK_TIM9_IRQHandler         /* IRQ 24: TIM1 Break y TIM9 */
  .word  TIM1_UP_TIM10_IRQHandler         /* IRQ 25: TIM1 Update y TIM10 */
  .word  TIM1_TRG_COM_TIM11_IRQHandler    /* IRQ 26: TIM1 Trigger/Commutation y TIM11 */
  .word  TIM1_CC_IRQHandler               /* IRQ 27: TIM1 Capture Compare */
  .word  TIM2_IRQHandler                  /* IRQ 28: TIM2 */
  .word  TIM3_IRQHandler                  /* IRQ 29: TIM3 */
  .word  TIM4_IRQHandler                  /* IRQ 30: TIM4 */
  .word  I2C1_EV_IRQHandler               /* IRQ 31: I2C1 evento */
  .word  I2C1_ER_IRQHandler               /* IRQ 32: I2C1 error */
  .word  I2C2_EV_IRQHandler               /* IRQ 33: I2C2 evento */
  .word  I2C2_ER_IRQHandler               /* IRQ 34: I2C2 error */
  .word  SPI1_IRQHandler                  /* IRQ 35: SPI1 */
  .word  SPI2_IRQHandler                  /* IRQ 36: SPI2 */
  .word  USART1_IRQHandler                /* IRQ 37: USART1 */
  .word  USART2_IRQHandler                /* IRQ 38: USART2 */
  .word  USART3_IRQHandler                /* IRQ 39: USART3 */
  .word  EXTI15_10_IRQHandler             /* IRQ 40: EXTI lineas 15 a 10 */
  .word  RTC_Alarm_IRQHandler             /* IRQ 41: Alarmas A y B del RTC por la linea EXTI */
  .word  OTG_FS_WKUP_IRQHandler           /* IRQ 42: USB OTG FS Wakeup por la linea EXTI */
  .word  TIM8_BRK_TIM12_IRQHandler        /* IRQ 43: TIM8 Break y TIM12 */
  .word  TIM8_UP_TIM13_IRQHandler         /* IRQ 44: TIM8 Update y TIM13 */
  .word  TIM8_TRG_COM_TIM14_IRQHandler    /* IRQ 45: TIM8 Trigger/Commutation y TIM14 */
  .word  TIM8_CC_IRQHandler               /* IRQ 46: TIM8 Capture Compare */
  .word  DMA1_Stream7_IRQHandler          /* IRQ 47: DMA1 stream 7 */
  .word  FMC_IRQHandler                   /* IRQ 48: FMC */
  .word  SDIO_IRQHandler                  /* IRQ 49: SDIO */
  .word  TIM5_IRQHandler                  /* IRQ 50: TIM5 */
  .word  SPI3_IRQHandler                  /* IRQ 51: SPI3 */
  .word  UART4_IRQHandler                 /* IRQ 52: UART4 */
  .word  UART5_IRQHandler                 /* IRQ 53: UART5 */
  .word  TIM6_DAC_IRQHandler              /* IRQ 54: TIM6 y errores de underrun de DAC1 y DAC2 */
  .word  TIM7_IRQHandler                  /* IRQ 55: TIM7 */
  .word  DMA2_Stream0_IRQHandler          /* IRQ 56: DMA2 stream 0 */
  .word  DMA2_Stream1_IRQHandler          /* IRQ 57: DMA2 stream 1 */
  .word  DMA2_Stream2_IRQHandler          /* IRQ 58: DMA2 stream 2 */
  .word  DMA2_Stream3_IRQHandler          /* IRQ 59: DMA2 stream 3 */
  .word  DMA2_Stream4_IRQHandler          /* IRQ 60: DMA2 stream 4 */
  .word  0                                /* IRQ 61: reservado */
  .word  0                                /* IRQ 62: reservado */
  .word  CAN2_TX_IRQHandler               /* IRQ 63: CAN2 TX */
  .word  CAN2_RX0_IRQHandler              /* IRQ 64: CAN2 RX0 */
  .word  CAN2_RX1_IRQHandler              /* IRQ 65: CAN2 RX1 */
  .word  CAN2_SCE_IRQHandler              /* IRQ 66: CAN2 SCE */
  .word  OTG_FS_IRQHandler                /* IRQ 67: USB OTG FS */
  .word  DMA2_Stream5_IRQHandler          /* IRQ 68: DMA2 stream 5 */
  .word  DMA2_Stream6_IRQHandler          /* IRQ 69: DMA2 stream 6 */
  .word  DMA2_Stream7_IRQHandler          /* IRQ 70: DMA2 stream 7 */
  .word  USART6_IRQHandler                /* IRQ 71: USART6 */
  .word  I2C3_EV_IRQHandler               /* IRQ 72: I2C3 evento */
  .word  I2C3_ER_IRQHandler               /* IRQ 73: I2C3 error */
  .word  OTG_HS_EP1_OUT_IRQHandler        /* IRQ 74: USB OTG HS endpoint 1 OUT */
  .word  OTG_HS_EP1_IN_IRQHandler         /* IRQ 75: USB OTG HS endpoint 1 IN */
  .word  OTG_HS_WKUP_IRQHandler           /* IRQ 76: USB OTG HS Wakeup por la linea EXTI */
  .word  OTG_HS_IRQHandler                /* IRQ 77: USB OTG HS */
  .word  DCMI_IRQHandler                  /* IRQ 78: DCMI */
  .word  0                                /* IRQ 79: reservado */
  .word  0                                /* IRQ 80: reservado */
  .word  FPU_IRQHandler                   /* IRQ 81: FPU */
  .word  0                                /* IRQ 82: reservado */
  .word  0                                /* IRQ 83: reservado */
  .word  SPI4_IRQHandler                  /* IRQ 84: SPI4 */
  .word  0                                /* IRQ 85: reservado */
  .word  0                                /* IRQ 86: reservado */
  .word  SAI1_IRQHandler                  /* IRQ 87: SAI1 */
  .word  0                                /* IRQ 88: reservado */
  .word  0                                /* IRQ 89: reservado */
  .word  0                                /* IRQ 90: reservado */
  .word  SAI2_IRQHandler                  /* IRQ 91: SAI2 */
  .word  QUADSPI_IRQHandler               /* IRQ 92: QuadSPI */
  .word  CEC_IRQHandler                   /* IRQ 93: HDMI-CEC */
  .word  SPDIFRX_IRQHandler               /* IRQ 94: SPDIF-Rx */
  .word  FMPI2C1_EV_IRQHandler            /* IRQ 95: FMPI2C1 evento */
  .word  FMPI2C1_ER_IRQHandler            /* IRQ 96: FMPI2C1 error */

  .size  g_pfnVectors, .-g_pfnVectors

/**
 * Alias debiles: cada manejador apunta a Default_Handler. Si en C se define una
 * funcion con el mismo nombre (por ejemplo TIM3_IRQHandler), esa funcion reemplaza al alias.
*/
   .weak      NMI_Handler
   .thumb_set NMI_Handler,Default_Handler

   .weak      HardFault_Handler
   .thumb_set HardFault_Handler,Default_Handler

   .weak      MemManage_Handler
   .thumb_set MemManage_Handler,Default_Handler

   .weak      BusFault_Handler
   .thumb_set BusFault_Handler,Default_Handler

   .weak      UsageFault_Handler
   .thumb_set UsageFault_Handler,Default_Handler

   .weak      SVC_Handler
   .thumb_set SVC_Handler,Default_Handler

   .weak      DebugMon_Handler
   .thumb_set DebugMon_Handler,Default_Handler

   .weak      PendSV_Handler
   .thumb_set PendSV_Handler,Default_Handler

   .weak      SysTick_Handler
   .thumb_set SysTick_Handler,Default_Handler

   .weak      WWDG_IRQHandler
   .thumb_set WWDG_IRQHandler,Default_Handler

   .weak      PVD_IRQHandler
   .thumb_set PVD_IRQHandler,Default_Handler

   .weak      TAMP_STAMP_IRQHandler
   .thumb_set TAMP_STAMP_IRQHandler,Default_Handler

   .weak      RTC_WKUP_IRQHandler
   .thumb_set RTC_WKUP_IRQHandler,Default_Handler

   .weak      FLASH_IRQHandler
   .thumb_set FLASH_IRQHandler,Default_Handler

   .weak      RCC_IRQHandler
   .thumb_set RCC_IRQHandler,Default_Handler

   .weak      EXTI0_IRQHandler
   .thumb_set EXTI0_IRQHandler,Default_Handler

   .weak      EXTI1_IRQHandler
   .thumb_set EXTI1_IRQHandler,Default_Handler

   .weak      EXTI2_IRQHandler
   .thumb_set EXTI2_IRQHandler,Default_Handler

   .weak      EXTI3_IRQHandler
   .thumb_set EXTI3_IRQHandler,Default_Handler

   .weak      EXTI4_IRQHandler
   .thumb_set EXTI4_IRQHandler,Default_Handler

   .weak      DMA1_Stream0_IRQHandler
   .thumb_set DMA1_Stream0_IRQHandler,Default_Handler

   .weak      DMA1_Stream1_IRQHandler
   .thumb_set DMA1_Stream1_IRQHandler,Default_Handler

   .weak      DMA1_Stream2_IRQHandler
   .thumb_set DMA1_Stream2_IRQHandler,Default_Handler

   .weak      DMA1_Stream3_IRQHandler
   .thumb_set DMA1_Stream3_IRQHandler,Default_Handler

   .weak      DMA1_Stream4_IRQHandler
   .thumb_set DMA1_Stream4_IRQHandler,Default_Handler

   .weak      DMA1_Stream5_IRQHandler
   .thumb_set DMA1_Stream5_IRQHandler,Default_Handler

   .weak      DMA1_Stream6_IRQHandler
   .thumb_set DMA1_Stream6_IRQHandler,Default_Handler

   .weak      ADC_IRQHandler
   .thumb_set ADC_IRQHandler,Default_Handler

   .weak      CAN1_TX_IRQHandler
   .thumb_set CAN1_TX_IRQHandler,Default_Handler

   .weak      CAN1_RX0_IRQHandler
   .thumb_set CAN1_RX0_IRQHandler,Default_Handler

   .weak      CAN1_RX1_IRQHandler
   .thumb_set CAN1_RX1_IRQHandler,Default_Handler

   .weak      CAN1_SCE_IRQHandler
   .thumb_set CAN1_SCE_IRQHandler,Default_Handler

   .weak      EXTI9_5_IRQHandler
   .thumb_set EXTI9_5_IRQHandler,Default_Handler

   .weak      TIM1_BRK_TIM9_IRQHandler
   .thumb_set TIM1_BRK_TIM9_IRQHandler,Default_Handler

   .weak      TIM1_UP_TIM10_IRQHandler
   .thumb_set TIM1_UP_TIM10_IRQHandler,Default_Handler

   .weak      TIM1_TRG_COM_TIM11_IRQHandler
   .thumb_set TIM1_TRG_COM_TIM11_IRQHandler,Default_Handler

   .weak      TIM1_CC_IRQHandler
   .thumb_set TIM1_CC_IRQHandler,Default_Handler

   .weak      TIM2_IRQHandler
   .thumb_set TIM2_IRQHandler,Default_Handler

   .weak      TIM3_IRQHandler
   .thumb_set TIM3_IRQHandler,Default_Handler

   .weak      TIM4_IRQHandler
   .thumb_set TIM4_IRQHandler,Default_Handler

   .weak      I2C1_EV_IRQHandler
   .thumb_set I2C1_EV_IRQHandler,Default_Handler

   .weak      I2C1_ER_IRQHandler
   .thumb_set I2C1_ER_IRQHandler,Default_Handler

   .weak      I2C2_EV_IRQHandler
   .thumb_set I2C2_EV_IRQHandler,Default_Handler

   .weak      I2C2_ER_IRQHandler
   .thumb_set I2C2_ER_IRQHandler,Default_Handler

   .weak      SPI1_IRQHandler
   .thumb_set SPI1_IRQHandler,Default_Handler

   .weak      SPI2_IRQHandler
   .thumb_set SPI2_IRQHandler,Default_Handler

   .weak      USART1_IRQHandler
   .thumb_set USART1_IRQHandler,Default_Handler

   .weak      USART2_IRQHandler
   .thumb_set USART2_IRQHandler,Default_Handler

   .weak      USART3_IRQHandler
   .thumb_set USART3_IRQHandler,Default_Handler

   .weak      EXTI15_10_IRQHandler
   .thumb_set EXTI15_10_IRQHandler,Default_Handler

   .weak      RTC_Alarm_IRQHandler
   .thumb_set RTC_Alarm_IRQHandler,Default_Handler

   .weak      OTG_FS_WKUP_IRQHandler
   .thumb_set OTG_FS_WKUP_IRQHandler,Default_Handler

   .weak      TIM8_BRK_TIM12_IRQHandler
   .thumb_set TIM8_BRK_TIM12_IRQHandler,Default_Handler

   .weak      TIM8_UP_TIM13_IRQHandler
   .thumb_set TIM8_UP_TIM13_IRQHandler,Default_Handler

   .weak      TIM8_TRG_COM_TIM14_IRQHandler
   .thumb_set TIM8_TRG_COM_TIM14_IRQHandler,Default_Handler

   .weak      TIM8_CC_IRQHandler
   .thumb_set TIM8_CC_IRQHandler,Default_Handler

   .weak      DMA1_Stream7_IRQHandler
   .thumb_set DMA1_Stream7_IRQHandler,Default_Handler

   .weak      FMC_IRQHandler
   .thumb_set FMC_IRQHandler,Default_Handler

   .weak      SDIO_IRQHandler
   .thumb_set SDIO_IRQHandler,Default_Handler

   .weak      TIM5_IRQHandler
   .thumb_set TIM5_IRQHandler,Default_Handler

   .weak      SPI3_IRQHandler
   .thumb_set SPI3_IRQHandler,Default_Handler

   .weak      UART4_IRQHandler
   .thumb_set UART4_IRQHandler,Default_Handler

   .weak      UART5_IRQHandler
   .thumb_set UART5_IRQHandler,Default_Handler

   .weak      TIM6_DAC_IRQHandler
   .thumb_set TIM6_DAC_IRQHandler,Default_Handler

   .weak      TIM7_IRQHandler
   .thumb_set TIM7_IRQHandler,Default_Handler

   .weak      DMA2_Stream0_IRQHandler
   .thumb_set DMA2_Stream0_IRQHandler,Default_Handler

   .weak      DMA2_Stream1_IRQHandler
   .thumb_set DMA2_Stream1_IRQHandler,Default_Handler

   .weak      DMA2_Stream2_IRQHandler
   .thumb_set DMA2_Stream2_IRQHandler,Default_Handler

   .weak      DMA2_Stream3_IRQHandler
   .thumb_set DMA2_Stream3_IRQHandler,Default_Handler

   .weak      DMA2_Stream4_IRQHandler
   .thumb_set DMA2_Stream4_IRQHandler,Default_Handler

   .weak      CAN2_TX_IRQHandler
   .thumb_set CAN2_TX_IRQHandler,Default_Handler

   .weak      CAN2_RX0_IRQHandler
   .thumb_set CAN2_RX0_IRQHandler,Default_Handler

   .weak      CAN2_RX1_IRQHandler
   .thumb_set CAN2_RX1_IRQHandler,Default_Handler

   .weak      CAN2_SCE_IRQHandler
   .thumb_set CAN2_SCE_IRQHandler,Default_Handler

   .weak      OTG_FS_IRQHandler
   .thumb_set OTG_FS_IRQHandler,Default_Handler

   .weak      DMA2_Stream5_IRQHandler
   .thumb_set DMA2_Stream5_IRQHandler,Default_Handler

   .weak      DMA2_Stream6_IRQHandler
   .thumb_set DMA2_Stream6_IRQHandler,Default_Handler

   .weak      DMA2_Stream7_IRQHandler
   .thumb_set DMA2_Stream7_IRQHandler,Default_Handler

   .weak      USART6_IRQHandler
   .thumb_set USART6_IRQHandler,Default_Handler

   .weak      I2C3_EV_IRQHandler
   .thumb_set I2C3_EV_IRQHandler,Default_Handler

   .weak      I2C3_ER_IRQHandler
   .thumb_set I2C3_ER_IRQHandler,Default_Handler

   .weak      OTG_HS_EP1_OUT_IRQHandler
   .thumb_set OTG_HS_EP1_OUT_IRQHandler,Default_Handler

   .weak      OTG_HS_EP1_IN_IRQHandler
   .thumb_set OTG_HS_EP1_IN_IRQHandler,Default_Handler

   .weak      OTG_HS_WKUP_IRQHandler
   .thumb_set OTG_HS_WKUP_IRQHandler,Default_Handler

   .weak      OTG_HS_IRQHandler
   .thumb_set OTG_HS_IRQHandler,Default_Handler

   .weak      DCMI_IRQHandler
   .thumb_set DCMI_IRQHandler,Default_Handler

   .weak      FPU_IRQHandler
   .thumb_set FPU_IRQHandler,Default_Handler

   .weak      SPI4_IRQHandler
   .thumb_set SPI4_IRQHandler,Default_Handler

   .weak      SAI1_IRQHandler
   .thumb_set SAI1_IRQHandler,Default_Handler

   .weak      SAI2_IRQHandler
   .thumb_set SAI2_IRQHandler,Default_Handler

   .weak      QUADSPI_IRQHandler
   .thumb_set QUADSPI_IRQHandler,Default_Handler

   .weak      CEC_IRQHandler
   .thumb_set CEC_IRQHandler,Default_Handler

   .weak      SPDIFRX_IRQHandler
   .thumb_set SPDIFRX_IRQHandler,Default_Handler

   .weak      FMPI2C1_EV_IRQHandler
   .thumb_set FMPI2C1_EV_IRQHandler,Default_Handler

   .weak      FMPI2C1_ER_IRQHandler
   .thumb_set FMPI2C1_ER_IRQHandler,Default_Handler
