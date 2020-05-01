#include "Acc1AF.H"

Acc1AF::Acc1AF(const char *att) {
}

Acc1AF::~Acc1AF() {}

void Acc1AF::init() {
  _first = true;
  _stopped = true;
}

void Acc1AF::incr(char *tuple) {

  if (_first) {
    // extract time, set it
    unsigned int time_offset = 12 + 8;
    unsigned int time;
    memcpy(&time,tuple+time_offset, sizeof(int));
    _time = time;
    // extract pos, set it (3 parts!)
    unsigned int pos; // a temp dude

    unsigned int pos_e_offset = 12 + 12;
    memcpy(&pos,tuple+pos_e_offset, sizeof(int));
    _pos_e = pos;

    unsigned int pos_s_offset = 12 + 16;
    memcpy(&pos,tuple+pos_s_offset, sizeof(int));
    _pos_s = pos;

    unsigned int pos_d_offset = 12 + 20;
    memcpy(&pos,tuple+pos_d_offset, sizeof(int));
    _pos_d = pos;


    // extract lane, set it
    unsigned int lane_offset = 12 + 24;
    unsigned int lane;
    memcpy(&lane,tuple+lane_offset, sizeof(int));
    _lane = lane;
    
    _first = false;
  } else {
    // extract pos (all 3 parts)
    unsigned int pos_e, pos_s, pos_d; // temp dudes

    unsigned int pos_e_offset = 12 + 12;
    memcpy(&pos_e,tuple+pos_e_offset, sizeof(int));

    unsigned int pos_s_offset = 12 + 16;
    memcpy(&pos_s,tuple+pos_s_offset, sizeof(int));

    unsigned int pos_d_offset = 12 + 20;
    memcpy(&pos_d,tuple+pos_d_offset, sizeof(int));

    // extract lane
    unsigned int lane_offset = 12 + 24;
    unsigned int lane;
    memcpy(&lane,tuple+lane_offset, sizeof(int));

    _stopped = (_stopped &&
		(_pos_e == pos_e) &&
		(_pos_s == pos_s) &&
		(_pos_d == pos_d) &&
		(_lane == lane));

  }
   
}

char* Acc1AF::final() {
  /** NOTE: DOCUMENTATION SAYS final outputs these three, but then
      the actual output schema seems to be missing lane */
  // return pos, lane, stopped (retuned as an int)
  char *p = new char[5 * sizeof(int)];
  *(int*)(p) = _pos_e;
  *(int*)(p+ sizeof(int)) = _pos_s;
  *(int*)(p+ 2 * sizeof(int)) = _pos_d;
  *(int*)(p+ 3 * sizeof(int)) = _lane;
  *(int*)(p+ 4 * sizeof(int)) = (int) _stopped; // from bool to int, hoping 0 and 1
  return p;
}

char* Acc1AF::evaluate(char *tuple) {
  incr(tuple);
  return final();
}

int Acc1AF::getReturnedSize() { return (3 * sizeof(int)); }
Acc1AF* Acc1AF::makeNew() { return (new Acc1AF(NULL)); }
