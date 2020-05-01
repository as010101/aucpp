#include <stringutil.H>
#include <StorageMgr_Exceptions.H>
#include <sstream>

string getSubstringBeforeChar(const string & s,
			       string::size_type startIdx,
			       const char terminator)
  throw (exception)
{
  if (startIdx >= s.length())
    {
      ostringstream os;
      os << "getLeftStringBeforeChar: startIdx = " << startIdx 
	 << " but s.length() = " << s.length();
      throw SmException(__FILE__, __LINE__, os.str());
    }

  string::size_type idx = s.find(terminator);
  if (idx == string::npos)
    {
      ostringstream os;
      os << "getLeftStringBeforeChar: the terminator character: '" 
	 << terminator << "' wasn't found in the string \""
	 << s << "\"";
      throw SmException(__FILE__, __LINE__, os.str());
    }

  return s.substr(0, idx);
}

//===============================================================================

char * newCstrCopy(const string & s)
  throw (exception)
{
  string::size_type len = s.length();

  char * pszOutput = new char[len+1];

  string::size_type charsCopied = s.copy(pszOutput, len);
  pszOutput[len] = '\0';

  assert(charsCopied = len);

  return pszOutput;
}
