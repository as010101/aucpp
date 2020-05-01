#ifndef RESTREAMQBOX_H
#define RESTREAMQBOX_H

#include "QBox.H"
#include "FieldExt.H"

#include "GroupByState.H"
#include "GroupByHash.H"

class RestreamQBox : public QBox
{
public:
  RestreamQBox();
  ~RestreamQBox() {};
  Box_Out_T doBox();
  void setModifier(const char* input);
  void setHash(GroupByHash *hash);

private:
  char          *_atts;
  int           _num_atts;
  int           _tuple_size;
  FieldExt      **_fields;
  int           _group_by_size;
  GroupByHash   *_group_hash;
  int           _sid_size;
  int           _ts_size;

};

#endif
