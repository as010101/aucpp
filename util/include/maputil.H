#ifndef MAPUTIL_H
#define MAPUTIL_H

#include <map>
#include <exception>
#include <assert.h>

#include <iostream> // just for debugging

using namespace std;

/**
   Deletes all objects from the specified multimap that have a key of 'k'
   and a value of 'v'.

   Returns the number of objects deleted.
 */
template<typename TKey, typename TValue>
int multimapEraseExact(multimap<TKey, TValue> & mm, const pair<TKey, TValue> & p)
  throw (std::exception)
{
  int numErased = 0;

  pair<
    typename multimap<TKey, TValue>::iterator,
    typename multimap<TKey, TValue>::iterator > bounds = mm.equal_range(p.first);

  typename multimap<TKey, TValue>::iterator & eraserPos = bounds.first;

  while (eraserPos != bounds.second)
    {
      if ((eraserPos->second) == p.second)
	{
	  // Note: I'm not totally positive that eraserPos remains valid after we do
	  // this erase(...) operation, so if we're having trouble, verify this code...
	  typename multimap<TKey, TValue>::iterator victimPos = eraserPos;  
	  eraserPos++;
	  mm.erase(victimPos);
	  numErased++;
	}
      else
	{
	  eraserPos++;
	}
    }

  return numErased;
}


#endif
