# Conexiones

Estos datos vienen de las pruebas del repositorio anterior, donde el panel funcionó con este cableado. Verificar con el multímetro antes de energizar.

## Panel P10 RGB (conector HUB75 IN del panel 1)

| Pin HUB75 | Señal | Pin de la Nucleo |
|---|---|---|
| 1 | R1 | PC0 |
| 2 | G1 | PC1 |
| 3 | B1 | PC2 |
| 5 | R2 | PC3 |
| 6 | G2 | PC4 |
| 7 | B2 | PC5 |
| 9 | A | PC10 |
| 10 | B | PC11 |
| 13 | CLK | PC8 |
| 14 | LAT | PC7 |
| 15 | OE | PC6 |
| 16 | GND | GND |
| 4, 8, 11, 12 | GND / C / D | sin conectar |

Cuidado con las filas del conector: los pines impares y pares están en filas distintas. En una prueba anterior los cables quedaron en la fila contraria, y B1, B2 y OE cayeron en GND (pines 4, 8 y 16). Con el cable enchufado y el panel apagado, ningún cable de señal debe dar continuidad con GND, solo el del pin 16.

Panel 2: el OUT del panel 1 va al IN del panel 2 con otro cable plano.

## Alimentación del panel
- 5 V externos a los cables rojo (+5 V) y negro (GND) de cada panel (hay dos rojos y dos negros por panel, van en paralelo).
- El GND de la fuente se une al GND de la Nucleo.
- No alimentar los paneles desde el pin 5 V de la Nucleo.
- Consumo: unos 3 A máximos por panel con todo en blanco. Con un solo color por objeto, una pantalla llena consume unos 2 A. Fuente recomendada: 5 V / 5 A o más.
- Mientras se prueba: un solo panel, pocos píxeles y brillo bajo.

## Botones del Joystick Shield V1.A
Activos en bajo, con pull-up interno de la Nucleo.

| Botón | Pin de la Nucleo | Pin Arduino |
|---|---|---|
| A | PA10 | D2 |
| B | PB3 | D3 |
| C | PB5 | D4 |
| D | PB4 | D5 |
| E | PB10 | D6 |
| F | PA8 | D7 |
| K (botón del stick) | PA9 | D8 (no se usa) |

El shield está conectado con jumpers al header de la Nucleo, no enchufado. La G del shield debe ir a GND de la Nucleo; la V no es necesaria, porque se usa el pull-up interno. Los ejes X e Y no se usan.

## Otros pines de la placa
- LED de usuario LD2: PA5.
- Botón azul: PC13.
- UART de depuración: USART2 en PA2 (TX) y PA3 (RX), 115200 baudios, por el puerto virtual del ST-LINK.
