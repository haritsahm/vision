#include <opencv2/opencv.hpp>
#include <opencv/cv.h>
#include <cstdio>

using namespace cv;
#define testcase_width 9
#define testcase_height 8

bool testcase[testcase_height][testcase_width]=	{									//test image to be reduced to skeleton
						{0,0,0,0,0,0,0,0,0},
						{0,1,1,1,1,1,1,1,0},
						{0,1,1,1,1,1,1,1,0},
						{0,1,1,1,1,1,1,1,0},
						{0,0,0,1,1,1,0,0,0},
						{0,0,0,1,1,1,0,0,0},
						{0,0,0,1,1,1,0,0,0},
						{0,0,0,0,0,0,0,0,0},
						};
/**
 * Perform one thinning iteration.
 * Normally you wouldn't call this function directly from your code.
 *
 * @param  im    Binary image with range = 0-1
 * @param  iter  0=even, 1=odd
 */
void thinningGuoHallIteration(bool testcase[testcase_height][testcase_width], int iter, IplImage* iteration)
{
    bool marker[testcase_height][testcase_width] = {0};

    for (int i = 1; i < testcase_height-1; i++)
    {
        for (int j = 1; j < testcase_width-1; j++)
        {

            bool p2 = testcase[i-1][j];
            bool p3 = testcase[i-1][j+1];
            bool p4 = testcase[i][j+1];
            bool p5 = testcase[i+1][j+1];
            bool p6 = testcase[i+1][j];
            bool p7 = testcase[i+1][j-1];
            bool p8 = testcase[i][j-1]; 
            bool p9 = testcase[i-1][j-1];

            int C  = (!p2 & (p3 | p4)) + (!p4 & (p5 | p6)) +
                     (!p6 & (p7 | p8)) + (!p8 & (p9 | p2));
            int N1 = (p9 | p2) + (p3 | p4) + (p5 | p6) + (p7 | p8);
            int N2 = (p2 | p3) + (p4 | p5) + (p6 | p7) + (p8 | p9);
            int N  = N1 < N2 ? N1 : N2;
            int m  = iter == 0 ? ((p6 | p7 | !p9) & p8) : ((p2 | p3 | !p5) & p4);

            if (C == 1 && (N >= 2 && N <= 3) & m == 0)
                marker[i][j] = 1;
        }
    }
    for (int i = 0; i < testcase_height; ++i)
    {
        for (int j = 0; j < testcase_width; ++j)
            testcase[i][j] &= ~marker[i][j];
    }
    // testcase &= ~marker;
    // bitwise_and(testcase,bitwise_or(marker,marker));
    for (int i = 0; i < testcase_height; ++i)
    {
        for (int j = 0; j < testcase_width; ++j)
        {
            if(testcase[i][j])
             cvCircle(iteration,cvPoint(40*(j) + 20,40*(i) + 20),17,CV_RGB(0,255,0));
            else
             cvCircle(iteration,cvPoint(40*(j) + 20,40*(i) + 20),17,CV_RGB(255,0,0));
            cvShowImage("iteration",iteration);
            cvMoveWindow("iteration",500,100);
            cvWaitKey();
        }
    }
}

int main()
{
	IplImage* sample = cvCreateImage(cvSize(testcase_width*40,testcase_height*40), 8, 3);
	IplImage* iteration = cvCreateImage(cvSize(testcase_width*40,testcase_height*40), 8, 3);
	cvZero(sample);
	cvZero(iteration);
	for (int i = 0; i < testcase_height; ++i)
	{
		for (int j = 0; j < testcase_width; ++j)
		{
			// cvLine(sample,cvPoint(0,40*i),cvPoint(360,40*i),CV_RGB(255,255,255));
			// cvLine(iteration,cvPoint(0,40*i),cvPoint(360,40*i),CV_RGB(255,255,255));
			if(testcase[i][j])
				cvCircle(sample,cvPoint(40*(j) + 20,40*(i) + 20),17,CV_RGB(0,255,0));
			else
				cvCircle(sample,cvPoint(40*(j) + 20,40*(i) + 20),17,CV_RGB(255,0,0));
		}
	}
	cvShowImage("sample",sample);
	cvMoveWindow("sample",100,100);
	thinningGuoHallIteration(testcase,0,iteration);
	thinningGuoHallIteration(testcase,1,iteration);
	cvWaitKey();
    return 0;
}