# Validación v0.2 · 2026-10-13

## Comprobaciones ejecutadas

- `tests/check_project.py`: sin GPIO duplicados; ADC1 GPIO0; C0..C11 para QRE y
  C12..C14 para selector. Motores fuera de strapping; BOOT y USB no se comparten.
  Trece GPIO utilizados contando BOOT, tres sketches independientes/homónimos.
- `tests/test_line_math.cpp`: normalización blanca/negra, saturación, centro,
  extremos, ausencia/umbral de línea. Compila con `-Wall -Wextra -Werror`.
- `tests/test_start_gate.cpp`: HIGH al encender no inicia; LOW 50 ms seguido de
  HIGH 20 ms inicia una vez; LOW breve no habilita rearranque; rollover de millis.
- Tres sketches compilados correctamente con **Arduino-ESP32 3.3.12**, Arduino
  CLI 1.5.2-rc.1, FQBN **esp32:esp32:esp32c3:CDCOnBoot=cdc**, warnings all.
- Sintaxis JavaScript de WebPage.h comprobada con `node --check`.
- Diagramas SVG renderizados y revisados visualmente.

| Sketch | Flash usada | RAM estática | Partición APP |
| --- | --- | --- | --- |
| RMP_UNICUM | 1.043.409 bytes (79%) | 38.696 bytes (11%) | Default, 1.310.720 bytes |
| RMP_UNICUM_SENSORES | 967.011 bytes (73%) | 36.576 bytes (11%) | Default, 1.310.720 bytes |
| RMP_UNICUM_MOTORES | 326.012 bytes (24%) | 14.636 bytes (4%) | Default, 1.310.720 bytes |

Compilación sin advertencias del proyecto. RAM indicada es la estática de link;
no mide memoria libre dinámica durante Wi-Fi/tarea. El test sensores incluye
Wi-Fi, por eso su binario es mayor que el de motores. No hay binarios publicados:
compilar/cargar con placa y opciones indicadas.

## Pendiente en el robot

No se probó en hardware. Verificar variante Super Mini, GPIO8/LED/strapping,
MicroStart real alimentado a 3V3 y polaridad de SIGNAL, cableado STBY/STOP,
selector, contraste/crosstalk del MUX, tiempo de asentamiento, latencia y jitter
bajo Wi-Fi, ciclo real del control y efecto de mensajes de arranque.
También consumo/bloqueo N20, 6 V regulados, picos de frenado, térmica del driver,
sentido de motores y ajuste PD en pista. Compilar no acredita rendimiento físico.

La v0.1 con ESP32 WROOM y lecturas directas está en el historial Git; su validación
y pinout ya no corresponden al firmware actual.
