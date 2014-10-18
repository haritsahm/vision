#include <iostream>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <cmath>

#include <opencv/cv.h>
#include <opencv/cvaux.h>
#include <opencv/highgui.h>

using namespace std;
using namespace cv;

#define returnPixel1C(image, x, y) ((uchar*)(image->imageData + image->widthStep*(y)))[x]

int main(int argc, char const *argv[])
{
	if(argc < 2)
	{
		printf("./colorConnect <FILENAME>\n");
		return 0;
	}
	IplImage *origImg = cvLoadImage(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
	IplImage *xtImage = cvCreateImage(cvSize(origImg->width, origImg->height), 8, 3);
	int IMAGE_WIDTH = origImg->width;
	int IMAGE_HEIGHT = origImg->height;
	for (int i = 1; i < IMAGE_WIDTH - 1; ++i)
	{
		for (int j = 1; j < IMAGE_HEIGHT - 1; ++j)
		{
			if(returnPixel1C(origImg, i, j) == 0)
				continue;
			int count = 0;
			for (int x = -1; x < 2; ++x)
			{
				for (int y = -1; y < 2; ++y)
				{
					if(returnPixel1C(origImg, i+x, j+y) == 255)
						count++;
				}
			}
			if(count == 4)
				cvCircle(xtImage, cvPoint(i, j), 1, CV_RGB(255, 0, 0));
			else if(count == 3)
				cvCircle(xtImage, cvPoint(i, j), 1, CV_RGB(0, 255, 0));
			// else if(count == 2)
			// 	cvCircle(xtImage, cvPoint(i, j), 2, CV_RGB(0, 0, 255));
			else
				cvCircle(xtImage, cvPoint(i, j), 1, CV_RGB(255, 255, 255));
		}
	}
	cvShowImage("origImg", origImg);
	cvShowImage("xtImage", xtImage);
	cvWaitKey();
	return 0;
}