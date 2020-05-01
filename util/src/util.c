#include <util.H>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <algorithm>
#include <iostream>
#include <sstream>

using namespace std;

string ensureStringEndsWith(string src, char suffix)
  throw (exception)
{
  if (src.length() == 0)
    {
      return string(1, suffix);
    }
  else if (src.at(src.length()-1) == suffix)
    {
      return src;
    }
  else
    {
      return src + suffix;
    }
}

// Returs true iff the specified directory exists. The path
// may contain a trailing slash without consequence.
// If the path exists but the specified file is not a directory,
// this still returns false.
bool dirExists(string dirPath)
  throw (SmIllegalParameterValueException,
	 SmFilesystemException,
	 SmInternalException,
	 std::exception)
{
  if (dirPath.length() == 0)
    {
      throw SmIllegalParameterValueException(__FILE__, __LINE__, "dirPath had a length of 0");
    }
  
  // Ensure the directory name doesn't end in a '/'...
  string aPath;
  if ((dirPath.length() > 1) && (dirPath.at(dirPath.length()-1) == '/'))
    {
      aPath = dirPath.substr(0, dirPath.length()-1);
    }
  else
    {
      aPath = dirPath;
    }
  
  struct stat fileStat;
  int rc = lstat(aPath.c_str(), &fileStat);
  if (rc == 0)
    {
      if (S_ISDIR(fileStat.st_mode))
	{
	  return true;
	}
      else
	{
	  return false;
	}
    }
  
  switch (errno)
    {
    case EBADF:
      throw SmInternalException(__FILE__, __LINE__, "stat returned EBADF, but only 'fstat' should return that code");
    case ENOENT:
      return false;
    case ENOTDIR:
      return false;
    case ELOOP:
      throw SmFilesystemException(__FILE__, __LINE__, "stat returned ELOOP (Too many symbolic links traversed).");
    case EFAULT:
      throw SmInternalException(__FILE__, __LINE__, "stat returned EFAULT");
    case EACCES:
      throw SmFilesystemException(__FILE__, __LINE__, "stat returned ECCES (Perimssion denied).");
    case ENOMEM:
      throw SmInternalException(__FILE__, __LINE__, "stat returned ENOMEM");
    case ENAMETOOLONG:
      throw SmIllegalParameterValueException(__FILE__, __LINE__, "stat returned ENAMETOOLONG");
    }

  // Control should never reach here, so this is just to shut-up the compiler's warnings...
  assert(false);
  return false;
}

//======================================================================================================

bool fileExists(string filePath)
  throw (SmIllegalParameterValueException,
	 SmFilesystemException,
	 SmInternalException,
	 std::exception)
{
  if (filePath.length() == 0)
    {
      throw SmIllegalParameterValueException(__FILE__, __LINE__, "filePath had a length of 0");
    }

  // Ensure the pathname doesn't end in a '/'...
  string aPath;
  if ((filePath.length() > 1) && (filePath.at(filePath.length()-1) == '/'))
    {
      aPath = filePath.substr(0, filePath.length()-1);
    }
  else
    {
      aPath = filePath;
    }

  struct stat fileStat;
  int rc = lstat(aPath.c_str(), &fileStat);
  if (rc == 0)
    {
      return true;
    }

  switch (errno)
    {
    case EBADF:
      throw SmInternalException(__FILE__, __LINE__, "stat returned EBADF, but only 'fstat' should return that code");
    case ENOENT:
      return false;
    case ENOTDIR:
      return false;
    case ELOOP:
      throw SmFilesystemException(__FILE__, __LINE__, "stat returned ELOOP (Too many symbolic links traversed).");
    case EFAULT:
      throw SmInternalException(__FILE__, __LINE__, "stat returned EFAULT");
    case EACCES:
      throw SmFilesystemException(__FILE__, __LINE__, "stat returned ECCES (Perimssion denied).");
    case ENOMEM:
      throw SmInternalException(__FILE__, __LINE__, "stat returned ENOMEM");
    case ENAMETOOLONG:
      throw SmIllegalParameterValueException(__FILE__, __LINE__, "stat returned ENAMETOOLONG");
    }
  
  // Control should never reach here, so this is just to shut-up the compiler's warnings...
  assert(false);
  return false;
}

//======================================================================================================

