#pragma once
#include <stdint.h>
// Exige LOW estable antes de HIGH; jamás arranca solo por encender con SIGNAL alto.
class StartGate {
  bool lowActive=false, highActive=false, qualified=false, fired=false;
  uint32_t lowSince=0,highSince=0;
public:
  bool update(uint32_t now,bool high) {
    if(!high) {
      if(!lowActive) { lowActive=true; lowSince=now; }
      highActive=false; fired=false;
      if(now-lowSince>=50) qualified=true;
      return false;
    }
    lowActive=false;
    if(!highActive) { highActive=true; highSince=now; }
    if(qualified && !fired && now-highSince>=20) {
      qualified=false; fired=true; return true;
    }
    return false;
  }
};
