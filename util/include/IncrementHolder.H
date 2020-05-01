#ifndef IncrementHolder_H
#define IncrementHolder_H

#include "PtMutex.H"
#include "LockHolder.H"
#include <iostream.h>

/*
 When this object is instantiated, it increments the specified value using
 the ++ operator. When this object is uninstantiated, it decrements the
 specified value using the -- operator. It temporarily holds a lock on the
 supplied mutex while performing each of those operations.
 */
template<typename T>
class IncrementHolder
{
public:
  // Throws an exception if the bounds are violated during this object's 
  // instantiation / uninstantiation.
  IncrementHolder(T& value, 
		  PtMutex &lock, 
		  T minValue, 
		  T maxValue)
    throw (std::exception);

  virtual ~IncrementHolder();

private:
  void verifyBounds()
    throw (std::exception);

  PtMutex & _lock;
  T& _value;
  T _minValue;
  T _maxValue;
};


//===============================================================================

template<typename T>
IncrementHolder::IncrementHolder(T& value, 
		  PtMutex &lock, 
		  T minValue, 
		  T maxValue)
  throw (std::exception) :
  _value(value),
  _lock(lock),
  _minValue(minValue),
  _maxValue(maxValue)
{
  LockHolder holder(_lock);
  _value++;
  verifyBounds();
}

//===============================================================================

template<typename T>
IncrementHolder::~IncrementHolder()
{
  try
    {
      LockHolder holder(_lock);
      _value--;
      verifyBounds();
    }
  catch (std::exception &e)
    {
      cerr << e.what() << endl;
      assert(false);
    }
}

//===============================================================================

template<typename T>
void IncrementHolder::verifyBounds()
    throw (std::exception)
{
  if (_value < _minValue)
    {
      throw SmException(__FILE__, __LINE__, "_value < _minValue");
    }

  if (_value > _maxValue)
    {
      throw SmException(__FILE__, __LINE__, "_value > _maxValue");
    }
}


#endif