void deleteFile(string filePath)
  throw (std::exception)
{
  int rc = unlink(filePath.c_str());
  if (rc != 0)
    {
      string errString = "Couldn't delete the file \"";
      errString += filePath + "\". Error = ";
      errString += strerror(errno);
      throw SmException(__FILE__, __LINE__, errString);
    }
}

//======================================================================================================

void listFilesWithPrefix(string dirpath, string prefix, vector<string> & filenames)
  throw (std::exception)
{
  // I'm avoiding using the scandir(...) Linux function, because I don't want to get into the use of
  // a static variable in this .C file just so we can tell that function what prefix to match on.
  // So yet, I'm doing it the ineffecient way, but we can optimize later if needed. -cjc
  filenames.clear();

  DIR * pDir = opendir(dirpath.c_str());
  if (pDir == NULL)
    {
      string errString = "Couldn't open directory: ";
      errString += strerror(errno);
      throw SmException(__FILE__, __LINE__, errString);
    }

  struct dirent * pDirEntry;
  while ((pDirEntry = readdir(pDir)) != NULL)
    {
      string aFilename(pDirEntry->d_name);
      if (aFilename.find(prefix) == 0)
	{
	  filenames.push_back(aFilename);
	}
    }

  int rc = closedir(pDir);
  if (rc == -1)
    {
      string errString = "Couldn't close directory: ";
      errString += strerror(errno);
      throw SmException(__FILE__, __LINE__, errString);
    }

  // Sort the hits...
  sort(filenames.begin(), filenames.end());
}

//======================================================================================================

void writeData(int fd, off_t fileOffset, const char *pBuffer, ssize_t numBytesToWrite)
  throw (SmIllegalParameterValueException,
	 SmFileFullException,
	 SmFileSystemFullException,
	 SmFilesystemException,
	 std::exception)
{
  // Validate the parameters...
  if (pBuffer == NULL)
    {
      throw SmIllegalParameterValueException(__FILE__, __LINE__, "pBuffer == NULL");
    }

  if (numBytesToWrite < 0)
    {
      throw SmIllegalParameterValueException(__FILE__, __LINE__, "numBytesToWrite < 0");
    }

  // Do the real work...
  setFileOffset(fd, fileOffset, SEEK_SET);

  ssize_t totalBytesWritten = 0;

  while (totalBytesWritten < numBytesToWrite)
    {
      int bytesJustWritten = write(fd, & (pBuffer[totalBytesWritten]), 
				   numBytesToWrite - totalBytesWritten);

      if (bytesJustWritten >= 0)
	{
	  totalBytesWritten += bytesJustWritten;
	}
      else if (errno == EFBIG)
	{
	  string errorMsg = "write failed: ";
	  errorMsg += strerror(errno);
	  throw SmFileFullException(__FILE__, __LINE__, errorMsg);
	}
      else if (errno == ENOSPC)
	{
	  string errorMsg = "write failed: ";
	  errorMsg += strerror(errno);
	  throw SmFileSystemFullException(__FILE__, __LINE__, errorMsg);
	}
      else if (errno != EINTR)
	{
	  string errorMsg = "write failed: ";
	  errorMsg += strerror(errno);
	  throw SmFilesystemException(__FILE__, __LINE__, errorMsg);
	}
    }
}

//======================================================================================================

void readData(int fd, off_t fileOffset, char *pBuffer, ssize_t numBytesToRead)
  throw (SmIllegalParameterValueException,
	 SmFilesystemException,
	 std::exception)
{
  // Validate the parameters...
  if (pBuffer == NULL)
    {
      throw SmIllegalParameterValueException(__FILE__, __LINE__, "pBuffer == NULL");
    }

  if (numBytesToRead < 0)
    {
      throw SmIllegalParameterValueException(__FILE__, __LINE__, "numBytesToWrite < 0");
    }

  // Do the real work...
  setFileOffset(fd, fileOffset, SEEK_SET);

  ssize_t totalBytesRead = 0;

  while (totalBytesRead < numBytesToRead)
    {
      int bytesJustRead = read(fd, & (pBuffer[totalBytesRead]), 
				   numBytesToRead - totalBytesRead);

      if (bytesJustRead >= 0)
	{
	  totalBytesRead += bytesJustRead;
	}
      else if (errno != EINTR)
	{
	  string errorMsg = "read failed: ";
	  errorMsg += strerror(errno);
	  throw SmFilesystemException(__FILE__, __LINE__, errorMsg);
	}
    }
}

