#include <StorageMgr_Exceptions.H>
#include <CountingSemHolder.H>
#include <iostream>

//================================================================================

CountingSemHolder::CountingSemHolder(CountingSem & semToHold)
  throw (std::exception)
  : _sem(semToHold)
{
  bool success = _sem.acquireNonBlocking();
  if (!success)
    {
      throw SmException(__FILE__, __LINE__, "Couldn't acquire a resource from the CountingSem");
    }
}

//================================================================================

CountingSemHolder::~CountingSemHolder()
{
  try
    {
      _sem.release();
    }
  catch (std::exception & e)
    {
      // Scott Meyers makes a good argument why we should never throw an
      // exception from a destructor, unfortunately.
      cerr << e.what() << endl;
      assert(false);
    }
}
