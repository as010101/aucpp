#ifndef FileDescPool_H
#define FileDescPool_H

#include "PtMutex.H"
#include "PtCondition.H"
#include <list>
#include <string>
#include <set>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

class FileDescPool
{
public:
  // Hint: Better make sure that the file open flags are ammenable to multiple
  // invocations of open(2) on the specified file.
  FileDescPool(string filepath, int openFlags, int minFDs, int maxFDs)
    throw (std::exception,
	   SmIllegalParameterValueException,
	   SmFilesystemException);

  // Hint: Better make sure that the file open flags are ammenable to multiple
  // invocations of open(2) on the specified file.
  FileDescPool(string filepath, int openFlags, mode_t permissions, int minFDs, int maxFDs)
    throw (std::exception,
	   SmIllegalParameterValueException,
	   SmFilesystemException);

  virtual ~FileDescPool();

  unsigned int getMinFDs()
    throw (std::exception);

  unsigned int getMaxFDs()
    throw (std::exception);

  unsigned int getTotalFDsInPool()
    throw (std::exception);

  unsigned int getNumLeasedFDs()
    throw (std::exception);

  // Fails if the number of currently leased FDs is greater than newMaxFDs.
  // Also fails if the parameters have silly values. '0' is not a silly value.
  void setFDLimits(unsigned int newMinFDs, unsigned int newMaxFDs)
    throw (std::exception);

  // Blocks until an FD is available in the pool, or until the closeAll()
  // method has been called on the pool. If this returns without an exception,
  // then the return value is a valid FD from this pool.
  //
  // Caller may do anything with the FD he pleases except to close it.
  int acquireFdBlocking()
    throw (std::exception,
	   SmClosedException);

  // Returns a valid FD from this pool, or -1 if none is available.
  //
  // Caller may do anything with the FD he pleases except to close it.
  int acquireFdNonBlocking()
    throw (std::exception);

  // Releases the specified FD back into the pool.
  void releaseFd(int fd)
    throw (std::exception);

  // Closes all of the file handles and sets the minimum number of open
  // FDs to 0. Throws an exception is one or more FDs is still leased.
  //
  // In all normal circumstances, this should be called before destroying
  // this object.
  void closeAll()
    throw (std::exception);

private:
  // Closes or opens FDs until the total number managed by this object is
  // numDesiredFDs. However, this will not close any FDs that are presently
  // leased (not a failure condition).
  void adjustFdCount(int numDesiredFDs)
    throw (std::exception);

  // Removes and closes one FD from _freeFDs.
  void deallocateOneFD()
    throw (std::exception);

  // Opens an FD and adds it to _freeFDs.
  void allocateOneFD()
    throw (std::exception);

  PtMutex _mtx;
  PtCondition _condAvailableFD; // Signal whenever another FD is available.
  list<int> _freeFDs;  // The FDs that are allocated but aren't in use
  set<int> _leasedFDs; // The FDs that are in use

  string _filepath;
  int _openFlags;
  unsigned int _minFDs;
  unsigned int _maxFDs;

  mode_t _permissions; // Only specified with one version of the c'tor
  bool _permissionsSet; // true iff the version of the c'tor that specifies file permissions was called.
};

#endif
