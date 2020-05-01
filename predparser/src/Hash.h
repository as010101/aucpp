#ifndef HASH_H
#define HASH_H

#include <ext/hash_map>
using __gnu_cxx::hash_map;

#include "State.H"

struct my_hash
{
  size_t operator()(key_type* s) const
  {
    unsigned long h = 0;
    for (int i = 0; i < s->key_size; i++)
      h = 5*h + s->key[i];
    return size_t(h);
  }
};

struct eqstr
{
  bool operator()(const key_type* s1, const key_type* s2) const
  {
    if (s1->key_size != s2->key_size)
      return 0;
    return memcmp(s1->key, s2->key, s1->key_size) == 0;
  } 
};

class Hash
{
public:
  Hash();
  ~Hash();
  //pair<key_type*, State*> makePair(State *s);
  State* getState(char *value);
  void addState(State *s, char *value, bool create_key);
  void removeState(char *value);
  void changeState(State *s);
  int getSize();
  void setKeySize(int key_size);

  hash_map<key_type*, State*, my_hash, eqstr> getHashMap();
  hash_map<key_type*, State*, my_hash, eqstr>::iterator begin();
  hash_map<key_type*, State*, my_hash, eqstr>::iterator end();
  void printAll();

private:  
  int _key_size;

  hash_map<key_type*, State*, my_hash, eqstr>               _hash_map;    
  hash_map<key_type*, State*, my_hash, eqstr>::iterator     _iter;
  
};

#endif
