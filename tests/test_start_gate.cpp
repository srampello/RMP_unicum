#include <cassert>
#include "../firmware/UNiCUM/RMP_UNICUM/StartGate.h"
int main(){
 StartGate gate;
 assert(!gate.update(0,true));assert(!gate.update(500,true)); // HIGH al boot no arranca
 assert(!gate.update(600,false));assert(!gate.update(640,false));
 assert(!gate.update(650,false)); // LOW calificado
 assert(!gate.update(700,true));assert(!gate.update(719,true));
 assert(gate.update(720,true));assert(!gate.update(800,true)); // un solo flanco
 assert(!gate.update(900,false));assert(!gate.update(910,true));
 assert(!gate.update(1000,true)); // LOW demasiado corto no permite rearranque
 assert(!gate.update(1100,false));assert(!gate.update(1150,false));
 assert(!gate.update(1200,true));assert(gate.update(1220,true));
 StartGate wrap;
 assert(!wrap.update(UINT32_MAX-25,false));assert(!wrap.update(30,false));
 assert(!wrap.update(40,true));assert(wrap.update(60,true));
}
