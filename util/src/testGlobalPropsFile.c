#include <GlobalPropsFile.H>
#include <util.H>
#include <unistd.h>
#include <string.h>
#include <exception>
#include <sstream>
#include <iostream>

using namespace std;

//===============================================================================

void printUsage()
{
  cout << "Usage: testGlobalPropsFile <scratch-filename>" << endl;
}

//===============================================================================

// Verifies that a instantiation and parsing work...
void test1(string filename)
  throw (exception)
{
  cout << "************************************************************" << endl
       << "                       Entering test 1                      " << endl
       << "************************************************************" << endl
       << endl << endl;

  // Write the file...
  int fd = openFileWithFlags(filename, O_CREAT | O_TRUNC | O_RDWR);

  vector<string> lines;
  lines.push_back("<props>");
  lines.push_back("<a type=\"bool\"      value=\"T\" />\n");
  lines.push_back("<b type=\"int\"       value=\"-1\" />\n");
  lines.push_back("<c type=\"uint\"      value=\"2\" />\n");
  lines.push_back("<d type=\"long\"      value=\"-3\" />\n");
  lines.push_back("<e type=\"ulong\"     value=\"4\" />\n");
  lines.push_back("<f type=\"ulonglong\" value=\"5\" />\n");
  lines.push_back("<g type=\"string\"    value=\"Hello, XML!\" />\n");
  lines.push_back("</props>");

  for (size_t i = 0; i < lines.size(); ++i)
    {
      const char * pszData = lines.at(i).c_str();
      size_t len = strlen(pszData);

      int rc = write(fd, pszData, len);
      if (rc == -1)
	{
	  ostringstream os;
	  os << "Problem writing to scratch file: " << strerror(errno);
	  throw SmException(__FILE__, __LINE__, os.str());
	}
    }

  close(fd);

  // Parse the file...
  GlobalPropsFile::makeInstance(filename);
  const PropsFile & f = *(GlobalPropsFile::getInstance());

  {
    bool value = f.getBool("a");
    if (value != true)
      {
	ostringstream os;
	os << "Test failed. For property 'a', expected 'true' but got '" << value << "'";
	throw SmException(__FILE__, __LINE__, os.str());
      }
  }

  {
    int value = f.getInt("b");
    if (value != -1)
      {
	ostringstream os;
	os << "Test failed. For property 'b', expected '-1' but got '" << value << "'";
	throw SmException(__FILE__, __LINE__, os.str());
      }
  }

  {
    unsigned int value = f.getUInt("c");
    if (value != 2)
      {
	ostringstream os;
	os << "Test failed. For property 'c', expected '2' but got '" << value << "'";
	throw SmException(__FILE__, __LINE__, os.str());
      }
  }

  {
    long value = f.getLong("d");
    if (value != -3)
      {
	ostringstream os;
	os << "Test failed. For property 'd', expected '-3' but got '" << value << "'";
	throw SmException(__FILE__, __LINE__, os.str());
      }
  }

  {
    unsigned long value = f.getULong("e");
    if (value != 4)
      {
	ostringstream os;
	os << "Test failed. For property 'e', expected '4' but got '" << value << "'";
	throw SmException(__FILE__, __LINE__, os.str());
      }
  }

  {
    unsigned long value = f.getULongLong("f");
    if (value != 5)
      {
	ostringstream os;
	os << "Test failed. For property 'f', expected '5' but got '" << value << "'";
	throw SmException(__FILE__, __LINE__, os.str());
      }
  }

  {
    string value = f.getString("g");
    if (value != "Hello, XML!")
      {
	ostringstream os;
	os << "Test failed. For property 'g', expected \"Hello, XML!\" but got \"" << value << "\"";
	throw SmException(__FILE__, __LINE__, os.str());
      }
  }

  // Cleanup...
  GlobalPropsFile::destroyInstance();

  cout << "************************************************************" << endl
       << "                        Test 1 passed                       " << endl
       << "************************************************************" << endl
       << endl << endl;
}

//===============================================================================

int main(int argc, char * argv[])
{
  if (argc != 2)
    {
      printUsage();
      return 1;
    }

  string filename(argv[1]);

  try
    {
      test1(filename);
    }
  catch (const exception & e)
    {
      cout << "main(...): Caught exception: " << e.what() << endl;
      return 1;
    }
}

//===============================================================================
