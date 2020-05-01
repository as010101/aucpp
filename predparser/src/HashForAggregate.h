#ifndef HASH_FOR_AGGREGATE_H
#define HASH_FOR_AGGREGATE_H

#include "HashForNewState.H"
#include "NewState.H"
using __gnu_cxx::hash;
using __gnu_cxx::hash_map;

class HashForAggregate
{
public:

  HashForAggregate();
  ~HashForAggregate() {};

  void addHash(HashForNewState *hash);
  HashForNewState* getHash(char *group_by_values);
  
  bool isEmpty();
  int getHashSize();

  hash_map<char*, HashForNewState*, hash<const char*>, equal_string> getHash();

  // to go through the Hash Table
  hash_map<char*, HashForNewState*, hash<const char*>, equal_string>::iterator begin();
  hash_map<char*, HashForNewState*, hash<const char*>, equal_string>::iterator end();
  
  
private:

  hash_map<char*, HashForNewState*, hash<const char*>, equal_string>  _hash_of_hash;
  hash_map<char*, HashForNewState*, hash<const char*>, equal_string>::iterator  _hash_iter;

};

#endif
