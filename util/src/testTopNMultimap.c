#include <iostream>
#include <string>
#include <algorithm>
#include <TopNMultimap.H>

//===============================================================================

template<typename TKey, typename TValue>
void printMmState(const multimap<TKey, TValue> & mm, string mmName)
  throw (std::exception)
{
  cout << "Multimap \"" << mmName << "\" has " << mm.size() << " elements:" 
       << endl;

  typename multimap<TKey, TValue>::const_iterator p = mm.begin();
  while (p != mm.end())
    {
      cout << "   {" << (p->first) << ", " << (p->second) << "}" << endl;
      p++;
    }
}

//===============================================================================

void verifyFindByKey(const TopNMultimap<string, int> & tnmm,
		     string aKey, 
		     int expectedValue,
		     bool expectedInTop,
		     bool expectedInBottom)
  throw (std::exception)
{
  int foundValue;
  bool inTop;

  bool found = tnmm.findByKey(aKey, foundValue, inTop);
  if ((! found) && (expectedInTop || expectedInBottom))
    {
      throw SmException(__FILE__, __LINE__, "The item wasn't found at all");
      
    }

  if (expectedValue != foundValue)
    {
      throw SmException(__FILE__, __LINE__, "The item was found, but had an unexpected value");
    }

  if (expectedInTop && (! inTop))
    {
      throw SmException(__FILE__, __LINE__, "The item was expected in the top, but found in the bottom");
    }

  if (expectedInBottom && inTop)
    {
      throw SmException(__FILE__, __LINE__, "The item was expected in the bottom, but found in the top");
    }
}

//===============================================================================

void test1()
  throw (std::exception)
{
  cout << "Test 1: Verify basic functionality." << endl << endl;
  
  TopNMultimap<string, int> tnmm(3);

  tnmm.insert(make_pair("7 One.1", 1));
  tnmm.insert(make_pair("1 One.2", 1));
  tnmm.insert(make_pair("2 Five",  5));
  tnmm.insert(make_pair("6 Four",  4));
  tnmm.insert(make_pair("4 Two",   2));
  tnmm.insert(make_pair("0 Three", 3));

  printMmState<int, string>(tnmm.getTopMultimap(), "Top(3) multimap");
  cout << endl;
  printMmState<int, string>(tnmm.getBottomMultimap(), "Bottom multimap");
  cout << endl;

  cout << endl << "About to remove \"Four\" to test shift-up..." << endl;
  tnmm.erase("Four");

  printMmState<int, string>(tnmm.getTopMultimap(), "Top(3) multimap");
  cout << endl;
  printMmState<int, string>(tnmm.getBottomMultimap(), "Bottom multimap");
  cout << endl;

  cout << endl << "About to add \"Six\" to test shift-down..." << endl;
  tnmm.insert(make_pair("3 Six", 6));

  printMmState<int, string>(tnmm.getTopMultimap(), "Top(3) multimap");
  cout << endl;
  printMmState<int, string>(tnmm.getBottomMultimap(), "Bottom multimap");
  cout << endl;

  cout << endl << "About to empty it out..." << endl;
  tnmm.erase("One.1");
  tnmm.erase("One.2");
  tnmm.erase("Five");
  tnmm.erase("Six");
  tnmm.erase("Two");
  tnmm.erase("Three");

  printMmState<int, string>(tnmm.getTopMultimap(), "Top(3) multimap");
  cout << endl;
  printMmState<int, string>(tnmm.getBottomMultimap(), "Bottom multimap");
  cout << endl;
}

//===============================================================================
/*
void test2()
{
  cout << "Test 2: Verify proper handling of duplicate-priority objects." << endl << endl;

  TopNMultimap<int, string> mm(4);

  mm.insert(make_pair(3, "Three.1"));
  mm.insert(make_pair(5, "Five.1"));
  mm.insert(make_pair(4, "Four"));
  mm.insert(make_pair(5, "Five.2"));
  mm.insert(make_pair(4, "Four"));
  mm.insert(make_pair(1, "One"));
  mm.insert(make_pair(4, "Four"));
  mm.insert(make_pair(3, "Three.2"));

  printMmState<int, string>(mm.getTopMultimap(), "Top(4) multimap");
  cout << endl;
  printMmState<int, string>(mm.getBottomMultimap(), "Bottom multimap");
  cout << endl;

  cout << "About to whack all {4, \"Four\"} objects..." << endl;
  mm.erase(make_pair(4, "Four"));

  printMmState<int, string>(mm.getTopMultimap(), "Top(4) multimap");
  cout << endl;
  printMmState<int, string>(mm.getBottomMultimap(), "Bottom multimap");
  cout << endl;
}

//===============================================================================

void test3()
  throw (std::exception)
{
  cout << "Test 3: Verify findByKey(...) functionality." << endl << endl;

  TopNMultimap<int, string> mm(2);

  mm.insert(make_pair(1, "1"));
  mm.insert(make_pair(2, "2"));
  mm.insert(make_pair(3, "3"));
  mm.insert(make_pair(4, "4"));

  verifyFindByKey(mm, 1, "1", false, true);
  verifyFindByKey(mm, 2, "2", false, true);
  verifyFindByKey(mm, 3, "3", true, false);
  verifyFindByKey(mm, 4, "4", true, false);
}

*/
//===============================================================================

int main()
{
  try
    {
      test1();
      //    test2();
      //      test3();
    }
  catch (std::exception & e)
    {
      cout << "Test failed: main() caught exception: " << e.what() << endl;
      return 1;
    }

  cout << "All tests passed." << endl;
}
