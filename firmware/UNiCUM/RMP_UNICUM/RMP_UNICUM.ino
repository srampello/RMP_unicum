/* UNICUM v0.2: C3 Super Mini + 74HC4067 + QRE12 + MicroStart + DIP de tres vias.
 * USB CDC On Boot enabled. S2/S3 en GPIO20/21 NO son UART en esta aplicación.
 * Control en tarea separada del servidor Wi-Fi; leer docs antes de conectar.
 */
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <math.h>
#include "Hardware.h"
#include "LineMath.h"
#include "StartGate.h"
#include "WebPage.h"

enum class Mode:uint8_t { STOPPED, ARMED, CALIBRATING, COUNTDOWN, RUNNING };
enum class Action:uint8_t { ARM, STOP, CAL, COLOR, PROFILE };
struct Command { Action action; int index=0,base=0,straight=0,maxPwm=0; float kp=0,kd=0; bool white=true; };
struct Profile { int base,straight,maxPwm; float kp,kd; };
struct Snapshot {
  Mode mode=Mode::STOPPED;
  bool calibrated=false,microHigh=false,motorReady=false,white=true;
  int selector=-1,active=0,left=0,right=0;
  uint16_t raw[12]={},signal[12]={},low[12]={},high[12]={},switchRaw[3]={},position=5500;
  Profile profiles[3]={};
  uint32_t cycleUs=0,overruns=0;
  const char* reason="Iniciando";
};
Snapshot published;
portMUX_TYPE snapshotLock=portMUX_INITIALIZER_UNLOCKED;
QueueHandle_t commandQueue=nullptr;
WebServer server(80);
Motors motors;
Preferences storage;
Button button;
StartGate startGate;
Snapshot state; // Solo la tarea de control modifica estado, ADC, motores y NVS.
bool nvsReady=false,autoArm=true;
uint32_t stateSince=0,lastSeen=0,lastSelector=0,selectorSince=0;
int selectorCandidate=-1;
float previousError=0,filteredDerivative=0,baseNow=0;
String serialBuffer;

