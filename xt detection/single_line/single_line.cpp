#include <opencv2/opencv.hpp>
#include <opencv/cv.h>
#include <cstdio>
#include <stdlib.h>
#include <cmath>
#include <string.h>
#include <vector>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <cassert>
#include <string.h>

using namespace cv;
using namespace std;

struct line_data
{
	double x1;
	double x2;
	double y1;
	double y2;
	double slope;
	double length;
};

#define tolerance (10)
#define bin_width (10)
#define number_of_bins (180/bin_width)
#define PI (3.14159265359)
#define rad2deg(a) (a*180.0/PI)
#define deg2rad(a) (a*PI/180.0)
// #define SHOW_ALL_LINES

std::vector <vector<line_data> > line_stack (number_of_bins);

double calc_linevector_length(line_data linevector)
{
	double length = sqrt((linevector.x2 - linevector.x1)*(linevector.x2 - linevector.x1) + (linevector.y2 - linevector.y1)*(linevector.y2 - linevector.y1));
	return length;
}

void classify_linevectors(line_data linevector)
{
	int bin1 = floor(linevector.slope)/bin_width;
	int bin2 = (floor(linevector.slope + tolerance))/bin_width;
	int bin3 = (floor(linevector.slope - tolerance))/bin_width;
	// printf("%d %d %d\n", bin1, bin2, bin3);
	line_stack[bin1].push_back(linevector);
	if(bin2 != bin1)
	{
		if(bin2 == 18)
			line_stack[0].push_back(linevector);
		else
			line_stack[bin2].push_back(linevector);
	}
	if(bin3 != bin1)
	{
		if(bin3 == -1)
			line_stack[number_of_bins - 1].push_back(linevector);
		else
			line_stack[bin3].push_back(linevector);
	}
}

int max_index_primary;
int max_index_secondary;
int max_count_primary = -1;
int max_count_secondary = -1;

void find_linevector_concentration()
{	
	max_count_primary = -1;
	max_count_secondary = -1;
	for (int i = 0; i < number_of_bins; ++i)
	{
		if(max_count_primary < (int)line_stack[i].size())
		{
			max_count_primary = line_stack[i].size();
			max_index_primary = i;
		}
	}
	int angle_shift = (int)number_of_bins/2;
	if((int)line_stack[(max_index_primary+angle_shift-1)%number_of_bins].size() > max_count_secondary)
	{
		max_count_secondary = line_stack[(max_index_primary+angle_shift-1)%number_of_bins].size();
		max_index_secondary = (max_index_primary+angle_shift-1)%number_of_bins;
	}
	if((int)line_stack[(max_index_primary+angle_shift)%number_of_bins].size() > max_count_secondary)
	{
		max_count_secondary = line_stack[(max_index_primary+angle_shift)%number_of_bins].size();
		max_index_secondary = (max_index_primary+angle_shift)%number_of_bins;
	}
	if((int)line_stack[(max_index_primary+angle_shift+1)%number_of_bins].size() > max_count_secondary)
	{
		max_count_secondary = line_stack[(max_index_primary+angle_shift+1)%number_of_bins].size();
		max_index_secondary = (max_index_primary+angle_shift+1)%number_of_bins;
	}
	// printf("index_primary: %d count_primary: %d index_secondary: %d count_secondary: %d\n", max_index_primary, max_count_primary, max_index_secondary, max_count_secondary);
}

double distance_point2point(int x1, int y1, int x2, int y2)
{
	double distance = sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
	return distance;
}

double distance_point2linevector(int x, int y, line_data linevector)
{
	double a = deg2rad(linevector.slope);
	double b = -1;
	double c = linevector.y1 - deg2rad(linevector.slope)*linevector.x1;
	double distance = fabs(a*x + b*y + c)/sqrt(a*a + b*b);
	return distance;
}

