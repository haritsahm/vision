#ifndef FEATUREDETECTION_H
#define FEATUREDETECTION_H//

#include "camcapture.h"
#include "../common/defines.h"
#include "../common/modes.h"
#include "headmotor.h"
#include "../common/commondefs.h"
#include <math.h>
#include <cvblob.h>
#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"
#include <fstream>
#include <stdio.h>
#include "../behavior/motionmodel.h"

using namespace std;
#define SYMMETRIC_LANDMARKS
#define PLOT_LANDMARKS

// #define PLOT_HOUGH_LINES
//new technique to get features:
//0. for each feature in main array, increase framecounter. if framecounter above a threshold, delete it.
//1. get features in current image. store in a temporary array
//2. for each new feature, check main array
//3. if same feature with similar location exists, refresh its values and set its framecounter to 0
//4. if feature does not exist, add it to main vector and set its counter to 0
struct parameters
{
	int motor_pos;
	double angle;
	double focal;
	double pix2cmx;
	double pix2cmy;
	double s_view_compensation;
};

class FeatureDetection
{
private:
	const int IMAGE_HEIGHT;
	const int IMAGE_WIDTH;
	static const double pix2cm = 0.28;	//WARNING: need to change this whenever changing size of image
	static const double s_height=93.0;//86.0;
	static const double neck_len=7.0; //constanty=4.5,rangey=10.0,constantx=8.0,rangex=16.0,rminx=70.0;
	static const double forwardTiltCorrection = -3.3;	//correction in degrees
	//No of frames to persist
	static const int LANDMARK_PERSISTENCE = 30;
	static const int OBSTACLE_PERSISTENCE = 30;
	static const double ax = -7.1e-06;
	cvb::CvBlobs blobs_red;
	cvb::CvBlobs blobs_blue;
	cvb::CvBlobs blobs_yellow;
	cvb::CvBlobs blobs_black;
	IplImage* seg_red;
	IplImage* seg_blue;
	IplImage* seg_yellow;
	IplImage* labelImg;
	IplImage* labelImg_small;
	IplImage* seg_white;
	IplImage* seg_black;
	IplImage* seg_green;
	IplImage* seg_white_count;
	void undistort(int xd, int yd, int* xu, int* yu);
	void findReal(int x,int y, double &objdis, double &objangdeg, HeadMotor &hm);
	void getGoals(CamCapture &cam, HeadMotor &hm);
	void getBlobs(CamCapture &cam);
	void getLPs(CamCapture &cam, HeadMotor &hm);
	bool getObstacles(CamCapture &cam, HeadMotor &hm);
	void getCorners(CamCapture &cam, HeadMotor &hm);
	void getBall(CamCapture &cam, HeadMotor &hm);
	bool ballFound_var;
	int ballX_var;
	int ballY_var;
	inline double getDistance(CvPoint2D64f a, CvPoint2D64f b)
	{
		return sqrt((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y));
	}
	inline bool isOnImageEdge(int x, int y)
	{
		//We only need to check if too far left, right or up. Too far down is fine
		if(x > IMAGE_WIDTH - 45 || x < 45 || y < 15)
			return true;
		else
			return false;
	}
	inline bool isOnImageEdgeObstacle(int x, int y)
	{
		if(x > IMAGE_WIDTH - 20 || x < 20 || y < 15 || y > IMAGE_HEIGHT - 15)
			return true;
		else
			return false;
	}
	inline bool isCloseLandmark(LOCALIZE_INTERNALS::Landmark &l1, LOCALIZE_INTERNALS::Landmark &l2, double distanceLimit, double angleLimit)
	{
		if(fabs(l1.distance-l2.distance) < distanceLimit && fabs(l1.angle-l2.angle) < angleLimit)
			return true;
		else
			return false;
	}
	inline bool isSameLandmark(LOCALIZE_INTERNALS::Landmark &l1, LOCALIZE_INTERNALS::Landmark &l2, double distanceLimit = 20.0, double angleLimit = 10.0)
	{
		if(l1.type!=l2.type)
			return false;
		return isCloseLandmark(l1, l2, distanceLimit, angleLimit);
	}
	inline bool isSameObstacle(LOCALIZE_INTERNALS::Obstacle &l1, LOCALIZE_INTERNALS::Obstacle &l2)
	{
		if(fabs(l1.distance-l2.distance) < 40.0 && fabs(l1.angle-l2.angle) < 40.0)
			return true;
		else
			return false;
	}
	void getInGreen(CamCapture &cam);
	void getInGreenKartik(CamCapture &cam);

public:
	LOCALIZE_INTERNALS::Landmark templ[20]; //Max landmarks you might detect in an image
	int tempnLand;
	std::vector<LOCALIZE_INTERNALS::Landmark> l;
	LOCALIZE_INTERNALS::Obstacle tempo[20];	//Max obstacles you might detect in an image
	int tempnObstacle;
	std::vector<LOCALIZE_INTERNALS::Obstacle> o;
	//Opening LUT for parameters
	ifstream constants;
	// int nLand;
	Coords ball;
	FeatureDetection(CamCapture &cam);
	void getLandmarks(CamCapture &cam, HeadMotor &hm, MotionModel &mm);
	inline bool ballFound(){return ballFound_var;};
	inline int ballX(){return ballX_var;};
	inline int ballY(){return ballY_var;};
	void updatePacket(FeaturesPacket &fp);
	double ballRatio;
};


#endif
