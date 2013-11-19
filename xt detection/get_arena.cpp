#include <iostream>
#include <stdio.h>

//OpenCV Headers
#include <opencv/cv.h>
#include <opencv/highgui.h>
//Input-Output
#include <stdio.h>

using namespace std;
// using namespace cvb;
using namespace cv;

CvScalar white = CV_RGB(255, 255, 255);

int main()
{
	//Image Variables
    Mat frame = imread("arena.bmp");
    Mat output (Size(752,480),CV_8UC3);
    Mat img_yuv (Size(752,480),CV_8UC3);                        //Image in HSV color space
    Mat threshy (Size(752,480),CV_8UC1);                                //Threshed Image
    Mat thresh (Size(752,480),CV_8UC1);    
    cvtColor(frame,img_yuv,CV_BGR2YUV);   
    inRange(img_yuv, Scalar(9, 0, 75), Scalar(72, 124, 128), threshy);  
    inRange(img_yuv, Scalar(76, 97, 111), Scalar(215, 135, 133), thresh);  
    output = frame.clone(); 
    // erode(threshy, threshy);
    // dilate(threshy, threshy);  
    // GaussianBlur(threshy, threshy, Size(7,7), 1.5, 1.5); 
    Mat kernel = Mat::ones(Size(1, 1), CV_8U);
    erode(threshy, threshy, kernel);
    dilate(threshy, threshy, 0, Point(-1, -1), 2, 1, 1);

    for(int i=0;i<752;++i)
    {
    	for(int j=0;j<480;++j)
    	{
    		if(threshy.at<uchar>(j,i) == 0 )//&& thresh.at<uchar>(j,i) == 0)
    		{
    			// img_yuv.at<uchar>(j, i) = 0;
    			// img_yuv.at<Vec3b> (j,i) [3]=0; 
    			output.data[output.channels()*(output.cols*j + i) + 0] = 0;
           		output.data[output.channels()*(output.cols*j + i) + 1] = 0;
           		output.data[output.channels()*(output.cols*j + i) + 2] = 0;
    		}
    	}
    }
    // GaussianBlur(output, output, Size(5, 5), 1.2, 1.2);

    imshow("Frame",frame);
    imshow("arena",output);
    imwrite("test.bmp",output);
    cvWaitKey();
    return 0;
}

// 76,97,111   215,135,133