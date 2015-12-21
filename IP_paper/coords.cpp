#include <iostream>
#include <stdio.h>

//OpenCV Headers
#include <opencv/cv.h>
#include <opencv/highgui.h>
//Input-Output
#include <stdio.h>
//Blob Library Headers
#include <cvblob.h>

using namespace std;
using namespace cv;

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
	if  ( event == EVENT_LBUTTONDOWN )
		cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
	else if  ( event == EVENT_RBUTTONDOWN )
		cout << "Right button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
}

int main(int argc, char const *argv[])
{
	Mat img = imread("image.bmp");
	if (img.empty()) 
	{ 
		cout << "Error loading the image" << endl;
		return -1; 
	}
	namedWindow("My Window", 1);
	setMouseCallback("My Window", CallBackFunc, NULL);
	imshow("My Window", img);
	waitKey(0);
	return 0;
}