#include <BinarySem.H>
#include <LockHolder.H>
#include <iostream>

//===============================================================================

BinarySem::BinarySem(bool initiallyPosted)
  throw (std::exception)
  : _isPosted(initiallyPosted)
{
}

//===============================================================================

BinarySem::~BinarySem()
{
}

//===============================================================================

bool BinarySem::post()
  throw (std::exception)
{
  LockHolder holder(_mtx);
  bool wasPosted = _isPosted;

  if (! _isPosted)
    {
      _isPosted = true;
      _cond.signal();
    }

  return wasPosted;
}

//===============================================================================

void BinarySem::awaitPostThenClear()
  throw (std::exception)
{
  LockHolder holder(_mtx);
  while (! _isPosted)
    {
      _mtx.waitCond(_cond);
    }

  _isPosted = false;
}

//===============================================================================

void BinarySem::awaitPost()
  throw (std::exception)
{
  LockHolder holder(_mtx);
  while (! _isPosted)
    {
      _mtx.waitCond(_cond);
    }
}

//===============================================================================

bool BinarySem::isPosted() const
  throw (std::exception)
{
  LockHolder holder(_mtx);
  return _isPosted;
}

//===============================================================================
