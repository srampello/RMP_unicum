/* RMP UNICUM | ESP32 clásico WROOM | Arduino-ESP32 3.x
 * 12 entradas analógicas directas. Wi-Fi solo detenido, sin leer ADC2.
 * Leer docs/PUESTA_EN_MARCHA.md antes de energizar motores.
 */
#include <WiFi.h>
#include <WebServer.h>
#include <BluetoothSerial.h>
#include <Preferences.h>
#include "Hardware.h"
#include "LineMath.h"
#include "WebPage.h"
#include <math.h>

enum class Mode { STOPPED, CALIBRATING, COUNTDOWN, RUNNING, WIFI_SETUP };
Motors motors;
Button buttons[3];
BluetoothSerial bt;
WebServer server(80);
Preferences storage;
Mode mode=Mode::STOPPED;
uint16_t raw[12]={}, lowCal[12]={}, highCal[12]={}, signalValues[12]={};
uint16_t linePosition=5500;
bool calibrated=false, whiteLine=Unicum::DEFAULT_WHITE_LINE, motorReady=false;
bool nvsReady=false, btReady=false;
uint8_t strategyIndex=0;
Unicum::Strategy strategies[3];
struct SavedProfile { int base, straight, maxPwm; float kp, kd; };
void saveProfiles() {
  if(!nvsReady) return;
  SavedProfile saved[3];
  for(uint8_t i=0;i<3;i++) {
    const auto& p=strategies[i]; saved[i]={p.base,p.straight,p.maxPwm,p.kp,p.kd};
  }
  storage.putBytes("profiles",saved,sizeof(saved));
}
uint32_t stateSince=0, lastTick=0, lastSeen=0, lastLog=0;
uint32_t cycleUs=0, overruns=0;
float previousError=0, derivativeFiltered=0, baseNow=0;
bool pendingStart=false, pendingExitWifi=false;
String serialBuffer, btBuffer;

