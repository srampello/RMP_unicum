# Distribución de pines · ESP32 WROOM clásico

Los números son **GPIO**, no posiciones del conector de la placa. Se presupone
una placa de desarrollo basada en WROOM/ESP32S sin PSRAM que expone estos GPIO.
Confirmar su serigrafía y pinout antes de soldar. No aplicar este mapa a C3, S3,
S2, ESP32-CAM ni WROVER; GPIO16/17 pueden estar ocupados en otros módulos.

## Sensores, izquierda a derecha mirando hacia el avance

| Sensor | GPIO | Unidad/canal | Alias frecuente |
| --- | --- | --- | --- |
| S01 | 36 | ADC1_CH0 | VP / SVP |
| S02 | 39 | ADC1_CH3 | VN / SVN |
| S03 | 34 | ADC1_CH6 | D34 |
| S04 | 35 | ADC1_CH7 | D35 |
| S05 | 32 | ADC1_CH4 | D32 |
| S06 | 33 | ADC1_CH5 | D33 |
| S07 | 25 | ADC2_CH8 | D25 |
| S08 | 26 | ADC2_CH9 | D26 |
| S09 | 27 | ADC2_CH7 | D27 |
| S10 | 14 | ADC2_CH6 | D14 |
| S11 | 13 | ADC2_CH4 | D13 |
| S12 | 4 | ADC2_CH0 | D4 |

GPIO34/35/36/39 solo son entradas y no disponen de pull-up interno; la barra
provee el pull-up del colector. GPIO37/38 del chip no se exponen en el WROOM usado.
Cada salida de sensor tiene su propia entrada. Se leen secuencialmente mediante
el ADC interno; **sin multiplexor externo**, sin conversión simultánea de 12 canales.

## Driver TB6612FNG

| Terminal del módulo | Conectar a | Función |
| --- | --- | --- |
| AIN1 | GPIO16 | Dirección izquierda 1 |
| AIN2 | GPIO17 | Dirección izquierda 2 |
| PWMA | GPIO21 + 10 kΩ a GND | PWM izquierda 20 kHz |
| BIN1 | GPIO18 | Dirección derecha 1 |
| BIN2 | GPIO19 | Dirección derecha 2 |
| PWMB | GPIO22 + 10 kΩ a GND | PWM derecha 20 kHz |
| STBY | 3V3 mediante jumper/interruptor; 10 kΩ a GND | Habilitación física |
| VCC | 3V3 | Alimentación lógica, compatible con GPIO |
| VM | Rail de motores apropiado | No es VCC lógico |
| GND | Masa común | Unir lógica, driver, barra y batería |
| AO1/AO2 | Motor izquierdo | Invertir cables o `INVERT_LEFT` si corresponde |
| BO1/BO2 | Motor derecho | Invertir cables o `INVERT_RIGHT` si corresponde |

Se usan seis GPIO en el driver: **STBY no consume GPIO**. PWM a cero frena según
la tabla del TB6612; abrir STBY produce alta impedancia, no frenado activo.
Los pull-down externos en PWM mantienen la orden a cero durante reset/carga.
Verificar que el módulo comprado no tenga pull-ups incompatibles o STBY puenteado.

## Botones

| Señal | GPIO | Cableado |
| --- | --- | --- |
| B1 | 5 | Pulsador normalmente abierto a GND; 10 kΩ a 3V3 |
| B2 | 15 | Pulsador normalmente abierto a GND; 10 kΩ a 3V3 |
| B3 | 23 | Pulsador normalmente abierto a GND; 10 kΩ a 3V3 |

Firmware con `INPUT_PULLUP`, activos en LOW y antirrebote 30 ms.
GPIO5/15 son strapping: mantenerlos altos durante reset con botones sueltos.
GPIO5 afecta temporización de SDIO slave; GPIO15 afecta SDIO y mensajes ROM.
El proyecto no usa SDIO slave; no se emplean GPIO0/2/12 para evitar afectar descarga
o tensión de flash. No agregar capacitores grandes que alteren los straps.

## Conector de barra J1 (propuesta 1×15)

| Pin J1 | Señal | Conectar |
| --- | --- | --- |
| 1 | +5V_LED | Regulador 5 V, solo emisores |
| 2 | +3V3_SENS | Lógica 3,3 V, pull-ups receptores |
| 3 | GND | Masa común |
| 4 | S01 | GPIO36 |
| 5 | S02 | GPIO39 |
| 6 | S03 | GPIO34 |
| 7 | S04 | GPIO35 |
| 8 | S05 | GPIO32 |
| 9 | S06 | GPIO33 |
| 10 | S07 | GPIO25 |
| 11 | S08 | GPIO26 |
| 12 | S09 | GPIO27 |
| 13 | S10 | GPIO14 |
| 14 | S11 | GPIO13 |
| 15 | S12 | GPIO4 |

Pin 1 marcado en ambas placas. Es un conector propuesto, no compatible pin a pin
con una QTR-8A. Reserva GPIO0/1/2/3/12; GPIO6..11 quedan para flash. No hay pines
seguros sobrantes suficientes para un juego completo de encoders sin rediseño.
