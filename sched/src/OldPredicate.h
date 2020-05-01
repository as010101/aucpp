#include <list>

using namespace std;

#ifndef OLD_PREDICATE_H
#define OLD_PREDICATE_H

#define GREATER_THAN 		0			// >
#define LESS_THAN    		1			// <
#define EQUAL        		2			// =
#define GREATER_OR_EQUAL 	3			// >=
#define LESS_OR_EQUAL    	4			// <=
#define NATURAL_JOIN            5

union old_u_type 
{
  int	 _ival;
  float  _fval;
  double _dval;
  char	*_sval;
};

// This is a very simple predicate class
// A predicate will be of the form:
//       field operator value
// or
//       field operator field
//
// Member Variables:
// 		_left_side_field_index is the field number of the left hand
//			side of the predicate
// 		_operator is obvious
//		_right_side_type is either 0 (for value) or 1 (for index)
//		_right_side_value is the value for the right had side if
//			_right_side_type is 0
// 		_right_side_field_index is the field number of the right hand
//			side of the predicate if _right_side_type is 1
//              _num_common_fields is the number of common fields two streams
//                      share for processing a join
//              _stream1_field_indices is an array of integers specifying the
//                      field indices of stream1 to be used for a join
//              _stream2_field_indices is an array of integers specifying the
//                      field indices of stream2 to be used for a join
//              _streamStarting indicates whether this is these are the first tuples
//                      being computed for the join
//
//
//
//		

class OldPredicate
{
public:
	OldPredicate() {};
	OldPredicate(int ls_field_index, int oper, int rs_type, 
		  old_u_type rs_value, int rs_value_type,  int rs_field_index  );
	OldPredicate(int ls_field_index, int oper, int rs_type, 
		  old_u_type rs_value, int rs_value_type,  int rs_field_index,
                  int num_com_fields, int *s1_field_indices, int *s2_field_indices );
	~OldPredicate() {}

	int     getLSFieldIndex() 	 { return _left_side_field_index; }
	int	getOperator()     	 { return _operator; }
	int	getRSType()       	 { return _right_side_type; }
	old_u_type 	getRSValue()             { return _right_side_value; }
        int     getRSIntValue()          { return _right_side_value._ival; } 
        float   getRSFloatValue()        { return _right_side_value._fval; }
        double  getRSDoubleValue()       { return _right_side_value._dval; }
        int	getRSFieldIndex() 	 { return _right_side_field_index; }
	int	getRSValueType() { return _right_side_value_type; }
        int     getNumCommonFields() { return _num_common_fields; }
        int*    getStream1JoinFields() {  return _stream1_field_indices; }
        int*    getStream2JoinFields() {  return _stream2_field_indices; } 


	void    setLSFieldIndex(int lsi) { _left_side_field_index = lsi; }
	void    setOperator(int oper)	 { _operator = oper; }
	void    setRSType(int type)	 { _right_side_type = type; }
	void 	setRSValue(old_u_type value) { _right_side_value = value; }
        void    setRSIntValue(int value) { _right_side_value._ival = value;}
        void    setRSFloatValue(int value) { _right_side_value._fval = value;}
        void    setRSDoubleValue(int value) { _right_side_value._dval = value;}
	void    setRSFieldIndex(int rsi) { _right_side_field_index = rsi; }
	void	setRSValueType(int type) { _right_side_value_type = type; }
        void    setNumCommonFields(int num) { _num_common_fields; }
        void    setStream1JoinFields(int *list) {  _stream1_field_indices = list; }
        void    setStream2JoinFields(int *list) {  _stream2_field_indices = list; }


private:
	int 	_left_side_field_index;	
	int	_operator;
	int	_right_side_type;	// value(0) or field index(1)
	old_u_type	_right_side_value;
	int	_right_side_field_index;
	int	_right_side_value_type;
        int     _num_common_fields;
        int     *_stream1_field_indices;
        int     *_stream2_field_indices;
};


#endif
