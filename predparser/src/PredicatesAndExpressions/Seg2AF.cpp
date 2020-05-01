#include "Seg2AF.H"
#include <iostream>

#define ABSOLUTE_SUM_OFFSET 28
#define ABSOLUTE_CNT_OFFSET 32

Seg2AF::Seg2AF(const char *att) {
  _cnt = 0;
  _sum = 0;
  _totcnt = 0;
}

Seg2AF::~Seg2AF() {}

void Seg2AF::init() {
  _cnt = 0;
  _sum = 0;
  _totcnt = 0;
}

void Seg2AF::incr(char *tuple) {
  // extract speed and add that to sum
  unsigned int sum_offset = ABSOLUTE_SUM_OFFSET; // warning: depends on schema
  unsigned int sum;
  memcpy(&sum,tuple+sum_offset,sizeof(int));
  _sum += sum;

  // extract count and set it
  unsigned int cnt_offset = ABSOLUTE_CNT_OFFSET; // warning: depends on schema
  unsigned int cnt;
  memcpy(&cnt,tuple+cnt_offset,sizeof(int));
  _cnt = cnt;
 
  _totcnt += cnt;
}

char* Seg2AF::final() {
  // return (sum/cnt) and cnt
  char *p = new char[sizeof(int) + sizeof(int)];
  //  cout << " SEG2 - FINAL CALLED WITH (_sum, _cnt) : ("<<_sum<<","<<_cnt<<")"<<endl;
  *(int*)(p) = (_totcnt == 0) ? 0 : (_sum / _totcnt);
  *(int*)(p+sizeof(int)) = _cnt;
  return p;
}

char* Seg2AF::evaluate(char *tuple) {
  incr(tuple);
  return final();
}

int Seg2AF::getReturnedSize() { return (2 * sizeof(int)); }
Seg2AF* Seg2AF::makeNew() { return (new Seg2AF(NULL)); }