//======================================================================================================

int openFileWithFlags(string filename, int flags)
  throw (SmIllegalParameterValueException,
	 SmFilesystemException,
	 std::exception)
{
  if (filename.empty())
    {
      throw SmIllegalParameterValueException(__FILE__, __LINE__, "The filename parameter is the empty string");
    }

  int fd = open(filename.c_str(), flags);

  if (fd == -1)
    {
      string errorMsg = "open failed: ";
      errorMsg += strerror(errno);
      throw SmFilesystemException(__FILE__, __LINE__, errorMsg);
    }

  return fd;
}

//======================================================================================================

int openFileWithFlagsAndPerms(string filename, int flags, int perms)
  throw (SmIllegalParameterValueException,
	 SmFilesystemException,
	 std::exception)
{
  if (filename.empty())
    {
      throw SmIllegalParameterValueException(__FILE__, __LINE__, "The filename parameter is the empty string");
    }

  int fd = open(filename.c_str(), flags, perms);

  if (fd == -1)
    {
      string errorMsg = "open failed on file \"";
      errorMsg += filename;
      errorMsg += "\". Error = ";
      errorMsg += strerror(errno);
      throw SmFilesystemException(__FILE__, __LINE__, errorMsg);
    }

  return fd;
}

//======================================================================================================

void closeFile(int fd)
  throw (SmFilesystemException,
	 std::exception)
{
  int rc = close(fd);

  if (rc == -1)
    {
      string errorMsg = "close failed: ";
      errorMsg += strerror(errno);
      throw SmFilesystemException(__FILE__, __LINE__, errorMsg);
    }
}

//======================================================================================================

off_t getFileLen(int fd)
  throw (SmFilesystemException,
	 std::exception)
{
  struct stat fileStats;
  int rc = fstat(fd, & fileStats);
  
  if (rc == -1)
    {
      string errorMsg = "fstat failed: ";
      errorMsg += strerror(errno);
      throw SmFilesystemException(__FILE__, __LINE__, errorMsg);
    }
  
  return  fileStats.st_size;
}

//======================================================================================================

unsigned long getBlockSize(int fd)
  throw (SmFilesystemException,
	 std::exception)
{
  struct stat fileStats;
  int rc = fstat(fd, & fileStats);
  
  if (rc == -1)
    {
      string errorMsg = "fstat failed: ";
      errorMsg += strerror(errno);
      throw SmFilesystemException(__FILE__, __LINE__, errorMsg);
    }
  
  return  fileStats.st_blksize;
}

//======================================================================================================

off_t getFileOffset(int fd)
  throw (SmFilesystemException,
	 std::exception)
{
  return setFileOffset(fd, SEEK_CUR, 0);
}

//======================================================================================================

off_t setFileOffset(int fd, off_t offset, int whence)
  throw (SmFilesystemException,
	 std::exception)
{
  off_t newOffset = lseek(fd, offset, whence);
  if (newOffset == -1)
    {
      string errorMsg = "lseek failed: ";
      errorMsg += strerror(errno);
      throw SmFilesystemException(__FILE__, __LINE__, errorMsg);
    }

  return offset;
}

//======================================================================================================

