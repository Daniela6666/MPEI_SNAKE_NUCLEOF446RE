/* Primera instruccion que ejecuta el programa: el manejador de reset (definido en crt0.S) */
ENTRY(Reset_Handler)

/* Mapa de memoria del STM32F446RE (RM0390) */
MEMORY
{
    /* Memoria flash: aqui se guarda el programa. Se lee y se ejecuta, no se escribe en ejecucion */
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 512K
    /* SRAM1 (112 KB) + SRAM2 (16 KB), contiguas: variables y pila */
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

/* Valor inicial del puntero de pila: el final de la RAM (0x20020000). La pila crece hacia abajo */
_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    /* Codigo: primero la tabla de vectores (queda en 0x08000000) y despues las funciones */
    .text :
    {
        *(.isr_vector)
        *(.text*)
    } > FLASH

    /* Constantes (const, textos): solo lectura, en flash */
    .rodata :
    {
        *(.rodata*)
        /* Alinear a 4 bytes para que el valor inicial de .data (en flash) empiece en direccion multiplo de 4 */
        . = ALIGN(4);
    } > FLASH

    /* Direccion en flash donde esta guardado el valor inicial de .data */
    _sidata = LOADADDR(.data);

    /* Variables globales con valor inicial: viven en RAM, pero su valor inicial se guarda en flash */
    .data :
    {
        _sdata = .;
        *(.data*)
        /* Alinear a 4 bytes: crt0.S copia de 4 en 4 bytes hasta _edata */
        . = ALIGN(4);
        _edata = .;
    } > RAM AT> FLASH

    /* Variables globales sin valor inicial (valen 0): viven en RAM y no ocupan flash */
    .bss (NOLOAD) :
    {
        _sbss = .;
        *(.bss*)
        /* Alinear a 4 bytes: crt0.S borra de 4 en 4 bytes hasta _ebss */
        . = ALIGN(4);
        _ebss = .;
    } > RAM
}
