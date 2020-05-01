#include <util.H>
#include <iostream>

void printUsage()
{
  cerr << "Usage: testUtil <dirpath> <dir-search-prefix>" << endl;
}

//===================================================================================

int main(int argc, char **argv)
{
  try
    {
      if (argc != 3)
	{
	  printUsage();
	  return 1;
	}
      
      string dirpath(argv[1]);
      string dirSearchPrefix(argv[2]);

      cout << "********* TEST 1: List a directory's contents *************" << endl;
      cout << "All files in directory \"" << dirpath << "\" with a prefix of \"" 
	   << dirSearchPrefix << "\":" << endl;
      vector<string> filenames;
      listFilesWithPrefix(dirpath, dirSearchPrefix, filenames);
      
      for (unsigned int i = 0; i < filenames.size(); i++)
	{
	  cout << "\t" << filenames[i] << endl;
	}

      cout << endl << "Success" << endl << endl;

      cout << "******** TEST 2: Verify getAvgTimevalBySums(...)" << endl;
      timeval tv1 = makeTimeval(0, 999999);
      timeval tv2 = makeTimeval(1, 999999);

      timeval tvSum = getAvgTimevalBySums(tv1.tv_sec + tv2.tv_sec,
					  tv1.tv_usec + tv2.tv_usec,
					  2);

      if ((tvSum.tv_sec == 1) && (tvSum.tv_usec == 499999))
	{
	  cout << endl << "Success" << endl << endl;
	}
      else
	{
	  cout << endl << "Failure: tvSum.tv_sec = " << tvSum.tv_sec 
	       << ", tvSum.tv_usec = " << tvSum.tv_usec << endl;
	}

      cout << "******** TEST 3: Verify getTimevalDiff(...)" << endl;
      timeval t1 = makeTimeval(2, 2);
      timeval t2 = makeTimeval(3, 3);

      timeval td = getTimevalDiff(t1, t2);
      if ((td.tv_sec != 1) || (td.tv_usec != 1))
	{
	  cout << endl << "Failure: td.tv_sec = " << td.tv_sec 
	       << ", td.tv_usec = " << td.tv_usec << endl;
	}
      
      t1 = makeTimeval(1, 23);
      t2 = makeTimeval(2, 20);
      td = getTimevalDiff(t1, t2);

      if ((td.tv_sec != 0) || (td.tv_usec != 999997))
	{
	  cout << endl << "Failure: td.tv_sec = " << td.tv_sec 
	       << ", td.tv_usec = " << td.tv_usec << endl;
	}

      cout << endl << "Success" << endl << endl;
    }
  catch (std::exception &e)
    {
      cerr << "Exception thrown:" << endl << e.what() << endl;
      return 1;
    }

  return 0;
}
