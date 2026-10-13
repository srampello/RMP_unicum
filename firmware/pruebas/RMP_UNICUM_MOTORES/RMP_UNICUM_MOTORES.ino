/* C3 + TB6612. Ruedas levantadas. USB CDC 115200.
 * Comandos: l 80 / r 80 / b 80 / b -80 / stop. +/-150, máximo 2 s.
 * Primero MicroStart LOW y luego START. Sin arranque automático de motores.
 */
#include "Hardware.h"
#include "StartGate.h"
#include <stdio.h>
Motors motors;
StartGate gate;
String buffer;
bool ready=false,remoteReady=false;
uint32_t lastCommand=0;
void command(String s) {
  s.trim();s.toLowerCase();
  if(s=="stop"){motors.stop();Serial.println("Detenido");return;}
  char side=0,extra=0;int pwm=0;
  if(sscanf(s.c_str(),"%c %d %c",&side,&pwm,&extra)!=2||
     (side!='l'&&side!='r'&&side!='b')||pwm < -150||pwm >150) {
    motors.stop();Serial.println("Usar l/r/b PWM (-150..150) o stop");return;
  }
  if(!ready||!remoteReady||digitalRead(Unicum::MICROSTART_PIN)==LOW||digitalRead(Unicum::CAL_BUTTON)==LOW) {
    motors.stop();Serial.println("Bloqueado: recibir LOW->START de MicroStart, BOOT suelto y PWM listo");return;
  }
  motors.set(side=='r'?0:pwm,side=='l'?0:pwm);lastCommand=millis();
  Serial.printf("L=%d R=%d (maximo 2 s)\n",motors.left,motors.right);
}
void setup() {
  Serial.begin(115200);ready=motors.begin();
  pinMode(Unicum::MICROSTART_PIN,INPUT_PULLDOWN);pinMode(Unicum::CAL_BUTTON,INPUT_PULLUP);
  Serial.println("Test N20 | l 80 / r 80 / b 80 / stop | ruedas levantadas");
}
void loop() {
  bool high=digitalRead(Unicum::MICROSTART_PIN)==HIGH;
  if(!high){remoteReady=false;motors.stop();}
  if(gate.update(millis(),high))remoteReady=true;
  if(digitalRead(Unicum::CAL_BUTTON)==LOW){remoteReady=false;motors.stop();}
  if(millis()-lastCommand>=2000)motors.stop();
  for(uint8_t i=0;i<32&&Serial.available();i++) {
    char c=Serial.read();
    if(c=='\n'){command(buffer);buffer="";}
    else if(c!='\r'){if(buffer.length()<40)buffer+=c;else{buffer="";motors.stop();}}
  }
  delay(1);
}
