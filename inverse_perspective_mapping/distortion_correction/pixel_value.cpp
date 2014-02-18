#include <inttypes.h>
// #include <ftdi.h>
#include <unistd.h>	//for usleep() function
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <sstream>  //include this to use string streams
#include <string>
#include <stdlib.h>

#include <cvblob.h>
#include <opencv/cv.h>
#include <opencv/highgui.h> 

#include <ueye.h>

using namespace std;
using namespace cvb;
using namespace cv;

// #define HEIGHT 480
// #define WIDTH 752
#define PI 3.14159265359
IplImage* bgr_frame;//fisheye image
IplImage* bgr_copy = cvCreateImage(cvSize(752, 480), 8, 3);

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
	// cvZero(bgr_copy);
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5);
	char A[50];
	if  ( event == EVENT_LBUTTONDOWN )
	{
	  // cout << "Left button of the mouse is clicked - position (" << x - 376 << ", " << 240 - y << ")" << endl;
	  cvCopy(bgr_frame,bgr_copy);
	  cvCircle(bgr_copy,cvPoint(x,y),2,CV_RGB(255,0,0),3);
	  sprintf(A,"(%d,%d)",x - 376 ,240 - y);
	  cvPutText(bgr_copy, A, cvPoint(x+10,y+10), &font, CV_RGB(255,0,0));
	}
	else if  ( event == EVENT_RBUTTONDOWN )
	{
		// cout << "Right button of the mouse is clicked - position (" << x - 376 << ", " << 240 - y << ")" << endl;
	    cvCopy(bgr_frame,bgr_copy);
	    cvCircle(bgr_copy,cvPoint(x,y),2,CV_RGB(255,0,0),3);
	    sprintf(A,"(%d,%d)",x - 376 ,240 - y);
	  	cvPutText(bgr_copy, A, cvPoint(x+10,y+10), &font, CV_RGB(255,0,0));
	}
	else if  ( event == EVENT_MBUTTONDOWN )
	{
	    // cout << "Middle button of the mouse is clicked - position (" << x - 376 << ", " << 240 - y << ")" << endl;
	    cvCopy(bgr_frame,bgr_copy);
	    cvCircle(bgr_copy,cvPoint(x,y),2,CV_RGB(255,0,0), 3);
	    sprintf(A,"(%d,%d)",x - 376 ,240 - y);
	  	cvPutText(bgr_copy, A, cvPoint(x+10,y+10), &font, CV_RGB(255,0,0));
	}
	// else if ( event == EVENT_MOUSEMOVE )
	// {
	//      cout << "Mouse move over the window - position (" << x << ", " << y << ")" << endl;

	// }
}

main(void)
{
	printf("ENTER IMAGE NAME (*.bmp): ");
	char img_name[50];
	scanf("%s",img_name);
	bgr_frame = cvLoadImage(img_name);
	// cvZero(bgr_copy);
	cvCopy(bgr_frame,bgr_copy);
	// double Lx = 63.0,Ly = 63.0;
	// double f = 1.18857;
	// double dx = 0.000005538, dy = 0.000006;
	// double D = 520.0;

	while(1)
	{
		// cvNamedWindow("frame");
		setMouseCallback("frame",CallBackFunc,NULL);
		cvShowImage("frame",bgr_copy);
		char c = cvWaitKey(10);
		if(c == 27)
			break;
	}
}