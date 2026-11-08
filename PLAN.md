# Plan paso a paso — Snake en panel P10 RGB (NUCLEO-F446RE, bare metal)

Cada paso termina con una prueba visible y con preguntas que debes poder contestar. No se pasa al siguiente paso hasta que el actual compile, funcione en la placa y lo puedas explicar. Los pasos 1 a 3 son para tener la base; la pantalla empieza en el paso 4. El juego completo se hace primero en un solo panel de 32x16, y el segundo panel se agrega en el paso 11.

## Reglas de código (para Daniela y para Claude Code)

1. **Claude no toma decisiones.** Su trabajo es programar lo que Daniela decide y explicarlo. Cuando haya más de una forma de hacer algo (un pin, un valor, una estructura, un nombre, un orden), o algo que no sepa o no esté seguro, debe detenerse y preguntar, con las opciones y su diferencia. No debe suponer, inventar valores ni elegir por su cuenta. Si un dato sale del manual de referencia (RM0390) o de los ejemplos del curso, debe decir en qué sección o archivo lo encontró.
2. **No usar nada que no se requiera.** Ni librerías, ni funciones, ni periféricos, ni macros, ni archivos, ni opciones del Makefile que el paso actual no necesite. Si cree que algo extra haría falta, lo pregunta antes.
3. **Ningún driver que no sea nuestro.** La pantalla y el control se manejan escribiendo registros directamente (GPIO, RCC, TIM, SysTick, NVIC, USART). No se usa HAL, CMSIS, ni librerías de pantallas HUB75 o de joysticks. Solo se aceptan librerías de dispositivos externos, y aquí no hay ninguna.
4. **Explicar todo el código**, línea por línea, bloque por bloque y parte por parte: qué hace cada línea, qué registro y qué bits toca, por qué ese valor, y cómo encaja el bloque en el programa. Lo explica antes de escribir (qué va a hacer y por qué) y después de escribir (el código ya escrito, sin saltarse nada).
5. Código lo más simple posible: funciones cortas, una idea por función, sin punteros a funciones, sin macros nuevas, sin trucos de una línea.
6. Nombres completos y claros, sin abreviaturas (`encender_pixel`, no `ep`). Cada registro o bit se escribe en una línea aparte con un comentario que diga qué hace y por qué.
7. Solo conceptos vistos en clase: registros por dirección, structs de registros, `volatile`, SysTick, interrupciones con NVIC, timers, GPIO, UART. Si hay que usar algo nuevo, se explica y se pregunta antes de escribirlo.
8. Cada paso compila sin advertencias y se prueba en la placa antes del siguiente. Daniela confirma que entendió antes de pasar al siguiente paso.
9. Todo uso de IA se anota en `AI_USO.md` en el mismo commit en que entra el código.
10. **Flujo con subagentes.** El código lo escribe el subagente `programador` (modelo barato) y lo revisa y corrige el subagente `revisor`; los dos están definidos en `.claude/agents/`. Como los subagentes no pueden preguntarle a Daniela, el agente principal resuelve con ella todas las decisiones antes de delegar y es quien le explica el código. Detalle en `CLAUDE.md`.
11. **Guía de estudio obligatoria.** Claude mantiene `docs/GUIA_DE_ESTUDIO.md`, un documento para que Daniela y su compañero estudien. Cada paso, modificación, archivo, bloque de código y corrección se explica ahí, en el mismo commit en que entra el código. No se da un paso por terminado si su sección de la guía no está escrita (ver "Guía de estudio" más abajo).

## Estructura del repositorio

```
CLAUDE.md             contexto mínimo para Claude Code
AI_USO.md             registro del uso de IA (obligatorio)
README.md             cómo compilar, grabar y conectar (obligatorio)
Makefile
linker_script.ld
crt0.S                setup en ensamblador, como el del profesor
main.c
inc/   registros_f446.h  gpio_config.h  system_config.h  usart.h  pantalla.h  juego.h
src/   gpio_config.c  system_config.c  usart.c  pantalla.c  juego.c  stub_stdlib.c
docs/  GUIA_DE_ESTUDIO.md  conexiones.md  mapeo_pixeles.md  informe/
```

## Guía de estudio (`docs/GUIA_DE_ESTUDIO.md`)

