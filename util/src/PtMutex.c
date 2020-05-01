#include <PtMutex.H>
#include <sstream>

#include <iostream>

PtMutex::PtMutex()
  throw (std::exception)
{
  int rc = pthread_mutex_init(& _mtx, NULL);
  if (rc != 0)
    {
      throw SmException(__FILE__, __LINE__, 
			"Failed call to pthread_mutex_init(...)");
    }

#ifdef SM_VERIFY_LOCKS
  rc = pthread_mutex_init(& _guard, NULL);
  if (rc != 0)
    {
      throw SmException(__FILE__, __LINE__, 
			"Failed call to pthread_mutex_init(...)");
    }
#endif
}

//===============================================================================

PtMutex::~PtMutex()
  throw (std::exception)
{
  int rc = pthread_mutex_destroy(& _mtx);
  if (rc != 0)
    {
      throw SmException(__FILE__, __LINE__, 
			"Failed call to pthread_mutex_destroy(...)");
    }

#ifdef SM_VERIFY_LOCKS
  rc = pthread_mutex_destroy(& _guard);
  if (rc != 0)
    {
      throw SmException(__FILE__, __LINE__, 
			"Failed call to pthread_mutex_destroy(...)");
    }
#endif
}

//===============================================================================

void PtMutex::lock()
  throw (std::exception)
{
  int rc = pthread_mutex_lock(& _mtx);
  if (rc != 0)
    {
      throw SmException(__FILE__, __LINE__, 
			"Failed call to pthread_mutex_lock(...)");
    }

#ifdef SM_VERIFY_LOCKS
  rc = pthread_mutex_lock(& _guard);
  if (rc != 0)
    {
      throw SmException(__FILE__, __LINE__, 
			"Failed call to pthread_mutex_lock(...)");
    }

  _holder = pthread_self();
  _held = true;

  rc = pthread_mutex_unlock(& _guard);
  if (rc != 0)
    {
      throw SmException(__FILE__, __LINE__, 
			"Failed call to pthread_mutex_unlock(...)");
    }
#endif
}

//===============================================================================

void PtMutex::unlock()
  throw (std::exception)
{
  int rc;

#ifdef SM_VERIFY_LOCKS
  rc = pthread_mutex_lock(& _guard);
  if (rc != 0)
    {
      throw SmException(__FILE__, __LINE__, 
			"Failed call to pthread_mutex_lock(...)");
    }

  _held = false;

  rc = pthread_mutex_unlock(& _guard);
  if (rc != 0)
    {
      throw SmException(__FILE__, __LINE__, 
			"Failed call to pthread_mutex_unlock(...)");
    }
#endif

  rc = pthread_mutex_unlock(& _mtx);
  if (rc != 0)
    {
      throw SmException(__FILE__, __LINE__, 
			"Failed call to pthread_mutex_unlock(...)");
    }
}

//===============================================================================

void PtMutex::waitCond(PtCondition & cond)
  throw (std::exception)
{
  int rc = pthread_cond_wait(& cond._cond, & _mtx);
  if (rc  != 0)
    {
      throw SmException(__FILE__, __LINE__, 
			"Failed call to pthread_cond_wait(...)");
    }
}



#ifdef SM_VERIFY_LOCKS

//===============================================================================

bool PtMutex::getHolder(pthread_t & tid)
  throw (exception)
{
  int rc = pthread_mutex_lock(& _guard);
  if (rc != 0)
    {
      throw SmException(__FILE__, __LINE__, 
			"Failed call to pthread_mutex_lock(...)");
    }
  
  bool returnVal = _held;

  if (_held)
    {
      tid = _holder;
    }

  rc = pthread_mutex_unlock(& _guard);
  if (rc != 0)
    {
      throw SmException(__FILE__, __LINE__, 
			"Failed call to pthread_mutex_unlock(...)");
    }

  return returnVal;
}

//===============================================================================

void PtMutex::ensureHeldByCaller(bool doAbort) const
  throw (exception)
{
  //  cout << "Entering PtMutex::ensureHeldByCaller" << endl;
  bool heldByCaller;

  // Fool the compiler into letting this method me const...
  const pthread_mutex_t * alias = & _guard;
  pthread_mutex_t * pNonConstGuard = const_cast<pthread_mutex_t *>(alias);

  int rc = pthread_mutex_lock(pNonConstGuard);
  if (rc != 0)
    {
      throw SmException(__FILE__, __LINE__, 
			"Failed call to pthread_mutex_lock(...)");
    }

  heldByCaller = ((_held) && (_holder == pthread_self()));
  //  cout << "_held =  " << _held << endl
  //       << "_holder = " << _holder << endl
  //       << "caller = " << pthread_self() << endl
  //       << "heldByCaller = " << heldByCaller << endl << endl;

  rc = pthread_mutex_unlock(pNonConstGuard);
  if (rc != 0)
    {
      throw SmException(__FILE__, __LINE__, 
			"Failed call to pthread_mutex_unlock(...)");
    }

  if (! heldByCaller)
    {
      if (doAbort)
	{
	  abort();
	}
      else
	{
	  ostringstream os;
	  os << "Lock not held by caller thread: " << pthread_self();
	  throw SmException(__FILE__, __LINE__, os.str());
	}
    }
  //  cout << "Exiting PtMutex::ensureHeldByCaller" << endl;
}

#endif
