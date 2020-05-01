#ifndef HASH_FOR_NEW_STATE_H
#define HASH_FOR_NEW_STATE_H

//#include <hash_map.h>
#include "NewState.H"
#include "HashForBufferList.H"  // for "equal_string" define in that class
using __gnu_cxx::hash;
using __gnu_cxx::hash_map;
#include <vector.h>

class HashForNewState
{
public:

  HashForNewState(char *group_by_values, int group_by_size, char *group_by_values_str,  char order_att_type);
  HashForNewState(char order_att_type);
  ~HashForNewState() {};

  void addState(NewState *s);
  void removeState(NewState *s);
  NewState* getState(char *value);
  void removeState(char *value);    // if we have to remove state given *value*

  int getHashSize();
  bool isEmpty();
  int getGroupBySize();
  char* getGroupByValues();
  //  int getGroupBySizeStr();
  char* getGroupByValuesStr();

  bool getFirstTupleDone();
  void setFirstTupleDone();

  char* getLastVal();
  void setLastVal(char *val);
  
  int getSlack();
  void increaseSlack();
  void setSlack(int new_slack);
  char* getThreshold();
  void setThreshold(char *new_threshold);
  void increaseThreshold();

  int countAndClearVector();
  void addToVector(char *value);

  void setTimeIncrement(long num);

  hash_map<char*, NewState*, hash<const char*>, equal_string> getHash();

  // to go through the Hash Table
  hash_map<char*, NewState*, hash<const char*>, equal_string>::iterator begin();
  hash_map<char*, NewState*, hash<const char*>, equal_string>::iterator end();
  
  
  vector<char*>* getTupleStore();
  vector<NewState*>* getTupleStoreStates();
  void clearTupleStore(int num);
  void addToTupleStore(char *tuple,NewState *ns);

private:

  char     *_group_by_values;
  int      _group_by_size;
  char     *_group_by_values_str;

  bool      _first_tuple_done;   // this has to be stored 
                            // the first tuple determines how the window is created
  char    _order_att_type;

  char    *_last_val;

  vector<char*>   _values_more_than_threshold;   
  vector<char*>::iterator   _vector_iter;

  int       _curr_slack;
  char      *_curr_threshold;

  long     _time_increment;

  hash_map<char*, NewState*, hash<const char*>, equal_string>     _hash_with_states;
  hash_map<char*, NewState*, hash<const char*>, equal_string>::iterator   _hash_iter;


  vector<char*>  *_tuple_store;
  // ARG!! You also need to keep the aggregate functions around man!!!
  // well, just keep the NewState objects then, that solves it all
  vector<NewState*> *_tuple_store_states;
};

#endif
