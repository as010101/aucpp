#ifndef MONITOR_H
#define MONITOR_H

#include <PtMutex.H>
#include <LockHolder.H>
#include <PtCondition.H>

/*
  Bundles together everything you need for a monitor variable.

  Note on setting values:
  There are two different, valid ways to change the Monitor's value. They are:

  (A) Call setValue(...), or

  (B) Have code like this:
      Monitor m(...);
      ...
      {
         LockHolder lh(m);
         ...
         (do modifying operations on m._value)
         ...
         m._cond.broadcast();
      }

  Remember, though, that the idea of this Monitor is that any state change should
  be theoretically apparent using == or != comparisons. This class is not intended
  to just be a general mutex-protector of any old interesting class. It's meant to
  be a low-level primitive to wrap things like strings, ints, sets, etc.
 */
template<typename T>
class Monitor
{
public:
  Monitor(const T & initialValue)
    throw (exception);

  Monitor()
    throw (exception);

  virtual ~Monitor()
    throw (exception);
  
  // These provide the raw building blocks from which a Monitor is built...
  PtMutex _mtx;
  PtCondition _cond;
  T _value;

  //-----------------------------------------------------------------------------
  // The following helper method can be used *in place of* the raw building 
  // blocks listed above. THE CALLER MUST *NOT* HOLD A LOCK ON _MTX WHEN CALLING
  // ANY OF THESE FUNCTIONS...
  //-----------------------------------------------------------------------------

  void setValue(const T & v)
    throw (exception);

  // Returns a snapshot of _value's current state. If you don't wany copying to
  // occur (as this method must do), you should instead write code that looks 
  // like the following:
  //    Monitor m(...);
  //    ...
  //    {
  //       LockHolder lh(m);
  //       // make extensive, read-only use of m._value
  //    }
  T getValueCopy()
    throw (exception);

  // Blocks until (_value == v). Uses operator==.
  void awaitValueEquals(const T & v)
    throw (exception);

  // Blocks until (_value != v). Uses operator!=.
  void awaitValueNotEquals(const T & v)
    throw (exception);

  // Like awaitValueNotEquals(...), but also returns the new value.
  T getValueWhenNotEquals(const T & v)
    throw (exception);
};

//===============================================================================

template<typename T> Monitor<T>::Monitor(const T & initialValue)
  throw (exception) :
  _value(initialValue)
{
}

//===============================================================================

template<typename T> Monitor<T>::Monitor()
  throw (exception)
{
}

//===============================================================================

template<typename T> Monitor<T>::~Monitor()
    throw (exception)
{
}

//===============================================================================

template<typename T> void Monitor<T>::setValue(const T & v)
    throw (exception)
{
  LockHolder lh(_mtx);
  _value = v;
  _cond.broadcast();
}

//===============================================================================

template<typename T> T Monitor<T>::getValueCopy()
    throw (exception)
{
  LockHolder lh(_mtx);
  return _value;
}

//===============================================================================

template<typename T> void Monitor<T>::awaitValueEquals(const T & v)
    throw (exception)
{
  while (true)
    {
      LockHolder lh(_mtx);

      if (_value == v)
	{
	  return;
	}
      else
	{
	  _mtx.waitCond(_cond);
	}     
    }
}

//===============================================================================

template<typename T> void Monitor<T>::awaitValueNotEquals(const T & v)
    throw (exception)
{
  while (true)
    {
      LockHolder lh(_mtx);

      if (_value != v)
	{
	  return;
	}
      else
	{
	  _mtx.waitCond(_cond);
	}     
    }
}

//===============================================================================

template<typename T> T Monitor<T>::getValueWhenNotEquals(const T & v)
    throw (exception)
{
  while (true)
    {
      LockHolder lh(_mtx);

      if (_value != v)
	{
	  return _value;
	}
      else
	{
	  _mtx.waitCond(_cond);
	}     
    }
}


//===============================================================================

#endif
