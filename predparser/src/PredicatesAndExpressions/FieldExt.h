#ifndef FIELD_EXT_H
#define FIELD_EXT_H

#include "Function.H"

class FieldExt : public Function
{
public:
  FieldExt (int port, char type, int offset, int size);
  FieldExt(const char* ident_string);
  virtual ~FieldExt();
  virtual char* evaluate(char *tuple); 
  virtual char* evaluateAsChar(char *tuple, int &return_size); 
  virtual char* evaluate(char *tuple1, char *tuple2); 
  int getReturnedSize(); 
  char getType();
  void setPort(int i);

private:
  int   _port;
  char  _type;
  int   _offset;
  int   _size;
};

#endif
