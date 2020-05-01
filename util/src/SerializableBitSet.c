#include <SerializableBitSet.H>
#include <util.H>
#include <unistd.h>
#include <string.h>
#include <iostream>

//=========================================================================================

SerializableBitSet::SerializableBitSet(int numBits)
  throw (std::exception)
{
  if (numBits < 1)
    {
      throw SmIllegalParameterValueException(__FILE__, __LINE__, "numBits < 1");
    }

  _numDataBits = numBits;

  if ((_numDataBits % 8) == 0)
    {
      _numDataBytes = _numDataBits / 8;
    }
  else
    {
      _numDataBytes = (_numDataBits / 8) + 1;
    }

  _pData = new unsigned char[_numDataBytes];
  memset(_pData, 0x00, _numDataBytes);
  _numTrueBits = 0;
  _numFalseBits = numBits;

}

//=========================================================================================

SerializableBitSet::SerializableBitSet(const SerializableBitSet & src)
  throw (exception)
{
  _pData = new unsigned char[src._numDataBytes];
  memcpy(_pData, src._pData, src._numDataBytes);

  _numDataBits  = src._numDataBits;
  _numDataBytes = src._numDataBytes;
  _numTrueBits  = src._numTrueBits;
  _numFalseBits = src._numFalseBits;
}

//=========================================================================================

SerializableBitSet::~SerializableBitSet()
{
  delete[] _pData;
}

//=========================================================================================

void SerializableBitSet::load(int fd)
  throw (std::exception)
{
  off_t offset = getFileOffset(fd);
  readData(fd, offset, reinterpret_cast<char *>(_pData), _numDataBytes);

  _numTrueBits = 0;
  _numFalseBits = 0;

  for (int i = 0; i < _numDataBits; i++)
    {
      if (testBit(i))
	{
	  _numTrueBits++;
	}
      else
	{
	  _numFalseBits++;
	}
    }
}

//=========================================================================================

void SerializableBitSet::save(int fd) const
  throw (std::exception)
{
  off_t offset = getFileOffset(fd);
  writeData(fd, offset, reinterpret_cast<char *>(_pData), _numDataBytes);
}

//=========================================================================================

bool SerializableBitSet::testBit(int bitNum) const
  throw (std::exception)
{
  if ((bitNum < 0) || (bitNum >= _numDataBits))
    {
      throw SmIllegalParameterValueException(__FILE__, __LINE__, 
						       "((bitNum < 0) || (bitNum >= _numDataBits))");
    }

  int byteOffset = (bitNum / 8);
  int bitOffset = (bitNum % 8);

  return ((_pData[byteOffset] & (0x01 << bitOffset)) != 0x00);
}

//=========================================================================================

int SerializableBitSet::getBitCountForValue(bool value) const
{
  if (value)
    {
      return _numTrueBits;
    }
  else
    {
      return _numFalseBits;
    }
}

//=========================================================================================

void SerializableBitSet::setBit(int bitNum, bool newValue)
  throw (std::exception)
{
  if ((bitNum < 0) || (bitNum >= _numDataBits))
    {
      throw SmIllegalParameterValueException(__FILE__, __LINE__, 
						       "((bitNum < 0) || (bitNum >= _numDataBits))");
    }
  
  int byteOffset = (bitNum / 8);
  int bitOffset = (bitNum % 8);

  bool oldValue = ((_pData[byteOffset] & (0x01 << bitOffset)) != 0x00);
  
  if (newValue)
    {
      _pData[byteOffset] |= (0x01 << bitOffset);

      if (newValue != oldValue)
	{
	  _numTrueBits++;
	  _numFalseBits--;
	}
    }
  else
    {
      _pData[byteOffset] ^= (0x01 << bitOffset);

      if (newValue != oldValue)
	{
	  _numTrueBits--;
	  _numFalseBits++;
	}
    }
}

//=========================================================================================

int SerializableBitSet::getFirstIdxForValue(bool value) const
{
  int wholeBytesInBitset = (_numDataBits / 8);
  int spareBitsInBitset  = (_numDataBits % 8);

  // Search all of the *full* bytes in the bitset for a matching bit...
  bool matchingByteFound = false;
  int currentByteIdx = 0;
  unsigned char skipByteValue = (value ? 0x00 : 0xff);

  while ((currentByteIdx < wholeBytesInBitset) && (! matchingByteFound))
    {
      if (_pData[currentByteIdx] == skipByteValue)
	{
	  currentByteIdx++;
	}
      else
	{
	  matchingByteFound = true;
	}
    }

  // OK, so far we've only looked at the whole bytes in the bitset. Did we find a match?
  if (matchingByteFound)
    {
      // We definitely have a match. We just have to find out which bit it is...
      for (int i = 0; i < 8; i++)
	{
	  bool bitSet = _pData[currentByteIdx] & (0x01 << i);
	  if (bitSet == value)
	    {
	      int returnVal = (8 * currentByteIdx) + i;
	      return returnVal;
	    }
	}

      // If we got here, something is wrong.
      assert(false);
    }

  // If we got here, then exactly one of two things is true: 
  // (a) the bitset has a multiple of 8 bits, and none of them had the desired value, or
  // (b) we need to look at the remainder bits from the last byte of the bitset...
  //  currentByteIdx++;
  for (int i = 0; i < spareBitsInBitset; i++)
    {
      bool bitSet = _pData[currentByteIdx] & (0x01 << i);
      if (bitSet == value)
	{
	  int returnVal = (8 * currentByteIdx) + i;
	  return returnVal;
	}
    }

  // No match was found.
  return -1;
}

//=========================================================================================

bool SerializableBitSet::operator==(const SerializableBitSet & rhs) const
{
  if (_numDataBits != rhs._numDataBits)
    {
      return false;
    }

  return (memcmp(_pData, rhs._pData, _numDataBytes) == 0);
}

//=========================================================================================

SerializableBitSet & SerializableBitSet::operator= (const SerializableBitSet & rhs) 
  throw (exception)
{
  delete[] _pData;
  _pData = new unsigned char[rhs._numDataBytes];
  memcpy(_pData, rhs._pData, rhs._numDataBytes);

  _numDataBits  = rhs._numDataBits;
  _numDataBytes = rhs._numDataBytes;
  _numTrueBits  = rhs._numTrueBits;
  _numFalseBits = rhs._numFalseBits;

  return *this;
}

//=========================================================================================
