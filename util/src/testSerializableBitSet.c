#include <SerializableBitSet.H>
#include <util.H>
#include <iostream>
#include <vector>
#include <stdlib.h>

void printUsage()
{
  cerr << "Usage: testSerializableBitSet <filename> <num-bits>" << endl;
}

//==============================================================================

bool test1(string filename, int numBits)
{
      vector<bool> testDataVector;
      testDataVector.reserve(numBits);

      SerializableBitSet testDataBitSet(numBits);


      for (int i = 0; i < numBits; i++)
	{
	  if ((random() % 2) == 1)
	    {
	      testDataVector[i] = (true);
	      testDataBitSet.setBit(i, true);
	    }
	  else
	    {
	      testDataVector[i] = (false);
	      testDataBitSet.setBit(i, false);
	    }
	}

      int fd = openFileWithFlagsAndPerms(filename, O_RDWR | O_CREAT, 0777);
      testDataBitSet.save(fd);

      SerializableBitSet loadedBitSet(numBits);
      setFileOffset(fd, 0, SEEK_SET);
      loadedBitSet.load(fd);
      closeFile(fd);

      return (testDataBitSet == loadedBitSet);
}

//==============================================================================

bool test2(int numBits)
{
  SerializableBitSet s(100);

  s.setBit(0, true);
  if (s.getFirstIdxForValue(true) != 0)
    {
      cout << "Failure: s.getFirstIdxForValue(true) != 0" << endl;
      return false;
    }
  if (s.getFirstIdxForValue(false) != 1)
    {
      cout << "Failure: s.getFirstIdxForValue(false) != 1" << endl;
      return false;
    }

  s.setBit(0, false);
  s.setBit(10, true);
  if (s.getFirstIdxForValue(true) != 10)
    {
      cout << "Failure: s.getFirstIdxForValue(true) != 10" << endl;
      return false;
    }
  if (s.getFirstIdxForValue(false) != 0)
    {
      cout << "Failure: s.getFirstIdxForValue(false) != 0" << endl;
      return false;
    }

  return true;
}

//==============================================================================

bool test3(int numBits)
{
  SerializableBitSet testDataBitSet(numBits);

  for (int i = 0; i < numBits; i++)
    {
      int idx = testDataBitSet.getFirstIdxForValue(false);
      if (idx != i)
	{
	  cout << "Failed: idx != i" << endl
	       << "\tidx = " << idx << endl
	       << "\ti = " << i << endl;
	  return false;
	}

      testDataBitSet.setBit(i, true);
    }

  return true;
}

//==============================================================================

bool test4(int numBits)
{
  SerializableBitSet testDataBitSet(numBits);

  int numTrues = 0;
  int numFalses = 0;

  for (int i = 0; i < numBits; i++)
    {
      if ((random() % 2) == 0)
	{
	  testDataBitSet.setBit(i, true);
	  numTrues++;

	  //  cout << "\tJust set bit " << i << " to true." << endl;
	}
      else
	{
	  testDataBitSet.setBit(i, false);
	  numFalses++;


	  //  cout << "\tJust set bit " << i << " to false." << endl;
	}

      /*
	cout << "\t\ttestDataBitSet.getBitCountForValue(true) == " 
	<< testDataBitSet.getBitCountForValue(true) << endl
	<< "\t\ttestDataBitSet.getBitCountForValue(false) == " 
	<< testDataBitSet.getBitCountForValue(false) << endl;
      */
    }

  cout << "Test4: numTrues = " << numTrues << ", numFalses = " << numFalses << endl;

  if (testDataBitSet.getBitCountForValue(true) != numTrues)
    {
      cout << "Failed: testDataBitSet.getBitCountForValue(true) == " 
	   << testDataBitSet.getBitCountForValue(true) << endl;
      return false;
    }


  if (testDataBitSet.getBitCountForValue(false) != numFalses)
    {
      cout << "Failed: testDataBitSet.getBitCountForValue(false) == " 
	   << testDataBitSet.getBitCountForValue(false) << endl;
      return false;
    }


  return true;
}

//==============================================================================

int main(int argc, char **argv)
{
  try
    {
      if (argc != 3)
	{
	  printUsage();
	  return 1;
	}
      
      string filename(argv[1]);
      
      long numBits = atol(argv[2]);
      if (numBits < 1)
	{
	  cerr << "numBits was < 1" << endl;
	  return 1;
	}
      
      // Do the real work...
      if (test1(filename, numBits))
	{
	  cout << "Test 1 passed." << endl;
	}
      else
	{
	  cout << "Test 1 failed." << endl;
	}

      if (test2(numBits))
	{
	  cout << "Test 2 passed." << endl;
	}
      else
	{
	  cout << "Test 2 failed." << endl;
	}

      if (test3(numBits))
	{
	  cout << "Test 3 passed." << endl;
	}
      else
	{
	  cout << "Test 3 failed." << endl;
	}

      if (test4(numBits))
	{
	  cout << "Test 4 passed." << endl;
	}
      else
	{
	  cout << "Test 4 failed." << endl;
	}
    }
  catch (std::exception &e)
    {
      cerr << "Exception thrown:" << endl << e.what() << endl;
      return 1;
    }

  return 0;
}