Es el documento con el que Daniela y su compañero estudian para la sustentación. Lo escribe Claude Code en español, con lenguaje sencillo, y se actualiza en cada paso, en el mismo commit que el código. Estructura:

1. **Índice** con un enlace a cada sección.
2. **Glosario** corto: cada término nuevo (registro, máscara, vector de interrupción, latch, escaneo 1/4, etc.) se define la primera vez que aparece, con una frase.
3. **Una sección por paso** (`## Paso N — título`). Cada sección tiene, en este orden:
   - **Objetivo:** qué se quiere lograr y por qué.
   - **Archivos creados o modificados:** lista con la ruta de cada uno.
   - **Concepto de clase que se usa:** qué se vio en clase y a qué ejemplo del profesor corresponde.
   - **Código explicado:** cada bloque de código copiado tal como quedó en el repo, seguido de su explicación línea por línea (qué hace, qué registro y qué bits toca, por qué ese valor, de qué sección del RM0390 o de qué ejemplo salió). Si algo viene del profesor, se dice cuál ejemplo y qué se cambió.
   - **Modificaciones y correcciones:** cada cambio respecto a la versión anterior o respecto al código del profesor (por ejemplo, la corrección de `WRITE_REG_FIELD`), con el motivo y qué fallaría sin ese cambio.
   - **Decisiones tomadas:** cada pregunta que Claude hizo, las opciones que había y lo que decidió Daniela. Esto sirve para defender las decisiones de diseño.
   - **Cómo se probó:** comandos, resultado esperado y resultado real.
   - **Errores encontrados y cómo se resolvieron.**
   - **Preguntas de autoevaluación:** 3 a 5 preguntas que el profesor podría hacer sobre este paso, con su respuesta corta.
4. **Resumen de sustentación** al final: los diagramas o tablas más importantes (mapa de memoria, tabla de pines, mapeo de píxeles, cálculo de reloj y timers, flujo del juego).

Reglas de la guía: el código que aparece en ella debe ser idéntico al del repo; no se resumen bloques con "..."; no se omite ninguna línea; y se actualiza también cuando un paso posterior cambia un archivo anterior.

## Qué se copia del profesor y qué se escribe nuevo

| Pieza | Origen | Cambios |
|---|---|---|
| `crt0.S` | ejemplo NUCLEO-64/24-ADC-1 | quitar `bl SystemInit` (viene de CMSIS), completar la tabla de vectores con las interrupciones de la F446 (manual RM0390) |
| `linker_script.ld` | ejemplo 24-ADC-1 | RAM de 128 KB (`LENGTH = 128K`); FLASH igual (512 KB en 0x08000000) |
| `Makefile` | ejemplo 24-ADC-1 | quitar rutas de CMSIS, sin `-DSTM32F401xE`, agregar `-mthumb`; target `flash` con openocd o st-flash |
| `registros_f446.h` | ejemplo 4-custom-hal (`stm32f401.h`) | renombrar; no se usa CMSIS. Agregar solo los registros que el paso necesite (GPIO, RCC, FLASH, SysTick, NVIC, TIM3, USART2), con sus direcciones tomadas del manual RM0390 |
| `gpio_config.c/.h` | ejemplos del profesor (los de pines) | copiar tal cual, con la corrección de abajo |
| `system_config.c/.h` | ejemplo 7-config-clock / 24-ADC-1 | copiar: reloj a 84 MHz, SysTick de 1 ms, `SysTick_Delay_ms`. Corrección de abajo |
| `usart.c/.h` | ejemplo 14 a 17 (USART2, PA2/PA3) | solo para imprimir mensajes de depuración; opcional pero muy útil |
| `stub_stdlib.c` | ejemplo del profesor | copiar |
| `pantalla.c/.h` | nuevo | driver del panel HUB75 |
| `juego.c/.h`, `main.c` | nuevo | lógica del Snake |

Decisión tomada: no se usa el header CMSIS de ST (`stm32f4xx.h`) ni ningún otro driver de periféricos. Los ejemplos 6 en adelante del curso dependen de él, así que lo que se copia de ellos (`gpio_config`, `system_config`, `usart`) hay que adaptarlo para que use `registros_f446.h` en lugar de los nombres de CMSIS. Esa adaptación se hace con Claude Code una función a la vez, y cada cambio de nombre o de máscara se explica. El ejemplo 4-custom-hal es el modelo de cómo se definen los registros a mano.

