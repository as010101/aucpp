#ifndef SORTLIST_H
#define SORTLIST_H

#include <ext/hash_map>
using __gnu_cxx::hash_map;

#include "State.H"
//#include "tupleGenerator.H"
#include <list>

using namespace std;

struct buffer_node
{
  char *tuple;
  Timestamp ts;

  bool  operator== ( buffer_node node1)   //, const buffer_node node2) 
  {
    if (node1.tuple != this->tuple)  //node2.tuple)
      return false;
    else if (!(node1.ts == this->ts))   // operator!= not defined for Timestamp class
      return false;                     // so have to do it the long way  
    else
      return true;
  }
};

class SortList
{
public:
  SortList();
  ~SortList();
  list<buffer_node>        _buffer;   //doubly linked list ASCENDING order
  int size();
  buffer_node front();
  buffer_node back();
  list<buffer_node>::iterator begin();
  list<buffer_node>::iterator end();
  void push_back(buffer_node new_node);
  void push_front(buffer_node new_node);
  void pop_front();
  void insert(list<buffer_node>::iterator iter, buffer_node node);
  /*  State* getState(char *value);
  void addState(State *s, char *value, bool create_key);
  void removeState(char *value);
  void changeState(State *s);
  int getSize();
  void setKeySize(int key_size);

  hash_map<key_type*, State*, my_hash, eqstr> getHashMap();
  hash_map<key_type*, State*, my_hash, eqstr>::iterator begin();
  hash_map<key_type*, State*, my_hash, eqstr>::iterator end();
  void printAll();*/

private:  /*
  int _key_size;

  hash_map<key_type*, State*, my_hash, eqstr>               _hash_map;    
  hash_map<key_type*, State*, my_hash, eqstr>::iterator     _iter;
	  */
};

#endif
