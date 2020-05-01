#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <exception>
#include <string>

using namespace std;

// Returns the substring of 's' between the 'startIdx' index to (but not 
// including) the first occurance of the character 'terminator'.
//
// If 'startIdx' is > the max index into 's', an exception is thrown.
//
// If 'terminator' doesn't appear in 's', an exception will be
// thrown.
string getSubstringBeforeChar(const string & s,
			       string::size_type startIdx,
			       const char terminator)
  throw (exception);


// Creates on the heap, using the 'new char[...]', a NULL-terminated c-string
// with the same content as 's'. The caller accepts ownership of the allocated
// c-str.
char * newCstrCopy(const string & s)
  throw (exception);


#endif
