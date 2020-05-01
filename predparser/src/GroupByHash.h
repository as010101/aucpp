#ifndef GROUP_BY_HASH_H
#define GROUP_BY_HASH_H

#include <ext/hash_map>
using __gnu_cxx::hash_map;

#include "GroupByState.H"
#include "Hash.H"

class GroupByHash
{
public:
  //  GroupByHash(int);
  GroupByHash();
  ~GroupByHash() {};
  pair<key_type*, GroupByState*> makePair(GroupByState *l);
  GroupByState* getGroupByState(char *value);
  void addGroupByState(GroupByState *s);
  void removeGroupByState(char *value);
  int getSize();

  void setKeySize(int key_size);

  hash_map<key_type*, GroupByState*, my_hash, eqstr> getGroupByHashMap();

  void printAll();
  hash_map<key_type*, GroupByState*, my_hash, eqstr>::iterator begin();
  hash_map<key_type*, GroupByState*, my_hash, eqstr>::iterator end();

private:
  int       _key_size;
  hash_map<key_type*, GroupByState*, my_hash, eqstr>               _group_hash_map;    
  hash_map<key_type*, GroupByState*, my_hash, eqstr>::iterator     _iter;
};

#endif




