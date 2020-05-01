#ifndef FdMultiPoolLease_H
#define FdMultiPoolLease_H

#include "FdMultiPool.H"

/*
  This class holds a leased FD from an FdMultiPool object. The lease is
  ascquired when this object is isntantiated, and is released when this
  object is destroyed.
 */
class FdMultiPoolLease
{
public:
  FdMultiPoolLease(FdMultiPool & pool, string pathname)
    throw (std::exception);

  ~FdMultiPoolLease();

  // Gives the FD that this lease holds.
  int getFd() const;

private:
  FdMultiPool & _pool;
  string _pathname;
  int _fd;
};

#endif
