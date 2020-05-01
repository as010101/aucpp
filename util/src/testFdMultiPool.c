#include <iostream>
#include <sys/stat.h>
#include <FdMultiPool.H>

void printUsage()
{
  cerr << "Usage: testFdMultiPool <filename1> <filename2> <filename3> " << endl;
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
      
      string fname1(argv[1]);
      string fname2(argv[2]);
      string fname3(argv[3]);

      FdMultiPool fmp(10);
      fmp.registerFile(fname1, O_CREAT | O_RDWR, S_IRWXU);
      fmp.registerFile(fname2, O_CREAT | O_RDWR, S_IRWXU);
      fmp.registerFile(fname3, O_CREAT | O_RDWR, S_IRWXU);

      // TEST 1: Verify that we can have at most 10 fds registered...
      cout << "************ TEST 1 ***********" << endl;
      fmp.printDebugInfo(cout);

      int leasedFds[10];
      for (int i = 0; i < 10; i++)
	{
	  leasedFds[i] = fmp.acquireFdNonBlocking(fname1);
	  cout << "acquired fd: " << leasedFds[i] << endl;
	}
      cout << "Test 1: Success: Can acquire 10 FDs" << endl;
      cout << endl;

      // TEST 2: Ensure we can't exceed our limit.
      cout << "************ TEST 2 ***********" << endl;
      fmp.printDebugInfo(cout);

      try
	{
	  int fd = fmp.acquireFdNonBlocking(fname1);
	  cout << "Test 2: Failed: Could acquire an 11th FD: " << fd << endl;
	}
      catch (SmException & e)
	{
	  cout << "Test 2: Success: Couldn't acquire an 11th FD" << endl
	       << "\t" << e.what() << endl;
	}
      cout << endl;

      // TEST 3: Ensure we can re-purpose existing FDs...
      cout << "************ TEST 3 ***********" << endl;
      fmp.printDebugInfo(cout);

      fmp.releaseFd(fname1, leasedFds[9]);
      fmp.printDebugInfo(cout);

      fmp.releaseFd(fname1, leasedFds[8]);
      fmp.printDebugInfo(cout);

      leasedFds[8] = fmp.acquireFdNonBlocking(fname2);
      fmp.printDebugInfo(cout);

      leasedFds[9] = fmp.acquireFdNonBlocking(fname3);
      fmp.printDebugInfo(cout);

      cout << "Test 3: Success: Could acquire two replacement FDs." << endl;
      cout << "\tacquired fd: " << leasedFds[8] << endl;
      cout << "\tacquired fd: " << leasedFds[9] << endl;
      cout << endl;

      fmp.printDebugInfo(cout);
      cout << endl;
      
      cout << "*********** TEST 4 *********" << endl;
      try
	{
	  fmp.close();
	  cout << "Test 4: Failure: close() succeeded, even with outstanding FD leases." << endl;
	}
      catch (std::exception & e)
	{
	  cout << "Test 4: Success: close() failed when there are outstanding leases." << endl
	       << "\t" << e.what() << endl;
	}

      cout << "*********** TEST 5 **************" << endl;
      for (int i = 0; i < 8; i++)
	{
	  fmp.releaseFd(fname1, leasedFds[i]);
	}
      fmp.releaseFd(fname2, leasedFds[8]);
      fmp.releaseFd(fname3, leasedFds[9]);

      fmp.close();
      cout << "Test 5: Success: Closed the FMP" << endl;
      fmp.printDebugInfo(cout);

      cout << "************ Test complete ************" << endl;
    }
  catch (std::exception &e)
    {
      cerr << "Exception thrown:" << endl << e.what() << endl;
      return 1;
    }
  catch (...)
    {
      cout << "Exception thrown, and it isn't a std::exception." << endl;
      return 1;
    }

  return 0;
}
