/* Ruedas levantadas. Comandos con nueva línea a 115200:
 * l 80 / r 80 / b 80 / b -80 / stop. PWM limitado a +/-150.
 * Frena tras 2 s sin nuevo comando o al presionar cualquier botón.
 */
#include "Hardware.h"
#include <stdio.h>
Motors motors;
String buffer;
bool ready=false;
uint32_t lastCommand=0;
void command(String s) {
  s.trim(); s.toLowerCase();
  if(s=="stop") { motors.stop(); Serial.println("Detenido"); return; }
  char side=0, extra=0; int pwm=0;
  if(sscanf(s.c_str(),"%c %d %c",&side,&pwm,&extra)!=2 ||
     (side!='l' && side!='r' && side!='b') || abs(pwm)>150) {
    motors.stop(); Serial.println("Usar l/r/b PWM (-150..150) o stop"); return;
  }
  for(uint8_t p:Unicum::BUTTON_PINS) if(digitalRead(p)==LOW) { motors.stop(); return; }
  if(!ready) return;
  motors.set(side=='r' ? 0:pwm,side=='l' ? 0:pwm);
  lastCommand=millis(); Serial.printf("L=%d R=%d (2 s maximo)\n",motors.left,motors.right);
}
void setup() {
  Serial.begin(115200); ready=motors.begin();
  for(uint8_t p:Unicum::BUTTON_PINS) pinMode(p,INPUT_PULLUP);
  Serial.println("Test motores | l 80 / r 80 / b 80 / stop | ruedas levantadas");
  if(!ready) Serial.println("Error inicializando PWM");
}
void loop() {
  for(uint8_t p:Unicum::BUTTON_PINS) if(digitalRead(p)==LOW) motors.stop();
  if(millis()-lastCommand>=2000) motors.stop();
  for(uint8_t i=0;i<32 && Serial.available();i++) {
    char c=Serial.read();
    if(c=='\n') { command(buffer); buffer=""; }
    else if(c!='\r') { if(buffer.length()<40) buffer+=c; else { buffer=""; motors.stop(); } }
  }
  delay(1);
}
