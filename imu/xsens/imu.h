#ifndef __IMU_H
#define __IMU_H
#include "cmt3.h"
class Imu
{
private:
	bool initialized;
	xsens::Cmt2s serial;
	xsens::Message msg, reply;
	unsigned long msgCount;
public:
	Imu()
	{
		initialized = false;
		roll = 0.;
		pitch = 0.;
		yaw = 0.;
		msgCount = 0;
	};
	bool init();
	void update();
	double return_yaw();
	double roll, pitch, yaw;
};
#endif