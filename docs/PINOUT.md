# Pinout v0.2 · ESP32-C3 Super Mini

Números **GPIO**, no posición de patas. Validar la placa concreta: Super Mini
es un formato comercial con variantes, no un modelo de placa oficial Espressif.
Se presupone que expone GPIO0..10 y GPIO20/21. USB nativo usa 18/19.

## 74HC4067

| Terminal | Conectar | Nota |
| --- | --- | --- |
| VCC | 3V3 | 74HC, no confundir con HCT |
| GND | GND común | Compartir masa con barra y driver |
| EN / E | GND | Habilitación activa en LOW, sin GPIO |
| S0 | GPIO2 | Bit menos significativo; 10 kΩ a 3V3 |
| S1 | GPIO8 | 10 kΩ a 3V3; puede compartir LED integrado |
| S2 | GPIO20 | Dirección bit 2 |
| S3 | GPIO21 | Dirección bit 3 |
| SIG / COM | GPIO0 | ADC1_CH0, entrada analógica |
| C0..C11 | S01..S12 | Doce colectores analógicos independientes |
| C12 / C13 / C14 | Selector posiciones 1 / 2 / 3 | Pull-up 10 kΩ a 3V3 en cada canal |
| C15 | GND | No usado |

GPIO2/8 son strapping: los pull-ups mantienen el arranque correcto, y el MUX no
fuerza esos pines. GPIO9 queda como BOOT/calibración. No usar LED de GPIO8 como
indicador desde otra librería: es S1 del MUX; que parpadee es normal en placas
con LED activo LOW. No cambiar resistencias/puentes del LED sin revisar esa placa.

| Sensor | Canal del MUX | Orden |
| --- | --- | --- |
| S01 | C0 | Extremo izquierdo |
| S02 | C1 | |
| S03 | C2 | |
| S04 | C3 | |
| S05 | C4 | |
| S06 | C5 | Centro izquierdo |
| S07 | C6 | Centro derecho |
| S08 | C7 | |
| S09 | C8 | |
| S10 | C9 | |
| S11 | C10 | |
| S12 | C11 | Extremo derecho |

Orden mirando hacia el avance. Centro de posición=5500. Solo SIG ocupa ADC.

## Selector de estrategias

Propuesta: **selector rotativo de un polo / tres posiciones (1P3T)**.
COM del selector a GND; cada contacto a C12, C13 o C14. Cada uno de esos canales
lleva pull-up propio de 10 kΩ a 3V3. La posición elegida queda cerca de 0 V y las
otras cerca de 3,3 V. **No conectar COM a la batería ni a 5 V.**

Firmware: elegido <500 ADC, no elegidos ≥2500. Entre rangos, ninguno elegido o
más de uno LOW = inválido. Estabilidad 50 ms; muestreo de selector cada 25 ms.
Cambiar solo detenido. En marcha se conserva el perfil capturado al START.
Un DIP puede reemplazarlo con tres interruptores, pero se debe activar exactamente
uno. Un selector de dos bits necesitaría cambiar el decodificador.

## TB6612FNG

| Terminal | Conectar | Función |
| --- | --- | --- |
| AIN1 | GPIO4 + 10 kΩ a GND | Dirección izquierda 1 |
| AIN2 | GPIO5 + 10 kΩ a GND | Dirección izquierda 2 |
| PWMA | GPIO1 + 10 kΩ a GND | PWM izquierdo, 20 kHz / 8 bits |
| BIN1 | GPIO6 + 10 kΩ a GND | Dirección derecha 1 |
| BIN2 | GPIO7 + 10 kΩ a GND | Dirección derecha 2 |
| PWMB | GPIO10 + 10 kΩ a GND | PWM derecho |
| VCC | 3V3 | Lógica |
| VM | Rail 6 V regulados | Potencia N20 |
| GND | Masa común | Señales/potencia |
| AO1/AO2 | N20 izquierdo | 6 V, 2000 RPM |
| BO1/BO2 | N20 derecho | 6 V, 2000 RPM |
| STBY | SIGNAL MicroStart por interruptor en serie; 10 kΩ a GND | Habilitación hardware |

No son cuatro PWM: se mantienen cuatro direcciones digitales y dos PWM normales.
GPIO5 es digital aquí; no se usa su ADC2. PWM a cero con STBY alto frena; STBY bajo
pone salidas en alta impedancia (rueda libre). No puente STBY a 3V3 en este diseño.
Si el breakout trae STBY unido a VCC o un pull-up, corregir según su esquema.

**USB CDC On Boot: Enabled obligatorio.** GPIO20/21 controlan S2/S3 del MUX:
no conectar UART ni usar `Serial0/Serial1` sobre ellos. Mensajes ROM pueden variar
la selección de canal al boot, pero no se usan como PWM de motores. Los PWM están
en GPIO1/10, con pull-downs a GND; firmware inicializa PWM a cero antes de dirección.
AIN/BIN también llevan pull-downs; mantener MicroStart en STOP e interruptor STBY
abierto durante carga/reset y pruebas de alimentación.

## MicroStart JSumo

| Terminal | Conectar |
| --- | --- |
| Alimentación (`+5V` en algunas serigrafías) | **3V3**, conforme a la compatibilidad 3,3 V del fabricante |
| GND | Masa común |
| SIGNAL | GPIO3 y, mediante interruptor de habilitación, STBY del TB6612 |

MicroStart alimentado a 3V3: SIGNAL HIGH=START, LOW=STOP, de acuerdo con el ejemplo
JSumo. Confirmar tensión/polaridad del módulo real con multímetro y motor desconectado.
Si se alimenta a 5 V, **no conectar SIGNAL directo al C3**: necesita adaptación de
nivel apropiada. Se propone 3,3 V para evitarla; no hay resistencia/divisor que
convierta la alimentación del módulo.

GPIO3 tiene `INPUT_PULLDOWN`; STBY tiene pull-down físico 10 kΩ. El MicroStart
actúa como autorización, no como interruptor de batería. El interruptor en serie
con STBY permite deshabilitar el driver; enviar STOP y detener antes de cerrarlo
otra vez, porque el firmware no detecta ese interruptor abierto.

## BOOT y conectores

GPIO9 / BOOT integrado: largo ≥1,2 s detenido = calibrar; corto = armar con
MicroStart en LOW; presionar durante marcha = detener. No mantener BOOT al reset
salvo para descargar firmware. Calibración exige MicroStart STOP.

Barra propuesta J1 1×15: 1=5V_LED, 2=3V3_SENS, 3=GND, 4..15=S01..S12 hacia
C0..C11. MUX montado inicialmente en la placa de control. J2 MicroStart: 3V3,
GND, SIGNAL (orden físico según módulo, no asumir); J3 selector: GND,C12,C13,C14.
Marcar pin 1 en todos los conectores. Se usan los 13 GPIO disponibles incluyendo
BOOT; no quedan GPIO libres para encoders sin ampliar/rediseñar.
