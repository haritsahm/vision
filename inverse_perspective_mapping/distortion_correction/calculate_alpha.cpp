/*	
	TODO -> change all values in structure "parameters" to double
	to avoid data definition conflicts in AcYut

	TODO -> formatting and proper commenting in all inverse_perspective mapping
	codes 

	ASSUMPTION -> that overall the net optimal f values are increasing (this is done because the optimal f values are NOT
																		constantly increasing as motor_value is changed)
	across all the values in the file if this not the case
	adjust the value of "prev" suitably
	or else no values will be generated in the file

	ASSUMPTION -> the first blob will be at 40cm from the camera
	consequently the other 2 blobs will be placed at 80cm
	and 120cm respectively
	pix2cmx and pix2cmy assume the distance difference to 40cm

	MAKE SURE -> the image supplied is of size 320x240
	and also that these constants are used in the exact same
	sized image (not sure why linear interpolation of constants to
	other image sizes does not work) [for example 320x240 constants will not 
	work on 640x480 image even if we multiply pix2cmx and pix2cmy by 2] <- [TODO -> find a reason for that]

	MAKE SURE -> the segmented images seg_red and seg_yellow
	in featuredetection.cpp are of size 320x240 or else
	distance to the goal post and/or the ball will come
	out to be wrong.

	MAKE SURE -> it is essential that the motor number 17 (neck motor)
	is being read correctly [take_picture.cpp] entire calculation is dependent on that fact

	ALSO -> in "calculate_constants_objdis.cpp" we have not sorted the blob coordinates
	as in cvblob the blobs are detected from the top to bottom and hence sorting is not required
	but in "calculate_constants_perpend.cpp" the blobs are horizontally arranged hence sorting is "ESSENTIAL"
*/

//Input-Output/Common Header Files
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <sstream>
#include <stdio.h>
#include <math.h>
#include <cmath>
//OpenCV Headers
#include <opencv/cv.h>
#include <opencv/highgui.h>
//Blob Library Headers
#include <cvblob.h>

//Definitions
#define WIDTHd 320
#define HEIGHTd 240
#define HEIGHTu 320
#define WIDTHu 240
#define PI 3.14159265359
#define rad2deg(x) x*180/PI
//NameSpaces
using namespace std;
using namespace cvb;
//STRUCTURE FOR PARAMETERS 
struct parameters												//everwhere this structure needs to be defined
{
	int motor_pos;
	float angle;
	float focal;
	float pix2cmx;
	float pix2cmy;
	float s_view_compensation;
}entry;

int XU[3];														//undistorted coordinates x - axis
int YU[3];														//undistorted coordinates y - axis
int Y[3];														//objdis in pixel differences
int diff[2];													//pixel differences
int prev=0;														//required to remove the unwanted spikes in the graph made from the constants calculated
																//using the images taken from the take picture code

double ax;														//distortion constants at which the fisheye image looks linear
void getLinearCoords(int xd, int yd, int* xu, int* yu)			//function to convert image coordinates from distorted fisheye
{																//image to image coordinates from undistorted linear image
	double r2 = xd*xd +yd*yd;	
	*xu = xd/(1+ax*r2);
	*yu = yd/(1+ax*r2);
}

void getPt(int u, int v, float f, int i,float x)
{
	float s=1.0;												//pixel width kept constant so that we can vary a single constant
	float theta= x/180.0*PI;									//theta in radians

	float objdis=(v+(f/s)*tan(theta))/(1-((v*s)/f)*tan(theta));	//inverse perspective mapping formulas
	float perpend=u*((s/f)*objdis*sin(theta)+cos(theta));		//for objdis and perpend
	Y[i]=int(objdis);											//where objdis is the distance along the line of sight
}																//of AcYut and perpend is the distance perpendicular to it

void sort_Y(int arrayToSort[],int arrayToSortSize)				//in case the blobs detected are not in the particular order
{																//will be used in the calculate_constants_perpend.cpp 
	for(int i=1; i<arrayToSortSize; ++i) {						//code
	    int* first = arrayToSort;
	    int* end = arrayToSort + arrayToSortSize;

	    for(first; first!=end-i; ++first) {
	        if (*first > *(first+1)) {
	            int temp = *first;
	            *first = *(first+1);
	            *(first+1) = temp;
	        }   
	    }
	}
}

