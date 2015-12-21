#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <algorithm>
#include <cmath>

#include <opencv2/opencv.hpp>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cvaux.h>
#include <cvblob.h>

using namespace std;
using namespace cv;
using namespace cvb;

int main(int argc, char const *argv[])
{
	CvCapture *capture = cvCaptureFromCam(1);
	int visited = 0;
	IplImage *frame = cvQueryFrame(capture);
	IplImage* img_hsv = cvCreateImage(cvSize(frame->width, frame->height), 8, 3);
	IplImage* threshy_red = cvCreateImage(cvSize(frame->width, frame->height), 8, 1);
	IplImage* threshy_green = cvCreateImage(cvSize(frame->width, frame->height), 8, 1);
	IplImage* threshy_black = cvCreateImage(cvSize(frame->width, frame->height), 8, 1);
	IplImage* threshy_white = cvCreateImage(cvSize(frame->width, frame->height), 8, 1);
	IplImage* threshy_yellow = cvCreateImage(cvSize(frame->width, frame->height), 8, 1);
	IplImage* threshy_pink = cvCreateImage(cvSize(frame->width, frame->height), 8, 1);
	IplImage* labelImg_red = cvCreateImage(cvSize(frame->width, frame->height),IPL_DEPTH_LABEL,1);
	IplImage* labelImg_green = cvCreateImage(cvSize(frame->width, frame->height),IPL_DEPTH_LABEL,1);
	IplImage* labelImg_black = cvCreateImage(cvSize(frame->width, frame->height),IPL_DEPTH_LABEL,1);
	IplImage* labelImg_white = cvCreateImage(cvSize(frame->width, frame->height),IPL_DEPTH_LABEL,1);
	IplImage* labelImg_yellow = cvCreateImage(cvSize(frame->width, frame->height),IPL_DEPTH_LABEL,1);
	IplImage* labelImg_pink = cvCreateImage(cvSize(frame->width, frame->height),IPL_DEPTH_LABEL,1);
	FILE *fp = fopen("/dev/ttyACM0", "w");
	while(1)
	{
		cvZero(img_hsv);
		cvZero(threshy_red);
		cvZero(threshy_black);
		cvZero(threshy_green);
		cvZero(threshy_white);
		cvZero(threshy_yellow);
		cvZero(labelImg_white);
		cvZero(labelImg_black);
		cvZero(labelImg_green);
		cvZero(labelImg_red);
		cvZero(labelImg_yellow);
		cvZero(labelImg_pink);
		cvCvtColor(frame,img_hsv,CV_BGR2HSV);
		CvBlobs blobs_red;
		CvBlobs blobs_green;
		CvBlobs blobs_black;
		CvBlobs blobs_white;
		CvBlobs blobs_yellow;
		CvBlobs blobs_pink;
		cvInRangeS(img_hsv, cvScalar(0, 44, 118), cvScalar(28, 191, 221), threshy_red);
		cvInRangeS(img_hsv, cvScalar(0, 0, 0), cvScalar(100, 125, 73), threshy_black);
		cvInRangeS(img_hsv, cvScalar(59, 74, 102), cvScalar(90, 255, 149), threshy_green);
		cvInRangeS(img_hsv, cvScalar(0, 0, 219), cvScalar(27, 62, 255), threshy_white);
		cvInRangeS(img_hsv, cvScalar(0, 0, 219), cvScalar(27, 62, 255), threshy_yellow);
		cvInRangeS(img_hsv, cvScalar(0, 0, 219), cvScalar(27, 62, 255), threshy_pink);
		cvSmooth(threshy_red,threshy_red,CV_MEDIAN,7,7);
		cvSmooth(threshy_green,threshy_green,CV_MEDIAN,7,7);
		cvSmooth(threshy_black,threshy_black,CV_MEDIAN,7,7);
		cvSmooth(threshy_white,threshy_white,CV_MEDIAN,7,7);
		cvSmooth(threshy_yellow,threshy_yellow,CV_MEDIAN,7,7);
		cvSmooth(threshy_pink,threshy_pink,CV_MEDIAN,7,7);
		unsigned int result1=cvLabel(threshy_red,labelImg_red,blobs_red);
		unsigned int result2=cvLabel(threshy_black,labelImg_black,blobs_black);
		unsigned int result3=cvLabel(threshy_green,labelImg_green,blobs_green);
		unsigned int result4=cvLabel(threshy_white,labelImg_white,blobs_white);
		unsigned int result5=cvLabel(threshy_yellow,labelImg_yellow,blobs_yellow);
		unsigned int result6=cvLabel(threshy_pink,labelImg_pink,blobs_pink);
		cvFilterByArea(blobs_red,100,100000);
		cvFilterByArea(blobs_green,100,100000);
		cvFilterByArea(blobs_white,100,100000);
		cvFilterByArea(blobs_black,100,100000);
		cvFilterByArea(blobs_yellow,100,100000);
		cvFilterByArea(blobs_pink,100,100000);
		cvRenderBlobs(labelImg_green,blobs_green,frame,frame);
		cvRenderBlobs(labelImg_red,blobs_red,frame,frame);
		cvRenderBlobs(labelImg_black,blobs_black,frame,frame);
		cvRenderBlobs(labelImg_white,blobs_white,frame,frame);
		cvRenderBlobs(labelImg_yellow,blobs_yellow,frame,frame);
		cvRenderBlobs(labelImg_pink,blobs_pink,frame,frame);
		vector<CvPoint> redCircle;
		vector<CvPoint> greenCircle;
		vector<CvPoint> blackSquare;
		vector<CvPoint> whiteSquare;
		CvPoint bot;
		CvPoint botf;
		for (CvBlobs::const_iterator it=blobs_red.begin(); it!=blobs_red.end(); ++it)
		{
			CvPoint temp;
			temp.x = (it->second->maxx+it->second->minx)/2;
			temp.y = (it->second->maxy+it->second->miny)/2;
			cvCircle(frame,temp,3,CV_RGB(255,0,0),-1);
			redCircle.push_back(temp);
		}
		for (CvBlobs::const_iterator it=blobs_green.begin(); it!=blobs_green.end(); ++it)
		{
			CvPoint temp;
			temp.x = (it->second->maxx+it->second->minx)/2;
			temp.y = (it->second->maxy+it->second->miny)/2;
			cvCircle(frame,temp,3,CV_RGB(0,255,0),-1);
			greenCircle.push_back(temp);
		}
		for (CvBlobs::const_iterator it=blobs_black.begin(); it!=blobs_black.end(); ++it)
		{
			CvPoint temp;
			temp.x = (it->second->maxx+it->second->minx)/2;
			temp.y = (it->second->maxy+it->second->miny)/2;
			cvCircle(frame,temp,3,CV_RGB(0,0,0),-1);
			blackSquare.push_back(temp);
		}
		for (CvBlobs::const_iterator it=blobs_white.begin(); it!=blobs_white.end(); ++it)
		{
			CvPoint temp;
			temp.x = (it->second->maxx+it->second->minx)/2;
			temp.y = (it->second->maxy+it->second->miny)/2;
			cvCircle(frame,temp,3,CV_RGB(255,255,255),-1);
			whiteSquare.push_back(temp);
		}
		for (CvBlobs::const_iterator it=blobs_yellow.begin(); it!=blobs_yellow.end(); ++it)
		{
			bot.x = (it->second->maxx+it->second->minx)/2;
			bot.y = (it->second->maxy+it->second->miny)/2;
			cvCircle(frame,bot,3,CV_RGB(255,255,255),-1);
		}
		for (CvBlobs::const_iterator it=blobs_pink.begin(); it!=blobs_pink.end(); ++it)
		{
			botf.x = (it->second->maxx+it->second->minx)/2;
			botf.y = (it->second->maxy+it->second->miny)/2;
			cvCircle(frame,botf,3,CV_RGB(255,255,255),-1);
		}
		vector<CvPoint> points;
		points.push_back(redCircle[3]);
		points.push_back(greenCircle[0]);
		points.push_back(redCircle[2]);
		points.push_back(redCircle[1]);
		points.push_back(redCircle[0]);
		points.push_back(whiteSquare[0]);
		points.erase(points.begin(),points.begin()+visited);
		double slope1 = (double)(botf.y - bot.y)/(double)(botf.x - bot.x);
		double slope2 = (double)(points[0].y - bot.y)/(double)(points[0].x - bot.x);
		double epsilon = 0.10;
		// char c;
		if(fabs(slope1 - slope2) < epsilon)
			fprintf(fp, "2");
		else if(slope1 - slope2 < epsilon)
			fprintf(fp, "1");
		else
			fprintf(fp, "0");
		cvLine(frame, cvPoint(frame->width/2,frame->height), redCircle[3], CV_RGB(255,255,255));
		cvLine(frame,redCircle[3], greenCircle[0], CV_RGB(255,255,255));
		cvLine(frame,greenCircle[0], redCircle[2], CV_RGB(255,255,255));
		cvLine(frame,redCircle[2], redCircle[1], CV_RGB(255,255,255));
		cvLine(frame,redCircle[1], redCircle[0], CV_RGB(255,255,255));
		cvLine(frame,redCircle[0], whiteSquare[0], CV_RGB(255,255,255));
		cvShowImage("img", frame);
		cvWaitKey(10);
	}
	return 0;
}