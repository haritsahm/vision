#ifndef BALLDETECTOR_H
#define BALLDETECTOR_H

#include <cstdio>
#include <iostream>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>

#include <opencv2/opencv.hpp>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cvaux.h>

#include <ueye.h>

using namespace std;
using namespace cv;

#define returnPixel1C(image, x, y) ((uchar*)(image->imageData + image->widthStep*(y)))[x]
//returns a float between 0 & 1
#define RANDOM_NUM ((float)rand()/(RAND_MAX+1.0))
#define PIXEL_VAL 0
#define INF 10000000

struct Circle
{
	int x;
	int y;
	int r;
};

class balldetector
{
	static const int POPULATION_SIZE = 70;
	static const double CROSSOVER_PROBABILITY = 0.40;
	static const double MUTATION_PROBABILITY = 0.10;
	static const int MUTATION_STEP = 10;
	static const int GENERATIONS = 1000;
	static const int ELITE_INDIV_COUNT = 1;
	static const int MIN_RADIUS = 50;

	int IMAGE_HEIGHT;
	int IMAGE_WIDTH;
	int edge_count;										//Number of points in the edges detected.
	int Ns;												//Number of points on the circumference of the candidate circle used to evaluate the fitness of the circle.
	IplImage* canny_image;
	IplImage* debug;
	vector<CvPoint> edgePoints;
	vector<CvPoint> testSet;							//Coordinates of the points on the circuference of the test set.
	Circle candidatePopulation[POPULATION_SIZE];
	double fitnessPopulation[POPULATION_SIZE];					//make sure it is kept normalised
	double rouletteWheel[POPULATION_SIZE];
	Circle bestCandidate;
public:
	balldetector(IplImage *img);
	// ~balldetector();
	
	double rand1();									//returns value between 0 - 1
	double rand2();									//returns value between -1 - 1
	Circle get3PointCircle(CvPoint pi, CvPoint pj, CvPoint pk);
	void addToTestSet(CvPoint p, Circle c);
	void generateTestSet(Circle c);
	double fitnessValue(Circle c);
	void updateFitness();
	void computeRouletteProb();
	Circle chooseFromPopulation();
	Circle crossover(Circle &c1, Circle &c2);
	void mutate(Circle &c);
	bool isValidCircle(Circle c);
	void findBall();
	void drawPopulation();
	void drawTestSet();
};

#endif