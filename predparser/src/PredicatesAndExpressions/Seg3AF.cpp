#include "Seg3AF.H"

#define ABSOLUTE_LAV_OFFSET 28
#define ABSOLUTE_CNT_OFFSET 32

Seg3AF::Seg3AF(const char *att) {
  _sum = 0;
}

Seg3AF::~Seg3AF() {}

void Seg3AF::init() {
  _sum = 0;
}

void Seg3AF::incr(char *tuple) {

  // extract lav and add that to sum, set lav (it should be the last attrib)
  unsigned int lav_offset = ABSOLUTE_LAV_OFFSET; // warning: depends on schema
  unsigned int lav;
  memcpy(&lav,tuple+lav_offset,sizeof(int));

  _sum += lav;
  _lav = lav;

  // extract count and set it here
  unsigned int cnt_offset = ABSOLUTE_CNT_OFFSET; // warning: depends on schema
  unsigned int cnt;
  memcpy(&cnt,tuple+cnt_offset,sizeof(int));
  _cnt = cnt;
   
}

char* Seg3AF::final() {
  // return lav, (sum/4) and cnt
  char *p = new char[3 * sizeof(int)];
  *(int*)(p) = _lav;
  *(int*)(p+sizeof(int)) = (_sum / 4);
  *(int*)(p+ 2 * sizeof(int)) = _cnt;
  return p;
}

char* Seg3AF::evaluate(char *tuple) {
  incr(tuple);
  return final();
}

int Seg3AF::getReturnedSize() { return (3 * sizeof(int)); }
Seg3AF* Seg3AF::makeNew() { return (new Seg3AF(NULL)); }
