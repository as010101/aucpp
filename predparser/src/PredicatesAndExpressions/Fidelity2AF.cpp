#include "Fidelity2AF.H"


Fidelity2AF::Fidelity2AF(const char *att) {
  _cnt = 0;
}

Fidelity2AF::~Fidelity2AF() {}

void Fidelity2AF::init() {
  _cnt = 0;
}

void Fidelity2AF::incr(char *tuple) {
  ++_cnt;
}

char* Fidelity2AF::final() {
  char *ret = new char[sizeof(int)];
  *(int*)(ret) = 1; // 1 means "alarm!"
  return ret;
}

char* Fidelity2AF::evaluate(char *tuple) {
  incr(tuple);
  return final();
}

int Fidelity2AF::getReturnedSize() { return (sizeof(int)); }
Fidelity2AF* Fidelity2AF::makeNew() { return (new Fidelity2AF(NULL)); }
