#include "Acc2AF.H"

Acc2AF::Acc2AF(const char *att) {
}

Acc2AF::~Acc2AF() {}

void Acc2AF::init() {
  _first = true;
  _acc = true;
}

void Acc2AF::incr(char *tuple) {

  if (_first) {

    // extract lane, set it
    unsigned int lane_offset = 12 + 20;
    unsigned int lane;
    memcpy(&lane,tuple+lane_offset, sizeof(int));
    _lane = lane;
    // extract car, set it
    unsigned int car_offset = 12;
    unsigned int car;
    memcpy(&car,tuple+car_offset, sizeof(int));
    _car = car;

    
    _first = false;
  } else {
    // extract stopped (last field)
    unsigned int stopped_offset = 12 + 24;
    unsigned int stopped;
    memcpy(&stopped,tuple+stopped_offset, sizeof(int));
    // extract car
    unsigned int car_offset = 12;
    unsigned int car;
    memcpy(&car,tuple+car_offset, sizeof(int));

    _acc = (_acc && (stopped) && (_car != car));

  }
   
}

char* Acc2AF::final() {
  // return lane, acc (retuned as an int)
  char *p = new char[2 * sizeof(int)];
  *(int*)(p) = _lane;
  *(int*)(p+sizeof(int)) = (int) _acc;
  return p;
}

char* Acc2AF::evaluate(char *tuple) {
  incr(tuple);
  return final();
}

int Acc2AF::getReturnedSize() { return (2 * sizeof(int)); }
Acc2AF* Acc2AF::makeNew() { return (new Acc2AF(NULL)); }