double linevector_distance(line_data linevector1, line_data linevector2)
{
	double distance = -1;
	distance += distance_point2linevector(linevector1.x1, linevector1.y1, linevector2);
	distance += distance_point2linevector(linevector1.x2, linevector1.y2, linevector2);
	distance += distance_point2linevector(linevector2.x1, linevector2.y1, linevector1);
	distance += distance_point2linevector(linevector2.x2, linevector2.y2, linevector1);
	return distance/4.0;
}

double linevector_displacement(line_data linevector1, line_data linevector2)
{
	double displacement;
	displacement = min(distance_point2point(linevector1.x1,linevector1.y1,linevector2.x1,linevector2.y1),displacement);
	displacement = min(distance_point2point(linevector1.x1,linevector1.y1,linevector2.x2,linevector2.y2),displacement);
	displacement = min(distance_point2point(linevector1.x2,linevector1.y2,linevector2.x1,linevector2.y1),displacement);
	displacement = min(distance_point2point(linevector1.x2,linevector1.y2,linevector2.x2,linevector2.y2),displacement);
	return displacement;
}

void merge_linevectors(int bin)
{
	
}

void showImage()
{
	IplImage* display = cvCreateImage(cvSize(423,423),8,3);
	for (int i = 0; i < max_count_primary; ++i)
	{
		cvLine(display, cvPoint(line_stack[max_index_primary][i].x1,line_stack[max_index_primary][i].y1)
				, cvPoint(line_stack[max_index_primary][i].x2,line_stack[max_index_primary][i].y2),CV_RGB(0,255,0));
	}
	for (int i = 0; i < max_count_secondary; ++i)
	{
		cvLine(display, cvPoint(line_stack[max_index_secondary][i].x1,line_stack[max_index_secondary][i].y1)
				, cvPoint(line_stack[max_index_secondary][i].x2,line_stack[max_index_secondary][i].y2),CV_RGB(0,0,255));
	}
	cvShowImage("output",display);
	int c = cvWaitKey();
	// assert(c != 27);
	cvReleaseImage(&display);
}
#ifdef SHOW_ALL_LINES
void showAllLines(char input_file[])
{
	fstream file;
	file.open(input_file);
	IplImage* display = cvCreateImage(cvSize(423,423),8,1);
	double x1,x2,y1,y2,temp;
	while(1)
	{
		if(file.eof())
		{
			file.close();
			break;
		}
		file>>x1;
		file>>y1;
		file>>x2;
		file>>y2;
		file>>temp;
		file>>temp;
		file>>temp;
		cvLine(display,cvPoint(x1,y1),cvPoint(x2,y2),cvScalar(255));
	}
	cvShowImage("display",display);
	cvWaitKey();
	cvReleaseImage(&display);
}
#endif

int main(int argc, char const *argv[])
{
	if(argc < 2)
	{
		printf("USAGE:\t%s <FILENAME>\n", argv[0]);
		return 0;
	}
	#ifdef SHOW_ALL_LINES
	if(argc > 2 && (strcmp(argv[2],"y") || strcmp(argv[2],"Y")))
	{
		showAllLines(argv[1]);
		return 0;
	}
	#endif

	fstream input_file;
	line_data reader;
	double temp;
	input_file.open(argv[1]);

	while(1)
	{
		if(input_file.eof())
		{
			input_file.close();
			break;
		}
		// printf("1\n");
		input_file>>reader.x1;
		input_file>>reader.y1;
		input_file>>reader.x2;
		input_file>>reader.y2;
		input_file>>temp;
		input_file>>temp;
		input_file>>temp;

		reader.slope = rad2deg(atan2((reader.y2 - reader.y1),(reader.x2 - reader.x1)));
		if(reader.slope < 0)
			reader.slope += 180.0;
		reader.length = calc_linevector_length(reader);
		// printf("slope: %lf\n", reader.slope);
		classify_linevectors(reader);
	}
	find_linevector_concentration();
	showImage();
	return 0;
}