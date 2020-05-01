#include <parseutil.H>
#include <StorageMgr_Exceptions.H>
#include <sstream>

//===============================================================================

vector<string> unpackString(const string & src, const string & delimiters)
{
  // Note: The basic algorithm here was taken from section 11.1.2 of Josuttis'
  // book "The Standard C++ Library"

  vector<string> returnVect;

  string::size_type startIdx;
  string::size_type endIdx;

  startIdx = src.find_first_not_of(delimiters);

  while (startIdx != string::npos)
    {
      endIdx = src.find_first_of(delimiters, startIdx);
      if (endIdx == string::npos)
	{
	  endIdx == src.length();
	}

      returnVect.push_back(src.substr(startIdx, endIdx - startIdx));

      startIdx = src.find_first_not_of(delimiters, endIdx);
    }

  return returnVect;
}

//===============================================================================

bool stringToBool(const string & s)
  throw (exception)
{
	if ((s == "T") ||
		(s == "t") ||
		(s == "TRUE") ||
		(s == "true"))
		{
			return true;
		}

	if ((s == "F") ||
		(s == "f") ||
		(s == "FALSE") ||
		(s == "false"))
		{
			return false;
		}

	ostringstream os;
	os << "Can't parse \"" << s << "\" as a bool";
	throw SmException(__FILE__, __LINE__, os.str());
}

//===============================================================================

unsigned int stringToUInt(const string & s)
  throw (exception)
{
	unsigned int returnVal;

	{
		istringstream iss(s.c_str());
		iss >> returnVal;
    
		if ((iss.fail()) || (! iss.eof()) || (iss.bad()))
			{
				ostringstream os;
				os << "Tried to parse as an unsigned int but we couldn't: \"" << s << "\"";
				throw SmException(__FILE__, __LINE__, os.str());     
			}
	}
  
	return returnVal;

	/*
  char * endPtr;
  unsigned long ulongVal = strtoul(s.c_str(), & endPtr, 10);

  if (*endPtr != '\0')
    {
      ostringstream os;
      os << "Tried to parse as an unsigned int, but we couldn'tL \"" << s << "\"";
      throw SmException(__FILE__, __LINE__, os.str());
    }

  if ((ulongVal < numeric_limits<unsigned int>::min()) ||
      (ulongVal > numeric_limits<unsigned int>::max()))
    {
      ostringstream  os;
      os << "Tried to parse as an unsigned int, but it was out of range: \"" 
		 << s << "\"";
      throw SmException(__FILE__, __LINE__, os.str());
    }
	
  // I'd like to have this as a single return statement, but new style C++ 
  // casting:   type-name(value) doesn't really work when type-name has 
  // qualifiers such as 'unsigend'. Such a scenario trigger's g++'s 
  // "old-style cast" warning... -cjc
  typedef unsigned int uint;

  return uint(ulongVal);
	*/
}

//===============================================================================

long stringToLong(const string & s)
  throw (exception)
{
  long longVal;

  {
    istringstream iss(s.c_str());
    iss >> longVal;
    
    if ((iss.fail()) || (! iss.eof()) || (iss.bad()))
      {
      ostringstream os;
      os << "Tried to parse as a long, but we couldn't: \"" << s << "\"";
      throw SmException(__FILE__, __LINE__, os.str());     
      }
  }

  return longVal;
}

//===============================================================================

unsigned long stringToULong(const string & s)
  throw (exception)
{
  unsigned long ulongVal;

  {
    istringstream iss(s.c_str());
    iss >> ulongVal;
    
    if ((iss.fail()) || (! iss.eof()) || (iss.bad()))
      {
      ostringstream os;
      os << "Tried to parse as an unsigned long, but we couldn't: \"" << s << "\"";
      throw SmException(__FILE__, __LINE__, os.str());     
      }
  }

  return ulongVal;
}

//===============================================================================

unsigned long long stringToULongLong(const string & s)
  throw (exception)
{
	unsigned long long ullVal;
	
	{
		istringstream iss(s.c_str());
		iss >> ullVal;
		
		if ((iss.fail()) || (! iss.eof()) || (iss.bad()))
			{
				ostringstream os;
				os << "Tried to parse as an unsigned long long, but we couldn't: \"" << s << "\"";
				throw SmException(__FILE__, __LINE__, os.str());     
		  }
	}
  
	return ullVal;
}

