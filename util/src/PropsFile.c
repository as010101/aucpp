#include <PropsFile.H>
#include <util.H>
#include <StorageMgr_Exceptions.H>
#include <sstream>
#include <iostream>
#include <parseutil.H>
#include <algorithm>
#include <xercesDomUtil.H>
#include <XmlTempString.H>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>

//===============================================================================

PropsFile::PropsFile(string filepath)
	throw (exception)
{
	loadFile(filepath);
}

//===============================================================================

PropsFile::PropsFile(string filepath, string prefix, int argc, const char* argv[])
	throw (exception)
{
	loadFile(filepath);

	if (argc < 2)
		{
			return;
		}

	string::size_type prefixLen = prefix.length();

	if (prefixLen == 0)
		{
			throw SmException(__FILE__, __LINE__, "prefix is the empty string");
		}

	if ((prefix.find(':') != string::npos) ||
		(prefix.find('=') != string::npos))
		{
			throw SmException(__FILE__, __LINE__, "prefix can't contain ':' or '='");
		}

	for (int i = 1; i < argc; ++i)
		{
			string strArg(argv[i]);

			if (strArg.find(prefix) == 0)
				{
					string::size_type colonOffset = strArg.find_first_of(':');
					string::size_type equalsOffset = strArg.find_first_of('=');				

					if (equalsOffset == string::npos)
						{
							ostringstream os;
							os << "The format for the props-file override \"" << strArg 
							   << "\" doesn't look like legal.";
							
							throw SmException (__FILE__, __LINE__, os.str());
						}


					string propName;
					string propType;

					if (colonOffset == string::npos)
						{
							// It's an alias use, or there's a problem...
							string aliasName = 
								strArg.substr(prefixLen, equalsOffset - prefixLen);

							map<string, AliasTarget>::const_iterator pos = _aliasMap.find(aliasName);
							if (pos != _aliasMap.end())
								{
									propName = pos->second._propName;
									propType = pos->second._propType;
								}
							else
								{
									ostringstream os;
									os << "The format for the props-file override \"" << strArg 
									   << "\" looks like an alias use, but no such alias is "
									   << "defined in the props file.";
									
									throw SmException (__FILE__, __LINE__, os.str());
								}
						}
					else
						{
							// It's a normal property name:type, or there's a problem...
							if (equalsOffset < colonOffset)
								{
									ostringstream os;
									os << "Malformed argument \"" << strArg 
									   << "\" '=' appears before ':'";

									throw SmException(__FILE__, __LINE__, os.str());
								}

							 propType = 
								strArg.substr(colonOffset + 1, equalsOffset - colonOffset - 1);

							 propName = 
								strArg.substr(prefixLen, colonOffset - prefixLen);
						}

					string strValue = strArg.substr(equalsOffset + 1, strArg.length() - equalsOffset - 1);
					if (propType == "bool")
						{
							_boolMap[propName] = stringToBool(strValue);
						}
					else if (propType == "int")
						{
							_intMap[propName] = stringToInt(strValue);
						}
					else if (propType == "uint")
						{
							_uintMap[propName] = stringToUInt(strValue);
						}
					else if (propType == "long")
						{
							_longMap[propName] = stringToLong(strValue);
						}
					else if (propType == "ulong")
						{
							_ulongMap[propName] = stringToULong(strValue);
						}
					else if (propType == "ulonglong")
						{
							_ulongLongMap[propName] = stringToULongLong(strValue);
						}
					else if (propType == "double")
						{
							_doubleMap[propName] = stringToDouble(strValue);
						}
					else if (propType == "string")
						{
							_stringMap[propName] = strValue;
						}
					else
						{
							ostringstream os;
							os << "The property '" << propName << "' has an invalid type of " << propType;
							throw SmException(__FILE__, __LINE__, os.str());
						}
				}
		}
}

//===============================================================================

PropsFile::~PropsFile()
{
}

//===============================================================================

