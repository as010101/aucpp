#ifndef PROPSFILE_H
#define PROPSFILE_H

#include <map>
#include <string>
#include <exception>
#include <algorithm>
#include <set>

using namespace std;

/**
   This class provides a read-only view of an XML-based properties file. The file
   is expected to have one element for every property. Each element has an element
   name (as per normal XML) which gives the property's name, and two attributes:
   "type", and "value".

   No two elements in a given properties file may have the same name.

   "type" must be one of the following values:
   bool
   int
   uint
   long
   ulong
   ulonglong
   double
   string

   "value" must be parsable according to the specified "type".

   For example:

   <foo type=int value=3 />
   <bar type=string value="antelope and deer" />
   <baz type=string value=porcshe />


   NOTES:
   The file's content is read only during the construction of this PropsFile 
   object. Changs to the underlying file won't be apparent until a new PropsFile
   object is created.

   CONCURRENCY:
   Concurrent invocations may be rampant.
*/


class PropsFile
{
public:
	// Loads a particular properties file, and models it with an instance of this
	// class.
	PropsFile(string filepath)
		throw (exception);

	// Loads a particular properties file, and models it with an instance of this
	// class.
	// Then, all of the command-line arguments (supplied via argc/argv) that begin
	// with 'prefix' will be used to override existing entries in the PropsFile
	// or to add new entries in the PropsFile.
	//
	// Example:  prefix = "-D", argc/v = "myprog -Dfoo:int=42 blah -Dcow:string=milk"
	// 
	//    In this case, regardless of what was given in the XML file, we know for
	//    sure that this PropsFile will have at least the following two entries:
	//    (int)    foo = 42
	//    (string) cow = milk
	//
	// An exception is thrown if any command line argument starting with the given
	// prefix is malformed, or if more than one cmd-line argument defines the
	// same property name.
	//
	// 'prefix' must not be the empty string, and argv[0] is ignored.
	PropsFile(string filepath, string prefix, int argc, const char* argv[])
		throw (exception);

	virtual ~PropsFile();


	//-----------------------------------------------------------------------------

	// Returns the full property names and types of all string-properies whose 
	// names begin with the substring 'prefix'.
	//
	// The key string in the map is the property's full name.
	//
	// The dependent value is one of the following values:
	//    "bool", "int", "uint", "long", "ulong", "ulonglong", "double", "string"
	map<string, string> findPropsByPrefix(string prefix) const
		throw (exception);

	//-----------------------------------------------------------------------------

	/*
	  This methods throw an exception if the specified property isn't defined with
	  the type indicated by the method.

	  If that's not acceptable, use the two-parameter versions, which return a default
	  value when the property isn't defined.
	*/

	bool getBool(string propName) const
		throw (exception);

	bool isBoolPropDefined(string propName) const
		throw (exception);

	bool getBoolWithDefault(string propName, bool defaultValue) const;

	//-----------------------------------------------------------------------------

	int getInt(string propName) const
		throw (exception);

	bool isIntPropDefined(string propName) const
		throw (exception);

	int getIntWithDefault(string propName, int defaultValue) const;

	//-----------------------------------------------------------------------------

	unsigned int getUInt(string propName) const
		throw (exception);

	bool isUIntPropDefined(string propName) const
		throw (exception);

	unsigned int getUIntWithDefault(string propName, unsigned int defaultValue) const;

	//-----------------------------------------------------------------------------

	long getLong(string propName) const
		throw (exception);

	bool isLongPropDefined(string propName) const
		throw (exception);

	long getLongWithDefault(string propName, long defaultValue) const;

	//-----------------------------------------------------------------------------

	unsigned long getULong(string propName) const
		throw (exception);

	bool isULongPropDefined(string propName) const
		throw (exception);

	unsigned long getULongWithDefault(string propName, unsigned long defaultValue) const;

	//-----------------------------------------------------------------------------

	unsigned long long getULongLong(string propName) const
		throw (exception);

	bool isULongLongPropDefined(string propName) const
		throw (exception);

	unsigned long long getULongLongWithDefault(string propName, unsigned long long defaultValue) const;

	//-----------------------------------------------------------------------------

	double getDouble(string propName) const
		throw (exception);

	bool isDoubleDefined(string propName) const
		throw (exception);

	double getDoubleWithDefault(string propName, double defaultValue) const;

	//-----------------------------------------------------------------------------

	string getString(string propName) const
		throw (exception);

	bool isStringPropDefined(string propName) const
		throw (exception);

	string getStringWithDefault(string propName, string defaultValue) const;

	//-----------------------------------------------------------------------------

private:
	// The various stored values. Key = prop name, dependent value = prop value.
	map<string, bool>               _boolMap;
	map<string, int>                _intMap;
	map<string, unsigned int>       _uintMap;
	map<string, long>               _longMap;
	map<string, unsigned long>      _ulongMap;
	map<string, unsigned long long> _ulongLongMap;
	map<string, double>             _doubleMap;
	map<string, string>             _stringMap;

	struct AliasTarget
	{
		string _propName;
		string _propType;

		AliasTarget(string propName, string propType)
			: _propName(propName),
			  _propType(propType)
		{
		}
	};

	// Maps aliases to the propetery name/type that alias is tied to.
	map<string, AliasTarget> _aliasMap;

	void loadFile(string filepath)
		throw (exception);
};


#endif
