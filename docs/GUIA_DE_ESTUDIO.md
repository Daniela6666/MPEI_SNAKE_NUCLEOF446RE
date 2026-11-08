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

Este paso se hace en cuatro piezas, una a una: Makefile, linker script, `crt0.S` y programación de la placa con el LED y el botón. Aquí está documentada la **pieza 1 (Makefile)**. Las otras piezas se agregan a esta sección cuando se hagan.

### Objetivo
Tener un Makefile propio, lo más simple posible, que compile el proyecto con `arm-none-eabi-gcc` para el Cortex-M4 y grabe la placa con `openocd`.

### Archivos creados o modificados
- `Makefile` (pieza 1).

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

## Resumen de sustentación
Al final del proyecto: mapa de memoria, tabla de pines, mapeo de píxeles, cálculo del reloj y de los timers, y diagrama del flujo del juego.
