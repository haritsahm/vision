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

// enum CamError {CAM_SUCCESS = 1, CAM_FAILURE = 0};

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

void getFrame(HIDS hCam, Mat frame)
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
    memcpy(frame.data, imgPointer, 752*480 * 3);
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
        Mat frame (Size(752,480),CV_8UC3);
        Mat img_bgr (Size(752,480),CV_8UC3);
        Mat img_hsv (Size(752,480),CV_8UC3);                        //Image in HSV color space
        Mat threshy (Size(752,480),CV_8UC1);                                //Threshed Image
        // Mat labelImg (Size(752,480),CV_8UC1);        //Image Variable for blobs
        IplImage* histogram= cvCreateImage(cvSize(752,480),8,3);                        //Image histograms
        Mat histograms (Size(752,480),CV_8UC1);         //greyscale image for histogram
        Mat empty (Size(752,1),CV_8UC1);   
        Mat img (Size(752,1),CV_8UC1);               //image colours (histogram)       

        

        // CvBlobs blobs;
        
        while(1)
        {
                getFrame(hCam,frame);
                // cam.RetrieveBuffer(&rawImage);
                // memcpy(frame.data, rawImage.GetData(), rawImage.GetDataSize());
                // histogram = empty.clone();
                img = empty.clone();
                cvZero(histogram);
                // cvAddS(frame, cvScalar(70,70,70), frame);
                // cvtColor(frame,img_bgr,CV_BayerBG2BGR);
                // cout<<"\n1";
                cvtColor(frame,img_hsv,CV_BGR2HSV);        
                // cout<<"\n2";                                        
                //Thresholding the frame for yellow
                // inRange(img_hsv, Scalar(10, 135, 20), Scalar(255, 199, 120), threshy); 
                inRange(img_hsv, Scalar(0, 150, 90), Scalar(120, 242, 255), threshy);                                       
                // cvInRangeS(img_hsv, cvScalar(0, 120, 100), cvScalar(255, 255, 255), threshy);
                //Filtering the frame - subsampling??
                // smooth(threshy,threshy,CV_MEDIAN,7,7);

                //Finding the blobs
                // unsigned int result=cvLabel(threshy,labelImg,blobs);
                //Filtering the blobs
                // cvFilterByArea(blobs,500,1000);
                //Rendering the blobs
                // cvRenderBlobs(labelImg,blobs,img_bgr,img_bgr);

                CvPoint prev = cvPoint(0,480);
                int max = 0;
                int threshold;

                for(int x=0;x<752;++x)
                {
                        Mat col = threshy.col(x);
                        int y = countNonZero(col);
                        if(max<y)
                            max=y;
                        // for(int j=0 ;j<img.rows;++j)
                            img.at<uchar>(0,x) = y;
                        CvPoint next = cvPoint(x,480-y);
                        // cvLine(histogram,prev,next,cColor);
                        // // cvCircle(histogram,next,2,cColor,3);
                        // prev=next;
                }

                threshold = max/5;
                cvLine(histogram,cvPoint(0,480-threshold),cvPoint(752,480-threshold),cColor1);

                GaussianBlur(img, img, Size(5, 5), 1.2, 1.2);


                for(int i=0;i<752;++i)
                {
                    int color = img.at<uchar>(0,i);
                    CvPoint next = cvPoint(i,480-color);
                    cvLine(histogram,prev,next,cColor);
                    prev = next;
                }
                // printf("%n\n", &threshold);
                for(int i=0;i<752;++i)
                {
                    if(img.at<uchar>(0,i)>threshold)
                    {
                        int peak = 0;
                        int color = 0;
                        while(1)
                        {
                            if(img.at<uchar>(0,i)<threshold)
                                break;
                            if(img.at<uchar>(0,i)>color)
                            {
                                peak = i;
                                color = img.at<uchar>(0,i);
                            }
                            ++i;
                        }
                        line(frame,cvPoint(peak,0),cvPoint(peak,480),cColor1);
                    }
                }

                //Showing the images
                imshow("Live",frame);
                imshow("Threshed",threshy);
                imshow("img",img);
                cvShowImage("Histogram",histogram);

                int c= cvWaitKey(50);

                if(c == 27)
                    break;
        }

        //Cleanup
        // cvReleaseImage(&frame);
        // cvReleaseImage(&threshy);
        // cvReleaseImage(&img_hsv);
        // cvReleaseImage(&labelImg);
        // cvReleaseImage(&histogram);
        cvDestroyAllWindows();
        return 0;
}