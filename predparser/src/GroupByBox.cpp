#include "GroupByBox.H"

GroupByBox::GroupByBox(char **atts, int num_atts, int tuple_size) 
{
  //atts are a list of attributes that are being grouped
  _atts = atts;
  _num_atts = num_atts;
  _tuple_size = tuple_size;
  _new_tuple_size = tuple_size;

  _fields = new FieldExt*[_num_atts];
  int a_offset;
  int a_size;
  char a_type;
  char *curr_string;
  for (int i = 0; i < _num_atts; i++) 
{
    curr_string = atts[i];
    int x = 1;
    while (curr_string[x] != ':') 
      x++;
    x++;
    a_type = curr_string[x];
    x += 2;
    a_offset = atoi(curr_string + x);
    while (curr_string[x] != ':') 
      x++;
    x++;
    a_size = atoi(curr_string + x);
    _new_tuple_size += a_size + 1; //the plus 1 is for the $ delimiters

    // cjc commented this out on January 25 so the code would compile. FieldExt
    // has no 3-arg c'tor.
    //    _fields[i] = new FieldExt(a_type, a_offset, a_size);
}
  _new_tuple_size--;
}

char* GroupByBox::doBox(char *tuple) 
{
  char *new_tuple = new char[_new_tuple_size + 1];
  for (int i = 0; i <= _new_tuple_size; i++) 
    new_tuple[i] = '\0';
  for (int i = 0; i < _num_atts; i++) {
    strcat(new_tuple, _fields[i]->evaluate(tuple));
    if (i != _num_atts-1)
      strcat(new_tuple, "$");
  }
  strcat(new_tuple, tuple);
  return new_tuple;
}



