#include "localize.h"
//
using namespace LOCALIZE_INTERNALS;

//Feature detection functions are in featuredetection.h


Localize::Localize()
{
	//Initializing. Choose particles randomly, each particle gets same probability
	int i;
	srand(time(NULL));
	for(i=0; i<NO_OF_PARTICLES; i++)
	{
		p[i].x = rand()%(MAX_X);	//Assigns a random number between 0 and MAX_X INCLUSIVE
		p[i].y = rand()%(MAX_Y);	//Assigns a random number between 0 and MAX_Y INCLUSIVE
		p[i].angle = ((double)((rand()%(360))))*PI/180.0;	//Assigns a random angle with given resolution
	}
	nFrames = 0;
#ifndef SHOW_FANCY_LOCALIZATION
	dispImage = cvCreateImage(cvSize(MAX_X, MAX_Y), 8, 1);
#endif
#ifdef SHOW_FANCY_LOCALIZATION
	loadedImage = cvLoadImage("field.jpg");
	dispImage = cvCreateImage(cvSize(loadedImage->width, loadedImage->height), loadedImage->depth, loadedImage->nChannels);
#endif
	createLandmarkList();
}



//Origin is corner side of yellow goal along BYB line
//NOTE: Coordinates are negative and beyond range also! (because of BYB and YBY)
void Localize::createLandmarkList()
{

	landT[0].x=MAX_X/2;
	landT[0].y = MAX_Y;
	landT[1].x=MAX_X/2;
	landT[1].y = 0;
	
	landX[0].x = MAX_X/2;
	landX[0].y = MAX_Y/2 - MAX_Y/8;
	landX[1].x = MAX_X/2;
	landX[1].y = MAX_Y/2 + MAX_Y/8;
	
	landGPY[1].x=0;
	landGPY[1].y=MAX_Y/4;
	landGPY[0].x=0;
	landGPY[0].y=MAX_Y*3/4;

	landGPB[0].x=MAX_X;
	landGPB[0].y=MAX_Y/4;
	landGPB[1].x=MAX_X;
	landGPB[1].y=MAX_Y*3/4;

	landYBY.x=MAX_X/2;
	landYBY.y=MAX_Y + MAX_Y/15;

	landBYB.x=MAX_X/2;
	landBYB.y=-MAX_Y/15;
	
}



void Localize::cvQuiver(IplImage* Image,int x,int y,int u,int v,CvScalar Color,int Size,int Thickness){
cv::Point pt1,pt2;
double Theta;


if(u==0)
    Theta=PI/2;
else
    Theta=atan2(double(v),(double)(u));


pt1.x=x;
pt1.y=y;

pt2.x=x+u;
pt2.y=y+v;

cvLine(Image,pt1,pt2,Color,Thickness,8);  //Draw Line


Size=(int)(Size*0.707);



if(Theta==PI/2 && pt1.y > pt2.y){
    pt1.x=(int)(Size*cos(Theta)-Size*sin(Theta)+pt2.x);
    pt1.y=(int)(Size*sin(Theta)+Size*cos(Theta)+pt2.y);
    cvLine(Image,pt1,pt2,Color,Thickness,8);  //Draw Line

    pt1.x=(int)(Size*cos(Theta)+Size*sin(Theta)+pt2.x);
    pt1.y=(int)(Size*sin(Theta)-Size*cos(Theta)+pt2.y);
    cvLine(Image,pt1,pt2,Color,Thickness,8);  //Draw Line
}
else{
    pt1.x=(int)(-Size*cos(Theta)-Size*sin(Theta)+pt2.x);
    pt1.y=(int)(-Size*sin(Theta)+Size*cos(Theta)+pt2.y);
    cvLine(Image,pt1,pt2,Color,Thickness,8);  //Draw Line

    pt1.x=(int)(-Size*cos(Theta)+Size*sin(Theta)+pt2.x);
    pt1.y=(int)(-Size*sin(Theta)-Size*cos(Theta)+pt2.y);
    cvLine(Image,pt1,pt2,Color,Thickness,8);  //Draw Line
}
}



