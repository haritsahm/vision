#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <algorithm>

#include <opencv2/opencv.hpp>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cvaux.h>
#include <cvblob.h>

using namespace std;
using namespace cv;
using namespace cvb;

typedef struct
{
	CvPoint st;
	CvPoint end;
	double getSlope()
	{
		return (double)(st.y - end.y)/(double)(st.x - end.x);
	}
	double getA()
	{

	}
	double getB()
	{

	}
	double getC()
	{

	}
}Line;

typedef struct
{
	CvPoint cen;
	int radius;
}Circle;

bool isIntersecting(Line l, Circle c)
{
	double a = l.getA();
	double b = l.getB();
	double c = l.getC();

}

vector<Line> joinPoint(CvPoint pt1, CvPoint pt2, vector<Circle> &obstacles)
{

}

vector<Line> generatePath(vector<CvPoint> &redCircle, vector<CvPoint> &greenCircle, vector<CvPoint> &blackSquare, vector<CvPoint> &whiteSquare)
{

}