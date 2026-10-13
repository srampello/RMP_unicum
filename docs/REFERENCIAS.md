# Referencias de diseño v0.2

Consultadas el 13 de octubre de 2026. Mapa de placa comercial Super Mini requiere
confirmación con su serigrafía/esquema: no existe una única variante oficial.

- [ESP32-C3 datasheet, Espressif](https://documentation.espressif.com/esp32-c3_datasheet_en.pdf): ADC1, strapping, USB y GPIO.
- [GPIO ESP32-C3](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/api-reference/peripherals/gpio.html).
- [Arduino ESP32 ADC](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html).
- [Arduino ESP32 LEDC](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/ledc.html): API 3.x PWM.
- [CD74HC4067, TI](https://www.ti.com/lit/ds/symlink/cd74hc4067.pdf): 16 canales, alimentación y niveles.
- [MicroStart JSumo](https://www.jsumo.com/microstart-sumo-minisumo-robot-start-module): alimentación 3,3/5 V y aprendizaje remoto.
- [Ejemplo START/STOP JSumo](https://blog.jsumo.com/xmotion-mini-sumo-robot-user-input-debug-codes-button-dipswith-and-start-module/): LOW=parar, HIGH=marchar.
- [QRE1113GR, onsemi](https://www.onsemi.com/download/data-sheet/pdf/qre1113-d.pdf): pinout, Vf, corriente y óptica.
- [QTR-8A/QTR-8RC, Pololu](https://www.pololu.com/docs/0J12/all): principio de salida analógica.
- [TB6612FNG, Toshiba](https://toshiba.semicon-storage.com/info/docget.jsp?did=10660&prodName=TB6612FNG): niveles, tabla funcional, corrientes y desacoplo.

Resistencias, geometría, perfiles y regulación son propuestas del proyecto; faltan
pruebas físicas. “N20 6 V 2000 RPM” no identifica fabricante/corriente de bloqueo.
