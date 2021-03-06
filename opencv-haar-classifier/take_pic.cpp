#include <inttypes.h>
#include <stdio.h>
#include <iostream>
#include <sstream>  //include this to use string streams
#include <string>
#include <stdlib.h>
#include <ueye.h>

#include <cvblob.h>
#include <opencv/cv.h>
#include <opencv/highgui.h> 

using namespace std;
using namespace cvb;
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

int main()
{
	//INITIALIZE CAMERA
    HIDS hCam = 1;
    initializeCam(hCam);
    setImgMem(hCam);
    IplImage* bgr_frame=cvCreateImage(cvSize(752, 480), 8, 3);      //fisheye image
    int pic =1;
    while(1)
    {
    	getFrame(hCam,bgr_frame);
        cvLine(bgr_frame,cvPoint(356,220),cvPoint(396,220),cvScalar(255,0,0));
        cvLine(bgr_frame,cvPoint(356,260),cvPoint(396,260),cvScalar(255,0,0));
        cvLine(bgr_frame,cvPoint(356,220),cvPoint(356,260),cvScalar(255,0,0));
        cvLine(bgr_frame,cvPoint(396,220),cvPoint(396,260),cvScalar(255,0,0));
    	char imgcounter[50]="negative_images/";
		string picnumber = static_cast<ostringstream*>( &(ostringstream() << pic) )->str();
		const char * d = picnumber.c_str();
		strcat(imgcounter,d);            
		strcat(imgcounter,".bmp");
        CvFont font;
        cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.4, 0.4, 0, 1, 8);
        cvPutText(bgr_frame,imgcounter,cvPoint(10,10),&font,cvScalar(255,255,0));
        cvShowImage("LIVE",bgr_frame);

		int c = cvWaitKey(100);
		if(c == int('s'))
		{
            cvSetImageROI(bgr_frame,cvRect(356,220,40,40));
			cvSaveImage(imgcounter,bgr_frame);
            cvResetImageROI(bgr_frame);
            printf("SAVED %d\n",pic);
			++pic;
		}
        if(c == 27)
        {
            exitCam(hCam);
            break;
        }
    }
    return 0;
}