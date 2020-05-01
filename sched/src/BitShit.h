#ifndef BIT_SHIT_H
#define BIT_SHIT_H

class BitShit
{
public:
	BitShit(int num_bits);
	void clear();
	void setBit(int bit_loc);
	void unsetBit(int bit_loc);
	int testBit(int bit_loc);
private:
	int _num_bits;
	unsigned char* _bitarray;
};

#endif
