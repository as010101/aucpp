#ifndef FILTERQBOX_H
#define FILTERQBOX_H

#include "QBox.H"
#include "Predicate.H"
#include "Parse.H"

class FilterQBox : public QBox
{
public:
  FilterQBox() {};  //  { _boxType = FILTER_BOX; }

  ~FilterQBox() {};

  virtual Box_Out_T doBox();
  void setBox(const char* modifier);
  void setPredicates(const char *modifier);


private:
  Predicate **_predicates;
  Predicate *_pred;
  int       _num_predicates;
  int       _tuple_size;
  bool _useFalsePort;
}; 

#endif
