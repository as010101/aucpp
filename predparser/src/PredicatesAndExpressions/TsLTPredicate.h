#ifndef TIMESTAMP_LESS_THAN_PREDICATE_H 
#define TIMESTAMP_LESS_THAN_PREDICATE_H 
 
#include "Predicate.H" 
 
class TsLTPredicate : public Predicate 
{ 
public: 
  TsLTPredicate (Expression *left_side, Expression *right_side); 
  TsLTPredicate(Expression *exp);   
  virtual ~TsLTPredicate(); 
  void setExpression(Expression *exp); 
  virtual bool evaluate(char *tuple);
  virtual bool evaluate(char *tuple1, char *tuple2); 
 
private: 
  Expression     *_ls; 
  Expression     *_rs; 
}; 
 
#endif
