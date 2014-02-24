#include <stdio.h>
#include "featuredetection.h"
#include <opencv2/opencv.hpp>
//#include <FlyCapture2.h>
#include "ueye.h"
#include "camcapture.h"
#include "headmotor.h"
#include "camcontrol.h"
#include "localize.h"

#include "imgproc.h"
#include "featuredetection.h"
#include <time.h>

#define SHOW_NODE

#define IMAGE_WIDTH 320//640/2
#define IMAGE_HEIGHT 240//480/2
#define R 120

using namespace cvb;
using namespace tbb;
using namespace LOCALIZE_INTERNALS;
using namespace std;

//4 color class - red, green, white, black
IplImage* seg_white_count1 = cvCreateImage(cvSize(IMAGE_WIDTH/4, IMAGE_HEIGHT/4), 8, 1);	//white with density
IplImage* seg_white1 = cvCreateImage(cvSize(IMAGE_WIDTH/4, IMAGE_HEIGHT/4), 8, 1);			//white
IplImage* seg_black1 = cvCreateImage(cvSize(IMAGE_WIDTH/4, IMAGE_HEIGHT/4), 8, 1);
IplImage* seg_green1 = cvCreateImage(cvSize(IMAGE_WIDTH/4, IMAGE_HEIGHT/4), 8, 1);

IplImage* skeleton = cvCreateImage(cvSize(IMAGE_WIDTH/4, IMAGE_HEIGHT/4), 8, 1);
IplImage* node_img = cvCreateImage(cvSize(IMAGE_WIDTH/4, IMAGE_HEIGHT/4), 8, 1);

int bound[IMAGE_WIDTH/4];

struct Point
{
	int x;
	int y;
};

struct Node
{
	Point p;
	Point c[4];		//connected points
	int degree;
};

