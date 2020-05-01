#ifndef HASH_FOR_BUFFER_LIST_H
#define HASH_FOR_BUFFER_LIST_H

// DEPRECATED!! INCLUDE as I did below ...
//#include <hash_map.h>
#include <algobase.h> //deprec too, but dunno what to replace with
#include <ext/hash_map>
using __gnu_cxx::hash;
using __gnu_cxx::hash_map;

#include "BufferList.H"

/*
struct buffer_hash
{
  size_t operator()(key_type* s) const
  {
    unsigned long h = 0;
    for (int i = 0; i < s->key_size; i++)
      h = 5*h + s->key[i];
    return size_t(h);
  }
};
*/

struct equal_string
{
  bool operator()(const char *s1, const char *s2) const
  {
    return strcmp(s1, s2) == 0;
  } 
};


class HashForBufferList
{
public:
  HashForBufferList();
  ~HashForBufferList() {};

  BufferList* getBufferList(char *value);

  void addBufferList(BufferList *list);
  int getHashSize();
  int getBufferListSize(char *value);
  bool isEmpty();

  //***************************
  //is this method even necessary?
  //what arguments??????????????????
  void removeBufferList(); 
 


private:

  hash_map<char*, BufferList*, hash<const char*>, equal_string>      _hash_map;
  hash_map<char*, BufferList*, hash<const char*>, equal_string>::iterator   _hash_iter;

};

#endif
