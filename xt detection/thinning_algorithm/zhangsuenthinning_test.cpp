#include <opencv2/opencv.hpp>
#include <opencv/cv.h>
#include <cstdio>
#define testcase_width 29
#define testcase_height 29
#define zero    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define blank   {0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0}
#define filled  {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0}
#define number_of_iterations 10
bool testcase[testcase_height][testcase_width] = { zero,blank,blank,blank,blank,blank,blank,blank,blank,blank,blank,filled,filled,filled,filled,filled,filled,filled,blank,blank,blank,blank,blank,blank,blank,blank,blank,blank,zero };
using namespace cv;
// bool testcase[testcase_height][testcase_width] =	{									//test image to be reduced to skeleton
// 						{0,0,0,0,0,0,0,0,0},
// 						{0,1,1,1,1,1,1,1,0},
// 						{0,1,1,1,1,1,1,1,0},
// 						{0,1,1,1,1,1,1,1,0},
// 						{0,0,0,1,1,1,0,0,0},
// 						{0,0,0,1,1,1,0,0,0},
// 						{0,0,0,1,1,1,0,0,0},
// 						{0,0,0,0,0,0,0,0,0},
// 						};
/**
 * Perform one thinning iteration.
 * Normally you wouldn't call this function directly from your code.
 *
 * @param  im    Binary image with range = 0-1
 * @param  iter  0=even, 1=odd
 */
void thinningZhangSuenIteration(bool testcase[testcase_height][testcase_width], int iter, IplImage* iteration)
{
    bool marker[testcase_height][testcase_width] = {0};

    for (int i = 1; i < testcase_height - 1; i++)
    {
        for (int j = 1; j < testcase_width - 1; j++)
        {

            bool p2 = testcase[i-1][j];
            bool p3 = testcase[i-1][j+1];
            bool p4 = testcase[i][j+1];
            bool p5 = testcase[i+1][j+1];
            bool p6 = testcase[i+1][j];
            bool p7 = testcase[i+1][j-1];
            bool p8 = testcase[i][j-1];
            bool p9 = testcase[i-1][j-1];

            int A  = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) +
                     (p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) +
                     (p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
                     (p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);
            int B  = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
            int m1 = iter == 0 ? (p2 * p4 * p6) : (p2 * p4 * p8);
            int m2 = iter == 0 ? (p4 * p6 * p8) : (p2 * p6 * p8);

            if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0)
                marker[i][j] = 1;
        }
    }
    for (int i = 0; i < testcase_height; ++i)
    {
        for (int j = 0; j < testcase_width; ++j)
            testcase[i][j] &= ~marker[i][j];
    }
    // testcase &= ~marker;
    for (int i = 0; i < testcase_height; ++i)
    {
        for (int j = 0; j < testcase_width; ++j)
        {
            if(testcase[i][j])
             cvCircle(iteration,cvPoint(10*(j) + 5,10*(i) + 5),4,CV_RGB(0,255,0));
            else
             cvCircle(iteration,cvPoint(10*(j) + 5,10*(i) + 5),4,CV_RGB(255,0,0));
            cvShowImage("iteration",iteration);
            cvMoveWindow("iteration",500,100);
            // cvWaitKey();
        }
    }
}

int main()
{
	IplImage* sample = cvCreateImage(cvSize(testcase_width*10,testcase_height*10), 8, 3);
	IplImage* iteration = cvCreateImage(cvSize(testcase_width*10,testcase_height*10), 8, 3);
	cvZero(sample);
	cvZero(iteration);
	for (int i = 0; i < testcase_height; ++i)
	{
		for (int j = 0; j < testcase_width; ++j)
		{
			// cvLine(sample,cvPoint(0,10*i),cvPoint(360,10*i),CV_RGB(255,255,255));
			// cvLine(iteration,cvPoint(0,10*i),cvPoint(360,10*i),CV_RGB(255,255,255));
			if(testcase[i][j])
				cvCircle(sample,cvPoint(10*(j) + 5,10*(i) + 5),4,CV_RGB(0,255,0));
			else
				cvCircle(sample,cvPoint(10*(j) + 5,10*(i) + 5),4,CV_RGB(255,0,0));
		}
	}
	cvShowImage("sample",sample);
	cvMoveWindow("sample",100,100);
    for (int i = 0; i < number_of_iterations; ++i)
    {
        printf("iteration : %d\n", i + 1);
        thinningZhangSuenIteration(testcase,0,iteration);
        thinningZhangSuenIteration(testcase,1,iteration);
        cvWaitKey();
    }
    cvWaitKey();
}
