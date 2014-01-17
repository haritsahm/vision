#ifndef PATH_IMPLEMENTATION
#define PATH_IMPLEMENTATION

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <time.h>
#include <cstring>
#include <set>
#include <unistd.h>
#include "graph.hpp"
#include "../common/common.h"
enum Intersecting_Cases
	{
		no_intersection,normal_intersection,special_intersection,final_orientation_intersection
	};
class Path
{
private:
	//member variables
	static const std::size_t MAXIMUM=300;
	static const int SIZEX=200;
	static const int SIZEY=200;
	int NO_OF_OBSTACLES;
	static const int OBSTACLE_RADIUS=25;
	static const int ENCIRCLE_THRESHOLD=25;
	//static const int ORIENTATION_RADIUS=ENCIRCLE_THRESHOLD-5;
	static const int KICKDIST=5; //dist in cm from ball where kicking is done
	static const int ORIENTEDANGLECONST=20;//this is the distance from the ball at which a -45 to 45 degree ballangle will return oriented and then kick
	static const int BADANGLEDIST=20;
	static const int STEPLENGTH=5;
	int com_id; //communication id
	double tolerance_angle;//minimum angle for encircling
	IplImage* image;
	enum circle_type {DNE,CIRCLE}; // 0-> does not exsist 1-> circle 2-> ellipse
	struct Point
	{
		double x;
		double y;
		std::size_t obstacle_id;
		std::size_t parent_id;
		double obstacle_radius;
		circle_type type;
	};
	Point start,ball,obstacle[20],goal;
	Point n1a, n1b, n2a, n2b;
	int temp_global_k;
	bool same_side_flag;
	bool midpoint_flag;
	graph_implementation::Graph<Point> tree; //tree is the instance of the class Graph in the header file.
	struct removed_index
	{		
		int connected_count; 
		int remove_count;
	};
	//member functions
	double cost_calculation(std::size_t source,std::size_t target, graph_implementation::Graph <Point> &tree);
	Intersecting_Cases  main_function(Point n1, Point n2);
	int closest_intersecting_obstacle(Point n1, Point n2);
	bool side_check_orientation(Point a,Point b);
	bool side_check(struct Point n1,struct Point n2, struct Point n1a, struct Point n2a);
	bool on_which_side(double a, double b,struct Point n1, struct Point n2,struct Point obstacle);
	void calculate_points(struct Point main_point,struct Point obstacle_centre, Point &p1, Point &p2);
	bool path_completed_flag;
	bool new_point_orientation(std::size_t point_index);
	int curve[100];
	int len_curve;
	int curvearray_index[50];
	int len_curvearray_index;
	int no_of_sticks;
	double sticks_r;
	double sticks_theta;
	double b;

	int ORIENTATION_RADIUS;
public:
	//note that this is in  radians
	//When DOORIENT is returned by the code take the curvearray(which will give the entire curve till the ball) otherwise take next which just gives the next step.

	Coords next; 
	Coords curvenext[100]; 
	int len_curvenext;
	//OBSOLETE->int path_return(PathStructure &ps); //main function to be called by behaviour // return values : 1: successful path   2: go at r,theta    3: encircle      4: path not found.
	PathReturns path_return(PathStructure ps);
	/*README - ABOUT RETURN VALUES
	*If path succesfully generated(case1 previously)-> update nextr and nexttheta as usual and return DOWALK
	*If close and there is no need to generate path(case2 previously), update nextr and nexttheta (and return DOWALK) till I reach ENCIRCLE_THRESHOLD_DIST(value to be decided during testing)
		*if I am at ENCIRCLE_THRESHOLD_DIST, send DOENCIRCLE(case3 previously) till i am not oriented.(encircling will be in steps of about 10-15 degrees so if ballangle is less than that ,then assume oriented)
	*if I am at ENCIRCLE_THRESHOLD_DIST and ORIENTED, then send DOWALK till i reach KICK_DIST, then send DOKICK, when ready for kick 
	*/
	void updatePathPacket();//0 if file writing to be off...else on.
	double NextEncircleAngle;
	Path()
	{
		NO_OF_OBSTACLES = 0;
		com_id=0;
		path_completed_flag=false;
		midpoint_flag=false;
		next.r=0;
		next.theta=0;
		len_curvenext=0;
		b=0;
		sticks_r=0;
		sticks_theta=0;
		no_of_sticks=0;
		len_curve=0;
		ORIENTATION_RADIUS=20;
	}
};
#endif
//-lopencv_highgui -lopencv_core -lopencv_imgproc
