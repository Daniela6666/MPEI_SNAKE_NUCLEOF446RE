# Guía de estudio — Snake en panel P10 RGB (NUCLEO-F446RE)

Documento para que Daniela y su compañero estudien para la sustentación. Lo mantiene Claude Code: cada paso, modificación, archivo y bloque de código se explica aquí, en el mismo commit en que entra el código. El código que aparece debe ser idéntico al del repositorio, sin "..." ni líneas omitidas.

**Estado de la plantilla:** propuesta por Claude, pendiente de aprobación de Daniela (paso 0).

## Índice
- [Glosario](#glosario)
- [Paso 0 — Repositorio y documentos](#paso-0--repositorio-y-documentos)
- Los pasos siguientes se agregan aquí a medida que se hacen (ver `PLAN.md`).
- [Resumen de sustentación](#resumen-de-sustentación)

## Glosario
Cada término se define la primera vez que aparece, en una frase.

| Término | Significado |
|---|---|
| (se llena al avanzar) | |

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

## Resumen de sustentación
Al final del proyecto: mapa de memoria, tabla de pines, mapeo de píxeles, cálculo del reloj y de los timers, y diagrama del flujo del juego.
