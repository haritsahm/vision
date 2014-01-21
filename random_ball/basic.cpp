#include <iostream>
#include <stdio.h>

//OpenCV Headers
#include <opencv/cv.h>
#include <opencv/highgui.h>
//Input-Output
#include <stdio.h>
//Blob Library Headers
#include <cvblob.h>

#include <ueye.h>

using namespace std;
using namespace cvb;
using namespace cv;

char* imgPointer = NULL;
int imgMemPointer;

enum CamError {CAM_SUCCESS = 1, CAM_FAILURE = 0};

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
        return CAM_FAILURE;
    }

    nRet = is_SetImageMem (hCam, imgPointer, imgMemPointer);
    if(nRet != IS_SUCCESS)
    {
        is_GetError (hCam, &err, &errMsg);
        printf("Could not set/activate image memory %d: %s\n",err, errMsg);
        return CAM_FAILURE;
    }

    return EXIT_SUCCESS;
}

bool getFrame(HIDS hCam, IplImage* frame)
{
    char* errMsg = (char*)malloc(sizeof(char)*200);
    int err = 0;


    int nRet = is_FreezeVideo (hCam, IS_WAIT) ;
    if(nRet != IS_SUCCESS)
    {
        is_GetError (hCam, &err, &errMsg);
        printf("Could not grab image %d: %s\n",err,errMsg);
        return CAM_FAILURE;
    }
    else
    {
        memcpy(frame->imageData, imgPointer, 752*480 * 3);
        return CAM_SUCCESS;  
    }      
    //fill in the OpenCV imaga data 
    //IplImage* img_rgb = cvCreateImage(cvSize(752,480),8,3)
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
typedef unsigned char uchar;
#define returnPixel1C(image, x, y) ((uchar*)(image->imageData + image->widthStep*(y)))[x]

inline uchar pixelColor1C(IplImage* image, int x, int y)
{
    return ((uchar*)(image->imageData + image->widthStep*y))[x];
}

int main()
{
     //INITIALIZE CAMERA
    HIDS hCam = 1;
    initializeCam(hCam);
    setImgMem(hCam);

    while(1)
    {
        IplImage* frame = cvCreateImage(cvSize(752,480),8,3);
        IplImage* img_hsv = cvCreateImage(cvSize(752,480),8,3);
        IplImage* threshy = cvCreateImage(cvSize(752,480),8,1);
        getFrame(hCam, frame);
        cvCvtColor(frame,img_hsv,CV_BGR2HSV);
        cvInRangeS(img_hsv, cvScalar(0, 120, 100), cvScalar(255, 255, 255), threshy);
        cvSmooth(threshy,threshy,CV_MEDIAN,7,7);
        for(int i = 0 ; i < 752 ; i++)
        {
            for(int j = 0 ; j < 480 ; j++)
                {
                    if(returnPixel1C(threshy,i,j) == 0)
                        returnPixel1C(threshy,i,j) = 255;
                    else
                        returnPixel1C(threshy,i,j) = 0;
                }
        }
        CvMemStorage* storage = cvCreateMemStorage(0);
        CvSeq* results = cvHoughCircles(threshy,storage,CV_HOUGH_GRADIENT,1,threshy->width/10,100,20,0,100);
        for( int i = 0; i < results->total; i++ ) 
        {
            float* p = (float*) cvGetSeqElem( results, i );
            CvPoint pt = cvPoint( cvRound( p[0] ), cvRound( p[1] ) );
            cvCircle(frame,pt,5,cvScalar(255,255,0),2,8);
            cvCircle(frame,pt,cvRound( p[2] ),CV_RGB(0,0,255),CV_FILLED);
            printf("FOUND\n");
        }



        cvShowImage("frame",frame);
        cvShowImage("threshed",threshy);
        int c = cvWaitKey(10);
        if( c == 27 )
            break;
        cvReleaseImage(&frame);
        cvReleaseImage(&img_hsv);
        cvReleaseImage(&threshy);
    }
    return 0;
}