void getBoundary(CamCapture &camera)
{
	// IplImage* show_image3 = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), 8, 3);			//show full size rgb image
	// IplImage* show_image = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), 8, 1);			//show full size rgb image
	// cvZero(show_image);
	IplImage* probImage = cvCreateImage(cvSize(IMAGE_WIDTH/4,IMAGE_HEIGHT/4), 8, 1);		//probab of being inside field
	cvZero(probImage);

	int rowsum[IMAGE_HEIGHT/4] = {};
	

	for(int x=0; x<IMAGE_WIDTH/4; x++)
	{
		for(int y=0; y<IMAGE_HEIGHT/4; y++)
		{
			int r_count=0, g_count=0, w_count=0, b_count=0;

			if((x-IMAGE_WIDTH/8)*(x-IMAGE_WIDTH/8) + (-y+IMAGE_HEIGHT/8)*(-y+IMAGE_HEIGHT/8) < R*R/16)
			{
				for(int xx=0; xx<4; xx++)
				{
					for(int yy=0; yy<4; yy++)
					{
						int tx = x*4 + xx;
						int ty = y*4 + yy;
						if(camera.isRed_small(tx, ty))
							r_count++;
						if(camera.isGreen_small(tx, ty))
							g_count++;
			 			if(camera.isWhite_small(tx, ty))
							w_count++;
						if(camera.isBlack_small(tx, ty))
							b_count++;
					}
				}
			}
			returnPixel1C(probImage, x, y) = (r_count*16 + g_count*8 + w_count + b_count);
			//white
			if(w_count == 16)
				returnPixel1C(seg_white_count1, x, y) = 255;
			else
				returnPixel1C(seg_white_count1, x, y) = (w_count*16)%256;
			if(w_count>4)
			{
				returnPixel1C(seg_white1, x, y) = 255;
				
				#ifdef SHOW_SUBSAMPLE
					returnPixel3C(subSampled, x, y, 0) = 255;
					returnPixel3C(subSampled, x, y, 1) = 255;
					returnPixel3C(subSampled, x, y, 2) = 255;
				#endif
			}				

			//black
			if(b_count>4)
			{
				returnPixel1C(seg_black1, x, y) = 255;
				
				#ifdef SHOW_SUBSAMPLE
					returnPixel3C(subSampled, x, y, 0) = 0;
					returnPixel3C(subSampled, x, y, 1) = 0;
					returnPixel3C(subSampled, x, y, 2) = 0;
				#endif
			}

			//green
			if(g_count>4)
			{
				returnPixel1C(seg_green1, x, y) = 255;
				#ifdef SHOW_SUBSAMPLE
					returnPixel3C(subSampled, x, y, 1) = 255;
				#endif
			}

			rowsum[y] += pixelColor1C(probImage, x, y);
		}
	}//probability image created

	//binarization of image - white(255) inside field, black(0) outside
	IplImage* binaryImage = cvCreateImage(cvSize(IMAGE_WIDTH/4,IMAGE_HEIGHT/4), 8, 1);		//binary img inside field
	cvZero(binaryImage);
	int tpix, trow, twin; 		//thresholds
	tpix = 30;					//minimum required probab image
	trow = IMAGE_WIDTH*4.5;
	twin = 36*18;

	for(int x=0; x<IMAGE_WIDTH/4; x++)
	{
		for(int y=0; y<IMAGE_HEIGHT/4; y++)
		{
			//check if point inside fisheye image
			if((x-IMAGE_WIDTH/8)*(x-IMAGE_WIDTH/8) + (-y+IMAGE_HEIGHT/8)*(-y+IMAGE_HEIGHT/8) > R*R/16)
				continue;

			//check if pixel passes minimum threshold
			if(pixelColor1C(probImage, x, y) < tpix)
			{
				returnPixel1C(binaryImage, x, y) = 0;
				continue;
			}
		
			//check if rowsum passes minimum threshold
			if(rowsum[y] >= trow)
			{
				returnPixel1C(binaryImage, x, y) = 255;
				continue;
			} 

			//else do the expensive test
			int sum = 0;
			for(int i = -4; i< 5; i++)
			{
				for(int j=-4; j<5; j++)
				{
					if( (x+i)<IMAGE_WIDTH/4 && (x+i)>=0 && (y+j)<IMAGE_HEIGHT/4 && (y+j)>=0 )
						sum += pixelColor1C(probImage, x+i, y+j);
				}
			}
			if (sum >= twin)
				returnPixel1C(binaryImage, x, y) = 255;
			else
				returnPixel1C(binaryImage, x, y) = 0;

		}
	}//binary image created
	// cvZero(show_image);
	// cvResize(binaryImage, show_image);
	// cvShowImage("binary", show_image);
	// cvWaitKey();

	//retrieving field boundary

	IplImage* histogram = cvCreateImage(cvSize(IMAGE_WIDTH/4, 1), 8, 1);			//image saves y boundary value for each x
	cvZero(histogram);
	returnPixel1C(histogram, 0, 0) = IMAGE_HEIGHT/4 - 1;
	for(int x = 1; x < IMAGE_WIDTH/4; x++)
	{
		int y=0, count=0;
		for(y = 0; y < IMAGE_HEIGHT/4; y++)
		{
			if(pixelColor1C(binaryImage, x, y))
			{
				count++;
				if(count==4)
					break;
			}
			else
				count=0;
		}
		if(count == 4)
			returnPixel1C(histogram, x, 0) = y;
		else
			returnPixel1C(histogram, x, 0) = IMAGE_HEIGHT/4 - 1;
	
	}
	

	//smoothning and gap filling
	cvSmooth(histogram, histogram, CV_GAUSSIAN, 3);
	cvSmooth(histogram, histogram, CV_MEDIAN, 3);
	
	//local convex hull
	IplImage* boundary_img = cvCreateImage(cvSize(IMAGE_WIDTH/4, IMAGE_HEIGHT/4), 8, 1);
	cvZero(boundary_img);
	for(int x=0; x<IMAGE_WIDTH/4; x++)
	{
		bound[x] = pixelColor1C(histogram, x, 0);
	}

	int convexity;
	CvPoint lastOnHull = cvPoint(0, bound[0]);
	for(int x=1; x<IMAGE_WIDTH/4 - 1; x++)
	{
		convexity = 2*(bound[x] - bound[x-1]) - (bound[x+1] - bound[x-1]);

		if(convexity > 0)
			continue;
		else
		{
			cvLine(boundary_img, lastOnHull, cvPoint(x, bound[x]), cvScalar(255,255,0), 1);			//connect
			lastOnHull = cvPoint(x, bound[x]);
		}
	}
	// cvZero(show_image);
	// cvResize(boundary_img, show_image);
	// cvShowImage("boundary", show_image);
	//cvWaitKey();

	//releaase memory
	//cvReleaseImage(&show_image);
	cvReleaseImage(&boundary_img);
	cvReleaseImage(&binaryImage);
	cvReleaseImage(&probImage);
	cvReleaseImage(&histogram);
}

