#include <stdio.h>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc_c.h>

using namespace std;
int main(int argc, char** argv)
{
  CvMemStorage* storage = cvCreateMemStorage(0);
cvNamedWindow("Image", 1);
int key = 0;
static CvScalar red_color[] ={0,0,255};
CvCapture* capture = cvCreateCameraCapture(0);
CvMat* prevgray = 0, *image = 0, *gray =0;
while( key != 'q' )
{
int firstFrame = gray == 0;
IplImage* frame = cvQueryFrame(capture);
if(!frame)
break;
if(!gray)
{
image = cvCreateMat(frame->height, frame->width, CV_8UC1);
}
//Convert the RGB image obtained from camera into Grayscale
cvCvtColor(frame, image, CV_BGR2GRAY);
//Define sequence for storing surf keypoints and descriptors
CvSeq *imageKeypoints = 0, *imageDescriptors = 0;
int i;

//Extract SURF points by initializing parameters
CvSURFParams params = cvSURFParams(300, 1);
cvExtractSURF( image, 0, &imageKeypoints, &imageDescriptors, storage, params );
printf("Image Descriptors: %d\n", imageDescriptors->total);

//draw the keypoints on the captured frame
for( i = 0; i < imageKeypoints->total; i++ )
{
CvSURFPoint* r = (CvSURFPoint*)cvGetSeqElem( imageKeypoints, i );
CvPoint center;
int radius;
center.x = cvRound(r->pt.x);
center.y = cvRound(r->pt.y);
radius = cvRound(r->size*1.2/9.*2);
cvCircle( frame, center, radius, red_color[0], 1, 8, 0 );
}
cvShowImage( "Image", frame );

cvWaitKey(10);
}
cvDestroyWindow("Image");
return 0;
}