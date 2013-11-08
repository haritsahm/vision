#include <iostream>
#include <stdio.h>

#include <cvblob.h>
#include <opencv/cv.h>
#include <opencv/highgui.h> 

#include <ueye.h>

using namespace std;
using namespace cvb;
using namespace cv;

// #define HEIGHT 752
// #define WIDTH 752
#define PI 3.14159265359
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

    int nRet = is_AllocImageMem(hCam, 752, 752, 24, &imgPointer, &imgMemPointer);
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

void getFrame(HIDS hCam,IplImage* frame)
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
    //IplImage* img_rgb = cvCreateImage(cvSize(752,752),8,3);
    memcpy(frame->imageData, imgPointer, 752*752 * 3);
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

int main()
{

	// //INITIALIZE CAMERA
 //    HIDS hCam = 1;
 //    initializeCam(hCam);
 //    setImgMem(hCam);

	while(1)
	{

		IplImage* frame= cvLoadImage("ueye_corrected.bmp");                      //Picture Frame
		IplImage* img_hsv=cvCreateImage(cvSize(752, 752), 8, 3);				//Image in HSV color space
		IplImage* threshy = cvCreateImage(cvSize(752, 752), 8, 1);				//Threshed Image
		IplImage* labelImg=cvCreateImage(cvSize(752, 752),IPL_DEPTH_LABEL,1);	//Image Variable for blobs

		int xd, yd, xd1, xd2, yd1, yd2;				//Goalpost Coordinates

		// //Getting the current frame
		// getFrame(hCam, frame);
		// //If failed to get break the loop
		// if(!frame)
		// 	break;

		cvCvtColor(frame,img_hsv,CV_BGR2HSV);					
		//Thresholding the frame for yellow
		// cvInRangeS(img_hsv, cvScalar(20, 100, 20), cvScalar(30, 255, 255), threshy);					
		cvInRangeS(img_hsv, cvScalar(0, 120, 100), cvScalar(255, 255, 255), threshy);
		//Filtering the frame - subsampling??
		cvSmooth(threshy,threshy,CV_MEDIAN,7,7);

		CvBlobs blobs;

		//Finding the blobs
		unsigned int result = cvLabel(threshy,labelImg,blobs);
		//Filtering the blobs
		cvFilterByArea(blobs,100,100000);
		//Rendering the blobs
		cvRenderBlobs(labelImg,blobs,frame,frame);

		for (CvBlobs::const_iterator it=blobs.begin(); it!=blobs.end(); ++it)
		{			
			xd1 = it->second->minx;
			yd1 = it->second->maxy;

			xd2 = it->second->maxx;
			yd2 = it->second->maxy;

           xd = (it->second->maxx+it->second->minx)/2;
           yd = (it->second->maxy+it->second->miny)/2;

			cvCircle(frame,cvPoint(xd1,yd1),2,CV_RGB(255,0,0),3);
			cvCircle(frame,cvPoint(xd2,yd2),2,CV_RGB(255,0,0),3);
            cvCircle(frame,cvPoint(xd,yd),2,CV_RGB(255,0,0),3);

			xd1 = xd1 - 752/2;
			yd1 = -yd1 + 752/2;

			xd2 = xd2 - 752/2;
			yd2 = -yd2 + 752/2;
			
			cout<<"\nnon-linear coords: xd1="<<xd1<<"     yd1="<<yd1;
			cout<<"\nnon-linear coords: xd2="<<xd2<<"     yd2="<<yd2;
			cout<<"\n\n\n\n";
		}

		//Showing the images
		cvShowImage("Live",frame);
		cvShowImage("Threshed",threshy);

		int c = cvWaitKey(10);

		if(c == 27)
			break;

		cvReleaseImage(&frame);
		cvReleaseImage(&threshy);
		cvReleaseImage(&img_hsv);
		cvReleaseImage(&labelImg);
	}

	//Cleanup
	cvDestroyAllWindows();
	// exitCam(hCam);
	return 0;
}