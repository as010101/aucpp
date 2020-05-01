#ifndef CLOSABLEFIFOQUEUE_H
#define CLOSABLEFIFOQUEUE_H

#include "StorageMgr_Exceptions.H"
#include "PtMutex.H"
#include "PtCondition.H"
#include "LockHolder.H"
#include "BinarySem.H"

#include <queue>
#include <assert.h>
#include <iostream>
#include <string>

using namespace std;

template<typename T>
class ClosableFifoQueue
{
public:
  // {pNonEmptyCond is broadcast to} and {pNonEmptyBinarySem is post()'ed} 
  //  when this queue goes from being empty to being non-empty, if the
  // corresponding parameter is not NULL.
  //
  // If 'pClosedAndEmpty' is supplied, this object will post() to it when 
  // and only when this object's close() method has been called and the object
  // is empty. (I.e., the same criteria that would cause the dequeue...() methods
  // to throw a SmClosedException.)
  //
  // The 'name' parameter is used to let each ClosableFifoQueue be named. That name
  // is used only in debugging messages.
  ClosableFifoQueue(PtCondition *pNonEmptyCond, 
		    BinarySem * pNonEmptyBinarySem,
		    BinarySem * pClosedAndEmpty,
		    string name = "(No name given)")
    throw (exception);

  virtual ~ClosableFifoQueue();

  // No additional enqueues can be made into the queue. Once the queue is
  // empty and is also closed, all pending and future calls to the dequeue()
  // method will throw the exception SmClosedException.
  //
  // This method is idempotent, which is why it won't throw a SmClosedException.
  void close()
    throw (exception);

  // Enqueues a copy of newValue.
  void enqueue(const T &newValue)
    throw (exception,
	   SmClosedException);

  // Blocks until the queue is not empty, or it's (empty and closed).
  T dequeueBlocking()
    throw (exception,
	   SmClosedException);

  // If the queue was empty, this returns false and doesn't modify the 'value' parameter.
  // If the queue was not empty, this returns true, and sets 'value' to the value that
  // wa dequeued.
  bool dequeueNonBlocking(T &value)
    throw (exception,
	   SmClosedException);

  // Returns the number of items currently in the queue.
  unsigned int size()
    throw (exception);

  bool isEmpty()
    throw (exception);

  bool isClosed()
    throw (exception);
private:
  // Disallow these until we feel like really implementing them...
  ClosableFifoQueue(const ClosableFifoQueue &rhs) {assert(false);}
  ClosableFifoQueue & operator =(const ClosableFifoQueue &rhs) {assert(false);}

  bool _closed;
  queue<T> _dataqueue;

  PtMutex _mutex;
  PtCondition _cond;
  PtCondition * _pNonEmptyCond;
  BinarySem * _pNonEmptyBinarySem;
  BinarySem * _pClosedAndEmpty;
  string _name;
};


//==============================================================================================

template<typename T> ClosableFifoQueue<T>::ClosableFifoQueue(PtCondition *pNonEmptyCond, 
							     BinarySem * pNonEmptyBinarySem,
							     BinarySem * pClosedAndEmpty,
							     string name)
  throw (exception) :
  _closed(false),
  _pNonEmptyCond(pNonEmptyCond),
  _pNonEmptyBinarySem(pNonEmptyBinarySem),
  _pClosedAndEmpty(pClosedAndEmpty),
  _name(name)
{
}

//==============================================================================================

template<typename T> ClosableFifoQueue<T>::~ClosableFifoQueue()
{
}

//==============================================================================================

template<typename T> void ClosableFifoQueue<T>::enqueue(const T &newValue)
  throw (exception,
	 SmClosedException)

{
  LockHolder holder(_mutex);

  if (_closed)
    {
      throw SmClosedException(__FILE__, __LINE__, "ClosableFifoQueue is closed.");
    }

  _dataqueue.push(newValue);

  if (_dataqueue.size() == 1)
    {
      if (_pNonEmptyCond != NULL)
	{
	  _pNonEmptyCond->broadcast();
	}
      
      if (_pNonEmptyBinarySem != NULL)
	{
	  //cout << "Queue(\"" << _name << "\"): Just become non-empty" << endl;
	  _pNonEmptyBinarySem->post();
	}
    }

  _cond.broadcast();
}

//==============================================================================================

template<typename T> void ClosableFifoQueue<T>::close()
  throw (exception)
{
  LockHolder holder(_mutex);

  _closed = true;
  _cond.broadcast();

  if ((_pClosedAndEmpty != NULL) && _dataqueue.empty())
    {
      //cout << "Queue(\"" << _name << "\"): Just become closed-and-empty" << endl;
      _pClosedAndEmpty->post();
    }
}

//==============================================================================================

template<typename T> T ClosableFifoQueue<T>::dequeueBlocking()
  throw (SmClosedException, 
	 exception)
{
  LockHolder holder(_mutex);

  while (true)
    {
      if (_dataqueue.empty() && _closed)
	{
	  throw SmClosedException(__FILE__, __LINE__, "ClosableFifoQueue is closed and empty.");
	}
      
      if (! _dataqueue.empty())
	{
	  // This block is the only way out of the loop...
	  T returnVal = _dataqueue.front();
	  _dataqueue.pop();

	  if ((_pClosedAndEmpty != NULL) && _dataqueue.empty() && _closed)
	    {
	      // We just entered the closed-and-empty state.
	      //cout << "Queue(\"" << _name << "\"): Just become closed-and-empty" << endl;
	      _pClosedAndEmpty->post();
	    }
	  
	  _cond.broadcast();
	  return returnVal;
	}

      _mutex.waitCond(_cond);
    }
}

//==============================================================================================


  // If the queue was empty, this returns false and doesn't modify the 'value' parameter.
  // If the queue was not empty, this returns true, and sets 'value' to the value that
  // wa dequeued.
template<typename T> bool ClosableFifoQueue<T>::dequeueNonBlocking(T &value)
    throw (exception,
	   SmClosedException)
{
  LockHolder holder(_mutex);

  if (_closed)
    {
      throw SmClosedException(__FILE__, __LINE__, "ClosableFifoQueue is closed and empty.");
    }

  if (_dataqueue.empty())
    {
      return false;
    }
  else
    {
      value = _dataqueue.front();
      _dataqueue.pop();

      if ((_pClosedAndEmpty != NULL) && _dataqueue.empty() && _closed)
	{
	  // We just entered the closed-and-empty state.
	  //cout << "Queue(\"" << _name << "\"): Just become closed-and-empty" << endl;
	  _pClosedAndEmpty->post();
	}

      _cond.broadcast();
      return true;
    }
}

//==============================================================================================

template<typename T> unsigned int ClosableFifoQueue<T>::size()
  throw (exception)
{
  LockHolder holder(_mutex);
  bool returnVal = _dataqueue.size();
  return returnVal;
}

//==============================================================================================

template<typename T> bool ClosableFifoQueue<T>::isEmpty()
  throw (exception)
{
  LockHolder holder(_mutex);
  bool returnVal = _dataqueue.empty();
  return returnVal;
}

//==============================================================================================

template<typename T> bool ClosableFifoQueue<T>::isClosed()
  throw (exception)
{
  LockHolder holder(_mutex);
  return _closed;
}


#endif