//===============================================================================

double stringToDouble(const string & s)
  throw (exception)
{
  istringstream iss(s.c_str());

  double v;
  iss >> v;
    
  if ((iss.fail()) || (! iss.eof()) || (iss.bad()))
    {
      ostringstream os;
      os << "Tried to parse \"" << s << "\" as a double, but parsing failed.";
      throw SmException(__FILE__, __LINE__, os.str());     
    }

  return v;
}

//===============================================================================

size_t stringToSize_t(const string & s)
  throw (exception)
{
  istringstream iss(s.c_str());

  size_t v;
  iss >> v;
    
  if ((iss.fail()) || (! iss.eof()) || (iss.bad()))
    {
      ostringstream os;
      os << "Tried to parse \"" << s << "\" as a size_t, but parsing failed.";
      throw SmException(__FILE__, __LINE__, os.str());     
    }

  return v;
}

//===============================================================================

int stringToInt(const string & s)
  throw (exception)
{
  istringstream iss(s.c_str());

  int v;
  iss >> v;
    
  if ((iss.fail()) || (! iss.eof()) || (iss.bad()))
    {
      ostringstream os;
      os << "Tried to parse \"" << s << "\" as a int, but parsing failed.";
      throw SmException(__FILE__, __LINE__, os.str());     
    }

  return v;
}

//===============================================================================

double getUniqueDoubleArg(string argname, 
			  int argc, 
			  const char * argv[],
			  double minValue,
			  double maxValue)
  throw (exception)
{
  size_t idx = getArgUniqueIndex(argname, false, argc, argv);
  double d = stringToDouble(argv[idx+1]);

  if ((d < minValue) || (d > maxValue))
    {
      ostringstream os;
      os << "The argument \"" << argname << "\" must have a value in the range "
	 << "[" << minValue << ", " << maxValue << "].";
      throw SmException(__FILE__, __LINE__, os.str());
    }

  return d;
}

//===============================================================================

int getUniqueIntArg(string argname, 
		    int argc, 
		    const char * argv[],
		    int minValue,
		    int maxValue)
  throw (exception)
{
  size_t idx = getArgUniqueIndex(argname, false, argc, argv);
  int i = stringToInt(argv[idx+1]);

  if ((i < minValue) || (i > maxValue))
    {
      ostringstream os;
      os << "The argument \"" << argname << "\" must have a value in the range "
	 << "[" << minValue << ", " << maxValue << "].";
      throw SmException(__FILE__, __LINE__, os.str());
    }

  return i;
}

//===============================================================================

string getUniqueStringArg(string argname, 
		       int argc, 
		       const char * argv[])
  throw (exception)
{
  size_t idx = getArgUniqueIndex(argname, false, argc, argv);
  return argv[idx+1];
}

//===============================================================================

size_t getArgUniqueIndex(string argname, bool canBeLast, int argc, 
			 const char * argv[])
  throw (exception)
{
  bool found = false;
  size_t idx;

  for (size_t i = 1; i < argc; ++i)
    {
      if (argname == argv[i])
	{
	  if (found)
	    {
	      ostringstream os;
	      os << "The parameter \"" << argname << "\" appears more than once";
	      throw SmException(__FILE__, __LINE__, os.str());
	    }

	  found = true;
	  idx = i; 
	}
    }

  if (! found)
    {
      ostringstream os;
      os << "The parameter \"" << argname << "\" wasn't specified";
      throw SmException(__FILE__, __LINE__, os.str());
    }

  if ((idx == (argc - 1)) && (! canBeLast))
    {
      ostringstream os;
      os << "The parameter \"" << argname << "\" can't be the last parameter, "
	 << "because a value is supposed to follow it.";
      throw SmException(__FILE__, __LINE__, os.str());
    }

  return idx;
}

//===============================================================================

bool argInCmdLine(string argumentName,
		    int argc, const char * argv[])
{

  for (size_t i = 1; i < argc; ++i)
    {
      if (argumentName == argv[i])
	{
	  return true;
	}
    }

  return false;
}

//===============================================================================
