#include <xercesDomUtil.H>
#include <XmlTempString.H>
#include <parseutil.H>

#include <StorageMgr_Exceptions.H>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <sstream>
#include <ctype.h>

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// *** NOTE: ALL OF THESE HELPER FUNCTIONS ASSUME THAT XERCES IS PRESENTLY
// *** INITIALIZED VIA XMLPlatformUtils::Initialize() 
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


//===============================================================================

void writeDomDocToFile(const DOMDocument & doc, string filename)
	throw (exception)
{
	try
		{
			DOMImplementationLS * pLsImpl = getDomImplementationLs();
			DOMWriter * pWriter = pLsImpl->createDOMWriter();

			// Try to use pretty-printing...
			if (pWriter->canSetFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true))
				{
					pWriter->setFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true);
				}

			XMLFormatTarget * pTarget = new LocalFileFormatTarget(filename.c_str());
			pWriter->writeNode(pTarget, doc);

			pWriter->release();
			delete pTarget;
		}
	catch (const DOMException & e)
		{
			ostringstream os;
			os << "DOMException thrown: " << XMLString::transcode(e.msg);
			throw SmException(__FILE__, __LINE__, os.str());
		}
}

//===============================================================================

DOMDocument * readDomDocFromFile(DOMImplementationLS & impl, string filename)
	throw (exception)

{
	DOMBuilder * pBuilder = 
		impl.createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, NULL);

	if (pBuilder == NULL)
		{
			throw SmException(__FILE__, __LINE__, "Couldn't create a DOM Builder");
		}

	XmlTempString xmlFilename(filename.c_str());
	return pBuilder->parseURI(xmlFilename.toXml());
}

//===============================================================================


DOMImplementationLS * getDomImplementationLs()
	throw (exception)
{
	try
		{
			XmlTempString xstrLS("LS");

			DOMImplementation * pImpl = DOMImplementationRegistry::getDOMImplementation(xstrLS.toXml());
			if (pImpl == NULL)
				{
					throw SmException(__FILE__, __LINE__, 
									  "DOMImplementationRegistry::getDOMImplementation(\"LS\") returned NULL");
				}
      
			DOMImplementationLS * pLS = dynamic_cast<DOMImplementationLS *>(pImpl);
			if (pImpl == NULL)
				{
					throw SmException(__FILE__, __LINE__, "Class cast problem");
				}

			return pLS;
		}
	catch (const DOMException & e)
		{
			ostringstream os;
			os << "DOMException thrown: " << XMLString::transcode(e.msg);
			throw SmException(__FILE__, __LINE__, os.str());
		}
}

//===============================================================================

DOMImplementation * getDomImplementation()
  throw (exception)
{
  try
    {
      XmlTempString xstrLS("LS");
      DOMImplementation * pImpl = DOMImplementationRegistry::getDOMImplementation(xstrLS.toXml());
      return pImpl;
    }
  catch (const DOMException & e)
    {
      ostringstream os;
      os << "DOMException thrown: " << XMLString::transcode(e.msg);
      throw SmException(__FILE__, __LINE__, os.str());
    }
}

//===============================================================================

DOMElement * createDomElement(DOMDocument & docOwner, string elementName)
  throw (exception)
{
  try
    {
      XmlTempString xstrElemName(elementName.c_str());
      DOMElement * returnVal = docOwner.createElement(xstrElemName.toXml());
      return returnVal;
    }
  catch (const DOMException & e)
    {
      ostringstream os;
      os << "DOMException thrown: " << XMLString::transcode(e.msg);
      throw SmException(__FILE__, __LINE__, os.str());
    }
}

//===============================================================================

DOMElement * getDomChildByKeyTag(const DOMElement & parent, string childTagName)
  throw (exception)
{
  try
    {
      XmlTempString xstrChildTag(childTagName.c_str());

      DOMNodeList * pChildList = parent.getElementsByTagName(xstrChildTag.toXml());

      if (pChildList->getLength() != 1)
	{
	  ostringstream os;
	  os << "Couldn't find exactly one child element with the tag \"" 
	     << childTagName << "\"" << endl;
	  throw SmException(__FILE__, __LINE__, os.str());
	}

      return dynamic_cast<DOMElement *>(pChildList->item(0));
    }
  catch (const DOMException & e)
    {
      ostringstream os;
      os << "DOMException thrown: " << XMLString::transcode(e.msg);
      throw SmException(__FILE__, __LINE__, os.str());
    }
}

