/*
This code is base on the thinning algorithm provided by T. Y. ZHANG and C. Y. SUEN 
Reference [1]: http://www-prima.inrialpes.fr/perso/Tran/Draft/gateway.cfm.pdf
Code Reference [2]: https://opencv-code.com/quick-tips/implementation-of-thinning-algorithm-in-opencv/
modified a bit to suit our needs
needed as a precomputation before LSD (Linear Time Line Segment Detector)
Reference [3]: http://www.ipol.im/pub/art/2012/gjmr-lsd/article.pdf
Reference [4]: http://www.ipol.im/pub/art/2012/gjmr-lsd/

MAKE SURE-> when the image is not square segmentation fault occurs
            I dont't know why -> (NEED TO FIGURE OUT ????)

NOTE->      number of iterations can be varied but it has been seen that
            after a point no effect is occuring on the image
            hence there is no point in running the code multiple times
            TODO-> need to find a viable threshold for the number_of_iterations
*/
#include <opencv2/opencv.hpp>
#include <opencv/cv.h>
#include <cstdio>
#include <string.h>
using namespace cv;
#define returnPixel1C(image, x, y) ((uchar*)(image->imageData + image->widthStep*(y)))[x]
#define number_of_iterations 6
int number_of_deletions = -1;                                                               //to break the loop when number_of_deletions is very less

void thinningZhangSuenIteration(IplImage* im, int iter)                                     //each iteration consists of 2 sub-iterations
{                                                                                           //one for even iter and one for odd both must be executed
    IplImage* marker = cvCreateImage(cvGetSize(im),8,1);
    cvZero(marker);
    if(!iter)
        number_of_deletions = 0;
    for (int i = 1; i < im->height; i++)
    {
        for (int j = 1; j < im->width; j++)
        {
            int p2 = returnPixel1C(im,i-1, j)/255;                                          //image here is either 255 or 0 hence converted to either 0 or 1
            int p3 = returnPixel1C(im,i-1, j+1)/255;
            int p4 = returnPixel1C(im,i, j+1)/255;
            int p5 = returnPixel1C(im,i+1, j+1)/255;
            int p6 = returnPixel1C(im,i+1, j)/255;
            int p7 = returnPixel1C(im,i+1, j-1)/255;
            int p8 = returnPixel1C(im,i, j-1)/255;
            int p9 = returnPixel1C(im,i-1, j-1)/255;

            int A  = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) + 
                     (p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) + 
                     (p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
                     (p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);
            int B  = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
            int m1 = iter == 0 ? (p2 * p4 * p6) : (p2 * p4 * p8);
            int m2 = iter == 0 ? (p4 * p6 * p8) : (p2 * p6 * p8);

            if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0)
            {
                returnPixel1C(marker,i,j) = 255;                                            //marking pixels to be deleted
                number_of_deletions++;
            }
        }
    }
    // printf("number_of_deletions: %d\niter: %d\n", number_of_deletions, iter);
    // im &= ~marker;
    // cvShowImage("marker",marker);
    // cvWaitKey();
    cvNot(marker,marker);                                                                   //inverting the marker image so that the pixels from the actual image
    // cvShowImage("marker",marker);                                                        //can be deleted
    // cvWaitKey();
    cvAnd(im,marker,im);                                                                    //deleting pixels using cvAnd() function
    // cvShowImage("im",im);
    // cvWaitKey();
    cvReleaseImage(&marker);
}

int main(int argc, char const *argv[])
{
	if(argc < 2)
	{
		printf("Usage: \n%s <Filename>\n", argv[0]);
		exit(1);
	}
	IplImage* frame = cvLoadImage(argv[1],CV_LOAD_IMAGE_GRAYSCALE);
    // cvNot(frame,frame);
    cvInRangeS(frame,cvScalar(180,0,0),cvScalar(255,0,0),frame);                            //segmenting white part (cvScalar values have been chosen arbitrarily)
    cvShowImage("frame",frame);
    cvWaitKey();
	for (int i = 0; i < number_of_iterations; ++i)
	{
		if(!number_of_deletions)
			break;
        printf("number_of_iterations: %d number_of_deletions: %d\n", i+1, number_of_deletions);
		thinningZhangSuenIteration(frame,0);                                                  //even sub-iteration
		thinningZhangSuenIteration(frame,1);                                                  //odd sub-iteration
	}

	string str(argv[1]);                                                                      //saving image
	int i;
	for (i = 0; i < strlen(argv[1]); ++i)
		if(argv[1][i] == '.')
			break;
	str.insert(i,"_thin");
	cvSaveImage(const_cast<char*>(str.c_str()),frame);
	return 0;
}