# RMP UNICUM · v0.2

Seguidor de línea de **RMP Robotics**: **ESP32-C3 Super Mini**, barra de **12
QRE1113GR**, **74HC4067**, **TB6612FNG**, dos **N20 6 V / 2000 RPM**, selector
**DIP switch de tres vías** para las velocidades de largada y arranque/parada por **MicroStart JSumo**.

**Esta versión reemplaza el mapa WROOM con entradas directas. No reutilizar su
cableado.** La versión anterior queda en el historial de Git.

## Arquitectura

| Elemento | Solución |
| --- | --- |
| Sensores | S01..S12, izquierda a derecha, a C0..C11 del 74HC4067 |
| Lectura | SIG del MUX a GPIO0 / ADC1 del ESP32-C3 |
| Selector | DIP 3 vías; cada interruptor entre C12/C13/C14 y GND |
| Driver | TB6612: A=izquierdo, B=derecho, dirección + PWM por motor |
| Arranque/parada | MicroStart a GPIO3 y a STBY; alimentado a 3,3 V |
| Wi-Fi | Permanece activo; lectura de los 12 sensores por ADC1 |
| Alimentación | Se mantiene LiPo 2S; propuesta de 6 V regulados para motores y 5 V para placa/LED |

El MUX y los pull-ups de receptores trabajan a **3,3 V**. Los emisores IR usan
5 V con una resistencia por LED. No llevar 5 V a SIG ni GPIO3. El MicroStart
**inicia la marcha**, no conmuta la alimentación general del robot.

## Documentación

| Archivo | Contenido |
| --- | --- |
| [PINOUT](docs/PINOUT.md) | GPIO, canales, selector y cableado del MicroStart |
| [HARDWARE](docs/HARDWARE.md) | Sensores, niveles, alimentación y driver |
| [PUESTA_EN_MARCHA](docs/PUESTA_EN_MARCHA.md) | Arduino, pruebas, calibración y START/STOP |
| [FIRMWARE](docs/FIRMWARE.md) | Control PD, estados y servidor Wi-Fi |
| [Conexiones](hardware/conexiones.svg) | Diagrama de la arquitectura actual |
| [Celda QRE](hardware/sensor_analogico.svg) | Circuito repetido 12 veces |
| [BOM](hardware/BOM.csv) | Lista de componentes inicial |
| [VALIDACION](docs/VALIDACION.md) | Comprobaciones reales y pruebas pendientes |
| [REFERENCIAS](docs/REFERENCIAS.md) | Documentación original de fabricantes |

## Firmware

| Sketch | Uso |
| --- | --- |
| [RMP_UNICUM_SENSORES](firmware/pruebas/RMP_UNICUM_SENSORES/RMP_UNICUM_SENSORES.ino) | Lecturas QRE, selector y MicroStart con AP Wi-Fi activo; motores quietos |
| [RMP_UNICUM_MOTORES](firmware/pruebas/RMP_UNICUM_MOTORES/RMP_UNICUM_MOTORES.ino) | Prueba USB por motor, ruedas levantadas, con habilitación MicroStart |
| [RMP_UNICUM](firmware/UNiCUM/RMP_UNICUM/RMP_UNICUM.ino) | Seguidor PD, estrategias, calibración NVS y telemetría web |

Arduino-ESP32 **3.x**, placa **ESP32C3 Dev Module**, **USB CDC On Boot: Enabled**,
flash según placa (4 MB habitual), partición Default. Monitor **115200 baud**.
Cada `.ino` está en una carpeta con su mismo nombre y sus headers.

USB CDC es obligatorio: GPIO20/21 se usan para S2/S3 del MUX, no para UART0.
El C3 dispone de Bluetooth LE, pero **esta versión implementa Wi-Fi y USB**,
no Bluetooth Classic/SPP ni un servicio BLE.

## Uso

1. Probar sensores, selector y MicroStart; luego dirección de motores.
2. MicroStart en STOP. Mantener BOOT 1,2 s o pulsar “Calibrar 8 s” en la web.
   Mover manualmente toda la barra sobre blanco y negro. Línea blanca por defecto.
3. DIP: solo SW1 en ON (SW2/SW3 OFF). Con calibración válida, el robot se arma automáticamente
   al encender o completar calibración, mientras MicroStart esté LOW.
4. Colocar barra sobre línea y enviar START al MicroStart. No se fuerza la marcha
   desde la web. Arranque sin demora adicional por defecto.
5. STOP del MicroStart deshabilita STBY por hardware y detiene por software.
   BOOT o web/USB también pueden detener; después hay que rearmar con MicroStart LOW.

Wi-Fi: **RMP_UNICUM**, clave inicial **Unicum2026**, **http://192.168.4.1**.
La interfaz muestra 12 barras 0..1000, valores ADC, selector, MicroStart y PWM.
Color de línea elegido = barra alta. Permite editar perfiles solo detenido con
MicroStart en STOP; elegir el perfil para carrera con el selector físico.

## Alcance

Diseño lógico, pinout, firmware, pruebas y diagramas; **PCB todavía no ruteado**.
Sin encoders, turbina, detección de meta ni lectura de batería/corriente.
Los perfiles son puntos de partida: ajustar en pista. Faltan corriente de bloqueo
real del N20, dimensiones de ruedas/chasis, capacidad de batería y validación física.

## Comprobaciones

```bash
python3 tools/sync_headers.py
python3 tests/check_project.py
g++ -std=c++11 -Wall -Wextra -Werror tests/test_line_math.cpp -o /tmp/unicum_line_math
/tmp/unicum_line_math
g++ -std=c++11 -Wall -Wextra -Werror tests/test_start_gate.cpp -o /tmp/unicum_start_gate
/tmp/unicum_start_gate
arduino-cli compile --fqbn esp32:esp32:esp32c3:CDCOnBoot=cdc firmware/UNiCUM/RMP_UNICUM
arduino-cli compile --fqbn esp32:esp32:esp32c3:CDCOnBoot=cdc firmware/pruebas/RMP_UNICUM_SENSORES
arduino-cli compile --fqbn esp32:esp32:esp32c3:CDCOnBoot=cdc firmware/pruebas/RMP_UNICUM_MOTORES
```

Cambios de hardware: editar `Config.h` y `Hardware.h` del principal, luego ejecutar
`tools/sync_headers.py`. La calibración usa namespace NVS `unicum-c3-v2` para no
reutilizar accidentalmente la de la arquitectura anterior.
