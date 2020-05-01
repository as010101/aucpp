#ifndef StorageMgr_Exceptions_H
#define StorageMgr_Exceptions_H

#include <assert.h>

#include <string>
#include <exception>

using namespace std;

class SmException : public std::exception
{
public:
  // Creates a new SmException.
  // 
  // Parameters:
  //   location - An optional indication of which source file contains the code
  //      that generated the exception.
  //   lineNum - An optional indication of which line number in the specified
  //      source file has the code that generated the exception.
  //   description - An optional specification of supplementary information
  //      regarding the exception.
  SmException(const string location, int lineNum, const string description);

    
  // Produces an SmException with an identical name and field definitions to those
  // of the supplied parameter.
  SmException(const SmException &src);
    
  // Sets the left hand side SmException object have a name and field definitions 
  // that are identical to those of the right-hand-side operand.
  SmException &operator=(const SmException & rhs);

  // (No externally visible behavior.)
  virtual ~SmException() throw();

  const char *what() const throw();
    
  string getLocation() const;

  int getLineNum() const;

  string getDescription() const;

private:
  SmException() : _location(string("")), _lineNum(0), _description(string("")) {assert(false);} // This should never be called.
  string _location;
  int _lineNum;
  string _description;
  string _whatValue;
};

//========================================================================================

class SmIllegalParameterValueException : public SmException
{
public:
  SmIllegalParameterValueException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmBufferTooSmallException : public SmException
{
public:
  SmBufferTooSmallException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmTooManyFieldsException : public SmException
{
public:
  SmTooManyFieldsException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmIllegalFieldNumException : public SmException
{
public:
  SmIllegalFieldNumException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmDatabaseLockedException : public SmException
{
public:
  SmDatabaseLockedException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmDatabaseExistsException : public SmException
{
public:
  SmDatabaseExistsException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmDatabaseNotFoundException : public SmException
{
public:
  SmDatabaseNotFoundException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmFilesystemException : public SmException
{
public:
  SmFilesystemException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmInternalException : public SmException
{
public:
  SmInternalException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmNoFieldDefinedException : public SmException
{
public:
  SmNoFieldDefinedException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmWrongFieldTypeException : public SmException
{
public:
  SmWrongFieldTypeException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmIllegalFieldLengthException : public SmException
{
public:
  SmIllegalFieldLengthException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmDatabaseClosedException : public SmException
{
public:
  SmDatabaseClosedException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmObjectAlreadyDefinedException : public SmException
{
public:
  SmObjectAlreadyDefinedException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmObjectNotDefinedException : public SmException
{
public:
  SmObjectNotDefinedException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmAlreadyUnpinnedException : public SmException
{
public:
  SmAlreadyUnpinnedException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmDifferentRecordTypesException : public SmException
{
public:
  SmDifferentRecordTypesException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmPageListAlreadyActiveException : public SmException
{
public:
  SmPageListAlreadyActiveException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmQBaseNotHistoricalException : public SmException
{
public:
  SmQBaseNotHistoricalException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmFieldNumUndefinedException : public SmException
{
public:
  SmFieldNumUndefinedException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmFieldNumInUseException : public SmException
{
public:
  SmFieldNumInUseException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmQueueClosedException : public SmException
{
public:
  SmQueueClosedException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmObjectNotLockedException : public SmException
{
public:
  SmObjectNotLockedException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmIllegalFrameNumException : public SmException
{
public:
  SmIllegalFrameNumException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmInvalidUserLockStateException : public SmException
{
public:
  SmInvalidUserLockStateException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmEmptyContainerException : public SmException
{
public:
  SmEmptyContainerException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmClosedException : public SmException
{
public:
  SmClosedException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmTooFullException : public SmException
{
public:
  SmTooFullException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmNotAllocatedException : public SmException
{
public:
  SmNotAllocatedException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmFileSystemFullException : public SmException
{
public:
  SmFileSystemFullException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmFileFullException : public SmException
{
public:
  SmFileFullException(const string location, int lineNum, const string description);
};

//========================================================================================

class SmTrainStepSuspendedException : public SmException
{
public:
  SmTrainStepSuspendedException(const string location, int lineNum, const string description);
};

//========================================================================================

#endif
