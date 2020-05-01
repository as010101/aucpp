#ifndef TUPLE_DESCRIPTION_H
#define TUPLE_DESCRIPTION_H

#define	INT_TYPE	7
#define	FLOAT_TYPE	8
#define	DOUBLE_TYPE	9
#define	STRING_TYPE 10
#define TIMESTAMP_TYPE 11

#include <string>
#include <map>
using namespace std;


class TupleDescription
{
public:
	TupleDescription(int num_fields,int *field_types, int *field_sizes);
	~TupleDescription() {}
	TupleDescription(TupleDescription& td);
	int	getNumOfFields() {return _num_fields; }
	int	getFieldType(int field_num); 
	int	getFieldSize(int field_num); 
	int	getFieldOffset(int field_num);
	int	getSize();
	void setFieldName(int index, string fn) {_field_names[index] = fn;}
	string getFieldName(int index) {return _field_names[index];}
private:
	int	_num_fields;
	int	*_field_type;
	int	*_field_size;
	map<int,string> _field_names;
	int	*_offset;
	int _tuple_size;
};


#endif
