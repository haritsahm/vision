//Input-Output/Common Header Files
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <sstream>
#include <stdio.h>
#include <math.h>
#include <cmath>
//OpenCV Headers
#include <opencv/cv.h>
#include <opencv/highgui.h>
//Blob Library Headers
#include <cvblob.h>

//Definitions
#define WIDTHd 752
#define HEIGHTd 480
#define HEIGHTu 752
#define WIDTHu 480
#define PI 3.14159265359
#define rad2deg(x) x*180/PI
//NameSpaces
using namespace std;
using namespace cvb;

CvPoint dis2undis(CvPoint dis, int degree)
{
	double R[7];							//coefficient for R polynomial
	double O[7];							//coefficient for O polynomial

	R[0] = 3.4633;							//values for R polynomial
	R[1] = 4.8994;
	R[2] = 0.0189;
	R[3] = -0.0002;
	R[4] = 0.0;
	R[5] = 0.0;

	O[0] = 0.0;								//values for O polynomial
	O[1] = 0.8793;
	O[2] = 0.2278;
	O[3] = -0.2707;
	O[4] = 0.1956;
	O[5] = -0.0538;

	double r = sqrt(dis.x*dis.x + dis.y*dis.y);
	double o = atan2(dis.y, dis.x);

	CvPoint undis;
	double ru = R[0] + R[1]*pow(r,1) + R[2]*pow(r,2) + R[3]*pow(r,3) + R[4]*pow(r,4) + R[5]*pow(r,5); 
	double ou = O[0] + O[1]*pow(o,1) + O[2]*pow(o,2) + O[3]*pow(o,3) + O[4]*pow(o,4) + O[5]*pow(o,5);
	undis.x = ru*cos(ou);
	undis.y = ru*sin(ou);
	return undis;
}

main(void)
{
	IplImage* bgr_frame = cvLoadImage("distorted.bmp");
	IplImage* undistorted = cvCreateImage(cvSize(WIDTHu,HEIGHTu), 8, 3);
	CvPoint dis, undis;

	for (int i = 0; i < WIDTHd; ++i)
	{
		for (int j = 0; j < HEIGHTd; ++j)
		{
			dis.x = i - WIDTHd/2;
			dis.y = HEIGHTd/2 - j;
			undis = dis2undis(dis, 5);
			undis.x = WIDTHu/2 + undis.x;
			undis.y = HEIGHTu/2 - undis.y;
			dis.x = WIDTHd/2 + dis.x;
			dis.y = HEIGHTd/2 - dis.y;
		}
	}
}