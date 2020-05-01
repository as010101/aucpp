#ifndef HASH_WITH_LIST_H
#define HASH_WITH_LIST_H

#include <ext/hash_map>
using __gnu_cxx::hash_map;

#include "List.H"
#include "Hash.H"

class HashWithList
{
public:
  HashWithList();
  ~HashWithList();
  List* getList(char *value); 
  void addList(List *l);
  void removeList(char *value);
  int getSize();
  void setKeySize(int key_size);

  hash_map<key_type*, List*, my_hash, eqstr>::iterator begin();
  hash_map<key_type*, List*, my_hash, eqstr>::iterator end();


private:
  int         _key_size;   // key for this has will 
                           // will be sid
  hash_map<key_type*, List*, my_hash, eqstr>              _hash_map;
  hash_map<key_type*, List*, my_hash, eqstr>::iterator    _iter;
 

};

#endif
