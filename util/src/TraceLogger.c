#include <TraceLogger.H>
#include <StorageMgr_Exceptions.H>
#include <sstream>
#include <iomanip>
#include <time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <unistd.h>
#include <LockHolder.H>

const string TraceLogger::timeHeader  = "YYYY/MM/DD HH/MM/SS";
const string TraceLogger::pidHeader   = "PID";
const string TraceLogger::tidHeader   = "TID";
const string TraceLogger::groupHeader = "Log group";
const string TraceLogger::msgHeader   = "Message";

//===============================================================================

TraceLogger::TraceLogger(const PropsFile & props)
  throw (exception)
{
  _logToStdout = props.getBool("TraceLogger.output.stdout");
  _logToStderr = props.getBool("TraceLogger.output.stderr");

  _lineAfterEntries = 
    props.getBoolWithDefault("TraceLogger.output.lineAfterEntries", false);

  string filename = props.getString("TraceLogger.output.file");
  if (filename.empty())
    {
      _pLogfileStream = NULL;
    }
  else
    {
      _pLogfileStream = new ofstream(filename.c_str(), ios::out | ios::app);

      if (! (*_pLogfileStream))
	{
	  ostringstream os;
	  os << "Can't open log file for appending: " << filename;
	  throw SmException(__FILE__, __LINE__, os.str());
	}
    }

  const string prefix = "TraceLogger.group.";
  const string::size_type prefixLen = prefix.length();

  const map<string, string> groupProps = 
    props.findPropsByPrefix(prefix);

  size_t maxGroupnameLen = 0;

  for (map<string, string>::const_iterator pos = groupProps.begin();
       pos != groupProps.end(); ++pos)
    {
      if (pos->second != "bool")
	{
	  ostringstream os;
	  os << "Found a TraceLogger.group.* property whose type isn't 'bool': "
	     << "name = " << pos->first << ", type = " << pos->second;
	  throw SmException(__FILE__, __LINE__, os.str());
	  
	}

      if (pos->first == prefix)
	{
	  ostringstream os;
	  os << "Properties whose name start with '" << prefix << "' must have "
	     << "at least on additional character at the end of the property "
	     << "name.";
	  throw SmException(__FILE__, __LINE__, os.str());
	}

      string groupName = pos->first.substr(prefixLen);

      if (groupName.length() > MAX_GROUP_NAME_LEN)
	{
	  ostringstream os;
	  os << "The property '" << pos->first << "' gives a group name '"
	     << groupName << "' that's too long." << endl
	     << "The max group name length is " << (unsigned long) MAX_GROUP_NAME_LEN
	     << ", but this group name's length is " << groupName.length();

	  throw SmException(__FILE__, __LINE__, os.str());  
	}

      if (props.getBool(pos->first))
	{
	  _loggedGroups.insert(groupName);
	  maxGroupnameLen = max(maxGroupnameLen, groupName.length());
	}
    }

  _groupColumnWidth = max(maxGroupnameLen, groupHeader.length());



  ostringstream strMaxPid;
  strMaxPid << numeric_limits<pid_t>::max();

  ostringstream strMinPid;
  strMinPid << numeric_limits<pid_t>::min();
  
  _maxPidWidth = max(strMaxPid.str().length(), strMinPid.str().length());



  ostringstream strMaxTid;
  strMaxTid << numeric_limits<pthread_t>::max();

  ostringstream strMinTid;
  strMinTid << numeric_limits<pthread_t>::min();
  
  _maxTidWidth = max(strMaxTid.str().length(), strMinTid.str().length());



  _pszTimeFormat = "%Y/%m/%d %H:%M:%S";
  _formattedTimeWidth = 19;


  // The '3' in the calculation is for the column separator we use: " | ".
  /*
  string::size_type numPadChars = 
    timeHeader.length() + 3 +
    _maxPidWidth + 3 +
    _maxTidWidth + 3 +
    groupHeader.length() + 3;

  _multiLineMsgPadding = string(numPadChars, ' ');

  */
  _multiLineMsgPadding = 
    string(timeHeader.length(), ' ') + " | " +
    string(_maxPidWidth,        ' ') + " | " +
    string(_maxTidWidth,        ' ') + " | " +
    string(_groupColumnWidth,   ' ') + " | ";

  writeColumnHeaderLines();
}

//===============================================================================

TraceLogger::~TraceLogger()
{
  delete _pLogfileStream;
}

