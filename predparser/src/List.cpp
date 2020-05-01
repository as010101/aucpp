#include "List.H"
#include <iostream>

using namespace std;

List::List(char *sid, int size)
{
  _key = new key_type(sid, size);
  _sid = new char[size];
  memcpy(_sid, sid, size);
}

List::~List()
{
  delete _key;
  delete[] _sid;
}

list<State*> List::getList()
{
  return _list;
}

int List::getSize()
{
  return _list.size();
}

key_type* List::getKey()
{
  return _key;
}

void List::addState(State *s)
{
  _list.push_back(s);
}

State* List::getState(char *value, int size)
{
  list<State*>::iterator iter;
  iter = _list.begin();
  State *temp_state;
  bool found = false;

  while (iter != _list.end())
    {
      temp_state = *iter;
      if (memcmp(value, temp_state->getAttributeVals(), size) == 0)
	{
	  found = true;
	  break;
	}
      else
	iter++;
    }
  if (found)
    return temp_state;
  else 
    return NULL;
}




void List::removeState(State *s)
{
  //  printList();
 cout << "the size of list before is " << _list.size() << endl;
 list<State*>::iterator iter;
  iter = _list.begin();
  State *temp_state;
  bool found = false;

  cout << " looking for    " ; 
  s->printAll();

  while (iter != _list.end())
    {
      temp_state = *iter;
      cout << " currently looking at state    " ;
      temp_state->printAll();
      if (temp_state == s)
	{
	  cout << "FOUND !!!!!" << endl;
	  
	  found = true;
	  break;
	}
      else
	iter++;
    }
  if (found)
    _list.erase(iter);
  cout << "the size of list after is   " << _list.size() << endl;
}

State* List::first()
{
  return _list.front();
}

list<State*>::iterator List::begin()
{
  return _list.begin();
}

list<State*>::iterator List::end()
{
  return _list.end();
}


// obviously for debugging
void List::printList()
{
  list<State*>::iterator iter = _list.begin();
  State *s;
  while (iter != _list.end())
    {
      s = *iter;
      s->printAll();
      iter++;
    }
}
