# Electrónica y barra analógica

## Una celda QRE1113GR, repetir doce veces

Pinout de onsemi: **1 ánodo, 2 cátodo, 3 colector, 4 emisor**. Revisar la vista
y orientación del encapsulado en el datasheet; no deducir el pin 1 por una foto.

| Componente/terminal | Conexión |
| --- | --- |
| +5V_LED | Resistencia de 180 Ω, mínimo 1/8 W, una por LED |
| Extremo de resistencia 180 Ω | QRE pin 1, ánodo |
| QRE pin 2, cátodo | GND |
| +3V3_SENS | Resistencia de 47 kΩ, una por receptor |
| Extremo de resistencia 47 kΩ | QRE pin 3, colector: nodo OUT |
| QRE pin 4, emisor | GND |
| OUT | Resistencia serie de 100 Ω hacia señal Sxx/Cxx del MUX |
| Sxx | Capacitor opcional 1 nF a GND, antes del MUX, por sensor |

![Celda analógica](../hardware/sensor_analogico.svg)

Este es el mismo principio de salida analógica por fototransistor que la QTR-8A,
adaptado a doce QRE1113GR y lógica de 3,3 V. **No es una copia de su circuito de
alimentación LED:** aquí cada emisor tiene su resistencia independiente a 5 V.
Emisores siempre encendidos; no hay control individual ni resta de luz ambiente.

Con Vf típica de 1,2 V: I_LED≈(5−1,2)/180≈21 mA por sensor, ≈253 mA para doce.
Potencia en cada resistencia ≈0,080 W; usar 1/8 W o más y considerar derating.
La corriente real depende de Vf/tolerancias. No alimentar los 12 LED desde un GPIO
ni desde el regulador 3V3 de una placa genérica. Si se cambia tensión, recalcular
resistencias. No poner varios LED en paralelo compartiendo una sola resistencia.

Mayor reflexión en blanco → más conducción → OUT baja; negro → OUT alta.
El pull-up a 3,3 V limita la salida a la lógica del ESP32. **No llevar el pull-up
a 5 V** aunque el LED use 5 V. ADC a 12 bits y atenuación 11 dB: la escala de
conversión del ESP32-C3 puede saturar antes de 3,3 V (~2,5 V), por lo que
4095 no implica medir con precisión 3,3 V. Para seguimiento se calibra cada canal.

47 kΩ es valor inicial inspirado en QTR-8A; validar contraste/ruido con QRE,
altura y piso reales. Si blanco y negro saturan ambos o tienen poco contraste,
ajustar altura y probar 10–22 kΩ. Capacitor 1 nF con 47 kΩ produce τ≈47 µs;
no instalar 100 nF en cada señal sin estudiar la respuesta temporal.

## Mecánica de barra (propuesta)

- Línea recta de 12 sensores; paso inicial **8 mm**, 88 mm entre S01/S12.
- Ancho orientativo de PCB 96–100 mm, a confirmar por encapsulado y montaje.
- Caras ópticas orientadas al piso, todas a la misma altura. Comenzar cerca de
  2–3 mm y medir contraste; no trasladar automáticamente alturas recomendadas
  para los sensores ópticos distintos de una QTR comercial.
- Serigrafía S01..S12 de izquierda a derecha según dirección de avance; flecha
  frontal en la base. Barra delante del eje, distancia a definir con chasis.
- Separar óptica y pistas analógicas del driver y cables de motores; masa continua
  en barra. Mantener zona de antena del ESP32 libre según su fabricante.

No se fijó geometría de fabricación: faltan dimensiones de ruedas/chasis y placa.

## MUX, selector y MicroStart

74HC4067 alimentado a **3,3 V**, EN a GND, C0..C11 a los doce OUT, C12..C14 al
selector y C15 a GND. SIG a ADC1 GPIO0 del C3. Usar 74HC, no alimentar 74HC a 5 V
suponiendo que una salida GPIO3V3 satisface sus umbrales. Pull-ups de receptores
siempre a 3V3. Desacoplar VCC del MUX con 100 nF junto al chip/módulo.

Añadir pull-ups de 10 kΩ en GPIO2 y GPIO8 (S0/S1) para strapping, y de 10 kΩ en
C12/C13/C14 para selector. El selector 1P3T conecta exactamente uno a GND. Sin
contacto o con dos contactos seleccionados, inicio bloqueado.

