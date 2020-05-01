#include <XmlTempString.H>

XmlTempString::XmlTempString(const char * pszString)
  throw (exception)
{
  _pXmlString = XMLString::transcode(pszString);
}

XmlTempString::~XmlTempString()
{
  delete[] _pXmlString;
}


const XMLCh * XmlTempString::toXml() const
{
  return _pXmlString;
}
