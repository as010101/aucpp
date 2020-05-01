#include <FileDescPool.H>
#include <LockHolder.H>
#include <util.H>
#include <algorithm>
#include <assert.h>

//===============================================================================

FileDescPool::FileDescPool(string filepath, int openFlags, int minFDs, int maxFDs)
  throw (std::exception,
	 SmIllegalParameterValueException,
	 SmFilesystemException) :
  _permissionsSet(false)
{
  if (minFDs < 0)
    {
      throw SmIllegalParameterValueException(__FILE__, __LINE__,
							   "minFDs < 0");
    }

  if (maxFDs < minFDs)
    {
      throw SmIllegalParameterValueException(__FILE__, __LINE__,
				      "maxFDs < minFDs");
    }

  if (filepath.empty())
    {
      throw SmIllegalParameterValueException(__FILE__, __LINE__,
							   "filepath.empty()");
    }

  _minFDs = minFDs;
  _maxFDs = maxFDs;
  _filepath = filepath;
  _openFlags = openFlags;

  adjustFdCount(_minFDs);
}

//===============================================================================

FileDescPool::FileDescPool(string filepath, int openFlags, mode_t permissions, int minFDs, int maxFDs)
  throw (std::exception,
	 SmIllegalParameterValueException,
	 SmFilesystemException) :
  _permissionsSet(true)
{
  _permissions = permissions;

  if (minFDs < 0)
    {
      throw SmIllegalParameterValueException(__FILE__, __LINE__,
							   "minFDs < 0");
    }

  if (maxFDs < minFDs)
    {
      throw SmIllegalParameterValueException(__FILE__, __LINE__,
				      "maxFDs < minFDs");
    }

  if (filepath.empty())
    {
      throw SmIllegalParameterValueException(__FILE__, __LINE__,
							   "filepath.empty()");
    }

  _minFDs = minFDs;
  _maxFDs = maxFDs;
  _filepath = filepath;
  _openFlags = openFlags;

  adjustFdCount(_minFDs);
}

//===============================================================================

FileDescPool::~FileDescPool()
{
}

//===============================================================================

unsigned int FileDescPool::getMinFDs()
  throw (std::exception)
{
  LockHolder holder(_mtx);
  return _minFDs;
}

//===============================================================================

unsigned int FileDescPool::getMaxFDs()
  throw (std::exception)
{
  LockHolder holder(_mtx);
  return _maxFDs;
}

//===============================================================================

unsigned int FileDescPool::getNumLeasedFDs()
  throw (std::exception)
{
  LockHolder holder(_mtx);
  return _leasedFDs.size();
}

//===============================================================================

unsigned int FileDescPool::getTotalFDsInPool()
  throw (std::exception)
{
  LockHolder holder(_mtx);
  return _leasedFDs.size() + _freeFDs.size();
}

//===============================================================================

void FileDescPool::setFDLimits(unsigned int newMinFDs, unsigned int newMaxFDs)
  throw (std::exception)
{
  if (newMinFDs > newMaxFDs)
    {
      throw SmException(__FILE__, __LINE__, "(newMinFDs > newMaxFDs)");
    }

  if (newMaxFDs < _leasedFDs.size())
    {
      throw SmException(__FILE__, __LINE__, "(newMaxFDs < _leasedFDs.size())");
    }

  LockHolder holder(_mtx);
  _minFDs = newMinFDs;
  _maxFDs = newMaxFDs;

  unsigned int currentFDCount = _freeFDs.size() + _leasedFDs.size();
  if (currentFDCount < newMinFDs)
    {
      // Come up to the minimum count...
      adjustFdCount(newMinFDs);
    }
  else if (currentFDCount > newMaxFDs)
    {
      // Come down to the new maximum count...
      adjustFdCount(newMaxFDs);
    }
}

//===============================================================================

