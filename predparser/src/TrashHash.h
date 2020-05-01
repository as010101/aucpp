#ifndef TRASH_HASH_H
#define TRASH_HASH_H

#include <ext/hash_map>
using __gnu_cxx::hash_map;

#include "TrashState.H"
#include "Hash.H"

class TrashHash
{
public:
  TrashHash();
  ~TrashHash();
  pair<key_type*, TrashState*> makePair(TrashState *l);
  TrashState* getTrashState(char *value);
  void addTrashState(TrashState *s);
  void removeTrashState(char *value);
  int getSize();
  void setKeySize(int key_size);

  void printAll();
  hash_map<key_type*, TrashState*, my_hash, eqstr>::iterator begin();
  hash_map<key_type*, TrashState*, my_hash, eqstr>::iterator end();

private:
  int _key_size;
  hash_map<key_type*, TrashState*, my_hash, eqstr>               _trash_hash_map;    
  hash_map<key_type*, TrashState*, my_hash, eqstr>::iterator     _iter;
};

#endif




