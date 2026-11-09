# Guía de estudio — Snake en panel P10 RGB (NUCLEO-F446RE)

Documento para que Daniela y su compañero estudien para la sustentación. Lo mantiene Claude Code: cada paso, modificación, archivo y bloque de código se explica aquí, en el mismo commit en que entra el código. El código que aparece debe ser idéntico al del repositorio, sin "..." ni líneas omitidas.

**Estado de la plantilla:** propuesta por Claude, pendiente de aprobación de Daniela (paso 0).

## Índice
- [Glosario](#glosario)
- [Paso 0 — Repositorio y documentos](#paso-0--repositorio-y-documentos)
- [Paso 1 — Compilar y grabar (linker, Makefile, crt0.S)](#paso-1--compilar-y-grabar-linker-makefile-crt0s)
- [Paso 2 — Reloj a 84 MHz y base de tiempo SysTick](#paso-2--reloj-a-84-mhz-y-base-de-tiempo-systick)
- [Paso 3 — Botones del shield (entradas con pull-up)](#paso-3--botones-del-shield-entradas-con-pull-up)
- Los pasos siguientes se agregan aquí a medida que se hacen (ver `PLAN.md`).
- [Resumen de sustentación](#resumen-de-sustentación)

## Glosario
Cada término se define la primera vez que aparece, en una frase.

| Término | Significado |
|---|---|
| Compilador cruzado | Compilador que corre en el PC pero genera código para otra CPU (aquí `arm-none-eabi-gcc` genera código para el Cortex-M4). |
| Thumb | Conjunto de instrucciones de 16/32 bits que ejecutan los Cortex-M; no ejecutan las instrucciones ARM de 32 bits. |
| `-ffreestanding` | Opción del compilador que dice que no hay sistema operativo ni librería estándar. |
| Linker script | Archivo que le dice al enlazador dónde están la FLASH y la RAM y en qué orden se colocan las secciones del programa. |
| `.elf` | Archivo resultado del enlazado: trae el código, las direcciones y la información de depuración. |
| Regla de patrón (`%.o: %.c`) | Regla de `make` que sirve para cualquier archivo con esa extensión; `%` es el nombre del archivo. |
| `openocd` | Programa que habla con el ST-LINK de la placa para grabar la FLASH y reiniciar el microcontrolador. |
| FLASH | Memoria no volátil de 512 KB (desde `0x08000000`) donde se guarda el programa; se conserva sin energía. |
| RAM (SRAM) | Memoria volátil de 128 KB (desde `0x20000000`) para variables y pila; se pierde al apagar. |
| VMA y LMA | VMA es la dirección donde una sección vive cuando el programa corre; LMA es la dirección donde se guarda en el archivo y la FLASH. Para `.data` son distintas (RAM y FLASH). |
| `.text` | Sección con el código (las instrucciones) y la tabla de vectores. |
| `.rodata` | Sección con constantes (`const`, textos); solo lectura, en FLASH. |
| `.data` | Variables globales con valor inicial distinto de cero; viven en RAM y su valor inicial se guarda en FLASH. |
| `.bss` | Variables globales sin valor inicial (valen 0); viven en RAM y no ocupan FLASH. |
| Pila (stack) | Zona de RAM para variables locales y direcciones de retorno; crece hacia direcciones más bajas desde el final de la RAM. |
| Struct de registros | `struct` cuyos campos son los registros de un periférico, en el mismo orden y con el mismo espaciado que en la memoria; con un puntero a la dirección base se escribe `GPIOA->ODR`. |
| Desplazamiento (offset) | Distancia en bytes desde la dirección base de un periférico hasta uno de sus registros (por ejemplo `ODR` está en `+0x14`). |
| `volatile` | Palabra de C que le dice al compilador que el valor puede cambiar por fuera del programa (un pin, un periférico) y que cada lectura o escritura debe hacerse de verdad. |
| `MODER` | Registro de modo de un puerto GPIO: 2 bits por pin (00 entrada, 01 salida, 10 función alterna, 11 analógico). |
| `OSPEEDR` | Registro de velocidad de cambio de la salida de cada pin: 2 bits por pin. |
| `PUPDR` | Registro de resistencia interna de cada pin: 2 bits por pin (00 ninguna, 01 pull-up, 10 pull-down). |
| `IDR` | Registro de lectura: un bit por pin con el estado real del pin. |
| `ODR` | Registro de salida: un bit por pin con el valor que se saca. |
| `AFRL` y `AFRH` | Registros de función alterna: 4 bits por pin (pines 0 a 7 en `AFRL`, 8 a 15 en `AFRH`). |
| `RCC_AHB1ENR` | Registro del RCC que enciende el reloj de los puertos GPIO; sin reloj, el puerto no responde. |
| Máscara | Número con unos solo en los bits que se quieren tocar; con `&` y `~` se borran y con `|` se ponen. |
| Pull-up interno | Resistencia dentro del microcontrolador que lleva el pin a 1 cuando nadie lo baja. |
| Activo en bajo | Señal que significa «activada» cuando vale 0; el botón azul da 0 al presionarlo. |
| Lectura-modificación-escritura | Leer un registro, cambiar unos bits y escribirlo de vuelta; no se puede interrumpir a la mitad de forma segura (por eso en el paso 4 se explicará `BSRR`). |
| Errata | Defecto conocido del chip publicado por el fabricante, con su forma de evitarlo. |
| Símbolo del linker | Nombre al que el linker le asigna una dirección (por ejemplo `_estack`) para que `crt0.S` lo use. |
| `Reset_Handler` | Código que corre al encender o reiniciar: prepara la pila, copia `.data`, borra `.bss` y llama a `main`. |
| Tabla de vectores | Arreglo de direcciones al inicio de la FLASH: la primera es el valor inicial de SP y las siguientes son los manejadores de reset, de fallas y de cada interrupción. |
| IRQ | Petición de interrupción de un periférico; cada una tiene un número (0 a 96 en la F446) y una posición fija en la tabla de vectores. |
| Manejador (handler) | Función que la CPU ejecuta cuando llega una excepción o interrupción. |
| Símbolo débil (`.weak`) | Nombre que puede ser reemplazado: si en otro archivo se define una función con el mismo nombre, el enlazador usa esa y no la débil. |
| `Default_Handler` | Manejador por defecto: un bucle infinito al que apuntan todas las interrupciones que no tienen manejador propio. |
| SP, `r0` a `r4` | SP es el puntero de pila; `r0` a `r12` son registros de uso general de la CPU. |
| Literal pool | Zona de constantes que el ensamblador deja cerca del código para `ldr rX, =valor`. |
| Bit Thumb | Bit 0 de una dirección de función en la tabla de vectores; debe valer 1 porque el Cortex-M solo ejecuta Thumb (por eso `Reset_Handler` en `0x080001e4` aparece como `0x080001e5`). |
| HSI | Oscilador RC interno de 16 MHz con el que arranca el chip; no necesita cristal, pero su exactitud es de ~1 % a 25 °C (por verificar en el datasheet). |
| PLL | Circuito que multiplica la frecuencia de un reloj: divide la entrada por `PLLM`, la multiplica por `PLLN` y divide la salida por `PLLP`. |
| VCO | Oscilador controlado por voltaje, la parte del PLL que genera la frecuencia alta (aquí 336 MHz); debe quedar entre 100 y 432 MHz. |
| `PLLM`, `PLLN`, `PLLP` | Campos de `RCC_PLLCFGR`: divisor de entrada, multiplicador del VCO y divisor de salida (código 01 = /4). |
| Jitter | Variación pequeña del instante de los flancos de un reloj; el manual recomienda 2 MHz en la entrada del VCO para reducirlo. |
| SYSCLK y HCLK | SYSCLK es el reloj del sistema; HCLK es el reloj del AHB (CPU, memoria y GPIO). Aquí los dos valen 84 MHz. |
| AHB y APB | Buses internos: AHB es rápido (84 MHz); APB1 (42 MHz, máx. 45) y APB2 (84 MHz, máx. 90) llevan los periféricos más lentos. |
| Estados de espera (wait states) | Ciclos extra que la FLASH necesita para entregar un dato cuando el reloj es rápido; `FLASH_ACR.LATENCY`. |
| `RCC_CFGR.SW` y `SWS` | `SW` pide la fuente del reloj del sistema; `SWS` (solo lectura) indica la que el hardware usa de verdad. |
| SysTick | Contador de 24 bits del núcleo Cortex-M4 que cuenta hacia abajo; sirve como base de tiempo. |
| `LOAD` y `VAL` | `LOAD` es el valor desde el que SysTick recarga la cuenta; `VAL` es el valor actual. El periodo es `LOAD + 1` ciclos. |
| `CTRL` de SysTick | Registro con `ENABLE` (bit 0), `TICKINT` (bit 1, interrupción al llegar a 0) y `CLKSOURCE` (bit 2, 1 = reloj del procesador). |
| Excepción y NVIC | SysTick es una excepción del núcleo y no pasa por el NVIC; el NVIC (controlador de interrupciones) maneja las IRQ de los periféricos. |
| Espera bloqueante | Espera en la que la CPU no hace otra cosa; en `esperar_milisegundos` solo la interrupción sigue contando. |
| Rebote (bounce) | Cambios repetidos entre 1 y 0 que produce el contacto mecánico de un botón durante unos milisegundos al presionarlo o soltarlo; se evita con un antirrebote. |
| Entrada flotante | Entrada sin nada que la lleve a 0 o 1: lee valores al azar por el ruido; el pull-up interno evita esa situación. |
| Pines de depuración (PB3, PB4) | Pines que tras el reset están en función alterna para el depurador (JTAG/SWD); para usarlos como entrada hay que configurarlos explícitamente. |
| Desbordamiento (vuelta) de un contador | Al pasar el máximo (`0xFFFFFFFF`) un entero sin signo vuelve a 0; la resta sin signo sigue dando la diferencia correcta. |

## Plantilla de sección de paso
Cada paso nuevo copia esta estructura:

```
## Paso N — título

### Objetivo
Qué se quiere lograr y por qué.

### Archivos creados o modificados
Lista con la ruta de cada archivo.

### Concepto de clase que se usa
Qué se vio en clase y a qué ejemplo del profesor corresponde.

### Código explicado
Cada bloque de código, idéntico al del repositorio, seguido de su explicación
línea por línea: qué hace, qué registro y bits toca, por qué ese valor y de qué
sección del manual RM0390 o de qué ejemplo salió.

### Modificaciones y correcciones
Cada cambio respecto a la versión anterior o al código del profesor, con su
motivo y qué fallaría sin él.

### Decisiones tomadas
Las preguntas que hizo Claude, las opciones y lo que decidió Daniela.

### Cómo se probó
Comandos, resultado esperado y resultado real.

### Errores encontrados y cómo se resolvieron

### Preguntas de autoevaluación
3 a 5 preguntas que podría hacer el profesor, con su respuesta corta.
```

## Paso 0 — Repositorio y documentos

### Objetivo
Dejar el repositorio organizado antes de escribir código: contexto para Claude Code, plan, registro de IA y documentación de hardware.

### Archivos creados o modificados
- `CLAUDE.md`: reglas y contexto que Claude Code lee al iniciar.
- `PLAN.md`: plan paso a paso, de este paso 0 a la entrega.
- `README.md`: descripción del proyecto y cómo reproducirlo (se completa en el paso 1).
- `AI_USO.md`: registro del uso de IA, requisito 10 del curso.
- `.gitignore`: evita subir archivos generados al compilar.
- `docs/conexiones.md`: pines del panel y de los botones, y la alimentación.
- `docs/mapeo_pixeles.md`: mapeo de píxeles medido, por verificar.
- `docs/GUIA_DE_ESTUDIO.md`: este documento.
- `inc/` y `src/`: carpetas vacías para el código.

### Concepto de clase que se usa
Ninguno todavía. Este paso es solo de organización.

### Decisiones tomadas
- No usar HAL ni CMSIS: los registros se escriben a mano, como el ejemplo 4-custom-hal del profesor.
- Claude no toma decisiones: pregunta cuando algo no está claro o hay varias opciones.
- Claude explica cada línea, bloque y parte del código, y lo deja escrito en esta guía.

### Preguntas de autoevaluación
1. ¿Por qué se debe documentar el uso de IA y dónde se hace en este proyecto?
   Porque lo exige el requisito 10 del curso; se hace en `AI_USO.md`, en el mismo commit en que entra el código.
2. ¿Por qué no se pueden usar los drivers de periféricos de ST?
   Porque el requisito dice que las librerías de los periféricos del MCU las desarrollamos nosotros; solo se aceptan librerías de dispositivos externos.

## Paso 1 — Compilar y grabar (linker, Makefile, crt0.S)

Este paso se hace en cuatro piezas, una a una: Makefile, linker script, `crt0.S` y programación de la placa con el LED y el botón. Aquí están documentadas la **pieza 1 (Makefile)**, la **pieza 2 (linker script)** y la **pieza 3 (`crt0.S`)** y la **pieza 4 (registros, GPIO y LED con botón)**. Las otras piezas se agregan a esta sección cuando se hagan.

### Objetivo
Tener un Makefile propio, lo más simple posible, que compile el proyecto con `arm-none-eabi-gcc` para el Cortex-M4 y grabe la placa con `openocd`.

### Archivos creados o modificados
- `Makefile` (pieza 1).
- `linker_script.ld` (pieza 2).
- `crt0.S` (pieza 3).
- `inc/registros_f446.h`, `inc/gpio_config.h`, `src/gpio_config.c` y `main.c` (pieza 4).

### Concepto de clase que se usa
El Makefile viene del ejemplo `5-ARM-Boards/NUCLEO-64/24-ADC-1` del profesor (repo `Comp-Arch-Emb-Sys`). La grabación es la misma del ejemplo `1-led-blink`, que usamos primero como prueba del entorno (WSL2, usbipd, openocd).

### Código explicado

```makefile
APPNAME = snake

CC = arm-none-eabi-gcc

CFLAGS = -Wall -Wextra -ffreestanding
CFLAGS += -mcpu=cortex-m4 -mthumb -mfloat-abi=soft
CFLAGS += -O0 -g
CFLAGS += -Iinc

LDFLAGS = -Tlinker_script.ld -nostdlib

C_SRC = main.c $(wildcard src/*.c)
ASM_SRC = crt0.S
OBJS = $(C_SRC:.c=.o) $(ASM_SRC:.S=.o)

all: $(APPNAME).elf

$(APPNAME).elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@
	arm-none-eabi-size $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o src/*.o *.elf

.PHONY: flash
flash: $(APPNAME).elf
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $(APPNAME).elf verify reset exit"
```

**Variables**
- `APPNAME = snake`: nombre del programa; produce `snake.elf`. Es variable porque se usa en la regla principal, en `flash` y en `clean`.
- `CC = arm-none-eabi-gcc`: compilador cruzado. Corre en el PC y genera código para el Cortex-M4. También sirve de ensamblador (`.S`) y de enlazador (llama a `ld`).

**Opciones de compilación (`CFLAGS`)**
- `-Wall -Wextra`: activa casi todos los avisos. La meta es compilar sin advertencias.
- `-ffreestanding`: no hay sistema operativo ni libc; el compilador no asume que existan `printf`, `memcpy`, etc.
- `-mcpu=cortex-m4`: genera instrucciones de la CPU del STM32F446.
- `-mthumb`: usa instrucciones Thumb. Un Cortex-M solo ejecuta Thumb. El ejemplo del profesor no lo traía; el plan pide agregarlo.
- `-mfloat-abi=soft`: los `float` se harían por software, sin FPU. No usamos `float`; lo dejamos explícito.
- `-O0`: sin optimización; el código C se traduce literalmente, lo que lo hace fácil de depurar y de leer en `objdump`.
- `-g`: agrega información de depuración al `.elf` (nombres y líneas). No cambia lo que se graba en la FLASH. Sirve para `objdump -S`.
- `-Iinc`: también busca los `#include "..."` en la carpeta `inc/`.

**Opciones de enlace (`LDFLAGS`)**
- `-Tlinker_script.ld`: usa nuestro linker script (dónde están FLASH y RAM y el orden de las secciones).
- `-nostdlib`: no enlaza el arranque ni las librerías estándar del compilador; usamos nuestro `crt0.S`.

**Archivos**
- `C_SRC = main.c $(wildcard src/*.c)`: fuentes en C. `wildcard` expande a todos los `.c` de `src/`; si está vacía no suma nada.
- `ASM_SRC = crt0.S`: archivo de ensamblador.
- `OBJS = $(C_SRC:.c=.o) $(ASM_SRC:.S=.o)`: cambia la extensión de cada fuente por `.o` (`main.c` → `main.o`, `src/gpio_config.c` → `src/gpio_config.o`, `crt0.S` → `crt0.o`).

**Reglas**
- `all: $(APPNAME).elf`: primera regla, la que corre `make` sin argumentos.
- `$(APPNAME).elf: $(OBJS)`: el `.elf` depende de todos los `.o`; si alguno cambia, se vuelve a enlazar.
  - `$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@`: enlaza los `.o` en un `.elf`. `$@` es el destino (`snake.elf`).
  - `arm-none-eabi-size $@`: muestra el tamaño de `.text` (código y constantes en FLASH), `.data` (variables inicializadas) y `.bss` (variables en cero).
- `%.o: %.S` con `$(CC) $(CFLAGS) -c $< -o $@`: regla general para ensamblar un `.S` en `.o`. `-c` es "compilar sin enlazar"; `$<` es la primera dependencia (el `.S`).
- `%.o: %.c`: lo mismo para `.c`. Como `%` incluye la ruta, también sirve para `src/loquesea.c`.
- `clean`: `rm -f *.o src/*.o *.elf` borra los objetos y el `.elf`. `.PHONY` avisa que `clean` es una acción y no un archivo.
- `flash`: depende del `.elf` (compila antes de grabar). `openocd` usa `interface/stlink.cfg` (el ST-LINK de la placa) y `target/stm32f4x.cfg` (familia STM32F4). El comando `program snake.elf verify reset exit` graba, verifica, reinicia y sale.

**Recordatorio:** el Makefile no mira los `.h`. Si se cambia un `.h`, hay que hacer `make clean` antes de `make`.

### Modificaciones y correcciones (respecto al Makefile del profesor)
- Se quitaron `MACHINE`, `QEMU`, `LINK`, `OBJCOPY`, `OBJDUMP` y los targets `run` y `debug`: son para QEMU/GDB o no los usa ninguna regla. Trabajamos con la placa real.
- Se quitó `-std=c11`: no es necesario (decisión de Daniela).
- Se quitaron `-fno-pic` y `-Wl,--no-relax`: se probó compilar sin ellos y compila y enlaza igual.
- Se quitaron `--specs=nosys.specs`, `--specs=nano.specs`, `-lc` y `-lgcc`: no usamos la librería de C. Por eso tampoco se copia `stub_stdlib.c`.
- `-Og` pasó a `-O0`.
- Se agregaron `-mthumb`, `-mfloat-abi=soft` e `-Iinc`.
- `wildcard *.c` pasó a `main.c` más `src/*.c`.

**Pendiente para la pieza 3:** el `crt0.S` del profesor llama a `__libc_init_array` (de libc). Al quitar libc, el enlace falla con `undefined reference to __libc_init_array`. Se resuelve en el `crt0.S`.

### Decisiones tomadas
| Pregunta | Opciones | Decisión de Daniela |
|---|---|---|
| Carpeta de los `.o` y `.elf` | `build/` o junto al código | Junto al código (más simple) |
| Librería estándar | Quitarla (y `stub_stdlib.c`) o dejarla | Quitarla; si algo falta se agrega y se explica |
| Nombre del programa | `main` o `snake` | `snake` |
| `-g`, `-std=c11`, `--no-relax` | Dejar o quitar | Dejar `-g`; quitar los otros dos |
| Dependencias de `.h` | Con `-MMD` o simple | Simple; hacer `make clean` al cambiar un `.h` |
| Optimización | `-O0` u optimizada | `-O0` |
| FPU | Sin FPU o con FPU | Sin FPU (`-mfloat-abi=soft`) |
| `OBJCOPY` y `OBJDUMP` | Dejar o quitar | Quitar |
| Cómo probar la pieza 1 | Solo el Makefile en el repo, o con archivos temporales | Solo el Makefile en el repo |

### Cómo se probó
- **Prueba de entorno:** se copió `1-led-blink` del profesor a una carpeta fuera del repo, `make` compiló y `make flash` terminó con `Verified OK`; el LED parpadeó.
- **Prueba del Makefile:** en una carpeta fuera del repo, con un `main.c` vacío (`while(1)`) y el `crt0.S` y linker del profesor (con `__libc_init_array` reemplazado por un `NOP`), `make` terminó sin advertencias. Resultado de `arm-none-eabi-size`: `text 140`, `data 0`, `bss 98304`. Las pruebas con y sin `-fno-pic` y `--no-relax` dieron lo mismo.
- En el repo todavía no hay `main.c`, `crt0.S` ni linker, así que `make` se probará completo en las piezas 2 y 3.

### Errores encontrados y cómo se resolvieron
- El `crt0.S` del profesor no enlaza sin libc (`undefined reference to __libc_init_array`). Queda como pendiente de la pieza 3.

### Preguntas de autoevaluación
1. ¿Por qué `CFLAGS` lleva `-mthumb`?
   Porque los Cortex-M solo ejecutan instrucciones Thumb; sin esa opción el compilador podría generar instrucciones ARM de 32 bits que la CPU no ejecuta.
2. ¿Qué hace `-nostdlib` y por qué lo usamos?
   No enlaza el arranque ni las librerías estándar del compilador. Lo usamos porque el arranque es nuestro `crt0.S` y no usamos libc.
3. ¿Qué significan `$@` y `$<` en una regla?
   `$@` es el destino de la regla y `$<` es su primera dependencia.
4. ¿Qué hace la regla `flash` y por qué depende del `.elf`?
   Graba la FLASH con `openocd` y el ST-LINK, verifica y reinicia. Depende del `.elf` para compilar antes de grabar.
5. ¿Qué pasa si cambio un `.h` y hago `make`?
   No se recompilan los `.c` que lo usan, porque el Makefile no mira los `.h`. Hay que hacer `make clean`.

### Pieza 2 — Linker script

**Objetivo de la pieza:** decirle al enlazador dónde están la FLASH y la RAM, en qué orden se colocan las secciones del programa y qué símbolos necesita `crt0.S` (inicio y fin de `.data`, de `.bss` y tope de la pila).

**Origen:** `linker_script.ld` del ejemplo `24-ADC-1` del profesor. El tamaño de la RAM se verificó en el RM0390, sección 2.3 «Boot configuration», tabla 3 «Memory mapping vs. Boot mode/physical remap in STM32F446xx»: SRAM1 de 112 KB en `0x2000 0000 – 0x2001 BFFF` y SRAM2 de 16 KB en `0x2001 C000 – 0x2001 FFFF`, contiguas (128 KB en total, sección 2.2.3). Se consultó una transcripción en línea del manual (docs.dirba.io), no el PDF original.

#### Código explicado

```ld
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
```

**`ENTRY(Reset_Handler)`**: primera instrucción del programa. Es el nombre del manejador de reset que se define en `crt0.S`. El procesador en realidad arranca leyendo la tabla de vectores; `ENTRY` le sirve al enlazador y al depurador (y evita que quite ese código).

**`MEMORY { ... }`**: las dos memorias del chip.
- `FLASH (rx)`: `r` lectura y `x` ejecución (no `w`: no se escribe mientras corre). Empieza en `0x08000000` y mide 512 KB.
- `RAM (rwx)`: lectura, escritura y ejecución. Empieza en `0x20000000` y mide 128 KB (RM0390, tabla 3). Termina en `0x20020000`.

**`_estack = ORIGIN(RAM) + LENGTH(RAM);`**: define el símbolo `_estack` con la dirección justo después del último byte de la RAM, `0x20020000`. `crt0.S` lo usa para cargar el puntero de pila y como primer valor de la tabla de vectores.
- **La pila crece hacia abajo.** Cada `push` o llamada a función *resta* a SP y escribe en direcciones más bajas. Por eso empieza en el final de la RAM: el primer dato queda en `0x2001FFFC`, el siguiente en `0x2001FFF8`, etc.
- **Dónde está el riesgo.** `.data` y `.bss` crecen hacia arriba desde `0x20000000`; la pila crece hacia abajo desde `0x20020000`. El espacio libre es el hueco entre `_ebss` y SP. Si el programa usa demasiada pila (recursión profunda, arreglos locales grandes), SP baja hasta pisar `.bss` o `.data`, y las variables globales se corrompen en silencio. **No hay aviso**: ni el enlazador (no reservamos tamaño de pila) ni el procesador (no hay protección de memoria activada) lo detectan. Con `arm-none-eabi-size` se ve cuánto ocupan `.data` y `.bss`; el resto de la RAM es lo que queda para la pila.

**`SECTIONS { ... }`**: cómo se ordena el contenido en cada memoria.

- **`.text : { *(.isr_vector) *(.text*) } > FLASH`**: un solo bloque en FLASH. Primero la tabla de vectores (sección `.isr_vector` que define `crt0.S`) y después todo el código (`.text*`). Como es lo primero, la tabla queda en `0x08000000`, que es donde el procesador la busca al arrancar. `*` significa «de todos los archivos objeto».
- **`.rodata : { *(.rodata*) . = ALIGN(4); } > FLASH`**: constantes de solo lectura, a continuación del código. `. = ALIGN(4);` mueve el contador de posición (`.`) hasta el siguiente múltiplo de 4, para que lo que sigue en FLASH (el valor inicial de `.data`) empiece alineado.
- **`_sidata = LOADADDR(.data);`**: guarda en `_sidata` la dirección en FLASH (LMA) donde se almacenó el valor inicial de `.data`. `crt0.S` copia desde ahí.
- **`.data : { _sdata = .; *(.data*) . = ALIGN(4); _edata = .; } > RAM AT> FLASH`**: variables globales con valor inicial.
  - `> RAM`: durante la ejecución viven en RAM (VMA).
  - `AT> FLASH`: el valor inicial se guarda en FLASH (LMA), porque la RAM se pierde al apagar. En el arranque `crt0.S` lo copia de FLASH a RAM.
  - `_sdata` y `_edata` marcan el inicio y fin en RAM; el `ALIGN(4)` antes de `_edata` se explica abajo.
- **`.bss (NOLOAD) : { _sbss = .; *(.bss*) . = ALIGN(4); _ebss = .; } > RAM`**: variables globales sin valor inicial. `NOLOAD` quiere decir que no ocupan espacio en FLASH ni se graban: solo se reserva RAM y `crt0.S` la rellena de ceros entre `_sbss` y `_ebss`, porque C garantiza que valen 0.

**Por qué `ALIGN(4)` antes de `_edata` y `_ebss` (ejemplo).** `crt0.S` copia y borra de 4 en 4 bytes con un bucle «mientras R0 < R1». Supón que `.data` tiene solo un arreglo de 5 bytes (`char texto[5]`). Sin `ALIGN`, `_sdata = 0x20000000` y `_edata = 0x20000005`. El bucle copia una palabra a `0x20000000` (R0 pasa a `0x20000004`); como `4 < 5` copia otra palabra a `0x20000004`, que escribe hasta `0x20000007`: **3 bytes de más**, que ya no son de `.data`. Con `ALIGN(4)`, `_edata = 0x20000008`, y el bucle copia exactamente 2 palabras. Con `.bss` pasa igual: sin alineación el bucle de borrado escribe ceros pasado `_ebss`, sobre memoria que no es suya. Al alinear, el bucle de palabras siempre termina justo en el límite.

#### Modificaciones y correcciones (respecto al linker del profesor)
- `RAM` de `96K` a `128K` (el chip tiene 128 KB; RM0390, tabla 3).
- Se quitó la sección `.stack` con el truco `. = ORIGIN(RAM) + LENGTH(RAM)`: con él `.stack` aparecía en `objdump -h` con tamaño `0x18000` y VMA `0x20000000`, o sea, superpuesta a `.data` y `.bss`. Ahora solo existe el símbolo `_estack`.
- Se quitó `stack_top` (nadie lo usaba; solo `_estack`), `_etext` y los alias `__bss_start__` y `__bss_end__` (nadie los usa en `crt0.S`).
- Se quitaron de `.rodata`: `.srodata` (es de RISC-V), `KEEP(*(.init))`, `KEEP(*(.fini))`, `*(.eh_frame)` y `*(.ARM.exidx)` (libc y C++). De `.data`: `.sdata` (RISC-V) y `KEEP(*(.init_array))` y `KEEP(*(.fini_array))` (constructores, que C no usa). Se probó: enlaza sin errores y no aparece ninguna sección huérfana (no hay `.init`, `.fini`, `.eh_frame` ni `.ARM.exidx` en `objdump -h`).
- Se agregaron `. = ALIGN(4);` antes de `_edata` y `_ebss` (el ejemplo solo alineaba `.rodata`).
- Se agregaron comentarios en español.

#### Decisiones tomadas
| Pregunta | Opciones | Decisión de Daniela |
|---|---|---|
| Tamaño de la RAM | 96K (profesor) o 128K | 128K, verificado en el RM0390 |
| Pila | Truco del profesor; solo el símbolo `_estack`; sección de tamaño fijo | Solo el símbolo `_estack` (sin protección de tamaño) |
| Tabla de vectores | Dentro de `.text` o sección propia | Dentro de `.text`, con los vectores primero |
| Limpieza de libc, C++ y RISC-V | Quitar o dejar | Quitar y avisar si algo falla (no falló) |
| `ALIGN(4)` | Agregarlo o no | Agregarlo antes de `_edata` y `_ebss` |
| Nombres de símbolos | Los del profesor | Los del profesor; solo se conservan los que usa `crt0.S` |

#### Cómo se probó
Fuera del repo, con un `main.c` de prueba (`volatile unsigned int variable_en_data = 0x11223344;`, `volatile unsigned int variable_en_bss;`, `const unsigned int constante_en_rodata = 0xAABBCCDD;`), el `Makefile` del repo, este linker y el `crt0.S` del ejemplo `24-ADC-1` con `bl SystemInit` y `bl __libc_init_array` reemplazados por `nop` (solo en la copia de prueba; el `crt0.S` real es la pieza 3).
- `arm-none-eabi-size`: `text 538`, `data 4`, `bss 4`. Sin advertencias.
- `arm-none-eabi-objdump -h`:

| Sección | VMA | LMA | Tamaño |
|---|---|---|---|
| `.text` | `0x08000000` | `0x08000000` | `0x216` |
| `.rodata` | `0x08000218` | `0x08000218` | `0x4` |
| `.data` | `0x20000000` | `0x0800021c` | `0x4` |
| `.bss` | `0x20000004` | `0x08000220` | `0x4` |

- `.data` tiene VMA en RAM y LMA en FLASH, justo después de `.rodata`. `_sidata = 0x0800021c`. `.bss` no tiene `CONTENTS`: no ocupa FLASH.
- `arm-none-eabi-nm -n`: `g_pfnVectors` en `0x08000000`, `constante_en_rodata` en `0x08000218`, `variable_en_data` en `0x20000000`, `variable_en_bss` en `0x20000004`, `_sdata = 0x20000000`, `_edata = _sbss = 0x20000004`, `_ebss = 0x20000008` y `_estack = 0x20020000`.
- `arm-none-eabi-objdump -p`: dos segmentos `LOAD`: FLASH (`0x08000000`, `0x21c` bytes) y RAM (`vaddr 0x20000000`, `paddr 0x0800021c`, `filesz 4`, `memsz 8`; el `memsz` incluye `.bss`).
- Primeras palabras en `0x08000000`: `0x20020000` (valor inicial de SP, igual a `_estack`) y `0x080001c9` (`Reset_Handler` en `0x080001c8` con el bit 0 en 1, por Thumb).

#### Errores encontrados y cómo se resolvieron
- Al principio el enlace falló con `undefined reference to stack_top`: el `crt0.S` de `1-led-blink` que había en la carpeta de prueba usa `stack_top`, y el de `24-ADC-1` usa `_estack`. Se cambió la copia de prueba por el `crt0.S` de `24-ADC-1`, que es el que se adapta en la pieza 3.

#### Preguntas de autoevaluación
1. ¿Por qué `.data` tiene VMA en RAM y LMA en FLASH?
   Porque las variables deben poder modificarse (RAM), pero la RAM se pierde al apagar, así que su valor inicial se guarda en FLASH y `crt0.S` lo copia a RAM al arrancar.
2. ¿Por qué `.bss` no ocupa espacio en FLASH?
   Porque su valor inicial es siempre 0; basta reservar la RAM y que `crt0.S` la rellene de ceros.
3. ¿Hacia dónde crece la pila y qué riesgo hay si no se reserva su tamaño?
   Hacia direcciones más bajas desde `_estack = 0x20020000`. Si baja hasta `_ebss` pisa las variables globales, y no hay ningún aviso.
4. ¿Qué pasaría sin `ALIGN(4)` antes de `_edata`?
   Si `.data` midiera 5 bytes, el bucle de `crt0.S` (de 4 en 4) copiaría 2 palabras y escribiría 3 bytes más allá de `.data`.
5. ¿Cómo se sabe que la tabla de vectores quedó donde la espera el procesador?
   `.isr_vector` es lo primero dentro de `.text`, y `.text` es lo primero en FLASH (`0x08000000`); se confirma con `nm` (`g_pfnVectors = 0x08000000`) y con `objdump -s`.

### Pieza 3 — `crt0.S` (arranque y tabla de vectores)

**Objetivo de la pieza:** el código en ensamblador que corre primero. Define la tabla de vectores completa de la F446 y el `Reset_Handler`, que deja el programa listo antes de llamar a `main`.

**Origen:** `startup_stm32f401xe.s` de ST, tal como lo trae el ejemplo `24-ADC-1` del profesor. La tabla de vectores se rehízo para la F446 con la **Table 38 «Vector table for STM32F446xx»** del RM0390 (capítulo 10 «Interrupts and events», sección 10.1.3), consultada en una transcripción en línea (docs.dirba.io), no en el PDF. Antes de sustentar conviene contrastarla con el PDF de ST: la página advierte que puede tener errores, y su sección 10.1.1 dice «96 maskable interrupt channels» mientras la tabla va de la IRQ 0 a la 96 (97 posiciones).

#### Código explicado

```asm
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
```

**Cabecera y directivas**
- Primera línea: origen del archivo (decisión de Daniela). Después, la cabecera de copyright de ST completa, sin cambios.
- `.syntax unified`: sintaxis unificada de ARM (la misma para ARM y Thumb).
- `.cpu cortex-m4`: ensambla para el Cortex-M4.
- `.fpu softvfp`: sin FPU (coincide con `-mfloat-abi=soft` del Makefile).
- `.thumb`: el código es Thumb.
- `.global g_pfnVectors`, `.global Default_Handler` y `.global Reset_Handler`: hacen visibles esos nombres al enlazador. `Reset_Handler` es global porque el linker script lo usa en `ENTRY(Reset_Handler)` (el ejemplo lo declaraba `.weak`; decisión de Daniela: `.global`).

**`Reset_Handler`**
- `.section .text.Reset_Handler` y `.type Reset_Handler, %function`: la función va en su propia sección dentro de `.text` y se marca como función.
- `ldr sp, =_estack`: carga SP con `_estack` (`0x20020000`, final de la RAM). Es lo primero porque las llamadas a funciones usan la pila. La CPU ya lo había cargado desde la palabra 0 de la tabla; aquí se repite para dejarlo explícito.
- **Copia de `.data`** (FLASH a RAM):
  - `ldr r0, =_sdata`: r0 = inicio de `.data` en RAM (símbolo del linker).
  - `ldr r1, =_edata`: r1 = fin de `.data` en RAM.
  - `ldr r2, =_sidata`: r2 = dónde está el valor inicial en FLASH.
  - `movs r3, #0`: r3 = desplazamiento en bytes, empieza en 0.
  - `b LoopCopyDataInit`: salta a la condición, para no copiar nada si `.data` está vacía.
  - `CopyDataInit`: `ldr r4, [r2, r3]` lee una palabra de FLASH (dirección `r2 + r3`); `str r4, [r0, r3]` la escribe en RAM (`r0 + r3`); `adds r3, r3, #4` avanza 4 bytes.
  - `LoopCopyDataInit`: `adds r4, r0, r3` calcula la dirección actual de RAM; `cmp r4, r1` la compara con `_edata`; `bcc CopyDataInit` (salta si es menor, sin signo) repite mientras no se llegue al final. Por eso hace falta `ALIGN(4)` en el linker: se copia de 4 en 4.
- **Borrado de `.bss`**:
  - `ldr r2, =_sbss`, `ldr r4, =_ebss`, `movs r3, #0`: inicio, fin y valor cero.
  - `b LoopFillZerobss`: salta a la condición.
  - `FillZerobss`: `str r3, [r2]` escribe un cero (4 bytes); `adds r2, r2, #4` avanza.
  - `LoopFillZerobss`: `cmp r2, r4` y `bcc FillZerobss` repiten hasta `_ebss`. C garantiza que las globales sin valor inicial valen 0; aquí se cumple esa garantía.
- `bl main`: llama a `main` (`bl` guarda la dirección de retorno en `lr`).
- `b .`: si `main` retorna, la CPU se queda en un bucle infinito (`.` es la dirección de la propia instrucción). El ejemplo hacía `bx lr`, que saltaría a una dirección sin sentido.
- `.size Reset_Handler, .-Reset_Handler`: le dice al enlazador y al depurador cuántos bytes mide la función.

**`Default_Handler`**
- Sección propia `.text.Default_Handler`. La etiqueta `Infinite_Loop` con `b Infinite_Loop` es un bucle infinito: si llega una interrupción que nadie atiende, el programa se queda aquí y se puede examinar con el depurador.

**Tabla de vectores (`g_pfnVectors`)**
- `.section .isr_vector,"a",%progbits`: sección `.isr_vector`, de solo lectura (`a` = alloc) y con datos. El linker script la coloca primero en FLASH, en `0x08000000`.
- Cada `.word` ocupa 4 bytes y es la dirección de un manejador; el procesador salta a esa dirección cuando ocurre el evento. `.word 0` marca una posición reservada.
- **Primeras 16 palabras (excepciones del sistema, `0x00` a `0x3C`):**
  - `0x00` `_estack`: valor inicial de SP. No es un manejador; la CPU lo carga en SP al arrancar.
  - `0x04` `Reset_Handler`: la CPU salta aquí al arrancar.
  - `0x08` a `0x18`: NMI, HardFault, MemManage, BusFault y UsageFault.
  - `0x1C` a `0x28`: cuatro palabras reservadas.
  - `0x2C` SVC, `0x30` DebugMon, `0x34` reservada, `0x38` PendSV y `0x3C` SysTick.
- **Interrupciones externas (IRQ 0 a 96):** la IRQ número `n` está en la dirección `0x40 + 4 × n`; por ejemplo, TIM3 es la IRQ 29 y queda en `0xB4`. Cada línea trae un comentario con su número y nombre según la Table 38. Hay 86 con nombre y 11 reservadas (IRQ 61, 62, 79, 80, 82, 83, 85, 86, 88, 89 y 90, que valen 0).
- `.size g_pfnVectors, .-g_pfnVectors`: tamaño de la tabla (113 palabras, `0x1C4` bytes).

**Alias débiles**
- Para cada uno de los 95 manejadores (9 del sistema y 86 IRQ): `.weak NOMBRE` lo declara débil y `.thumb_set NOMBRE,Default_Handler` hace que apunte a `Default_Handler` (con el bit Thumb en 1).
- Si en C se define una función con el mismo nombre (por ejemplo `void TIM3_IRQHandler(void)`), el enlazador usa la tuya y descarta el alias. Si no se define, la IRQ cae en el bucle infinito de `Default_Handler`.

#### Modificaciones y correcciones (respecto al `crt0.S` del profesor)
- **Tabla de vectores corregida para la F446.** El ejemplo es del F401, y varios vectores que allí son «Reserved» en la F446 son IRQ reales (USART3, TIM8 y TIM12 a 14, FMC, UART4 y UART5, TIM6_DAC, TIM7, CAN1 y CAN2, OTG_HS, DCMI, SAI1 y SAI2, QuadSPI, HDMI-CEC, SPDIF-Rx y FMPI2C1). Sin la corrección, todas las IRQ posteriores a la 38 (USART2) quedarían desplazadas, y una IRQ podría ir a un manejador equivocado.
- Se quitó `bl SystemInit` (viene de CMSIS, ya decidido en `CLAUDE.md`).
- Se quitó `bl __libc_init_array` (viene de libc; con `-nostdlib` el enlace fallaba con `undefined reference`).
- Después de `main`, el `bx lr` se cambió por `b .`.
- Se quitaron las 5 líneas `.word _sidata`, `_sdata`, `_edata`, `_sbss` y `_ebss` sueltas del inicio (no las usa nadie).
- `Reset_Handler` pasó de `.weak` a `.global`.
- Se agregó la línea de origen y comentarios en español en cada vector y en cada bloque de `Reset_Handler`.
- Los nombres de los manejadores son los de CMSIS: `SVC_Handler`, `DebugMon_Handler`, `QUADSPI_IRQHandler`, `CEC_IRQHandler`, `SPDIFRX_IRQHandler`, `FMPI2C1_EV_IRQHandler` y `FMPI2C1_ER_IRQHandler` (decisión de Daniela).
- El archivo se generó con un script a partir de la Table 38 para evitar errores de transcripción; el script no forma parte del repo.

#### Decisiones tomadas
| Pregunta | Opciones | Decisión de Daniela |
|---|---|---|
| Si `main` retorna | Bucle infinito, `bx lr` o `Default_Handler` | Bucle infinito (`b .`) |
| 5 líneas `.word` sueltas | Quitarlas o dejarlas | Quitarlas |
| Cabecera de ST | Solo una línea de origen o la cabecera completa | Cabecera completa, con una línea de origen arriba |
| Nombres de manejadores | Los de CMSIS u otros | Los de CMSIS, como se propusieron |
| Manejadores por defecto | Pareja `.weak` y `.thumb_set` por cada uno, o `PROVIDE` en el linker | Pareja `.weak` y `.thumb_set` (como el profesor) |
| `Reset_Handler` | `.weak` o `.global` | `.global` |
| Comentarios | Uno por vector y por bloque | Sí |

#### Cómo se probó
Fuera del repo, con el `Makefile`, el `linker_script.ld` y el `crt0.S` del repo, y un `main.c` de prueba:
- `make` termina sin errores ni advertencias, sin parches. El enlace ya no falla (no hay `SystemInit` ni `__libc_init_array`).
- `arm-none-eabi-size`: `text 562`, `data 4`, `bss 4`.
- `nm -n`: `g_pfnVectors = 0x08000000`, `Reset_Handler = 0x080001e4`, `Default_Handler = 0x0800022c`, `_estack = 0x20020000`.
- `objdump -s` a partir de `0x08000000`: la primera palabra es `0x20020000` y la segunda `0x080001e5` (`Reset_Handler` con el bit Thumb).
- Con un `main.c` que define una función distinta para cada uno de los 95 manejadores, se comprobó que las 113 palabras de la tabla apuntan al manejador que dice su línea (0 errores) y que las 16 reservadas (5 del sistema y 11 IRQ) valen 0. Ejemplos: `SysTick_Handler` en `0x03C`, `TIM3_IRQHandler` en `0x0B4`, `USART2_IRQHandler` en `0x0D8` y `USART3_IRQHandler` en `0x0DC`, igual que en la Table 38.
- Lo que falta: comprobar en la placa que el arranque copia `.data` y borra `.bss` bien. Se hace en la pieza 4 con `make flash`.

#### Errores encontrados y cómo se resolvieron
- Con el `crt0.S` del profesor sin cambios, el enlace falla por `__libc_init_array`. Se quitó la llamada (ver arriba).
- En la prueba, ver los símbolos de cada vector con `nm` no distinguía un manejador de otro, porque todos los alias débiles apuntan a la misma dirección (`Default_Handler`). Se resolvió definiendo una función distinta por manejador solo para la prueba.

#### Preguntas de autoevaluación
1. ¿Qué hace el procesador al encender, antes de ejecutar tu código?
   Lee la palabra 0 de la tabla de vectores y la carga en SP, lee la palabra 1 (`Reset_Handler`) y salta allí.
2. ¿Por qué `Reset_Handler` copia `.data` y borra `.bss` antes de llamar a `main`?
   Porque las variables globales con valor inicial viven en RAM, pero su valor está guardado en FLASH (hay que copiarlo), y C garantiza que las globales sin valor inicial valen 0 (la RAM arranca con basura).
3. ¿Qué pasa si llega una IRQ para la que no definiste manejador?
   Su vector apunta, por el alias débil, a `Default_Handler`, que es un bucle infinito.
4. ¿Cómo se reemplaza un alias débil, por ejemplo para TIM3?
   Definiendo en C una función con el mismo nombre exacto, `void TIM3_IRQHandler(void)`; el enlazador usa esa y no el alias.
5. ¿Por qué había que rehacer la tabla de vectores del profesor?
   Porque era la del F401 y en la F446 varias posiciones «reservadas» son IRQ reales; sin corregirla, las IRQ posteriores quedarían desplazadas.
6. ¿Qué dirección ocupa la IRQ número `n` en la tabla?
   `0x40 + 4 × n`; por ejemplo, TIM3 (IRQ 29) está en `0xB4`.

### Pieza 4 — Registros, `gpio_config` y LED con botón

**Objetivo de la pieza:** definir a mano los registros que se necesitan (RCC, GPIOA y GPIOC), escribir las funciones de `gpio_config` (adaptadas del profesor) y un `main.c` que enciende el LED PA5 mientras se mantiene presionado el botón azul PC13. Con esto se prueba todo lo anterior en la placa con `make flash`.

**Cómo se hizo:** el agente principal resolvió con Daniela las decisiones; el subagente `programador` (Haiku) escribió los cuatro archivos; el subagente `revisor` (Sonnet) los revisó y no cambió nada; el agente principal agregó `(void)lectura_descartada;` por decisión de Daniela, recompiló y grabó la placa.

**Nota (política de la guía, opción a):** `inc/registros_f446.h` y `main.c` que aparecen abajo son las **versiones del Paso 1**. El Paso 2 los cambia: ver «Paso 2». `registros_f446.h` conserva todo lo de abajo y agrega registros; en `main.c` se quita el botón. `gpio_config.c` y `gpio_config.h` también cambian en el Paso 3 (reloj del puerto B); lo demás de la pieza no cambia.

**Origen:** `gpio_config.c/.h` del ejemplo `24-ADC-1` y `stm32f401.h` del ejemplo `4-custom-hal` del profesor (struct de registros), `main.c` del ejemplo `3-input-butt` (con la lógica del botón invertida, ver abajo). Datos del RM0390: direcciones base, sección 2.2.2, tabla 1 (leída literal en una transcripción en línea, docs.dirba.io); desplazamientos de GPIO, sección 7.4, tabla 26, y bits de `RCC_AHB1ENR`, sección 6.3.10 (solo de un resumen de la transcripción, coinciden con los ejemplos del profesor). **Antes de sustentar conviene comprobar estas tablas en el PDF de ST.**

#### Código explicado

`inc/registros_f446.h`

```c
#ifndef REGISTROS_F446_H
#define REGISTROS_F446_H

#include <stdint.h>

// Direcciones base de los buses. Fuente: RM0390 (manual de referencia STM32F446),
// sección 2.2.2, tabla 1 «STM32F446xx register boundary addresses».
#define PERIFERICOS_BASE (0x40000000U)
// Bus AHB1: aquí están los puertos GPIO y el RCC.
#define AHB1_BASE (PERIFERICOS_BASE + 0x20000U)

// Direcciones base de cada periférico. Fuente: RM0390, sección 2.2.2, tabla 1.
// Puerto A de GPIO (0x40020000).
#define GPIOA_BASE (AHB1_BASE)
// Puerto C de GPIO (0x40020800).
#define GPIOC_BASE (AHB1_BASE + 0x800U)
// Controlador de reloj RCC (0x40023800).
#define RCC_BASE (AHB1_BASE + 0x3800U)

// Mapa de los registros del puerto: cada campo está 4 bytes después del anterior,
// por eso el orden no se puede cambiar. Fuente: RM0390, capítulo 7, sección 7.4, tabla 26.
typedef struct {
    // Desplazamiento 0x00. Modo de cada pin: 2 bits por pin (00 entrada, 01 salida, 10 alterna, 11 analógico).
    volatile uint32_t MODER;
    // Desplazamiento 0x04. Tipo de salida de cada pin: 1 bit por pin (0 push-pull, 1 open-drain).
    volatile uint32_t OTYPER;
    // Desplazamiento 0x08. Velocidad de cambio de cada pin: 2 bits por pin.
    volatile uint32_t OSPEEDR;
    // Desplazamiento 0x0C. Resistencia pull-up o pull-down de cada pin: 2 bits por pin.
    volatile uint32_t PUPDR;
    // Desplazamiento 0x10. Lectura del estado de los pines de entrada: 1 bit por pin.
    volatile uint32_t IDR;
    // Desplazamiento 0x14. Valor que se saca por cada pin de salida: 1 bit por pin.
    volatile uint32_t ODR;
    // Desplazamiento 0x18. Pone (bits 0 a 15) o borra (bits 16 a 31) bits de ODR de forma atómica.
    volatile uint32_t BSRR;
    // Desplazamiento 0x1C. Bloquea la configuración de los pines hasta el próximo reinicio.
    volatile uint32_t LCKR;
    // Desplazamiento 0x20. Función alterna de los pines 0 a 7: 4 bits por pin.
    volatile uint32_t AFRL;
    // Desplazamiento 0x24. Función alterna de los pines 8 a 15: 4 bits por pin.
    volatile uint32_t AFRH;
} RegistrosGPIO;

// Mapa de los registros del RCC (reloj). Cada campo está en su desplazamiento del manual.
// Fuente: RM0390, capítulo 6, sección 6.3, «RCC registers», mapa de registros.
typedef struct {
    // Desplazamiento 0x00. Control del reloj (HSI, HSE, PLL). No se usa todavía (paso 2).
    volatile uint32_t CR;
    // Desplazamiento 0x04. Configuración del PLL. No se usa todavía (paso 2).
    volatile uint32_t PLLCFGR;
    // Desplazamiento 0x08. Configuración del reloj del sistema y de los buses. No se usa todavía (paso 2).
    volatile uint32_t CFGR;
    // Desplazamiento 0x0C. Interrupciones del reloj. No se usa todavía (paso 2).
    volatile uint32_t CIR;
    // Desplazamiento 0x10. Reinicia los puertos AHB1. No se usa todavía (paso 2).
    volatile uint32_t AHB1RSTR;
    // Desplazamiento 0x14. Reinicia los periféricos AHB2. No se usa todavía (paso 2).
    volatile uint32_t AHB2RSTR;
    // Desplazamiento 0x18. Reinicia los periféricos AHB3. No se usa todavía (paso 2).
    volatile uint32_t AHB3RSTR;
    // Desplazamiento 0x1C. Reservado por el fabricante, no se usa.
    uint32_t RESERVADO_0;
    // Desplazamiento 0x20. Reinicia los periféricos del bus APB1. No se usa todavía (paso 2).
    volatile uint32_t APB1RSTR;
    // Desplazamiento 0x24. Reinicia los periféricos del bus APB2. No se usa todavía (paso 2).
    volatile uint32_t APB2RSTR;
    // Desplazamientos 0x28 y 0x2C. Reservados por el fabricante, no se usan.
    uint32_t RESERVADO_1[2];
    // Desplazamiento 0x30. Habilita el reloj de cada puerto GPIO (bit 0 = puerto A, bit 2 = puerto C).
    volatile uint32_t AHB1ENR;
} RegistrosRCC;

// Punteros a los periféricos: se usan como GPIOA->ODR, RCC->AHB1ENR, etc.
// Cada puntero apunta a la dirección base del periférico.
#define GPIOA ((RegistrosGPIO *)GPIOA_BASE)
// Puntero al puerto C de GPIO (botón azul PC13).
#define GPIOC ((RegistrosGPIO *)GPIOC_BASE)
// Puntero al controlador de reloj RCC.
#define RCC ((RegistrosRCC *)RCC_BASE)

// Bit 0 de RCC_AHB1ENR: 1 enciende el reloj del puerto A. Fuente: RM0390, sección 6.3.10.
#define RCC_AHB1ENR_GPIOAEN (1U << 0)
// Bit 2 de RCC_AHB1ENR: 1 enciende el reloj del puerto C. Fuente: RM0390, sección 6.3.10.
#define RCC_AHB1ENR_GPIOCEN (1U << 2)

#endif
```

- `#include <stdint.h>`: trae `uint32_t`, un entero sin signo de 32 bits, el tamaño de cada registro.
- `PERIFERICOS_BASE` y `AHB1_BASE`: los periféricos empiezan en `0x40000000`; el bus AHB1 (donde están los puertos GPIO y el RCC) en `+0x20000`.
- `GPIOA_BASE`, `GPIOC_BASE` y `RCC_BASE`: `0x40020000`, `0x40020800` y `0x40023800`.
- **`RegistrosGPIO` es el mapa de los registros del puerto: cada campo está 4 bytes después del anterior**, por eso el orden no se puede cambiar. Si el puntero apunta a `0x40020000`, `puerto->ODR` es la dirección `0x40020014`: el compilador suma el desplazamiento del campo. Cada campo es `volatile`.
- `RegistrosRCC`: igual, hasta `AHB1ENR` (`0x30`). `CR`, `PLLCFGR`, `CFGR`, `CIR`, `AHB1RSTR`, `AHB2RSTR`, `AHB3RSTR`, `APB1RSTR` y `APB2RSTR` no se usan todavía (paso 2); están para que `AHB1ENR` quede en su lugar. `RESERVADO_0` y `RESERVADO_1[2]` son los huecos del manual (`0x1C` y `0x28` a `0x2C`).
- `GPIOA`, `GPIOC` y `RCC`: convierten la dirección base en un puntero al struct.
- `RCC_AHB1ENR_GPIOAEN` (bit 0) y `RCC_AHB1ENR_GPIOCEN` (bit 2): máscaras para encender el reloj de cada puerto.

`inc/gpio_config.h`

```c
#ifndef GPIO_CONFIG_H
#define GPIO_CONFIG_H

#include "registros_f446.h"

// Valores para los campos de configuración de un pin.
// Fuente: RM0390, sección 7.4, descripción de los registros GPIOx_MODER, GPIOx_PUPDR y GPIOx_OSPEEDR.
// Modo entrada: MODER = 00.
#define MODO_ENTRADA 0U
// Modo salida: MODER = 01.
#define MODO_SALIDA 1U
// Sin pull-up ni pull-down: PUPDR = 00.
#define RESISTENCIA_NINGUNA 0U
// Pull-up interno: PUPDR = 01 (el pin queda en 1 si nadie lo baja).
#define RESISTENCIA_PULL_UP 1U
// Velocidad baja: OSPEEDR = 00.
#define VELOCIDAD_BAJA 0U

// Datos para configurar un pin de una sola vez.
typedef struct {
    // Modo del pin: MODO_ENTRADA o MODO_SALIDA.
    uint32_t modo;
    // Número del pin dentro del puerto (0 a 15).
    uint32_t pin;
    // Resistencia interna: RESISTENCIA_NINGUNA o RESISTENCIA_PULL_UP.
    uint32_t resistencia;
    // Velocidad del pin: VELOCIDAD_BAJA.
    uint32_t velocidad;
    // Función alterna (AF0 a AF15). Usar 0U cuando el pin no es de función alterna.
    uint32_t funcion_alterna;
} ConfiguracionPin;

// Enciende el reloj del puerto indicado (solo puerto A o C).
void habilitar_reloj_puerto(RegistrosGPIO *puerto);
// Escribe en los registros del puerto la configuración de un pin.
void configurar_pin(RegistrosGPIO *puerto, ConfiguracionPin configuracion);
// Devuelve 1 si el pin está en 1 y 0 si está en 0.
uint32_t leer_pin(RegistrosGPIO *puerto, uint32_t pin);
// Pone el pin en 1 si estado es distinto de 0, o en 0 si estado es 0.
void escribir_pin(RegistrosGPIO *puerto, uint32_t pin, uint32_t estado);
// Cambia el pin de 0 a 1 o de 1 a 0.
void invertir_pin(RegistrosGPIO *puerto, uint32_t pin);

#endif
```

- Las cinco constantes (`MODO_ENTRADA`, `MODO_SALIDA`, `RESISTENCIA_NINGUNA`, `RESISTENCIA_PULL_UP`, `VELOCIDAD_BAJA`) son los valores que se escriben en `MODER`, `PUPDR` y `OSPEEDR`; solo se definieron las que el paso 1 usa.
- `ConfiguracionPin`: los cinco datos para configurar un pin con una sola llamada.
- Los cinco prototipos son las funciones de `gpio_config.c`.

`src/gpio_config.c`

```c
#include "gpio_config.h"

// Enciende el reloj del puerto. Sin reloj, los registros del puerto no responden.
void habilitar_reloj_puerto(RegistrosGPIO *puerto)
{
    // Si el puerto es A, se pone en 1 el bit 0 de RCC->AHB1ENR (|= no cambia los demás bits).
    if (puerto == GPIOA) {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
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
```

- `habilitar_reloj_puerto`: pone en 1 el bit del puerto en `RCC->AHB1ENR` con `|=` (no apaga los relojes de los otros puertos). Sin reloj, los registros del puerto no responden. Las dos lecturas descartadas vienen del ejemplo del profesor (errata del chip: el periférico necesita unos ciclos antes de poder usarse; no verificado en el RM0390). `(void)lectura_descartada;` le dice al compilador que el valor se descarta a propósito y evita la advertencia.
- `configurar_pin`: primero habilita el reloj (sin él no se pueden escribir los registros). Después, para cada registro, dos pasos en una línea: `registro & ~(0x3U << (pin * 2U))` borra los 2 bits del pin, y `| (valor << (pin * 2U))` pone el valor nuevo en esos 2 bits. Con `AFRL` y `AFRH` es igual, pero con 4 bits por pin (máscara `0xFU`); para los pines 8 a 15 se usa `AFRH` y la posición `(pin - 8U) * 4U`.
- **Corrección al `WRITE_REG_FIELD` del profesor.** La macro no se usa (su `POSITION_VAL` viene de CMSIS), pero el error está en las 5 líneas de arriba: los paréntesis de `~( ... )` son obligatorios. Ejemplo con el pin 5 (posición 10): sin paréntesis, `~0x3U << 10` se evalúa como `(~0x3U) << 10` y da `…1111 1100 0000 0000`; al hacer `&`, los 10 bits bajos quedan en 0 y se borra la configuración de los pines 0 a 4. Con paréntesis, `~(0x3U << 10)` da `…1111 0011 1111 1111` y solo se borran los bits 10 y 11, que son los del pin 5. En el archivo del profesor la macro aparece 11 veces (6 en el reloj, 3 en `MODER`, `OSPEEDR` y `PUPDR`, 2 en `AFRL` y `AFRH`).
- `leer_pin`: `IDR & (1U << pin)` deja solo el bit del pin; devuelve 1 si estaba en 1 y 0 si no.
- `escribir_pin`: con `estado != 0` pone el bit en `ODR` con `|=`; con 0 lo borra con `&= ~(1U << pin)`. Es lectura-modificación-escritura, como el profesor (el paso 4 explicará `BSRR`).
- `invertir_pin`: `^=` con la máscara del pin invierte solo ese bit de `ODR`.

`main.c`

```c
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
```

- `PIN_LED` (5) es el LED PA5 y `PIN_BOTON_AZUL` (13) es el botón PC13 (`CLAUDE.md`, sección Hardware).
- Se configura el LED como salida (sin resistencia, velocidad baja, función alterna 0), asignando cada campo de `ConfiguracionPin` en su línea, y después el botón como entrada con pull-up interno.
- En el bucle, `leer_pin(GPIOC, PIN_BOTON_AZUL) == 0U` significa «botón presionado» (activo en bajo: el pull-up lo deja en 1 y el botón lo lleva a 0); entonces se pone PA5 en 1 y el LED enciende. Si no, se pone en 0.

#### Modificaciones y correcciones (respecto al código del profesor)
- Los registros ya no vienen de CMSIS: son los structs `RegistrosGPIO` y `RegistrosRCC` (modelo: `stm32f401.h` del ejemplo 4), con nombres en español para los tipos y los nombres del manual para los campos.
- `WRITE_REG_FIELD` no se usa: se escribe directo en el registro con `~( ... )` con paréntesis, porque la macro dependía de `POSITION_VAL` (CMSIS) y traía el error de los paréntesis.
- Funciones con nombres en español: `habilitar_reloj_puerto`, `configurar_pin`, `leer_pin`, `escribir_pin`, `invertir_pin`; la configuración es `ConfiguracionPin` y no `GPIO_InitTypeDef`.
- Constantes con nombre en lugar de números sueltos.
- `habilitar_reloj_puerto` solo maneja los puertos A y C (el profesor hace A, B, C, D, E y H); B se agrega en el paso 3.
- El ejemplo del profesor enciende el LED cuando el botón **no** está presionado (`!= 0`); aquí se enciende mientras se presiona (`== 0`).
- `main.c` no usa `<stdlib.h>` ni `<string.h>`.
- Se agregó `(void)lectura_descartada;` para que el paso compile sin advertencias.

#### Decisiones tomadas
| Pregunta | Opciones | Decisión de Daniela |
|---|---|---|
| Cómo definir los registros | Struct de registros, `#define` con base y desplazamiento, o `#define` por puerto | Struct de registros (cambia la decisión anterior de `#define` sueltos para poder copiar las funciones del profesor) |
| `WRITE_REG_FIELD` | No usarla (escritura directa) o redefinirla sin `POSITION_VAL` | No usarla; explicar el error de los paréntesis en el comentario |
| Nombres | Español completo; constantes con nombre o números | Español; constantes con nombre (`#define` de un solo valor) |
| Puertos y reloj | Solo A y C ahora; B en el paso 3 | Solo RCC, GPIOA y GPIOC; el reloj de B en el paso 3 |
| `escribir_pin` | `ODR` con lectura-modificación-escritura o `BSRR` | Como el profesor (`ODR`); `BSRR` se explica en el paso 4 |
| Resistencia de PC13 | Ninguna o pull-up interno | Pull-up interno |
| Tipos | `stdint.h` o `unsigned int` | `stdint.h` y `uint32_t` |
| Struct de RCC | Relleno `RESERVADO[12]` o los nombres del manual | Los nombres del manual hasta `AHB1ENR` |
| Constantes | Solo las que se usan o todas | Solo las que el paso usa |
| Nombres de los tipos | `RegistrosGPIO`, `RegistrosRCC`, `ConfiguracionPin` | Aceptados |
| Advertencia de `lectura_descartada` | `(void)lectura_descartada;`, dejarla documentada o quitar la variable | `(void)lectura_descartada;` |
| Una o dos lecturas | Una o dos | Dos, como el ejemplo |

#### Cómo se probó
- `make clean && make`: sin advertencias; `arm-none-eabi-size`: `text 1130`, `data 0`, `bss 2`.
- El `revisor` comprobó con `offsetof` y `_Static_assert` que cada campo de los structs está en el desplazamiento del manual, y en el desensamblado (`objdump -d`) que se usan los desplazamientos `#0`, `#8`, `#12`, `#32` y `#36` y las direcciones `0x40020000`, `0x40023800` y `0x40020800`.
- `make flash`: `device id = 0x10006421`, `flash size = 512 kbytes`, `Programming Finished`, `Verified OK`, `Resetting Target`.
- En la placa: el LED PA5 se enciende mientras Daniela mantiene presionado el botón azul y se apaga al soltarlo.
- `nm`: `g_pfnVectors = 0x08000000`, `main = 0x080001c4`, `Reset_Handler = 0x08000420`, `_sidata = 0x0800046c`, `_estack = 0x20020000`.

#### Errores encontrados y cómo se resolvieron
- La primera compilación dio una advertencia `-Wunused-but-set-variable` en `lectura_descartada` (una variable `volatile` que solo se escribe). El `revisor` comprobó que con `(void)lectura_descartada;` o con `(void)RCC->AHB1ENR;` desaparece; Daniela eligió la primera.
- VS Code subrayaba `RegistrosGPIO` en `src/gpio_config.c` (Problems marcaba 6) aunque `make` compilaba sin advertencias y la placa funcionaba. Causa probable: IntelliSense no lee el `-Iinc` del Makefile y, con la carpeta `Microprocesadores/` abierta, puede encontrar el `gpio_config.h` del profesor. Solución: abrir directamente la carpeta del repo en una ventana nueva. Resultado confirmado por Daniela: 0 errores y 0 advertencias. No hizo falta crear `.vscode/c_cpp_properties.json`.

#### Preguntas de autoevaluación
1. ¿Por qué el orden de los campos de `RegistrosGPIO` no se puede cambiar?
   Porque el struct es el mapa de los registros del puerto: cada campo está 4 bytes después del anterior y el compilador calcula la dirección de `puerto->ODR` sumando el desplazamiento del campo (`0x14`) a la dirección base.
2. ¿Qué pasa si se escribe `~0x3U << posición` sin paréntesis alrededor de `0x3U << posición`?
   Se evalúa como `(~0x3U) << posición`, la máscara queda con ceros en todos los bits por debajo de la posición y el `&` borra también la configuración de los pines con número menor.
3. ¿Por qué el LED se enciende cuando `leer_pin(GPIOC, PIN_BOTON_AZUL) == 0`?
   Porque el botón es activo en bajo: el pull-up deja el pin en 1 y al presionarlo el botón lo lleva a 0.
4. ¿Por qué `configurar_pin` habilita el reloj antes de escribir los registros?
   Porque los relojes de los puertos están apagados tras el reset y, sin reloj, los registros del puerto no responden.
5. ¿Para qué sirve `volatile` en los campos de los registros?
   Para que el compilador no optimice ni elimine las lecturas y escrituras: el valor puede cambiar por fuera del programa (un pin) y cada acceso debe hacerse de verdad.
6. ¿Qué hace cada parte de `(registro & ~(0x3U << (pin * 2U))) | (valor << (pin * 2U))`?
   El `&` con la máscara invertida borra los 2 bits del pin y deja los demás; el `|` con el valor desplazado escribe el valor nuevo en esos 2 bits.

## Paso 2 — Reloj a 84 MHz y base de tiempo SysTick

### Objetivo
Subir el reloj del sistema de 16 MHz (HSI, el reloj con el que arranca el chip) a **84 MHz** usando el PLL, y tener una **base de tiempo de 1 ms** con SysTick para poder esperar tiempos exactos (`esperar_milisegundos`). Se prueba haciendo parpadear el LED PA5 a 1 Hz y midiendo 60 destellos con cronómetro.

### Archivos creados o modificados
- `inc/registros_f446.h` (modificado): se agregan `RegistrosFLASH`, `RegistrosSysTick`, los punteros `FLASH` y `SysTick`, y las constantes de posición y máscara de los campos que se usan. Los comentarios de `CIR`, `AHB1RSTR`, `AHB2RSTR`, `AHB3RSTR`, `APB1RSTR` y `APB2RSTR` pasan de «No se usa todavía (paso 2)» a «No se usa todavía».
- `inc/system_config.h` (nuevo): constantes de frecuencia y declaración de las funciones.
- `src/system_config.c` (nuevo): `configurar_reloj`, `iniciar_systick`, `SysTick_Handler` y `esperar_milisegundos`.
- `main.c` (modificado): ya no configura el botón; hace parpadear el LED a 1 Hz con `esperar_milisegundos`.
- La versión de `registros_f446.h` y de `main.c` del paso 1 se conserva en la pieza 4 del Paso 1, con una nota (decisión de Daniela, opción a).
- El Paso 3 cambia otra vez `registros_f446.h` y `main.c` (y `gpio_config.*`): los que aparecen en esta sección son las **versiones del Paso 2**.

### Concepto de clase que se usa
Reloj del sistema (RCC y PLL), estados de espera de la FLASH, SysTick, interrupciones y variables `volatile`. Los archivos `system_config.*` son los que `CLAUDE.md` lista como base del profesor (ejemplos 4, 7 y 24); aquí se reescribieron con nuestros structs y nombres, sin CMSIS.

**Cómo se hizo:** el agente principal resolvió con Daniela todas las decisiones (ver más abajo); el subagente `programador` (Haiku) escribió los archivos; el subagente `revisor` (Sonnet) los revisó y solo cambió dos comentarios de fuente (los límites de 45 MHz y 90 MHz); el agente principal leyó todo, recompiló y grabó la placa.

**Fuentes:** RM0390 secciones 3.4.1 (tabla 5, estados de espera), 3.8.1 (`FLASH_ACR`), 6.2 «Clocks», 6.3.1 (`RCC_CR`), 6.3.2 (`RCC_PLLCFGR`) y 6.3.3 (`RCC_CFGR`), leídas en una transcripción en línea (docs.dirba.io), **no en el PDF de ST**. Los bits de SysTick vienen del PM0214 (manual del núcleo Cortex-M4); el agente principal **no pudo verificar el número de sección** y en el código dice «por verificar por Daniela». Las direcciones y bits de SysTick, FLASH y RCC se compararon con `core_cm4.h` y `stm32f446xx.h` de STM32CubeF4 (carpeta hermana, solo como verificación; no se usan en el proyecto) y coinciden todas.
**Antes de sustentar conviene comprobar estas tablas en el PDF del RM0390 y la sección de SysTick en el PM0214.**

### Cálculo del PLL, paso a paso
El PLL toma un reloj de entrada, lo divide, lo multiplica y lo vuelve a dividir. Fórmulas del RM0390, sección 6.3.2 (`RCC_PLLCFGR`):

```
f_entrada_VCO = f_fuente / PLLM
f_VCO         = f_entrada_VCO × PLLN
f_sistema     = f_VCO / PLLP
```

1. **Fuente.** `PLLSRC = 0` (valor tras el reset): la fuente es la HSI, **16 MHz**. Por eso no se escribe `PLLSRC`.
2. **Divisor M.** Decisión de Daniela: `PLLM = 16`. Entrada del VCO = 16 MHz / 16 = **1 MHz**.
   - El manual exige que la entrada del VCO esté entre **1 y 2 MHz**; 1 MHz es el mínimo permitido (con `PLLM = 17` ya serían 0,94 MHz, fuera de rango).
   - El manual recomienda 2 MHz porque reduce el jitter (variación del instante de los flancos). Con `PLLM = 8` serían 2 MHz y `PLLN = 168` daría los mismos 84 MHz. Daniela decidió `PLLM = 16`; el resultado es el mismo para este proyecto.
3. **Multiplicador N.** `PLLN = 336`. VCO = 1 MHz × 336 = **336 MHz**. El VCO debe quedar entre 100 y 432 MHz, y 336 está dentro.
4. **Divisor P.** El campo `PLLP` es un código: `00` = /2, `01` = /4, `10` = /6, `11` = /8. Se escribe el código `01` (/4). Reloj del sistema = 336 MHz / 4 = **84 MHz**.
5. **Buses derivados de los 84 MHz.**
   - `HPRE = 0xxx` (sin división): AHB = HCLK = **84 MHz** (CPU, memoria y GPIO).
   - `PPRE1 = 100` (/2): APB1 = **42 MHz** (máximo permitido: 45 MHz; sin el /2 serían 84 y se pasaría).
   - `PPRE2 = 0xx` (sin división): APB2 = **84 MHz** (máximo permitido: 90 MHz).
   - Regla del RM0390: cuando el divisor de un bus APB no es 1, los **timers** de ese bus reciben el doble. Por eso los timers de APB1 (TIM3) reciben 42 × 2 = **84 MHz**.
6. **Estados de espera de la FLASH.** La FLASH no puede leer a 84 MHz sin esperar. Con VDD entre 2,7 y 3,6 V (la placa mide ~3,27 V), la tabla 5 (RM0390, 3.4.1) pide **2 estados de espera** para 60 < HCLK ≤ 90 MHz. Por eso `LATENCY = 2` y se escribe **antes** de subir la frecuencia.

### Cálculo de `LOAD` (SysTick), paso a paso
SysTick cuenta hacia abajo desde `LOAD` hasta 0, y al pasar de 0 recarga `LOAD`. Ese recorrido tiene `LOAD + 1` valores (incluye el 0), es decir, `LOAD + 1` ciclos del reloj.

1. Se quiere una interrupción cada 1 ms, o sea 1000 por segundo.
2. Ciclos por interrupción = 84 000 000 / 1000 = **84 000**.
3. `LOAD + 1 = 84 000` → `LOAD = 84 000 − 1 =` **83 999** (`0x1481F`).
4. Cabe en 24 bits (máximo 16 777 215).
5. Comprobación: cada interrupción son 84 000 ciclos × (1 / 84 000 000 s) = 1 ms exacto. Con `LOAD = 84 000` serían 84 001 ciclos y cada «milisegundo» sería un poco más largo.

### Código explicado

`inc/registros_f446.h`

```c
#ifndef REGISTROS_F446_H
#define REGISTROS_F446_H

#include <stdint.h>

// Direcciones base de los buses. Fuente: RM0390 (manual de referencia STM32F446),
// sección 2.2.2, tabla 1 «STM32F446xx register boundary addresses».
#define PERIFERICOS_BASE (0x40000000U)
// Bus AHB1: aquí están los puertos GPIO y el RCC.
#define AHB1_BASE (PERIFERICOS_BASE + 0x20000U)

// Direcciones base de cada periférico. Fuente: RM0390, sección 2.2.2, tabla 1.
// Puerto A de GPIO (0x40020000).
#define GPIOA_BASE (AHB1_BASE)
// Puerto C de GPIO (0x40020800).
#define GPIOC_BASE (AHB1_BASE + 0x800U)
// Controlador de reloj RCC (0x40023800).
#define RCC_BASE (AHB1_BASE + 0x3800U)
// Interfaz de la memoria FLASH (0x40023C00). Fuente: RM0390, sección 2.2.2, tabla 1, fila «Flash interface register».
#define FLASH_BASE (AHB1_BASE + 0x3C00U)
// Registros SysTick del núcleo Cortex-M4 (0xE000E010). Fuente: PM0214, sección «SysTick timer (STK)» (número de sección y tabla por verificar por Daniela); coincide con core_cm4.h de CMSIS de ARM.
#define SYSTICK_BASE (0xE000E010U)

// Mapa de los registros del puerto: cada campo está 4 bytes después del anterior,
// por eso el orden no se puede cambiar. Fuente: RM0390, capítulo 7, sección 7.4, tabla 26.
typedef struct {
    // Desplazamiento 0x00. Modo de cada pin: 2 bits por pin (00 entrada, 01 salida, 10 alterna, 11 analógico).
    volatile uint32_t MODER;
    // Desplazamiento 0x04. Tipo de salida de cada pin: 1 bit por pin (0 push-pull, 1 open-drain).
    volatile uint32_t OTYPER;
    // Desplazamiento 0x08. Velocidad de cambio de cada pin: 2 bits por pin.
    volatile uint32_t OSPEEDR;
    // Desplazamiento 0x0C. Resistencia pull-up o pull-down de cada pin: 2 bits por pin.
    volatile uint32_t PUPDR;
    // Desplazamiento 0x10. Lectura del estado de los pines de entrada: 1 bit por pin.
    volatile uint32_t IDR;
    // Desplazamiento 0x14. Valor que se saca por cada pin de salida: 1 bit por pin.
    volatile uint32_t ODR;
    // Desplazamiento 0x18. Pone (bits 0 a 15) o borra (bits 16 a 31) bits de ODR de forma atómica.
    volatile uint32_t BSRR;
    // Desplazamiento 0x1C. Bloquea la configuración de los pines hasta el próximo reinicio.
    volatile uint32_t LCKR;
    // Desplazamiento 0x20. Función alterna de los pines 0 a 7: 4 bits por pin.
    volatile uint32_t AFRL;
    // Desplazamiento 0x24. Función alterna de los pines 8 a 15: 4 bits por pin.
    volatile uint32_t AFRH;
} RegistrosGPIO;

// Mapa de los registros del RCC (reloj). Cada campo está en su desplazamiento del manual.
// Fuente: RM0390, capítulo 6, sección 6.3, «RCC registers», mapa de registros.
typedef struct {
    // Desplazamiento 0x00. Control del reloj (HSI, HSE, PLL). Se usa en el paso 2 (configurar_reloj).
    volatile uint32_t CR;
    // Desplazamiento 0x04. Configuración del PLL. Se usa en el paso 2 (configurar_reloj).
    volatile uint32_t PLLCFGR;
    // Desplazamiento 0x08. Configuración del reloj del sistema y de los buses. Se usa en el paso 2 (configurar_reloj).
    volatile uint32_t CFGR;
    // Desplazamiento 0x0C. Interrupciones del reloj. No se usa todavía.
    volatile uint32_t CIR;
    // Desplazamiento 0x10. Reinicia los puertos AHB1. No se usa todavía.
    volatile uint32_t AHB1RSTR;
    // Desplazamiento 0x14. Reinicia los periféricos AHB2. No se usa todavía.
    volatile uint32_t AHB2RSTR;
    // Desplazamiento 0x18. Reinicia los periféricos AHB3. No se usa todavía.
    volatile uint32_t AHB3RSTR;
    // Desplazamiento 0x1C. Reservado por el fabricante, no se usa.
    uint32_t RESERVADO_0;
    // Desplazamiento 0x20. Reinicia los periféricos del bus APB1. No se usa todavía.
    volatile uint32_t APB1RSTR;
    // Desplazamiento 0x24. Reinicia los periféricos del bus APB2. No se usa todavía.
    volatile uint32_t APB2RSTR;
    // Desplazamientos 0x28 y 0x2C. Reservados por el fabricante, no se usan.
    uint32_t RESERVADO_1[2];
    // Desplazamiento 0x30. Habilita el reloj de cada puerto GPIO (bit 0 = puerto A, bit 2 = puerto C).
    volatile uint32_t AHB1ENR;
} RegistrosRCC;

// Solo se define ACR: los demás registros de la FLASH no se usan todavía.
typedef struct {
    // Desplazamiento 0x00. Control de acceso a la FLASH: estados de espera (LATENCY, bits 3:0). Fuente: RM0390, sección 3.8.1.
    volatile uint32_t ACR;
} RegistrosFLASH;

// Mapa de los registros de SysTick, mismo criterio que RegistrosGPIO.
typedef struct {
    // Desplazamiento 0x00. Control y estado: habilitar, interrupción y fuente de reloj (bits 0 a 2).
    volatile uint32_t CTRL;
    // Desplazamiento 0x04. Valor de recarga del contador, de 24 bits (bits 23:0).
    volatile uint32_t LOAD;
    // Desplazamiento 0x08. Valor actual del contador; escribir cualquier valor lo pone en 0.
    volatile uint32_t VAL;
    // Desplazamiento 0x0C. Calibración del contador. No se usa.
    volatile uint32_t CALIB;
} RegistrosSysTick;

// Punteros a los periféricos: se usan como GPIOA->ODR, RCC->AHB1ENR, etc.
// Cada puntero apunta a la dirección base del periférico.
#define GPIOA ((RegistrosGPIO *)GPIOA_BASE)
// Puntero al puerto C de GPIO (botón azul PC13).
#define GPIOC ((RegistrosGPIO *)GPIOC_BASE)
// Puntero al controlador de reloj RCC.
#define RCC ((RegistrosRCC *)RCC_BASE)
// Puntero a la interfaz de la FLASH (se usa para FLASH->ACR).
#define FLASH ((RegistrosFLASH *)FLASH_BASE)
// Puntero a los registros de SysTick (se usa para SysTick->CTRL, LOAD y VAL).
#define SysTick ((RegistrosSysTick *)SYSTICK_BASE)

// Bit 0 de RCC_AHB1ENR: 1 enciende el reloj del puerto A. Fuente: RM0390, sección 6.3.10.
#define RCC_AHB1ENR_GPIOAEN (1U << 0)
// Bit 2 de RCC_AHB1ENR: 1 enciende el reloj del puerto C. Fuente: RM0390, sección 6.3.10.
#define RCC_AHB1ENR_GPIOCEN (1U << 2)

// Bit 24 de RCC_CR: 1 enciende el PLL. Fuente: RM0390, sección 6.3.1.
#define RCC_CR_PLLON (1U << 24)
// Bit 25 de RCC_CR: 1 indica que el PLL ya enganchó y está listo. Fuente: RM0390, sección 6.3.1.
#define RCC_CR_PLLRDY (1U << 25)

// Posición del campo PLLM (divisor de entrada del PLL) en RCC_PLLCFGR, bits 5:0. Fuente: RM0390, sección 6.3.2.
#define RCC_PLLCFGR_PLLM_POSICION 0U
// Máscara de los 6 bits de PLLM, ya desplazada a su posición. Fuente: RM0390, sección 6.3.2.
#define RCC_PLLCFGR_PLLM_MASCARA (0x3FU << RCC_PLLCFGR_PLLM_POSICION)
// Posición del campo PLLN (multiplicador del VCO) en RCC_PLLCFGR, bits 14:6. Fuente: RM0390, sección 6.3.2.
#define RCC_PLLCFGR_PLLN_POSICION 6U
// Máscara de los 9 bits de PLLN, ya desplazada a su posición. Fuente: RM0390, sección 6.3.2.
#define RCC_PLLCFGR_PLLN_MASCARA (0x1FFU << RCC_PLLCFGR_PLLN_POSICION)
// Posición del campo PLLP (divisor de salida del PLL) en RCC_PLLCFGR, bits 17:16. Fuente: RM0390, sección 6.3.2.
#define RCC_PLLCFGR_PLLP_POSICION 16U
// Máscara de los 2 bits de PLLP, ya desplazada a su posición. Fuente: RM0390, sección 6.3.2.
#define RCC_PLLCFGR_PLLP_MASCARA (0x3U << RCC_PLLCFGR_PLLP_POSICION)

// Posición del campo SW (fuente del reloj del sistema) en RCC_CFGR, bits 1:0. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_SW_POSICION 0U
// Máscara de los 2 bits de SW, ya desplazada a su posición. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_SW_MASCARA (0x3U << RCC_CFGR_SW_POSICION)
// Posición del campo SWS (fuente que el hardware está usando de verdad) en RCC_CFGR, bits 3:2. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_SWS_POSICION 2U
// Máscara de los 2 bits de SWS, ya desplazada a su posición. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_SWS_MASCARA (0x3U << RCC_CFGR_SWS_POSICION)
// Posición del campo HPRE (divisor del AHB) en RCC_CFGR, bits 7:4. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_HPRE_POSICION 4U
// Máscara de los 4 bits de HPRE, ya desplazada a su posición. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_HPRE_MASCARA (0xFU << RCC_CFGR_HPRE_POSICION)
// Posición del campo PPRE1 (divisor del APB1) en RCC_CFGR, bits 12:10. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_PPRE1_POSICION 10U
// Máscara de los 3 bits de PPRE1, ya desplazada a su posición. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_PPRE1_MASCARA (0x7U << RCC_CFGR_PPRE1_POSICION)
// Posición del campo PPRE2 (divisor del APB2) en RCC_CFGR, bits 15:13. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_PPRE2_POSICION 13U
// Máscara de los 3 bits de PPRE2, ya desplazada a su posición. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_PPRE2_MASCARA (0x7U << RCC_CFGR_PPRE2_POSICION)

// Posición del campo LATENCY (estados de espera de la FLASH) en FLASH_ACR, bits 3:0. Fuente: RM0390, sección 3.8.1.
#define FLASH_ACR_LATENCY_POSICION 0U
// Máscara de los 4 bits de LATENCY, ya desplazada a su posición. Fuente: RM0390, sección 3.8.1.
#define FLASH_ACR_LATENCY_MASCARA (0xFU << FLASH_ACR_LATENCY_POSICION)

// Bit 0 de SysTick CTRL: 1 enciende el contador. Fuente: PM0214, sección SysTick (por verificar por Daniela).
#define SYSTICK_CTRL_ENABLE (1U << 0)
// Bit 1 de SysTick CTRL: 1 genera la interrupción al llegar el contador a 0. Fuente: PM0214, sección SysTick (por verificar por Daniela).
#define SYSTICK_CTRL_TICKINT (1U << 1)
// Bit 2 de SysTick CTRL: 1 usa el reloj del procesador, 0 usa AHB/8. Fuente: PM0214, sección SysTick (por verificar por Daniela).
#define SYSTICK_CTRL_CLKSOURCE (1U << 2)

#endif
```

Lo que **ya existía** (direcciones base, `RegistrosGPIO`, `RegistrosRCC`, punteros y `GPIOAEN`/`GPIOCEN`) está explicado en la pieza 4 del Paso 1. Lo **nuevo**:
- `FLASH_BASE` = `AHB1_BASE + 0x3C00` = `0x40023C00` (RM0390, 2.2.2, tabla 1, fila «Flash interface register»).
- `SYSTICK_BASE` = `0xE000E010`. Está en la región del núcleo (PPB, `0xE0000000` en adelante), no en un bus del fabricante; por eso la fuente es el PM0214 y no el RM0390.
- `RegistrosFLASH`: solo `ACR` (desplazamiento `0x00`), el único registro de la FLASH que usamos. `LATENCY` son los bits 3:0.
- `RegistrosSysTick`: `CTRL` (`0x00`), `LOAD` (`0x04`), `VAL` (`0x08`) y `CALIB` (`0x0C`, no se usa). Mismo criterio que `RegistrosGPIO`: cada campo, 4 bytes después del anterior.
- Punteros `FLASH` y `SysTick`: se usan como `FLASH->ACR` y `SysTick->LOAD`.
- `RCC_CR_PLLON` (bit 24) y `RCC_CR_PLLRDY` (bit 25): encender el PLL y saber que ya enganchó.
- Para cada campo hay `_POSICION` (dónde empieza) y `_MASCARA` (unos en todos sus bits, ya desplazados). Ejemplo: `PLLN` ocupa los bits 14:6 (9 bits), posición 6 y máscara `0x1FF << 6`. Las máscaras tienen **paréntesis**, para que `~RCC_PLLCFGR_PLLN_MASCARA` invierta toda la máscara y no solo `0x1FF` (el error de los paréntesis del Paso 1).
- Campos usados: `PLLM` (5:0), `PLLN` (14:6), `PLLP` (17:16), `SW` (1:0), `SWS` (3:2), `HPRE` (7:4), `PPRE1` (12:10), `PPRE2` (15:13) y `LATENCY` (3:0).
- Bits de `SysTick->CTRL`: `ENABLE` (bit 0), `TICKINT` (bit 1) y `CLKSOURCE` (bit 2).

`inc/system_config.h`

```c
#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include "registros_f446.h"

// Frecuencia del reloj del sistema (SYSCLK y HCLK) después de configurar_reloj: 84 MHz.
#define FRECUENCIA_RELOJ_HZ 84000000U
// Interrupciones de SysTick por segundo: 1000 = una interrupción por milisegundo (base de tiempo de 1 ms).
#define INTERRUPCIONES_SYSTICK_POR_SEGUNDO 1000U

// Configura el PLL a 84 MHz, la latencia de la FLASH y los divisores de los buses. Se llama antes que cualquier periférico.
void configurar_reloj(void);
// Enciende SysTick para que interrumpa cada 1 ms (base de tiempo).
void iniciar_systick(void);
// Espera la cantidad de milisegundos indicada. Mientras espera, la interrupción de SysTick sigue contando.
void esperar_milisegundos(uint32_t milisegundos);

#endif
```

- Las dos constantes son la fuente única de los 84 MHz y de los 1000 ticks por segundo; `iniciar_systick` las usa para calcular `LOAD`.
- Las tres declaraciones son las funciones públicas. `SysTick_Handler` no se declara aquí: el nombre lo usa la tabla de vectores, no se llama desde C.

`src/system_config.c`

```c
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
}

void esperar_milisegundos(uint32_t milisegundos)
{
    // Se guarda el instante en que empieza la espera.
    uint32_t instante_inicial = contador_milisegundos;
    // La resta funciona aunque el contador de 32 bits dé la vuelta. main espera aquí y la interrupción sigue contando.
    while ((contador_milisegundos - instante_inicial) < milisegundos) { }
}
```

**Constantes del principio (líneas 3 a 18).** Cada valor tiene nombre y su fuente: `FLASH_ESTADOS_DE_ESPERA` = 2, `PLL_DIVISOR_ENTRADA_M` = 16, `PLL_MULTIPLICADOR_N` = 336, `PLL_CODIGO_DIVISOR_SALIDA_P` = 1 (el código `01`, no el divisor 4), `AHB_SIN_DIVISION` = 0, `APB1_DIVIDIR_ENTRE_2` = 4 (código `100`), `APB2_SIN_DIVISION` = 0 y `RELOJ_SISTEMA_PLL` = 2 (código `10`).

**`contador_milisegundos`.** `uint32_t` cuenta ~49,7 días antes de dar la vuelta. `volatile` porque lo cambia la interrupción y, sin él, el compilador podría leerlo una vez y hacer infinito el bucle de espera. `static` porque solo este archivo la usa.

**`configurar_reloj`, en este orden y por qué:**
1. **Latencia de la FLASH = 2** (primero). Se escribe solo el campo `LATENCY` con `(ACR & ~MASCARA) | (2 << POSICION)` y se relee hasta que valga 2, como recomienda el RM0390 (3.4.1). Si se subiera la frecuencia antes, la FLASH leería mal.
2. **Campos del PLL** (`PLLM`, `PLLN`, `PLLP`), uno por línea y con el PLL **apagado** (tras el reset lo está). No se escribe `PLLCFGR` completo para no pisar `PLLR` ni `PLLQ`, que conservan su valor de reset; `PLLSRC` queda en 0 (HSI).
3. **Divisores de los buses** (`HPRE`, `PPRE1`, `PPRE2`), todavía a 16 MHz, para que ningún bus supere su máximo cuando suba la frecuencia.
4. **Encender el PLL**: `RCC->CR |= RCC_CR_PLLON`. Se usa `|=` para no apagar la HSI (bit 0 de `CR`), que sigue siendo el reloj en uso.
5. **Esperar `PLLRDY`**: el bucle vacío repite mientras el bit 25 valga 0.
6. **Elegir el PLL**: se escribe `SW = 2`. Es lo que se **pide**.
7. **Esperar `SWS`**: `SWS` (bits 3:2) es lo que el hardware **usa de verdad**. Se compara con `RELOJ_SISTEMA_PLL << RCC_CFGR_SWS_POSICION` (2 << 2 = 8), porque el campo está en los bits 3:2. Al salir, el chip corre a 84 MHz.

No hay límite de tiempo en los bucles de espera: si el PLL no enganchara, el programa se quedaría esperando. Con el oscilador interno es muy improbable y agregar un límite es código que no se pidió.

**`iniciar_systick`:**
1. `CTRL &= ~ENABLE`: apaga el contador mientras se configura.
2. `LOAD = (84 000 000 / 1000) − 1` = 83 999 (cálculo de arriba).
3. `VAL = 0`: limpia la cuenta actual.
4. `CLKSOURCE = 1`: usa el reloj del procesador (84 MHz); con 0 usaría AHB/8 y el cálculo no valdría.
5. `TICKINT = 1`: al llegar a 0 se produce la excepción. SysTick es una excepción del núcleo (número 15, posición `0x3C` de la tabla de vectores): **no se habilita en el NVIC** ni necesita bit en RCC (no se copió el `SYSCFGEN` del ejemplo del profesor).
6. `ENABLE = 1`: al final, cuando todo está listo.

**`SysTick_Handler`.** El nombre debe ser exactamente el de `crt0.S`. Al definirla, reemplaza al alias débil que apuntaba a `Default_Handler`. En la placa se comprobó que el vector `0x3C` contiene `0x080004F9` (= `SysTick_Handler` en `0x080004F8` con el bit Thumb). No hay que limpiar ningún flag: el hardware limpia la solicitud al entrar. Solo suma 1 a `contador_milisegundos`.

**`esperar_milisegundos`.** Guarda `instante_inicial` y repite mientras `(contador − instante_inicial) < milisegundos`. Con enteros sin signo la resta da el resultado correcto aunque el contador dé la vuelta (si `inicial = 0xFFFFFFFF` y luego `contador = 1`, la resta es 2). Es una espera **bloqueante**: la CPU solo espera, la interrupción sigue contando. El error es de hasta 1 ms, porque el instante inicial cae a mitad de un milisegundo.

`main.c`

```c
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
```

- `configurar_reloj()` primero, porque todo lo posterior supone 84 MHz; después `iniciar_systick()`.
- La configuración del LED PA5 es la misma del Paso 1 (`configurar_pin` habilita el reloj de GPIOA por dentro).
- En el bucle, `invertir_pin` y esperar 500 ms: 500 ms encendido + 500 ms apagado = periodo de 1 s = **1 Hz**.
- Se quitó la configuración del botón PC13: este paso solo prueba el reloj.

### Modificaciones y correcciones
- `registros_f446.h` crece con los registros nuevos y cambia seis comentarios (ver arriba). `main.c` ya no tiene botón (la versión del Paso 1 queda en su pieza 4).
- El `revisor` cambió solo los comentarios de fuente de los límites de 45 MHz (APB1) y 90 MHz (APB2): ahora dicen «RM0390 sección 6.2 «Clocks»; código del campo en 6.3.3», porque la 6.3.3 solo da el código del campo y el límite está en la 6.2.
- El `revisor` comprobó con `offsetof` y `_Static_assert` los desplazamientos de los structs, todas las constantes contra los encabezados de STM32CubeF4, la secuencia de `configurar_reloj` y el desensamblado (`LOAD = 0x1481F`, base `0x40023C00`).
- Diferencias con el profesor: no se escribe `PLLCFGR` completo, no se habilita `SYSCFGEN`, los campos se escriben sin `WRITE_REG_FIELD` (con `~( ... )` entre paréntesis) y los nombres están en español.

### Decisiones tomadas
| Pregunta | Opciones | Decisión de Daniela |
|---|---|---|
| Estados de espera de la FLASH | Según la tabla 5 del RM0390 | 2 (2,7 a 3,6 V; la placa mide ~3,27 V) |
| Qué campos del PLL escribir | Todo `PLLCFGR` o solo los campos que se usan | Solo `PLLM`, `PLLN` y `PLLP`; `PLLQ`, `PLLR` y `PLLSRC` no se escriben |
| Valores del PLL | Por ejemplo M = 8, N = 168 o M = 16, N = 336 | M = 16, N = 336, P código 01 (/4) |
| Divisores de los buses | Varios | AHB sin dividir, APB1 /2, APB2 sin dividir |
| Constantes de los campos | Un valor mezclado o posición y máscara por campo | Posición y máscara entre paréntesis, un campo por línea |
| Base de tiempo | Varias | SysTick con interrupción cada 1 ms, reloj del procesador |
| Contador y espera | Varias | `contador_milisegundos` y `esperar_milisegundos`; los temporizadores por software se ven en el paso 7 |
| Prueba | Varias | LED a 1 Hz y 60 destellos con cronómetro |
| Política de la guía | (a) conservar en el Paso 1 la versión original con una nota, o (b) reemplazar por la versión actual | (a) |
| Comentarios «No se usa todavía (paso 2)» | Dejarlos o cambiarlos | Cambiarlos a «No se usa todavía» |

Decisiones menores que tomó el agente principal y que Daniela puede objetar: releer `LATENCY` (viene del RM0390, 3.4.1), `static` en el contador, no escribir `PLLSRC`, y `main` sin botón.

### Cómo se probó
- `make clean && make`: 0 advertencias; `arm-none-eabi-size`: `text 1422`, `data 0`, `bss 6`.
- `make flash`: `device id = 0x10006421`, `flash size = 512 kbytes`, `Programming Finished`, `Verified OK`, `Resetting Target`; voltaje medido: 3,2656 V.
- `nm`: `main = 0x080001c4`, `configurar_reloj = 0x080003e0`, `iniciar_systick = 0x080004a8`, `SysTick_Handler = 0x080004f8`, `esperar_milisegundos = 0x08000514`, `contador_milisegundos = 0x20000000`; vector `0x3C` = `0x080004f9`.
- En la placa (2026-11-09): el LED parpadea de forma estable; **60 destellos en 58,03 s**.

**Cómo interpretar la medida.** Con un reloj equivocado el resultado sería muy distinto: 16 MHz con el `LOAD` de 84 MHz daría unos 5 min, y un `LOAD` mal calculado daría un parpadeo mucho más rápido o lento. 58,03 s confirma que el PLL, `LOAD` y la interrupción funcionan. Pero la diferencia con 60 s es de **−3,3 %** (el reloj corre ~3 % más rápido), más que el ±1 % que se esperaba de la HSI. Posibles causas, sin comprobar: (1) la HSI es un oscilador RC y, según lo que recuerda el agente principal, su ±1 % es a 25 °C y puede ser mayor con otra temperatura (**por verificar en el datasheet del F446**); (2) error de cronometraje o de conteo de ±1 destello (~1 s, 1,7 %). Para saber más: repetir con 120 destellos. No bloquea el paso, pero **conviene vigilarlo en el paso 3 (USART a 115200)**, porque la UART tolera poco error de reloj.

### Errores encontrados y cómo se resolvieron
- Compilación: sin errores ni advertencias desde la primera vez.
- Un comentario del código decía que los límites de 45 y 90 MHz salían de la sección 6.3.3; el `revisor` lo corrigió a 6.2 (ver arriba).
- Medición de 58,03 s en lugar de ~60 s: pendiente de entender mejor (ver «Cómo interpretar la medida»).

### Preguntas de autoevaluación
1. ¿Por qué se escribe la latencia de la FLASH **antes** de cambiar al PLL?
   Porque a 84 MHz la FLASH necesita 2 estados de espera para leer bien (RM0390, tabla 5); si se sube la frecuencia con 0 estados, el procesador leería instrucciones mal y se bloquearía.
2. ¿Por qué `LOAD` vale 83 999 y no 84 000?
   Porque SysTick cuenta de `LOAD` hasta 0 incluido, que son `LOAD + 1` ciclos; para 84 000 ciclos (1 ms a 84 MHz) hace falta `LOAD = 83 999`.
3. ¿Por qué después de escribir `SW` se espera a que `SWS` valga 2 (comparado como `2 << 2`)?
   `SW` es la petición y `SWS` es la fuente que el hardware ya está usando; el cambio no es instantáneo. El campo `SWS` está en los bits 3:2, por eso el 2 se desplaza 2 posiciones.
4. ¿Qué pasaría si `contador_milisegundos` no fuera `volatile`?
   El compilador podría leerlo una sola vez y guardarlo en un registro de la CPU, porque en `main` nadie lo cambia; `esperar_milisegundos` no terminaría nunca, ya que la interrupción sí lo cambia pero el bucle no lo vería.
5. ¿Por qué `(contador − instante_inicial) < milisegundos` sigue funcionando cuando el contador de 32 bits da la vuelta?
   Porque con enteros sin signo la resta es módulo 2³²: si `inicial = 0xFFFFFFFF` y `contador = 1`, la resta da 2, que son los milisegundos reales transcurridos.

## Paso 3 — Botones del shield (entradas con pull-up)

### Objetivo
Configurar los seis botones del shield (A a F) como entradas con pull-up interno y comprobar que cada uno funciona. Esto obliga a agregar el puerto B (reloj y registros), porque cuatro de los seis botones están en él. La prueba usa el LED PA5: se enciende mientras cualquiera de los seis esté presionado. El USART queda para un paso aparte.

### Archivos creados o modificados
- `inc/registros_f446.h` (modificado): se agregan `GPIOB_BASE`, el puntero `GPIOB` y `RCC_AHB1ENR_GPIOBEN`; el comentario de `AHB1ENR` menciona el bit 1.
- `src/gpio_config.c` (modificado): `habilitar_reloj_puerto` maneja también el puerto B.
- `inc/gpio_config.h` (modificado): solo el comentario de `habilitar_reloj_puerto` («puerto A, B o C»).
- `main.c` (modificado): seis constantes de pin, seis bloques de configuración y un bucle que enciende el LED si hay algún botón presionado. Ya no hace parpadear el LED.
- Las versiones anteriores de `registros_f446.h`, `gpio_config.*` y `main.c` se conservan en el Paso 1 (pieza 4) y en el Paso 2, con una nota (política de la guía: opción a).

### Concepto de clase que se usa
Entradas digitales de GPIO (`MODER`, `PUPDR`, `IDR`) y pull-up interno, como el ejemplo `3-input-butt` del profesor, ahora con seis botones en dos puertos. No se agrega ningún concepto nuevo de hardware: se reutiliza `configurar_pin` y `leer_pin` del Paso 1.

**Cómo se hizo:** el agente principal resolvió con Daniela las decisiones (ver más abajo); el subagente `programador` (Haiku) escribió los cambios; el subagente `revisor` (Sonnet) los revisó y no cambió nada; el agente principal leyó todo, recompiló y grabó la placa.

**Fuentes:** `GPIOB_BASE` = `0x40020400` (RM0390, sección 2.2.2, tabla 1) y el bit 1 de `RCC_AHB1ENR` (sección 6.3.10), comprobados contra `stm32f446xx.h` de STM32CubeF4 (carpeta hermana, solo como verificación) y contra el desensamblado; **no contra el PDF de ST**. Los pines de los botones (A=PA10, B=PB3, C=PB5, D=PB4, E=PB10, F=PA8) son los del shield, según `CLAUDE.md`. **Antes de sustentar conviene comprobar la tabla 1 y la sección 6.3.10 en el PDF del RM0390, y la tabla de funciones alternas de PB3 y PB4.**

### Por qué los botones son activos en bajo
Cada botón conecta el pin a GND cuando se presiona. Cuando no se presiona, el pin quedaría «flotando» (sin un nivel definido), y una entrada flotante lee 0 o 1 al azar por el ruido. El **pull-up interno** (`PUPDR = 01`) es una resistencia dentro del microcontrolador que lleva el pin a 3,3 V cuando nadie lo baja. Resultado:
- botón **suelto** → el pin vale **1**;
- botón **presionado** → el botón lo conecta a GND y el pin vale **0**.

Por eso «presionado» se escribe `leer_pin(...) == 0U`. Se dice **activo en bajo** porque la acción se señala con el nivel bajo.

### Código explicado

`inc/registros_f446.h`

```c
#ifndef REGISTROS_F446_H
#define REGISTROS_F446_H

#include <stdint.h>

// Direcciones base de los buses. Fuente: RM0390 (manual de referencia STM32F446),
// sección 2.2.2, tabla 1 «STM32F446xx register boundary addresses».
#define PERIFERICOS_BASE (0x40000000U)
// Bus AHB1: aquí están los puertos GPIO y el RCC.
#define AHB1_BASE (PERIFERICOS_BASE + 0x20000U)

// Direcciones base de cada periférico. Fuente: RM0390, sección 2.2.2, tabla 1.
// Puerto A de GPIO (0x40020000).
#define GPIOA_BASE (AHB1_BASE)
// Puerto B de GPIO (0x40020400). Fuente: RM0390, sección 2.2.2, tabla 1.
#define GPIOB_BASE (AHB1_BASE + 0x400U)
// Puerto C de GPIO (0x40020800).
#define GPIOC_BASE (AHB1_BASE + 0x800U)
// Controlador de reloj RCC (0x40023800).
#define RCC_BASE (AHB1_BASE + 0x3800U)
// Interfaz de la memoria FLASH (0x40023C00). Fuente: RM0390, sección 2.2.2, tabla 1, fila «Flash interface register».
#define FLASH_BASE (AHB1_BASE + 0x3C00U)
// Registros SysTick del núcleo Cortex-M4 (0xE000E010). Fuente: PM0214, sección «SysTick timer (STK)» (número de sección y tabla por verificar por Daniela); coincide con core_cm4.h de CMSIS de ARM.
#define SYSTICK_BASE (0xE000E010U)

// Mapa de los registros del puerto: cada campo está 4 bytes después del anterior,
// por eso el orden no se puede cambiar. Fuente: RM0390, capítulo 7, sección 7.4, tabla 26.
typedef struct {
    // Desplazamiento 0x00. Modo de cada pin: 2 bits por pin (00 entrada, 01 salida, 10 alterna, 11 analógico).
    volatile uint32_t MODER;
    // Desplazamiento 0x04. Tipo de salida de cada pin: 1 bit por pin (0 push-pull, 1 open-drain).
    volatile uint32_t OTYPER;
    // Desplazamiento 0x08. Velocidad de cambio de cada pin: 2 bits por pin.
    volatile uint32_t OSPEEDR;
    // Desplazamiento 0x0C. Resistencia pull-up o pull-down de cada pin: 2 bits por pin.
    volatile uint32_t PUPDR;
    // Desplazamiento 0x10. Lectura del estado de los pines de entrada: 1 bit por pin.
    volatile uint32_t IDR;
    // Desplazamiento 0x14. Valor que se saca por cada pin de salida: 1 bit por pin.
    volatile uint32_t ODR;
    // Desplazamiento 0x18. Pone (bits 0 a 15) o borra (bits 16 a 31) bits de ODR de forma atómica.
    volatile uint32_t BSRR;
    // Desplazamiento 0x1C. Bloquea la configuración de los pines hasta el próximo reinicio.
    volatile uint32_t LCKR;
    // Desplazamiento 0x20. Función alterna de los pines 0 a 7: 4 bits por pin.
    volatile uint32_t AFRL;
    // Desplazamiento 0x24. Función alterna de los pines 8 a 15: 4 bits por pin.
    volatile uint32_t AFRH;
} RegistrosGPIO;

// Mapa de los registros del RCC (reloj). Cada campo está en su desplazamiento del manual.
// Fuente: RM0390, capítulo 6, sección 6.3, «RCC registers», mapa de registros.
typedef struct {
    // Desplazamiento 0x00. Control del reloj (HSI, HSE, PLL). Se usa en el paso 2 (configurar_reloj).
    volatile uint32_t CR;
    // Desplazamiento 0x04. Configuración del PLL. Se usa en el paso 2 (configurar_reloj).
    volatile uint32_t PLLCFGR;
    // Desplazamiento 0x08. Configuración del reloj del sistema y de los buses. Se usa en el paso 2 (configurar_reloj).
    volatile uint32_t CFGR;
    // Desplazamiento 0x0C. Interrupciones del reloj. No se usa todavía.
    volatile uint32_t CIR;
    // Desplazamiento 0x10. Reinicia los puertos AHB1. No se usa todavía.
    volatile uint32_t AHB1RSTR;
    // Desplazamiento 0x14. Reinicia los periféricos AHB2. No se usa todavía.
    volatile uint32_t AHB2RSTR;
    // Desplazamiento 0x18. Reinicia los periféricos AHB3. No se usa todavía.
    volatile uint32_t AHB3RSTR;
    // Desplazamiento 0x1C. Reservado por el fabricante, no se usa.
    uint32_t RESERVADO_0;
    // Desplazamiento 0x20. Reinicia los periféricos del bus APB1. No se usa todavía.
    volatile uint32_t APB1RSTR;
    // Desplazamiento 0x24. Reinicia los periféricos del bus APB2. No se usa todavía.
    volatile uint32_t APB2RSTR;
    // Desplazamientos 0x28 y 0x2C. Reservados por el fabricante, no se usan.
    uint32_t RESERVADO_1[2];
    // Desplazamiento 0x30. Habilita el reloj de cada puerto GPIO (bit 0 = puerto A, bit 1 = puerto B, bit 2 = puerto C).
    volatile uint32_t AHB1ENR;
} RegistrosRCC;

// Solo se define ACR: los demás registros de la FLASH no se usan todavía.
typedef struct {
    // Desplazamiento 0x00. Control de acceso a la FLASH: estados de espera (LATENCY, bits 3:0). Fuente: RM0390, sección 3.8.1.
    volatile uint32_t ACR;
} RegistrosFLASH;

// Mapa de los registros de SysTick, mismo criterio que RegistrosGPIO.
typedef struct {
    // Desplazamiento 0x00. Control y estado: habilitar, interrupción y fuente de reloj (bits 0 a 2).
    volatile uint32_t CTRL;
    // Desplazamiento 0x04. Valor de recarga del contador, de 24 bits (bits 23:0).
    volatile uint32_t LOAD;
    // Desplazamiento 0x08. Valor actual del contador; escribir cualquier valor lo pone en 0.
    volatile uint32_t VAL;
    // Desplazamiento 0x0C. Calibración del contador. No se usa.
    volatile uint32_t CALIB;
} RegistrosSysTick;

// Punteros a los periféricos: se usan como GPIOA->ODR, RCC->AHB1ENR, etc.
// Cada puntero apunta a la dirección base del periférico.
#define GPIOA ((RegistrosGPIO *)GPIOA_BASE)
// Puntero al puerto B de GPIO (botones B, C, D y E del shield).
#define GPIOB ((RegistrosGPIO *)GPIOB_BASE)
// Puntero al puerto C de GPIO (botón azul PC13).
#define GPIOC ((RegistrosGPIO *)GPIOC_BASE)
// Puntero al controlador de reloj RCC.
#define RCC ((RegistrosRCC *)RCC_BASE)
// Puntero a la interfaz de la FLASH (se usa para FLASH->ACR).
#define FLASH ((RegistrosFLASH *)FLASH_BASE)
// Puntero a los registros de SysTick (se usa para SysTick->CTRL, LOAD y VAL).
#define SysTick ((RegistrosSysTick *)SYSTICK_BASE)

// Bit 0 de RCC_AHB1ENR: 1 enciende el reloj del puerto A. Fuente: RM0390, sección 6.3.10.
#define RCC_AHB1ENR_GPIOAEN (1U << 0)
// Bit 1 de RCC_AHB1ENR: 1 enciende el reloj del puerto B. Fuente: RM0390, sección 6.3.10.
#define RCC_AHB1ENR_GPIOBEN (1U << 1)
// Bit 2 de RCC_AHB1ENR: 1 enciende el reloj del puerto C. Fuente: RM0390, sección 6.3.10.
#define RCC_AHB1ENR_GPIOCEN (1U << 2)

// Bit 24 de RCC_CR: 1 enciende el PLL. Fuente: RM0390, sección 6.3.1.
#define RCC_CR_PLLON (1U << 24)
// Bit 25 de RCC_CR: 1 indica que el PLL ya enganchó y está listo. Fuente: RM0390, sección 6.3.1.
#define RCC_CR_PLLRDY (1U << 25)

// Posición del campo PLLM (divisor de entrada del PLL) en RCC_PLLCFGR, bits 5:0. Fuente: RM0390, sección 6.3.2.
#define RCC_PLLCFGR_PLLM_POSICION 0U
// Máscara de los 6 bits de PLLM, ya desplazada a su posición. Fuente: RM0390, sección 6.3.2.
#define RCC_PLLCFGR_PLLM_MASCARA (0x3FU << RCC_PLLCFGR_PLLM_POSICION)
// Posición del campo PLLN (multiplicador del VCO) en RCC_PLLCFGR, bits 14:6. Fuente: RM0390, sección 6.3.2.
#define RCC_PLLCFGR_PLLN_POSICION 6U
// Máscara de los 9 bits de PLLN, ya desplazada a su posición. Fuente: RM0390, sección 6.3.2.
#define RCC_PLLCFGR_PLLN_MASCARA (0x1FFU << RCC_PLLCFGR_PLLN_POSICION)
// Posición del campo PLLP (divisor de salida del PLL) en RCC_PLLCFGR, bits 17:16. Fuente: RM0390, sección 6.3.2.
#define RCC_PLLCFGR_PLLP_POSICION 16U
// Máscara de los 2 bits de PLLP, ya desplazada a su posición. Fuente: RM0390, sección 6.3.2.
#define RCC_PLLCFGR_PLLP_MASCARA (0x3U << RCC_PLLCFGR_PLLP_POSICION)

// Posición del campo SW (fuente del reloj del sistema) en RCC_CFGR, bits 1:0. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_SW_POSICION 0U
// Máscara de los 2 bits de SW, ya desplazada a su posición. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_SW_MASCARA (0x3U << RCC_CFGR_SW_POSICION)
// Posición del campo SWS (fuente que el hardware está usando de verdad) en RCC_CFGR, bits 3:2. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_SWS_POSICION 2U
// Máscara de los 2 bits de SWS, ya desplazada a su posición. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_SWS_MASCARA (0x3U << RCC_CFGR_SWS_POSICION)
// Posición del campo HPRE (divisor del AHB) en RCC_CFGR, bits 7:4. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_HPRE_POSICION 4U
// Máscara de los 4 bits de HPRE, ya desplazada a su posición. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_HPRE_MASCARA (0xFU << RCC_CFGR_HPRE_POSICION)
// Posición del campo PPRE1 (divisor del APB1) en RCC_CFGR, bits 12:10. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_PPRE1_POSICION 10U
// Máscara de los 3 bits de PPRE1, ya desplazada a su posición. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_PPRE1_MASCARA (0x7U << RCC_CFGR_PPRE1_POSICION)
// Posición del campo PPRE2 (divisor del APB2) en RCC_CFGR, bits 15:13. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_PPRE2_POSICION 13U
// Máscara de los 3 bits de PPRE2, ya desplazada a su posición. Fuente: RM0390, sección 6.3.3.
#define RCC_CFGR_PPRE2_MASCARA (0x7U << RCC_CFGR_PPRE2_POSICION)

// Posición del campo LATENCY (estados de espera de la FLASH) en FLASH_ACR, bits 3:0. Fuente: RM0390, sección 3.8.1.
#define FLASH_ACR_LATENCY_POSICION 0U
// Máscara de los 4 bits de LATENCY, ya desplazada a su posición. Fuente: RM0390, sección 3.8.1.
#define FLASH_ACR_LATENCY_MASCARA (0xFU << FLASH_ACR_LATENCY_POSICION)

// Bit 0 de SysTick CTRL: 1 enciende el contador. Fuente: PM0214, sección SysTick (por verificar por Daniela).
#define SYSTICK_CTRL_ENABLE (1U << 0)
// Bit 1 de SysTick CTRL: 1 genera la interrupción al llegar el contador a 0. Fuente: PM0214, sección SysTick (por verificar por Daniela).
#define SYSTICK_CTRL_TICKINT (1U << 1)
// Bit 2 de SysTick CTRL: 1 usa el reloj del procesador, 0 usa AHB/8. Fuente: PM0214, sección SysTick (por verificar por Daniela).
#define SYSTICK_CTRL_CLKSOURCE (1U << 2)

#endif
```

Lo **nuevo** respecto al Paso 2:
- `GPIOB_BASE` = `AHB1_BASE + 0x400` = `0x40020400`. Los puertos están separados por `0x400` bytes: A en `0x40020000`, B en `0x40020400`, C en `0x40020800`.
- `GPIOB` es el puntero `(RegistrosGPIO *)GPIOB_BASE`. Se usa el mismo struct `RegistrosGPIO` porque todos los puertos tienen los mismos registros en los mismos desplazamientos.
- `RCC_AHB1ENR_GPIOBEN` = `1U << 1`: el bit 1 de `RCC->AHB1ENR` enciende el reloj del puerto B.
- El comentario de `AHB1ENR` ahora dice «bit 0 = puerto A, bit 1 = puerto B, bit 2 = puerto C».

`inc/gpio_config.h`

```c
#ifndef GPIO_CONFIG_H
#define GPIO_CONFIG_H

#include "registros_f446.h"

// Valores para los campos de configuración de un pin.
// Fuente: RM0390, sección 7.4, descripción de los registros GPIOx_MODER, GPIOx_PUPDR y GPIOx_OSPEEDR.
// Modo entrada: MODER = 00.
#define MODO_ENTRADA 0U
// Modo salida: MODER = 01.
#define MODO_SALIDA 1U
// Sin pull-up ni pull-down: PUPDR = 00.
#define RESISTENCIA_NINGUNA 0U
// Pull-up interno: PUPDR = 01 (el pin queda en 1 si nadie lo baja).
#define RESISTENCIA_PULL_UP 1U
// Velocidad baja: OSPEEDR = 00.
#define VELOCIDAD_BAJA 0U

// Datos para configurar un pin de una sola vez.
typedef struct {
    // Modo del pin: MODO_ENTRADA o MODO_SALIDA.
    uint32_t modo;
    // Número del pin dentro del puerto (0 a 15).
    uint32_t pin;
    // Resistencia interna: RESISTENCIA_NINGUNA o RESISTENCIA_PULL_UP.
    uint32_t resistencia;
    // Velocidad del pin: VELOCIDAD_BAJA.
    uint32_t velocidad;
    // Función alterna (AF0 a AF15). Usar 0U cuando el pin no es de función alterna.
    uint32_t funcion_alterna;
} ConfiguracionPin;

// Enciende el reloj del puerto indicado (puerto A, B o C).
void habilitar_reloj_puerto(RegistrosGPIO *puerto);
// Escribe en los registros del puerto la configuración de un pin.
void configurar_pin(RegistrosGPIO *puerto, ConfiguracionPin configuracion);
// Devuelve 1 si el pin está en 1 y 0 si está en 0.
uint32_t leer_pin(RegistrosGPIO *puerto, uint32_t pin);
// Pone el pin en 1 si estado es distinto de 0, o en 0 si estado es 0.
void escribir_pin(RegistrosGPIO *puerto, uint32_t pin, uint32_t estado);
// Cambia el pin de 0 a 1 o de 1 a 0.
void invertir_pin(RegistrosGPIO *puerto, uint32_t pin);

#endif
```

Solo cambió el comentario de `habilitar_reloj_puerto` (ahora «puerto A, B o C»). Las firmas y las constantes son las del Paso 1.

`src/gpio_config.c`

```c
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
```

Lo **nuevo**: el bloque `else if (puerto == GPIOB)`. Compara la dirección del puerto recibido con la de B y, si coincide, hace `RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN`. El `|=` conserva los relojes de A y C. Como `configurar_pin` llama a `habilitar_reloj_puerto` primero, el reloj de B se enciende solo al configurar el primer botón de ese puerto. El resto de la función se explicó en el Paso 1.

`main.c`

```c
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
```

- **Constantes `PIN_BOTON_A` a `PIN_BOTON_F`.** Son los números de pin dentro de su puerto: 10 (PA10), 3 (PB3), 5 (PB5), 4 (PB4), 10 (PB10) y 8 (PA8). Dos botones tienen el número 10, pero uno es del puerto A y otro del B; el pin solo se entiende junto con su puerto.
- **Un bloque por botón** (los seis son iguales y cambian el pin y el puerto):
  1. `ConfiguracionPin configuracion_boton_X;` crea el struct.
  2. `.modo = MODO_ENTRADA`: los 2 bits del pin en `MODER` quedan en `00`.
  3. `.pin = PIN_BOTON_X`.
  4. `.resistencia = RESISTENCIA_PULL_UP`: los 2 bits en `PUPDR` quedan en `01`.
  5. `.velocidad = VELOCIDAD_BAJA`: no importa en una entrada, pero `configurar_pin` escribe el campo.
  6. `.funcion_alterna = 0U`: AF0; en modo entrada no tiene efecto.
  7. `configurar_pin(GPIOA o GPIOB, ...)`. Botones **A y F** en `GPIOA`; **B, C, D y E** en `GPIOB`.
- **PB3 y PB4.** Tras el reset están en función alterna de depuración (el `revisor` comprobó que `MODER` vale `10`). Escribir `MODER = 00` los convierte en entrada. PB3 además no tiene resistencia tras el reset: sin esta configuración quedaría flotante. Con depuración SWD por ST-LINK, la depuración no necesita PB4 y solo usaría PB3 (SWO) si se activara el trace, algo que este proyecto no hace. Los comentarios de `main.c` dicen solo «función alterna de depuración» (decisión de Daniela); los nombres de las funciones (JTDO/TRACESWO y NJTRST) están por verificar en la tabla de funciones alternas del manual.
- **El bucle.** El `if` une las seis condiciones `leer_pin(...) == 0U` con `||`. Si **cualquiera** se cumple, el LED se enciende (`escribir_pin(GPIOA, PIN_LED, 1U)`); si no, el `else` lo apaga. El `||` se evalúa de izquierda a derecha y se detiene en la primera condición verdadera; no cambia el resultado. No hay espera: el bucle lee los botones lo más rápido que puede, por eso el LED responde de inmediato.
- `configurar_reloj()` e `iniciar_systick()` se siguen llamando aunque este paso no usa el tiempo, porque el reloj de 84 MHz es la base del proyecto y SysTick se usará en el paso 7.

### Rebote (no afecta en este paso)
Los contactos mecánicos de un botón no cambian de 1 a 0 de una vez: durante un tiempo corto (en general del orden de milisegundos; no se midió en este shield) el pin pasa varias veces entre 1 y 0 antes de estabilizarse. Esto se llama **rebote**. En este paso no importa porque el LED sigue el nivel del pin: si rebota unos milisegundos, el LED apenas parpadea. Sí importará en el paso 7, cuando una pulsación deba contar **una sola vez** (por ejemplo, para cambiar la dirección de la serpiente): ahí habrá que ignorar los cambios durante un tiempo corto (antirrebote), usando la base de tiempo de SysTick.

### Modificaciones y correcciones
- `registros_f446.h` y `gpio_config.*` crecen con el puerto B (ver arriba).
- `main.c` ya no hace parpadear el LED ni usa `esperar_milisegundos` (la versión del Paso 2 queda en su sección).
- El `programador` actualizó el comentario de `RegistrosRCC.AHB1ENR` para mencionar el bit 1; el `revisor` lo encontró correcto.
- Sin correcciones del `revisor`: no encontró errores claros.

### Decisiones tomadas
| Pregunta | Opciones | Decisión de Daniela |
|---|---|---|
| Qué botones se configuran | Los seis, o solo los cuatro de dirección | Los seis (A a F) |
| PB3 y PB4 | Entrada explícita con pull-up, o dejarlos como arrancan | Entrada explícita con pull-up |
| Cómo se prueban | LED, UART o LED con parpadeos | LED (se enciende con cualquiera de los seis) |
| USART | En este paso o en uno aparte | En un paso aparte |
| Dónde va el código de botones | `main.c` o `botones.h/.c` | Todo en `main.c` |
| Cómo se escribe la configuración | Seis bloques explícitos o arreglo con `for` | Seis bloques explícitos |
| Qué hace el LED | Encendido con cualquiera presionado, o se invierte con cada pulsación | Encendido mientras cualquiera esté presionado |
| Macros `PIN_BOTON_*` | Dejarlas o escribir los números dentro de cada bloque | Dejarlas (igual que `PIN_LED`) |
| Comentarios de PB3 y PB4 | Genéricos o nombrando JTDO/TRACESWO y NJTRST | Genéricos |

Decisión menor del agente principal que Daniela puede objetar: unir las seis lecturas con `||` en un solo `if`.

### Cómo se probó
- `make clean && make`: 0 advertencias; `arm-none-eabi-size`: `text 1826`, `data 0`, `bss 6`.
- El `revisor` comprobó en `stm32f446xx.h` (STM32CubeF4) que `GPIOB_BASE` y el bit `GPIOBEN` coinciden, y en el desensamblado que `habilitar_reloj_puerto` compara con `0x40020000`, `0x40020400` y `0x40020800` y que para B hace `orr #2` sobre `RCC + 0x30`. También revisó que `configurar_pin` deja PB3 y PB4 en `MODER = 00` y `PUPDR = 01`.
- `make flash`: voltaje 3,258 V, `device id = 0x10006421`, `flash size = 512 kbytes`, `Programming Finished`, `Verified OK`, `Resetting Target`.
- `nm`: `main = 0x080001c4`, `habilitar_reloj_puerto = 0x08000390`, `configurar_pin = 0x08000400`.
- En la placa (2026-11-09): **los seis botones (A a F) encienden el LED mientras se mantienen presionados y lo apagan al soltarlos.**

### Errores encontrados y cómo se resolvieron
- La primera grabación falló con `Error: open failed`: la placa había dejado de estar conectada a WSL. Se resolvió volviendo a conectarla con `usbipd attach --wsl --busid 1-5` desde PowerShell como administrador; la segunda grabación salió bien.
- Compilación: sin errores ni advertencias.

### Preguntas de autoevaluación
1. ¿Por qué los botones son activos en bajo y qué hace el pull-up interno?
   El botón conecta el pin a GND al presionarlo. El pull-up interno (`PUPDR = 01`) lleva el pin a 1 cuando el botón está suelto; sin él quedaría flotando. Por eso suelto = 1 y presionado = 0.
2. ¿Por qué PB3 y PB4 se configuran explícitamente como entrada?
   Porque tras el reset están en función alterna de depuración (`MODER = 10`) y PB3 no tiene resistencia. Escribir `MODER = 00` y el pull-up los deja como entradas con un estado conocido.
3. ¿Qué pasaría si `habilitar_reloj_puerto` no encendiera el reloj de GPIOB?
   Los registros del puerto B no responderían: la configuración de los botones B, C, D y E se ignoraría, y esos pines no funcionarían como entradas con pull-up.
4. ¿Por qué el `if` del bucle usa `||` y qué pasa si se presionan dos botones a la vez?
   Porque basta con que uno cumpla `leer_pin(...) == 0U` para encender el LED. Con dos botones presionados el LED sigue encendido, porque la condición sigue siendo verdadera.
5. ¿Qué es el rebote y por qué no afecta a este paso pero sí al paso 7?
   Es el cambio repetido entre 1 y 0 que hace el contacto del botón durante unos milisegundos al presionarlo. Aquí el LED sigue el nivel del pin y no se nota; en el paso 7 una pulsación debe contar una sola vez, así que habrá que ignorar los cambios durante un tiempo corto.

## Resumen de sustentación
Al final del proyecto: mapa de memoria, tabla de pines, mapeo de píxeles, cálculo del reloj y de los timers, y diagrama del flujo del juego.
