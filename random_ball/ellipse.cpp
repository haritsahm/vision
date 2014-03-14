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

// #include <flycapture/FlyCapture2.h>

using namespace std;
using namespace cvb;
using namespace cv;

char* imgPointer = NULL;
int imgMemPointer;

enum CamError {CAM_SUCCESS = 1, CAM_FAILURE = 0};

CvScalar cColor = CV_RGB(255, 255, 255);
CvScalar cColor1 = CV_RGB(255,0,0);

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

bool getFrame(HIDS hCam, Mat frame)
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
        memcpy(frame.data, imgPointer, 752*480 * 3);
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
        Mat frame = Mat(Size(752,480),CV_8UC3);
        Mat img_hsv = Mat(Size(752,480),CV_8UC3);
        Mat threshy = Mat(Size(752,480),CV_8UC1);
        getFrame(hCam, frame);
        cvtColor(frame,img_hsv,CV_BGR2HSV);
        inRange(img_hsv, Scalar(0, 120, 100), Scalar(255, 255, 255), threshy);
        // smooth(threshy,threshy,CV_MEDIAN,7,7);
        for(int i = 0 ; i < 752 ; i++)
        {
            for(int j = 0 ; j < 480 ; j++)
                {
                    if(threshy.at<uchar>(i,j) == 0)
                        threshy.at<uchar>(i,j) = 255;
                    else
                        threshy.at<uchar>(i,j) = 0;
                }
        }
        
        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;
        findContours( threshy, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
        /// Find the rotated rectangles and ellipses for each contour
        vector<RotatedRect> minRect( contours.size() );
        vector<RotatedRect> minEllipse( contours.size() );
        for( int i = 0; i < contours.size(); i++ )
        { minRect[i] = minAreaRect( Mat(contours[i]) );
            if( contours[i].size() > 5 )
            { minEllipse[i] = fitEllipse( Mat(contours[i]) ); }
        }

        Mat drawing = Mat( frame.size(), CV_8UC3 );
        for( int i = 0; i< contours.size(); i++ )
            {
            Scalar color = Scalar( 255,255,0 );
            // contour
            drawContours( drawing, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
            // ellipse
            ellipse( drawing, minEllipse[i], color, 2, 8 );
            // rotated rectangle
            Point2f rect_points[4]; minRect[i].points( rect_points );
            for( int j = 0; j < 4; j++ )
                line( drawing, rect_points[j], rect_points[(j+1)%4], color, 1, 8 );
            }

        imshow("live",img_hsv);
        imshow("threshy",threshy);
        imshow("drawing",drawing);

        int c = cvWaitKey();
        if( c == 27 )
            break;
        // cvReleaseImage(&frame);
        // cvReleaseImage(&img_hsv);
        // cvReleaseImage(&threshy);
    }
    return 0;
}