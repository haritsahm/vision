#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <math.h>
#include <cmath>

//OpenCV Headers
#include <opencv/cv.h>
#include <opencv/highgui.h>
//Input-Output
#include <stdio.h>
//Blob Library Headers
#include <cvblob.h>

//Definitions
#define WIDTHd 752
#define HEIGHTd 480
#define HEIGHTu 752
#define WIDTHu 480
#define PI 3.14159265359
#define rad2deg(x) x*180/PI
//NameSpaces
using namespace std;
using namespace cvb;

struct parameters
{
	int motor_pos;
	float angle;
	float focal;
	float pix2cmx;
	float pix2cmy;
	float s_view_compensation;
}entry;

int XU[3];			//undistorted coordinates x - axis*/
int YU[3];			//undistorted coordinates y - axis*/

int Y[3];			//objdis in pixel differences*/
int diff[2];		//pixel differences*/

int prev=0;			//required to remove the unwanted spikes in the graph made from the constants calculated
					//using the images taken from the take picture code

double ax = -7.1e-06;										//distortion constants at which the fisheye image looks linear*/
void getLinearCoords(int xd, int yd, int* xu, int* yu)		//function to convert image coordinates from distorted fisheye
{															//image to image coordinates from undistorted linear image
	double r2 = xd*xd +yd*yd;	
	*xu = xd/(1+ax*r2);
	*yu = yd/(1+ax*r2);
}

void getPt(int u, int v, float f, int i,float x)
{
	float s=1.0;					//pixel width kept constant so that we can vary a single constant
	float theta= x/180.0*PI;		//theta in radians

	float objdis=(v+(f/s)*tan(theta))/(1-((v*s)/f)*tan(theta));	//inverse perspective mapping formulas
	float perpend=u*((s/f)*objdis*sin(theta)+cos(theta));		//for objdis and perpend
	Y[i]=int(objdis);											//where objdis is the distance along the line of sight
}																//of AcYut and perpend is the distance perpendicular to it*/

// void sort_diff(int x[2])
// {
// 	int temp;
// 	if(x[1]>x[2])
// 	{
// 		temp=x[1];
// 		x[1]=x[2];
// 		x[2]=temp;
// 	}
// }

void sort_Y(int arrayToSort[],int arrayToSortSize)		//in case the blobs detected are not in the particular order
{														//will be used in the calculate_constants_perpend.cpp 
	for(int i=1; i<arrayToSortSize; ++i) {				//code*/
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
	// IplImage* frame;
	IplImage* img_hsv=cvCreateImage(cvSize(752, 480), 8, 3);				//Image in HSV color space
	IplImage* threshy = cvCreateImage(cvSize(752, 480), 8, 1);				//Threshed Image
	IplImage* labelImg=cvCreateImage(cvSize(752, 480),IPL_DEPTH_LABEL,1);	//Image Variable for blobs

	

	CvBlobs blobs;

	int xu, yu;				//coordinates of undistorted image
	int xd, yd;				//coordinates in distorted image

	ifstream filenames ("filenames_objdis.txt");
	ofstream constants ("acyut_constants_objdis.dat",ios::binary);

	int detected_blobs;

		while(1)
		{
			if(filenames.eof())
				break;

			char filename[20];		
			// getline(filenames,filename);
			filenames>>filename;
			char motor_loc[3];
			int j=0;

				for(int i=0;;++i)			//get motor location from filename
				{
					if(filename[i]=='.')
						break;
					motor_loc[i]=filename[i];
				}			

					string optimum_focal;
					string pix2cmyf;
					string angle_s;
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
					
					cvInRangeS(img_hsv, cvScalar(0, 120, 100), cvScalar(255, 255, 255), threshy);
					//Filtering the frame - subsampling??
					cvSmooth(threshy,threshy,CV_MEDIAN,7,7);

				int counter = 0;

				while(counter!=1)
				{
					//Finding the blobs
					unsigned int result=cvLabel(threshy,labelImg,blobs);
					//Filtering the blobs
					cvFilterByArea(blobs,100,10000);
					//Rendering the blobs
					cvRenderBlobs(labelImg,blobs,frame,frame);

					int i=0;
					for (CvBlobs::const_iterator it=blobs.begin(); it!=blobs.end(); ++it)
					{			
						xd = (it->second->maxx+it->second->minx)/2;
						yd = (it->second->maxy+it->second->miny)/2;
						xd = xd - 752/2;
						yd = -yd + 480/2;
						// cout<<"non-linear coords: xd="<<xd<<"     yd="<<yd<<endl;
						getLinearCoords(xd, yd, &xu, &yu);
						XU[i]=xu;
						YU[i]=yu;
						++i;
					}
					detected_blobs=i;

					if ( detected_blobs!=3 )		//if the number of blobs detected will be not equal to 3
						break;						//then the program will not consider to that instance

						int motor_pos=0;

						for(int k=0;k<3;++k)		//calculate motor_pos from motor_loc
						{
							int x;
							x=int(motor_loc[k]-'0');
							motor_pos=(motor_pos*10)+x;
						}

						motor=motor_pos;

					angle=((-512.0+motor_pos)/1024.0)*300.0;	//can depend on how you have attached the camera to the motor
																//basically required to calculate the angle at which IPM needs
					cout<<"\ndetected blobs :"<<detected_blobs;	//to be calculated
					cout<<"\nmotor_pos :"<<motor_pos;			
					cout<<"\n"<<"angle :"<<angle;
					

					for(float f=10.0;f<1500.0;f+=0.01)			//calculate constants and optimal focal length
					{											//it scans for f from 10 - 1500 to check where
						for (int j=0;j<detected_blobs;++j)		//the deviation in pixel difference is minimum
							getPt(XU[j],YU[j],f,j,angle);		//for that value pix2cmy and side view compensation
																//constant is evaluated (assuming that the first blob
						// sort_Y(Y,3);							//is at a distance of 40cm from the camera)
					
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

						pix2cmy=40.0/pixel_difference;			//calculated based on the assumption that individual blobs
																//are placed at a distance of 40 cm
						for(int h=0;h<detected_blobs;++h)
							getPt(XU[h],YU[h],optimum_f,h,angle);

						for(int l=0;l<3;++l)
							Y[l]=Y[l]*pix2cmy;

						
						cout<<"\nOptimum Focal Length :"<<optimum_f;
						cout<<"\nmin deviation :"<<min;
						cout<<"\npix2cmy :"<<pix2cmy;
						cout<<"\nDistance :";

						for(int l=0;l<3;++l)
							cout<<Y[l]<<"\t\t";

						cout<<"\nDifference :";
						
						for(int l=0;l<2;++l)
							cout<<Y[l+1]-Y[l]<<"\t\t";
						
						cout<<"\n\n\n";

						entry.motor_pos=motor;
						entry.angle=angle;
						entry.focal=optimum_f;
						entry.pix2cmy=pix2cmy;
						entry.s_view_compensation=(-(Y[2]-40.0));

			            if(prev<optimum_f)			//assumption that overall the net f values are increasing
			            {
			            	constants.write((char*)&entry,sizeof(entry));		//writing to file
							prev=optimum_f;
						}
		}

	constants.close();
	filenames.close();
	converttotext();	
	return 0;
}