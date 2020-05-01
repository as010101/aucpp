#ifndef NOT_PREDICATE_H
#define NOT_PREDICATE_H

#include "Predicate.H"

class NOTPredicate : public Predicate
{
public:
  NOTPredicate(Predicate *child);
  virtual ~NOTPredicate();
  virtual bool evaluate(char *tuple);
  virtual bool evaluate(char *tuple1, char *tuple2);

private:
  Predicate   *_child;
};

#endif   
