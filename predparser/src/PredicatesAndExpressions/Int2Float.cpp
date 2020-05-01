#include "Int2Float.H"
 
Int2Float::Int2Float(Expression *expr)
{
  _expr = expr;
}

Int2Float::~Int2Float() {}
 
char* Int2Float::evaluate(char *tuple)
{
  char* temp = _expr->evaluate(tuple);
  int tmp1 = *(int*)(temp);
  float tmp2 = (float) tmp1;
  char* result = new char[sizeof(float)];
  *(float*)(result) = tmp2;
  delete[] temp;
  return  result;
}
 
char* Int2Float::evaluate(char *tuple1, char *tuple2)
{
  char* temp = _expr->evaluate(tuple1, tuple2);
  int tmp1 = *(int*)(temp);
  float tmp2 = (float) tmp1;
  char* result = new char[sizeof(float)];
  *(float*)(result) = tmp2;
  delete[] temp;
  return  result;
}
 
Int2Float::Int2Float()
{

}
 
void Int2Float::setExpression(Expression *expr)
{
  _expr = expr;
}

int Int2Float::getReturnedSize()
{
  return sizeof(float);
}
