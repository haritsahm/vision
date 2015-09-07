#include <iostream>
#include <stdio.h>
#include <vector>
#include <algorithm>

#include <opencv/cv.h>
#include <opencv/highgui.h> 
#include <cvblob.h>

using namespace std;
using namespace cvb;

#define PI 3.1415926536
#define INF 1000000007

double findAngularDistortion(int width, int height, vector<int> u, vector<int> v)
{
	for (int i = 0; i < u.size(); ++i)
		u[i] -= width/2;
	for (int i = 0; i < v.size(); ++i)
		v[i] = height/2 - v[i];
	vector<double> x(4);
	vector<double> y(4);
	double f = 0.176;
	double s = 0.0006;
	double start = -90.0;
	double angle = start;
	double dist_angle = -1;
	double deviation = INF;
	while(angle < -start)
	{
		double alpha = angle*(PI/180.0);
		x[0] = (u[0] + (f/s)*tan(alpha))/(1 - (s/f)*u[0]*tan(alpha));
		y[0] = v[0]*((s/f)*x[0]*sin(alpha) + cos(alpha));
		x[1] = (u[1] + (f/s)*tan(alpha))/(1 - (s/f)*u[1]*tan(alpha));
		y[1] = v[1]*((s/f)*x[1]*sin(alpha) + cos(alpha));
		x[2] = (u[2] + (f/s)*tan(alpha))/(1 - (s/f)*u[2]*tan(alpha));
		y[2] = v[2]*((s/f)*x[2]*sin(alpha) + cos(alpha));
		x[3] = (u[3] + (f/s)*tan(alpha))/(1 - (s/f)*u[3]*tan(alpha));
		y[3] = v[3]*((s/f)*x[3]*sin(alpha) + cos(alpha));
		double temp1 = x[0] - x[2];
		double temp2 = y[1] - y[3];
		double diff = fabs(temp1 - temp2);
		if(diff < deviation)
		{
			deviation = diff;
			dist_angle = angle;
		}
		angle += 0.01;
	}
	return dist_angle;
}

IplImage* undistortImage(double angle, IplImage* img)
{
	double alpha = angle*(PI/180.0);
	double f = 0.176;
	double s = 0.0006;
	IplImage* undistorted_img = cvCreateImage(cvSize(img->width*3, img->height*3), 8, 3);
	// printf("%d %d\n", img->width, img->height);
	for (int i = 0; i < img->width; ++i)
	{
		for (int j = 0; j < img->height; ++j)
		{
			// printf("%d %d\n", i, j);
			double u, v, x, y;
			u = i - img->width;
			v = img->height - j;
			x = (u + (f/s)*tan(alpha))/(1 - (s/f)*u*tan(alpha));
			y = v*((s/f)*x*sin(alpha) + cos(alpha));
			int col = x + undistorted_img->width - 600;
			int row = undistorted_img->height - y - 300;
			uchar b = img->imageData[img->widthStep * j + i * 3];
			uchar g = img->imageData[img->widthStep * j + i * 3 + 1];
			uchar r = img->imageData[img->widthStep * j + i * 3 + 2];
			if(row < 0 || col < 0)
				continue;
			if(abs(row) >= undistorted_img->height || abs(col) >= undistorted_img->width)
				continue;
			undistorted_img->imageData[undistorted_img->widthStep * row + col * 3] = b;
			undistorted_img->imageData[undistorted_img->widthStep * row + col * 3 + 1] = g;
			undistorted_img->imageData[undistorted_img->widthStep * row + col * 3 + 2] = r;
		}
	}
	cvShowImage("undistorted_img", undistorted_img);
}

int main(int argc, char const *argv[])
{
	if(argc < 2)
	{
		printf("USAGE ./%s <FILENAME>\n", argv[0]);
		return -1;
	}
	IplImage *img = cvLoadImage(argv[1]);
	int width = img->width;
	int height = img->height;
	IplImage *img_labeled = cvCreateImage(cvSize(width, height), 8, 3);
	IplImage *img_hsv = cvCreateImage(cvSize(width, height), 8, 3);
	IplImage *threshy = cvCreateImage(cvSize(width, height), 8, 1);
	IplImage *labelImg = cvCreateImage(cvSize(width, height), IPL_DEPTH_LABEL, 1);
	CvBlobs blobs;
	cvCvtColor(img, img_hsv, CV_BGR2HSV);
	cvInRangeS(img_hsv, cvScalar(0, 0, 26), cvScalar(10, 10, 154), threshy);
	cvSmooth(threshy,threshy,CV_MEDIAN,7,7);
	unsigned int result = cvLabel(threshy, labelImg, blobs);
	cvFilterByArea(blobs, (int)0.2*width*height, (int)width*height);
	cvRenderBlobs(labelImg, blobs, img, img_labeled);
	vector<int> u(4);
	vector<int> v(4);
	for(CvBlobs::const_iterator it=blobs.begin(); it!=blobs.end(); ++it)
	{
		u[0] = it->second->maxx;
		v[0] = it->second->centroid.y;
		u[1] = it->second->centroid.x;
		v[1] = it->second->miny;
		u[2] = it->second->minx;
		v[2] = it->second->centroid.y;
		u[3] = it->second->centroid.x;
		v[3] = it->second->maxy;
	}
	cvCircle(img_labeled, cvPoint(u[0], v[0]), 2, CV_RGB(0, 255, 255), -1);
	cvCircle(img_labeled, cvPoint(u[1], v[1]), 2, CV_RGB(0, 255, 255), -1);
	cvCircle(img_labeled, cvPoint(u[2], v[2]), 2, CV_RGB(0, 255, 255), -1);
	cvCircle(img_labeled, cvPoint(u[3], v[3]), 2, CV_RGB(0, 255, 255), -1);
	double angle = findAngularDistortion(width, height, u, v);
	printf("%lf\n", angle);
	IplImage* undistorted_img = undistortImage(80.0, img);
	cvShowImage("img", img_labeled);
	cvWaitKey();
	return 0;
}