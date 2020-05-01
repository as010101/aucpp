#include <FileDescPool.H>
#include <util.H>
#include <iostream>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>


//===============================================================================

void printUsage()
{
  cout << "Usage: testFileDescPool <filename> <min-handles> <max-handles>" << endl;
}

//===============================================================================

int main(int argc, char** argv)
{
  if (argc != 4)
    {
      printUsage();
      exit(1);
    }

  string filename = argv[1];
  int minHandles = atoi(argv[2]);
  int maxHandles = atoi(argv[3]);

  try
    {
      FileDescPool aPool(filename, 
			 O_CREAT | O_RDWR, 
			 S_IRWXU | S_IRWXG | S_IRWXO, 
			 minHandles, 
			 maxHandles);

      int *handles = new int[maxHandles];
      for (int i = 0; i < maxHandles; i++)
	{
	  handles[i] = aPool.acquireFdBlocking();
	  cout << "FD allocated: " << handles[i] << endl;
	}
     
      cout << endl;
      cout << "OK: Succesfully acquired maxHandles FDs" << endl;


      int badFd = aPool.acquireFdNonBlocking();
      if (badFd == -1)
	{
	  cout << "OK: Verified that no more than maxHandles FDs could be allocated." << endl;
	}
      else
	{
	  cout << "FAIL: More than maxHandles FDs could be allocated." << endl;
	}


      // Make sure all of the FDs work...
      for (int i = 0; i < maxHandles; i++)
	{
	  int fd = handles[i];
	  const char * pData = "ABCD";
	  const char * pChar = pData + (i % 4);

	  writeData(fd, i, pChar, 1);
	}


      try
	{
	  aPool.releaseFd(-2);
	  cout << "FAIL: Successfully released FD -2" << endl;
	}
      catch (SmException & e)
	{
	  cout << "OK: Wasn't able to released FD -2" << endl;
	}

      for (int i = 0; i < maxHandles; i++)
	{
	  aPool.releaseFd(handles[i]);
	}

      cout << "OK: Succesfully released all of the FDs" << endl;


      aPool.setFDLimits(maxHandles + 10, maxHandles + 20);
      unsigned int numHandles = aPool.getTotalFDsInPool();
      unsigned int m = aPool.getMinFDs();
      if (m == numHandles)
	{
	  cout << "OK: Bumping the min FD count way up caused more FDs to be created." << endl;
	}
      else
	{
	  cout << "FAIL: Bumping the min FD count way up DIDN'T cause more FDs to be created." << endl;
	}

      aPool.setFDLimits(0, 0);
      numHandles = aPool.getTotalFDsInPool();
      if (numHandles == 0)
	{
	  cout << "OK: Zeroing-out the max FD count released all the FDs in the pool." << endl;
	}
      else
	{
	  cout << "FAIL: Zeroing-out the max FD count DIDN'T release all the FDs in the pool." << endl;
	  cout << "\tnumHandles = " << numHandles << endl;
	}

      aPool.closeAll();
      cout << "OK: Succesfully called the closeAll() method." << endl;
    }
  catch (std::exception &e)
    {
      cerr << "Exception thrown:" << endl << e.what() << endl;
      exit(1);
    }
}

//===============================================================================