map<string, string> PropsFile::findPropsByPrefix(string prefix) const
	throw (exception)
{
	map<string, string> returnSet;

	for (map<string, bool>::const_iterator pos = _boolMap.begin();
		 pos != _boolMap.end(); ++pos)
		{
			if (pos->first.find(prefix) == 0)
				{
					returnSet.insert(make_pair(pos->first, "bool"));
				}
		}

	for (map<string, int>::const_iterator pos = _intMap.begin();
		 pos != _intMap.end(); ++pos)
		{
			if (pos->first.find(prefix) == 0)
				{
					returnSet.insert(make_pair(pos->first, "int"));
				}
		}

	for (map<string, unsigned int>::const_iterator pos = _uintMap.begin();
		 pos != _uintMap.end(); ++pos)
		{
			if (pos->first.find(prefix) == 0)
				{
					returnSet.insert(make_pair(pos->first, "uint"));
				}
		}

	for (map<string, long>::const_iterator pos = _longMap.begin();
		 pos != _longMap.end(); ++pos)
		{
			if (pos->first.find(prefix) == 0)
				{
					returnSet.insert(make_pair(pos->first, "long"));
				}
		}

	for (map<string, unsigned long>::const_iterator pos = _ulongMap.begin();
		 pos != _ulongMap.end(); ++pos)
		{
			if (pos->first.find(prefix) == 0)
				{
					returnSet.insert(make_pair(pos->first, "ulong"));
				}
		}

	for (map<string, unsigned long long>::const_iterator pos = _ulongLongMap.begin();
		 pos != _ulongLongMap.end(); ++pos)
		{
			if (pos->first.find(prefix) == 0)
				{
					returnSet.insert(make_pair(pos->first, "ulonglong"));
				}
		}

	for (map<string, double>::const_iterator pos = _doubleMap.begin();
		 pos != _doubleMap.end(); ++pos)
		{
			if (pos->first.find(prefix) == 0)
				{
					returnSet.insert(make_pair(pos->first, "double"));
				}
		}

	for (map<string, string>::const_iterator pos = _stringMap.begin();
		 pos != _stringMap.end(); ++pos)
		{
			if (pos->first.find(prefix) == 0)
				{
					returnSet.insert(make_pair(pos->first, "string"));
				}
		}

	return returnSet;
}

//===============================================================================

bool PropsFile::getBool(string propName) const
	throw (exception)
{
	map<string, bool>::const_iterator pos = _boolMap.find(propName);
	if (pos == _boolMap.end())
		{
			ostringstream os;
			os << "The property '" << propName << "' isn't defined with the specified type";
			throw SmException(__FILE__, __LINE__, os.str());
		}

	return pos->second;
}

//===============================================================================

bool PropsFile::isBoolPropDefined(string propName) const
	throw (exception)
{
	return (_boolMap.find(propName) != _boolMap.end());
}

//===============================================================================

bool PropsFile::getBoolWithDefault(string propName, bool defaultValue) const
{
	map<string, bool>::const_iterator pos = _boolMap.find(propName);
	if (pos == _boolMap.end())
		{
			return defaultValue;
		}
	else
		{
			return pos->second;
		}
}

//===============================================================================


int PropsFile::getInt(string propName) const
	throw (exception)
{
	map<string, int>::const_iterator pos = _intMap.find(propName);
	if (pos == _intMap.end())
		{
			ostringstream os;
			os << "The property '" << propName << "' isn't defined with the specified type";
			throw SmException(__FILE__, __LINE__, os.str());
		}

	return pos->second;
}

//===============================================================================

bool PropsFile::isIntPropDefined(string propName) const
	throw (exception)
{
	return (_intMap.find(propName) != _intMap.end());
}

//===============================================================================

int PropsFile::getIntWithDefault(string propName, int defaultValue) const
{
	map<string, int>::const_iterator pos = _intMap.find(propName);
	if (pos == _intMap.end())
		{
			return defaultValue;
		}
	else
		{
			return pos->second;
		}
}

//===============================================================================

unsigned int PropsFile::getUInt(string propName) const
	throw (exception)
{
	map<string, unsigned int>::const_iterator pos = _uintMap.find(propName);
	if (pos == _uintMap.end())
		{
			ostringstream os;
			os << "The property '" << propName << "' isn't defined with the specified type";
			throw SmException(__FILE__, __LINE__, os.str());
		}

	return pos->second;
}

//===============================================================================

bool PropsFile::isUIntPropDefined(string propName) const
	throw (exception)
{
	return (_uintMap.find(propName) != _uintMap.end());
}

//===============================================================================

unsigned int PropsFile::getUIntWithDefault(string propName, unsigned int defaultValue) const
{
	map<string, unsigned int>::const_iterator pos = _uintMap.find(propName);
	if (pos == _uintMap.end())
		{
			return defaultValue;
		}
	else
		{
			return pos->second;
		}
}

//===============================================================================

long PropsFile::getLong(string propName) const
	throw (exception)
{
	map<string, long>::const_iterator pos = _longMap.find(propName);
	if (pos == _longMap.end())
		{
			ostringstream os;
			os << "The property '" << propName << "' isn't defined with the specified type";
			throw SmException(__FILE__, __LINE__, os.str());
		}

	return pos->second;
}

//===============================================================================

bool PropsFile::isLongPropDefined(string propName) const
	throw (exception)
{
	return (_longMap.find(propName) != _longMap.end());
}

