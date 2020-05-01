#ifndef GROUP_BY_BOX_H
#define GROUP_BY_BOX_H

#include "FieldExt.H"

class GroupByBox 
{
public:
  GroupByBox(char **atts, int num_atts, int tuple_size);
  ~GroupByBox() {};
  char* doBox(char *tuple);

private:
  char       **_atts;
  int        _num_atts;
  int        _tuple_size;
  int        _new_tuple_size;
  FieldExt   **_fields;

};

#endif
