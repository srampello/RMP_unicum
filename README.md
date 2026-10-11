# RMP UNICUM

Seguidor de línea de **RMP Robotics** con ESP32 clásico WROOM/ESP32S,
TB6612FNG, tres botones y doce QRE1113GR analógicos conectados directamente.
Base de diseño y firmware v0.1 para poner en marcha y ajustar en pista.

## Hardware previsto

- Placa de desarrollo ESP32-WROOM/ESP32S, Wi-Fi y Bluetooth 4.2.
- Barra propia de 12 QRE1113GR, principio analógico de QTR-8A, sin MUX.
- Driver TB6612FNG; canal A izquierdo, canal B derecho.
- 3 botones: selección/arranque de estrategias, calibración y configuración.
- **Motores Pololu de 6 V y batería LiPo 2S**, confirmados para este proyecto.
  Falta modelo/relación de reducción y corriente de bloqueo de los motores.

> **Limitación que determina el proyecto:** el ESP32 WROOM expone seis entradas
> ADC1 útiles para esta placa y las otras seis deben usar ADC2. ADC2 no está
> disponible con Wi-Fi activo. El firmware utiliza Wi-Fi exclusivamente para
> configurar detenido y lo apaga antes de calibrar, leer la barra o arrancar.
> No hay telemetría de 12 sensores por Wi-Fi durante la marcha con este hardware.
> Bluetooth Classic SPP queda disponible para comandos, sin streaming de carrera.

## Documentación

| Archivo | Contenido |
| --- | --- |
| [Distribución de pines](docs/PINOUT.md) | 12 ADC, TB6612FNG, botones y conector de barra |
| [Electrónica y barra](docs/HARDWARE.md) | Circuito por sensor, alimentación, motor y criterios PCB |
| [Puesta en marcha](docs/PUESTA_EN_MARCHA.md) | Arduino IDE, pruebas, calibración, botones y Wi-Fi |
| [Firmware y control](docs/FIRMWARE.md) | PD, estrategias, polaridad y pérdida de línea |
| [Diagrama de conexiones](hardware/conexiones.svg) | Plano lógico de conexiones; no es un PCB |
| [Circuito del sensor](hardware/sensor_analogico.svg) | Celda QRE1113GR que se repite 12 veces |
| [Lista de componentes](hardware/BOM.csv) | Componentes y cantidades iniciales |
| [Fuentes](docs/REFERENCIAS.md) | Documentación original de fabricantes |

## Firmware

| Sketch | Uso |
| --- | --- |
| [RMP_UNICUM_SENSORES](firmware/pruebas/RMP_UNICUM_SENSORES/RMP_UNICUM_SENSORES.ino) | 12 lecturas crudas a 115200 baud; motores desactivados |
| [RMP_UNICUM_MOTORES](firmware/pruebas/RMP_UNICUM_MOTORES/RMP_UNICUM_MOTORES.ino) | Probar dirección/PWM con ruedas levantadas |
| [RMP_UNICUM](firmware/UNiCUM/RMP_UNICUM/RMP_UNICUM.ino) | Calibración, tres estrategias, PD y configuración inalámbrica |

Cada `.ino` está dentro de una carpeta con exactamente su mismo nombre.
Usar Arduino-ESP32 **3.x**, placa **ESP32 Dev Module**, flash 4 MB si corresponde
a la placa real, partición **Huge APP (3 MB No OTA/1 MB SPIFFS)**.
USB/serial: **115200 baud**. Línea blanca sobre fondo negro por defecto.

STBY se habilita por jumper/interruptor a 3V3, con pull-down de 10 kΩ a GND.
La parada por software actúa sobre PWM. Abrir STBY deshabilita físicamente el driver.

## Botones

| Botón | GPIO | Pulsación corta detenido | Pulsación larga detenido |
| --- | --- | --- | --- |
| B1 | 5 | Estrategia 1 + arranque en 3 s | Calibración manual de 8 s |
| B2 | 15 | Estrategia 2 + arranque en 3 s | Wi-Fi de configuración |
| B3 | 23 | Estrategia 3 + arranque en 3 s | Salir de Wi-Fi / detener |

Cualquier botón durante marcha, cuenta atrás o calibración cancela y detiene.
El arranque exige calibración válida de los 12 canales y línea presente.
Las velocidades y ganancias incluidas son **semillas de prueba**, pendientes de
ajuste al conjunto motor, rueda, batería, altura de barra y pista.

## Alcance v0.1

Incluye diseño eléctrico lógico, conexiones, lista de componentes, firmware y
pruebas. No incluye archivos de fabricación ni PCB ruteado. Antes de fijar un PCB
hay que comprobar la placa ESP32 exacta, los motores y las dimensiones del chasis.
No hay encoders, turbina, identificación de cruces ni detección automática de meta.

## Verificación reproducible

```bash
python3 tests/check_project.py
g++ -std=c++11 -Wall -Wextra -Werror tests/test_line_math.cpp -o /tmp/unicum_line_math
/tmp/unicum_line_math
arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=huge_app firmware/UNiCUM/RMP_UNICUM
arduino-cli compile --fqbn esp32:esp32:esp32 firmware/pruebas/RMP_UNICUM_SENSORES
arduino-cli compile --fqbn esp32:esp32:esp32 firmware/pruebas/RMP_UNICUM_MOTORES
```

Si se modifica `Config.h` o `Hardware.h` del sketch principal, ejecutar
`python3 tools/sync_headers.py` antes de compilar las pruebas. Ver
[VALIDACION.md](docs/VALIDACION.md) para el resultado real de la versión inicial.
