#pragma once
#include "Config.h"

class Motors {
public:
  int left=0, right=0;
  bool begin() {
    for(uint8_t p : {Unicum::AIN1,Unicum::AIN2,Unicum::BIN1,Unicum::BIN2,Unicum::PWMA,Unicum::PWMB}) {
      pinMode(p,OUTPUT); digitalWrite(p,LOW);
    }
    bool a=ledcAttach(Unicum::PWMA,Unicum::PWM_HZ,8);
    bool b=ledcAttach(Unicum::PWMB,Unicum::PWM_HZ,8);
    stop(); return a && b;
  }
  void set(int l,int r) {
    left=constrain(l,-255,255); right=constrain(r,-255,255);
    one(Unicum::AIN1,Unicum::AIN2,Unicum::PWMA,Unicum::INVERT_LEFT ? -left:left);
    one(Unicum::BIN1,Unicum::BIN2,Unicum::PWMB,Unicum::INVERT_RIGHT ? -right:right);
  }
  void stop() { set(0,0); }
private:
  void one(uint8_t in1,uint8_t in2,uint8_t pwm,int v) {
    // PWM a cero antes de cambiar dirección; cero mantiene short-brake.
    ledcWrite(pwm,0);
    digitalWrite(in1,v>=0 ? HIGH:LOW);
    digitalWrite(in2,v>=0 ? LOW:HIGH);
    ledcWrite(pwm,abs(v));
  }
};

inline void beginSensors() {
  analogReadResolution(12);
  for(uint8_t p:Unicum::SENSOR_PINS) { pinMode(p,INPUT); analogSetPinAttenuation(p,ADC_11db); }
}
inline void readSensors(uint16_t* raw) {
  for(uint8_t i=0;i<Unicum::SENSOR_COUNT;i++) {
    analogRead(Unicum::SENSOR_PINS[i]); // Descartar primera conversión tras cambio de canal.
    uint32_t sum=0;
    for(uint8_t s=0;s<Unicum::ADC_SAMPLES;s++) sum+=analogRead(Unicum::SENSOR_PINS[i]);
    raw[i]=sum/Unicum::ADC_SAMPLES;
  }
}

class Button {
  uint8_t pin=0;
  bool last=false,stable=false,longSent=false;
  uint32_t changed=0,pressed=0;
public:
  void begin(uint8_t p) { pin=p; pinMode(pin,INPUT_PULLUP); last=stable=(digitalRead(pin)==LOW); }
  // 1=corto al soltar; 2=largo una vez. Sin evento corto después del largo.
  uint8_t poll(uint32_t now) {
    bool down=digitalRead(pin)==LOW;
    if(down!=last) { last=down; changed=now; }
    if(now-changed>=30 && stable!=last) {
      stable=last;
      if(stable) { pressed=now; longSent=false; }
      else if(!longSent) return 1;
    }
    if(stable && !longSent && now-pressed>=1200) { longSent=true; return 2; }
    return 0;
  }
};
