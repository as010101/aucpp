#ifndef XERCESDOMUTIL_H
#define XERCESDOMUTIL_H

#include <exception>
#include <string>
#include <vector>
#include <xercesc/util/XMLString.hpp>
#include <sys/time.h>

using namespace std;

class DOMElement;
class DOMDocument;
class DOMImplementation;
class DOMImplementationLS;


//===============================================================================

// Creates an XML file with the specified name, whose contents is the specified
// subtree.
void writeDomDocToFile(const DOMDocument & doc, string filename)
  throw (exception);

// Creates a DOMDocument from the specified XML file using the provided
// implementation object.
DOMDocument * readDomDocFromFile(DOMImplementationLS & impl, string filename)
  throw (exception);

// Uses the DOMImplementationRegistry. Will throw an exception rather than 
// returning NULL.
DOMImplementationLS * getDomImplementationLs()
  throw (exception);

// Uses the DOMImplementationRegistry
DOMImplementation * getDomImplementation()
  throw (exception);

DOMElement * createDomElement(DOMDocument & docOwner, string elementName)
  throw (exception);

// Returns a pointer to the one and only child element with the the specified
// tag. Throws an exception if there wasn't exactly one such child.
DOMElement * getDomChildByKeyTag(const DOMElement & parent, string childTagName)
  throw (exception);

//===============================================================================

// Tries to parse the specified string as a bool ('T' or 'F'). Throws an 
// exception if it can't.
bool parseXmlStringAsBool(const XMLCh * pXml)
  throw (exception);

// Parse the XML string as the specified kind of number. Throw an exception if it
// can't be done...

int parseXmlStringAsInt(const XMLCh * pXml)
  throw (exception);

unsigned int parseXmlStringAsUInt(const XMLCh * pXml)
  throw (exception);

unsigned long parseXmlStringAsLong(const XMLCh * pXml)
  throw (exception);

unsigned long parseXmlStringAsULong(const XMLCh * pXml)
  throw (exception);

// Note: This won't necessarily discover overflow, because I had to implement 
// this on my own and didn't want to take the time to make a perfect bad-parse
// detector. -cjc
unsigned long long parseXmlStringAsULongLong(const XMLCh * pXml)
  throw (exception);

double parseXmlStringAsDouble(const XMLCh * pXml)
  throw (exception);

//===============================================================================

// Creates an XML subtree, rooted at 'elem', representing the content of the 
// vector.
void saveIntVectorToXml(const vector<int> & v, DOMDocument & doc, DOMElement & elem)
  throw (exception);

// Given an element populated by saveIntVectorToXml, this will append 'v' with
// the saved vector's elements (in proper order).
void loadIntVectorFromXml(vector<int> & v, DOMElement & elem)
  throw (exception);

//===============================================================================

// Creates an XML subtree, rooted at 'elem', representing the content of the 
// timeval.
void saveTimevalToXml(const timeval & tv, DOMDocument & doc, DOMElement & elem)
  throw (exception);

// Given an element populated by saveTimevalToXml, this will populate 'tv' with
// the saved timeval's fields.
void loadTimevalFromXml(timeval & tv, DOMElement & elem)
  throw (exception);

//===============================================================================

// Numeric type promotion + normal overloading keeps us out of trouble here, so
// we can avoid user-specified templates. Thus, the proper version of this 
// function will always be called for parameters like size_t and ssize_t.

void setDomAttribute(DOMElement & elem, string attributeName, bool newValue)
  throw (exception);

void setDomAttribute(DOMElement & elem, string attributeName, int newValue)
  throw (exception);

void setDomAttribute(DOMElement & elem, string attributeName, unsigned int newValue)
  throw (exception);

void setDomAttribute(DOMElement & elem, string attributeName, long newValue)
  throw (exception);

void setDomAttribute(DOMElement & elem, string attributeName, unsigned long newValue)
  throw (exception);

void setDomAttribute(DOMElement & elem, string attributeName, unsigned long long newValue)
  throw (exception);

// NOTE! This might produce a messed up XML file if 'newValue' has characters 
// that are XML control characters. If/when we need to handle this, we can must 
// an encoding scheme in this function and the corresponding 'getDomAttribute' 
// function.
void setDomAttribute(DOMElement & elem, string attributeName, string newValue)
  throw (exception);

void setDomAttribute(DOMElement & elem, string attributeName, double newValue)
  throw (exception);



void getDomAttribute(const DOMElement & elem, string attributeName, bool & value)
  throw (exception);

void getDomAttribute(const DOMElement & elem, string attributeName, int & value)
  throw (exception);

void getDomAttribute(const DOMElement & elem, string attributeName, unsigned int & value)
  throw (exception);

void getDomAttribute(const DOMElement & elem, string attributeName, long & value)
  throw (exception);

void getDomAttribute(const DOMElement & elem, string attributeName, unsigned long & value)
  throw (exception);

void getDomAttribute(const DOMElement & elem, string attributeName, unsigned long long & value)
  throw (exception);

void getDomAttribute(const DOMElement & elem, string attributeName, string & newValue)
  throw (exception);

void getDomAttribute(const DOMElement & elem, string attributeName, double & newValue)
  throw (exception);


// Returns true if the attribute with the specified name is defined in the 
// element, otherwise returns false.
bool isDomAttributePresent(const DOMElement & elem, string attributeName)
  throw (exception);

#endif
