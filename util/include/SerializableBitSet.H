#ifndef _SERIALIZABLEBITSET_H
#define _SERIALIZABLEBITSET_H

#include "StorageMgr_Exceptions.H"

class SerializableBitSet
{
public:
  // Creates a bitset with numBits, numbered 0...(numBits-1),
  // that are all initialized to 'false'.
  SerializableBitSet(int numBits)
    throw (exception);

  SerializableBitSet(const SerializableBitSet & src)
    throw (exception);

  virtual ~SerializableBitSet();

  // Populates the bitset from the specified file.
  // The file pointer must be positioned at the first byte of the
  // bitset data. This will read from the file exactly as many bytes 
  // as are needed to contain the number of bits stored by this bitset.
  void load(int fd)
    throw (std::exception);

  // Writes the bitset's data to the specified file.
  // The file pointer must be positioned at the first byte where the bitset
  // is to be written to. This will write to the file exactly as many bytes 
  // as are needed to contain the number of bits stored by this bitset.
  void save(int fd) const
    throw (std::exception);

  // Bit counting is zero-based (0... (numBits - 1)).
  bool testBit(int bitNum) const
    throw (std::exception);

  // Bit counting is zero-based (0... (numBits - 1)).
  void setBit(int bitNum, bool newValue)
    throw (std::exception);

  // Returns the number of bits in the bit set with the specified value.
  int getBitCountForValue(bool value) const;

  // Returns the lowest index of the bits in the bitset that have the
  // specified value.
  // Returns -1 if no bit in the bitset has the specified value.
  int getFirstIdxForValue(bool value) const;

  SerializableBitSet & operator= (const SerializableBitSet & rhs) 
    throw (exception);

  bool operator==(const SerializableBitSet & rhs) const;

private:
  unsigned char *_pData;
  int _numDataBits;
  int _numDataBytes;

  int _numTrueBits;
  int _numFalseBits;
};

#endif