// Tries to set the file the specified size. If the file needs to grow to meet the
// specified size, but we run out of disk space first, then this reset the file to the size it was
// when this function was invoked, and then throw a SmTooFullException exception.
//
// If 'forceAlloc' is true, then this will explicitely write a '.' character to every 
// byte that the file is grown by, to ensure that the space is really given to it
// by the file system. 
void setFileLen(int fd, off_t newFileLen, bool forceAlloc)
  throw (SmIllegalParameterValueException,
	 SmFileFullException,
	 SmFileSystemFullException,
	 SmFilesystemException,
	 std::exception)
{
  if (newFileLen < 0)
    {
      throw SmIllegalParameterValueException(__FILE__, __LINE__, "newFileLen < 0");
    }

  off_t oldFileLen = getFileLen(fd);

  if (newFileLen == oldFileLen)
    {
      return; // Nothing to do.
    }
  else if (newFileLen < oldFileLen)
    {
      int rc = ftruncate(fd, newFileLen);
      if (rc == -1)
	{
	  string errorMsg = "ftruncate failed: ";
	  errorMsg += strerror(errno);
	  throw SmFilesystemException(__FILE__, __LINE__, errorMsg);
	}
    }
  else // newFileLen > currentFileLen
    {
      try
	{
	  if (! forceAlloc)
	    {
	      const char * pFillChar = ".";
	      writeData(fd, newFileLen - 1, pFillChar, 1); 
	    }
	  else
	    {
	      // Set up our fill pattern...
	      unsigned long blockSize = getBlockSize(fd);
	      char * pFillBuffer = new char[blockSize];
	      memset(pFillBuffer, '.', blockSize);
	      
	      // Let's do this in a block-alligned manner, if we can.
	      off_t currentFileLen = oldFileLen;

	      if ((currentFileLen % blockSize) != 0)
		{
		  // The beginning of the new file area isn't block alligned. Let's get 
		  // block alligned, if we have enough bytes to do so.
		  off_t nextFullBlockFileLen = blockSize * ((currentFileLen / blockSize) + 1);
		  off_t shortTermFileLen = min(nextFullBlockFileLen, newFileLen);
		  off_t bytesToAdd = shortTermFileLen - currentFileLen;
		  
		  writeData(fd, currentFileLen, pFillBuffer, bytesToAdd);
		  currentFileLen += bytesToAdd;
		}
	      
	      // Now copy as many whole blocks as possible...
	      int wholeBlocksToCopy = ((newFileLen - currentFileLen) / blockSize);
	      
	      for (int i = 0; i < wholeBlocksToCopy; i++)
		{
		  writeData(fd, currentFileLen, pFillBuffer, blockSize);
		  currentFileLen += blockSize;
		}
	      
	      // Finally, copy any remaining data...
	      off_t bytesToAdd = newFileLen - currentFileLen;
	      if (bytesToAdd > 0)
		{
		  writeData(fd, currentFileLen, pFillBuffer, bytesToAdd);
		}
	    }
	}
      catch (SmFileFullException &e)
	{
	  // The files couldn't handle the desired growth. Undo any growth this method
	  // invocation accomplished...
	  int rc = ftruncate(fd, oldFileLen);
	  if (rc == -1)
	    {
	      string errorMsg = "ftruncate failed: ";
	      errorMsg += strerror(errno);
	      throw SmFilesystemException(__FILE__, __LINE__, errorMsg);
	    }

	  throw e;
	}
      catch (SmFileSystemFullException &e)
	{
	  // The filesystem couldn't handle the desired growth. Undo any growth this method
	  // invocation accomplished...
	  int rc = ftruncate(fd, oldFileLen);
	  if (rc == -1)
	    {
	      string errorMsg = "ftruncate failed: ";
	      errorMsg += strerror(errno);
	      throw SmFilesystemException(__FILE__, __LINE__, errorMsg);
	    }

	  throw e;
	}
    }
}

//======================================================================================================

timeval makeTimeval(long seconds,
		    long microSeconds)
  throw (exception)
{
  if (seconds < 0)
    {
      throw SmException(__FILE__, __LINE__, "seconds < 0");
    }

  if ((microSeconds < 0) || (microSeconds >= 1000000))
    {
      throw SmException(__FILE__, __LINE__, "(microSeconds < 0) || (microSeconds >= 1000000)");
    }

  timeval tv;
  tv.tv_sec = seconds;
  tv.tv_usec = microSeconds;
  return tv;
}

//======================================================================================================

// Creates a timeval from the specified components. It's OK for 'usec' to have a
// value > 999,999. This function will normalize it appropriately.
//
// Throws an exception if the specified values exceed the capacity of a timeval.
// (E.g., if after normalization, 'sec' > max-long-value.)
timeval makeNormalTimeval(unsigned long long sec,
			  unsigned long long usec)
  throw (exception)
{
  unsigned long long usecWholeSeconds = (usec / 1000000);
  int usecMicroseconds                = (usec % 1000000);

  static long maxLong = numeric_limits<long>::max();

  sec += usecWholeSeconds;

  // A timeval's fields are of type 'long'...
  if (sec > maxLong)
    {
      throw SmException(__FILE__, __LINE__, "Number of seconds (after normalization) doesn't fit in a long int");
    }

  timeval tv;
  tv.tv_sec = sec;
  tv.tv_usec = usecMicroseconds;
  return tv;
}

