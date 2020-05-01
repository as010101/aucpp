#include <FileDescHolder.H>

//================================================================================

FileDescHolder::FileDescHolder(FileDescPool & poolToDrawFrom, bool blockToAcquire)
  throw (std::exception)
  : _pool(poolToDrawFrom)
{
  if (blockToAcquire)
    {
      _fd = _pool.acquireFdBlocking();
    }
  else
    {
      _fd = _pool.acquireFdNonBlocking();
      if (_fd == -1)
	{
	  throw SmException(__FILE__, __LINE__, 
				      "The FileDescPool didn't have any available FDs");
	}
    }

}

//================================================================================

FileDescHolder::~FileDescHolder()
{
  try
    {
      _pool.releaseFd(_fd);
    }
  catch (std::exception & e)
    {
      // Scott Meyers makes a good argument why we should never throw an
      // exception from a destructor, unfortunately.
      assert(false);
    }
}

//================================================================================

int FileDescHolder::getFD()
{
  return _fd;
}
