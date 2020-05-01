#include <FieldExt.H>

FieldExt::FieldExt (int port, char type, int offset, int size) 
{
  _port = port;
  _offset = offset + 12;
  _size = size;
  _type = type;
}

FieldExt::FieldExt (const char* ident_string) 
{
  int x = 0;
  while (ident_string[x] != ':') 
    x++;
  x++;
  _port = atoi(ident_string + x);
  while (ident_string[x] != ':')
    x++;
  x++;
  _type = ident_string[x];
  x += 2;
  _offset = atoi(ident_string + x) + 12;
  while (ident_string[x] != ':') 
    x++;
  x++;
  // NOTE!!!!!!!!!!! I THINK _size SHOULD BE + 1 WHEN THE TYPE IS "s" (string)
  // SINCE evaluate() RETURNS THE STRING AND A NULL CHARACTER!
  // - eddie
  _size = atoi(ident_string + x);
}

FieldExt::~FieldExt() {}

char* FieldExt::evaluate(char* tuple1, char *tuple2) 
{
  if (_port == 0)
    return evaluate(tuple1);
  else
    return evaluate(tuple2);
}

char* FieldExt::evaluate(char* tuple) 
{
  char* result;
  if (_type == 's')
    result = new char [_size+1];
  else
    result = new char [_size];
  int i = 0;
  while (i < _size) {
    result[i] = tuple[_offset + i];
    i++;
  }
  if (_type == 's')
    result[_size] = '\0';
  return result;
}

// Will return as a stream of chars THAT ARE NOT NULL TERMINATED!!!
// It will also tell the size of the returned
char* FieldExt::evaluateAsChar(char* tuple, int &return_size)
{
  char* result;
  // For the string type, easy, we know the size in bytes, just copy
  if (_type == 's') 
    {
      result = new char [_size];
      memcpy(result,&tuple[_offset],_size);
    } 
  else 
    {
      // Why 64 ? precision of 6 for floats and the left
      //  number maximum of say 40 digits (see IEEE max float value
      //  is about 3.4 * 10^38)... so 64 chars should be enough
      result = (char*) malloc(64);
      int nchars;
      if (_type == 'i') 
	{
	  // See how many chars sprintf outputs. Note that sprintf does not count the \0
	  nchars = sprintf(result,"%d",*((int*) &tuple[_offset])); 
	} 
      else if (_type == 'f') 
	{
	  nchars = sprintf(result,"%.6f",*((float*) &tuple[_offset]));
	}
      else if (_type == 't')
	{
	  
	  nchars = sprintf(result, "%d,%d", ((timeval*) &tuple[_offset])->tv_sec, ((timeval*) &tuple[_offset])->tv_usec);
	}
      return_size = nchars;
      // Now take just what we need...
      result = (char*) realloc(result,nchars);
    }
  
  return result;

}
// NOTE!!!!!!!!!!! I THINK _size SHOULD BE + 1 WHEN THE TYPE IS "s" (string)
// SINCE evaluate() RETURNS THE STRING AND A NULL CHARACTER!
// NOTE 2: I HAVE SEEN, HOWEVER, ASSUMPTIONS IN OTHER PLACES OF CODE THAT ADD ONE
//  TO THE RETURN VALUE FROM HERE SO........
// BUT MY REASONING AND ANGER STANDS!!!!! evaluate() ADDS THE \0 !!! SO YOU MUST NOT
// LIE HERE BY SAYING 1 less THAN WHAT IT REALLY IS!!!!!!!!
int FieldExt::getReturnedSize()
{
  return _size;
}  

char FieldExt::getType()
{
  return _type;
}

void FieldExt::setPort(int i)
{
  _port = i;
}
