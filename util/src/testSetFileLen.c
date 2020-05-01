#include <util.H>
#include <iostream>

void printUsage()
{
  cerr << "Usage: testSetFileLen <filename> <num-bytes> {force|noforce}" << endl;
}

//===================================================================================

int main(int argc, char **argv)
{
  try
    {
      if (argc != 4)
	{
	  printUsage();
	  return 1;
	}
      
      string fname(argv[1]);
      
      long fileLen = atol(argv[2]);
      if (fileLen < 0)
	{
	  cerr << "filelength was < 0" << endl;
	  return 1;
	}
      
      bool forceFill;
      string forceFillStr = argv[3];
      
      if (forceFillStr.compare("force") == 0)
	{
	  forceFill = true;
	}
      else if (forceFillStr.compare("noforce") == 0)
	{
	  forceFill = false;
	}
      else
	{
	  cerr << "3rd parameter was niether 'force' nor 'noforce'" << endl;
	  return 1;
	}
      
      // Do the real work...
      int fd = openFileWithFlagsAndPerms(fname, O_RDWR | O_CREAT, 0777);
      
      cout << "Old file length: " << getFileLen(fd) << endl;

      try
	{
	  setFileLen(fd, off_t(fileLen), forceFill);
	}
      catch (SmException & e)
	{
	  cerr << "Exception thrown during call to setFileLen(...):" << endl
	       << "   Location   : " << e.getLocation() << endl
	       << "   Line num   : " << e.getLineNum() << endl
	       << "   Description: " << e.getDescription() << endl;
	}

      cout << "New file length: " << getFileLen(fd) << endl;

      closeFile(fd);
    }
  catch (std::exception &e)
    {
      cerr << "Exception thrown:" << endl << e.what() << endl;
      return 1;
    }

  return 0;
}
