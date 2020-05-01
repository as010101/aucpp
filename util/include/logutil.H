// Convenienve methods, mainly meant for debugging when you want to know the
// values of a bunch of variables...

#ifndef LOGUTIL_H
#define LOGUTIL_H

#include <exception>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

//===============================================================================

template<typename T1>
vector<string> makeDebugLogLines(const string & title,
					const string & t1Name, const T1 & t1Value)
  throw (exception)
{
  vector<string> v;
  v.push_back(title);
  
  ostringstream os;
  os << t1Name << " = " << t1Value;
  v.push_back(os.str());

  return v;
}

//===============================================================================

template<typename T1,
	 typename T2>
vector<string> makeDebugLogLines(const string & title,
					const string & t1Name, const T1 & t1Value,
					const string & t2Name, const T2 & t2Value)
  throw (exception)
{
  vector<string> v;
  v.push_back(title);
  
  ostringstream os;
  os << t1Name << " = " << t1Value;
  v.push_back(os.str());

  os.str("");
  os << t2Name << " = " << t2Value;
  v.push_back(os.str());

  return v;
}

//===============================================================================

template<typename T1,
	 typename T2,
	 typename T3>
vector<string> makeDebugLogLines(const string & title,
					const string & t1Name, const T1 & t1Value,
					const string & t2Name, const T2 & t2Value,
					const string & t3Name, const T3 & t3Value)
  throw (exception)
{
  vector<string> v;
  v.push_back(title);
  
  ostringstream os;
  os << t1Name << " = " << t1Value;
  v.push_back(os.str());

  os.str("");
  os << t2Name << " = " << t2Value;
  v.push_back(os.str());

  os.str("");
  os << t3Name << " = " << t3Value;
  v.push_back(os.str());

  return v;
}

//===============================================================================

template<typename T1,
	 typename T2,
	 typename T3,
	 typename T4>
vector<string> makeDebugLogLines(const string & title,
					const string & t1Name, const T1 & t1Value,
					const string & t2Name, const T2 & t2Value,
					const string & t3Name, const T3 & t3Value,
					const string & t4Name, const T4 & t4Value)
  throw (exception)
{
  vector<string> v;
  v.push_back(title);
  
  ostringstream os;
  os << t1Name << " = " << t1Value;
  v.push_back(os.str());

  os.str("");
  os << t2Name << " = " << t2Value;
  v.push_back(os.str());

  os.str("");
  os << t3Name << " = " << t3Value;
  v.push_back(os.str());

  os.str("");
  os << t4Name << " = " << t4Value;
  v.push_back(os.str());

  return v;
}


//===============================================================================

template<typename T1,
	 typename T2,
	 typename T3,
	 typename T4,
	 typename T5>
vector<string> makeDebugLogLines(const string & title,
					const string & t1Name, const T1 & t1Value,
					const string & t2Name, const T2 & t2Value,
					const string & t3Name, const T3 & t3Value,
					const string & t4Name, const T4 & t4Value,
					const string & t5Name, const T5 & t5Value)
  throw (exception)
{
  vector<string> v;
  v.push_back(title);
  
  ostringstream os;
  os << t1Name << " = " << t1Value;
  v.push_back(os.str());

  os.str("");
  os << t2Name << " = " << t2Value;
  v.push_back(os.str());

  os.str("");
  os << t3Name << " = " << t3Value;
  v.push_back(os.str());

  os.str("");
  os << t4Name << " = " << t4Value;
  v.push_back(os.str());

  os.str("");
  os << t5Name << " = " << t5Value;
  v.push_back(os.str());

  return v;
}


//===============================================================================

template<typename T1,
	 typename T2,
	 typename T3,
	 typename T4,
	 typename T5,
	 typename T6>
vector<string> makeDebugLogLines(const string & title,
					const string & t1Name, const T1 & t1Value,
					const string & t2Name, const T2 & t2Value,
					const string & t3Name, const T3 & t3Value,
					const string & t4Name, const T4 & t4Value,
					const string & t5Name, const T5 & t5Value,
					const string & t6Name, const T6 & t6Value)
  throw (exception)
{
  vector<string> v;
  v.push_back(title);
  
  ostringstream os;
  os << t1Name << " = " << t1Value;
  v.push_back(os.str());

  os.str("");
  os << t2Name << " = " << t2Value;
  v.push_back(os.str());

  os.str("");
  os << t3Name << " = " << t3Value;
  v.push_back(os.str());

  os.str("");
  os << t4Name << " = " << t4Value;
  v.push_back(os.str());

  os.str("");
  os << t5Name << " = " << t5Value;
  v.push_back(os.str());

  os.str("");
  os << t6Name << " = " << t6Value;
  v.push_back(os.str());

  return v;
}


//===============================================================================

template<typename T1,
		 typename T2,
		 typename T3,
		 typename T4,
		 typename T5,
		 typename T6,
		 typename T7,
		 typename T8,
		 typename T9,
		 typename T10,
		 typename T11,
		 typename T12,
		 typename T13,
		 typename T14,
		 typename T15>
vector<string> makeDebugLogLines(const string & title,
					const string & t1Name,  const T1  & t1Value,
					const string & t2Name,  const T2  & t2Value,
					const string & t3Name,  const T3  & t3Value,
					const string & t4Name,  const T4  & t4Value,
					const string & t5Name,  const T5  & t5Value,
					const string & t6Name,  const T6  & t6Value,
					const string & t7Name,  const T7  & t7Value,
					const string & t8Name,  const T8  & t8Value,
					const string & t9Name,  const T9  & t9Value,
					const string & t10Name, const T10 & t10Value,
					const string & t11Name, const T11 & t11Value,
					const string & t12Name, const T12 & t12Value,
					const string & t13Name, const T13 & t13Value,
					const string & t14Name, const T14 & t14Value,
					const string & t15Name, const T15 & t15Value)
  throw (exception)
{
  vector<string> v;
  v.push_back(title);
  
  ostringstream os;
  os << t1Name << " = " << t1Value;
  v.push_back(os.str());

  os.str("");
  os << t2Name << " = " << t2Value;
  v.push_back(os.str());

  os.str("");
  os << t3Name << " = " << t3Value;
  v.push_back(os.str());

  os.str("");
  os << t4Name << " = " << t4Value;
  v.push_back(os.str());

  os.str("");
  os << t5Name << " = " << t5Value;
  v.push_back(os.str());

  os.str("");
  os << t6Name << " = " << t6Value;
  v.push_back(os.str());

  os.str("");
  os << t7Name << " = " << t7Value;
  v.push_back(os.str());

  os.str("");
  os << t8Name << " = " << t8Value;
  v.push_back(os.str());

  os.str("");
  os << t9Name << " = " << t9Value;
  v.push_back(os.str());

  os.str("");
  os << t10Name << " = " << t10Value;
  v.push_back(os.str());

  os.str("");
  os << t11Name << " = " << t11Value;
  v.push_back(os.str());

  os.str("");
  os << t12Name << " = " << t12Value;
  v.push_back(os.str());

  os.str("");
  os << t13Name << " = " << t13Value;
  v.push_back(os.str());

  os.str("");
  os << t14Name << " = " << t14Value;
  v.push_back(os.str());

  os.str("");
  os << t15Name << " = " << t15Value;
  v.push_back(os.str());

  return v;
}

#endif
