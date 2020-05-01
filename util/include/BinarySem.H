#ifndef BINARYSEM_H
#define BINARYSEM_H

#include "StorageMgr_Exceptions.H"
#include "PtMutex.H"
#include "PtCondition.H"


using namespace std;

class BinarySem
{
public:
  BinarySem(bool initiallyPosted = false)
    throw (std::exception);

  virtual ~BinarySem();

	// Ensures that the sem is posted. One threads presently invoking the
	// 'awaitPostThenClear()' method will unblock, and the sem will be cleared.
	// Returns 'true' iff the sem was already posted at the time this method
	// was called.
  bool post()
    throw (std::exception);

  // If the sem is posted, this clears the sem and returns immediately.
  // Otherwise, this waits until the sem is posted, and THEN clears it
  // and returns.
  void awaitPostThenClear()
    throw (std::exception);

  // If the sem is posted, this clears the sem and returns immediately.
  // Otherwise, this waits until the sem is posted, and returns. This
  // does *not* clear the sem.
  void awaitPost()
    throw (std::exception);

  // Simple, non-blocking method to test if the sem is currently posted.
  bool isPosted() const
    throw (std::exception);

private:
  mutable PtMutex _mtx;
  mutable PtCondition _cond;
  bool _isPosted;
};

#endif
