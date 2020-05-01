//static char *cvs_id="@(#) $Id: TupleDescription.C,v 1.5 2003/04/03 16:14:54 cjc Exp $";
#include "TupleDescription.H"
#include <stdio.h>

TupleDescription::TupleDescription(int num_fields,int *field_types, int *field_sizes)
{
	_num_fields = num_fields;
	_field_type = new int[_num_fields];
	_field_size = new int[_num_fields];
	_offset = new int[_num_fields];

	_offset[0] = 0;
	_tuple_size = 0;
	for ( int i = 0; i < _num_fields; i++ )
	{
		_field_type[i] = field_types[i];
		_field_size[i] = field_sizes[i];
		_tuple_size += field_sizes[i];
		if ( i < (_num_fields - 1) )
			_offset[i+1] = field_sizes[i] + _offset[i];
	}
	//printf("GOT TO _tuple_size: %i\n",_tuple_size);
}
TupleDescription::TupleDescription(TupleDescription& td)
{
    // this is a copy constructor
    _num_fields = td._num_fields;
	_tuple_size = td._tuple_size;
	_field_type = new int[_num_fields];
	_field_size = new int[_num_fields];
  	_offset = new int[_num_fields+1];
    for ( int i = 0; i < _num_fields; i++ )
	{
		_field_type[i] = td._field_type[i];
		_field_size[i] = td._field_size[i];
		_offset[i] = td._offset[i];
	}
}


int TupleDescription::getFieldType(int field_num)
{
  return ( _field_type[field_num] );
}
int TupleDescription::getFieldSize(int field_num)
{
  return ( _field_size[field_num] );
}
int TupleDescription::getFieldOffset(int field_num)
{
  return ( _offset[field_num] );
}
int TupleDescription::getSize()
{
  return (_tuple_size);
}
