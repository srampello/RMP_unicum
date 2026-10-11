# Validación de v0.1 · 2026-10-11

## Verificaciones ejecutadas

- `tests/check_project.py`: correcto; 21 GPIO sin duplicados, 12 entradas ADC,
  ningún uso de GPIO de flash/USB ni GPIO12, y tres `.ino` en carpetas homónimas.
- `tests/test_line_math.cpp`: compilado con g++ C++11, `-Wall -Wextra -Werror`;
  pasan casos de normalización blanca/negra, saturación, calibración degenerada,
  centro, extremos y ausencia/umbral de línea.
- Los **tres sketches compilaron correctamente** para `esp32:esp32:esp32` con
  Arduino CLI 1.5.2-rc.1 y **Arduino-ESP32 3.3.12**.
- Diagramas SVG renderizados y revisados visualmente.

| Sketch | Flash usada | RAM estática usada | Partición de aplicación |
| --- | --- | --- | --- |
| RMP_UNICUM | 1.672.703 bytes (53%) | 63.936 bytes (19%) | Huge APP, 3.145.728 bytes |
| RMP_UNICUM_SENSORES | 279.508 bytes (21%) | 22.220 bytes (6%) | Default, 1.310.720 bytes |
| RMP_UNICUM_MOTORES | 299.771 bytes (22%) | 22.336 bytes (6%) | Default, 1.310.720 bytes |

El principal genera una advertencia del core 3.3.12: `BluetoothSerial` está
marcado como deprecado para el soporte por defecto futuro en 4.0.0. Compila y
enlaza en 3.3.12; conservar core 3.x para esta versión del proyecto.
RAM indicada es estática de compilación, no memoria dinámica libre durante radio.

## Validación pendiente en hardware

No se ejecutó firmware en el robot. Quedan por medir: placa comercial exacta,
consumo/bloqueo de Pololu, reguladores bajo carga, contraste de los doce canales,
tiempo de lectura real, latencia de parada, transición Wi-Fi/ADC2, Bluetooth,
interfaz web en el ESP32, dirección motores y ajuste PD en pista. La compilación
no sustituye esas pruebas. Ver la secuencia de `PUESTA_EN_MARCHA.md`.
