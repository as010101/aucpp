#ifndef TRACELOGGER_H
#define TRACELOGGER_H

#include <PropsFile.H>
#include <exception>
#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <vector>
#include <PtMutex.H>

using namespace std;

/*
  Provides a thread-safe logger to help in debugging programs. It takes the 
  following from the supplied PropsFile:


  OUTPUT CHANNEL CONTROL
  ======================
  All of the following properties are mandatory:
     <TraceLogger.output.file   type="string" value="foo"/>
     <TraceLogger.output.stdout type="bool"   value="bar"/>
     <TraceLogger.output.stderr type="bool"   value="baz"/>

     The file-related property has a special meaning: The file 'foo' will be
     written to iff 'foo' is not the empty string. If 'foo' can't be written to
     for some reason (and isn't the empty string), failed attempts to write to 
     it will be mentioned on stdout/stderr (if each is enable for logging).

     If the specified log file can't even be *initially* opened, then the c'tor
     will throw an exception.

     If a log file is specified, this TraceLogger might keep an open handle on 
     the file up until this object's destructor returns.

  The following property is optional:
     <TraceLogger.output.lineAfterEntries type="bool" value="foo"/>

     Whenn it's defined with a value of 'T' (true), a blank line will follow each
     logfile entry. The intention of this feature is to make log files easier to
     read by humans.

  LOG MESSAGE FILTERING
  =====================
  Additional optional properties can be defined, to describe which set of 
  logging attempts should actually be sent to the logger's outputs. These
  must be of the form:
     <TraceLogger.group.foo type="bool" value="bar"/>

  In this case, 'foo' can be any non-empty string. A calls to the 'log' method
  will appear on the logger's output iff the 'group' parameter to the 'log' 
  method corresponded to one of these properties *and* the property's value
  is 'T'.

  NOTE: The group-name may not be longer than 'MAX_GROUP_NAME_LEN' characters.
  The main reason for this restriction is for logfile formatting.

  For example:
     <TraceLogger.group.StorageMgr.PageFaultEvent type="bool" value="T">

     myLogger.log("StorageMgr.PageFaultEvent", "Page eviction occured");
 */
class TraceLogger
{
public:
  enum { MAX_GROUP_NAME_LEN = 20 }; // Value choice is arbitrary/aesthetic

  // Reads the specified props file, but doesn't keep a reference to it.
  TraceLogger(const PropsFile & props)
    throw (exception);

  // Just don't call this when an invocation if 'log(...)' is pending.
  virtual ~TraceLogger();
    
  // Logs the specified message according to the rules stated above. Concurrent
  // calls to this method are threadsafe and have the normal linearization
  // qualities.
  //
  // Each logged message is immediately flushed.
  //
  // Don't include cr/lr characters in 'message'. If you want a multi-line
  // log message, use the verion of log(...) that takes a vector<string>
  // argument.
  void log(const string & group, const string & message)
    throw (exception);

  // Atomically logs numerous lines of text.
  void log(const string & group, const vector<string> & messages)
    throw (exception);

private:
  static const string timeHeader;
  static const string pidHeader;
  static const string tidHeader;
  static const string groupHeader;
  static const string msgHeader;

  bool _logToStdout;
  bool _logToStderr;
  bool _lineAfterEntries;
  
  ofstream * _pLogfileStream; // NULL if the user didn't want one.

  // Lists all of the values for the log(...) method's 'group' parameter that 
  // will cause the corresponding message to be logged.
  set<string> _loggedGroups;

  // Max # of chars needed to textually represent a TID/PID
  size_t _maxTidWidth;
  size_t _maxPidWidth;

  const char * _pszTimeFormat;
  size_t _formattedTimeWidth;

  PtMutex _mtx;

  size_t _groupColumnWidth;

  // The string that's used to cause message lines in multi-line messages to
  // start at the proper column.
  string _multiLineMsgPadding;

  void writeColumnHeaderLines()
    throw (exception);

  // Assumes the parameters have been validated, and that the caller currently
  // holds _mtx.
  string formatMsgMajorLine(const string & group, const string & messageLine)
    throw (exception);

  // Assumes the parameters have been validated, and that the caller currently
  // holds _mtx.
  string formatMsgMinorLine(const string & messageLine)
    throw (exception);


  // Sends this one line of text to all of the logger's outputs. Assumes that the
  // caller currently holds _mtx's lock.
  void writeLineToOutputs(const string & line)
    throw (exception);

  // Returns true iff a message from the specified log group really needs to be
  // logged.
  //
  // Will throw an exception if 'group' has in illegal value.
  inline bool mustLogMessage(const string & group)
    throw (exception);
};

#endif
