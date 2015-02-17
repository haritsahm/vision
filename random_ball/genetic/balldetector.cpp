#include "balldetector.h"

balldetector::balldetector(IplImage* img)
{
	srand(time(NULL));
	canny_image = img;
	IMAGE_WIDTH = img->width;
	IMAGE_HEIGHT = img->height;
	debug = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), 8, 1);
	for (int i = 0; i < IMAGE_WIDTH; ++i)
	{
		for (int j = 0; j < IMAGE_HEIGHT; ++j)
		{
			if(returnPixel1C(img, i, j) == 255)
				edgePoints.push_back(cvPoint(i, j));
		}
	}

	edge_count = edgePoints.size();

	for (int i = 0; i < POPULATION_SIZE; ++i)
	{
		int pos1 = rand1() * (double) edge_count;
		int pos2 = rand1() * (double) edge_count;
		int pos3 = rand1() * (double) edge_count;
		candidatePopulation[i].pt1 = edgePoints[pos1];
		candidatePopulation[i].pt2 = edgePoints[pos2];
		candidatePopulation[i].pt3 = edgePoints[pos3];
		candidateCircles[i] = get3PointCircle(candidatePopulation[i]);
	}
}

double balldetector::rand1()
{
	return ((double)rand() / (double) RAND_MAX);
}

double balldetector::rand2()
{
	return rand1()*2 - 1;
}

CircleFeat balldetector::get3PointCircle(Circle c)
{
	double xi = c.pt1.x;
	double yi = c.pt1.y;
	double xj = c.pt2.x;
	double yj = c.pt2.y;
	double xk = c.pt3.x;
	double yk = c.pt3.y;
	double x0, y0, r;
	double ri = (xi*xi + yi*yi);
	double rj = (xj*xj + yj*yj);
	double rk = (xk*xk + yk*yk);

	x0 = (2.0 * (rj - ri) * (yk - yi)) - (2.0 * (rk - ri) * (yj - yi));
	x0 /= 4.0 * ((xj - xi) * (yk - yi) - (xk - xi) * (yj - yi));

	y0 = (2.0 * (xj - xi) * (rk - ri)) - (2.0 * (xk - xi) * (rj - ri));
	y0 /= 4.0 * ((xj - xi) * (yk - yi) - (xk - xi) * (yj - yi));

	r = sqrt((xi - x0)*(xi - x0) + (yi - y0)*(yi - y0));

	CircleFeat c0;
	c0.x = (int) x0;
	c0.y = (int) y0;
	c0.r = (int) r;

	return c0;
}

void balldetector::addToTestSet(CvPoint p, CircleFeat c)
{
	int x = c.x + p.x;
	int y = c.y - p.y;
	if(x <= 0 || x >= IMAGE_WIDTH)
		return;
	if(y <= 0 || y >= IMAGE_HEIGHT)
		return;
	testSet.push_back(cvPoint(x, y));
	Ns++;
}

void balldetector::generateTestSet(CircleFeat c)
{
	testSet.clear();
	Ns = 0;
	//Implemented the Midpoint Circle Algorithm (MCA) to generate points on the circle.
	int x = 0;
	int y = c.r;
	int p = 1 - c.r;
	while(y > x)
	{
		if(p < 0)
			p += 2*x + 3;
		else
		{
			p += 2*(x - y) + 5;
			y--;
		}
		x++;
		addToTestSet(cvPoint(x,y), c);
		addToTestSet(cvPoint(x,-y), c);
		addToTestSet(cvPoint(-x,y), c);
		addToTestSet(cvPoint(-x,-y), c);
		addToTestSet(cvPoint(y,x), c);
		addToTestSet(cvPoint(y,-x), c);
		addToTestSet(cvPoint(-y,x), c);
		addToTestSet(cvPoint(-y,-x) , c);
	}
}

double balldetector::fitnessValue(CircleFeat c)
{
	generateTestSet(c);
	double fitVal = 0.0;
	int countPos = 0;

	for (int i = 0; i < Ns; ++i)
	{
		if(returnPixel1C(canny_image, testSet[i].x, testSet[i].y) == 255)
			countPos++;
	}

	fitVal = (double) countPos/(double) Ns;
	if(c.r < MIN_RADIUS)
		fitVal *= (0.01 * (double) c.r)/(double) MIN_RADIUS;
	return fitVal;
}

void balldetector::updateFitness()
{
	double normFactor = 0.0;
	for (int i = 0; i < POPULATION_SIZE; ++i)
	{
		fitnessPopulation[i] = fitnessValue(candidateCircles[i]);
		normFactor += fitnessPopulation[i];
	}
	//Normalising
	for (int i = 0; i < POPULATION_SIZE; ++i)
		fitnessPopulation[i] /= normFactor;
}

