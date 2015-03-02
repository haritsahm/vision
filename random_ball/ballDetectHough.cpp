#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <algorithm>

#include <opencv2/opencv.hpp>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cvaux.h>

#include <ueye.h>

using namespace std;
using namespace cv;

char* imgPointer = NULL;
int imgMemPointer;

int initializeCam(HIDS hCam)
{	
	char* errMsg = (char*)malloc(sizeof(char)*200);
	int err = 0;

	int nRet = is_InitCamera (&hCam, NULL);
	if (nRet != IS_SUCCESS)
	{
		is_GetError (hCam, &err, &errMsg);
		printf("Camera Init Error %d: %s\n",err,errMsg);
		return EXIT_FAILURE;
	}

	nRet = is_SetColorMode(hCam, IS_CM_BGR8_PACKED);
    if (nRet != IS_SUCCESS) 
    {
         	is_GetError (hCam, &err, &errMsg);
			printf("Color Mode Error %d: %s\n",err,errMsg);
			return EXIT_FAILURE;  
    }

    nRet = is_SetHardwareGain(hCam, 100, 4, 0, 13);
    if (nRet != IS_SUCCESS) 
    {
         	is_GetError (hCam, &err, &errMsg);
			printf("Hardware Gain Error %d: %s\n",err,errMsg);
			return EXIT_FAILURE;
    }

	return EXIT_SUCCESS;
}

int setImgMem(HIDS hCam)
{
	char* errMsg = (char*)malloc(sizeof(char)*200);
	int err = 0;

	int nRet = is_AllocImageMem(hCam, 752, 480, 24, &imgPointer, &imgMemPointer);
	if(nRet != IS_SUCCESS)
	{
		is_GetError (hCam, &err, &errMsg);
		printf("MemAlloc Unsuccessful %d: %s\n",err,errMsg);
		return EXIT_FAILURE;
	}

	nRet = is_SetImageMem (hCam, imgPointer, imgMemPointer);
	if(nRet != IS_SUCCESS)
	{
		is_GetError (hCam, &err, &errMsg);
		printf("Could not set/activate image memory %d: %s\n",err, errMsg);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void getFrame(HIDS hCam, IplImage* frame)
{
	char* errMsg = (char*)malloc(sizeof(char)*200);
	int err = 0;


	int nRet = is_FreezeVideo (hCam, IS_WAIT) ;
	if(nRet != IS_SUCCESS)
	{
		is_GetError (hCam, &err, &errMsg);
		printf("Could not grab image %d: %s\n",err,errMsg);
		//return EXIT_FAILURE;
	}
		
	//fill in the OpenCV imaga data 
	//IplImage* img_rgb = cvCreateImage(cvSize(752,480),8,3);
	memcpy(frame->imageData, imgPointer, 752*480 * 3);
	//img_rgb->imageData = imgPointer;
	//return img_rgb;
}

int exitCam(HIDS hCam)
{
	int	nRet = is_ExitCamera (hCam);
	if(nRet != IS_SUCCESS)
	{
		printf("Could not exit camera \n");
		return EXIT_FAILURE;
	}
}

int main(int argc, char const *argv[])
{
	//INITIALIZE CAMERA
	HIDS hCam = 1;
	initializeCam(hCam);
	setImgMem(hCam);

	IplImage *frame = cvCreateImage(cvSize(752, 480), 8, 3);
	IplImage *img_gray = cvCreateImage(cvSize(752, 480), 8, 1);
	IplImage *canny_edge = cvCreateImage(cvSize(752, 480), 8, 1);
	IplImage *img_green = cvCreateImage(cvSize(752, 480), 8, 1);
	IplImage *img_hsv = cvCreateImage(cvSize(752, 480), 8, 3);

	while(1)
	{
		getFrame(hCam, frame);
		cvCvtColor(frame, img_gray, CV_BGR2GRAY);
		cvCvtColor(frame, img_hsv, CV_BGR2HSV);
		cvInRangeS(img_hsv, cvScalar(50, 49, 8), cvScalar(79, 130, 96), img_green);
		// cvGaussianBlur(img_green, img_green, cvSize(9, 9), 2, 2);
		cvDilate(img_green, img_green);
		cvSmooth( img_green, img_green, CV_GAUSSIAN, 7, 7 );
		// cvFloodFill(img_green, cvPoint(376, 240), CV_RGB(255,255,255));
		// cvNot(img_green, img_green);
		cvCanny(img_green, canny_edge, 1, 200, 5);
		cvSetImageROI(img_green, cvRect(40, 40, 500, 400));
		CvMemStorage* storage = cvCreateMemStorage(0);
		CvSeq* circles = cvHoughCircles(img_green, storage, CV_HOUGH_GRADIENT, 1, 100, 100, 20,10,40);
		for (int i = 0; i < circles->total; i++)
	    {
	         // round the floats to an int
	         float* p = (float*)cvGetSeqElem(circles, i);
	         cv::Point center(cvRound(p[0] + 40), cvRound(p[1]) + 40);
	         int radius = cvRound(p[2]);

	         // draw the circle center
	         cvCircle(frame, center, 3, CV_RGB(0,255,0), -1, 8, 0 );

	         // draw the circle outline
	         cvCircle(frame, center, radius+1, CV_RGB(0,0,255), 2, 8, 0 );
	    }
	    cvShowImage("img_green", img_green);
		cvShowImage("canny_edge", canny_edge);
		cvShowImage("frame", frame);
		int c = cvWaitKey(10);
		if(c == 27)
			exit(0);
		if(c == 's' || c == 'S')
			cvSaveImage("test5.bmp", canny_edge);
		cvResetImageROI(img_green);
	}
	return 0;
}

// void HoughCircles(InputArray image, OutputArray circles, int method, double dp, double minDist, double param1=100, double param2=100, int minRadius=0, int maxRadius=0 )
//  Parameters:
// image – 8-bit, single-channel, grayscale input image.
// circles – Output vector of found circles. Each vector is encoded as a 3-element floating-point vector (x,y,radius) .
// circle_storage – Memory storage that will contain the output sequence of found circles.
// method – Currently, the only implemented method is CV_HOUGH_GRADIENT.
// dp – Inverse ratio of the accumulator resolution to the image resolution. For example, if dp=1 , the accumulator has the same resolution as the input image. If dp=2 , the accumulator has half as big width and height.
// minDist – Minimum distance between the centers of the detected circles. If the parameter is too small, multiple neighbor circles may be falsely detected in addition to a true one. If it is too large, some circles may be missed.
// param1 – First method-specific parameter. In case of CV_HOUGH_GRADIENT , it is the higher threshold of the two passed to the Canny()edge detector (the lower one is twice smaller).
// param2 – Second method-specific parameter. In case of CV_HOUGH_GRADIENT , it is the accumulator threshold for the circle centers at the detection stage. The smaller it is, the more false circles may be detected. Circles, corresponding to the larger accumulator values, will be returned first.
// minRadius – Minimum circle radius.
// maxRadius – Maximum circle radius.