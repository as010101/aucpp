#ifndef SORTEDVECTOR_H
#define SORTEDVECTOR_H

#include <exception>
#include <vector>
#include <map>
#include <algorithm>
#include <assert.h>

using namespace std;

template<typename TValue>
class SortedIndex
{
public:
  SortedIndex();
  virtual ~SortedIndex();

  // Sets the index map and the sorted multimap to both have the specified value 
  // for the given index. If they already contain an entry for the specified 
  // index, it's overridden.
  void set(size_t i, const TValue & v)
    throw (std::exception);

  // Ensures that the map and multimap have no entry for the specified index.
  void unset(size_t i)
    throw (std::exception);

  // If the specified key is in this container, then 'v' is set to that key's 
  // dependent data, and  the method returns 'true'.
  //
  // If the specified key is not in this container, then this method returns
  // 'false'.
  bool findByIndex(size_t k, TValue & v) const
    throw (std::exception);

  // Returns the actual map used by this object, so it's going to be updated
  // whenever you call 'set(...)'.
  const map<size_t, TValue> & getIndex() const
    throw (std::exception);

  // Returns the actual multimap used by this object, so it's going to be updated
  // whenever you call 'set(...)'. Elements are partially sorted according to 
  // TValue's "<" operator.
  const multimap<TValue, size_t> & getSortedMultimap() const
    throw (std::exception);

  // Returns the maximum index that's currently mapped. If there container is
  // empty, this throws an exception...
  size_t getMaxIndex() const
    throw (std::exception);

  // Returns the number of elements currently mapped...
  size_t size() const
    throw (std::exception);

private:
  // Private implementation that does what unset(...) does.
  void ensureUnset(size_t i)
    throw (std::exception);

  map<size_t, TValue> _index;
  multimap<TValue, size_t> _sorted;
};

//===============================================================================

template<typename TValue>
void SortedIndex<TValue>::set(size_t i, const TValue & v)
    throw (std::exception)
{
  ensureUnset(i);
  _index.insert(make_pair(i, v));
  _sorted.insert(make_pair(v, i));
}

//===============================================================================

template<typename TValue>
void SortedIndex<TValue>::unset(size_t i)
    throw (std::exception)
{
  ensureUnset(i);
}

//===============================================================================

template<typename TValue>
bool SortedIndex<TValue>::findByIndex(size_t k, TValue & v) const
    throw (std::exception)
{
  typename map<size_t, TValue>::const_iterator indexPos = _index.find(i);

  if (indexPos != _index.end())
    {
      v = indexPos->second;
      return true;
    }
  else
    {
      return false;
    }
}

//===============================================================================

template<typename TValue>
const map<size_t, TValue> & SortedIndex<TValue>::getIndex() const
  throw (std::exception)
{
  return _index;
}

//===============================================================================

template<typename TValue>
const multimap<TValue, size_t> & SortedIndex<TValue>::getSortedMultimap() const
  throw (std::exception)
{
  return _sorted;
}

//===============================================================================

template<typename TValue>
size_t SortedIndex<TValue>::getMaxIndex() const
  throw (std::exception)
{
  typename map<size_t, TValue>::reverse_iterator rPos = _index.rbegin();
  if (rPos == _index.rend())
    {
      throw SmException(__FILE__, __LINE__, "The container is empty");
    }

  return rPos->first;
}

//===============================================================================

template<typename TValue>
size_t SortedIndex<TValue>::size() const
  throw (std::exception)
{
  return _index.size();
}

//===============================================================================

template<typename TValue>
void SortedIndex<TValue>::ensureUnset(size_t i)
  throw (std::exception)
{
  typename map<size_t, TValue>::iterator indexPos = _index.find(i);
  const TValue & oldValue = indexPos->second;

  if (indexPos != _index.end())
    {
      multimapEraseExact<TValue, size_t>(_sorted, make_pair(oldValue, i));
      _index.erase(indexPos);
    }
}

#endif