void balldetector::computeRouletteProb()
{
	rouletteWheel[0] = fitnessPopulation[0];
	double sum = rouletteWheel[0];
	for (int i = 1; i < POPULATION_SIZE; ++i)
	{
		rouletteWheel[i] = rouletteWheel[i-1] + fitnessPopulation[i];
		sum += rouletteWheel[i];
	}
}

Circle balldetector::chooseFromPopulation()
{
	double prob = rand1();
	for (int i = 0; i < POPULATION_SIZE; ++i)
	{
		if(prob <= rouletteWheel[i])
			return candidatePopulation[i];
	}
	return candidatePopulation[0];
}

Circle balldetector::crossover(Circle c1, Circle c2)
{
	if(rand1() > CROSSOVER_PROBABILITY)
		return c1;
	vector<CvPoint> arr1;
	vector<CvPoint> arr2;
	vector<CvPoint> arr3;

	arr1.push_back(c1.pt1);
	arr1.push_back(c1.pt2);
	arr1.push_back(c1.pt3);
	arr2.push_back(c2.pt1);
	arr2.push_back(c2.pt2);
	arr2.push_back(c2.pt3);

	int pos = rand1()*3.0;
	for (int i = 0; i < pos; ++i)
		arr3.push_back(arr1[i]);
	for (int i = pos; i < 3; ++i)
		arr3.push_back(arr2[i]);

	Circle offspring;
	offspring.pt1 = arr3[0];
	offspring.pt2 = arr3[1];
	offspring.pt3 = arr3[2];
	return offspring;
}

void balldetector::mutate(Circle &c)
{
	if(rand1() > MUTATION_PROBABILITY)
		return;
	int pos = rand1() * (double) edge_count;
	c.pt1 = edgePoints[pos];
	if(rand1() > MUTATION_PROBABILITY)
		return;
	pos = rand1() * (double) edge_count;
	c.pt2 = edgePoints[pos];
	if(rand1() > MUTATION_PROBABILITY)
		return;
	pos = rand1() * (double) edge_count;
	c.pt3 = edgePoints[pos];
}

bool balldetector::isValidCircle(CircleFeat c)
{
	if(c.x <= 0 || c.x >= IMAGE_WIDTH)
		return false;
	if(c.y <= 0 || c.y >= IMAGE_HEIGHT)
		return false;
	return true;
}

void balldetector::findBall()
{
	int gen = 0;
	while(gen < GENERATIONS)
	{
		printf("generation: %d\n", gen);
		updateFitness();
		computeRouletteProb();
		Circle nextGeneration[POPULATION_SIZE];
		//preserving the best candidate to the next generation
		Circle eliteIndiv;
		double maxFitness = 0.0;
		for (int i = 0; i < POPULATION_SIZE; ++i)
		{
			if(fitnessPopulation[i] > maxFitness)
			{
				maxFitness = fitnessPopulation[i];
				eliteIndiv = candidatePopulation[i];
			}
		}
		nextGeneration[0] = eliteIndiv;
		bestCandidate = get3PointCircle(eliteIndiv);

		for (int i = 1; i < POPULATION_SIZE; ++i)
		{
			Circle c1 = chooseFromPopulation();
			Circle c2 = chooseFromPopulation();
			Circle offspring = crossover(c1, c2);
			mutate(offspring);
			nextGeneration[i] = offspring;
		}
		for (int i = 0; i < POPULATION_SIZE; ++i)
		{
			candidatePopulation[i] = nextGeneration[i];
			candidateCircles[i] = get3PointCircle(nextGeneration[i]);
		}
		gen++;
	}
	printf("fitVal: %lf\n", fitnessValue(bestCandidate));
	cvZero(debug);
	cvCopy(canny_image, debug);
	cvCircle(debug, cvPoint(bestCandidate.x, bestCandidate.y), bestCandidate.r, CV_RGB(255, 255, 255));
	cvCircle(debug, cvPoint(bestCandidate.x, bestCandidate.y), 2, CV_RGB(255, 255, 255), 1);
	cvShowImage("debug", debug);
	cvWaitKey();
}

void balldetector::drawPopulation()
{
	for (int i = 0; i < POPULATION_SIZE; ++i)
	{
		cvCircle(debug, cvPoint(candidateCircles[i].x, candidateCircles[i].y), 2, CV_RGB(255, 255, 255));
		// printf("x: %d y: %d\n", candidateCircles[i].x, candidateCircles[i].y);
	}
}

void balldetector::drawTestSet()
{
	cvZero(debug);
	for (int i = 0; i < Ns; ++i)
		cvCircle(debug, cvPoint(testSet[i].x, testSet[i].y), 2, CV_RGB(255, 255, 255));
}