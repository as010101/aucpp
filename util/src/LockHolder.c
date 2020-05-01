#include <LockHolder.H>
#include <StorageMgr_Exceptions.H>
#include <assert.h>

//================================================================================

LockHolder::LockHolder(ILockable & lockToHold)
  throw (std::exception)
  : _lock(lockToHold),
    _presentlyHeld(true)
{
  _lock.lock();
}

//================================================================================

LockHolder::~LockHolder()
{
  try
    {
      if (_presentlyHeld)
	{
	  _lock.unlock();
	}
    }
  catch (std::exception & e)
    {
      // Scott Meyers makes a good argument why we should never throw an
      // exception from a destructor, unfortunately.
      assert(false);
    }
}

//================================================================================

void LockHolder::release()
  throw (exception)
{
  if (_presentlyHeld)
    {
      _presentlyHeld = false;
      _lock.unlock();
    }
  else
    {
      throw SmException(__FILE__, __LINE__, 
			"Lock not presently held by this LockHolder");
    }
}

//================================================================================

void LockHolder::reacquire()
  throw (exception)
{
  if (! _presentlyHeld)
    {
      _presentlyHeld = true;
      _lock.lock();
    }
  else
    {
      throw SmException(__FILE__, __LINE__, 
			"Lock already held by this LockHolder");
    }
}

//================================================================================
