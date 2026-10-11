# Referencias de diseño

Consultadas el 11 de octubre de 2026. Son documentación de fabricantes; el mapa
concreto de la placa comercial debe confirmarse con su serigrafía/ficha propia.

- [ESP32-WROOM-32 datasheet, Espressif](https://documentation.espressif.com/esp32-wroom-32_datasheet_en.pdf): pines expuestos, lógica, strapping.
- [GPIO ESP32 / limitación ADC2-Wi-Fi](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/gpio.html).
- [Arduino ESP32 ADC](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html): resolución y atenuación.
- [Arduino ESP32 LEDC](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/ledc.html): API 3.x del PWM.
- [Bluetooth Arduino ESP32](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/bluetooth.html): ejemplo BluetoothSerial/SPP.
- [QRE1113/QRE1113GR, onsemi](https://www.onsemi.com/download/data-sheet/pdf/qre1113-d.pdf): pines, Vf, corriente LED y curvas ópticas.
- [Guía QTR-8A/QTR-8RC, Pololu](https://www.pololu.com/docs/0J12/all): salida analógica y resistencia de colector.
- [TB6612FNG, Toshiba](https://toshiba.semicon-storage.com/info/docget.jsp?did=10660&prodName=TB6612FNG): niveles lógicos, corrientes, PWM y desacoplo.

La selección de resistencias, paso mecánico, perfiles PD y reguladores es propuesta
de este proyecto. No representa un diseño certificado por esos fabricantes.