int FileDescPool::acquireFdBlocking()
  throw (std::exception,
	 SmClosedException)
{
  // Await the availability of a FD...
  _mtx.lock();
  while ((_freeFDs.empty()) && 
	 ((_freeFDs.size() + _leasedFDs.size()) >= _maxFDs))
    {
      _mtx.waitCond(_condAvailableFD);
    }

  try
    {
      if (_freeFDs.empty())
	{
	  // We need to grow the pool...
	  unsigned int numCurrentFDs = _freeFDs.size() + _leasedFDs.size();
	  assert(numCurrentFDs < _maxFDs);
	  
	  adjustFdCount(numCurrentFDs + 1);
	}
      
      assert(! _freeFDs.empty());
      
      int fd = _freeFDs.front();
      _freeFDs.pop_front();
      _leasedFDs.insert(fd);

      _mtx.unlock();
      return fd;
    }
  catch (std::exception &e)
    {
      _mtx.unlock();
      throw e;
    }
}

//===============================================================================

int FileDescPool::acquireFdNonBlocking()
    throw (std::exception)
{
  LockHolder aLock(_mtx);

  // Grow the free FD pool, if we must and if we're allowed to...
  unsigned int numCurrentFDs = _freeFDs.size() + _leasedFDs.size();
  if ((_freeFDs.empty()) && (numCurrentFDs < _maxFDs))
    {
      adjustFdCount(numCurrentFDs + 1);
    }

  // If there's a FD in the free FD pool, lease it to the caller...
  if (! _freeFDs.empty())
    {
      int fd = _freeFDs.front();
      _freeFDs.pop_front();
      _leasedFDs.insert(fd);
      return fd;
    }
  else
    {
      return -1;
    }
}

//===============================================================================

void FileDescPool::releaseFd(int fd)
  throw (std::exception)
{
  LockHolder aLock(_mtx);

  int numRemoved = _leasedFDs.erase(fd);

  if (numRemoved == 0)
    {
      throw SmInternalException(__FILE__, __LINE__, 
				      "The supplied FD doesn't appear leased at the moment.");
    }
  else if (numRemoved > 1)
    {
      throw SmInternalException(__FILE__, __LINE__, 
					      "Somehow, the 'leased' pool got > 1 copy of the supplied FD in it.");
    }

  _freeFDs.push_front(fd);
  _condAvailableFD.signal();
}

//===============================================================================

void FileDescPool::closeAll()
  throw (std::exception)
{
  LockHolder aLock(_mtx);

  if (! _leasedFDs.empty())
    {
      throw SmException(__FILE__, __LINE__, "(! _leasedFDs.empty())");
    }

  _minFDs = 0;
  adjustFdCount(0);
}

//===============================================================================

void FileDescPool::adjustFdCount(int numDesiredFDs)
  throw (std::exception)
{
  int numCurrentFDs = _freeFDs.size() + _leasedFDs.size();

  if (numDesiredFDs == numCurrentFDs)
    {
      return; // No action needed.
    }
  else if (numDesiredFDs > numCurrentFDs)
    {
      // Growth is needed...
      while (numDesiredFDs > numCurrentFDs)
	{
	  allocateOneFD();
	  numCurrentFDs++;
	}
    }
  else // (numDesiredFDs < numCurrentFDs)
    {
      size_t idealReduction = numCurrentFDs - numDesiredFDs;

      for (unsigned int i = 0; i < idealReduction; i++)
	{
	  deallocateOneFD();
	}
    }
}

//===============================================================================

void FileDescPool::deallocateOneFD()
  throw (std::exception)
{
  if (_freeFDs.empty())
    {
      throw SmInternalException(__FILE__, __LINE__,
					      "_freeFDs.empty()");
    }

  int fd = _freeFDs.front();
  _freeFDs.pop_front();

  closeFile(fd);
}

//===============================================================================

void FileDescPool::allocateOneFD()
  throw (std::exception)
{
  int fd;

  if (_permissionsSet)
    { 
      fd = openFileWithFlagsAndPerms(_filepath, _openFlags, _permissions);
    }
  else
    {
      fd = openFileWithFlags(_filepath, _openFlags);
    } 

  _freeFDs.push_front(fd);
  _condAvailableFD.signal();
}

//===============================================================================