## Cosas que el repo del compañero ya descubrió (no hay que redescubrirlas)

- **Error en `WRITE_REG_FIELD` del profesor.** Está escrita con `~MASK` sin paréntesis. Cuando la máscara es `Mascara << (pin * 2)`, C calcula `(~Mascara) << (pin * 2)` y borra la configuración de los pines más bajos del puerto. La versión correcta usa `~(MASK)`. Al copiar el código del profesor, Claude debe usar la versión corregida (con paréntesis en `MASK`, `REG` y `VAL`), avisar de cada lugar donde aparecía la macro y explicar por qué se corrige. Si la macro no hace falta en algún punto, se prefiere una escritura directa al registro más fácil de explicar, previa pregunta a Daniela.
- **PB3 y PB4** arrancan en función alterna de depuración (SWO y JTAG). Hay que configurarlos como entrada de forma explícita.
- **TIM3 en APB1**: si el divisor de APB1 no es 1, el reloj de los timers APB1 es el doble del bus. Con el reloj del curso, APB1 = 42 MHz y TIM3 recibe 84 MHz. Los cálculos de PSC y ARR se hacen con 84 MHz.
- **Mapeo de píxeles del panel** ya medido (ver paso 5 y `docs/mapeo_pixeles.md`).

## Datos de hardware que no cambian

- Panel: HUB75, 32x16 por módulo (32 columnas, 16 filas), escaneo 1/4 (señales A y B), R1 G1 B1 (filas 0-7) y R2 G2 B2 (filas 8-15).
- **Etapa 1 (ahora): un solo panel**, 32x16 = 512 píxeles. **Etapa 2 (paso 11): segundo panel**, 32x32 = 1024 píxeles. Todo el código se escribe pensando en que el tamaño del tablero está en constantes, para que la ampliación sea un cambio pequeño.
- Pines: R1=PC0, G1=PC1, B1=PC2, R2=PC3, G2=PC4, B2=PC5, OE=PC6, LAT=PC7, CLK=PC8, A=PC10, B=PC11, GND común.
- Botones (activos en bajo, pull-up interno): A=PA10, B=PB3, C=PB5, D=PB4, E=PB10, F=PA8.
- LED de la placa: PA5. Botón azul: PC13. UART de depuración: USART2 en PA2/PA3, 115200.
- Alimentación del panel: 5 V externos con GND común. Nunca pantalla completa en blanco con brillo alto.

## Reglas del juego

- Serpiente verde, manzana roja y veneno azul.
- Manzana: suma 1 punto y alarga la serpiente 1 segmento.
- Veneno: resta 2 puntos y quita 2 segmentos de la serpiente. Aparece, espera un tiempo y luego desaparece solo.
- Sin límite de longitud de la serpiente. El puntaje se muestra al terminar.
- Pendiente de decidir con Daniela en el paso 9: duración del veneno, cada cuánto reaparece, qué pasa si el puntaje es menor que 2 y qué pasa si la serpiente tiene 2 segmentos o menos.

## Pasos

### Paso 0 — Repositorio y archivos de contexto
- Crear el repo, agregar los dos compañeros como colaboradores, `.gitignore` (`*.o`, `*.elf`, `*.bin`, `build/`) y la estructura de carpetas.
- Poner `CLAUDE.md` y crear `AI_USO.md` vacío con su formato (fecha, herramienta, para qué, qué código se generó).
- Crear `docs/GUIA_DE_ESTUDIO.md` con el índice, el glosario vacío y la plantilla de sección de paso (ver "Guía de estudio"). Claude propone la plantilla y Daniela la aprueba.
- Prueba: `git push` y ver el repo en GitHub.
- Debes poder explicar: por qué la regla 10 del profesor exige documentar la IA.

### Paso 1 — Compilar y grabar (linker, Makefile, crt0.S)
- Copiar y adaptar `crt0.S`, `linker_script.ld`, `Makefile`, `registros_f446.h`, `gpio_config.*`, `stub_stdlib.c`.
- `main.c` mínimo: encender el LED PA5 y leer el botón PC13 (como el ejemplo 4).
- Prueba: `make` sin errores, `make flash`, el LED responde al botón.
- Guía de estudio: sección con `crt0.S`, linker y Makefile explicados línea por línea, y con la lista de qué se cambió respecto al ejemplo del profesor.
- Debes poder explicar: qué hace cada línea de `crt0.S` (pila, copia de `.data`, limpieza de `.bss`, salto a `main`), qué es la tabla de vectores, cada sección del linker, cada línea del Makefile, y por qué existen `.data` y `.bss`.

