#include "HashForAggregate.H"

HashForAggregate::HashForAggregate()
{
  _hash_of_hash = hash_map<char*, HashForNewState*, hash<const char*>, equal_string>();
}

void HashForAggregate::addHash(HashForNewState *dahash)
{
  
  char* group_by_value = (char*) malloc(strlen(dahash->getGroupByValuesStr())+1);
  strcpy(group_by_value,dahash->getGroupByValuesStr());

  //  int group_by_size = dahash->getGroupBySizeStr();
  //char *group_by_value = new char[group_by_size];
  //memcpy(group_by_value, dahash->getGroupByValuesStr(), group_by_size);

  _hash_of_hash.insert(pair<char*, HashForNewState*> (group_by_value, dahash));
  return;
}

HashForNewState* HashForAggregate::getHash(char *group_by_values)
{
  _hash_iter = _hash_of_hash.find(group_by_values);
  
  if (_hash_iter != _hash_of_hash.end())
    return (*_hash_iter).second;
  else
    {
      //cout << "[AggregateQBox] [HashForAggregate] No hash for group by [" << group_by_values << "]" << endl;
      return NULL;
    }
}
  
bool HashForAggregate::isEmpty()
{
  return _hash_of_hash.empty();
}

int HashForAggregate::getHashSize()
{
  return _hash_of_hash.size();
}

hash_map<char*, HashForNewState*, hash<const char*>, equal_string> HashForAggregate::getHash()
{
  return _hash_of_hash;
}

hash_map<char*, HashForNewState*, hash<const char*>, equal_string>::iterator HashForAggregate::begin()
{
  return _hash_of_hash.begin();
}

hash_map<char*, HashForNewState*, hash<const char*>, equal_string>::iterator HashForAggregate::end()
{
  return _hash_of_hash.end();
}

