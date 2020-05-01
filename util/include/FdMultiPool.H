#ifndef FdMultiPool_H
#define FdMultiPool_H

#include "StorageMgr_Exceptions.H"
#include "PtMutex.H"
#include <map>
#include <list>
#include <set>
#include <string>
#include <fcntl.h>

/*
 This manages a pool of file descriptors, which covers a registered set of files.
 This pool will try to minimize the number of open(...) calls that needs to be
 made to satisfy all of the requests.
 */
class FdMultiPool
{
public:
  FdMultiPool(int maxFdsInPool)
    throw (std::exception);

  virtual ~FdMultiPool();

  //-----------------------------------------------------------------------------

  // Makes the pool capable of offering file handles for the specified file.
  void registerFile(string pathname, int openFlags)
    throw (std::exception);

  // Makes the pool capable of offering file handles for the specified file.
  void registerFile(string pathname, int openFlags, mode_t permissions)
    throw (std::exception);

  // If no FD is currently leased, this closes all FDs and disables this object
  // from future operations. Otherwise, this throws an exception.
  void close()
    throw (std::exception);

  //-----------------------------------------------------------------------------

  // Leases an FD from the pool. Throws an exception of if there was a problem
  // opening the file, or if the pool's maxFD's would be exceeded, or if something
  // else goes wrong.
  //
  // Returns ./the FD to be leased.
  int acquireFdNonBlocking(string pathname)
    throw (std::exception);

  void releaseFd(string pathname, int releasedFd)
    throw (std::exception);

  //-----------------------------------------------------------------------------

  void printDebugInfo(ostream & o)
    throw (std::exception);

private:
  PtMutex _mtx;

  const int _maxFdsInPool;
  bool _closed;

  // Gives all the data we track on a per-file basis in this FdMultiPool object.
  struct FileInfo
  {
    int _openFlags;

    mode_t _permissions;
    bool _permissionsSpecified;

    set<int> _leasedFds;     // Set of FDs that are currently in use for this file
    list<int> _nonLeasedFds; // Set of FDs that are open for this file but aren't currently in use
  };

  map<string, FileInfo> _registeredFiles; // key = pathname

  // Given the pathname of a file in _registeredFiles, this opens another FD for
  // that file and returns it. This method doesn't modify the FileInfo object
  // associated with the file.
  int openFile(string pathname)
    throw (std::exception);

  // Evicts one FD, that's not currently being used, from the pool. If all FDs
  // in the pool are currently leased out, this throws an exception.
  void evictOneFd()
    throw (std::exception);

  // Returns the number of FDs that are presently allocated in this pool. This is
  // the number that are leased + the number that are open but not leased.
  int getNumOpenFds()
    throw (std::exception);
};

#endif
