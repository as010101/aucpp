#include <FdMultiPool.H>
#include <LockHolder.H>
#include <util.H>
#include <iostream>

//===============================================================================

FdMultiPool::FdMultiPool(int maxFdsInPool)
  throw (std::exception) :
  _maxFdsInPool(maxFdsInPool),
  _closed(false)
{
}

//===============================================================================

FdMultiPool::~FdMultiPool()
{
}

//===============================================================================

void FdMultiPool::registerFile(string pathname, int openFlags)
  throw (std::exception)
{
  LockHolder mtxRight(_mtx);

  if (_closed)
    {
      throw SmException(__FILE__, __LINE__, "Object has already been close()'d.");
    }

  FileInfo & fi = _registeredFiles[pathname];

  fi._openFlags = openFlags;
  fi._permissionsSpecified = false;
}

//===============================================================================

void FdMultiPool::registerFile(string pathname, int openFlags, mode_t permissions)
  throw (std::exception)
{
  LockHolder mtxRight(_mtx);

  if (_closed)
    {
      throw SmException(__FILE__, __LINE__, "Object has already been close()'d.");
    }

  FileInfo & fi = _registeredFiles[pathname];

  fi._openFlags = openFlags;
  fi._permissions = permissions;
  fi._permissionsSpecified = true;
}

//===============================================================================

void FdMultiPool::close()
    throw (std::exception)
{
  LockHolder mtxRight(_mtx);

  if (_closed)
    {
      throw SmException(__FILE__, __LINE__, "Object has already been close()'d.");
    }

  // Pass #1: Make sure there's no currently leased FDs...
  map<string, FileInfo>::iterator pos;
  for (pos = _registeredFiles.begin(); pos != _registeredFiles.end(); ++pos)
    {
      FileInfo & fi = pos->second;
      if (fi._leasedFds.size() > 0)
	{
	  throw SmException(__FILE__, __LINE__, "The FdMultiPool has at least one outstanding lease");
	}
    }

  // Pass #2: Cleanup all the FDs...
  for (pos = _registeredFiles.begin(); pos != _registeredFiles.end(); ++pos)
    {
      FileInfo & fi = pos->second;

      for (list<int>::iterator fdIter = fi._nonLeasedFds.begin();
	   fdIter != fi._nonLeasedFds.end(); ++fdIter)
	{
	  closeFile(*fdIter);
	}
    }

  _closed = true;
}

//===============================================================================

int FdMultiPool::acquireFdNonBlocking(string pathname)
  throw (std::exception)
{
  LockHolder mtxRight(_mtx);

  if (_closed)
    {
      throw SmException(__FILE__, __LINE__, "Object has already been close()'d.");
    }

  // First, try to find an FD that's already open for this file...
  FileInfo & fi = _registeredFiles[pathname];
  if (fi._nonLeasedFds.size() > 0)
    {
      int fd = fi._nonLeasedFds.back();
      fi._nonLeasedFds.pop_back();
      fi._leasedFds.insert(fd);
      return fd;
    }

  // I guess we need to try to grow the number of open FDs for this file. If the
  // pool is already at its max capacity of open FDs, free one first from some
  // other file...
  int numOpenFds = getNumOpenFds();
  if (numOpenFds == _maxFdsInPool)
    {
      evictOneFd();
    }

  int fd = openFile(pathname);
  fi._leasedFds.insert(fd);
  return fd;
}

//===============================================================================

void FdMultiPool::releaseFd(string pathname, int releasedFd)
  throw (std::exception)
{
  LockHolder mtxRight(_mtx);

  if (_closed)
    {
      throw SmException(__FILE__, __LINE__, "Object has already been close()'d.");
    }

  FileInfo & fi = _registeredFiles[pathname];

  int numFound =  fi._leasedFds.erase(releasedFd);

  if (numFound == 0)
    {
      throw SmException(__FILE__, __LINE__, 
				      "The specified FD didn't appear in the file's list of leased FDs");
    }

  if (numFound > 1)
    {
      throw SmException(__FILE__, __LINE__, 
				      "The specified FD appeared more than once in the file's list of leased FDs");
    }

  fi._nonLeasedFds.push_back(releasedFd);
}

//===============================================================================

void FdMultiPool::printDebugInfo(ostream & o)
  throw (std::exception)
{
  LockHolder mtxRight(_mtx);

  o << "FdMultiPool (" << this << ") state:" << endl
    << "\t_maxFdsInPool = " << _maxFdsInPool << endl
    << "\t_closed = " << _closed << endl << endl
    << "\t_registeredFiles:" << endl
    << "\t=================" << endl;

  for (map<string, FileInfo>::iterator pos = _registeredFiles.begin();
       pos != _registeredFiles.end(); ++pos)
    {
      const string & filename = pos->first;
      FileInfo & fi = pos->second;

      o << "\tfilename = " << filename << endl
	<< "\t\t_openFlags = " << fi._openFlags << endl
	<< "\t\t_permissions = " << fi._permissions << endl
	<< "\t\t_permissionsSpecified = " << fi._permissionsSpecified << endl
	<< "\t\t_leasedFds = {";

      for (set<int>::iterator pos2 = fi._leasedFds.begin(); pos2 != fi._leasedFds.end(); ++pos2)
	{
	  o << " " << *pos2;
	}

      o << "}" << endl;

      o << "\t\t_nonLeasedFds = {";

      for (list<int>::iterator pos3 = fi._nonLeasedFds.begin(); pos3 != fi._nonLeasedFds.end(); ++pos3)
	{
	  o << " " << *pos3;
	}

      o << "}" << endl << endl;
    }
}

//===============================================================================

int FdMultiPool::openFile(string pathname)
  throw (std::exception)
{
  FileInfo & fi = _registeredFiles[pathname];
  
  int fd;
  if (fi._permissionsSpecified)
    {
      fd = openFileWithFlagsAndPerms(pathname, fi._openFlags, fi._permissions);
    }
  else
    {
      fd = openFileWithFlags(pathname, fi._openFlags);
    }

  return fd;
}

//===============================================================================

void FdMultiPool::evictOneFd()
  throw (std::exception)
{
  // Discover a file with the smallest number of leased FDs. That's perhaps an
  // indicator that that file hasn't has much business lately, and is a good
  // candidate for a reduced budget...
  
  int minLeasedFds = _maxFdsInPool + 1; // a value no file could have...
  FileInfo * pfiForEviction = NULL;

  map<string, FileInfo>::iterator pos;
  for (pos = _registeredFiles.begin(); pos != _registeredFiles.end(); ++pos)
    {
      FileInfo & fi = pos->second;

      int numLeasedFds = fi._leasedFds.size();
      if ((numLeasedFds < minLeasedFds) && (fi._nonLeasedFds.size() > 0))
	{
	  minLeasedFds = numLeasedFds;
	  pfiForEviction = & fi;
	}
    }

  if (pfiForEviction == NULL)
    {
      throw SmException(__FILE__, __LINE__, "No FD was available for eviction");
    }

  int fd = pfiForEviction->_nonLeasedFds.back();
  pfiForEviction->_nonLeasedFds.pop_back();
  closeFile(fd);
}

//===============================================================================

int FdMultiPool::getNumOpenFds()
  throw (std::exception)
{
  int returnVal = 0;

  map<string, FileInfo>::iterator pos;
  for (pos = _registeredFiles.begin(); pos != _registeredFiles.end(); ++pos)
    {
      FileInfo & fi = pos->second;
      returnVal += (fi._leasedFds.size() + fi._nonLeasedFds.size());
    }

  return returnVal;
}

//===============================================================================