const char* modeName() {
  switch(mode) {
    case Mode::CALIBRATING: return "CALIBRANDO";
    case Mode::COUNTDOWN: return "CUENTA_ATRAS";
    case Mode::RUNNING: return "MARCHA";
    case Mode::WIFI_SETUP: return "WIFI_CONFIG";
    default: return "DETENIDO";
  }
}
void stopRobot() { motors.stop(); mode=Mode::STOPPED; baseNow=0; pendingStart=false; }
void exitWifi() {
  server.stop(); WiFi.softAPdisconnect(true); WiFi.mode(WIFI_OFF);
  // Esperar liberación del ADC2 con motores detenidos, fuera del lazo de control.
  delay(50); mode=Mode::STOPPED;
}
void saveCalibration() {
  if(!nvsReady) return;
  storage.putBool("valid",false);
  storage.putBytes("low",lowCal,sizeof(lowCal)); storage.putBytes("high",highCal,sizeof(highCal));
  storage.putBool("valid",true);
}
void beginCalibration() {
  motors.stop();
  if(mode==Mode::WIFI_SETUP) exitWifi();
  calibrated=false;
  for(uint8_t i=0;i<12;i++) { lowCal[i]=4095; highCal[i]=0; }
  mode=Mode::CALIBRATING; stateSince=millis();
  Serial.println("Mover manualmente TODA la barra sobre blanco y negro durante 8 s.");
}
void startRobot() {
  if(!motorReady || !calibrated || mode==Mode::CALIBRATING) {
    Serial.println("Inicio rechazado: calibrar 12 sensores y comprobar PWM."); return;
  }
  motors.stop();
  if(mode==Mode::WIFI_SETUP) exitWifi();
  mode=Mode::COUNTDOWN; stateSince=millis();
  previousError=derivativeFiltered=baseNow=0;
  Serial.println("Inicio en 3 segundos. Wi-Fi apagado.");
}
String statusJson() {
  String s="{\"mode\":\""; s+=modeName();
  s+="\",\"calibrated\":"; s+=calibrated ? "true":"false";
  s+=",\"strategy\":"; s+=strategyIndex+1;
  s+=",\"white\":"; s+=whiteLine ? "true":"false";
  s+=",\"motorReady\":"; s+=motorReady ? "true":"false";
  s+=",\"position\":"; s+=linePosition;
  s+=",\"leftPwm\":"; s+=motors.left; s+=",\"rightPwm\":"; s+=motors.right;
  s+=",\"cycleUs\":"; s+=cycleUs;
  s+=",\"overruns\":"; s+=overruns;
  s+=",\"sensorsAvailable\":"; s+=(mode==Mode::WIFI_SETUP) ? "false":"true";
  s+=",\"strategies\":[";
  for(uint8_t i=0;i<3;i++) {
    if(i) s+=",";
    const auto& p=strategies[i];
    s+="{\"name\":\""; s+=p.name;
    s+="\",\"base\":"; s+=p.base; s+=",\"straight\":"; s+=p.straight;
    s+=",\"max\":"; s+=p.maxPwm; s+=",\"kp\":"; s+=String(p.kp,4);
    s+=",\"kd\":"; s+=String(p.kd,4); s+="}";
  }
  s+="]}"; return s;
}
bool parseNumber(const String& s,float& v) {
  if(!s.length()) return false;
  char* end=nullptr; v=strtof(s.c_str(),&end);
  return end!=s.c_str() && *end=='\0' && isfinite(v);
}
void enterWifi() {
  if(mode==Mode::WIFI_SETUP) return;
  stopRobot(); mode=Mode::WIFI_SETUP; // Detiene lectura antes de activar radio.
  WiFi.mode(WIFI_AP);
  if(!WiFi.softAP(Unicum::AP_SSID,Unicum::AP_PASSWORD)) {
    WiFi.mode(WIFI_OFF); mode=Mode::STOPPED; Serial.println("Error creando AP"); return;
  }
  server.begin(); Serial.print("Wi-Fi configuración: http://"); Serial.println(WiFi.softAPIP());
}
void setupWeb() {
  server.on("/",HTTP_GET,[](){ server.send_P(200,"text/html; charset=utf-8",WEB_PAGE); });
  server.on("/status",HTTP_GET,[](){ server.send(200,"application/json",statusJson()); });
  server.on("/config",HTTP_POST,[](){
    float idx,b,s,m,kp,kd;
    if(!parseNumber(server.arg("strategy"),idx) || idx<1 || idx>3 || idx!=floorf(idx) ||
       !parseNumber(server.arg("base"),b) || !parseNumber(server.arg("straight"),s) ||
       !parseNumber(server.arg("max"),m) || !parseNumber(server.arg("kp"),kp) ||
       !parseNumber(server.arg("kd"),kd) || b<0 || b>s || s>m || m>255 ||
       kp<0 || kp>0.2f || kd<0 || kd>2 ||
       (server.arg("white")!="0" && server.arg("white")!="1")) {
      server.send(400,"text/plain","Valores invalidos. 0 <= base <= recta <= max <= 255."); return;
    }
    strategyIndex=(uint8_t)idx-1;
    auto& p=strategies[strategyIndex]; p.base=(int)b; p.straight=(int)s; p.maxPwm=(int)m; p.kp=kp; p.kd=kd;
    whiteLine=server.arg("white")=="1";
    if(nvsReady) { saveProfiles(); storage.putBool("white",whiteLine); }
    server.send(200,"text/plain","Guardado. Wi-Fi se apaga al iniciar.");
  });
  server.on("/start",HTTP_POST,[](){
    if(!calibrated || !motorReady) { server.send(409,"text/plain","Calibrar primero por botones/USB/Bluetooth."); return; }
    server.send(200,"text/plain","Inicio en 3 s. Se cerrara el Wi-Fi."); pendingStart=true;
  });
  server.on("/exit",HTTP_POST,[](){ server.send(200,"text/plain","Volviendo a sensores, detenido."); pendingExitWifi=true; });
}
void handleCommand(String cmd,Stream& out) {
  cmd.trim(); cmd.toLowerCase();
  if(cmd=="stop") { if(mode==Mode::WIFI_SETUP) exitWifi(); stopRobot(); out.println("Detenido"); }
  else if(cmd=="cal") { beginCalibration(); out.println("Calibracion manual 8 s"); }
  else if(cmd=="start") { startRobot(); out.println(modeName()); }
  else if(cmd=="wifi") { enterWifi(); out.println("http://192.168.4.1 (sensores suspendidos)"); }
  else if(cmd=="exit") { if(mode==Mode::WIFI_SETUP) exitWifi(); out.println(modeName()); }
  else if(cmd=="1" || cmd=="2" || cmd=="3") {
    if(mode==Mode::RUNNING || mode==Mode::COUNTDOWN) out.println("Detener antes de cambiar estrategia");
    else { strategyIndex=cmd.toInt()-1; out.println(strategies[strategyIndex].name); }
  }
  else if(cmd=="white" || cmd=="black") {
    if(mode==Mode::RUNNING || mode==Mode::COUNTDOWN) out.println("Detener primero");
    else { whiteLine=cmd=="white"; if(nvsReady) storage.putBool("white",whiteLine); out.println("Polaridad guardada"); }
  }
  else if(cmd=="status") out.println(statusJson());
  else if(cmd=="sensors") {
    if(mode==Mode::WIFI_SETUP) out.println("No disponibles: ADC2 ocupado por Wi-Fi");
    else if(mode==Mode::RUNNING || mode==Mode::COUNTDOWN) out.println("Detener para imprimir sensores");
    else {
      for(uint8_t i=0;i<12;i++) { out.printf("S%02u raw=%u linea=%u min=%u max=%u\n",i+1,raw[i],signalValues[i],lowCal[i],highCal[i]); }
    }
  }
  else out.println("Comandos: cal start stop 1 2 3 white black wifi exit status sensors");
}
void readCommands(Stream& in,String& buffer) {
  // Máximo 32 caracteres por iteración: evita monopolizar el control.
  for(uint8_t i=0;i<32 && in.available();i++) {
    char c=in.read();
    if(c=='\n') { handleCommand(buffer,in); buffer=""; }
    else if(c!='\r') { if(buffer.length()<80) buffer+=c; else buffer=""; }
  }
}
void controlTick(uint32_t now) {
  if(mode==Mode::WIFI_SETUP) return; // Nunca leer ADC2 con Wi-Fi activo.
  uint32_t started=micros(); readSensors(raw);
  if(mode==Mode::CALIBRATING) {
    for(uint8_t i=0;i<12;i++) { lowCal[i]=min(lowCal[i],raw[i]); highCal[i]=max(highCal[i],raw[i]); }
    if(now-stateSince>=Unicum::CAL_MS) {
      calibrated=true;
      for(uint8_t i=0;i<12;i++) {
        if(highCal[i]-lowCal[i]<Unicum::MIN_CAL_SPAN) { calibrated=false; Serial.printf("S%u sin contraste suficiente\n",i+1); }
      }
      if(calibrated) saveCalibration();
      mode=Mode::STOPPED; Serial.println(calibrated ? "Calibracion guardada":"Calibracion rechazada");
    }
  }
  for(uint8_t i=0;i<12;i++) signalValues[i]=calibrated ? LineMath::normalized(raw[i],lowCal[i],highCal[i],whiteLine):0;
  bool seen=calibrated && LineMath::position(signalValues,12,Unicum::LINE_THRESHOLD,linePosition);
  if(mode==Mode::COUNTDOWN && now-stateSince>=Unicum::START_DELAY_MS) {
    if(!seen) { stopRobot(); Serial.println("Inicio cancelado: colocar barra sobre la linea"); }
    else { mode=Mode::RUNNING; stateSince=lastSeen=now; previousError=(int)linePosition-5500; }
  }
  if(mode==Mode::RUNNING) {
    if(!seen) {
      motors.stop(); baseNow=0; // Frenar inmediato; no buscar a ciegas.
      if(now-lastSeen>=Unicum::LOST_MS) { stopRobot(); Serial.println("Detenido: linea perdida"); }
    } else {
      bool reacquired=now-lastSeen>20; lastSeen=now;
      auto& p=strategies[strategyIndex];
      float error=(int)linePosition-5500;
      if(reacquired) { previousError=error; derivativeFiltered=0; }
      float delta=error-previousError; previousError=error;
      derivativeFiltered=0.7f*derivativeFiltered+0.3f*delta;
      float correction=p.kp*error+p.kd*derivativeFiltered;
      correction=constrain(correction,-(float)p.maxPwm,(float)p.maxPwm);
      int target=abs((int)error)<400 && fabsf(delta)<100 ? p.straight:p.base;
      baseNow=constrain((float)target,baseNow-2.0f,baseNow+0.8f);
      // Error positivo = línea a derecha: acelerar izquierda / reducir derecha.
      int l=constrain((int)(baseNow+correction),0,p.maxPwm);
      int r=constrain((int)(baseNow-correction),0,p.maxPwm);
      motors.set(l,r);
    }
  }
  cycleUs=micros()-started;
  if(cycleUs>Unicum::CONTROL_US) overruns++;
}
void setup() {
  Serial.begin(115200);
  motorReady=motors.begin();
  WiFi.mode(WIFI_OFF); beginSensors();
  for(uint8_t i=0;i<3;i++) { strategies[i]=Unicum::STRATEGIES[i]; buttons[i].begin(Unicum::BUTTON_PINS[i]); }
  nvsReady=storage.begin("unicum-v1",false);
  if(nvsReady) {
    // No persistir punteros de nombres: cargar solo campos numéricos.
    SavedProfile saved[3];
    if(storage.getBytesLength("profiles")==sizeof(saved)) {
      storage.getBytes("profiles",saved,sizeof(saved));
      for(uint8_t i=0;i<3;i++) {
        auto& s=saved[i];
        if(s.base>=0 && s.base<=s.straight && s.straight<=s.maxPwm && s.maxPwm<=255 &&
           isfinite(s.kp) && s.kp>=0 && s.kp<=0.2f && isfinite(s.kd) && s.kd>=0 && s.kd<=2) {
          strategies[i].base=s.base; strategies[i].straight=s.straight; strategies[i].maxPwm=s.maxPwm;
          strategies[i].kp=s.kp; strategies[i].kd=s.kd;
        }
      }
    }
    whiteLine=storage.getBool("white",Unicum::DEFAULT_WHITE_LINE);
    calibrated=storage.getBool("valid",false) && storage.getBytesLength("low")==sizeof(lowCal) && storage.getBytesLength("high")==sizeof(highCal);
    if(calibrated) {
      storage.getBytes("low",lowCal,sizeof(lowCal)); storage.getBytes("high",highCal,sizeof(highCal));
      for(uint8_t i=0;i<12;i++) if(highCal[i]>4095 || lowCal[i]>=highCal[i] || highCal[i]-lowCal[i]<Unicum::MIN_CAL_SPAN) calibrated=false;
    }
  }
  if(Unicum::ENABLE_BLUETOOTH) btReady=bt.begin(Unicum::BT_NAME);
  setupWeb();
  Serial.println("RMP UNICUM v0.1 | detenido | 115200 baud | help para comandos");
  if(!motorReady) Serial.println("Error PWM: inicio bloqueado");
}
void loop() {
  uint32_t now=millis();
  // Procesar botones antes de imprimir/comunicar.
  for(uint8_t i=0;i<3;i++) {
    uint8_t event=buttons[i].poll(now);
    if(!event) continue;
    if(mode==Mode::RUNNING || mode==Mode::COUNTDOWN || mode==Mode::CALIBRATING) { stopRobot(); continue; }
    if(event==1) { strategyIndex=i; startRobot(); }
    else if(i==0) beginCalibration();
    else if(i==1) enterWifi();
    else { if(mode==Mode::WIFI_SETUP) exitWifi(); stopRobot(); }
  }
  readCommands(Serial,serialBuffer);
  if(btReady) readCommands(bt,btBuffer);
  if(mode==Mode::WIFI_SETUP) {
    server.handleClient();
    if(pendingStart) { pendingStart=false; startRobot(); }
    else if(pendingExitWifi) { pendingExitWifi=false; exitWifi(); }
  }
  uint32_t t=micros();
  if(t-lastTick>=Unicum::CONTROL_US) { lastTick=t; controlTick(millis()); }
  // Sin streaming en marcha: imprimir comandos/status puede alterar temporización.
  delay(1);
}
