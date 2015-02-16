#include <cstdio>
#include <iostream>
#include <vector>

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
    HIDS hCam = 1;
    initializeCam(hCam);
    setImgMem(hCam);
    IplImage *frame = cvCreateImage(cvSize(752, 480), 8, 3);
    IplImage *img_hsv = cvCreateImage(cvSize(752, 480), 8, 3);
    IplImage *threshy = cvCreateImage(cvSize(752, 480), 8, 1);
    int hl = 0;
    int hh = 255;
    int sl = 0;
    int sh = 255;
    int vl = 0;
    int vh = 255;
    cvNamedWindow("sliders", CV_WINDOW_NORMAL);
    cvCreateTrackbar("Hl","sliders",&hl,255,0);
    cvCreateTrackbar("Hh","sliders",&hh,255,0);
    cvCreateTrackbar("Sl","sliders",&sl,255,0);
    cvCreateTrackbar("Sh","sliders",&sh,255,0);
    cvCreateTrackbar("Vl","sliders",&vl,255,0);
    cvCreateTrackbar("Vh","sliders",&vh,255,0);
    while(1)
    {
        getFrame(hCam, frame);
        cvCvtColor(frame, img_hsv, CV_BGR2HSV);
        cvInRangeS(img_hsv, cvScalar(hl, sl, vl), cvScalar(hh, sh, vh), threshy);
        cvShowImage("threshy", threshy);
        int c = cvWaitKey(10);
        if(c == 27)
            break;
        if(c == 'd' || c == 'D')
            printf("hl: %d\nhh: %d\nsl: %d\nsh: %d\nvl: %d\nvh: %d\n", hl, hh, sl, sh, vl, vh);
    }
    return 0;
}