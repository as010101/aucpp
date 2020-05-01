#ifndef ExperimentQBox_Header
#define ExperimentQBox_Header

#include "QBox.H"

class ExperimentQBox : public QBox 
{
public:
	ExperimentQBox()  { 
			_boxType = EXPERIMENT_BOX; 
	}
	~ExperimentQBox() {}
	virtual Box_Out_T	doBox();
	double findYGivenLineAndX(double x1, double y1, double x2, double y2, double x3);
	double findYGivenXAndQos(double x,int qos_graph_num);
	double 				use_time(double time_to_use);
	void 				init_etime(struct itimerval *first);
	double 				get_etime(struct itimerval *first);
};

#endif