//======================================================================================================

timeval getAvgTimevalBySums(unsigned long long summedSeconds,
			    unsigned long long summedMicroSeconds,
			    unsigned int numValues)
  throw (exception)
{
  //-----------------------------------------------------------------------------
  // Later on, it would be nice to do this check only at build time...
  //
  // Ensure no overflow:
  // We know that (summedSeconds % numValues) is in the range [0, (numValues - 1)].
  // Therefore, the experession (1000000 * summedSeconds / numValues) is definitely 
  // in the range:
  //    [0, 1000000 * (numValues - 1)].
  // summedMicroSeconds is bounded by the range:
  //    [0, 999999 * numValues].
  //
  // Finally, we conclude that the expression: (leftoverMicroSec + summedMicroSeconds)
  // has an upper bound value of:
  //    (1000000 * (numValues - 1) + 999999 * numValues), which reduces to:
  //    (1999999 * numValues - 1000000).
  // So just make sure that the intermediate variable holding the value
  //    (leftoverMicroSec + summedMicroSeconds) 
  // has the needed capacity.
  static unsigned long long maxUsignedLongLong = 
    numeric_limits<unsigned long long>::max();

  static unsigned int maxUnsignedInt = numeric_limits<unsigned int>::max();

  assert(maxUsignedLongLong >= (1999999 * maxUnsignedInt - 1000000));

  //-----------------------------------------------------------------------------

  if (numValues == 0)
    {
      throw SmException(__FILE__, __LINE__, "numValues == 0");
    }

  timeval returnVal;
  returnVal.tv_sec = (summedSeconds / numValues);

  unsigned long long leftoverMicroSec = 1000000 * (summedSeconds % numValues);
  unsigned long long usec = leftoverMicroSec + summedMicroSeconds;
  unsigned long long avgUsec = usec / numValues;

  returnVal.tv_usec =  (avgUsec % 1000000);

  if ((avgUsec / 1000000) > numValues)
    {
      ostringstream os;
      os << "Internal error: Unexpected overflow in an averaging calculation" << endl
	 << "   summedSeconds        = " << summedSeconds << endl
	 << "   summedMicroSeconds   = " << summedMicroSeconds << endl
	 << "   numValues            = " << numValues << endl
	 << "   leftoverMicroSec     = " << leftoverMicroSec << endl
	 << "   usec                 = " << usec << endl
	 << "   avgUsec              = " << avgUsec;
      throw SmException(__FILE__, __LINE__, os.str());
    }

  returnVal.tv_sec += (avgUsec/ 1000000);

  return returnVal;
}

//======================================================================================================

timeval getTimevalDiff(const timeval & t1,
		       const timeval & t2)
  throw (exception)
{
  timeval tv;
  tv.tv_sec  = t2.tv_sec  - t1.tv_sec;
  tv.tv_usec = t2.tv_usec - t1.tv_usec;

  // Normalize if we need to...
  if (tv.tv_usec < 0)
    {
      tv.tv_usec += 1000000;
      tv.tv_sec  -= 1;
      assert(tv.tv_usec >= 0);
    }

  return tv;
}

//======================================================================================================

string timevalToSimpleString(const timeval & tv)
{
  ostringstream os;
  os << "{ tv_sec = " << tv.tv_sec << ", tv_usec = " << tv.tv_usec << " }";
  return os.str();
}

//======================================================================================================

string tmToSimpleString(const tm & aTime)
{
  ostringstream os;

  os << "{ tm_sec = " << aTime.tm_sec
	 << ", tm_min = " << aTime.tm_min
	 << ", tm_hour = " << aTime.tm_hour
	 << ", tm_mday = " << aTime.tm_mday
	 << ", tm_mon = " << aTime.tm_mon
	 << ", tm_year = " << aTime.tm_year
	 << ", tm_wday = " << aTime.tm_wday
	 << ", tm_yday = " << aTime.tm_yday
	 << ", tm_isdst = " << aTime.tm_isdst
	 << " }";

  return os.str();
}

//======================================================================================================

bool timevalsEqual(const timeval & t1,
		   const timeval & t2)
{
  return (t1.tv_sec == t2.tv_sec) && (t1.tv_usec == t2.tv_usec);
}

