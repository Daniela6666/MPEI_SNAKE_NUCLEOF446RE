# Mapeo de píxeles del panel P10 RGB 32x16

Resumen del mapeo medido en el repositorio anterior (archivo `MAPEO.md` de la carpeta `PRUEBA PANTALLA`). **Pendiente de verificar en este repositorio** con el paso 5: el fabricante puede cambiar el orden de los píxeles entre módulos.

## Cómo se envía una fase
- El panel es de escaneo 1/4: en cada fase (0 a 3, escogida con A y B) se encienden las filas `fase`, `fase + 4`, `fase + 8` y `fase + 12`.
- Por cada fase se desplazan 64 bits (por módulo), en 4 bloques de 16.
- Cada bit lleva 6 datos a la vez: R1 G1 B1 para la mitad de arriba (filas 0 a 7) y R2 G2 B2 para la de abajo (filas 8 a 15).

## Fórmula medida (bit → x, y)
```
bloque = bit / 16
j      = bit % 16

si j < 8:   x = 8 * bloque + (7 - j)     y = fase          (de derecha a izquierda)
si j >= 8:  x = 8 * bloque + (j - 8)     y = fase + 4      (de izquierda a derecha)

Para R2, G2 y B2 se suma 8 a y.
```
La fórmula inversa (de x, y a fase y bit) se deduce de esta y se documenta en el paso 5.

## Dos paneles
Al encadenar el segundo panel cada fase pasa a ser de 128 bits. El orden del segundo panel se mide en el paso 7 con un píxel que camina.
