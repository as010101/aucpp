#include <CountingSem.H>
#include <LockHolder.H>

//===============================================================================

CountingSem::CountingSem(int maxConcurrentHoldings)
  throw (std::exception) :
  _maxConcurrentHoldings(maxConcurrentHoldings),
  _currentHoldings(0)
{
  if (maxConcurrentHoldings < 0)
    {
      throw SmException(__FILE__, __LINE__, "maxConcurrentHoldings < 0");
    }
}

//===============================================================================

CountingSem::~CountingSem()
{
}

//===============================================================================

void CountingSem::setMaxConcurrentHoldings(int maxConcurrentHoldings)
  throw (std::exception)
{
  LockHolder holder(_mtx);

  if (maxConcurrentHoldings < _currentHoldings)
    {
      throw SmException(__FILE__, __LINE__, "maxConcurrentHoldings < _currentHoldings");
    }

  _maxConcurrentHoldings = maxConcurrentHoldings;
}

//===============================================================================

bool CountingSem::acquireNonBlocking()
  throw (std::exception)
{
  LockHolder holder(_mtx);
  if (_currentHoldings < _maxConcurrentHoldings)
    {
      _currentHoldings++;
      return true;
    }
  else
    {
      return false;
    }
}

//===============================================================================

void CountingSem::release()
  throw (std::exception)
{
  LockHolder holder(_mtx);
  if (_currentHoldings == 0)
    {
      throw SmException(__FILE__, __LINE__, "_currentHoldings == 0");
    }

  _currentHoldings--;
}

//===============================================================================
