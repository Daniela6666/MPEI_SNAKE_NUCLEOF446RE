# Snake en panel P10 RGB — NUCLEO-F446RE (bare metal)

Proyecto final de Microprocesadores e Interfaces (Ingeniería Electrónica, MPEI 2026-2). Juego Snake en paneles P10 RGB de 32x16: primero un panel (32x16) y, cuando el juego funcione, dos paneles encadenados (32x32), controlado con los botones de un Joystick Shield V1.A, programado sin HAL ni CMSIS: todos los periféricos se manejan escribiendo registros.

Integrantes: Daniela y Daniel (completar con nombres completos).

## Reglas del juego
- La serpiente crece al comer comida.
- Se achica al comer veneno.
- No hay límite de longitud (cabe hasta en todos los píxeles del tablero: 512 con un panel, 1024 con dos).
- Al terminar el juego se muestra el puntaje.

## Hardware
- Placa NUCLEO-F446RE (Cortex-M4).
- Módulos P10 RGB exterior 32x16, HUB75, escaneo 1/4, alimentados con 5 V externos. Ahora hay 1; el segundo se agrega en el paso 11.
- Joystick Shield V1.A (solo se usan los botones).

Conexiones completas en [`docs/conexiones.md`](docs/conexiones.md).

## Estructura
```
CLAUDE.md          contexto y reglas para Claude Code
PLAN.md            plan paso a paso
AI_USO.md          registro del uso de IA (requisito del curso)
docs/              guía de estudio, conexiones y mapeo de píxeles
inc/  src/         código fuente (se llena a partir del paso 1)
```

## Cómo compilar y grabar
Se completa en el paso 1, cuando existan el `Makefile`, el linker y `crt0.S`. Requisitos previstos: `arm-none-eabi-gcc`, `make` y `openocd` (o `st-flash`) con el ST-LINK de la placa.

## Estado
Paso 0 (repositorio y documentos) hecho. Siguiente: paso 1, ver [`PLAN.md`](PLAN.md).
