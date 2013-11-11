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

int main()
{
	Mat img (Size(255,255),CV_8UC1);
	//Before changing
	// imshow("Before",img);
	//change some pixel value
	for(int j=0;j<img.rows;j++) 
	{
	  for (int i=0;i<img.cols;i++)
	  {
	    // if( i== j)   
	       img.at<uchar>(j,i) = i; //white
	       // cout<<img.at<unsigned>(i,j)<<endl;
	       cout<<(int)(uchar*)(img.at<uchar>(j,i))<<endl;
	  }
	}
	//After changing
	imshow("After",img);
	cvWaitKey ();
}