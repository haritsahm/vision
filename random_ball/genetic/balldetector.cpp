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
		candidatePopulation[i] = get3PointCircle(edgePoints[pos1], edgePoints[pos2], edgePoints[pos3]);
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

Circle balldetector::get3PointCircle(CvPoint pi, CvPoint pj, CvPoint pk)
{
	double xi = pi.x;
	double yi = pi.y;
	double xj = pj.x;
	double yj = pj.y;
	double xk = pk.x;
	double yk = pk.y;
	double x0, y0, r;
	double ri = (xi*xi + yi*yi);
	double rj = (xj*xj + yj*yj);
	double rk = (xk*xk + yk*yk);

	x0 = (2.0 * (rj - ri) * (yk - yi)) - (2.0 * (rk - ri) * (yj - yi));
	x0 /= 4.0 * ((xj - xi) * (yk - yi) - (xk - xi) * (yj - yi));

	y0 = (2.0 * (xj - xi) * (rk - ri)) - (2.0 * (xk - xi) * (rj - ri));
	y0 /= 4.0 * ((xj - xi) * (yk - yi) - (xk - xi) * (yj - yi));

	r = sqrt((xi - x0)*(xi - x0) + (yi - y0)*(yi - y0));

	Circle c;
	c.x = (int) x0;
	c.y = (int) y0;
	c.r = (int) r;

	return c;
}

void balldetector::addToTestSet(CvPoint p, Circle c)
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

void balldetector::generateTestSet(Circle c)
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

double balldetector::fitnessValue(Circle c)
{
	if(!isValidCircle(c))
		return 0.0001;
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
		fitnessPopulation[i] = fitnessValue(candidatePopulation[i]);
		normFactor += fitnessPopulation[i];
	}
	//Normalising
	for (int i = 0; i < POPULATION_SIZE; ++i)
		fitnessPopulation[i] /= normFactor;
}

void balldetector::computeRouletteProb()
{
	rouletteWheel[0] = 1.0/fitnessPopulation[0];
	double sum = rouletteWheel[0];
	for (int i = 1; i < POPULATION_SIZE; ++i)
	{
		rouletteWheel[i] = rouletteWheel[i-1] + 1.0/fitnessPopulation[i];
		sum += rouletteWheel[i];
	}
	for (int i = 0; i < POPULATION_SIZE; ++i)
	{
		rouletteWheel[i] /= sum;
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

Circle balldetector::crossover(Circle &c1, Circle &c2)
{
	Circle dominant, recessive, offspring;
	if(fitnessValue(c1) > fitnessValue(c2))
	{
		dominant = c1;
		recessive = c2;
	}
	else
	{
		dominant = c2;
		recessive = c1;
	}
	double x1 = dominant.x;
	double y1 = dominant.y;
	double r1 = dominant.r;
	double x2 = recessive.x;
	double y2 = recessive.y;
	double r2 = recessive.r;

	offspring.x = x1 + rand2()*(x1 - x2);
	offspring.y = y1 + rand2()*(y1 - y2);
	offspring.r = r1 + rand2()*(r1 - r2);
	return offspring;
}

void balldetector::mutate(Circle &c)
{
	if(rand1() > MUTATION_PROBABILITY)
		return;
	int offsetx = ((rand2())*MUTATION_STEP);
	int offsety = ((rand2())*MUTATION_STEP);
	int offsetr = ((rand2())*MUTATION_STEP);
	c.x += offsetx;
	c.y += offsety;
	c.r += offsetr;
}

bool balldetector::isValidCircle(Circle c)
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
		// printf("generation: %d\n", gen);
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
		bestCandidate = eliteIndiv;

		for (int i = 1; i < POPULATION_SIZE; ++i)
		{
			Circle c1 = chooseFromPopulation();
			Circle c2 = chooseFromPopulation();
			Circle offspring = crossover(c1, c2);
			mutate(offspring);
		}
		for (int i = 0; i < POPULATION_SIZE; ++i)
			candidatePopulation[i] = nextGeneration[i];
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
		cvCircle(debug, cvPoint(candidatePopulation[i].x, candidatePopulation[i].y), 2, CV_RGB(255, 255, 255));
		// printf("x: %d y: %d\n", candidatePopulation[i].x, candidatePopulation[i].y);
	}
}

void balldetector::drawTestSet()
{
	cvZero(debug);
	for (int i = 0; i < Ns; ++i)
		cvCircle(debug, cvPoint(testSet[i].x, testSet[i].y), 2, CV_RGB(255, 255, 255));
}