//===============================================================================

long PropsFile::getLongWithDefault(string propName, long defaultValue) const
{
	map<string, long>::const_iterator pos = _longMap.find(propName);
	if (pos == _longMap.end())
		{
			return defaultValue;
		}
	else
		{
			return pos->second;
		}
}

//===============================================================================

unsigned long PropsFile::getULong(string propName) const
	throw (exception)
{
	map<string, unsigned long>::const_iterator pos = _ulongMap.find(propName);
	if (pos == _ulongMap.end())
		{
			ostringstream os;
			os << "The property '" << propName << "' isn't defined with the specified type";
			throw SmException(__FILE__, __LINE__, os.str());
		}

	return pos->second;
}

//===============================================================================

bool PropsFile::isULongPropDefined(string propName) const
	throw (exception)
{
	return (_ulongMap.find(propName) != _ulongMap.end());
}

//===============================================================================

unsigned long PropsFile::getULongWithDefault(string propName, unsigned long defaultValue) const
{
	map<string, unsigned long>::const_iterator pos = _ulongMap.find(propName);
	if (pos == _ulongMap.end())
		{
			return defaultValue;
		}
	else
		{
			return pos->second;
		}
}

//===============================================================================

unsigned long long PropsFile::getULongLong(string propName) const
	throw (exception)
{
	map<string, unsigned long long>::const_iterator pos = _ulongLongMap.find(propName);
	if (pos == _ulongLongMap.end())
		{
			ostringstream os;
			os << "The property '" << propName << "' isn't defined with the specified type";
			throw SmException(__FILE__, __LINE__, os.str());
		}

	return pos->second;
}

//===============================================================================

bool PropsFile::isULongLongPropDefined(string propName) const
	throw (exception)
{
	return (_ulongLongMap.find(propName) != _ulongLongMap.end());
}

//===============================================================================

unsigned long long PropsFile::getULongLongWithDefault(string propName, unsigned long long defaultValue) const
{
	map<string, unsigned long long>::const_iterator pos = _ulongLongMap.find(propName);
	if (pos == _ulongLongMap.end())
		{
			return defaultValue;
		}
	else
		{
			return pos->second;
		}
}

//===============================================================================

double PropsFile::getDouble(string propName) const
	throw (exception)
{
	map<string, double>::const_iterator pos = _doubleMap.find(propName);
	if (pos == _doubleMap.end())
		{
			ostringstream os;
			os << "The property '" << propName << "' isn't defined with the specified type";
			throw SmException(__FILE__, __LINE__, os.str());
		}

	return pos->second;
}

//===============================================================================

bool PropsFile::isDoubleDefined(string propName) const
	throw (exception)
{
	return (_doubleMap.find(propName) != _doubleMap.end());
}

//===============================================================================

double PropsFile::getDoubleWithDefault(string propName, double defaultValue) const
{
	map<string, double>::const_iterator pos = _doubleMap.find(propName);
	if (pos == _doubleMap.end())
		{
			return defaultValue;
		}
	else
		{
			return pos->second;
		}
}

//===============================================================================

string PropsFile::getString(string propName) const
	throw (exception)
{
	map<string, string>::const_iterator pos = _stringMap.find(propName);
	if (pos == _stringMap.end())
		{
			ostringstream os;
			os << "The property '" << propName << "' isn't defined with the specified type";
			throw SmException(__FILE__, __LINE__, os.str());
		}

	return pos->second;
}

//===============================================================================

bool PropsFile::isStringPropDefined(string propName) const
	throw (exception)
{
	return (_stringMap.find(propName) != _stringMap.end());
}

//===============================================================================

string PropsFile::getStringWithDefault(string propName, string defaultValue) const
{
	map<string, string>::const_iterator pos = _stringMap.find(propName);
	if (pos == _stringMap.end())
		{
			return defaultValue;
		}
	else
		{
			return pos->second;
		}
}

//===============================================================================

