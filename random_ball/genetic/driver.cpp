#include "balldetector.h"

int main(int argc, char const *argv[])
{
	if(argc < 2)
	{
		printf("USAGE ./%s <FILENAME>\n", argv[0]);
		return -1;
	}
	IplImage *img = cvLoadImage(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
	cvShowImage("img", img);
	BallDetector b(img);
	b.findBall();
	// Circle c;
	// c.x = 100;
	// c.y = 100;
	// c.r = 10;
	// IplImage *image = cvCreateImage(cvSize(400, 400), 8, 1);
	// cvZero(image);
	// cvCircle(image, cvPoint(200, 200), 100, CV_RGB(255, 255, 255));
	// for (int i = 0; i < 1000; ++i)
	// {
	// 	CvPoint p;
	// 	p.x = b.rand1()*400;
	// 	p.y = b.rand1()*400;
	// 	cvCircle(image, p, 1, CV_RGB(255, 255, 255));
	// }
	// cvSaveImage("test4.bmp", image);
	return 0;
}