#include "balldetector.h"
#include <ueye.h>

// #define USE_UEYE

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
	#ifdef USE_UEYE
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
			cvDilate(img_green, img_green);
			cvSmooth( img_green, img_green, CV_GAUSSIAN, 7, 7 );
			cvNot(img_green, img_green);
			cvCanny(img_green, canny_edge, 1, 200, 5);
			BallDetector b(canny_edge);
			b.findBall();
		}
	#endif
	if(argc < 2)
	{
		printf("USAGE ./%s <FILENAME>\n", argv[0]);
		return -1;
	}
	IplImage *img = cvLoadImage(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
	cvShowImage("img", img);
	BallDetector b(img);
	b.findBall();
	return 0;
}