void PropsFile::loadFile(string filepath)
	throw (exception)
{
	if (! fileExists(filepath))
		{
			ostringstream os;
			os << "The specified props file: '" << filepath << "' doesn't exist";
			throw SmException(__FILE__, __LINE__, os.str());
		}

	if (dirExists(filepath))
		{
			ostringstream os;
			os << "The specified props file: '" << filepath << "' is a directory.";
			throw SmException(__FILE__, __LINE__, os.str());
		}

	//cout << "INIT" << endl;
  
	XMLPlatformUtils::Initialize();

	DOMDocument * pDoc = readDomDocFromFile(*getDomImplementationLs(), filepath);
	DOMElement * pElement = pDoc->getDocumentElement();
	DOMNodeList * pChildList = pElement->getChildNodes();

	for (XMLSize_t i = 0; i < (pChildList->getLength()); ++i)
		{
			DOMElement * pPropElem = (dynamic_cast<DOMElement *>(pChildList->item(i)));
			if (pPropElem != NULL)
				{
					DOMElement & propElem = *pPropElem;

					string propName(XMLString::transcode(propElem.getTagName()));

					string propType;
					getDomAttribute(propElem, "type", propType);

					if (propType == "bool")
						{
							bool propValue;
							getDomAttribute(propElem, "value", propValue);
							if (! _boolMap.insert(make_pair(propName, propValue)).second)
								{
									ostringstream os;
									os << "The property '" << propName << "' is already defined";
									throw SmException(__FILE__, __LINE__, os.str());
								}
						}
					else if (propType == "int")
						{
							int propValue;
							getDomAttribute(propElem, "value", propValue);

							if (! _intMap.insert(make_pair(propName, propValue)).second)
								{
									ostringstream os;
									os << "The property '" << propName << "' is already defined";
									throw SmException(__FILE__, __LINE__, os.str());
								}
						}
					else if (propType == "uint")
						{
							unsigned int propValue;
							getDomAttribute(propElem, "value", propValue);

							if (! _uintMap.insert(make_pair(propName, propValue)).second)
								{
									ostringstream os;
									os << "The property '" << propName << "' is already defined";
									throw SmException(__FILE__, __LINE__, os.str());
								}
						}
					else if (propType == "long")
						{
							long propValue;
							getDomAttribute(propElem, "value", propValue);

							if (! _longMap.insert(make_pair(propName, propValue)).second)
								{
									ostringstream os;
									os << "The property '" << propName << "' is already defined";
									throw SmException(__FILE__, __LINE__, os.str());
								}
						}
					else if (propType == "ulong")
						{
							unsigned long propValue;
							getDomAttribute(propElem, "value", propValue);

							if (! _ulongMap.insert(make_pair(propName, propValue)).second)
								{
									ostringstream os;
									os << "The property '" << propName << "' is already defined";
									throw SmException(__FILE__, __LINE__, os.str());
								}
						}
					else if (propType == "ulonglong")
						{
							unsigned long long propValue;
							getDomAttribute(propElem, "value", propValue);

							if (! _ulongLongMap.insert(make_pair(propName, propValue)).second)
								{
									ostringstream os;
									os << "The property '" << propName << "' is already defined";
									throw SmException(__FILE__, __LINE__, os.str());
								}
						}
					else if (propType == "double")
						{
							double propValue;
							getDomAttribute(propElem, "value", propValue);

							if (! _doubleMap.insert(make_pair(propName, propValue)).second)
								{
									ostringstream os;
									os << "The property '" << propName << "' is already defined";
									throw SmException(__FILE__, __LINE__, os.str());
								}
						}
					else if (propType == "string")
						{
							string propValue;
							getDomAttribute(propElem, "value", propValue);

							if (! _stringMap.insert(make_pair(propName, propValue)).second)
								{
									ostringstream os;
									os << "The property '" << propName << "' is already defined";
									throw SmException(__FILE__, __LINE__, os.str());
								}
						}
					else
						{
							ostringstream os;
							os << "The property '" << propName << "' has an invalid 'type' of " << propType;
							throw SmException(__FILE__, __LINE__, os.str());
						}

					if (isDomAttributePresent(propElem, "alias"))
						{
							string aliasName;
							getDomAttribute(propElem, "alias", aliasName);

							if (aliasName.find_first_of(':') != string::npos)
								{
									ostringstream os;
									os << "The alias name " << aliasName
									   << "contains a ':' character, which is forbidden";

									throw SmException(__FILE__, __LINE__, os.str());
								}


							if (aliasName.find_first_of('=') != string::npos)
								{
									ostringstream os;
									os << "The alias name " << aliasName
									   << "contains a ':' character, which is forbidden";

									throw SmException(__FILE__, __LINE__, os.str());
								}

							if (_aliasMap.find(aliasName) != _aliasMap.end())
								{
									ostringstream os;
									os << "The alias name " << aliasName << " is "
									   << "defined on at least two properties: \""
									   << propName << "\" and \"" 
									   << _aliasMap.find(aliasName)->second._propName << "\"";

									throw SmException(__FILE__, __LINE__, os.str());
								}

							_aliasMap.insert(make_pair(aliasName, 
													   AliasTarget(propName, propType)));
						}
				}

		}

	pDoc->release();

	XMLPlatformUtils::Terminate();
}

//===============================================================================
