#include <IntegerCounter.H>
#include <LockHolder.H>

//===============================================================================

IntegerCounter::IntegerCounter()
  throw (std::exception) :
  _intValue(0)
{
  _pMtx = new PtMutex;
}

//===============================================================================

IntegerCounter::~IntegerCounter()
{
  try
    {
      delete _pMtx;
    }
  catch (std::exception)
    {
      assert(false);
    }
}

//===============================================================================

bool IntegerCounter::incrementWithTest(int maxValue)
    throw (std::exception)
{
  LockHolder holder(* _pMtx);

  if (_intValue < maxValue)
    {
      _intValue++;
      return true;
    }
  else
    {
      return false;
    }
}

//===============================================================================

void IntegerCounter::decrement()
    throw (std::exception)
{
  LockHolder holder(* _pMtx);

  if (_intValue < 0)
    {
      throw SmException(__FILE__, __LINE__, "Decrementing the value would make it negative.");
    }
  else
    {
      _intValue--;
    }
}


//===============================================================================

int IntegerCounter::getValue()
  throw (std::exception)
{
  LockHolder holder(* _pMtx);
  return _intValue;
}

//===============================================================================

IntegerCounter::IntegerCounter(const IntegerCounter & rhs)
{
  assert(false); // I don't want this called ever. That's why it's private.
}

//===============================================================================

IntegerCounter & IntegerCounter::operator= (const IntegerCounter & rhs)
{
  assert(false); // I don't want this called ever. That's why it's private.
  return *this;
}

//===============================================================================
