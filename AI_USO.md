# Registro de uso de IA

Requisito 10 del curso: todo uso de IA debe quedar documentado (herramienta, propósito y qué código se generó). Cada entrada se agrega en el mismo commit en que entra el código o documento.

| # | Fecha | Herramienta | Para qué se usó | Código o documento generado | Revisión propia |
|---|---|---|---|---|---|
| 1 | 2026-10-19 a 2026-10-22 | Claude (chat) y Claude Code | Elegir la conexión del panel HUB75, diagnosticar por qué no encendía (filas del conector invertidas) y medir el mapeo de píxeles, en un repositorio anterior | Programas de prueba de pantalla (`PRUEBA PANTALLA`, repositorio anterior). No se reutiliza su código en este repositorio | Pendiente de completar |
| 2 | 2026-11-08 | Claude (Cowork) | Analizar los ejemplos del profesor (carpeta NUCLEO-64), leer los PDFs del curso y armar el plan paso a paso | `PLAN.md`, `CLAUDE.md`, `README.md` y los documentos de `docs/`. No se generó código de firmware | Pendiente de completar |
| 3 | 2026-11-08 | Claude Code (Sonnet 5.5) | Paso 1, pieza 1: revisar el entorno WSL2, probar el ejemplo `1-led-blink` del profesor como prueba de entorno y adaptar su Makefile a nuestro proyecto | `Makefile` y la sección del paso 1 de `docs/GUIA_DE_ESTUDIO.md`. Lo escribió y lo revisó el agente principal (Sonnet 5.5); no se usaron los subagentes `programador` ni `revisor` en esta pieza. Corrección: se quitaron `-fno-pic` y `--no-relax` tras comprobar que compila sin ellos | Pendiente de completar |
| 4 | 2026-11-08 | Claude Code (Sonnet 5.5) | Paso 1, pieza 2: adaptar el linker script del ejemplo 24-ADC-1 (RAM de 128K, sin libc ni C++, con `ALIGN(4)`), verificar la RAM en el RM0390 y revisar el mapa de memoria con `size`, `objdump` y `nm` | `linker_script.ld` y la pieza 2 de la sección del paso 1 de `docs/GUIA_DE_ESTUDIO.md`. Lo escribió y lo revisó el agente principal (Sonnet 5.5); no se usaron los subagentes `programador` ni `revisor`. Correcciones: se quitó la sección `.stack` del profesor y símbolos no usados | Pendiente de completar |
| 5 | 2026-11-08 | Claude Code (Sonnet 5.5) | Paso 1, pieza 3: adaptar el `crt0.S` (startup del F401 de ST, vía el ejemplo 24-ADC-1) a la F446: tabla de vectores completa según la Table 38 del RM0390, sin `SystemInit` ni `__libc_init_array`, y verificar que cada vector apunta a su manejador | `crt0.S` y la pieza 3 de la sección del paso 1 de `docs/GUIA_DE_ESTUDIO.md`. La tabla y los alias se generaron con un script (no incluido en el repo) a partir de la Table 38, consultada en una transcripción en línea del RM0390. Lo escribió y lo revisó el agente principal (Sonnet 5.5); no se usaron los subagentes `programador` ni `revisor` (el subagente `web-fetch` solo leyó la página con la tabla). Correcciones: tabla de vectores del F401 rehecha para la F446 | Pendiente de completar |

Desde el paso 1 cada entrada indica además qué modelo escribió el código (`programador`), qué modelo lo revisó (`revisor`) y qué correcciones hizo.

## Cómo se completa "Revisión propia"
Por cada entrada, anotar qué se revisó, qué se entendió, qué se cambió y qué dudas quedaron. Es lo que el profesor va a preguntar en la sustentación.
