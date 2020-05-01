#ifndef LIST_H
#define LIST_H

#include <list>
#include "State.H"

using namespace std;

class List
{
public:
  List(char *sid, int size);
  ~List();

  void addState(State *s);
  void removeState(State *s);
  list<State*> getList();
  int getSize();
  key_type* getKey();
  State* getState(char *value, int size);
  State* first();

  void printList();
  
  list<State*>::iterator begin();
  list<State*>::iterator end();
private:
  char            *_sid;
  key_type        *_key;
  list<State*>    _list;

};

#endif
