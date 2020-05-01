#ifndef TOPNMULTIMAP_H
#define TOPNMULTIMAP_H

#include <exception>
#include <map>
#include <algorithm>
#include <iterator>
#include "StorageMgr_Exceptions.H"
#include "maputil.H"

using namespace std;

// Provides a pair of multimaps that divide objects according to 
// whether or not an object meets the top-Nth ranking (where N is
// set a constuction time of this container).
//
// An object A is closer to the "top" of the ranking than another
// object B if A > B.
// 
// If two objects have an equivalent priority ( (!(A<B)) && (!(B<A)) ), and
// exactly one of A or B can fit into the top N group, then the first of those 
// objects added to this container will be in the top N group. That will remain 
// true until something causes that object to be ejected from the top N: Either 
// (1) an object with higher priority is inserted into this container, or
// (2) the object in question is erased from this container.

// *** FUTURE IMPROVEMENT POSSIBILITIES ***
// This uses a 

template<typename TKey, typename TValue>
class TopNMultimap
{
public:
  // n must be >= 0.
  TopNMultimap(unsigned int n)
    throw (std::exception);

  // Adds an object with the specified key and value into the apprioriate
  // multimap. Will push some other object from the Top multimap to the Bottom 
  // multimap if apprioate.
  //
  // If there's already an object in the map with the specified key, it's
  // overwritten with the new value.
  void insert(const pair<TKey, TValue> & p)
    throw (std::exception);

  // Removes all objects with the specified (key, value) pair from this 
  // container. This will move some object's from the Bottom multimap
  // into the top multimap if apprioriate.
  void erase(const TKey & p)
    throw (std::exception);

  // If the specified key is in this container, then...
  //    'd' is set to that key's dependent data, and 
  //    'inTop' is set to 'true' if the key is in the top group and
  //       'false' if it's in the bottom group, and
  //    the method returns 'true'.
  //
  // If the specified key is not in this container, then this method returns
  // false.
  bool findByKey(const TKey & k, TValue & d, bool & inTop) const
    throw (std::exception);

  // Gives a reference to the multimap container that contains the top N
  // objects. This is a reference to the TopNMultimap's live container, 
  // so the reference's data is never stale.
  const multimap<TValue, TKey> & getTopMultimap() const
    throw (std::exception);
 
  // Gives a reference to the multimap container that contains the objects that
  // aren't in this container but aren't in the top N ranking. This is a 
  // reference to the TopNMultimap's live container,  so the reference's data is 
  // never stale.
  const multimap<TValue, TKey> & getBottomMultimap() const
    throw (std::exception);

private:
  unsigned int _n;

  struct IndexContent
  {
    IndexContent(const TValue & v, char whichPool)
      : _v(v),
	_whichPool(whichPool)
    {}

    // The TValue data that the user supplied to go along with the key.
    TValue _v;

    // Either 't' (the object is stored in _topObjects) or 'b' (the object is
    // stored in _bottomObjects).
    char _whichPool;
  };

  // This information is indexed by key to enable fast lookups.
  // The char value is 't' or 'b', indicating which pool the object's data
  // can be found in.
  map<TKey, IndexContent> _index;

  // These multimaps are used to retain sorting on the objects so that we can
  // quickly calculate whether or not an object is in the top(n) and also so
  // that our users can get the sorted data for their own uses...
  multimap<TValue, TKey> _topObjects;    // Those objects in the top N
  multimap<TValue, TKey> _bottomObjects; // Those objects not in the top N

  // Ensures the specified key isn't anywhere in this data structure.
  // If it had to be removed from _topObjects, this returns 't'.
  // If it had to be removed from _bottomObjects, this returns 'b'.
  // If it wasn't in this data structure, this returns 'a' (absent).
  char ensureKeyDeleted(const TKey & k)
    throw (std::exception);

  // Deletes the specified key from the specified pool ('b' or 't'). The 
  // specified pool must contain an entry with the specified key.
  //
  // This also removes the key's entry from _index.
  void deleteKeyFromPool(const TKey & k, char whichPool)
    throw (std::exception);