void Localize::printPosition()
{

#ifdef SHOW_FANCY_LOCALIZATION
	int tempx, tempy;
	cvCopy(loadedImage, dispImage);

	// int line_thickness = 2;
	
	 CvScalar particle_color= {0,0,255};
	 CvScalar line_color= {0,255,255};
	 CvScalar circle_color= {50,50,50};
	
	for (int i = 0; i < NO_OF_PARTICLES; ++i)
	{
		tempx = p[i].x;
		tempy = p[i].y;
		if(tempx <0)
			tempx = 0;
		if(tempy<0)
			tempy = 0;
		if(tempx >= MAX_X)
			tempx = MAX_X - 1;
		if(tempy >= MAX_Y)
			tempy = MAX_Y - 1;
		cvCircle(dispImage, cvPoint(98+tempx*2.5,71+ (MAX_Y - tempy)*2.5), 1, particle_color, 2);
		//returnPixel1C(dispImage, tempx, MAX_Y - tempy) = 255;
	}
	cvQuiver(dispImage, 98+selfX*2.5, 71+ (MAX_Y - selfY)*2.5, 100*cos(deg2rad(selfAngle)), -100*sin(deg2rad(selfAngle)),line_color, 20,2);
	cvCircle(dispImage, cvPoint(98+selfX*2.5,71+ (MAX_Y - selfY)*2.5), 5, circle_color, 2);
	#ifndef ALL_PRINTING_OFF
		printf("X: %lf Y:%lf Angle:%lf\n", selfX, selfY, selfAngle);
	#endif

	return;
#endif

#ifndef SHOW_FANCY_LOCALIZATION

	int tempx, tempy;
	cvZero(dispImage);
	int count0 = 0;
	int count180 = 0;
	for (int i = 0; i < NO_OF_PARTICLES; ++i)
	{
		tempx = p[i].x;
		tempy = p[i].y;
		if(tempx <0)
			tempx = 0;
		if(tempy<0)
			tempy = 0;
		if(tempx >= MAX_X)
			tempx = MAX_X - 1;
		if(tempy >= MAX_Y)
			tempy = MAX_Y - 1;
		if(fabs(CV_PI - p[i].angle) < 20.0*CV_PI/180.0)
			count180++;
		if(fabs(p[i].angle) <  20.0*CV_PI/180.0 || fabs(p[i].angle - 2*CV_PI) < 20.0*CV_PI/180.0)
			count0++;
		returnPixel1C(dispImage, tempx, MAX_Y - tempy) = 255;
	}
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5);
	cvPutText(dispImage, "Y", cvPoint(2, dispImage->height/2), &font, cvScalar(255));
	cvPutText(dispImage, "B", cvPoint(dispImage->width - 13, dispImage->height/2), &font, cvScalar(255));
	cvPutText(dispImage, "BYB", cvPoint(dispImage->width/2 - 12, dispImage->height - 8), &font, cvScalar(255));
	#ifndef ALL_PRINTING_OFF
		printf("X: %lf Y:%lf Angle:%lf\n", selfX, selfY, selfAngle);
		printf("Count0: %d Count180: %d\n", count0, count180);
	#endif
	return;
#endif
}



void Localize::walkersalias(
int n,   // number of classes
double *p, // relative weights of each class
int nans,  // sample size to return
int *ans  // sample as an array of class indices
){
    int *a = (int*)calloc(n,sizeof(int));
	double *q, rU;
	int i,j,k;
	int *HL,*H,*L;
	HL = (int *)calloc(n,sizeof(int));
	q = (double *)calloc(n,sizeof(double));
	H = HL - 1; L = HL + n;
	double sum = 0;
	for(i = 0; i < n; i++)
	{
		sum += p[i];
	}
//	printf("Sum=%lf\n", sum);
	for(i = 0; i < n; i++)
	{
		p[i] /= sum;
	}
	for(i = 0; i < n; i++)
	{
		q[i] = p[i] * n;
		if(q[i] < 1.) *++H = i; else *--L = i;
	}
	if(H >= HL && L < HL +n)
	{
		for(k = 0; k < n-1; k++)
		{
			i = HL[k];
			j = *L;
			a[i] = j;
			q[j] += q[i] - 1;
			if(q[j] < 1.) L++;
			if(L >= HL + n) break;
		}
	}
	for(i = 0; i < n; i++) q[i] += i;
	for(i = 0; i < nans; i++)
	{
		rU = (double) rand() / RAND_MAX * n;
		k = (int) rU;
		ans[i] = (rU < q[k]) ? k : a[k];
	}
	free(HL);
	free(q);
        free(a);
}



