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

inline void selectMux(uint8_t channel) {
  for(uint8_t bit=0;bit<4;bit++) digitalWrite(Unicum::MUX_SELECT_PINS[bit],(channel>>bit)&1);
}
inline void beginSensors() {
  for(uint8_t p:Unicum::MUX_SELECT_PINS) { pinMode(p,OUTPUT); digitalWrite(p,LOW); }
  analogReadResolution(12); pinMode(Unicum::MUX_SIG,INPUT);
  analogSetPinAttenuation(Unicum::MUX_SIG,ADC_11db);
}
inline uint16_t readMux(uint8_t channel) {
  selectMux(channel); delayMicroseconds(Unicum::MUX_SETTLE_US);
  analogRead(Unicum::MUX_SIG);
  uint32_t sum=0;
  for(uint8_t i=0;i<Unicum::ADC_SAMPLES;i++) sum+=analogRead(Unicum::MUX_SIG);
  return sum/Unicum::ADC_SAMPLES;
}
inline void readSensors(uint16_t* raw) {
  for(uint8_t i=0;i<Unicum::SENSOR_COUNT;i++) raw[i]=readMux(Unicum::SENSOR_CHANNELS[i]);
}
inline int readStrategy(uint16_t* values) {
  int selected=-1;
  for(uint8_t i=0;i<3;i++) values[i]=readMux(Unicum::STRATEGY_CHANNELS[i]);
  for(uint8_t i=0;i<3;i++) {
    if(values[i]<Unicum::SELECTOR_LOW) { if(selected!=-1) return -1; selected=i; }
    else if(values[i]<Unicum::SELECTOR_HIGH) return -1;
  }
  return selected;
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
