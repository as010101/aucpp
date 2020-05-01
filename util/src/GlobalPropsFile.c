#include <GlobalPropsFile.H>
#include <util.H>
#include <StorageMgr_Exceptions.H>
#include <sstream>

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
// Static initialization....
//===============================================================================

PropsFile * GlobalPropsFile::s_pInstance = NULL;

//===============================================================================
// Method implementations...
//===============================================================================

void GlobalPropsFile::makeInstance(string filepath)
  throw (exception)
{
  if (s_pInstance != NULL)
    {
      throw SmException(__FILE__, __LINE__, "An instance already exists");
    }

  s_pInstance = new PropsFile(filepath);
}

//===============================================================================

void GlobalPropsFile::destroyInstance()
  throw (exception)
{
  if (s_pInstance == NULL)
    {
      throw SmException(__FILE__, __LINE__, "An instance doesn't currently exist");
    }

  delete s_pInstance;
  s_pInstance = NULL;
}

//===============================================================================

const PropsFile * GlobalPropsFile::getInstance()
  throw (exception)
{
  if (s_pInstance == NULL)
    {
      throw SmException(__FILE__, __LINE__, "An instance doesn't currently exist");
    }

  return s_pInstance;
}

//===============================================================================

GlobalPropsFile::GlobalPropsFile()
{
}

//===============================================================================
GlobalPropsFile::~GlobalPropsFile()
{
}

//===============================================================================