  // Deletes the specified entry into the specified pool ('b' or 't'). The 
  // specified pool must not contain an entry with the specified key.
  //
  // This also add the key's entry into _index.
  void insertKeyIntoPool(const TKey & k, const TValue & v, char whichPool)
    throw (std::exception);

  // Moves the specified key and its data from the oldPool to the newPool.
  // The specified key must exist in the oldPool when this is called.
  //
  // This makes the appropriate changes to _index, _bottomPool, and _topPool.
  //
  // The pools are desigated 't' (top) and 'b' (bottom).
  void migrateKey(const TKey & k, char oldPool, char newPool)
    throw (std::exception);
};

//===============================================================================

template<typename TKey, typename TValue>
TopNMultimap<TKey, TValue>::TopNMultimap(unsigned int n)
  throw (std::exception)
{
  _n = n;
}

//===============================================================================

template<typename TKey, typename TValue>
void TopNMultimap<TKey, TValue>::insert(const pair<TKey, TValue> & p)
  throw (std::exception)
{
  const TKey & keyToInsert = p.first;
  const TValue & valueToInsert = p.second;

  char oldKeyLocation = ensureKeyDeleted(keyToInsert);

  size_t topSize = _topObjects.size();
  
  if (topSize < _n)
    {
      // We know there's no need to shuffle up an existing datum from 
      // _bottomObjects to _topObjects, so we can just go ahead and make our
      // insertion into _topObjects...
      insertKeyIntoPool(keyToInsert, valueToInsert, 't');
      return;    
    }

  if (oldKeyLocation == 't')
    {
      // p has to compete with a top-ranked element from _bottomObjects to get
      // into the vacancy that we created in _topObjects when we called
      // ensureKeyDeleted(p)...

      typename multimap<TValue, TKey>::reverse_iterator bottomPos =
	_bottomObjects.rbegin();

      // Use >=, not >, to minimize the reshuffling in ambiguous cases...
      if ((valueToInsert) >= (bottomPos->first))
	{
	  insertKeyIntoPool(keyToInsert, valueToInsert, 't');
	  return;
	}
      else
	{
	  migrateKey(bottomPos->second, 'b', 't');
	  insertKeyIntoPool(keyToInsert, valueToInsert, 'b');
	  return;
	}     
    }

  // _topObjects hasn't been perturbed, and is full. We need to have p compete
  // with a lowest-ranked datum in _topObjects to see who gets to occupy that
  // data structure. The lower goes to _bottomObjects...
  typename multimap<TValue, TKey>::iterator topPos = _topObjects.begin();

  // Use >, not >=, to minimize the reshuffling in ambiguous cases...
  if (valueToInsert > topPos->first)
    {
      migrateKey(topPos->second, 't', 'b');
      insertKeyIntoPool(keyToInsert, valueToInsert, 't');
      return;
    }
  else
    {
      insertKeyIntoPool(keyToInsert, valueToInsert, 'b');
      return;
    }
}

//===============================================================================

template<typename TKey, typename TValue>
void TopNMultimap<TKey, TValue>::erase(const TKey & p)
  throw (std::exception)
{
  char oldKeyLocation = ensureKeyDeleted(p);

  if ((oldKeyLocation != 't') || (_bottomObjects.size() == 0))
    {
      return;
    }

  // We need to migrate a highest-ranked object from _bottomPool to _topPool to
  // fill the vacancy we just made...
  typename multimap<TValue, TKey>::iterator pos = _bottomObjects.end();
  --pos;
  migrateKey(pos->second, 'b', 't');
}

//===============================================================================

template<typename TKey, typename TValue>
bool TopNMultimap<TKey, TValue>::findByKey(const TKey & k, 
					   TValue & d, 
					   bool & inTop) const
  throw (std::exception)
{
  typename map<TKey, IndexContent>::const_iterator posIndex = _index.find(k);

  if (posIndex == _index.end())
    {
      return false;
    }

  d = posIndex->second._v;
  inTop = (posIndex->second._whichPool == 't');
  return true;
}

//===============================================================================

template<typename TKey, typename TValue>
const multimap<TValue, TKey> & TopNMultimap<TKey, TValue>::getTopMultimap() const
  throw (std::exception)
{
  return _topObjects;
}

