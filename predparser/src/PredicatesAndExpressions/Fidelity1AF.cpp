#include "Fidelity1AF.H"

#define FE_CHARS 3 // length of feed and exchange in chars
Fidelity1AF::Fidelity1AF(const char *att) {
}

Fidelity1AF::~Fidelity1AF() {}

void Fidelity1AF::init() {
  _cnt = 0;
}

void Fidelity1AF::incr(char *tuple) {
  // increment a counter
  ++_cnt;
 // store the feed and exchange from the tuple, for later emission
  feed_exchange = (char*) malloc(FE_CHARS); // malloc enough
  memcpy(feed_exchange,tuple + 12 + 8, FE_CHARS); // get this right!
 }

char* Fidelity1AF::final() {
  //cout << " FINAL : _cnt is " << _cnt << endl;
  char *ret = new char[sizeof(int) + FE_CHARS];
  *(int*)(ret) = (_cnt == 1) ? 1 : 0;
  // and now slap the feed and exchange we stored
  memcpy(ret + sizeof(int),feed_exchange,FE_CHARS);
  return ret;
}

char* Fidelity1AF::evaluate(char *tuple) {
  incr(tuple);
  return final();
}

int Fidelity1AF::getReturnedSize() { return (sizeof(int) + FE_CHARS); }
Fidelity1AF* Fidelity1AF::makeNew() { return (new Fidelity1AF(NULL)); }
