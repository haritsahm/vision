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
#define INF 1000000007

void calcCij(IplImage* image, vector<vector<int> > &cij)
{
	int IMAGE_WIDTH = image->width;
	int IMAGE_HEIGHT = image->height;
	cij.clear();
	cij.resize(IMAGE_WIDTH, vector<int>(IMAGE_HEIGHT, INF));
	vector<vector<int> > whiteCount(vector<vector<int> >(IMAGE_WIDTH, vector<int>(IMAGE_HEIGHT, 0)));
	for (int i = 1; i < IMAGE_WIDTH - 1; ++i)
	{
		for (int j = 1; j < IMAGE_HEIGHT; ++j)
		{
			for (int x = -1; x < 2; ++x)
			{
				for (int y = -1; y < 2; ++y)
				{
					if(returnPixel1C(image, i+x, j+y) == 255)
						whiteCount[i][j]++;
				}
			}
		}
	}
	for (int i = 1; i < IMAGE_WIDTH - 1; ++i)
	{
		for (int j = 1; j < IMAGE_HEIGHT - 1; ++j)
		{
			if(whiteCount[i][j] == 0)
				continue;
			int count = 0;
			for (int x = -1; x < 2; ++x)
			{
				for (int y = -1; y < 2; ++y)
				{
					if(x == 0 && y == 0)
						continue;
					if(whiteCount[i+x][j+y] >= whiteCount[i][j])
						count++;
				}
			}
			cij[i][j] = count;
		}
	}
}

void createImageNodes(IplImage* image, vector<vector<int> > &cij)
{
	for (int i = 0; i < cij.size(); ++i)
	{
		for (int j = 0; j < cij[0].size(); ++j)
		{
			if(cij[i][j] == 0)
				cvCircle(image, cvPoint(i, j), 1, CV_RGB(255, 0, 0));
			if(cij[i][j] == 1 || cij[i][j] == 2)
				cvCircle(image, cvPoint(i, j), 1, CV_RGB(0, 255, 0));
		}
	}
}

int main(int argc, char const *argv[])
{
	if(argc < 2)
	{
		printf("./makeNodes <FILENAME>\n");
		return 0;
	}
	IplImage *origImg = cvLoadImage(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
	IplImage *nodes = cvCreateImage(cvSize(origImg->width, origImg->height), 8, 3);
	int IMAGE_WIDTH = origImg->width;
	int IMAGE_HEIGHT = origImg->height;
	vector<vector<int> > cij;
	calcCij(origImg, cij);
	createImageNodes(nodes, cij);
	cvShowImage("origImg", origImg);
	cvShowImage("nodes", nodes);
	cvWaitKey();
	return 0;
}