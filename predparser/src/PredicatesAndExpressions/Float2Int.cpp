#include "Float2Int.H"
 
Float2Int::Float2Int(Expression *expr)
{
  _expr = expr;
}

Float2Int::~Float2Int() {}
 
char* Float2Int::evaluate(char *tuple)
{
  char *valuec = _expr->evaluate(tuple);
  float *valuef = (float *)valuec;
  char *resultc = new char[sizeof(int)];
  int *resulti = (int *)resultc;
  *resulti = (int)(*valuef);
  delete[] valuec;
  return resultc;
}
 
char* Float2Int::evaluate(char *tuple1, char *tuple2)
{
  char *valuec = _expr->evaluate(tuple1, tuple2);
  float *valuef = (float *)valuec;
  char *resultc = new char[sizeof(int)];
  int *resulti = (int *)resultc;
  *resulti = (int)(*valuef);
  delete[] valuec;
  return resultc;
}
 
Float2Int::Float2Int()
{

}
 
void Float2Int::setExpression(Expression *expr)
{
  _expr = expr;
}

int Float2Int::getReturnedSize()
{
  return sizeof(int);
}
