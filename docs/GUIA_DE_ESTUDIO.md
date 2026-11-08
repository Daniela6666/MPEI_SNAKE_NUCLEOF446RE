# Guía de estudio — Snake en panel P10 RGB (NUCLEO-F446RE)

Documento para que Daniela y su compañero estudien para la sustentación. Lo mantiene Claude Code: cada paso, modificación, archivo y bloque de código se explica aquí, en el mismo commit en que entra el código. El código que aparece debe ser idéntico al del repositorio, sin "..." ni líneas omitidas.

**Estado de la plantilla:** propuesta por Claude, pendiente de aprobación de Daniela (paso 0).

## Índice
- [Glosario](#glosario)
- [Paso 0 — Repositorio y documentos](#paso-0--repositorio-y-documentos)
- [Paso 1 — Compilar y grabar (linker, Makefile, crt0.S)](#paso-1--compilar-y-grabar-linker-makefile-crt0s)
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

Este paso se hace en cuatro piezas, una a una: Makefile, linker script, `crt0.S` y programación de la placa con el LED y el botón. Aquí están documentadas la **pieza 1 (Makefile)**, la **pieza 2 (linker script)** y la **pieza 3 (`crt0.S`)**. Las otras piezas se agregan a esta sección cuando se hagan.

### Objetivo
Tener un Makefile propio, lo más simple posible, que compile el proyecto con `arm-none-eabi-gcc` para el Cortex-M4 y grabe la placa con `openocd`.

### Archivos creados o modificados
- `Makefile` (pieza 1).
- `linker_script.ld` (pieza 2).
- `crt0.S` (pieza 3).

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

## Resumen de sustentación
Al final del proyecto: mapa de memoria, tabla de pines, mapeo de píxeles, cálculo del reloj y de los timers, y diagrama del flujo del juego.
