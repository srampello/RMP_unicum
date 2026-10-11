#pragma once
#include <stdint.h>

namespace LineMath {
inline uint16_t normalized(uint16_t raw, uint16_t low, uint16_t high, bool white) {
  if (high <= low) return 0;
  int32_t v=((int32_t)raw-low)*1000/(high-low);
  if(v<0) v=0;
  if(v>1000) v=1000;
  return white ? 1000-v : v;
}
// Pesos 0..11000; centro=5500. Devuelve false cuando no hay línea.
inline bool position(const uint16_t* values, uint8_t n, uint16_t threshold, uint16_t& pos) {
  uint32_t weighted=0, total=0;
  bool seen=false;
  for(uint8_t i=0;i<n;i++) {
    if(values[i]>=threshold) seen=true;
    if(values[i]>50) { total+=values[i]; weighted+=(uint32_t)values[i]*i*1000; }
  }
  if(!seen || !total) return false;
  pos=weighted/total;
  return true;
}
}
