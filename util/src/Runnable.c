#include <Runnable.H>
#include <StorageMgr_Exceptions.H>
#include <typeinfo>

//===============================================================================

Runnable::Runnable()
  :  _lastRunException(NULL)
{
}

//===============================================================================

Runnable::~Runnable()
{
  if (_lastRunException != NULL)
    {
      delete _lastRunException;
      _lastRunException = NULL;
    }
}

//===============================================================================

void Runnable::cloneAndSetRunException(const std::exception &e)
{
  std::exception * pE = NULL;

  const SmException * pSmException = dynamic_cast<const SmException *>(& e);
  if (pSmException != NULL)
    {
      pE = new SmException(* pSmException);
    }
  else
    {
      pE = new std::exception(e);
    }

  setRunException(pE);
}

//===============================================================================

void Runnable::setRunException(std::exception *pException)
{
  if (_lastRunException != NULL)
    {
      delete _lastRunException;
    }

  _lastRunException = pException;
}

//===============================================================================


std::exception * Runnable::getRunException() const
{
  return _lastRunException;
}

//===============================================================================

void Runnable::deleteRunException()
{
  if (_lastRunException != NULL)
    {
      delete _lastRunException;
      _lastRunException = NULL;
    }
}

//===============================================================================

Runnable::Runnable(const Runnable & rhs)
  : _lastRunException(rhs._lastRunException)
{
}

//===============================================================================

Runnable & Runnable::operator=(const Runnable & rhs)
{
  _lastRunException = rhs._lastRunException;
  return *this;
}

//===============================================================================
