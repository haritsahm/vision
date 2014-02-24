#ifndef LOCALIZE_H//
#define LOCALIZE_H
//NOTE: Origin is on Yellow goal post and BYB side
//WARNING: imgproc contains dangerous #defines, must be places after all other headers, otherwise causes errors
//TODO: Read HeadMotor header for TODO.
#include <opencv2/opencv.hpp>
#include "../common/defines.h"
#include "camcapture.h"
#include "headmotor.h"
#include "../common/commondefs.h"
#include <math.h>
#include "featuredetection.h"
#include "imgproc.h"
#include <time.h>
#include <stdlib.h>
#include "../xsens/imu.h"
#include "../behavior/motionmodel.h"

//#define SHOW_FANCY_LOCALIZATION

/*
TECHNIQUE FOR LOCALIZATION:
Given - we have a probability distribution for position and orientation
Monte carlo - N random samples must be selected from this. Sample: position + orientation + its probability
From these N random samples, generate N more random samples based on the observations
Repeat

IMPLEMENTATION:
1. Select N random particles from distribution made up by last time's N particles (can't be the same ones!)
2. Apply motion model to these particles - for eg shift each of these particles in the direction moved or something
3. For each particle, find probability that this is the position according to landmarks observed. Multiply this to existing probability.
4. Repeat

PROBLEM: Is random sample selection dependent on PDF or not? 
ie are high probability points more likely to be selected in sampling, or is each point as likely as the other?

TODO: How to get a PDF from N random particles? We need this because:
1. We need to get the position from this thing (ob)
2. We need to get N random particles from this
Can this be achieved without explicitly generating a PDF? I think it can.

TODO: To prevent kidnap problem, some random particles need to be inserted(?)

TODO: Feature Detection:
1. Xs and Ts - Use schulz, current method is SLOW.
2. Landmark poles and goal posts - Current thing seems ok, maybe small modifications
3. Need to add line detection


PROBLEM: Kidnap problem happening. Need to sprinkle some random points each iteration
SOLUTION: Added 2% random points each iteration
*/


class Localize
{
private:
	void calcConfidence();
	static const int NO_OF_PARTICLES = 200;
	//NOTE: Field is defined to be within white lines (as done previously). Green outside white lines is not used in calculations
	static const int MIN_DISTANCE = 5;	//Minimum resolution of distance in cms
	static const int MIN_ANGLE = 10;	//Minimum resolution of angle in degrees
	static const int MAX_X = 450;	//Maximum width of field in cms
	static const int MAX_Y = 300;	//Maximum height of field in cms
	static const double U_DIST = 5.0;
	static const double U_ANGLE = 10.0;
	static const double U_DIST_P = 0.1;

	LOCALIZE_INTERNALS::Particle p[NO_OF_PARTICLES];
	LOCALIZE_INTERNALS::Particle p_temp[NO_OF_PARTICLES];
	double w[NO_OF_PARTICLES];

	LOCALIZE_INTERNALS::FieldLandmark landX[2];	//Put landmarks here
	LOCALIZE_INTERNALS::FieldLandmark landT[2];
	LOCALIZE_INTERNALS::FieldLandmark landGPY[2];
	LOCALIZE_INTERNALS::FieldLandmark landGPB[2];
	LOCALIZE_INTERNALS::FieldLandmark landYBY;
	LOCALIZE_INTERNALS::FieldLandmark landBYB;

	void createLandmarkList();
	//This function is used for resampling.
	//Simple way to resample would be to make a cumulative list of probability, 
	//generate a random number, and then look for interval corresponding to that number in the 
	//cumulative list using binary search
	//However, there is a method called alias method which speeds up this process.
	//for exact implementation, look for alias method on the internet. This function has been
	//directly copied from somewhere.
	void walkersalias(
	int n,   // number of classes
	double *p, // relative weights of each class
	int nans,  // sample size to return
	int *ans  // sample as an array of class indices
	);
	double meanX;
	double meanY;
	double meanAngle;
	unsigned long nFrames;
#ifdef SHOW_FANCY_LOCALIZATION
	IplImage* loadedImage;
#endif
	void updatePosition();
	void cvQuiver(IplImage*Image,int x,int y,int u,int v,CvScalar Color,int Size,int Thickness);

public:
	double selfX, selfY, selfAngle;
	double confidence;
	IplImage* dispImage;
	Localize();
	void printPosition();
	void doLocalize(FeatureDetection &fd, MotionModel &mm, CamCapture &cam, int imuangle = -9999);
	
	AbsCoords getGoalCoords(int g);
	void randomize();
};

#endif
