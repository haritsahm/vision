#include <iostream>
#include <stdio.h>

//OpenCV Headers
#include <opencv/cv.h>
#include <opencv/highgui.h>
//Input-Output
#include <stdio.h>
//Blob Library Headers
#include <cvblob.h>

// #include <flycapture/FlyCapture2.h>

using namespace std;
using namespace cvb;
using namespace cv;

// enum CamError {CAM_SUCCESS = 1, CAM_FAILURE = 0};

CvScalar cColor = CV_RGB(255, 255, 255);

int main()
{
        //Image Variables
        Mat frame;
        Mat img_rgb (Size(752,480),CV_8UC3);
        Mat img_hsv (Size(752,480),CV_8UC3);                        //Image in HSV color space
        Mat threshy (Size(752,480),CV_8UC1);                                //Threshed Image
        // Mat labelImg (Size(752,480),CV_8UC1);        //Image Variable for blobs
        IplImage* histogram= cvCreateImage(cvSize(752,480),8,3);                        //Image histograms
        Mat empty (Size(752,480),CV_8UC3); 
        Mat histograms (Size(752,480),CV_8UC1);         //greyscale image for histogram 

        

        // CvBlobs blobs;
        
        while(1)
        {
                // cam.RetrieveBuffer(&rawImage);
                // memcpy(frame.data, rawImage.GetData(), rawImage.GetDataSize());
                histograms = empty.clone();
                cvZero(histogram);
                frame = imread("ueye.bmp",CV_LOAD_IMAGE_COLOR);
                // cvAddS(frame, cvScalar(70,70,70), frame);
                // cvtColor(frame,img_rgb,CV_BayerBG2BGR);
                // cout<<"\n1";
                cvtColor(frame,img_hsv,CV_RGB2HSV);        
                // cout<<"\n2";                                        
                //Thresholding the frame for yellow
                // inRange(img_hsv, Scalar(20, 100, 20), Scalar(70, 255, 255), threshy);        
                inRange(img_hsv, Scalar(0, 150, 90), Scalar(120, 242, 255), threshy);                                
                // cvInRangeS(img_hsv, cvScalar(0, 120, 100), cvScalar(255, 255, 255), threshy);
                //Filtering the frame - subsampling??
                // smooth(threshy,threshy,CV_MEDIAN,7,7);

                //Finding the blobs
                // unsigned int result=cvLabel(threshy,labelImg,blobs);
                //Filtering the blobs
                // cvFilterByArea(blobs,500,1000);
                //Rendering the blobs
                // cvRenderBlobs(labelImg,blobs,img_rgb,img_rgb);

                CvPoint prev = cvPoint(0,0);
                int max = 0;
                for (int i = 0; i < 752; ++i)
                {
                        Mat col = threshy.col(i);
                        if(480-countNonZero(col)>= max)
                                max = 480-countNonZero(col);
                }

                for(int x=0;x<752;++x)
                {
                        Mat col = threshy.col(x);
                        int i = 480 - countNonZero(col);
                        for(int y=0 ; y<480 ; ++y)
                             histograms.at<uchar>(x,y) = int((i/max)*255);
                        CvPoint next = cvPoint(x,i);
                        cvLine(histogram,prev,next,cColor);
                        // cvCircle(histogram,next,2,cColor,3);
                        prev=next;
                }

                //Showing the images
                imshow("Live",frame);
                imshow("Threshed",threshy);
                cvShowImage("Histogram",histogram);
                imshow("image",histograms);

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