#pragma once
#include <Arduino.h>

namespace Unicum {
constexpr uint8_t SENSOR_COUNT = 12;
// Izquierda -> derecha, mirando en la dirección de avance del robot.
constexpr uint8_t SENSOR_PINS[SENSOR_COUNT] = {36,39,34,35,32,33,25,26,27,14,13,4};
constexpr uint8_t BUTTON_PINS[3] = {5,15,23};
constexpr uint8_t AIN1=16, AIN2=17, PWMA=21; // Canal A = izquierdo
constexpr uint8_t BIN1=18, BIN2=19, PWMB=22; // Canal B = derecho
// STBY se habilita por jumper/interruptor a 3V3, con 10k a GND.
constexpr bool INVERT_LEFT=false, INVERT_RIGHT=false;
constexpr uint32_t PWM_HZ=20000, CONTROL_US=5000; // PD inicial a 200 Hz
constexpr uint8_t ADC_SAMPLES=4;
constexpr uint16_t MIN_CAL_SPAN=150, LINE_THRESHOLD=250;
constexpr uint32_t CAL_MS=8000, START_DELAY_MS=3000, LOST_MS=150;
constexpr bool ENABLE_BLUETOOTH=true;
constexpr char BT_NAME[]="RMP_UNICUM_BT";
constexpr char AP_SSID[]="RMP_UNICUM_SETUP";
constexpr char AP_PASSWORD[]="Unicum2026"; // Cambiar antes de utilizar.
// La línea blanca produce lectura ADC baja con el circuito de colector.
constexpr bool DEFAULT_WHITE_LINE=true;
struct Strategy { const char* name; int base, straight, maxPwm; float kp, kd; };
constexpr Strategy STRATEGIES[3] = {
  {"Conservadora",80,95,130,0.035f,0.10f},
  {"Equilibrada",105,120,170,0.040f,0.14f},
  {"Rapida",130,145,200,0.045f,0.18f}
};
// Semillas de puesta en marcha; NO son ajustes validados en este robot.
}