void makeSkeleton(CamCapture &camera)
{
	//Skeletonization
	cvZero(skeleton);
	for(int x=0; x<IMAGE_WIDTH/4 - 1; x++)
	{
		int w_count;			//stores w_count of pixels of 3x3 pixel region
		int count;				//no. of neighbouring pixel wit w_count >= w_count of centre pixel
		int centre_w_count ;	//w_count of center pixel in 3x3 region
		for(int y=bound[x]; y<IMAGE_HEIGHT/4 - 1; y++)
		{
			count = 0;
			centre_w_count = pixelColor1C(seg_white_count1, x+1, y+1);
			
			if(centre_w_count == 0)
				continue;	//not a part of the skeleton
			
			for(int xx=0; xx<3; xx++)
			{
				for(int yy=0; yy<3; yy++)
				{
					int tx = x + xx;
					int ty = y + yy;
					if(tx<IMAGE_WIDTH && ty<IMAGE_HEIGHT) 
						w_count = pixelColor1C(seg_white_count1, tx, ty);
					else 
						w_count = 0;

					if (w_count >= centre_w_count)
						count++;
				}
			}
			
			if(count > 2)
				continue;	//not part of skeleton
			else
				returnPixel1C(skeleton, x+1, y+1) = 255-count;	//count is substarcted so that peaks (count = 0) can be distinguished i.e. peaks will have pixel value 255
		}
	}
	// IplImage* show_skeleton_image = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), 8, 1);			//show full size binary image
	// cvZero(show_skeleton_image);
	// cvResize(skeleton, show_skeleton_image);
	// cvShowImage("skeleton", show_skeleton_image);

	// cvReleaseImage(&show_skeleton_image);
}

