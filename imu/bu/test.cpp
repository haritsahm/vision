#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <ftdi.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include <iomanip>

#include "cmtdef.h"
#include "xsens_time.h"
#include "xsens_list.h"
#include "cmtscan.h"
#include "cmt3.h"
#include "imu.h"

using namespace std;

int main()
{
	Imu imu;
	imu.init();
	while(1)
	{
		imu.update();
		cout<<"\n"<<setprecision(3)<<double(imu.yaw);
	}
	return 0;
}