//===============================================================================

bool parseXmlStringAsBool(const XMLCh * pXml)
  throw (exception)
{
  if (pXml == NULL)
    {
      throw SmException(__FILE__, __LINE__, "pXml == NULL");
    }

  const char * pValue = XMLString::transcode(pXml);

  bool returnVal;
  try
	  {
		  returnVal = stringToBool(pValue);
	  }
  catch (...)
	  {
		  delete pValue;
		  throw;
	  }

  delete [] pValue;
  return returnVal;
}

//===============================================================================

int parseXmlStringAsInt(const XMLCh * pXml)
  throw (exception)
{
  if (pXml == NULL)
    {
      throw SmException(__FILE__, __LINE__, "pXml == NULL");
    }

  const char * pValue = XMLString::transcode(pXml);

  int returnVal;
  try
	  {
		  returnVal = stringToInt(pValue);
	  }
  catch (...)
	  {
		  delete pValue;
		  throw;
	  }

  delete [] pValue;
  return returnVal;

  /*

  const char  * pValue = XMLString::transcode(pXml);

  char * endPtr;
  long longVal = strtol(pValue, & endPtr, 10);

  if (*endPtr != '\0')
    {
      ostringstream os;
      os << "Tried to parse an XML string as an int, but we couldn't: \"" 
	 << pValue << "\"";
      throw SmException(__FILE__, __LINE__, os.str());
    }

  if ((longVal < numeric_limits<int>::min()) ||
      (longVal > numeric_limits<int>::max()))
    {
      ostringstream  os;
      os << "Tried to parse an XML string as an int, but it was out of range: \"" 
	 << pValue << "\"";
      throw SmException(__FILE__, __LINE__, os.str());
    }

  delete [] pValue;

  return int(longVal);
  */
}

//===============================================================================

unsigned int parseXmlStringAsUInt(const XMLCh * pXml)
  throw (exception)
{
  if (pXml == NULL)
    {
      throw SmException(__FILE__, __LINE__, "pXml == NULL");
    }

  const char  * pValue = XMLString::transcode(pXml);

  unsigned int returnVal;
  try
	  {
		  returnVal = stringToUInt(pValue);
	  }
  catch (...)
	  {
		  delete pValue;
		  throw;
	  }

  delete [] pValue;
  return returnVal;
}

//===============================================================================

unsigned long parseXmlStringAsLong(const XMLCh * pXml)
  throw (exception)
{
  if (pXml == NULL)
    {
      throw SmException(__FILE__, __LINE__, "pXml == NULL");
    }

  const char * pValue = XMLString::transcode(pXml);

  long longVal;
  try
	  {
		  longVal = stringToLong(pValue);
	  }
  catch (...)
	  {
		  delete pValue;
		  throw;
	  }

  delete [] pValue;
  return longVal;
}

//===============================================================================

unsigned long parseXmlStringAsULong(const XMLCh * pXml)
  throw (exception)
{
  if (pXml == NULL)
    {
      throw SmException(__FILE__, __LINE__, "pXml == NULL");
    }

  const char * pValue = XMLString::transcode(pXml);

  unsigned long ulongVal;
  try
	  {
		  ulongVal = stringToULong(pValue);
	  }
  catch (...)
	  {
		  delete pValue;
		  throw;
	  }

  delete [] pValue;
  return ulongVal;
}

//===============================================================================

unsigned long long parseXmlStringAsULongLong(const XMLCh * pXml)
  throw (exception)
{
  if (pXml == NULL)
    {
      throw SmException(__FILE__, __LINE__, "pXml == NULL");
    }

  const char * pValue = XMLString::transcode(pXml);

  unsigned long long ullVal;
  try
	  {
		  ullVal = stringToULongLong(pValue);
	  }
  catch (...)
	  {
		  delete pValue;
		  throw;
	  }


	/*
  size_t numDigits = strlen(pValue);
  if ((numDigits == 0) || 
      (numDigits > size_t(numeric_limits<unsigned long long>::digits10)))
    {
      ostringstream os;
      os << "Tried to parse an XML string as an unsigned long long, but we couldn't: \"" 
	 << pValue << "\" (it had an inappropriate number of digits";
      throw SmException(__FILE__, __LINE__, os.str());    
    }

  unsigned long long accumulator = 0;

  for (size_t i = 0; i < numDigits; ++i)
    {
      char c = pValue[i];

      if (! isdigit(c))
	{
	  ostringstream os;
	  os << "Tried to parse an XML string as an unsigned long long, but we couldn't: \"" 
	     << pValue << "\"";
	  throw SmException(__FILE__, __LINE__, os.str());      
	}
      
      accumulator = (accumulator * 10) + (c - '0');
    }
	*/

  delete [] pValue;
  return ullVal;
}