//======================================================================================================

int timevalsComp(const timeval & t1,
		 const timeval & t2)
{
  if ((t1.tv_usec < 0) || (t1.tv_usec >= 1000000))
    {
      throw SmException(__FILE__, __LINE__, "t1 isn't normalized");
    }

  if ((t2.tv_usec < 0) || (t2.tv_usec >= 1000000))
    {
      throw SmException(__FILE__, __LINE__, "t2 isn't normalized");
    }

  if (t1.tv_sec > t2.tv_sec)
    {
      return 1;
    }
  else if (t1.tv_sec < t2.tv_sec)
    {
      return -1;
    }
  else if (t1.tv_usec > t2.tv_usec)
    {
      return 1;
    }
  else if (t1.tv_usec < t2.tv_usec)
    {
      return -1;
    }
  else
    {
      return 0;
    }
}

//===============================================================================

void subtractTimestampSums(unsigned long long & accumulatorSeconds,
			   unsigned long long & accumulatorMicroSeconds,
			   unsigned long long   subtrahendSeconds,
			   unsigned long long   subtrahendMicroSeconds)
  throw (exception)
{
  const unsigned long long ullMillion = 1000000;

  if ((accumulatorSeconds < subtrahendSeconds) && 
      (accumulatorMicroSeconds < subtrahendMicroSeconds))
    {
      ostringstream os;
      os << "Underflow w/no chance of borrowing: " << endl
	 << "   accumulatorSeconds = " << accumulatorSeconds << endl
	 << "   accumulatorMicroSeconds = " << accumulatorMicroSeconds << endl
	 << "   subtrahendSeconds = " << subtrahendSeconds << endl
	 << "   subtrahendMicroSeconds = " << subtrahendMicroSeconds;
      throw SmException(__FILE__, __LINE__, os.str());
    }

  // We know borrowing will be at most one-way because of the earlier test...

  if (accumulatorMicroSeconds < subtrahendMicroSeconds)
    {
      //-------------------------------------------------------------------------
      // Tackle microsSeconds borrowing from seconds...
      //-------------------------------------------------------------------------
      accumulatorSeconds -= subtrahendSeconds;

      unsigned long long microSecondsDeficit = 
	subtrahendMicroSeconds - accumulatorMicroSeconds;

      // Borrow from the Seconds, if we can...
      unsigned long long secondsToBorrow = ((microSecondsDeficit % ullMillion) == 0) ? 
	(microSecondsDeficit / ullMillion) : 
	((microSecondsDeficit / ullMillion) + 1);

      if (secondsToBorrow > accumulatorSeconds)
	{
	  throw SmException(__FILE__, __LINE__, "Mathematical underflow");
	}

      accumulatorSeconds -= secondsToBorrow;
      accumulatorMicroSeconds += 
	((ullMillion * secondsToBorrow) - subtrahendMicroSeconds);
    }
  else if (accumulatorSeconds < subtrahendSeconds)
    {
      //-------------------------------------------------------------------------
      // Tackle borrowing from microseconds...
      //-------------------------------------------------------------------------
      accumulatorMicroSeconds -= subtrahendMicroSeconds;

      unsigned long long secondsDeficit = subtrahendSeconds - accumulatorSeconds;

      // Borrow from the MicroSeconds, if we can...
      unsigned long long microSecondsToBorrow = secondsDeficit * ullMillion;

      if (microSecondsToBorrow > accumulatorMicroSeconds)
	{
	  throw SmException(__FILE__, __LINE__, "Mathematical underflow");
	}

      accumulatorMicroSeconds -= microSecondsToBorrow;

      // we borrowed exactly enough Microseconds to be non-negative, so Seconds 
      // is definitely 0.
      accumulatorSeconds = 0; 
    }
  else
    {
      //-------------------------------------------------------------------------
      // No borrowing needed in from either field...
      //-------------------------------------------------------------------------
      accumulatorSeconds -= subtrahendSeconds;
      accumulatorMicroSeconds -= subtrahendMicroSeconds;
    }
}

//===============================================================================

string timevalToHumanString(const timeval & tv)
{
	char x[20+1];
	tm myTm;
	localtime_r(& tv.tv_sec, & myTm);
	strftime(x, 100, "%d %b %Y %T", & myTm);
	return string(x);
}

//===============================================================================
