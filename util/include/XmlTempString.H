#ifndef XMLTEMPSTRING_H
#define XMLTEMPSTRING_H

#include <exception>
#include <assert.h>
#include <xercesc/util/XMLString.hpp>

using namespace std;

// So long as this class remains instantiated, the XMLChr * it provides is valid.
class XmlTempString
{
public:
  XmlTempString(const char * pszString)
    throw (exception);

  virtual ~XmlTempString();

  // This object retains ownership of the string whose pointer is retunrned.
  const XMLCh * toXml() const;

private:
  XmlTempString() {assert(false);}
  XmlTempString(const XmlTempString &) {assert(false);}
  XmlTempString & operator= (const XmlTempString &) {assert(false); return *this;}

  XMLCh * _pXmlString;
};

#endif
