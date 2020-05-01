#ifndef ACC1AF_H
#define ACC1AF_H

#include "AggregateFunction.H"
#include "FieldExt.H"

class Acc1AF : public AggregateFunction {
public:
  Acc1AF(const char *att);
  virtual ~Acc1AF();
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char *tuple2) {};
  int getReturnedSize();
  Acc1AF* makeNew();
private:
  bool _first;
  bool _stopped;
  int _lane;
  int _time; // or timeval if it becomes that
  int _pos_e; // expressway
  int _pos_s; // segment
  int _pos_d; // direction
};

#endif //  ACC1AF_H
