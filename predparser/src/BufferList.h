#ifndef BUFFERLIST_H
#define BUFFERLIST_H

// DEPRECATED!!!
//#include <list.h>
#include <algobase.h>
#include <alloc.h>
#include <list>
using std::list;
#include "Timestamp.H"
#include <vector.h>

struct node
{
  char *att; // Null delimited
  char *tuple;

  bool operator==  (node other)
  {
    if (this->att != other.att)   // or is it MEMCMP?????????????
      return false;
    else if (this->tuple != other.tuple)
      return false;
    else
      return true;
  }
};

class BufferList
{
public:

  // first constructor for BSort
  BufferList(char *group_by, int group_by_size);
  
  // second constructor for join
  BufferList(char *group_by_values, int group_by_size, char *group_by_values_str, int group_by_size_str, char order_att_type);
  
  BufferList(char order_att_type);

~BufferList() {};

  // It would be good here to say "Why we use a list"
  // My stab:
  //  list gives constant time for inserts and removes. its only random access
  //   that is slowed down. since we really only care about the "smallest value" each time
  //   this works great for us -eddie
  list<node> _buffer;    // doubly linked list 

  int size();
  node front();
  node back();
  list<node>::iterator begin();
  list<node>::iterator end();
  
  bool isEmpty();
  void push_back(node new_node);
  void push_front(node new_node);
  void insert(list<node>::iterator iter, node new_node);
  node pop_front();

  char *getGroupByValues();
  int getGroupBySize();
  Timestamp getLastEmittedTime();
  void setLastEmittedTime(Timestamp ts);

  // NEW
  int getGroupByStrSize();
  char* getGroupByStr();

  int getSlack();
  void increaseSlack();
  void setSlack(int new_slack);
  char* getThreshold();
  void setThreshold(char *new_threshold);
  void increaseThreshold();
  int countAndClearVector();
  void addToVector(char *value);
  void erase(char *threshold, int size);
  
  void setTimeIncrement(long num);
  
private:
  
  //  int _max_size;
  char *_group_by_values;
  int _group_by_size;
  Timestamp _last_time_emitted;

  // new from here for join
  char  *_group_by_values_str;
  int   _group_by_size_str;

  char _order_att_type;

  int   _curr_slack;
  char  *_curr_threshold;

  int    _time_increment;
  //  long _hello;

  vector<char*> _values_gt_threshold;
  vector<char*>::iterator  _vector_iter;
  
};

#endif
