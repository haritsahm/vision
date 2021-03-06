#include <iostream>
#include <stdio.h>

//OpenCV Headers
#include <opencv/cv.h>
#include <opencv/highgui.h>
//Input-Output
#include <stdio.h>
//Blob Library Headers
#include <cvblob.h>
#include <cmath>

#include <ueye.h>
#define returnPixel1C(image, x, y) ((uchar*)(image->imageData + image->widthStep*(y)))[x]

using namespace std;
using namespace cvb;
using namespace cv;

char* imgPointer = NULL;
int imgMemPointer;

// enum CamError {CAM_SUCCESS = 1, CAM_FAILURE = 0};

CvScalar cColor = CV_RGB(255, 255, 255);
CvScalar cColor1 = CV_RGB(255,0,0);

inline uchar pixelColor1C(IplImage* image, int x, int y)
{
    return ((uchar*)(image->imageData + image->widthStep*y))[x];
}

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

int main()
{
         //INITIALIZE CAMERA
        HIDS hCam = 1;
        initializeCam(hCam);
        setImgMem(hCam);

        //Image Variables
        IplImage* frame = cvCreateImage(cvSize(752,480),8,3);
        // Mat img_bgr (Size(752,480),CV_8UC3);
        IplImage* img_hsv = cvCreateImage(cvSize(752,480),8,3); //Image in HSV color space
        IplImage* threshy = cvCreateImage(cvSize(752,480),8,1); //Threshed Image
        // IplImage* labelImg = cvCreateImage(cvSize(752,480),8,1); //Image Variable for blobs
        IplImage* histogram= cvCreateImage(cvSize(752,480),8,3);    //Image histograms
        // IplImage* histograms = cvCreateImage(cvSize(752,480),8,1);   //greyscale image for histogram
        IplImage* empty = cvCreateImage(cvSize(752,1),8,1);   
        IplImage* img = cvCreateImage(cvSize(752,1),8,1);   //image colours (histogram)       

        

        // CvBlobs blobs;
        
        while(1)
        {
            // printf("1\n");
                getFrame(hCam,frame);
                // cam.RetrieveBuffer(&rawImage);
                // memcpy(frame.data, rawImage.GetData(), rawImage.GetDataSize());
                // histogram = empty.clone();
                // img = empty.clone();
                cvZero(img);
                cvZero(histogram);
                // // cvAddS(frame, cvScalar(70,70,70), frame);
                // // cvtColor(frame,img_bgr,CV_BayerBG2BGR);
                // // cout<<"\n1";
                cvCvtColor(frame,img_hsv,CV_BGR2HSV);
                // cvCvtColor(frame,img_hsv,CV_BGR2YUV);        
                // // cout<<"\n2";                                        
                // //Thresholding the frame for yellow
                // // inRange(img_hsv, Scalar(10, 135, 20), Scalar(255, 199, 120), threshy); 
                cvInRangeS(img_hsv, cvScalar(18, 145, 28), cvScalar(43, 255, 137), threshy);
                // cvInRangeS(img_hsv, cvScalar(45, 133, 70), cvScalar(117, 160, 115), threshy);
                cvErode(threshy,threshy);
                cvDilate(threshy,threshy);                                     
                // // cvInRangeS(img_hsv, cvScalar(0, 120, 100), cvScalar(255, 255, 255), threshy);
                // // Filtering the frame - subsampling??
                // cvSmooth(threshy,threshy,CV_MEDIAN,7,7);
                // CvBlobs blobs;
                // // Finding the blobs
                // unsigned int result = cvLabel(threshy,labelImg,blobs);
                // // Filtering the blobs
                // cvFilterByArea(blobs,500,1000);
                // // Rendering the blobs
                // cvRenderBlobs(labelImg,blobs,frame,frame);

                // int gpy = 0;

                // for (CvBlobs::const_iterator it=blobs.begin(); it!=blobs.end(); ++it)
                //     gpy = it->second->maxy;

                CvPoint prev = cvPoint(0,480);
                int max = 0;
                int threshold;

                for(int x=0;x<752;++x)
                {
                    cvSetImageROI(threshy,cvRect(x,0,1,480));
                    // IplImage* copy = cvCreateImage(cvGetSize(threshy),8,1);
                    // cvCopy(threshy,copy,NULL);
                    int y = cvCountNonZero(threshy);
                    cvResetImageROI(threshy);
                    if(max<=y)
                        max=y;
                    returnPixel1C(img,x,0) = y;
                    // CvPoint next = cvPoint(x,480-y);
                }

                threshold = max/5;
                cvLine(histogram,cvPoint(0,480-threshold),cvPoint(752,480-threshold),cColor1);
                cvSmooth(img,img);


                for(int i=0;i<752;++i)
                {
                    int color = returnPixel1C(img,i,0);
                    CvPoint next = cvPoint(i,480-color);
                    cvLine(histogram,prev,next,cColor);
                    prev = next;
                }

                int gpx1 = 0,gpx2 = 0;
                int pixels_at_gpx1 = 0,pixels_at_gpx2 = 0;
                for(int i=0;i<752;++i)
                {
                    if(returnPixel1C(img,i,0)>threshold)
                    {
                        int peak_x = 0;
                        int pixels_at_x = 0;                        
                        while(1)
                        {
                            if(i>=752)
                                break;
                            if(returnPixel1C(img,i,0)<threshold)
                                break;
                            if(returnPixel1C(img,i,0)>pixels_at_x)
                            {
                                peak_x = i;
                                pixels_at_x = returnPixel1C(img,i,0);
                            }
                            ++i;
                        }
                        if(pixels_at_x >= pixels_at_gpx1)     //COMPARING COLOR SO THAT ONLY THE TWO PEAKS WITH MAX WHITE REGION ARE CONSIDERED AS GOAL POSTS
                        {
                            gpx2 = gpx1;
                            pixels_at_gpx2 = pixels_at_gpx1;
                            gpx1 = peak_x;
                            pixels_at_gpx1 = pixels_at_x;
                        }

                        else if (pixels_at_x < pixels_at_gpx1 && pixels_at_x >=pixels_at_gpx2)
                        {
                            gpx2 = peak_x;
                            pixels_at_gpx2 = pixels_at_x;
                        }
                    }
                }
                cvLine(frame,cvPoint(gpx1,0),cvPoint(gpx1,480),cColor1);
                cvLine(frame,cvPoint(gpx2,0),cvPoint(gpx2,480),cColor1);
                // printf("%d\n", gpx1);
                // printf("%d\n", gpx2);
                // printf("1\n");
                //CONVERTING INTO TWO SEPARATE BLOBS
                cvSetImageROI(threshy,cvRect(int((3*gpx1+gpx2)/4),0,int(abs(gpx1-gpx2)/10),480));
                cvZero(threshy);
                cvResetImageROI(threshy);

                //Showing the images
                cvShowImage("Live",frame);
                cvShowImage("Threshed",threshy);
                cvShowImage("img",img);
                cvShowImage("Histogram",histogram);

                int c= cvWaitKey(50);
                if(c == 27)
                    break;
        }
        exitCam(hCam);
        // Cleanup
        cvReleaseImage(&frame);
        cvReleaseImage(&threshy);
        cvReleaseImage(&img_hsv);
        // cvReleaseImage(&labelImg);
        cvReleaseImage(&histogram);
        cvDestroyAllWindows();
        return 0;
}