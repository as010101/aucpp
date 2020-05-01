//static char *cvs_id="@(#) $Id: OldPredicate.C,v 1.2 2003/03/26 19:06:25 cjc Exp $";
#include "OldPredicate.H"


OldPredicate::OldPredicate(int ls_field_index, int oper, int rs_type,
				old_u_type rs_value, int rs_value_type, int rs_field_index )
{
	_left_side_field_index = ls_field_index;
	_operator = oper;
	_right_side_type = rs_type;
	_right_side_value = rs_value;
	_right_side_field_index = rs_field_index;
	_right_side_value_type = rs_value_type;
	
}


OldPredicate::OldPredicate(int ls_field_index, int oper, int rs_type,
				old_u_type rs_value, int rs_value_type, int rs_field_index, int num_com_fields, int *s1_field_indices, int *s2_field_indices)
{
	_left_side_field_index = ls_field_index;
	_operator = oper;
	_right_side_type = rs_type;
	_right_side_value = rs_value;
	_right_side_field_index = rs_field_index;
	_right_side_value_type = rs_value_type;
        _num_common_fields = num_com_fields;
        _stream1_field_indices = s1_field_indices;
        _stream2_field_indices = s2_field_indices;
	
}
