#include <FdMultiPoolLease.H>
#include <iostream>

//==============================================================================

FdMultiPoolLease::FdMultiPoolLease(FdMultiPool & pool, string pathname)
  throw (std::exception) :
  _pool(pool),
  _pathname(pathname)
{
  _fd = _pool.acquireFdNonBlocking(_pathname);
}

//==============================================================================

FdMultiPoolLease::~FdMultiPoolLease()
{
  try
  {
    _pool.releaseFd(_pathname, _fd);
  }
  catch (std::exception & e)
  {
    cerr << "FdMultiPoolLease::~FdMultiPoolLease(): Exception caught: " 
	 << e.what() << endl;
    assert(false);
  }
}

//==============================================================================

int FdMultiPoolLease::getFd() const
{
  return _fd;
}

//==============================================================================