//===============================================================================

double parseXmlStringAsDouble(const XMLCh * pXml)
  throw (exception)
{
  if (pXml == NULL)
    {
      throw SmException(__FILE__, __LINE__, "pXml == NULL");
    }

  const char * pValue = XMLString::transcode(pXml);

  double returnVal;
  try
	  {
		  returnVal = stringToDouble(pValue);
	  }
  catch (...)
	  {
		  delete pValue;
		  throw;
	  }

  delete [] pValue;
  return returnVal;
}

//===============================================================================

void saveIntVectorToXml(const vector<int> & v, DOMDocument & doc, DOMElement & elem)
  throw (exception)
{
  for (size_t i = 0; i < v.size(); ++i)
    {
      DOMElement * pIntElem = createDomElement(doc, "int");
      setDomAttribute(* pIntElem, "value", v[i]);
      elem.appendChild(pIntElem);
    }
}

//===============================================================================

void loadIntVectorFromXml(vector<int> & v, DOMElement & elem)
  throw (exception)
{
  DOMNodeList * pChildList = elem.getChildNodes();

  XmlTempString xstrInt("int");

  for (XMLSize_t i = 0; i < (pChildList->getLength()); ++i)
    {
      DOMElement & intElem = *(dynamic_cast<DOMElement *>(pChildList->item(i)));

      if (XMLString::compareString(intElem.getTagName(), 
				   xstrInt.toXml()) != 0)
	{
	  throw SmException(__FILE__, __LINE__, "A child element wasn't tagged as 'int'");
	}

      int value;
      getDomAttribute(intElem, "value", value);
      v.push_back(value);
    }
}

//===============================================================================

void saveTimevalToXml(const timeval & tv, DOMDocument & doc, DOMElement & elem)
  throw (exception)
{
  setDomAttribute(elem, "tv_sec",  tv.tv_sec);
  setDomAttribute(elem, "tv_usec", tv.tv_usec);
}

//===============================================================================

void loadTimevalFromXml(timeval & tv, DOMElement & elem)
  throw (exception)
{
  getDomAttribute(elem, "tv_sec",  tv.tv_sec);
  getDomAttribute(elem, "tv_usec", tv.tv_usec);
}

//===============================================================================

void setDomAttribute(DOMElement & elem, string attributeName, bool newValue)
  throw (exception)
{
  try
    {
      char szBool[2];
      if (newValue)
	{
	  szBool[0] = 'T';
	}
      else
	{
	  szBool[0] = 'F';
	}
      
      szBool[1] = '\0';

      XmlTempString xstrAttrName(attributeName.c_str());
      XmlTempString xstrAttrValue(szBool);

      elem.setAttribute(xstrAttrName.toXml(), xstrAttrValue.toXml());
    }
  catch (const DOMException & e)
    {
      ostringstream os;
      os << "DOMException thrown: " << XMLString::transcode(e.msg);
      throw SmException(__FILE__, __LINE__, os.str());
    }
}

//===============================================================================

void setDomAttribute(DOMElement & elem, string attributeName, int newValue)
  throw (exception)
{
  try
    {
      char szValue[30];
      if (snprintf(szValue, sizeof(szValue), "%d", newValue) == -1)
	{
	  throw SmException(__FILE__, __LINE__, "Undersized text buffer");
	}

      XmlTempString xstrAttrName(attributeName.c_str());
      XmlTempString xstrAttrValue(szValue);

      elem.setAttribute(xstrAttrName.toXml(), xstrAttrValue.toXml());
    }
  catch (const DOMException & e)
    {
      ostringstream os;
      os << "DOMException thrown: " << XMLString::transcode(e.msg);
      throw SmException(__FILE__, __LINE__, os.str());
    }
}

//===============================================================================

