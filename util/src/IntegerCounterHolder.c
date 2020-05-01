#include <IntegerCounterHolder.H>

//===============================================================================

IntegerCounterHolder::IntegerCounterHolder(IntegerCounter & aCounter, int maxAllowableCount)
  throw (std::exception)
  : _counter(aCounter)
{
  bool success = _counter.incrementWithTest(maxAllowableCount);
  if (! success)
    {
      throw SmException(__FILE__, __LINE__, "Exceeded maxAllowableCount");
    }
}

//===============================================================================

IntegerCounterHolder::~IntegerCounterHolder()
{
  try
    {
      _counter.decrement();
    }
  catch (std::exception)
    {
      assert(false);
    }
}

//===============================================================================
