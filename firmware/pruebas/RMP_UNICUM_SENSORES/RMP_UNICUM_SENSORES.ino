/* Lectura de 12 QRE1113GR sin MUX; ESP32 WROOM; 115200 baud.
 * No inicia Wi-Fi/Bluetooth y mantiene PWM de motores a cero.
 */
#include "Hardware.h"
uint16_t sensorRaw[12];
void setup() {
  Serial.begin(115200);
  pinMode(Unicum::PWMA,OUTPUT); digitalWrite(Unicum::PWMA,LOW);
  pinMode(Unicum::PWMB,OUTPUT); digitalWrite(Unicum::PWMB,LOW);
  beginSensors();
  Serial.println("S01..S12 izquierda a derecha | ADC 0..4095 | blanco = bajo");
}
void loop() {
  readSensors(sensorRaw);
  for(uint8_t i=0;i<12;i++) {
    Serial.print(sensorRaw[i]); Serial.print(i==11 ? '\n':'\t');
  }
  delay(100);
}
