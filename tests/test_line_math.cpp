#include <cassert>
#include "../firmware/UNiCUM/RMP_UNICUM/LineMath.h"
int main() {
  using namespace LineMath;
  assert(normalized(300,300,3500,true)==1000);
  assert(normalized(3500,300,3500,true)==0);
  assert(normalized(1900,300,3500,true)==500);
  assert(normalized(0,300,3500,false)==0);
  assert(normalized(4095,300,3500,false)==1000);
  assert(normalized(1000,1000,1000,true)==0);
  uint16_t v[12]={},p=5500;
  assert(!position(v,12,250,p) && p==5500);
  v[5]=v[6]=1000; assert(position(v,12,250,p) && p==5500);
  v[5]=v[6]=0; v[0]=1000; assert(position(v,12,250,p) && p==0);
  v[0]=0; v[11]=1000; assert(position(v,12,250,p) && p==11000);
  v[11]=249; assert(!position(v,12,250,p));
  v[11]=250; assert(position(v,12,250,p));
}
