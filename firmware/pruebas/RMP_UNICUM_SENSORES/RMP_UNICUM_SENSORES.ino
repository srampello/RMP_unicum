/* QRE12 + 74HC4067 sobre ADC1 GPIO0, con Wi-Fi activo. USB CDC, 115200. */
#include <WiFi.h>
#include "Hardware.h"
Motors motors;
uint16_t sensorRaw[12],switchRaw[3];
void setup() {
  Serial.begin(115200);motors.begin();beginSensors();
  pinMode(Unicum::MICROSTART_PIN,INPUT_PULLDOWN);
  WiFi.mode(WIFI_AP);WiFi.softAP("RMP_UNICUM_TEST",Unicum::AP_PASSWORD);
  Serial.println("S01..S12=C0..C11, izquierda a derecha | ADC 0..4095 | blanco=bajo");
}
void loop() {
  motors.stop();readSensors(sensorRaw);int selected=readStrategy(switchRaw);
  for(uint8_t i=0;i<12;i++){Serial.print(sensorRaw[i]);Serial.print('\t');}
  Serial.printf("selector=%d [%u/%u/%u] micro=%d\n",selected+1,switchRaw[0],switchRaw[1],switchRaw[2],digitalRead(Unicum::MICROSTART_PIN));
  delay(100);
}
