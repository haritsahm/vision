#include <cstdio>
#include <iostream>
#include <vector>
#include <algorithm>

#include <ueye.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

using namespace std;
using namespace cv;

#define returnPixel1C(image, x, y) ((uchar*)(image->imageData + image->widthStep*(y)))[x]

inline uchar pixelColor1C(IplImage* image, int x, int y)
{
    return ((uchar*)(image->imageData + image->widthStep*y))[x];
}

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
    int nRet = is_ExitCamera (hCam);
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
	IplImage *threshy = cvCreateImage(cvSize(752, 480), 8, 1);
	IplImage *img_hsv = cvCreateImage(cvSize(752, 480), 8, 3);
	IplImage *img_hist_x = cvCreateImage(cvSize(752, 480), 8, 3);
	IplImage *img_hist_y = cvCreateImage(cvSize(752, 480), 8, 3);
    IplImage *layer_mask = cvCreateImage(cvSize(752, 480), 8, 1);
	while(1)
	{
        int max_x = 0;
        int max_y = 0;
        cvZero(img_hist_y);
        cvZero(img_hist_x);
        cvZero(layer_mask);
		getFrame(hCam, frame);
		cvCvtColor(frame, img_hsv, CV_BGR2HSV);
		cvInRangeS(img_hsv, cvScalar(30, 60, 23), cvScalar(76, 105, 64), threshy);
		vector<int> hist_x(752, 0);
		vector<int> hist_y(480, 0);
		for (int i = 0; i < 752; ++i)
		{
			cvSetImageROI(threshy, cvRect(i, 0, 1, 480));
			hist_x[i] = cvCountNonZero(threshy);
			cvResetImageROI(threshy);
		}
		for (int i = 0; i < 480; ++i)
		{
			cvSetImageROI(threshy, cvRect(0, i, 752, 1));
			hist_y[i] = cvCountNonZero(threshy);
			cvResetImageROI(threshy);
		}
		CvPoint prev = cvPoint(0, 480);
		for (int i = 0; i < 752; ++i)
		{
			CvPoint next = cvPoint(i, 480 - hist_x[i]);
			cvLine(img_hist_x, prev, next, cvScalar(255, 255, 255));
			prev = next;
            max_x = max(max_x, hist_x[i]);
		}
		prev = cvPoint(0, 0);
		for (int i = 0; i < 480; ++i)
		{
			CvPoint next = cvPoint(hist_y[i], i);
			cvLine(img_hist_y, prev, next, cvScalar(255, 255, 255));
			prev = next;
            max_y = max(max_y, hist_y[i]);
		}
        int threshold_x = max_x/5;
        int threshold_y = max_y/5;
        for (int i = 0; i < 752; ++i)
        {
            for (int j = 0; j < 480; ++j)
            {
                if(returnPixel1C(threshy, i, j) == 255)
                    returnPixel1C(layer_mask, i, j) = 255;
                else if(hist_x[i] >= threshold_x && hist_y[j] >= threshold_y)
                    returnPixel1C(layer_mask, i, j) = 255;
            }
        }
        cvShowImage("threshy", threshy);
		cvShowImage("img_hist_x", img_hist_x);
		cvShowImage("img_hist_y", img_hist_y);
        cvShowImage("layer_mask", layer_mask);
		cvWaitKey(10);
	}
	return 0;
}