//TODO: when value of particle goes beyond min max limits due to randomness, assign that particle
//a random location (currently something weird is happening)

void Localize::randomize()
{
	for(int i=0; i<NO_OF_PARTICLES; i++)
	{
		p[i].x = rand()%(MAX_X);	//Assigns a random number between 0 and MAX_X INCLUSIVE
		p[i].y = rand()%(MAX_Y);	//Assigns a random number between 0 and MAX_Y INCLUSIVE
		p[i].angle = ((double)((rand()%(360))))*PI/180.0;	//Assigns a random angle with given resolution
	}
}


void Localize::doLocalize(FeatureDetection &fd, MotionModel &mm, CamCapture &cam, int imuangle)
{
	// if(mm.updated==1)
	// {	
	// 	#ifndef ALL_PRINTING_OFF
	// 		printf("Confidence before %lf\n", confidence());
	// 		printf("[localize] Angle changed %f \n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n", mm.theta);
	// 		printf("[localize] Moved ahead %f cm\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n", mm.r/10.0);
	// 		printf("[localize] Angle2 changed %f \n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n", mm.theta2);
	// 	#endif

	// 	for(int k=0; k<NO_OF_PARTICLES; k++)
	// 	{
	// 		if((rand()%(200)==0))
	// 		{
	// 			p[k].x = rand()%(MAX_X);	//Assigns a random number between 0 and MAX_X INCLUSIVE
	// 			p[k].y = rand()%(MAX_Y);	//Assigns a random number between 0 and MAX_Y INCLUSIVE
	// 			//p[k].angle = ((double)((rand()%(360))))*PI/180.0;	//Assigns a random angle with given resolution
	// 			p[k].angle = (double)imuangle + (rand()%(10) - 5);
	// 			continue;
	// 		}
	// 		p[k].angle = p[k].angle + ((float)(rand()%4 - 2 - mm.theta))*PI/180.0;
	// 		while(p[k].angle < 0)
	// 			p[k].angle += 2*PI;

	// 		while(p[k].angle > 2*PI)
	// 			p[k].angle -= 2*PI;

	// 		if(mm.r > 0.0)
	// 		{
	// 			double addx = (mm.r*cos(p[k].angle))/10.0;
	// 			double addy = (mm.r*sin(p[k].angle))/10.0;
	// 			p[k].x = p[k].x + addx + rand()%4 - 2;
	// 			//following code for taking care of particles outside the field is probably wrong!
	// 			// if(p[k].x >= MAX_X)
	// 			// 	p[k].x = p[k].x - MAX_X - 1;
	// 			// if(p[k].x < 0)
	// 			// 	p[k].x = MAX_X + p[k].x;

	// 			p[k].y = p[k].y + addy + rand()%4 - 2;
	// 			// following code for taking care of particles outside the field is probably wrong!
	// 			// if(p[k].y >= MAX_Y)
	// 			// 	p[k].y = p[k].y - MAX_Y - 1;
	// 			// if(p[k].y < 0)
	// 			// 	p[k].y = MAX_Y + p[k].y;
	// 		}
	// 		p[k].angle = p[k].angle + ((float)(rand()%4 - 2 - mm.theta2))*PI/180.0;
	// 		while(p[k].angle < 0)
	// 			p[k].angle += 2*PI;

	// 		while(p[k].angle > 2*PI)
	// 			p[k].angle -= 2*PI;
	// 	}
	// 	updatePosition();
	// 	#ifndef ALL_PRINTING_OFF
	// 		printf("Confidence after %lf\n", confidence());
	// 	#endif
	// }
	// else
	{
		for(int k=0; k<NO_OF_PARTICLES; k++)
		{
			// Adding a bit of randomness, plus replacing 2% of total points by completely random points
			if((rand()%(2000)==0))
			{
				p[k].x = rand()%(MAX_X);	//Assigns a random number between 0 and MAX_X INCLUSIVE
				p[k].y = rand()%(MAX_Y);	//Assigns a random number between 0 and MAX_Y INCLUSIVE
				p[k].angle = ((double)((rand()%(360))))*PI/180.0;	//Assigns a random angle with given resolution
				continue;
			}
		}
	}

	if(imuangle != -9999 && fd.l.size()>1)
	{
		double angle = imuangle ;
		angle = deg2rad(angle);
		while(angle >= 2*PI)
			angle -= 2*PI;
		while(angle < 0)
			angle += 2*PI;
		for (int i = 0; i < NO_OF_PARTICLES; ++i)
		{
			p[i].angle =  angle + ((float)(rand()%50 - 25))*PI/180.0;
			while(p[i].angle >= 2*PI)
				p[i].angle -= 2*PI;
			while(p[i].angle < 0)
				p[i].angle += 2*PI;
		}
	}

	printPosition();
	if(fd.l.size()<=1)
		{
			confidence=0.0;
		}

	else
	{
	#ifndef ALL_PRINTING_OFF
	printf("NO OF LANDMARKS = %d !!!!!!!!!!!!!!!!!", fd.l.size());
	#endif
	nFrames++;
	//If no motion model but at least 2 landmark spotted, randomize a bit
	// if(mm.updated==0)
	if(1)//---^
	{
		for(int k = 0; k < NO_OF_PARTICLES; k++)
		{
			p[k].angle = p[k].angle + ((float)(rand()%10 - 5))*PI/180.0;
			while(p[k].angle < 0)
				p[k].angle += 2*PI;
			p[k].x = p[k].x + rand()%10 - 5;
			// if(p[k].x >= MAX_X)
			// 	p[k].x = p[k].x - MAX_X - 1;
			// if(p[k].x < 0)
			// 	p[k].x = MAX_X + p[k].x;
			p[k].y = p[k].y + rand()%10 - 5;
			// if(p[k].y >= MAX_Y)
			// 	p[k].y = p[k].y - MAX_Y - 1;
			// if(p[k].y < 0)
			// 	p[k].y = MAX_Y + p[k].y;
		}
	}

	for(int i = 0; i < NO_OF_PARTICLES; i++)
		w[i] = 1.0;

	//For each landmark
	for(int i=0; i<fd.l.size(); i++)
	{
		int temp_x1, temp_y1;
		int temp_x2, temp_y2;
		int temp_x3, temp_y3;
		int temp_x4, temp_y4;
		int no_of_instance;
		double prob1, prob2, prob3, prob4;
		//For each particle, multiply weight according to prob of landmark
		switch(fd.l[i].type)
		{
			case LAND_GPY:
			#ifdef SYMMETRIC_LANDMARKS
			no_of_instance=4;
			temp_x1=landGPY[0].x;
			temp_y1=landGPY[0].y;
			temp_x2=landGPY[1].x;
			temp_y2=landGPY[1].y;	
			temp_x3=landGPB[0].x;
			temp_y3=landGPB[0].y;
			temp_x4=landGPB[1].x;
			temp_y4=landGPB[1].y;
			break;
			#endif
			#ifndef SYMMETRIC_LANDMARKS
			no_of_instance=2;
			temp_x1=landGPY[0].x;
			temp_y1=landGPY[0].y;
			temp_x2=landGPY[1].x;
			temp_y2=landGPY[1].y;
			break;
			#endif
			

			case LAND_GPB:
			#ifdef SYMMETRIC_LANDMARKS
			no_of_instance=4;
			temp_x1=landGPY[0].x;
			temp_y1=landGPY[0].y;
			temp_x2=landGPY[1].x;
			temp_y2=landGPY[1].y;	
			temp_x3=landGPB[0].x;
			temp_y3=landGPB[0].y;
			temp_x4=landGPB[1].x;
			temp_y4=landGPB[1].y;
			break;
			#endif
			#ifndef SYMMETRIC_LANDMARKS
			no_of_instance=2;
			temp_x1=landGPB[0].x;
			temp_y1=landGPB[0].y;
			temp_x2=landGPB[1].x;
			temp_y2=landGPB[1].y;
			break;
			#endif


			case LAND_YBY:
			no_of_instance=1;
			temp_x1=landYBY.x;
			temp_y1=landYBY.y;
			break;
			case LAND_BYB:
			no_of_instance=1;
			temp_x1=landBYB.x;
			temp_y1=landBYB.y;
			break;
			case LAND_X:
			no_of_instance = 2;
			temp_x1=landX[0].x;
			temp_y1=landX[0].y;
			temp_x2=landX[1].x;
			temp_y2=landX[1].y;
			break;
			case LAND_T:
			no_of_instance = 2;
			temp_x1=landT[0].x;
			temp_y1=landT[0].y;
			temp_x2=landT[1].x;
			temp_y2=landT[1].y;
			break;
		}

		for(int j=0; j<NO_OF_PARTICLES; j++)
		{
			if(no_of_instance==1)
			{
				double temp_angle = rad2deg(atan2(temp_y1-p[j].y,temp_x1-p[j].x)-p[j].angle);
				temp_angle += fd.l[i].angle;	//+ because landmark angle conventions are bad
				while(temp_angle < 0)
					temp_angle += 360.0;
				if(temp_angle > 180.0)
					temp_angle = 360.0 - temp_angle;
				w[j]*=exp(-pow((sqrt(pow((temp_x1-p[j].x),2)+pow((temp_y1-p[j].y),2))-fd.l[i].distance),2)/((U_DIST + fd.l[i].distance*U_DIST_P)*(U_DIST + fd.l[i].distance*U_DIST_P))-pow((temp_angle),2)/(U_ANGLE*U_ANGLE));
			//	printf("%lf\n", pow((sqrt(pow((temp_x1-p[j].x),2)+pow((temp_y1-p[j].y),2))-fd.l[i].distance),2)-pow((rad2deg(atan2(temp_y1-p[j].y,temp_x1-p[j].x)-p[j].angle)- fd.l[i].angle),2));
			}
			else if(no_of_instance==2)
			{
				double temp_angle1 = rad2deg(atan2(temp_y1-p[j].y,temp_x1-p[j].x)-p[j].angle);
				temp_angle1 += fd.l[i].angle;
				while(temp_angle1 < 0)
					temp_angle1 += 360.0;
				if(temp_angle1 > 180.0)
					temp_angle1 = 360.0 - temp_angle1;
				double temp_angle2 = rad2deg(atan2(temp_y2-p[j].y,temp_x2-p[j].x)-p[j].angle);
				temp_angle2 += fd.l[i].angle;
				while(temp_angle2 < 0)
					temp_angle2 += 360.0;
				if(temp_angle2 > 180.0)
					temp_angle2 = 360.0 - temp_angle2;
				prob1=exp(-pow((sqrt(pow((temp_x1-p[j].x),2)+pow((temp_y1-p[j].y),2))-fd.l[i].distance),2)/((U_DIST + fd.l[i].distance*U_DIST_P)*(U_DIST + fd.l[i].distance*U_DIST_P))-pow((temp_angle1),2)/(U_ANGLE*U_ANGLE));
				prob2=exp(-pow((sqrt(pow((temp_x2-p[j].x),2)+pow((temp_y2-p[j].y),2))-fd.l[i].distance),2)/((U_DIST + fd.l[i].distance*U_DIST_P)*(U_DIST + fd.l[i].distance*U_DIST_P))-pow((temp_angle2),2)/(U_ANGLE*U_ANGLE));

				if(prob1>prob2)
					w[j]*=prob1;
				else
					w[j]*=prob2;
			}
			else if(no_of_instance==4)
			{
				double temp_angle1 = rad2deg(atan2(temp_y1-p[j].y,temp_x1-p[j].x)-p[j].angle);
				temp_angle1 += fd.l[i].angle;
				while(temp_angle1 < 0)
					temp_angle1 += 360.0;
				if(temp_angle1 > 180.0)
					temp_angle1 = 360.0 - temp_angle1;
				double temp_angle2 = rad2deg(atan2(temp_y2-p[j].y,temp_x2-p[j].x)-p[j].angle);
				temp_angle2 += fd.l[i].angle;
				while(temp_angle2 < 0)
					temp_angle2 += 360.0;
				if(temp_angle2 > 180.0)
					temp_angle2 = 360.0 - temp_angle2;

				double temp_angle3 = rad2deg(atan2(temp_y3-p[j].y,temp_x3-p[j].x)-p[j].angle);
				temp_angle3 += fd.l[i].angle;
				while(temp_angle3 < 0)
					temp_angle3 += 360.0;
				if(temp_angle3 > 180.0)
					temp_angle3 = 360.0 - temp_angle3;
				double temp_angle4 = rad2deg(atan2(temp_y4-p[j].y,temp_x4-p[j].x)-p[j].angle);
				temp_angle4 += fd.l[i].angle;
				while(temp_angle4 < 0)
					temp_angle4 += 360.0;
				if(temp_angle4 > 180.0)
					temp_angle4 = 360.0 - temp_angle4;
				prob1=exp(-pow((sqrt(pow((temp_x1-p[j].x),2)+pow((temp_y1-p[j].y),2))-fd.l[i].distance),2)/((U_DIST + fd.l[i].distance*U_DIST_P)*(U_DIST + fd.l[i].distance*U_DIST_P))-pow((temp_angle1),2)/(U_ANGLE*U_ANGLE));
				prob2=exp(-pow((sqrt(pow((temp_x2-p[j].x),2)+pow((temp_y2-p[j].y),2))-fd.l[i].distance),2)/((U_DIST + fd.l[i].distance*U_DIST_P)*(U_DIST + fd.l[i].distance*U_DIST_P))-pow((temp_angle2),2)/(U_ANGLE*U_ANGLE));
				prob3=exp(-pow((sqrt(pow((temp_x3-p[j].x),2)+pow((temp_y3-p[j].y),2))-fd.l[i].distance),2)/((U_DIST + fd.l[i].distance*U_DIST_P)*(U_DIST + fd.l[i].distance*U_DIST_P))-pow((temp_angle3),2)/(U_ANGLE*U_ANGLE));
				prob4=exp(-pow((sqrt(pow((temp_x4-p[j].x),2)+pow((temp_y4-p[j].y),2))-fd.l[i].distance),2)/((U_DIST + fd.l[i].distance*U_DIST_P)*(U_DIST + fd.l[i].distance*U_DIST_P))-pow((temp_angle4),2)/(U_ANGLE*U_ANGLE));
				double probmax = prob1;
				if(prob2 > probmax)
					probmax = prob2;
				if(prob3 > probmax)
					probmax = prob3;
				if(prob4 > probmax)
					probmax = prob4;
				w[j]*=probmax;
			}

		}
	}
	//Weights assigned. Resampling begins
	int index_list[NO_OF_PARTICLES];
	for(int k=0; k<NO_OF_PARTICLES; k++)
		index_list[k] = 0;
	walkersalias(NO_OF_PARTICLES, w, NO_OF_PARTICLES, index_list);
	for(int k=0; k<NO_OF_PARTICLES; k++)
	{
		p_temp[k] = p[index_list[k]];
	}
	for(int k = 0; k < NO_OF_PARTICLES; k++)
		p[k] = p_temp[k];
	updatePosition();
	calcConfidence();

	}
	CvFont font;
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.3, 0.3, 0, 1, 8);
    char A[100] = "POSITION : ";
    char B[100] = "ANGLE : ";
    char C[100];
    char D[100];
    sprintf(D,"CONFIDENCE : %lf",confidence);
    sprintf(C, "       Y: %lf", selfY);
    ostringstream s1;
    s1<< "X: " << selfX;
    const std::string tmp1 = s1.str();
    const char* cs1 = tmp1.c_str();
    strcat(A,cs1);
    ostringstream s2;
    s2<< selfAngle;
    const std::string tmp2 = s2.str();
    const char* cs2 = tmp2.c_str();
    strcat(B,cs2);

    cvPutText(cam.rgbimg,A,cvPoint(10,10),&font,cvScalar(255,255,255));
    cvPutText(cam.rgbimg,B,cvPoint(10,30),&font,cvScalar(255,255,255));
    cvPutText(cam.rgbimg,C,cvPoint(30,20),&font,cvScalar(255,255,255));
    cvPutText(cam.rgbimg,D,cvPoint(10,40),&font,cvScalar(255,255,255));
}