void makeNodes(CamCapture &camera)
{
	cvZero(node_img);
	int no_of_nodes = 0;
	
	for(int x=0; x<IMAGE_WIDTH/4; x++)
	{
		for(int y=0; y<IMAGE_HEIGHT/4; y++)
		{
			int pixel = pixelColor1C(skeleton, x, y);
			if( pixel == 255)
			{
				returnPixel1C(node_img, x, y) = 255;		//place node - peaks
				no_of_nodes++;
			}
		}
	}
	// cout<<"\nno.of nodes = "<<no_of_nodes;

	for(int x=0; x<IMAGE_WIDTH/4; x++)
	{
		for(int y=0; y<IMAGE_HEIGHT/4; y++)
		{
			int pixel = pixelColor1C(skeleton, x, y);
			if(pixel == 254 || pixel == 253)
			{
				for(int xx=-2; xx<3; xx++)
				{
					for(int yy=-2; yy<3; yy++)
					{
						int tx = x+xx;
						int ty = y+yy;

						if(tx < 0 || ty<0 || tx >= IMAGE_WIDTH/4 || ty >= IMAGE_HEIGHT/4)
						 	continue;
						
						if(pixelColor1C(skeleton, tx, ty))
						{
							if(tx<2 && ty<2)
								continue;
							else
							{
								if(pixelColor1C(node_img, x, y) == 255)
									continue;
								returnPixel1C(node_img, x, y) = 255;		//place node
								no_of_nodes++;
							}
						}

					}
				}
			}
		}
	}
	// cout<<"\ntotal nodes = "<<no_of_nodes;

	// IplImage* show_node_image = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), 8, 1);			//show full size binary image
	// cvZero(show_node_image);
	// cvResize(node_img, show_node_image);
	// cvShowImage("node_img", show_node_image);
	// cvWaitKey();

	cout<<"no_of_nodes = "<<no_of_nodes<<endl;	
	Node node[no_of_nodes];				//array of nodes
	int node_index=0;

	for(int x=0; x<IMAGE_WIDTH/4; x++)
	{	
		for(int y=0; y<IMAGE_HEIGHT/4; y++)
		{
			if(pixelColor1C(node_img, x, y))
			{
				node[node_index].p.x = x;
				node[node_index].p.y = y;
				node[node_index].degree = 0;
				for(int i=0; i<4; i++)
					{
						node[node_index].c[i].x=-1;
						node[node_index].c[i].y=-1;
					}
				
				node_index++;
			}
		}
	}
	cout<<"node index = "<<node_index<<endl;
	cvWaitKey();

	//connecting nodes
	IplImage* reskeleton = cvCreateImage(cvSize(IMAGE_WIDTH/4, IMAGE_HEIGHT/4), 8, 3);
	cvZero(reskeleton);
	int continueFlag;	

		for(int i=0; i<no_of_nodes-1; i++)
		{
			for(int j=i+1; j<no_of_nodes-i; j++)
			{
				continueFlag = 0;
				if(abs(node[i].p.x - node[j].p.x) <= 2  && abs(node[i].p.y - node[j].p.y) <= 2 )
				{
					//check if already connected
					for(int t=0; t<4; t++)
					{
						if(node[i].c[t].x == node[j].p.x && node[i].c[t].y == node[j].p.y)
							continueFlag = 1;
					}
					if(continueFlag == 1)
						continue;

					//connect nodes
					cvLine(reskeleton, cvPoint(node[i].p.x, node[i].p.y), cvPoint(node[j].p.x, node[j].p.y), cvScalar(255,255,0), 1);

					node[i].c[node[i].degree] = node[j].p;
					node[j].c[node[j].degree] = node[i].p;

					//increse degree of both nodes

					node[i].degree += 1;
					node[j].degree += 1;
					if(node[i].degree > 4 || node[j].degree > 4)
					{
						cout<<"\n\ndegree exceded limit!!!\n\n";
						break;
					}
				}
			}
			//cout<<i+1<<".  node degree="<<node[i].degree<<endl;
		}
	IplImage* show_node_image = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), 8, 3);			//show full size binary image
	cvZero(show_node_image);
	cvResize(reskeleton, show_node_image);
	cvShowImage("reskeleon", show_node_image);
	//cvWaitKey();

	//Add conditions for elemination on nodes like degree greater thn 4 or no continuous line formation.  

	//Release memory
	cvReleaseImage(&reskeleton);
	cvReleaseImage(&show_node_image);	
}		

int main()
{
	char q='a';
	int count = 0;
	CamCapture camera(true, 100, 50);

	if(camera.init()==CAM_FAILURE)
	{
		printf("\n Exiting after init error\n");
		return 0;
	}

	while(1)
	{
		if(camera.getImage() == CAM_FAILURE)
		{
			count++;
			cout<<"CAM failed\n";
			if(count == 5)
			{
				cout<<"cannot get img\n";
				return 0;
			}
			else
			continue;
		}
		count =0;

		cvShowImage("rgbimg", camera.rgbimg);
		
		getBoundary(camera);
		makeSkeleton(camera);
		makeNodes(camera);
		
		q = cvWaitKey(50);
		if(q=='q')
			break;
	}
	return 0;
}