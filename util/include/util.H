#ifndef UTIL_H
#define UTIL_H

#include "StorageMgr_Exceptions.H"
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <vector>

using namespace std;

// If src ends with suffic, this just returns src. Otherwise, this returns
// src + suffix.
string ensureStringEndsWith(string src, char suffix)
  throw (exception);

bool dirExists(string dirPath)
  throw (SmIllegalParameterValueException,
	 SmFilesystemException,
	 SmInternalException,
	 std::exception);

// Returns true iff any object exists with the specified pathname.
bool fileExists(string filePath)
  throw (SmIllegalParameterValueException,
	 SmFilesystemException,
	 SmInternalException,
	 std::exception);

// Deletes the specified file. Throws an exception of the file didn't exist or
// if any other problem occured.
void deleteFile(string filePath)
  throw (std::exception);

// Returns a vector populated with the simple filename of each matching
// file. A file matches if its filename starts with the string 'prefix'.
// The filenames are sorted alphapetically in the return value, with 
// filenames[0] being alphabetically-first result.
//
// After this method returns, 'filenames' will be populated with and only with
// the sorted filename lists of the hits. The filenames don't include the
// pathname of the directory.
void listFilesWithPrefix(string dirpath, string prefix, vector<string> & filenames)
  throw (std::exception);

// Like the write(2) function, but automatically handles lseek, partial writes, and
// the EINTR return code.
void writeData(int fd, off_t fileOffset, const char *pBuffer, ssize_t numBytesToWrite)
  throw (SmIllegalParameterValueException,
	 SmFileFullException,
	 SmFileSystemFullException,
	 SmFilesystemException,
	 std::exception);

// Like the read(2) function, but automatically handles lseek, partial writes, and
// the EINTR return code.
void readData(int fd, off_t fileOffset, char *pBuffer, ssize_t numBytesRead)
  throw (SmIllegalParameterValueException,
	 SmFilesystemException,
	 std::exception);

// Thin wrapper on the a use of the open system call. Returns the handle, or throws
// an exception.
int openFileWithFlags(string filename, int flags)
  throw (SmIllegalParameterValueException,
	 SmFilesystemException,
	 std::exception);

// Thin wrapper on the a use of the open system call. Returns the handle, or throws
// an exception.
int openFileWithFlagsAndPerms(string filename, int flags, int perms)
  throw (SmIllegalParameterValueException,
	 SmFilesystemException,
	 std::exception);

// Thin wrapper on the a use of the close system call. 
void closeFile(int fd)
  throw (SmFilesystemException,
	 std::exception);

// Thin wrapper on the a use of the fstat system call. 
off_t getFileLen(int fd)
  throw (SmFilesystemException,
	 std::exception);

// Thin wrapper on the a use of the fstat system call. 
unsigned long getBlockSize(int fd)
  throw (SmFilesystemException,
	 std::exception);

// Returns the current offset in the file, like what's returned by lseek(...).
off_t getFileOffset(int fd)
  throw (SmFilesystemException,
	 std::exception);

// Thin wrapper on the lseek system call.
off_t setFileOffset(int fd, off_t offset, int whence)
  throw (SmFilesystemException,
	 std::exception);

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
	 std::exception);

// As trivial of a function as you're ever likely to find. This exists because 
// timeval doesn't have such a constructor. 'seconds' and 'microSeconds' must 
// both be non-negative, and 'microSeconds' must be < 1,000,000.
timeval makeTimeval(long seconds,
		    long microSeconds)
  throw (exception);

// Creates a timeval from the specified components. It's OK for 'usec' to have a
// value > 999,999. This function will normalize it appropriately.
//
// Throws an exception if the specified values exceed the capacity of a timeval.
// (E.g., if after normalization, 'sec' > max-long-value.)
//
// *** NOTE: THIS ASSUMES THAT (sec + (usec/1,000,000)) IS LESS THAN OR EQUAL TO
// *** numeric_limits<unsigned long long>::max(). IF THAT'S NOT TRUE, THEN THIS
// *** FUNCTION WILL FAIL TO DETECT OVERFLOW IN ITS CALCULATIONS.
timeval makeNormalTimeval(unsigned long long sec,
			  unsigned long long usec)
  throw (exception);

// Returns a tmeval that's the average of the specified timevals. The timevals
// are encoded as some bunch of seconds, and some bunch of microseconds.
// 'numValues' must be > 0.
timeval getAvgTimevalBySums(unsigned long long summedSeconds,
			    unsigned long long summedMicroSeconds,
			    unsigned int numValues)
  throw (exception);

// Returns the difference in times between the two moments (t2 - t1). This will
// throw an exception if overflow seems likely, but some overflows may not be
// caught.
//
// If...
//   (A) t2 >= t1, and
//   (B) both fields in each structure are non-negative, and
//   (C) t1.tv_usec < 1,000,000 and t2.tv_usec < 1,000,000
//
// Then the returned timeval will also have properties (B) and (C) (i.e., be
// 'normal').
timeval getTimevalDiff(const timeval & t1,
		       const timeval & t2)
  throw (exception);

// Just represents the fields of the timeval struct.
string timevalToSimpleString(const timeval & tv);

// Just represents the fields of the tm struct.
string tmToSimpleString(const tm & aTime);

// Renders the timeval into a form suitable for humans to read.
string timevalToHumanString(const timeval & tv);


// Returns true iff t1's members = t2's (corresponding) members.
bool timevalsEqual(const timeval & t1,
		   const timeval & t2);

// If t1 > t2, returns 1. If t1 = t2, returns 0. If t1 < t2, returns -1.
// If the timevals aren't normalized (tv_usec in [0, 999999]), this throws an 
// exception.
int timevalsComp(const timeval & t1,
		 const timeval & t2);


// Reduces the accumulator by the subtrahend. Will borrow as needed, based on the
// assumption that the numbers involved are actually:
//    ((1,000,000 * Seconds) + Micrseconds) microseconds.
//
// No assumption is made about normality of the numbers. If this method does
// happen to notice underflow (which it's not obligated to do), it will throw
// an exception.
//
// Behavior isn't defined when the accumulator will be forced negative by the
// operation.
void subtractTimestampSums(unsigned long long & accumulatorSeconds,
			   unsigned long long & accumulatorMicroSeconds,
			   unsigned long long   subrahendSeconds,
			   unsigned long long   subrahendMicroSeconds)
  throw (exception);

#endif