const char* modeName(Mode mode) {
  switch(mode) {
    case Mode::ARMED:return "ARMADO";
    case Mode::CALIBRATING:return "CALIBRANDO";
    case Mode::COUNTDOWN:return "ESPERA_START";
    case Mode::RUNNING:return "MARCHA";
    default:return "DETENIDO";
  }
}
Snapshot getSnapshot() {
  portENTER_CRITICAL(&snapshotLock); Snapshot copy=published; portEXIT_CRITICAL(&snapshotLock); return copy;
}
void publish() {
  state.left=motors.left; state.right=motors.right;
  portENTER_CRITICAL(&snapshotLock); published=state; portEXIT_CRITICAL(&snapshotLock);
}
void halt(const char* reason,bool rearm=false) {
  motors.stop(); state.mode=Mode::STOPPED; state.reason=reason;
  autoArm=rearm; baseNow=0;
}
bool motionMode() { return state.mode==Mode::RUNNING || state.mode==Mode::COUNTDOWN; }
bool editable() { return !motionMode() && state.mode!=Mode::CALIBRATING && !state.microHigh; }
void arm() {
  if(!editable() || !state.calibrated || !state.motorReady || state.selector<0) {
    state.reason="Para armar: MicroStart LOW, calibracion valida y selector estable"; return;
  }
  motors.stop(); state.mode=Mode::ARMED; state.reason="Esperando START del MicroStart"; autoArm=false;
}
void calibrate(uint32_t now) {
  if(state.microHigh || motionMode()) { state.reason="Calibrar con MicroStart STOP y robot detenido"; return; }
  halt("Mover barra sobre blanco y negro durante 8 s"); state.calibrated=false;
  if(nvsReady)storage.putBool("valid",false);
  for(uint8_t i=0;i<12;i++) { state.low[i]=4095; state.high[i]=0; }
  state.mode=Mode::CALIBRATING; stateSince=now;
}
void saveProfiles() {
  if(!nvsReady) return;
  storage.putBytes("profiles",state.profiles,sizeof(state.profiles)); storage.putBool("white",state.white);
}
void execute(const Command& cmd,uint32_t now) {
  switch(cmd.action) {
    case Action::STOP:halt("Parada local/Web/USB; volver a armar para recibir START");break;
    case Action::ARM:arm();break;
    case Action::CAL:calibrate(now);break;
    case Action::COLOR:
      if(editable()) { state.white=cmd.white; saveProfiles(); state.reason="Polaridad guardada"; }
      else state.reason="Configuracion rechazada: detener MicroStart";
      break;
    case Action::PROFILE:
      if(editable() && cmd.index>=0 && cmd.index<3) {
        state.profiles[cmd.index]={cmd.base,cmd.straight,cmd.maxPwm,cmd.kp,cmd.kd};
        state.white=cmd.white; saveProfiles(); state.reason="Perfil guardado; seleccion por switch fisico";
      } else state.reason="Configuracion rechazada: detener MicroStart";
      break;
  }
}
void updateSelector(uint32_t now) {
  if(now-lastSelector<25) return;
  lastSelector=now;
  int candidate=readStrategy(state.switchRaw);
  if(candidate!=selectorCandidate) { selectorCandidate=candidate; selectorSince=now; state.selector=-1; }
  if(candidate<0) state.selector=-1;
  else if(now-selectorSince>=50) state.selector=candidate;
}
void controlStep() {
  const uint32_t now=millis(),started=micros();
  state.microHigh=digitalRead(Unicum::MICROSTART_PIN)==HIGH;
  bool startEdge=startGate.update(now,state.microHigh);
  // STOP no tiene antirrebote retardado; STBY además está conectado a SIGNAL.
  if(!state.microHigh && motionMode()) halt("STOP MicroStart",true);
  if(state.microHigh && state.mode==Mode::CALIBRATING) halt("Calibracion cancelada: MicroStart activo");
  if(digitalRead(Unicum::CAL_BUTTON)==LOW && motionMode()) halt("Parada por BOOT");
  uint8_t event=button.poll(now);
  if(event==2) calibrate(now);
  else if(event==1 && !state.microHigh) arm();
  Command cmd;
  for(uint8_t i=0;i<4 && xQueueReceive(commandQueue,&cmd,0)==pdTRUE;i++) execute(cmd,now);
  readSensors(state.raw); updateSelector(now);
  if(state.mode==Mode::CALIBRATING) {
    for(uint8_t i=0;i<12;i++) { state.low[i]=min(state.low[i],state.raw[i]); state.high[i]=max(state.high[i],state.raw[i]); }
    if(now-stateSince>=Unicum::CAL_MS) {
      state.calibrated=true;
      for(uint8_t i=0;i<12;i++) if(state.high[i]-state.low[i]<Unicum::MIN_CAL_SPAN) state.calibrated=false;
      if(nvsReady) {
        storage.putBool("valid",false);
        if(state.calibrated) {
          storage.putBytes("low",state.low,sizeof(state.low)); storage.putBytes("high",state.high,sizeof(state.high));
          storage.putBool("valid",true);
        }
      }
      halt(state.calibrated ? "Calibracion guardada":"Calibracion rechazada: revisar rangos",state.calibrated);
    }
  }
  for(uint8_t i=0;i<12;i++) state.signal[i]=state.calibrated ? LineMath::normalized(state.raw[i],state.low[i],state.high[i],state.white):0;
  bool seen=state.calibrated && LineMath::position(state.signal,12,Unicum::LINE_THRESHOLD,state.position);
  if(autoArm && state.mode==Mode::STOPPED && !state.microHigh && state.calibrated && state.selector>=0) arm();
  if(startEdge && state.mode==Mode::ARMED) {
    if(state.selector<0 || !seen) halt("START rechazado: selector invalido o sin linea");
    else {
      state.active=state.selector; state.mode=Mode::COUNTDOWN; stateSince=now;
      previousError=(int)state.position-5500; filteredDerivative=baseNow=0;
      state.reason="START MicroStart recibido";
    }
  }
  if(state.mode==Mode::COUNTDOWN && now-stateSince>=Unicum::START_DELAY_MS) {
    if(!seen || !state.microHigh) halt("START cancelado");
    else { state.mode=Mode::RUNNING; lastSeen=now; state.reason="Siguiendo linea"; }
  }
  if(state.mode==Mode::RUNNING) {
    if(!seen) {
      motors.stop(); baseNow=0;
      if(now-lastSeen>=Unicum::LOST_MS) halt("Linea perdida; requiere volver a armar");
    } else {
      if(now-lastSeen>20) { previousError=(int)state.position-5500; filteredDerivative=0; }
      lastSeen=now;
      const auto& p=state.profiles[state.active];
      float error=(int)state.position-5500,delta=error-previousError;
      previousError=error; filteredDerivative=0.7f*filteredDerivative+0.3f*delta;
      float correction=constrain(p.kp*error+p.kd*filteredDerivative,-(float)p.maxPwm,(float)p.maxPwm);
      int target=fabsf(error)<400 && fabsf(delta)<100 ? p.straight:p.base;
      baseNow=constrain((float)target,baseNow-2.0f,baseNow+0.8f);
      motors.set(constrain((int)(baseNow+correction),0,p.maxPwm),constrain((int)(baseNow-correction),0,p.maxPwm));
    }
  }
  state.cycleUs=micros()-started;
  if(state.cycleUs>Unicum::CONTROL_MS*1000) state.overruns++;
  publish();
}
void controlTask(void*) {
  TickType_t wake=xTaskGetTickCount();
  const TickType_t period=max((TickType_t)1,pdMS_TO_TICKS(Unicum::CONTROL_MS));
  for(;;) {
    controlStep();
    if(xTaskGetTickCount()-wake>=period) { wake=xTaskGetTickCount(); vTaskDelay(1); }
    else vTaskDelayUntil(&wake,period);
  }
}
bool enqueue(const Command& cmd) { return commandQueue && xQueueSend(commandQueue,&cmd,0)==pdTRUE; }
String statusJson() {
  Snapshot s=getSnapshot(); String json; json.reserve(2400);
  json="{\"mode\":\""; json+=modeName(s.mode); json+="\",\"reason\":\""; json+=s.reason;
  json+="\",\"calibrated\":";json+=s.calibrated ? "true":"false";
  json+=",\"microstart\":";json+=s.microHigh ? "true":"false";
  json+=",\"motorReady\":";json+=s.motorReady ? "true":"false";
  json+=",\"white\":";json+=s.white ? "true":"false";
  json+=",\"selector\":";json+=s.selector<0 ? 0:s.selector+1;
  json+=",\"active\":";json+=s.active+1;
  json+=",\"position\":";json+=s.position;
  json+=",\"leftPwm\":";json+=s.left;json+=",\"rightPwm\":";json+=s.right;
  json+=",\"cycleUs\":";json+=s.cycleUs;json+=",\"overruns\":";json+=s.overruns;
  const char* names[]={"raw","signal","low","high","switchRaw"};
  const uint16_t* arrays[]={s.raw,s.signal,s.low,s.high,s.switchRaw};
  for(uint8_t a=0;a<5;a++) {
    json+=",\"";json+=names[a];json+="\":[";
    for(uint8_t i=0;i<(a==4 ? 3:12);i++) { if(i)json+=",";json+=arrays[a][i]; }json+="]";
  }
  json+=",\"strategies\":[";
  for(uint8_t i=0;i<3;i++) {
    if(i)json+=",";const auto& p=s.profiles[i];
    json+="{\"name\":\"";json+=Unicum::STRATEGIES[i].name;
    json+="\",\"base\":";json+=p.base;json+=",\"straight\":";json+=p.straight;
    json+=",\"max\":";json+=p.maxPwm;json+=",\"kp\":";json+=String(p.kp,4);
    json+=",\"kd\":";json+=String(p.kd,4);json+="}";
  }json+="]}";return json;
}
bool parseNumber(const String& s,float& value) {
  if(!s.length())return false;char* end=nullptr;value=strtof(s.c_str(),&end);
  return end!=s.c_str() && *end=='\0' && isfinite(value);
}
void commandResponse(const Command& cmd) {
  server.send(enqueue(cmd) ? 202:503,"text/plain","Orden en cola; verificar estado en pantalla.");
}
void setupWeb() {
  server.on("/",HTTP_GET,[](){server.send_P(200,"text/html; charset=utf-8",WEB_PAGE);});
  server.on("/status",HTTP_GET,[](){server.send(200,"application/json",statusJson());});
  server.on("/arm",HTTP_POST,[](){Command cmd;cmd.action=Action::ARM;commandResponse(cmd);});
  server.on("/stop",HTTP_POST,[](){Command cmd;cmd.action=Action::STOP;commandResponse(cmd);});
  server.on("/cal",HTTP_POST,[](){Command cmd;cmd.action=Action::CAL;commandResponse(cmd);});
  server.on("/config",HTTP_POST,[](){
    float idx,b,s,m,kp,kd;
    if(!parseNumber(server.arg("strategy"),idx)||idx<1||idx>3||idx!=floorf(idx)||
       !parseNumber(server.arg("base"),b)||!parseNumber(server.arg("straight"),s)||
       !parseNumber(server.arg("max"),m)||!parseNumber(server.arg("kp"),kp)||!parseNumber(server.arg("kd"),kd)||
       b<0||b>s||s>m||m>255||kp<0||kp>0.2f||kd<0||kd>2||
       (server.arg("white")!="0" && server.arg("white")!="1")) {
      server.send(400,"text/plain","Valores invalidos: 0 <= base <= recta <= max <= 255.");return;
    }
    Snapshot ss=getSnapshot();
    if(ss.microHigh || ss.mode==Mode::RUNNING || ss.mode==Mode::COUNTDOWN || ss.mode==Mode::CALIBRATING) {
      server.send(409,"text/plain","Poner MicroStart en STOP y detener calibracion.");return;
    }
    Command cmd;cmd.action=Action::PROFILE;cmd.index=(int)idx-1;
    cmd.base=b;cmd.straight=s;cmd.maxPwm=m;cmd.kp=kp;cmd.kd=kd;cmd.white=server.arg("white")=="1";
    commandResponse(cmd);
  });
}
void usbCommand(String input) {
  input.trim();input.toLowerCase();Command cmd;
  if(input=="arm")cmd.action=Action::ARM;
  else if(input=="stop")cmd.action=Action::STOP;
  else if(input=="cal")cmd.action=Action::CAL;
  else if(input=="white"||input=="black") {cmd.action=Action::COLOR;cmd.white=input=="white";}
  else if(input=="status") {Serial.println(statusJson());return;}
  else if(input=="sensors") {
    Snapshot s=getSnapshot();
    for(uint8_t i=0;i<12;i++) Serial.printf("S%02u C%u raw=%u linea=%u min=%u max=%u\n",i+1,Unicum::SENSOR_CHANNELS[i],s.raw[i],s.signal[i],s.low[i],s.high[i]);
    Serial.printf("Selector=%d MicroStart=%d\n",s.selector+1,s.microHigh);return;
  } else {Serial.println("arm stop cal white black status sensors | arranque solo MicroStart");return;}
  Serial.println(enqueue(cmd) ? "Orden en cola; revisar status":"Cola llena");
}
void loadSettings() {
  state.motorReady=motors.begin();state.white=Unicum::DEFAULT_WHITE_LINE;
  for(uint8_t i=0;i<3;i++) { const auto& p=Unicum::STRATEGIES[i];state.profiles[i]={p.base,p.straight,p.maxPwm,p.kp,p.kd}; }
  nvsReady=storage.begin("unicum-c3-v2",false);
  if(!nvsReady)return;
  Profile saved[3];
  if(storage.getBytesLength("profiles")==sizeof(saved)) {
    storage.getBytes("profiles",saved,sizeof(saved));
    for(uint8_t i=0;i<3;i++) {
      auto& s=saved[i];
      if(s.base>=0&&s.base<=s.straight&&s.straight<=s.maxPwm&&s.maxPwm<=255&&
         isfinite(s.kp)&&s.kp>=0&&s.kp<=0.2f&&isfinite(s.kd)&&s.kd>=0&&s.kd<=2)state.profiles[i]=s;
    }
  }
  state.white=storage.getBool("white",Unicum::DEFAULT_WHITE_LINE);
  state.calibrated=storage.getBool("valid",false)&&storage.getBytesLength("low")==sizeof(state.low)&&storage.getBytesLength("high")==sizeof(state.high);
  if(state.calibrated) {
    storage.getBytes("low",state.low,sizeof(state.low));storage.getBytes("high",state.high,sizeof(state.high));
    for(uint8_t i=0;i<12;i++)if(state.high[i]>4095||state.low[i]>=state.high[i]||state.high[i]-state.low[i]<Unicum::MIN_CAL_SPAN)state.calibrated=false;
  }
}
void setup() {
  Serial.begin(115200);loadSettings();beginSensors();
  pinMode(Unicum::MICROSTART_PIN,INPUT_PULLDOWN);button.begin(Unicum::CAL_BUTTON);
  commandQueue=xQueueCreate(8,sizeof(Command));
  if(!commandQueue || xTaskCreate(controlTask,"unicum-control",6144,nullptr,3,nullptr)!=pdPASS) {
    motors.stop();Serial.println("ERROR creando control. Arranque bloqueado.");while(true)delay(100);
  }
  WiFi.mode(WIFI_AP);WiFi.setSleep(false);
  bool ap=WiFi.softAP(Unicum::AP_SSID,Unicum::AP_PASSWORD);
  setupWeb();if(ap)server.begin();
  Serial.println("UNICUM v0.2 C3 + MUX12 | USB CDC 115200 | http://192.168.4.1");
  if(!ap)Serial.println("AP fallo; control local MicroStart sigue disponible.");
}
void loop() {
  for(uint8_t i=0;i<32 && Serial.available();i++) {
    char c=Serial.read();
    if(c=='\n') {usbCommand(serialBuffer);serialBuffer="";}
    else if(c!='\r') {if(serialBuffer.length()<80)serialBuffer+=c;else serialBuffer="";}
  }
  server.handleClient();delay(2);
}
