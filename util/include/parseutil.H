#ifndef PARSEUTIL_H
#define PARSEUTIL_H

#include <string>
#include <exception>
#include <vector>

using namespace std;

// Returns a vector of the constituent strings that make up 'src'.
// 'src' is interpreted to be a sequence of constituent strings, separated
// by any of the characters appearing in 'delimiters'.
// 
// The returned vector's elements have the same ordering as the constituent
// strings in 'src'. The delimiter characters don't appear as elements in the 
// output vector, or as part of the constituent strings that are in the output
// vector.
//
// Consecutive delimiters are taken to delimit an empty string, which will
// appear in the output vector.
vector<string> unpackString(const string & src, const string & delimiters);


// The following functions attempt to parse the specified string to a particular
// data type. The functions throws an exception if the parse is unsuccessful.
bool stringToBool(const string & s)
  throw (exception);

size_t stringToSize_t(const string & s)
  throw (exception);

int stringToInt(const string & s)
  throw (exception);

unsigned int stringToUInt(const string & s)
  throw (exception);

long stringToLong(const string & s)
  throw (exception);

unsigned long stringToULong(const string & s)
  throw (exception);

unsigned long long stringToULongLong(const string & s)
  throw (exception);

double stringToDouble(const string & s)
  throw (exception);


// Finds the index info argv of the command-line argument with the specified 
// value. Throws an exception if it can't be found, or if it apperas more than 
// once.
//
// Doesn't check argv[0], as that's the name of the program, not an argument.
//
// If argname is the very last element in argv, and (! canBeLast), an exception
// is thrown. This is to benefit arguments that indicate that the next argument
// is the value to some certain parameter.
size_t getArgUniqueIndex(string argname, bool canBeLast, int argc, 
			 const char * argv[])
  throw (exception);


// The following functions find and parse a command-line argument of the form 
// "argname". 
// They throw an exception if it can't be found, or if the following cmd-line 
// argument can't be parsed as a the data type indicated by the function's name.
//
// "argname" must appear only once in the parameters.
//
// For the functions that have range parameters, if the valid value range is 
// violated, an exception is thrown.
double getUniqueDoubleArg(string argname, 
			  int argc, const char * argv[],
			  double minValue, double maxValue)
  throw (exception);


int getUniqueIntArg(string argname, 
		    int argc, const char * argv[],
		    int minValue, int maxValue)
  throw (exception);


string getUniqueStringArg(string argname, 
			  int argc, 
			  const char * argv[])
  throw (exception);


// Simply returns whether or not the specified token appears in the cmd-line
// arguments.
// 
// Doesn't check argv[0], as that's the name of the program, not an argument.
bool argInCmdLine(string argumentName,
		  int argc, const char * argv[]);

#endif
