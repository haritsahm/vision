#include <iostream>
#include <stdio.h>

//OpenCV Headers
#include <opencv/cv.h>
#include <opencv/highgui.h>
//Input-Output
#include <stdio.h>
//Blob Library Headers
#include <cvblob.h>

using namespace std;
using namespace cv;

#define returnPixel3C(image, x, y, color) ((uchar*)(image->imageData + image->widthStep*(y)))[(x)*3 + color]

inline uchar pixelColor3C(IplImage* image, int x, int y, int color)
{
	return ((uchar*)(image->imageData + image->widthStep*y))[x*3 + color];
}

inline void setPixel3C(CvScalar &pixel, uchar blue, uchar green, uchar red)
{
	pixel.val[0] = blue;
	pixel.val[1] = green;
	pixel.val[2] = red;
	return;
}

void undistort_regression(int xd, int yd, int* xu, int* yu)
{
    long double angd = atan2(yd, xd);
    long double rd = sqrt(xd*xd + yd*yd);
    
    // long double b0= 0.033194635 ,b1=-1.713992386, b2= 0.077231385, b3=-0.000801141 , b4=3.64E-06, b5= -5.81E-09;       //angles
    // long double a0= 0.223774577 ,a1= 0.527567, a2=-0.334783789, a3=0.329890623, a4=0.053758761, a5=-0.041780601;        //r

    long double a0= 0 ,a1=0.9987, a2= 3.9523e-17, a3=5.2673e-04 , a4=-7.5649e-18, a5= -3.8569e-5;       //angles
    // long double b0= 0.0378 ,b1= 5.1859, b2=0.0056, b3=4.1804e-05, b4=-4.8267e-07, b5=2.8331e-09;        //r

    long double b0 = 0.0335796977287738;
	long double b1 = -1.66579621568387;
	long double b2 = 0.0758694252746315;
	long double b3 = -0.000790860340061123;
	long double b4 = 3.61028161687448e-06;
	long double b5 = -5.78307382770355e-09;

	// long double a0 = 0.222520384431437;
	// long double a1 = 0.497053272792960;
	// long double a2 = -0.333230330677235;
	// long double a3 = 0.343462374810931;
	// long double a4 = 0.0535130063360605;
	// long double a5 = -0.0429035701605813;

    long double angu = a0+ a1*angd + a2*angd*angd + a3*angd*angd*angd + a4*angd*angd*angd*angd + a5*angd*angd*angd*angd*angd;
    long double ru = (b0 + b1*rd + b2*rd*rd + b3*rd*rd*rd + b4*rd*rd*rd*rd + rd*rd*b5*rd*rd*rd);

    *xu = (int)(ru*cos(angu) + 0.5);
    *yu = (int)(ru*sin(angu) + 0.5);
}

double ax = -8.1e-06;										//distortion constants at which the fisheye image looks linear*/
void undistort_barrel(int xd, int yd, int* xu, int* yu)		//function to convert image coordinates from distorted fisheye
{															//image to image coordinates from undistorted linear image
	double r2 = xd*xd +yd*yd;	
	*xu = xd/(1+ax*r2);
	*yu = yd/(1+ax*r2);
}

int main(int argc, char const *argv[])
{
	if(argc < 2)
	{
		printf("USAGE ./%s <FILENAME>\n", argv[0]);
		return -1;
	}
	IplImage* frame = cvLoadImage(argv[1]);
	IplImage* undistorted = cvCreateImage(cvSize(2000, 2000), 8, 3);
	for (int i = 0; i < frame->width; ++i)
	{
		for (int j = 0; j < frame->height; ++j)
		{
			int xu, yu;
			undistort_barrel(376 - i, 240 - j, &xu, &yu);
			xu = 376 - xu;
			yu = 240 - yu;
			if(xu < undistorted->width && yu < undistorted->height && xu > 0 && yu > 0)
			{
				returnPixel3C(undistorted, xu, yu, 0) = pixelColor3C(frame, i, j, 0);
				returnPixel3C(undistorted, xu, yu, 1) = pixelColor3C(frame, i, j, 1);
				returnPixel3C(undistorted, xu, yu, 2) = pixelColor3C(frame, i, j, 2);
			}
		}
	}
	IplImage *final = cvCreateImage(cvSize(1000, 1000), 8, 3);
	cvResize(undistorted, final);
	cvShowImage("undistorted", final);
	int c = cvWaitKey();
	if(c == 27)
		return 0;
	if(c == 's')
		cvSaveImage("undistorted.bmp", undistorted);
	return 0;
}