MicroStart alimentado a 3V3, compatible según JSumo. SIGNAL va a GPIO3 y a STBY
por interruptor de habilitación; STBY con 10 kΩ a GND. Con STOP (LOW), driver en
standby por hardware. Con START (HIGH), driver habilitado, pero PWM solo se activa
tras validación/START del firmware. Verificar nivel/polaridad en módulo real.
No conectar salida a 5 V directamente al ESP32. No conectar SIGNAL a batería.

## Batería 2S y motores N20 6 V / 2000 RPM

Se mantiene la batería LiPo 2S elegida: 7,4 V nominal, **8,4 V cargada**. Rail
propuesto **6 V regulados para VM**, y otro **5 V para Super Mini y emisores**.
3V3 de la placa alimenta MUX, receptores, MicroStart y TB6612 VCC. No alimentar
motores ni LED de barra desde el regulador 3V3 del C3.

N20 6 V / 2000 RPM no indica corriente de bloqueo, fabricante ni consumo real:
esos valores faltan para dimensionar regulador, fusible y validar TB6612. RPM es
valor nominal sin carga; no determina velocidad del robot sin diámetro de ruedas.
No asumir que limitando PWM se convierte una batería 8,4 V en fuente de 6 V: los
pulsos siguen siendo de 8,4 V. Elegir regulador que tolere transitorios/frenado;
medir picos y tensión bajo carga. No hay protección LiPo por firmware.

| Rail | Destino | Criterio |
| --- | --- | --- |
| LiPo 2S | Reguladores por fusible/interruptor | 8,4 V máxima, protección externa |
| 6 V | TB6612 VM, dos N20 | Corriente por motores/arranques y térmica del driver |
| 5 V | Pin 5V de Super Mini y LED | Propuesta 2 A con margen; verificar placa |
| 3,3 V | MUX, receptores, MicroStart, TB6612 VCC | Comprobar margen del regulador Super Mini |
| GND | Todos los componentes | Retornos de motor separados hasta punto común |

Revisar la Super Mini concreta antes de conectar USB y 5 V externo simultáneamente.
No llevar LiPo directamente al pin 5V ni 3V3. El MicroStart no es interruptor de
alimentación; el robot queda alimentado y espera su señal.

## TB6612, reset y ruido

TB6612 VCC=3,3 V, VM=6 V. Rango recomendado VM 2,5..13,5 V y VCC 2,7..5,5 V.
Datasheet actual: 1 A en rango operativo sin PWM a VM≥4,5 V; 1,2 A figura como
máximo absoluto, 3,2 A es pico de un único pulso de 10 ms. Dimensionar por térmica
real, corriente de motores y módulo; no usar el pico como corriente de trabajo.

- PWMA/PWMB (GPIO1/10) con 10 kΩ a GND. AIN1/2 y BIN1/2 también con 10 kΩ
  a GND. Los pines UART GPIO20/21 se destinan a S2/S3 del MUX, por lo que mensajes
  ROM no pulsan PWM. Usar USB CDC para evitar que UART interfiera con selección
  de canales. Mantener MicroStart STOP y STBY abierto al cargar/reiniciar.
- STBY con 10 kΩ a GND y SIGNAL a través de interruptor. Retirar un puente a VCC
  si el breakout lo trae. MicroStart LOW → alta impedancia; PWM cero con STBY HIGH
  → frenado activo. Son comportamientos diferentes.
- Junto a driver: 100 nF + 10 µF en VM y VCC; reserva inicial 470 µF ≥16 V en VM.
  Descontar capacitores ya presentes en breakout y dimensionar transitorios reales.
- Barra: 100 nF + 10 µF por rail. Motores: 100 nF entre bornes, cables cortos/trenzados.
- No añadir un diodo único paralelo a cada motor reversible: puente H ya tiene
  caminos de recirculación. Mantener potencia/ruido lejos de señales ADC.
- Evitar capacitores grandes en SIG común que retengan la tensión del canal anterior.
  Capacitores opcionales 1 nF son **por sensor, antes del MUX**. No instalar 100 nF
  en SIG sin recalcular asentamiento/frecuencia. 120 µs es valor inicial a validar.

PCB aún no ruteado. Confirmar dimensiones, placa Super Mini, consumo de N20 y
altura de barra antes de diseñar archivos de fabricación.
