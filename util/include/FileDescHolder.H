#ifndef FileDescHolder_H
#define FileDescHolder_H

#include "FileDescPool.H"

// For the entire time this object is instantiated, it holds a lock 
// on the specified mutex.
class FileDescHolder
{
public:
  FileDescHolder(FileDescPool & poolToDrawFrom, bool blockToAcquire)
    throw (std::exception);

  virtual ~FileDescHolder();

  int getFD();
private:
  FileDescPool & _pool;
  int _fd;
};

#endif
