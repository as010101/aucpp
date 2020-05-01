#ifndef FLOAT_DELTA_AGGREGATE_FUNCTION_H
#define FLOAT_DELTA_AGGREGATE_FUNCTION_H

#include "AggregateFunction.H"
#include "FieldExt.H"

class FloatDeltaAF : public AggregateFunction
{
public:  
	FloatDeltaAF(const char *att);
	virtual ~FloatDeltaAF();
	void init();
	void incr(char *tuple);
	char* final();
	char* evaluate(char *tuple);
	char* evaluate(char *tuple1, char *tuple2) {};
	int getReturnedSize();
	FloatDeltaAF* makeNew();

private:
	char       *_att;
	FieldExt   *_field;

	float      _first;
	float      _last;

	bool       _seenFirst;
};

#endif 