void converttotext()											//converts the binary file to text for easy debugging
{
	ifstream constants("acyut_constants_objdis.dat",ios::binary);
	ofstream output("acyut_constants_objdis.txt");
	while(1)
	{
		if(constants.eof())
			break;

		constants.read((char*)&entry,sizeof(entry));
		
		output<<entry.motor_pos<<"\t\t\t"<<setprecision(5)<<entry.angle<<"\t\t\t"
				<<setprecision(5)<<entry.focal<<"\t\t\t"<<setprecision(5)<<entry.pix2cmy<<"\t\t\t"
				<<setprecision(5)<<entry.s_view_compensation<<"\n";
	}
	constants.close();
	output.close();
}

int main()
{
	//Image Variables
	IplImage* img_hsv = cvCreateImage(cvSize(320, 240), 8, 3);					//Image in HSV color space
	IplImage* threshy = cvCreateImage(cvSize(320, 240), 8, 1);					//Threshed Image
	IplImage* labelImg = cvCreateImage(cvSize(320, 240), IPL_DEPTH_LABEL, 1);	//Image Variable for blobs
	IplImage* debug_console = cvCreateImage(cvSize(50,50), 8, 1);				//Display values across code

	CvBlobs blobs;
	int xu, yu;													//coordinates of undistorted image
	int xd, yd;													//coordinates in distorted image
	int detected_blobs;
	ifstream filenames ("filenames_objdis.txt");
	// ofstream constants ("acyut_constants_objdis.dat",ios::binary);
	double optimal_ax;
	double min_ax = 1000.0;
	for (ax = -4.0e-06; ax > -10.0e-06 ; ax-=0.1e-06)
	{
		if(filenames.eof())
			break;

		char filename[20];		
		// getline(filenames,filename);
		filenames>>filename;
		char motor_loc[3];
		int j=0;

		for(int i=0;;++i)										//get motor location from filename
		{
			if(filename[i]=='.')
				break;
			motor_loc[i]=filename[i];
		}			

		// string optimum_focal;
		// string pix2cmyf;
		// string angle_s;
		float optimum_f=0;
		int min=1000;
		float pix2cmy;
		int pixel_difference=0;
		float angle;
		int motor;

		IplImage* frame = cvLoadImage(filename);
		// cvAddS(frame, cvScalar(70,70,70), frame);
		cvCvtColor(frame,img_hsv,CV_BGR2HSV);				
		//Thresholding the frame for yellow
		//cvInRangeS(img_hsv, cvScalar(20, 100, 20), cvScalar(30, 255, 255), threshy);				
		cvInRangeS(img_hsv, cvScalar(0, 60, 100), cvScalar(255, 255, 255), threshy);
		//Filtering the frame - subsampling??
		cvSmooth(threshy,threshy,CV_MEDIAN,7,7);
		int counter = 0;

		while(counter!=1)
		{
			//Finding the blobs
			unsigned int result = cvLabel(threshy,labelImg,blobs);
			//Filtering the blobs
			cvFilterByArea(blobs,100,10000);
			//Rendering the blobs
			cvRenderBlobs(labelImg,blobs,frame,frame);
			cvShowImage("frame",frame);
			cvWaitKey(5);

			int i=0;
			for (CvBlobs::const_iterator it=blobs.begin(); it!=blobs.end(); ++it)
			{			
				xd = (it->second->maxx+it->second->minx)/2;
				yd = (it->second->maxy+it->second->miny)/2;
				xd = xd - 320/2;
				yd = -yd + 240/2;
				// cout<<"non-linear coords: xd="<<xd<<"     yd="<<yd<<endl;
				getLinearCoords(xd, yd, &xu, &yu);
				XU[i]=xu;
				YU[i]=yu;
				++i;
			}
			detected_blobs=i;

			if ( detected_blobs!=3 )							//if the number of blobs detected will be not equal to 3
				break;											//then the program will not consider to that instance
			int motor_pos=0;
			for(int k=0;k<3;++k)								//calculate motor_pos from motor_loc
			{
				int x;
				x=int(motor_loc[k]-'0');
				motor_pos=(motor_pos*10)+x;
			}
			motor=motor_pos;
			angle=((-512.0+motor_pos)/1024.0)*300.0;			//can depend on how you have attached the camera to the motor
																//basically required to calculate the angle at which IPM needs
			cout<<"\ndetected blobs :"<<detected_blobs;			//to be calculated
			cout<<"\nmotor_pos :"<<motor_pos;			
			cout<<"\n"<<"angle :"<<angle;					

			for(float f=10.0;f<1500.0;f+=0.01)					//calculate constants and optimal focal length
			{													//it scans for f from 10 - 1500 to check where
				for (int j=0;j<detected_blobs;++j)				//the deviation in pixel difference is minimum
					getPt(XU[j],YU[j],f,j,angle);				//for that value pix2cmy and side view compensation
																//constant is evaluated (assuming that the first blob
				// sort_Y(Y,3);									//is at a distance of 40cm from the camera)			
				for (int z=0;z<2;++z)
					diff[z]= abs(Y[z+1]-Y[z]);
				// sort_diff(diff);
				if (min>=abs(diff[1] - diff[0]))
				{
					min=abs(diff[1] - diff[0]);
					optimum_f=f;
					pixel_difference= abs(diff[1]);
				}
			}
			++counter;
			if(min==1000)
			break;
		}
		pix2cmy=40.0/pixel_difference;							//calculated based on the ASSUMPTION that individual blobs
																//are placed at a distance of 40 cm
		for(int h=0;h<detected_blobs;++h)
			getPt(XU[h],YU[h],optimum_f,h,angle);

		for(int l=0;l<3;++l)
			Y[l]=Y[l]*pix2cmy;
		
		// cout<<"\nOptimum Focal Length :"<<optimum_f;
		// cout<<"\nmin deviation :"<<min;
		// cout<<"\npix2cmy :"<<pix2cmy;
		// cout<<"\nDistance :";

		// for(int l=0;l<3;++l)
		// 	cout<<Y[l]<<"\t\t";

		// cout<<"\nDifference :";
		
		// for(int l=0;l<2;++l)
		// 	cout<<Y[l+1]-Y[l]<<"\t\t";
		
		// cout<<"\n\n\n";

		entry.motor_pos=motor;
		entry.angle=angle;
		entry.focal=optimum_f;
		entry.pix2cmy=pix2cmy;
		entry.s_view_compensation=-(Y[2]-40.0);

  //       if(prev<optimum_f)										//ASSUMPTION that overall the net f values are increasing
  //       {
  //       	constants.write((char*)&entry,sizeof(entry));		//writing to file
		// 	prev=optimum_f;
		// }
		cvZero(frame);
		cvZero(img_hsv);
		cvZero(threshy);
		cvZero(debug_console);
		frame = cvLoadImage(filename);
		// cvAddS(frame, cvScalar(70,70,70), frame);
		cvCvtColor(frame,img_hsv,CV_BGR2HSV);				
		//Thresholding the frame for yellow
		cvInRangeS(img_hsv, cvScalar(20, 100, 20), cvScalar(30, 255, 255), threshy);				
		// cvInRangeS(img_hsv, cvScalar(0, 60, 100), cvScalar(255, 255, 255), threshy);
		//Filtering the frame - subsampling??
		cvSmooth(threshy,threshy,CV_MEDIAN,7,7);
		//Finding the blobs
		unsigned int result = cvLabel(threshy,labelImg,blobs);
		//Filtering the blobs
		cvFilterByArea(blobs,100,10000);
		//Rendering the blobs
		cvRenderBlobs(labelImg,blobs,frame,frame);
		for (CvBlobs::const_iterator it=blobs.begin(); it!=blobs.end(); ++it)
		{			
			xd = (it->second->maxx+it->second->minx)/2;
			yd = (it->second->maxy+it->second->miny)/2;
			xd = xd - 320/2;
			yd = -yd + 240/2;
			// cout<<"non-linear coords: xd="<<xd<<"     yd="<<yd<<endl;
			getLinearCoords(xd, yd, &xu, &yu);
			getPt(xu,yu,entry.focal,0,entry.angle);
			Y[0] = Y[0]*entry.pix2cmy + entry.s_view_compensation;
			if(abs(Y[0] - 250.0) < min_ax)
			{
				optimal_ax = ax;
				min_ax = abs(Y[0] - 250.0);
			}
		}
		CvFont font;
		cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5);
		char A[500];
		sprintf(A,"CURRENT ax : %lf\nOPTIMAL ax : %lf\nMIN_AX : %lf\n\nOPTIMAL FOCAL : %f\nANGLE : %f\nMOTOR POS : %d\nPIX2CMY : %f\n",
				ax,optimal_ax,min_ax,entry.focal,entry.angle,entry.motor_pos,entry.pix2cmy);
		cvPutText(debug_console, A, cvPoint(10,10), &font, cvScalar(255));
		cvShowImage("debug_console",debug_console);
		cvWaitKey(5);
	}
	cvWaitKey();
	// constants.close();
	filenames.close();
	// converttotext();	
	return 0;
}