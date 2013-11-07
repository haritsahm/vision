//OpenCV Headers
#include <opencv/cv.h>
#include <opencv/highgui.h>
//Input-Output
#include <stdio.h>
//Blob Library Headers
#include <cvblob.h>

#include <ueye.h>
#include <ftdi.h>
#include <fstream>
//Definitions
#define WIDTHd 752
#define HEIGHTd 480
#define HEIGHTu 752
#define WIDTHu 480
#define PI 3.14159265359

//NameSpaces
using namespace std;
using namespace cvb;
//using namespace std;

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

double ax = -8.1e-06;
void getLinearCoords(int xd, int yd, int* xu, int* yu)
{
	double r2 = xd*xd +yd*yd;	
	*xu = xd/(1+ax*r2);
	*yu = yd/(1+ax*r2);
}


int main()
{
	//INITIALIZE CAMERA
	HIDS hCam = 1;
	initializeCam(hCam);
	setImgMem(hCam);

	//Windows
	cvNamedWindow("Live",CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Threshed", CV_WINDOW_AUTOSIZE);

	while(1)
		{

			//Image Variables
			IplImage* frame=cvCreateImage(cvSize(752, 480), 8, 3);//fisheye image
			IplImage* img_hsv=cvCreateImage(cvSize(752, 480), 8, 3);//Image in HSV color space
			IplImage* threshy = cvCreateImage(cvSize(752, 480), 8, 1);
			IplImage* labelImg=cvCreateImage(cvSize(752, 480),IPL_DEPTH_LABEL,1);//Image Variable for blobs

			CvBlobs blobs;

			int xu, yu;		//coordinates of undistorted image
			int xd, yd;		//coordinates in distorted image

			//Getting the current frame
			getFrame(hCam, frame);
			//If failed to get break the loop
			if(!frame)
				break;

			cvCvtColor(frame,img_hsv,CV_BGR2HSV);
			//Thresholding the frame for yellow
			cvInRangeS(img_hsv, cvScalar(20, 100, 20), cvScalar(30, 255, 255), threshy);
			// cvInRangeS(img_hsv, cvScalar(0,0,0), cvScalar(16,255,255), threshy);
			// cvInRangeS(img_hsv, cvScalar(0,0,0),cvScalar(16,255,255), threshy);//cvScalar(0, 120, 40), cvScalar(255, 255, 255)
			//Filtering the frame
			cvSmooth(threshy,threshy,CV_MEDIAN,7,7);
			//Finding the blobs
			unsigned int result=cvLabel(threshy,labelImg,blobs);
			//Filtering the blobs
			cvFilterByArea(blobs,100,10000);
			//Rendering the blobs
			cvRenderBlobs(labelImg,blobs,frame,frame);

			for (CvBlobs::const_iterator it=blobs.begin(); it!=blobs.end(); ++it)
			{
				xd =(it->second->maxx + it->second->minx )/2;
				// yd =(it->second->maxy + it->second->miny )/2;
				yd =(it->second->maxy);

				cvCircle(frame,cvPoint(xd,yd),2,CV_RGB(255,0,0),3);
				
				xd = xd - 752/2;
				yd = -yd + 480/2;

				cout<<"\nnon-linear coords: xd="<<xd<<"     yd="<<yd<<endl;

				getLinearCoords(xd, yd, &xu, &yu);
				cout<<"\nlinear coords: x="<<xu<<"     y="<<yu<<endl;			
				// getPt(xu, yu,current.angle,current.focal,current.pix2cmy,current.s_view_compensation);
			}
			//Showing the images
			cvShowImage("Live",frame);
			cvShowImage("Threshed",threshy);

			char c=cvWaitKey(10);

			if(int(c) == 27)
				break;

			cvReleaseImage(&frame);
			cvReleaseImage(&threshy);
			cvReleaseImage(&img_hsv);
			cvReleaseImage(&labelImg);
		}

	//Cleanup
	cvDestroyAllWindows();
	exitCam(hCam);
	return 0;
}