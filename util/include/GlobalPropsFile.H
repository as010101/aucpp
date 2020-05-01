#ifndef GLOBALPROPSFILE_H
#define GLOBALPROPSFILE_H

#include <map>
#include <exception>
#include <PropsFile.H>

using namespace std;

/**
 Provides a singleton instance of a PropsFile object.

 CONCURRENCY:
 When a thread is invoking "makeInstance(...)" or "destroyInstance", that must
 be the only pending method invocation on this class.

 Other than that, concurrent invocations may be rampant.
 */


class GlobalPropsFile
{
public:
  // Sets the file that the global PropsFile will report on, and creates the one
  // instance that's used by all the other methods.
  //
  // Throws an exception if any of the elements in the file aren't properly 
  // parsable, or if an instance already exists, or if the file doesn't exist.
  //
  // The instance is valid until destroyInstance() is called.
  static void makeInstance(string filepath)
    throw (exception);

  // Destroys the instance of this object created by makeInstance.
  static void destroyInstance()
    throw (exception);

  // Returns a pointer to the instance of the PropsFile. There must *be* a 
  // current instance for this to work.
  static const PropsFile * getInstance()
    throw (exception);

private:
  GlobalPropsFile();
  virtual ~GlobalPropsFile();

  static PropsFile * s_pInstance;
};

#endif
