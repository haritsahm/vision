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
#define WIDTHd 320
#define HEIGHTd 240
#define HEIGHTu 320
#define WIDTHu 240
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
}entry,lut[1000];

int XU[3];			//undistorted coordinates x - axis
int YU[3];			//undistorted coordinates y - axis

int X[3];			//perpend
int diff[2];		//pixel differences

int lut_no=0;

double ax = -7.1e-06;
void getLinearCoords(int xd, int yd, int* xu, int* yu)
{
	double r2 = xd*xd +yd*yd;	
	*xu = xd/(1+ax*r2);
	*yu = yd/(1+ax*r2);
}

void getPt(int u, int v, float f, int i,float x)
{
	float s=1.0;
	float theta= x/180.0*PI;

	float objdis=(v+(f/s)*tan(theta))/(1-((v*s)/f)*tan(theta));
	float perpend=u*((s/f)*objdis*sin(theta)+cos(theta));
	X[i]=int(perpend);
}

void sort_X(int arrayToSort[],int arrayToSortSize)
{
	for(int i=1; i<arrayToSortSize; ++i) {
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

void converttotext()
{
	ifstream constants("acyut_constants_perpend.dat",ios::binary);
	ofstream output("acyut_constants_perpend.txt");
	while(1)
	{
		if(constants.eof())
			break;

		constants.read((char*)&entry,sizeof(entry));
		
		output<<entry.motor_pos<<"\t\t\t"<<setprecision(5)<<entry.angle<<"\t\t\t"
				<<setprecision(5)<<entry.focal<<"\t\t\t"<<setprecision(5)<<entry.pix2cmx<<"\t\t\t"
				<<setprecision(5)<<entry.pix2cmy<<"\t\t\t"<<setprecision(5)<<entry.s_view_compensation<<"\n";
	}
	constants.close();
	output.close();
}

int search_lut(int motor)
{
	int pos;
	for(int i=0;i<lut_no;++i)
	{
		if(lut[i].motor_pos==motor)
		{
			pos=i;
			break;
		}
	}
	return pos;
}

int main()
{
	ifstream data;
	data.open("acyut_constants_objdis.dat",ios::binary);
	if(data.is_open())
		cout<<"\nFILE OPENED SUCCESSFULLY";
	else
		cout<<"\nUNABLE TO OPEN FILE";
	ofstream output;
	output.open("acyut_constants_perpend.dat",ios::binary);
	if(output.is_open())
		cout<<"\nACYUT_CONSTANTS_PERPEND.DAT OPENED SUCCESSFULLY";
	else
		cout<<"\nUNABLE TO OPEN ACYUT_CONSTANTS_PERPEND.DAT";
	ifstream filenames;
	filenames.open("filenames_perpend.txt");
	if(filenames.is_open())
		cout<<"\nFILENAMES_PERPEND.TXT OPENED SUCCESSFULLY";
	else
		cout<<"\nUNABLE TO OPEN FILENAMES_PERPEND.TXT";

	//Image Variables
	// IplImage* frame;
	IplImage* img_hsv=cvCreateImage(cvSize(320, 240), 8, 3);//Image in HSV color space
	IplImage* threshy = cvCreateImage(cvSize(320, 240), 8, 1);//Threshed Image
	IplImage* labelImg=cvCreateImage(cvSize(320, 240),IPL_DEPTH_LABEL,1);//Image Variable for blobs

	

	CvBlobs blobs;

	int xu, yu;		//coordinates of undistorted image
	int xd, yd;		//coordinates in distorted image

	while(1)
	{
		if(data.eof())
		{
			data.close();
			break;
		}

		data.read((char*)&lut[lut_no],sizeof(lut[lut_no]));
		++lut_no;
	}

	cout<<"\n"<<lut_no;

	for(int i=0;i<lut_no-1;++i)				//bubble sort according to parameter.motor
	{
		parameters temp;
		for(int j=0;j<lut_no-i-1;++j)
		{
			if(lut[j].motor_pos>lut[j+1].motor_pos)
			{
				temp=lut[j];
				lut[j]=lut[j+1];
				lut[j+1]=temp;
			}
		}
	}

	while(1)
		{
			if(filenames.eof())
				break;

			char filename[20];		
			// getline(filenames,filename,'\t');
			filenames>>filename;
			char motor_loc[3];
			// int j=0;

				for(int i=0;;++i)
				{
					if(filename[i]=='.')
						break;
					motor_loc[i]=filename[i];
				}

					IplImage* frame = cvLoadImage(filename);
					// cvAddS(frame, cvScalar(70,70,70), frame);
					cvCvtColor(frame,img_hsv,CV_BGR2HSV);
					//Thresholding the frame for yellow
					//cvInRangeS(img_hsv, cvScalar(20, 100, 20), cvScalar(30, 255, 255), threshy);
					cvInRangeS(img_hsv, cvScalar(0, 60, 100), cvScalar(255, 255, 255), threshy);
					//Filtering the frame - subsampling??
					cvSmooth(threshy,threshy,CV_MEDIAN,7,7);						
					
					//Finding the blobs
					unsigned int result=cvLabel(threshy,labelImg,blobs);
					//Filtering the blobs
					cvFilterByArea(blobs,100,10000);
					//Rendering the blobs
					cvRenderBlobs(labelImg,blobs,frame,frame);

					cvShowImage("frame",frame);
					cvWaitKey();

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
					int detected_blobs=i;
					cout<<"\n"<<detected_blobs;
					if ( detected_blobs!=3 )
						{
							break;
							cout<<"\n"<<detected_blobs;
							cout<<"\nbreaking";
						}

						int motor_pos=0;

						for(int k=0;k<3;++k)
						{
							int x;
							x=int(motor_loc[k]-'0');
							motor_pos=(motor_pos*10)+x;
						}

					float angle=((-512.0+motor_pos)/1024.0)*300.0;
					
					int pos= search_lut(motor_pos);
					entry=lut[pos];
					for(int i=0;i<3;++i)
						getPt(XU[i],YU[i],lut[pos].focal,i,angle);
					sort_X(X,3);

					// for(int i=0;i<3;++i)
					// 	cout<<"\n"<<"X[i] :"<<X[i];
					// cout<<"\n\n\n";

					for(int i=0;i<2;++i)
						diff[i]=X[i+1]-X[i];

					cout<<"\n"<<"diff[0] :"<<diff[0]<<"\t\t\t"<<"diff[1] :"<<diff[1]<<"\n\n\n";

					float pix2cmxf= (80.0/(diff[0]+diff[1]));
					lut[pos].pix2cmx= pix2cmxf;
					cout<<"\npix2cmx :"<<pix2cmxf;
					cout<<"\nWriting...";
					// if(diff[0]!=0)
						output.seekp(0,ios::end);
						output.write((char*)&lut[pos],sizeof(lut[pos]));
		}
	cout<<"\nConverting...";
	converttotext();
	return 0;
}