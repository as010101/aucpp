#include "Seg1AF.H"

#define ABSOLUTE_SPEED_OFFSET 40

Seg1AF::Seg1AF(const char *att) {
  _cnt = 0;
  _sum = 0;
}

Seg1AF::~Seg1AF() {}

void Seg1AF::init() {
  _cnt = 0;
  _sum = 0;
}

void Seg1AF::incr(char *tuple) {
  ++_cnt;
  // extract speed and add that to sum (it should be the last attrib)
  unsigned int speed_offset = ABSOLUTE_SPEED_OFFSET; // warning: depends on schema!
  unsigned int speed;
  memcpy(&speed,tuple+speed_offset,sizeof(int));
  _sum += speed;
  
}

char* Seg1AF::final() {
  // return both sum and cnt
  char *p = new char[sizeof(int) + sizeof(int)];
  *(int*)(p) = _sum;
  *(int*)(p+sizeof(int)) = _cnt;
  return p;
}

char* Seg1AF::evaluate(char *tuple) {
  incr(tuple);
  return final();
}

int Seg1AF::getReturnedSize() { return (2 * sizeof(int)); }
Seg1AF* Seg1AF::makeNew() { return (new Seg1AF(NULL)); }