void setDomAttribute(DOMElement & elem, string attributeName, unsigned int newValue)
  throw (exception)
{
  try
    {
      char szValue[30];
      if (snprintf(szValue, sizeof(szValue), "%u", newValue) == -1)
	{
	  throw SmException(__FILE__, __LINE__, "Undersized text buffer");
	}

      XmlTempString xstrAttrName(attributeName.c_str());
      XmlTempString xstrAttrValue(szValue);

      elem.setAttribute(xstrAttrName.toXml(), xstrAttrValue.toXml());
    }
  catch (const DOMException & e)
    {
      ostringstream os;
      os << "DOMException thrown: " << XMLString::transcode(e.msg);
      throw SmException(__FILE__, __LINE__, os.str());
    }
}

//===============================================================================

void setDomAttribute(DOMElement & elem, string attributeName, long newValue)
  throw (exception)
{
  try
    {
      char szValue[30];
      if (snprintf(szValue, sizeof(szValue), "%ld", newValue) == -1)
	{
	  throw SmException(__FILE__, __LINE__, "Undersized text buffer");
	}

      XmlTempString xstrAttrName(attributeName.c_str());
      XmlTempString xstrAttrValue(szValue);

      elem.setAttribute(xstrAttrName.toXml(), xstrAttrValue.toXml());
    }
  catch (const DOMException & e)
    {
      ostringstream os;
      os << "DOMException thrown: " << XMLString::transcode(e.msg);
      throw SmException(__FILE__, __LINE__, os.str());
    }
}

//===============================================================================

void setDomAttribute(DOMElement & elem, string attributeName, unsigned long newValue)
  throw (exception)
{
  try
    {
      char szValue[30];
      if (snprintf(szValue, sizeof(szValue), "%lu", newValue) == -1)
	{
	  throw SmException(__FILE__, __LINE__, "Undersized text buffer");
	}

      XmlTempString xstrAttrName(attributeName.c_str());
      XmlTempString xstrAttrValue(szValue);

      elem.setAttribute(xstrAttrName.toXml(), xstrAttrValue.toXml());
    }
  catch (const DOMException & e)
    {
      ostringstream os;
      os << "DOMException thrown: " << XMLString::transcode(e.msg);
      throw SmException(__FILE__, __LINE__, os.str());
    }
}

//===============================================================================

void setDomAttribute(DOMElement & elem, string attributeName, 
		     unsigned long long newValue)
  throw (exception)
{
  try
    {
      char szValue[numeric_limits<unsigned long long>::digits10 + 1];
      if (snprintf(szValue, sizeof(szValue), "%llu", newValue) == -1)
	{
	  throw SmException(__FILE__, __LINE__, "Undersized text buffer");
	}

      XmlTempString xstrAttrName(attributeName.c_str());
      XmlTempString xstrAttrValue(szValue);

      elem.setAttribute(xstrAttrName.toXml(), xstrAttrValue.toXml());
    }
  catch (const DOMException & e)
    {
      ostringstream os;
      os << "DOMException thrown: " << XMLString::transcode(e.msg);
      throw SmException(__FILE__, __LINE__, os.str());
    }
}

//===============================================================================

void setDomAttribute(DOMElement & elem, string attributeName, string newValue)
  throw (exception)
{
  try
    {
      XmlTempString xstrAttrName(attributeName.c_str());
      XmlTempString xstrAttrValue(newValue.c_str());

      elem.setAttribute(xstrAttrName.toXml(), xstrAttrValue.toXml());
    }
  catch (const DOMException & e)
    {
      ostringstream os;
      os << "DOMException thrown: " << XMLString::transcode(e.msg);
      throw SmException(__FILE__, __LINE__, os.str());
    }
}

//===============================================================================

void setDomAttribute(DOMElement & elem, string attributeName, double newValue)
  throw (exception)
{
  ostringstream os;
  os << newValue;

  try
    {
      XmlTempString xstrAttrName(attributeName.c_str());
      XmlTempString xstrAttrValue(os.str().c_str());

      elem.setAttribute(xstrAttrName.toXml(), xstrAttrValue.toXml());
    }
  catch (const DOMException & e)
    {
      ostringstream os;
      os << "DOMException thrown: " << XMLString::transcode(e.msg);
      throw SmException(__FILE__, __LINE__, os.str());
    }
}

//===============================================================================

void getDomAttribute(const DOMElement & elem, string attributeName, bool & value)
  throw (exception)
{
  try
    {
      XmlTempString xstrAttrName(attributeName.c_str());
      value = parseXmlStringAsBool(elem.getAttribute(xstrAttrName.toXml()));
    }
  catch (const DOMException & e)
    {
      ostringstream os;
      os << "DOMException thrown: " << XMLString::transcode(e.msg);
      throw SmException(__FILE__, __LINE__, os.str());
    }
}

