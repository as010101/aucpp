#include "BitShit.H"
#include <math.h>
#include <string.h>

BitShit::BitShit(int num_bits)
{
	_num_bits = num_bits;
	_bitarray = new unsigned char[(int)ceil((double)_num_bits/(sizeof(char)*8))];
}

void BitShit::clear()
{
	memset(_bitarray,0,(int)ceil((double)_num_bits/(sizeof(char)*8)));
}
void BitShit::setBit(int bit_loc)
{
	switch ( bit_loc % 8 )
	{
		case 0: 
			_bitarray[bit_loc/8] = _bitarray[bit_loc/8] | 1;
			break;
		case 1: 
			_bitarray[bit_loc/8] = _bitarray[bit_loc/8] | 2;
			break;
		case 2: 
			_bitarray[bit_loc/8] = _bitarray[bit_loc/8] | 4;
			break;
		case 3: 
			_bitarray[bit_loc/8] = _bitarray[bit_loc/8] | 8;
			break;
		case 4: 
			_bitarray[bit_loc/8] = _bitarray[bit_loc/8] | 16;
			break;
		case 5: 
			_bitarray[bit_loc/8] = _bitarray[bit_loc/8] | 32;
			break;
		case 6: 
			_bitarray[bit_loc/8] = _bitarray[bit_loc/8] | 64;
			break;
		case 7: 
			_bitarray[bit_loc/8] = _bitarray[bit_loc/8] | 128;
			break;
	}
}
void BitShit::unsetBit(int bit_loc)
{
	switch ( bit_loc % 8 )
	{
		case 0: 
			_bitarray[bit_loc/8] = _bitarray[bit_loc/8] & 0xfe;
			break;
		case 1: 
			_bitarray[bit_loc/8] = _bitarray[bit_loc/8] & 0xfd;
			break;
		case 2: 
			_bitarray[bit_loc/8] = _bitarray[bit_loc/8] & 0xfb;
			break;
		case 3: 
			_bitarray[bit_loc/8] = _bitarray[bit_loc/8] & 0xf7;
			break;
		case 4: 
			_bitarray[bit_loc/8] = _bitarray[bit_loc/8] & 0xef;
			break;
		case 5: 
			_bitarray[bit_loc/8] = _bitarray[bit_loc/8] & 0xdf;
			break;
		case 6: 
			_bitarray[bit_loc/8] = _bitarray[bit_loc/8] & 0xbf;
			break;
		case 7: 
			_bitarray[bit_loc/8] = _bitarray[bit_loc/8] & 0x7f;
			break;
	}
}
int BitShit::testBit(int bit_loc)
{
	switch ( bit_loc % 8 )
	{
		case 0: 
			if( _bitarray[bit_loc/8] & 1)
				return 1;
			break;
		case 1: 
			if ( _bitarray[bit_loc/8] & 2 )
				return 1;
			break;
		case 2: 
			if ( _bitarray[bit_loc/8] & 4 )
				return 1;
			break;
		case 3: 
			if ( _bitarray[bit_loc/8] & 8 )
				return 1;
			break;
		case 4: 
			if ( _bitarray[bit_loc/8] & 16 )
				return 1;
			break;
		case 5: 
			if ( _bitarray[bit_loc/8] & 32 )
				return 1;
			break;
		case 6: 
			if ( _bitarray[bit_loc/8] & 64 )
				return 1;
			break;
		case 7: 
			if ( _bitarray[bit_loc/8] & 128 )
				return 1;
			break;
	}
	return 0;
}