### Paso 2 — Reloj y tiempo
- Copiar `system_config.*` con la macro corregida. Reloj a 84 MHz, SysTick con interrupción cada 1 ms, `SysTick_Delay_ms`.
- Prueba: el LED parpadea a exactamente 1 Hz (medirlo con el celular o con el reloj).
- Debes poder explicar: cálculo del PLL (16 MHz / 16 × 336 / 4 = 84 MHz), latencia de flash, valor de `LOAD` de SysTick, qué pasa en la interrupción.

### Paso 3 — Botones y UART de depuración
- Configurar los seis botones como entrada con pull-up. Copiar `usart.*` si se quiere depurar.
- Prueba: al presionar cada botón se imprime su letra por UART, o se enciende el LED.
- Debes poder explicar: por qué los botones son activos en bajo, qué es un pull-up interno, rebote.

### Paso 4 — Primer píxel sin interrupciones
- Configurar los 11 pines del panel como salida. Escribir una función que, con GPIO normal, desplace los datos de una sola fase y haga el latch.
- Prueba: se enciende un píxel o una fila conocida, con brillo bajo.
- Debes poder explicar: la secuencia desplazar, apagar OE, pulso de LAT, fijar A y B, encender OE, y por qué el latch va después del desplazamiento.

### Paso 5 — Refresco con interrupción de timer
- Crear el framebuffer (un arreglo con los colores de cada píxel). Configurar TIM3 para que interrumpa cada ~500 µs a 1 ms; en cada interrupción se dibuja una fase (0 a 3).
- Función `encender_pixel(x, y, color)` que aplica el mapeo medido:
  bloque = bit / 16, j = bit % 16; si j < 8: x = 8·bloque + (7 − j), y = fase; si j >= 8: x = 8·bloque + (j − 8), y = fase + 4; las salidas R2/G2/B2 suman 8 a y.
  Primero comprobar `docs/mapeo_pixeles.md` (el MAPEO.md del repo anterior) y usar la fórmula inversa que ahí está.
- Prueba: llenar de rojo, verde y azul; dibujar un marco y las diagonales.
- Debes poder explicar: frecuencia de refresco, por qué solo 1/4 de los LED está encendido a la vez, `volatile` en el framebuffer, qué hace la interrupción y qué hace `main`.

### Paso 6 — Biblioteca gráfica mínima
- Funciones: borrar pantalla, poner un píxel con 7 colores, dibujar un rectángulo, dibujar un dígito (fuente propia de 3x5).
- Prueba: mostrar números 0 al 9 y un marco.
- Debes poder explicar: cómo se guarda un color en 3 bits (R, G, B).

### Paso 7 — Mover un punto con los botones
- Definir el tamaño del tablero en un solo lugar (dos constantes: ancho y alto). Ahora son 32 columnas y 16 filas. Un punto se mueve cada cierto tiempo (temporizador de software con SysTick) hacia la dirección elegida con los botones. No puede salir del borde.
- Prueba: A, B, C y D mueven el punto; E pausa; F reinicia.
- Debes poder explicar: por qué el tiempo del juego se separa del refresco de la pantalla, y por qué el tamaño del tablero está en constantes.

### Paso 8 — Snake básico (primer avance para mostrar al profesor)
- Serpiente verde que se mueve con los botones. Cuerpo en un arreglo circular con tantas posiciones como píxeles tiene el tablero (512 con un panel, 1024 con dos), cabeza y cola. No poder girar 180°; choque con paredes y con el propio cuerpo = fin del juego.
- Manzana roja en una celda libre; al comerla, la serpiente crece 1 segmento (no se quita la cola) y el puntaje sube 1.
- **Pendiente de decidir con Daniela al empezar el paso 8:** cómo se elige la posición de la manzana, porque el generador aleatorio (LFSR) llega en el paso 9. Opciones: (a) calcularla con el contador de milisegundos de SysTick, (b) una lista fija de posiciones que se repite, (c) adelantar el LFSR a este paso.
- Prueba: la serpiente verde se mueve con los botones, crece al comer una manzana roja y muere al chocar.
- Debes poder explicar: el arreglo circular (índices de cabeza y cola), y por qué no hay límite de longitud.