void Localize::updatePosition()
{
	//Need to use some other mode of calculating current position, such as mode or something
	selfX = 0;
	selfY = 0;
	selfAngle = 0;
	double cosAngleSum = 0;
	double sinAngleSum = 0;
	for (int i = 0; i < NO_OF_PARTICLES; ++i)
	{
		selfX += p[i].x;
		selfY += p[i].y;
		cosAngleSum += cos(p[i].angle);
		sinAngleSum += sin(p[i].angle);
	}
	selfX = selfX/NO_OF_PARTICLES;
	selfY = selfY/NO_OF_PARTICLES;
	selfAngle = atan2(sinAngleSum/NO_OF_PARTICLES, cosAngleSum/NO_OF_PARTICLES);
	if(selfAngle < 0)
		selfAngle += 2*PI;
	selfAngle = rad2deg(selfAngle);
}



void Localize::calcConfidence()
{
	
	

	meanX = 0;
	meanY = 0;
	meanAngle = 0;
	double cosAngleSum = 0;
	double sinAngleSum = 0;
	double varX = 0, varY = 0, varAngle = 0;
	for(int i = 0; i < NO_OF_PARTICLES; i++)
	{
		meanX+= p[i].x;
		meanY+= p[i].y;
		cosAngleSum += cos(p[i].angle);
		sinAngleSum += sin(p[i].angle);
	}
	meanX /= NO_OF_PARTICLES;
	meanY /= NO_OF_PARTICLES;
	meanAngle = atan2(sinAngleSum/NO_OF_PARTICLES, cosAngleSum/NO_OF_PARTICLES);

	if(meanAngle < 0)
		meanAngle += 2*PI;
	for(int i = 0; i < NO_OF_PARTICLES; i++)
	{
		varX += (p[i].x - meanX)*(p[i].x - meanX);
		varY += (p[i].y - meanY)*(p[i].y - meanY);
		if((p[i].angle - meanAngle > PI)||(p[i].angle - meanAngle < -PI))
		{
			if((p[i].angle - meanAngle) > PI)
				varAngle += (p[i].angle - meanAngle - 2*PI)*(p[i].angle - meanAngle - 2*PI);
			else
				varAngle += (p[i].angle - meanAngle + 2*PI)*(p[i].angle - meanAngle + 2*PI);
		}
		else
			varAngle += (p[i].angle - meanAngle)*(p[i].angle - meanAngle);
	}
	varX /= NO_OF_PARTICLES;
	varY /= NO_OF_PARTICLES;
	varAngle /= NO_OF_PARTICLES;

	varAngle *= (180.0*180.0)/(PI*PI);
	confidence=125.0/(varX + varY + varAngle);

	if(confidence > 4.0)
		confidence=0.0;
	// printf("Confidence: X: %lf Y: %lf Angle: %lf\n", varX, varY, varAngle);
	#ifndef ALL_PRINTING_OFF
		// printf("Confidence: %lf\n", 125.0/(varX + varY + varAngle));
	#endif
	if(nFrames < 5)
		confidence=0.0;
	
}



AbsCoords Localize::getGoalCoords(int g)
{
	AbsCoords result;
	if(g==0)	//Blue
	{
		result.x = landGPB[0].x;
		result.y = (landGPB[0].y + landGPB[1].y)/2.0;
	}
	else
	{
		result.x = landGPY[0].x;
		result.y = (landGPY[0].y + landGPY[1].y)/2.0;	
	}
	return result;
}
