#include <StorageMgr_Exceptions.H>
#include <sstream>
#include <iostream>

//*********************************************************************************
// Exception class implementation
//*********************************************************************************
  
SmException::SmException(const string location, 
			 int lineNum, 
			 const string description) :
  exception(), 
  _location(location),
  _lineNum(lineNum),
  _description(description)
{
  try
    {
      ostringstream msgStream;
      msgStream << "StorageMgr::SmException-based exception thrown:" << endl
		<< "   Location: "    << _location                 << endl
		<< "   Line number: " << _lineNum                  << endl
		<< "   Description: " << _description              << endl;

      _whatValue = msgStream.str();
    }
  catch (...)
    {
      // Things got just too bad, but we don't want to hide this problem either.
      abort();
    }
}
  
//================================================================================
  
SmException::SmException(const SmException &src) :
  exception(),
  _location(src._location),
  _lineNum(src._lineNum),
  _description(src._description),
  _whatValue(src._whatValue)
{
}
  
//================================================================================
  
SmException & SmException::operator=(const SmException & src)
{
  _location = src._location;
  _lineNum = src._lineNum;
  _description = src._description;
  _whatValue = src._whatValue;
  return *this;
}
  
//================================================================================
  
SmException::~SmException() 
  throw()
{
}
  
//================================================================================

const char * SmException::what() 
  const throw()
{
  return _whatValue.c_str();
}
  
//================================================================================
  
string SmException::getLocation() const
{
  return _location;
}
  
//================================================================================
  
int SmException::getLineNum() const
{
  return _lineNum;
}
  
//================================================================================
  
string SmException::getDescription() const
{
  return _description;
}
  
//****************************************************************************************
// Exception-derived classes implementations
//****************************************************************************************

SmInternalException::SmInternalException(const string location, 
					 int lineNum, 
					 const string description) :
  SmException(location, lineNum, description)
{
}
  
SmFilesystemException::SmFilesystemException(const string location, 
					     int lineNum, 
					     const string description) :
  SmException(location, lineNum, description)
{
}
  
SmIllegalParameterValueException::SmIllegalParameterValueException(const string location, 
								   int lineNum, 
								   const string description) :
  SmException(location, lineNum, description)
{
}
  
SmDatabaseExistsException::SmDatabaseExistsException(const string location, 
						     int lineNum, 
						     const string description) :
  SmException(location, lineNum, description)
{
}
  
SmDatabaseClosedException::SmDatabaseClosedException(const string location, 
						     int lineNum, 
						     const string description) :
  SmException(location, lineNum, description)
{
}
  
SmDatabaseNotFoundException::SmDatabaseNotFoundException(const string location, 
							 int lineNum, 
							 const string description) :
  SmException(location, lineNum, description)
{
}
  
SmObjectAlreadyDefinedException::SmObjectAlreadyDefinedException(const string location, 
								 int lineNum, 
								 const string description) :
  SmException(location, lineNum, description)
{
}
  
SmObjectNotDefinedException::SmObjectNotDefinedException(const string location, 
							 int lineNum, 
							 const string description) :
  SmException(location, lineNum, description)
{
}
  
SmDifferentRecordTypesException::SmDifferentRecordTypesException(const string location, 
								 int lineNum, 
								 const string description) :
  SmException(location, lineNum, description)
{
}
  
SmQueueClosedException::SmQueueClosedException(const string location, 
					       int lineNum, 
					       const string description) :
  SmException(location, lineNum, description)
{
}

SmObjectNotLockedException::SmObjectNotLockedException(const string location, 
						       int lineNum, 
						       const string description) :
  SmException(location, lineNum, description)
{
}
  
SmIllegalFrameNumException::SmIllegalFrameNumException(const string location, 
						       int lineNum, 
						       const string description) :
  SmException(location, lineNum, description)
{
}
  
SmInvalidUserLockStateException::SmInvalidUserLockStateException(const string location, 
								 int lineNum, 
								 const string description) :
  SmException(location, lineNum, description)
{
}
  
SmEmptyContainerException::SmEmptyContainerException(const string location, 
						     int lineNum, 
						     const string description) :
  SmException(location, lineNum, description)
{
}
  
SmClosedException::SmClosedException(const string location, 
				     int lineNum, 
				     const string description) :
  SmException(location, lineNum, description)
{
}

SmTooFullException::SmTooFullException(const string location, 
				       int lineNum, 
				       const string description) :
  SmException(location, lineNum, description)
{
}

SmNotAllocatedException::SmNotAllocatedException(const string location, 
						 int lineNum, 
						 const string description) :
  SmException(location, lineNum, description)
{
}

SmFileSystemFullException::SmFileSystemFullException(const string location, 
						     int lineNum, 
						     const string description) :
  SmException(location, lineNum, description)
{
}

SmFileFullException::SmFileFullException(const string location, 
					 int lineNum, 
					 const string description) :
  SmException(location, lineNum, description)
{
}

SmTrainStepSuspendedException::SmTrainStepSuspendedException(const string location, 
							     int lineNum, 
							     const string description) :
  SmException(location, lineNum, description)
{
}
