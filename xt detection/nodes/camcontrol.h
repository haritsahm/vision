#ifndef CAMCONTROL_H//
#define CAMCONTROL_H
//WARNING: imgproc contains dangerous #defines, must be places after all other headers, otherwise causes errors
//TODO: Read HeadMotor header for TODO.
#include <opencv2/opencv.hpp>
#include "featuredetection.h"
#include "headmotor.h"

class CamControl
{
private:
	const int IMAGE_WIDTH;
	const int IMAGE_HEIGHT;
	const int CENTRE_RECT_X1;
	const int CENTRE_RECT_Y1;
	const int CENTRE_RECT_X2;
	const int CENTRE_RECT_Y2;
	static const int LEFT_LIMIT = 120;	//These are max angle limits for camera motion
	static const int RIGHT_LIMIT = 180;

	int state_of_motion;
	int prev_img_flag;
	int prev_ballx;
	int prev_bally;
	int pass_counter;

	
	void moveSearch(HeadMotor &hm, int lower, int upper);
	int moveToPoint(int x, int y, HeadMotor &hm);
	bool inCentreRect(int x, int y, int x1, int y1, int x2, int y2);
	

public:
	//TODO:
	//Maybe variables should be private and there should be functions
	//returning their values?
	CamControl(CamCapture &cam);
	BallReturns findBall(FeatureDetection &fd, HeadMotor &hm);
	void search(HeadMotor &hm);
	float ballRatio;
};

#endif
