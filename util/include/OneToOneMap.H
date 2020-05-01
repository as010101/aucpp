#ifndef ONETOONEMAP_H
#define ONETOONEMAP_H

#include <map>

using namespace std;

// Your basic 1:1 map container...

template<typename T1, typename T2>
class OneToOneMap<T1, T2>
{
public:
  OneToOneMap();
  virtual ~OneToOneMap();

  // Overwrite on conflict with either key, don't barf.
  void insert(const T1 & aT1, const T2 & aT2)
    throw (exception);

  // Ensures the bimap contains no record whose T1 value is aT1.
  void clearByT1(const T1 & aT1)
    throw (exception);

  // Ensures the bimap contains no record whose T1 value is aT2.
  void clearByT2(const T2 & aT2)
    throw (exception);

  const map<T1, T2> & getT1T2Map() const;
  const map<T2, T1> & getT2T1Map() const;

private:
  map<T1, T2> _map12;
  map<T2, T1> _map21;
};

//===============================================================================

template<typename T1, typename T2>
OneToOneMap<T1, T2>::OneToOneMap() {}

//===============================================================================

template<typename T1, typename T2>
OneToOneMap<T1, T2>::~OneToOneMap() {}

//===============================================================================

template<typename T1, typename T2>
void OneToOneMap<T1, T2>::insert(const T1 & aT1, const T2 & aT2)
  throw (exception)
{
  map<T1, T2>::iterator pos1 = _map12.find(aT1);
  if (pos1 != _map12.end())
    {
      _map12.erase(pos1);
    }

  map<T1, T2>::iterator pos2 = _map21.find(aT2);
  if (pos2 != _map21.end())
    {
      _map21.erase(pos2);
    }

  _map12.insert(make_pair(aT1, aT2));
  _map21.insert(make_pair(aT2, aT1));
}

//===============================================================================

template<typename T1, typename T2>
void OneToOneMap<T1, T2>::clearByT1(const T1 & aT1)
  throw (exception)
{
  map<T1, T2>::iterator pos1 = _map12.find(aT1);
  if (pos1 == _map12.end())
    {
      return;
    }

  map<T1, T2>::iterator pos2 = _map21.find(pos1->second);
  if (pos2 != _map21.end())
    {
      _map21.erase(pos2);
    }

  _map12.erase(pos1);
}

//===============================================================================

template<typename T1, typename T2>
void OneToOneMap<T1, T2>::clearByT2(const T2 & aT2)
  throw (exception)
{
  map<T1, T2>::iterator pos2 = _map21.find(aT2);
  if (pos2 == _map21.end())
    {
      return;
    }

  map<T1, T2>::iterator pos1 = _map12.find(pos2->second);
  if (pos1 != _map12.end())
    {
      _map12.erase(pos1);
    }

  _map21.erase(pos2);
}

//===============================================================================

template<typename T1, typename T2>
const map<T1, T2> & OneToOneMap<T1, T2>::getT1T2Map() const
{
  return _map12;
}

//===============================================================================

template<typename T1, typename T2>
const map<T2, T1> & OneToOneMap<T1, T2>::getT2T1Map() const
{
  return _map21;
}

//===============================================================================

#endif
