#include <opencv2/opencv.hpp>
#include <opencv/cv.h>
#include <cstdio>
#include <string.h>
#include <vector>
#include <algorithm>
#include <fstream>
#include <cmath>
using namespace cv;
using namespace std;

struct line
{
	double x1;
	double x2;
	double y1;
	double y2;
	double slope;
	double length;
};

#define tolerance 10
#define bin_width 10
#define number_of_bins 180/bin_width
#define PI 3.1415
#define rad2deg(a) (a*PI/180.0)
std::vector <vector<line> > line_stack (number_of_bins);

double mod(double x)
{
	double temp = x - floor(x);
	return floor(x)%bin_width + temp;
}

line calc_linevector_length(line linevector)
{
	linevector.length = sqrt((linevector.x2 - linevector.x1)^2 + (linevector.y2 - linevector.y1)^2);
}

void classify_linevectors(line linevector)
{
	int bin1 = floor(linevector.slope)/bin_width;
	int bin2 = (floor(linevector.slope + tolerance))/bin_width;
	int bin3 = (floor(linevector.slope - tolerance))/bin_width;
	line_stack[bin1].push_back(linevector);
	if(bin2 != bin1)
		line_stack[bin2].push_back(linevector);   
	if(bin3 != bin1)
		line_stack[bin3].push_back(linevector);
}

int max_index_primary;
int max_index_secondary;

void find_linevector_concentration()
{
	int max_count_primary = -1
	for (int i = 0; i < number_of_bins; ++i)
	{
		if(max_count_primary < line_stack[i].length())
		{
			max_count_primary = line_stack[i].length());
			max_index_primary = i;
		}
	}
	int max_count_secondary = -1;
	int angle_shift = (int)number_of_bins/2;
	if(line_stack[(i+angle_shift-1)%number_of_bins].length() > max_count_secondary)
	{
		max_count_secondary = line_stack[(i+angle_shift-1)%number_of_bins].length();
		max_index_secondary = (i+angle_shift-1)%number_of_bins;
	}
	if(line_stack[(i+angle_shift)%number_of_bins].length() > max_count_secondary)
	{
		max_count_secondary = line_stack[(i+angle_shift)%number_of_bins].length();
		max_index_secondary = (i+angle_shift)%number_of_bins;
	}
	if(line_stack[(i+angle_shift+1)%number_of_bins].length() > max_count_secondary)
	{
		max_count_secondary = line_stack[(i+angle_shift+1)%number_of_bins].length();
		max_index_secondary = (i+angle_shift+1)%number_of_bins;
	}
}

double distance_point2linevector(int x, int y, line linevector)
{
	double a = slope;
	double b = -1;
	double c = linevector.y - slope*linevector.x;
	double distance = fabs(a*x + b*y + c)/sqrt(a^2 + b^2);
	return distance;
}

double linevector_distance(line linevector1, line linevector2)
{
	double distance = 0;
	distance += distance_point2linevector(linevector1.x1, linevector1.y1, linevector2);
	distance += distance_point2linevector(linevector1.x2, linevector1.y2, linevector2);
	distance += distance_point2linevector(linevector2.x1, linevector2.y1, linevector1);
	distance += distance_point2linevector(linevector2.x2, linevector2.y2, linevector1);
	return distance/4.0;
}

int main(int argc, char const *argv[])
{
	if(argc < 2)
	{
		printf("USAGE:\t%s <FILENAME>\n", argv[0]);
		return 0;
	}

	fstream input_file;
	line reader;
	double temp;
	input_file.open(argv[1]);

	while(1)
	{
		if(!input_file.eof())
		{
			input_file.close();
			break;
		}

		input_file>>reader.x1;
		input_file>>reader.y1;
		input_file>>reader.x2;
		input_file>>reader.y2;
		input_file>>temp;
		input_file>>temp;
		input_file>>temp;

		reader.slope = rad2deg(atan((reader.y2 - reader.y1)/(reader.x2 - reader.x1)));
		reader = calc_linevector_length(reader);
		classify_linevectors(reader);
	}
	find_linevector_concentration();
	return 0;
}