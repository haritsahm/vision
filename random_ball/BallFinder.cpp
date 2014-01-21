#include "BallFinder.h"
#include "common/StopWatch.h"
#include "localdefs.h"

#include <iostream>

BallFinder::BallFinder()
{
	mask = cvCreateImage( cvSize(640,480), IPL_DEPTH_8U, 1 );
}

BallFinder::~BallFinder()
{

  cvReleaseImage(&mask);

}

RGBInterval::RGBInterval(double rMin, double rMax,
						double gMin, double gMax,
						double bMin, double bMax) {
	// CV_RGB is a macro for the construction of a CvScalar. We use it because
	// OpenCV has an internal BGR representation, not RGB --- using CV_RGB makes
	// it so we don't have to remember the internal representation, or care
	// if this representation changes.
	this->min = CV_RGB(rMin,gMin,bMin);
	this->max = CV_RGB(rMax,gMax,bMax);
}

RGBInterval::~RGBInterval() {
	
}


int BallFinder::findBalls(IplImage* frame, RGBInterval& ballcolor, double *x, double *y, double *r) {
	int nBalls = 0;
	
	//Apply rgb threshold to find colored pixels.
  ODEBUG("BallFinder::findBalls | apply rgb threshold\n");
  cvInRangeS( frame, ballcolor.getMin(), ballcolor.getMax(), mask );
  
  // Find all connected components (connected regions of white pixels) in
	// the edge image. We don't care about holes. If we wanted to compute
	// holes as well, we would use CV_RETR_CCOMP instead of CV_RETR_EXTERNAL.
	// Finding contours modifies the source image, edges, but in this case
	// we don't care.
	compstorage = cvCreateMemStorage(0);
	contours = 0;
	cvFindContours( mask, compstorage, &contours, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE );
  
	// Find the connected component with area > 5000. (Area is the number
	// of pixels in the component.) This roughly corresponds to a marble.
	// Note that if you want to detect more than one marble, you will have
	// to change this algorithm.
	while (1) {
		// Check if we ran out of components... this should never happen.
		// If it does, you need to make this algorithm more robust.
		if (contours == 0) {
			printf("Processed all contours.\n");
      break;
		}
		
		// Get the area of the current contour. We take absolute value because
		// the sign of cvContourArea depends on the orientation of the contour.
		double area = fabs( cvContourArea(contours, CV_WHOLE_SEQ) );
		printf("%10.0f\n",area);
		
		// If the area is big enough, store information. Then,
		// advance to the next contour.
		if (area>80 && area<200) {
			// Fit an ellipse to the contour we found.
			double xc, yc, rc;
			double rMajor, rMinor;
			fitEllipse(contours,&xc,&yc,&rMajor,&rMinor);
			rc = rMajor;
			if(rMajor>5 && rMajor<8  && rMinor>5 && rMinor<8)
			{
				x[nBalls] = xc;
				y[nBalls] = yc;
				r[nBalls] = rc;
				nBalls++;
			}
		}
		
		contours = contours->h_next;
	}
	
  // Clear memory associated with the components.
	cvClearMemStorage(compstorage);
  
  return nBalls;
}

int BallFinder::processFrame(IplImage* frame, double *x, double *y, double *r, IplImage* result)
{
	RGBInterval myRGBInterval = RGBInterval(0,80,0,80,0,80);
	int nBalls = findBalls(frame,myRGBInterval,x,y,r);
	if (nBalls==0) {
		printf("No Balls found!\n");
		return 0;
	}
	
	cvCopy(mask,result);
	return nBalls;
}


// We assume that "contour" comes from a binary image. You should add one
// more argument and compute the angle of the major/minor axes as well.
void BallFinder::fitEllipse(CvSeq* contour, double *x, double *y, double *rMajor, double *rMinor) {
  CvMoments moment;
  cvMoments(contour,&moment,1);

  // Use raw (spatial) moments to get the centroid.
  *x = moment.m10/moment.m00;
  *y = moment.m01/moment.m00;

  // Use central moments to get the major and minor axes.
  double mu00 = cvGetCentralMoment(&moment,0,0);
  double mu11 = cvGetCentralMoment(&moment,1,1)/mu00;
  double mu20 = cvGetCentralMoment(&moment,2,0)/mu00;
  double mu02 = cvGetCentralMoment(&moment,0,2)/mu00;

  *rMajor = 2*sqrt(((mu20+mu02)+sqrt((4*mu11*mu11)+((mu20-mu02)*(mu20-mu02))))/2);
  *rMinor = 2*sqrt(((mu20+mu02)-sqrt((4*mu11*mu11)+((mu20-mu02)*(mu20-mu02))))/2);
}