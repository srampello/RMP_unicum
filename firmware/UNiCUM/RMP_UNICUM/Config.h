#pragma once
#include <Arduino.h>
#if !defined(CONFIG_IDF_TARGET_ESP32C3)
#error "UNICUM v0.2 requiere ESP32-C3. Seleccionar ESP32C3 Dev Module."
#endif
#if !ARDUINO_USB_CDC_ON_BOOT
#error "Habilitar USB CDC On Boot: GPIO20/21 se usan para el MUX, no UART0."
#endif
namespace Unicum {
constexpr uint8_t SENSOR_COUNT=12;
constexpr uint8_t SENSOR_CHANNELS[SENSOR_COUNT]={0,1,2,3,4,5,6,7,8,9,10,11};
constexpr uint8_t MUX_SELECT_PINS[4]={2,8,20,21}; // S0..S3
constexpr uint8_t MUX_SIG=0; // ADC1_CH0: lectura con Wi-Fi activo
constexpr uint8_t STRATEGY_CHANNELS[3]={12,13,14}; // Selector 1P3T, común a GND
constexpr uint8_t MICROSTART_PIN=3, CAL_BUTTON=9; // BOOT, activo LOW
constexpr uint8_t AIN1=4, AIN2=5, PWMA=1;
constexpr uint8_t BIN1=6, BIN2=7, PWMB=10;
// STBY <- SIGNAL MicroStart (3.3 V), con interruptor en serie y 10k a GND.
constexpr bool INVERT_LEFT=false, INVERT_RIGHT=false;
constexpr uint32_t PWM_HZ=20000, CONTROL_MS=5;
constexpr uint8_t ADC_SAMPLES=2;
constexpr uint16_t MUX_SETTLE_US=120;
constexpr uint16_t MIN_CAL_SPAN=150, LINE_THRESHOLD=250;
constexpr uint16_t SELECTOR_LOW=500, SELECTOR_HIGH=2500;
constexpr uint32_t CAL_MS=8000, START_DELAY_MS=0, LOST_MS=150;
constexpr bool DEFAULT_WHITE_LINE=true;
constexpr char AP_SSID[]="RMP_UNICUM";
constexpr char AP_PASSWORD[]="Unicum2026";
struct Strategy { const char* name; int base, straight, maxPwm; float kp, kd; };
constexpr Strategy STRATEGIES[3]={
 {"Conservadora",80,95,130,0.035f,0.10f},
 {"Equilibrada",105,120,170,0.040f,0.14f},
 {"Rapida",130,145,200,0.045f,0.18f}
}; // Valores iniciales de prueba, pendientes de pista y ruedas.
}
