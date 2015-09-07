#include <iostream>
#include <stdio.h>
#include <vector>
#include <algorithm>

#include <opencv/cv.h>
#include <opencv/highgui.h> 
#include <cvblob.h>

using namespace cv;
using namespace std;

int main(int argc, char const *argv[])
{
	IplImage* img = cvLoadImage("img1.png", 1);
	IplImage* output = cvCreateImage(cvSize(img->width, img->height), 8, 3);
	cvCvtColor(img, output, CV_BGR2Lab);
	// cvCvtPixToPlane(output, l_channel, a_channel, b_channel, 0);
	cvShowImage("img", img);
	cvShowImage("output", output);
	cvWaitKey();
	return 0;
}