//===============================================================================

void TraceLogger::log(const string & group, const string & message)
  throw (exception)
{
  if (! mustLogMessage(group))
    {
      return;
    }

  string line = formatMsgMajorLine(group, message);
  
  {
    LockHolder lh(_mtx);
    writeLineToOutputs(line);

    if (_lineAfterEntries)
      {
	static const string emptyString("");
	writeLineToOutputs(emptyString);
      }
  }
}

//===============================================================================

void TraceLogger::log(const string & group, const vector<string> & messages)
  throw (exception)
{
  if ((! mustLogMessage(group)) || messages.empty())
    {
      return;
    }

  // Format the output...
  size_t numLines = messages.size();

  vector<string> formattedLines;
  formattedLines.reserve(numLines);

  formattedLines.push_back(formatMsgMajorLine(group, messages[0]));
  
  for (size_t i = 1; i < numLines; ++i)
    {
      formattedLines.push_back(formatMsgMinorLine(messages[i]));
    }

  
  // Print the output...
  {
    LockHolder lh(_mtx);

    for (size_t i = 0; i < numLines; ++i)
      {
	writeLineToOutputs(formattedLines[i]);
      }

    if (_lineAfterEntries)
      {
	static const string emptyString("");
	writeLineToOutputs(emptyString);
      }
  }

}

//===============================================================================

void TraceLogger::writeColumnHeaderLines()
  throw (exception)
{
  assert(timeHeader.length()  <= _formattedTimeWidth);
  assert(pidHeader.length()   <= _maxPidWidth);
  assert(tidHeader.length()   <= _maxTidWidth);
  assert(groupHeader.length() <= MAX_GROUP_NAME_LEN);

  ostringstream osColumnNames;
  osColumnNames << timeHeader
		<< " | " << setw(_maxPidWidth)      << left << pidHeader
		<< " | " << setw(_maxTidWidth)      << left << tidHeader
		<< " | " << setw(_groupColumnWidth) << left << groupHeader
		<< " | " << msgHeader;

  string separatorLine(osColumnNames.str().length(), '=');


  writeLineToOutputs(separatorLine);
  writeLineToOutputs(osColumnNames.str());
  writeLineToOutputs(separatorLine);
}

//===============================================================================

string TraceLogger::formatMsgMajorLine(const string & group, 
				       const string & messageLine)
  throw (exception)
{
  time_t time_tNow = time(NULL);

  struct tm tmNow;
  if (localtime_r(& time_tNow, & tmNow) == NULL)
    {
      throw SmException(__FILE__, __LINE__, "Failed call to localtime_r");
    }
  
  char szTime[_formattedTimeWidth + 1];
  strftime(szTime, sizeof(szTime), _pszTimeFormat, & tmNow);
  szTime[_formattedTimeWidth] = '\0';

  ostringstream os;
  os << szTime 
     << " | " << setw(_maxPidWidth)              << getpid()
     << " | " << setw(_maxTidWidth)              << pthread_self()
     << " | " << setw(_groupColumnWidth) << left << group
     << " | " << messageLine;

  return os.str();
}

//===============================================================================

string TraceLogger::formatMsgMinorLine(const string & messageLine)
  throw (exception)
{
  return _multiLineMsgPadding + messageLine;
}

//===============================================================================

void TraceLogger::writeLineToOutputs(const string & line)
  throw (exception)
{
  if (_logToStdout)
    {
      cout << line << endl;
    }

  if (_logToStderr)
    {
      cerr << line << endl;
    }

  if (_pLogfileStream != NULL)
    {
      (*_pLogfileStream) << line << endl;
    }
}

//===============================================================================

inline bool TraceLogger::mustLogMessage(const string & group)
  throw (exception)
{
  if (((! _logToStdout) && (! _logToStderr) && (_pLogfileStream == NULL))
      || _loggedGroups.empty())
    {
      return false;
    }

  if (group.length() > MAX_GROUP_NAME_LEN)
    {
      ostringstream os;
      os << "The group name '" << group << "' is too long." << endl
	 << "The max group name length is " << (unsigned long) MAX_GROUP_NAME_LEN
	 << ", but this group name's length is " << group.length();

      throw SmException(__FILE__, __LINE__, os.str());  
    }

  if (_loggedGroups.find(group) == _loggedGroups.end())
      return false;

  return true;
}
