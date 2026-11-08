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

Este paso se hace en cuatro piezas, una a una: Makefile, linker script, `crt0.S` y programación de la placa con el LED y el botón. Aquí están documentadas la **pieza 1 (Makefile)** y la **pieza 2 (linker script)**. Las otras piezas se agregan a esta sección cuando se hagan.

### Objetivo
Tener un Makefile propio, lo más simple posible, que compile el proyecto con `arm-none-eabi-gcc` para el Cortex-M4 y grabe la placa con `openocd`.

### Archivos creados o modificados
- `Makefile` (pieza 1).
- `linker_script.ld` (pieza 2).

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

## Resumen de sustentación
Al final del proyecto: mapa de memoria, tabla de pines, mapeo de píxeles, cálculo del reloj y de los timers, y diagrama del flujo del juego.
