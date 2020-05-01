#ifndef GROUP_BY_STATE_H
#define GROUP_BY_STATE_H

#include "State.H"

class GroupByState
{
public:
  GroupByState(char *group_by_values, int size, int sid);
  ~GroupByState();
  char* getGroupByValues();
  int getNewSid();
  key_type* getKey();

private:
  key_type   *_key;
  char       *_group_by_values;
  int        _new_sid;

};

#endif
