#include <opencv2/opencv.hpp>
#include <opencv/cv.h>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <deque>
#include <stack>
#include <queue>
#include <bitset>
#include <algorithm>
#include <functional>
#include <numeric>
#include <utility>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>

using namespace std;

#define KERNEL_SIZE 3
#define LOAD_KERNEL
#define returnPixel1C(image, x, y) ((uchar*)(image->imageData + image->widthStep*(y)))[x]

int IMAGE_WIDTH;
int IMAGE_HEIGHT;

void generateKernels(vector<vector<int> > &kernel45, vector<vector<int> > &kernel90, vector<vector<int> > &kernel135, vector<vector<int> > &kernel180)
{
	for (int i = 0; i < KERNEL_SIZE; ++i)
	{
		for (int j = 0; j < KERNEL_SIZE; ++j)
		{
			if(j == KERNEL_SIZE/2)
				kernel180[i][j] = KERNEL_SIZE - 1;
			if(i == KERNEL_SIZE/2)
				kernel90[i][j] = KERNEL_SIZE - 1;
			if(i == j)
				kernel135[i][j] = KERNEL_SIZE - 1;
			if(i == KERNEL_SIZE - j - 1)
				kernel45[i][j] = KERNEL_SIZE - 1;
		}
	}
}

void readKernels(int n, vector<vector<vector<int> > > &kernels)
{
	kernels.resize(n, vector<vector<int> >(3, vector<int>(3)));
	for (int no = 0; no < n; ++no)
	{
		for (int i = 0; i < KERNEL_SIZE; ++i)
		{
			for (int j = 0; j < KERNEL_SIZE; ++j)
			{
				scanf("%d", &kernels[no][i][j]);
			}
		}
	}
}

void applyKernel(vector<vector<int> > &kernel, IplImage *orignalImage, IplImage *image)
{
	for (int i = KERNEL_SIZE/2; i < IMAGE_HEIGHT - KERNEL_SIZE/2; ++i)
	{
		for (int j = KERNEL_SIZE/2; j < IMAGE_WIDTH - KERNEL_SIZE/2; ++j)
		{
			int value = 0;
			for (int x = 0; x < KERNEL_SIZE; ++x)
			{
				for (int y = 0; y < KERNEL_SIZE; ++y)
				{
					value += kernel[x][y] * returnPixel1C(orignalImage, i + x - KERNEL_SIZE/2, j + y - KERNEL_SIZE/2);
				}
			}
			if(value <= 0)
				returnPixel1C(image, i , j) = 0;
			else
				returnPixel1C(image, i, j) = 255;
		}
	}
}

int main(int argc, char const *argv[])
{
	if(argc == 1)
	{
		printf("USAGE ./line_kernel <FILENAME>\n");
		return 0;
	}
	#ifdef LOAD_KERNEL
	    freopen("line_kernel.txt", "r", stdin);
	#endif
	int n;
	scanf("%d", &n);
	IplImage *orignalImage = cvLoadImage(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
	IplImage *result[n];
	IMAGE_HEIGHT = orignalImage->height;
	IMAGE_WIDTH = orignalImage->width;
	vector<vector<vector<int> > > kernels;
	readKernels(n, kernels);
	for (int i = 0; i < n; ++i)
	{
		result[i] = cvCreateImage(cvGetSize(orignalImage), 8, 1);
	}
	for (int i = 0; i < n; ++i)
	{
		applyKernel(kernels[i], orignalImage, result[i]);
	}
	cvShowImage("orignalImage", orignalImage);
	for (int i = 0; i < n; ++i)
	{
		char str[10];
		sprintf(str, "result %d", i);
		cvShowImage(str, result[i]);
	}
	cvWaitKey();
	return 0;
}

// g++ -I/opt/local/include -L/opt/local/lib -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_objdetect line_kernel.cpp -o line_kernel