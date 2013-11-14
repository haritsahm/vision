#include <iostream>
#include <stdio.h>

//OpenCV Headers
#include <opencv/cv.h>
#include <opencv/highgui.h>
//Input-Output
#include <stdio.h>
//Blob Library Headers
#include <cvblob.h>

#include <flycapture/FlyCapture2.h>

using namespace std;
using namespace cvb;
using namespace cv;

enum CamError {CAM_SUCCESS = 1, CAM_FAILURE = 0};

CvScalar cColor = CV_RGB(255, 255, 255);

int main()
{
    FlyCapture2::Error error;
    FlyCapture2::PGRGuid guid;
    FlyCapture2::BusManager busMgr;
    FlyCapture2::Camera cam;
    FlyCapture2::VideoMode vm;
    FlyCapture2::FrameRate fr;
    FlyCapture2::Image rawImage;

    //Initializing camera
        error = busMgr.GetCameraFromIndex(0, &guid);
    if (error != FlyCapture2::PGRERROR_OK)
    {
        error.PrintErrorTrace();
        return CAM_FAILURE;
    }

          vm = FlyCapture2::VIDEOMODE_640x480Y8;
    fr = FlyCapture2::FRAMERATE_60;

    error = cam.Connect(&guid);
    if (error != FlyCapture2::PGRERROR_OK)
    {
        error.PrintErrorTrace();
        return CAM_FAILURE;
    }

    cam.SetVideoModeAndFrameRate(vm, fr);
    //Starting the capture
    error = cam.StartCapture();
    if (error != FlyCapture2::PGRERROR_OK)
    {
        error.PrintErrorTrace();
               return CAM_FAILURE;
    }

        //Image Variables
        Mat frame (Size(640,480),CV_8UC1);
        Mat img_bgr (Size(640,480),CV_8UC3);
        Mat img_hsv (Size(640,480),CV_8UC3);                        //Image in HSV color space
        Mat threshy (Size(640,480),CV_8UC1);                                //Threshed Image
        // Mat labelImg (Size(640,480),CV_8UC1);        //Image Variable for blobs
        IplImage* histogram= cvCreateImage(cvSize(640,480),8,3);                        //Image histograms
        Mat histograms (Size(640,480),CV_8UC1);         //greyscale image for histogram
        Mat empty (Size(640,480),CV_8UC3);        

        

        // CvBlobs blobs;
        
        while(1)
        {
                cam.RetrieveBuffer(&rawImage);
                memcpy(frame.data, rawImage.GetData(), rawImage.GetDataSize());
                // histogram = empty.clone();
                cvZero(histogram);
                // cvAddS(frame, cvScalar(70,70,70), frame);
                cvtColor(frame,img_bgr,CV_BayerBG2BGR);
                // cout<<"\n1";
                cvtColor(img_bgr,img_hsv,CV_BGR2HSV);        
                // cout<<"\n2";                                        
                //Thresholding the frame for yellow
                inRange(img_hsv, Scalar(20, 100, 20), Scalar(70, 255, 255), threshy);                                        
                // cvInRangeS(img_hsv, cvScalar(0, 120, 100), cvScalar(255, 255, 255), threshy);
                //Filtering the frame - subsampling??
                // smooth(threshy,threshy,CV_MEDIAN,7,7);

                //Finding the blobs
                // unsigned int result=cvLabel(threshy,labelImg,blobs);
                //Filtering the blobs
                // cvFilterByArea(blobs,500,1000);
                //Rendering the blobs
                // cvRenderBlobs(labelImg,blobs,img_bgr,img_bgr);

                CvPoint prev = cvPoint(0,0);

                for(int x=0;x<640;++x)
                {
                        Mat col = threshy.col(x);
                        int y = 480 - countNonZero(col);
                        for(int i=0 ; i<480 ; ++i)
                             histograms.at<uchar>(x,i) = y;
                        CvPoint next = cvPoint(x,y);
                        cvLine(histogram,prev,next,cColor);
                        // cvCircle(histogram,next,2,cColor,3);
                        prev=next;
                }

                //Showing the images
                imshow("Live",img_bgr);
                imshow("Threshed",threshy);
                cvShowImage("Histogram",histogram);

                int c= cvWaitKey(10);

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