//===============================================================================
 
template<typename TKey, typename TValue>
const multimap<TValue, TKey> & TopNMultimap<TKey, TValue>::getBottomMultimap() const
  throw (std::exception)
{
  return _bottomObjects;
}

//===============================================================================

template<typename TKey, typename TValue>
char TopNMultimap<TKey, TValue>::ensureKeyDeleted(const TKey & k)
  throw (std::exception)
{
  typename map<TKey, IndexContent>::iterator posIndex = _index.find(k);

  if (posIndex == _index.end())
    {
      return 'a';
    }

  char whichPool = posIndex->second._whichPool;
  deleteKeyFromPool(k, whichPool);
  return whichPool;
}

//===============================================================================

template<typename TKey, typename TValue>
void TopNMultimap<TKey, TValue>::deleteKeyFromPool(const TKey & k, char whichPool)
  throw (std::exception)
{
  // Discover its dependent value, so we can constrain our search through the
  // multimap when doing our delete...
  typename map<TKey, IndexContent>::iterator posIndex = _index.find(k);
  if (posIndex == _index.end())
    {
      throw SmException(__FILE__, __LINE__, "The specified key wasn't found in _index");
    }

  const TValue & v = posIndex->second._v;

  // Note that in the calls to make_pair(...) below, what our users think of as 
  // the 'key' and 'data' are swapped. This is because we're about to work on
  // _topObjects or _bottomObjects, which have also swapped the users' notions of
  // key/data...

  int numDeleted = 777;

  if (whichPool == 't')
    {
      numDeleted = multimapEraseExact(_topObjects, make_pair(v, k));
    }
  else if (whichPool == 'b')
    {
      numDeleted = multimapEraseExact(_bottomObjects, make_pair(v, k));
    }
  else
    {
      throw SmException(__FILE__, __LINE__, "Invalid value supplied for 'whichPool'");
    }

  if (numDeleted != 1)
    {
      throw SmException(__FILE__, __LINE__, "An invalid number of entries was deleted.");
    }

  _index.erase(posIndex);
}

//===============================================================================

template<typename TKey, typename TValue>
void TopNMultimap<TKey, TValue>::insertKeyIntoPool(const TKey & k, const TValue & v, char whichPool)
  throw (std::exception)
{
  // Note that in the calls to make_pair(...) below, what our users think of as 
  // the 'key' and 'data' are swapped. This is because we're about to work on
  // _topObjects or _bottomObjects, which have also swapped the users' notions of
  // key/data...

  if (whichPool == 't')
    {
      _index.insert(make_pair(k, IndexContent(v, 't')));
      _topObjects.insert(make_pair(v, k));
    }
  else if (whichPool = 'b')
    {
      _index.insert(make_pair(k, IndexContent(v, 'b')));
      _bottomObjects.insert(make_pair(v, k));
    }
  else
    {
      throw SmException(__FILE__, __LINE__, "Invalid value supplied for 'whichPool'");
    }
}

//===============================================================================

template<typename TKey, typename TValue>
void TopNMultimap<TKey, TValue>::migrateKey(const TKey & k, 
					    char oldPool, 
					    char newPool)
  throw (std::exception)
{
  // Find some stuff out...
  typename map<TKey, IndexContent>::iterator posIndex = _index.find(k);
  if (posIndex == _index.end())
    {
      throw SmException(__FILE__, __LINE__, "The specified key wasn't found in _index");
    }

  const TValue & v = posIndex->second._v;

  // Move the pair to the correct multimap...
  pair<TValue, TKey> mmPair(v, k);

  if ((oldPool == 'b') && (newPool = 't'))
    {
      multimapEraseExact(_bottomObjects, mmPair);
      _topObjects.insert(mmPair);
    }
  else if ((oldPool == 't') && (newPool = 'b'))
    {
      multimapEraseExact(_topObjects, mmPair);
      _bottomObjects.insert(mmPair);
    }
  else
    {
      throw SmException(__FILE__, __LINE__, "Invalid pair of values supplied "
				  "for 'oldPool' and 'newPool'");
    }

  // Update the index...
  posIndex->second._whichPool = newPool;
}

#endif