### Paso 9 — Números aleatorios y veneno azul
- Generador pseudoaleatorio sencillo (LFSR) sembrado con el valor de un timer cuando se presiona el primer botón. Se usa para la posición de la manzana y del veneno.
- Veneno azul: al comerlo, el puntaje baja 2 y la serpiente pierde 2 segmentos. Aparece, espera un tiempo y desaparece solo.
- Antes de programar, Claude pregunta a Daniela: (1) cuánto dura el veneno en pantalla, (2) cada cuánto reaparece, (3) qué pasa si el puntaje es menor que 2 al comer veneno, (4) qué pasa si la serpiente tiene 2 segmentos o menos.
- Prueba: manzana y veneno aparecen en celdas libres; el veneno desaparece solo y, al comerlo, resta puntos y acorta la serpiente.
- Debes poder explicar: cómo funciona el LFSR y por qué hace falta sembrarlo con algo que cambie.

### Paso 10 — Puntaje y fin de juego
- Contador de puntos mostrado al terminar con la fuente 3x5 (hasta 4 dígitos; caben en 16 columnas). Pantalla de fin de juego y reinicio.
- El puntaje puede bajar con el veneno; cómo se muestra si queda negativo o en 0 se decide en el paso 9, junto con la regla del puntaje menor que 2.
- Prueba: al morir se ve el puntaje correcto y F reinicia.
- Hito: el juego completo funciona en un solo panel de 32x16. Con esto se compra el segundo panel.

### Paso 11 — Ampliación a 32x32 con el segundo panel
- Antes: comprar el segundo panel con tiempo, porque el proyecto debe presentarse en 32x32. Conectar el OUT del panel 1 con el IN del panel 2 con otro cable plano, y alimentar ambos con 5 V y GND común (ver la fuente en `docs/conexiones.md`; revisar también que las filas del conector queden bien, como en el panel 1).
- Cada fase pasa a ser de 128 bits. Medir el orden del segundo panel con un píxel que camina y actualizar `docs/mapeo_pixeles.md` y el driver.
- Cambiar el alto del tablero de 16 a 32 (la constante del paso 7) y el tamaño del arreglo circular a 1024.
- Prueba: marco en los 32x32, diagonales completas y el juego completo en los dos paneles.
- Debes poder explicar: por qué dos paneles encadenados se ven como uno de 64x16 y cómo se reordena a 32x32, y qué partes del código no tuvieron que cambiar.

### Paso 12 — Pulido (solo si hay tiempo)
- Niveles de velocidad, brillo con PWM en OE, pausa, doble buffer para evitar parpadeos entre el juego y la interrupción.
- Debes poder explicar: la condición de carrera entre la interrupción y el juego, y cómo la evita el doble buffer.

### Paso 13 — Entrega
- Revisar que `docs/GUIA_DE_ESTUDIO.md` esté completa (todas las secciones, glosario y resumen de sustentación) y que cada integrante la haya leído y contestado las preguntas de autoevaluación.
- `README.md` con instrucciones para reproducir (conexiones, comandos, versión de la toolchain).
- Informe IEEE de 6 páginas: introducción, solución y metodología, montaje experimental, resultados, conclusiones y máximo 10 referencias.
- Presentación de 10 minutos + 5 de preguntas. Ensayar la explicación de linker, Makefile y crt0.S.
- Subir el repo el día anterior a la evaluación. Los dos estudiantes como colaboradores.

## Lista rápida de preguntas de sustentación
- Qué hace `crt0.S` línea por línea y para qué sirve la tabla de vectores.
- Qué hay en el linker y por qué `.data` se copia de flash a RAM.
- Cómo se calcula el reloj de 84 MHz y los valores de PSC y ARR de TIM3.
- Escaneo 1/4: por qué A y B, qué es un latch, qué hace OE.
- Cálculo de la frecuencia de refresco y de la corriente que consume el panel.
- Para qué sirve `volatile`; condiciones de carrera ISR vs `main`.
- Qué IA se usó, para qué y qué parte del código la generó.