//===============================================================================

void getDomAttribute(const DOMElement & elem, string attributeName, int & value)
  throw (exception)
{
  try
    {
      XmlTempString xstrAttrName(attributeName.c_str());
      value = parseXmlStringAsInt(elem.getAttribute(xstrAttrName.toXml()));
    }
  catch (const DOMException & e)
    {
      ostringstream os;
      os << "DOMException thrown: " << XMLString::transcode(e.msg);
      throw SmException(__FILE__, __LINE__, os.str());
    }
}

//===============================================================================

void getDomAttribute(const DOMElement & elem, string attributeName, unsigned int & value)
  throw (exception)
{
  try
    {
      XmlTempString xstrAttrName(attributeName.c_str());
      value = parseXmlStringAsUInt(elem.getAttribute(xstrAttrName.toXml()));
    }
  catch (const DOMException & e)
    {
      ostringstream os;
      os << "DOMException thrown: " << XMLString::transcode(e.msg);
      throw SmException(__FILE__, __LINE__, os.str());
    }
}

//===============================================================================

void getDomAttribute(const DOMElement & elem, string attributeName, long & value)
  throw (exception)
{
  try
    {
      XmlTempString xstrAttrName(attributeName.c_str());
      value = parseXmlStringAsLong(elem.getAttribute(xstrAttrName.toXml()));
    }
  catch (const DOMException & e)
    {
      ostringstream os;
      os << "DOMException thrown: " << XMLString::transcode(e.msg);
      throw SmException(__FILE__, __LINE__, os.str());
    }
}

//===============================================================================

void getDomAttribute(const DOMElement & elem, string attributeName, unsigned long & value)
  throw (exception)
{
  try
    {
      XmlTempString xstrAttrName(attributeName.c_str());
      value = parseXmlStringAsULong(elem.getAttribute(xstrAttrName.toXml()));
    }
  catch (const DOMException & e)
    {
      ostringstream os;
      os << "DOMException thrown: " << XMLString::transcode(e.msg);
      throw SmException(__FILE__, __LINE__, os.str());
    }
}

//===============================================================================

void getDomAttribute(const DOMElement & elem, string attributeName, 
		     unsigned long long & value)
  throw (exception)
{
  try
    {
      XmlTempString xstrAttrName(attributeName.c_str());
      value = parseXmlStringAsULongLong(elem.getAttribute(xstrAttrName.toXml()));
    }
  catch (const DOMException & e)
    {
      ostringstream os;
      os << "DOMException thrown: " << XMLString::transcode(e.msg);
      throw SmException(__FILE__, __LINE__, os.str());
    }
}

//===============================================================================

void getDomAttribute(const DOMElement & elem, string attributeName, string & value)
  throw (exception)
{
  try
    {
      XmlTempString xstrAttrName(attributeName.c_str());

      const XMLCh * pXml = elem.getAttribute(xstrAttrName.toXml());
      if (pXml == NULL)
	{
	  throw SmException(__FILE__, __LINE__, "pXml == NULL");
	}

      const char * pValue = XMLString::transcode(pXml);
      value = pValue;
    }
  catch (const DOMException & e)
    {
      ostringstream os;
      os << "DOMException thrown: " << XMLString::transcode(e.msg);
      throw SmException(__FILE__, __LINE__, os.str());
    }
}

//===============================================================================

void getDomAttribute(const DOMElement & elem, string attributeName, double & value)
  throw (exception)
{
  try
    {
      XmlTempString xstrAttrName(attributeName.c_str());
      value = parseXmlStringAsDouble(elem.getAttribute(xstrAttrName.toXml()));
    }
  catch (const DOMException & e)
    {
      ostringstream os;
      os << "DOMException thrown: " << XMLString::transcode(e.msg);
      throw SmException(__FILE__, __LINE__, os.str());
    }
}

//===============================================================================

bool isDomAttributePresent(const DOMElement & elem, string attributeName)
  throw (exception)
{
  try
    {
      XmlTempString xstrAttrName(attributeName.c_str());
	  return (elem.getAttributeNode(xstrAttrName.toXml()) != NULL);
    }
  catch (const DOMException & e)
    {
      ostringstream os;
      os << "DOMException thrown: " << XMLString::transcode(e.msg);
      throw SmException(__FILE__, __LINE__, os.str());
    }
}
