
#include "path.hpp"
#define car2pol(x,y) sqrt((x)*(x)+(y)*(y))
#define dist(x1,y1,x2,y2) sqrt(pow(((x1)-(x2)),2)+pow(((y1)-(y2)),2))
// #define FRAMEPAINTING
//#define OLDENCIRCLING
//#define NEWENCIRCLING
//#define NEWNEWENCIRCLING
#define OLDDELETION
// #define NEWDELETION


using namespace std;
using namespace graph_implementation;
void Path::calculate_points(struct Point main_point,struct Point obstacle_centre, Point &p1, Point &p2) //Returns you the four points on the obstacle for drawing the tangent.
{
	double m1,m2;
	m1=(((main_point.x-obstacle_centre.x)*(main_point.y-obstacle_centre.y))+(obstacle_centre.obstacle_radius*sqrt((pow((main_point.x-obstacle_centre.x),2))+(pow((main_point.y-obstacle_centre.y),2))-(pow((double)obstacle_centre.obstacle_radius,2)))))/(pow((main_point.x-obstacle_centre.x),2)-pow((double)obstacle_centre.obstacle_radius,2));
	if(((main_point.y-obstacle_centre.y)-(m1*(main_point.x-obstacle_centre.x)))<0)
	{
		p1.x=(m1*obstacle_centre.obstacle_radius)/(sqrt(1+pow(m1,2)))+obstacle_centre.x;
		p1.y=-(obstacle_centre.obstacle_radius)/(sqrt(1+pow(m1,2)))+obstacle_centre.y;
	}
	else
	{
		p1.x=-(m1*obstacle_centre.obstacle_radius)/(sqrt(1+pow(m1,2)))+obstacle_centre.x;
		p1.y=(obstacle_centre.obstacle_radius)/(sqrt(1+pow(m1,2)))+obstacle_centre.y;
	}
	m2=(((main_point.x-obstacle_centre.x)*(main_point.y-obstacle_centre.y))-(obstacle_centre.obstacle_radius*sqrt((pow((main_point.x-obstacle_centre.x),2))+(pow((main_point.y-obstacle_centre.y),2))-(pow((double)obstacle_centre.obstacle_radius,2)))))/(pow((main_point.x-obstacle_centre.x),2)-pow((double)obstacle_centre.obstacle_radius,2));
	if(((main_point.y-obstacle_centre.y)-(m2*(main_point.x-obstacle_centre.x)))<0)
	{
		p2.x=(m2*obstacle_centre.obstacle_radius)/(sqrt(1+pow(m2,2)))+obstacle_centre.x;
		p2.y=-(obstacle_centre.obstacle_radius)/(sqrt(1+pow(m2,2)))+obstacle_centre.y;
	}
	else
	{
		p2.x=-(m2*obstacle_centre.obstacle_radius)/(sqrt(1+pow(m2,2)))+obstacle_centre.x;
		p2.y=(obstacle_centre.obstacle_radius)/(sqrt(1+pow(m2,2)))+obstacle_centre.y;
	}
}

bool Path::on_which_side(double a, double b,struct Point n1, struct Point n2,struct Point obstacle) //returns true if on the other side of the origin....i.e. the side favourable to us
{
	double m,c1,c2;
	m=b/a;
	c1=n1.y-(m*n1.x);
	c2=n2.y-(m*n2.x);
	if(((obstacle.y-(m*obstacle.x)-c1)*(obstacle.y-(m*obstacle.x)-c2))<0)
		return true;
	else
		return false;
}

bool Path::side_check(struct Point n1,struct Point n2, struct Point n1a, struct Point n2a)//returns true if and only if n1a and n2a are on the same side of the line joining
{
	double m,c;
	m=(n2.y-n1.y)/(n2.x-n1.x);
	c=n1.y-(m*n1.x);
	if(((n1a.y-(m*n1a.x)-c)*(n2a.y-(m*n2a.x)-c))>0)
		return true;
	else
		return false;
}

bool Path::side_check_orientation(Point a,Point b) // a is the parent point , b is the child point
{
	cout<<" the coordinates for the parent points are "<<a.x<<" "<<a.y<<endl;
	cout<<" the coordinates for the child points are "<<b.x<<" "<<b.y<<endl;
	double m = (a.y-ball.y)/(a.x-ball.x);
	cout<<"m "<<m<<endl;
	double c = ball.y-(m*ball.x);
	cout<<"c "<<c<<endl;
	cout<<"the coordinates of the ball are "<<ball.x<<" "<<ball.y<<endl;
	cout<<"the equation of the line joining ball and the parent point is y="<<m<<"x+"<<c<<endl;
	if(((goal.y-(m*goal.x)-c)*(b.y-(m*b.x)-c))<0) // other side of the line...point to be kept

		return true;
	else
		return false;
}

bool Path::new_point_orientation(size_t point_index)
{
	double m=(goal.y-ball.y)/(goal.x-ball.x);
	double m_prime=-1/m;
	double c=ball.y-m_prime*ball.x;
	if((goal.y-m_prime*goal.x-c)*(tree[point_index].y-m_prime*tree[point_index].x-c)<0) //i.e the point is same side as the goal wrt the line per to joing the goal and ball
		return true;
	else
		return false;
}


int Path::closest_intersecting_obstacle(Point n1,Point n2) //Returns the closest intersecting obstacles in the interested region.
{
	double a,b,c,slope,dist,min_dist_from_n1=DBL_MAX,dist_from_n1;//k is initially -1,if no obstacle is found then k remains -1
	int k = -1;
	slope=(n2.y-n1.y)/(n2.x-n1.x);
	b=1;
	a=-slope;
	c=(slope*n2.x)-n2.y;
	for(int i=0;i<NO_OF_OBSTACLES+2;i++)
	{
		if(on_which_side(a,b,n1,n2,obstacle[i]))//i.e only the obstacles which are in between the two lines will be checked
		{
			dist=fabs((a*obstacle[i].x)+(b*obstacle[i].y)+(c))/sqrt(pow(a,2)+pow(b,2));
			if(dist<obstacle[i].obstacle_radius-0.05)
			{
				dist_from_n1=sqrt(pow((n1.x - obstacle[i].x),2)+pow((n1.y - obstacle[i].y),2));
				if(dist_from_n1<min_dist_from_n1)
				{
					min_dist_from_n1 = dist_from_n1;
					k=i;
				}
			}
		}
	}
	return k;
}

Intersecting_Cases Path::main_function(Point n1, Point n2)
{
	temp_global_k=0;
	same_side_flag=true;
	int k=closest_intersecting_obstacle(n1,n2);
	if(k==-1)
	{
		////////cout<<"\t\tNo Intersection\n";
		return no_intersection;
	}

	if((k == NO_OF_OBSTACLES || k == NO_OF_OBSTACLES +1) && n2.x==ball.x && n2.y==ball.y)
	{
		calculate_points(n1,obstacle[k],n1a,n1b);
		n1a.obstacle_id=k;
		n1b.obstacle_id=k;
		////////cout<<"\t\tFinal Orientation Intersection\n";
		return final_orientation_intersection;
	}

	if(n1.obstacle_id==k || n2.obstacle_id==k)
	{
		Point midpoint, p1, p2;
		p1.x=0;
		p2.x=0;
		p1.y=0;
		p2.y=0;
		midpoint.x=(obstacle[n1.obstacle_id].x + obstacle[n2.obstacle_id].x)/2;
		midpoint.y=(obstacle[n1.obstacle_id].y + obstacle[n2.obstacle_id].y)/2;
		if((((pow((midpoint.x-obstacle[n1.obstacle_id].x),2))+(pow((midpoint.y-obstacle[n1.obstacle_id].y),2))-pow(obstacle[n1.obstacle_id].obstacle_radius,2))>0) && (((pow((midpoint.x-obstacle[n2.obstacle_id].x),2))+(pow((midpoint.y-obstacle[n2.obstacle_id].y),2))-pow(obstacle[n2.obstacle_id].obstacle_radius,2))>0))
		{
			calculate_points(midpoint,obstacle[n1.obstacle_id],p1,p2);
			double dist1,dist2,min;
			int z=1;
			dist1=sqrt(pow(p1.x-n1.x,2)+pow(p1.y-n1.y,2));
			min=dist1;
			dist2=sqrt(pow(p2.x-n1.x,2)+pow(p2.y-n1.y,2));
			if(dist2<min)
			{
				min=dist2;
				z=2;
			}
			if(z==1)
				n1a=p1;
			else
				n1a=p2;
			calculate_points(midpoint,obstacle[n2.obstacle_id],p1,p2);
			z=1;
			dist1=sqrt(pow(p1.x-n2.x,2)+pow(p1.y-n2.y,2));
			min=dist1;
			dist2=sqrt(pow(p2.x-n2.x,2)+pow(p2.y-n2.y,2));
			if(dist2<min)
			{
				min=dist2;
				z=2;
			}
			if(z==1)
				n2a=p1;
			else
				n2a=p2;
			temp_global_k = k;
		}
		else
		{
			midpoint_flag=true;
		}
		////////cout<<"\t\tSpecial Intersection\n";
		return special_intersection;
	}
	temp_global_k = k;
	calculate_points(n1,obstacle[k], n1a, n1b);
	calculate_points(n2,obstacle[k], n2a, n2b);
	n1a.obstacle_id=k;
	n1b.obstacle_id=k;
	n2a.obstacle_id=k;
	n2b.obstacle_id=k;
	same_side_flag=side_check(n1,n2,n1a,n2a); //same_side_flag is true if and only if they are on the same side
	////////cout<<"\t\tNormal Intersection\n";
	return normal_intersection;
}

double Path::cost_calculation(std::size_t source,std::size_t target, Graph <Point> &tree) //Returns cost of the path b/w the two points given. Note that the if the points are on circle then the cost is calculated accordingly.
{
	double cost;
	Point source_point=tree[source];
	Point target_point=tree[target];
	if(tree.is_onCircle(source,target)==false) //
	{
		cost=sqrt(pow((source_point.y - target_point.y),2)+pow((source_point.x - target_point.x),2));
	}
	else
	{
		assert(source_point.obstacle_id==target_point.obstacle_id);
		Point obstacle_point=obstacle[source_point.obstacle_id];
		double m1=(source_point.y - obstacle_point.y)/(source_point.x - obstacle_point.x);
		double m2=(target_point.y - obstacle_point.y)/(target_point.x - obstacle_point.x);
		double m=fabs((m1-m2)/(1+(m1*m2)));
		double theta=atan(m);
		cost=obstacle_point.obstacle_radius*theta;
	}
	return cost;
}

PathReturns Path::path_return(PathStructure ps)
{
	cout<<".....................................................PATH CALLED.......................................................";
	srand(time(NULL));
	IplImage* image= cvCreateImage(cvSize(SIZEX*2, SIZEY*2), 8, 3); //displaying the results.
	cvZero(image);
	int flag=1; //Flag used for iteration in the graph...i.e. assigned the value 0 once iteration through the entire graph is done to break out of loop.
	start.x=0;
	start.y=0;
	start.obstacle_id=-1;
	start.parent_id=0;
	cvCircle(image, cvPoint(start.x + 200, start.y + 200), 2, cvScalar(0,255,0)); //Initialising the start point and painting it.
	NO_OF_OBSTACLES = ps.n_obstacles;
	ball.x=ps.ball.x;
	ball.y=ps.ball.y;
	ball.obstacle_id=-1;
	ball.parent_id=0;
	//////cout<<"Ball Position:\n"<<ball.x<<" "<<ball.y<<endl;
	cvCircle(image, cvPoint(ball.x+200, ball.y+200), 2, cvScalar(255,0,255)); //Initialising the ball point and painting it.
	goal.x=ps.goal.x;
	goal.y=ps.goal.y;
	goal.obstacle_id=-1;
	//////cout<<"Goal Position\n"<<goal.x<<" "<<goal.y<<endl;
	cvCircle(image, cvPoint(goal.x+200, goal.y+200), 10, cvScalar(255,0,255)); //Initialising the goal point and painting it.
	tree.cleartree(); //Clearing all previous data from the graphs....i.e. initialises the graph for a fresh start.
	for(int i = 0; i < NO_OF_OBSTACLES; i++)
	{
		obstacle[i].x = ps.absObstacles[i].x;
		obstacle[i].y = ps.absObstacles[i].y;
		obstacle[i].obstacle_radius=OBSTACLE_RADIUS;
		obstacle[i].type= CIRCLE;
		//////cout<<"Obstacle Position(s)\n"<<obstacle[i].x<<" "<<obstacle[i].y<<endl;
		cvCircle(image, cvPoint(obstacle[i].x+200,obstacle[i].y+200), obstacle[i].obstacle_radius, cvScalar(255,0,0));
	} //Initialising the obstacles and painting them.
	cvCircle(image, cvPoint(ball.x+200, ball.y+200), ENCIRCLE_THRESHOLD , cvScalar(0,130,70));
	//Making the two orientation markers near the ball  ---->
	double m=(goal.y-ball.y)/(goal.x-ball.x);
	double m_prime=-1/m;
	double cos_prime=1/(sqrt(pow(m_prime,2)+1));
	double sin_prime=m_prime/(sqrt(pow(m_prime,2)+1));
#ifdef NEWNEWENCIRCLING
	if(car2pol(ball.x,ball.y) <= 2*ORIENTATION_RADIUS)
	{
		ORIENTATION_RADIUS=car2pol(ball.x,ball.y)/2-0.5;

	}

#endif
	obstacle[NO_OF_OBSTACLES].obstacle_radius= ORIENTATION_RADIUS;
	obstacle[NO_OF_OBSTACLES].x=ball.x - ( ORIENTATION_RADIUS*cos_prime);
	obstacle[NO_OF_OBSTACLES].y=ball.y - ( ORIENTATION_RADIUS*sin_prime);
	obstacle[NO_OF_OBSTACLES].obstacle_id=-2;
	obstacle[NO_OF_OBSTACLES].type= CIRCLE;
	cvCircle(image, cvPoint(obstacle[NO_OF_OBSTACLES].x+200, obstacle[NO_OF_OBSTACLES].y+200),  ORIENTATION_RADIUS, cvScalar(255,0,0));
	obstacle[NO_OF_OBSTACLES+1].obstacle_radius= ORIENTATION_RADIUS;
	obstacle[NO_OF_OBSTACLES+1].x=ball.x + ( ORIENTATION_RADIUS*cos_prime);
	obstacle[NO_OF_OBSTACLES+1].y=ball.y + ( ORIENTATION_RADIUS*sin_prime);
	obstacle[NO_OF_OBSTACLES+1].obstacle_id=-2;
	obstacle[NO_OF_OBSTACLES+1].type= CIRCLE;
	cvCircle(image, cvPoint(obstacle[NO_OF_OBSTACLES+1].x+200, obstacle[NO_OF_OBSTACLES+1].y+200),  ORIENTATION_RADIUS, cvScalar(255,0,0));
	for(int i=0;i< NO_OF_OBSTACLES;i++)
	{
		for(int j=NO_OF_OBSTACLES;j<NO_OF_OBSTACLES+2;j++)
		{
			double d=sqrt(pow((obstacle[i].x - obstacle[j].x),2)+pow((obstacle[i].y - obstacle[j].y),2));
			if(d < (obstacle[i].obstacle_radius+obstacle[j].obstacle_radius)-0.005)
			{										//now the obstacles are intersecting
				obstacle[j].x=1000;
				obstacle[j].y=1000;
			}

		}

	}
//implementing circle design
	for(int i=0;i< NO_OF_OBSTACLES;i++)
	{
		for(int j=i+1;j<NO_OF_OBSTACLES;j++)
		{
			double d=sqrt(pow((obstacle[i].x - obstacle[j].x),2)+pow((obstacle[i].y - obstacle[j].y),2));

			if(d < (obstacle[i].obstacle_radius+obstacle[j].obstacle_radius)-0.005)
			{//now the obstacles are intersecting
				obstacle[i].x=obstacle[i].x/2+obstacle[j].x/2;
				obstacle[i].y=obstacle[i].y/2+obstacle[j].y/2;
				obstacle[i].obstacle_radius= (obstacle[i].obstacle_radius+obstacle[j].obstacle_radius+d)/2;
				obstacle[j].type=DNE;
			}
		}
	}
	for(int i=0;i<NO_OF_OBSTACLES+2;i++)
	{
		if(obstacle[i].type==DNE)
		{
			for (int j = i; j < NO_OF_OBSTACLES-1+2; ++j)
			{
				obstacle[j]=obstacle[j+1];
			}
			NO_OF_OBSTACLES--;
		}
	}
	for(int i=0;i<NO_OF_OBSTACLES+2;i++)
	{
		if(dist(obstacle[i].x,obstacle[i].y,0,0)<obstacle[i].obstacle_radius)
		{
			obstacle[i].x=1000;
			obstacle[i].y=1000;
		}
	}

#ifdef OLDENCIRCLING
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
	//Encircling, Walking, Kicking decision ----->

	cvShowImage("Field", image);

	if(car2pol(ball.x,ball.y) <= KICKDIST)
	{
		cout<<"\npath returning \n\nDOKICK\n\n " ;

		return DOKICK;
	}
	//ENCIRCLE_THRES is always > KICKDIST
	else if(car2pol(ball.x,ball.y) <= ENCIRCLE_THRESHOLD)
	{
		cout<<"\n\nless than encircel threshold......\n\n";
		//ballgoalangle=>angle made by line joining acyut and ball with line joining ball and goal
		tolerance_angle=rad2deg(atan(BADANGLEDIST/sqrt(pow(ball.x,2)+pow(ball.y,2)))); //dyanmically generating tolerance angle for orientation purposes(i.e facing the ball before orienting)
		double ballangle=rad2deg(atan2(ball.y,ball.x));
		if(abs(ballangle)>tolerance_angle)
		{
			if(ballangle>0)
			{
				next.theta=(abs(ballangle)-tolerance_angle)*-1; //-1 is due to the ip convention and walk convention
				next.r=0;
				cout<<"\npath returning \n\nDOWALK\n\n  theta "<<next.theta<< " distance "<<next.r<<endl;
				return DOWALK;
			}
			else
			{
				next.theta=(-(abs(ballangle)-tolerance_angle))*-1;
				next.r=0;
				cout<<"\npath returning \n\nDOWALK\n\n  theta "<<next.theta<< " distance "<<next.r<<endl;
				return DOWALK;
			}
		} // acyut is now facing the ball .... ready to encircle
		double a=car2pol(ball.x,ball.y);
		double b=car2pol(ball.x-goal.x,ball.y-goal.y);
		double c=car2pol(goal.x,goal.y);
		double ballgoalangle=180- rad2deg(acos((a*a+b*b-c*c)/(2*a*b))); //using cosine rule to get angle required to encircle ( for understanding refer encircleangleidea.jpg in source)
		NextEncircleAngle=ballgoalangle*-1; // the -1 is due to ip convention and walk convention.
		cout<<"\npath returning \n\nDOENCIRCLE\n\n  theta "<<NextEncircleAngle<<endl;
		return DOENCIRCLE; //encircle
	}
	else if(car2pol(ball.x,ball.y) <= 2*ORIENTATION_RADIUS) //if i am inside the orientation circle while on the path then follow straight till u get to the encircle radius
	{
		//cvWaitKey();
		cout<<"\n\ninside 2*orientaion radius\n\n";
		double ballangle=rad2deg(atan2(ball.y,ball.x));
		next.r=car2pol(ball.x,ball.y);
		next.theta=ballangle;
		cout<<"\npath returning \n\nDOWALK\n\n  theta "<<next.theta<< " distance "<<next.r<<endl;
		return DOWALK;
	}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#endif
	tree.add_vertex(start);
	tree.add_vertex(ball);
	tree.add_edge(0,1,cost_calculation(0,1,tree));
	cvLine(image,cvPoint(200,200),cvPoint(ball.x+200,ball.y+200),cvScalar(255,255,0)); // creates the nodes for start and ball point and paints a line.

	while(flag) // iterates over the entire tree.
	{
		flag=0;
		for(size_t n1_index=0;n1_index<tree.size(); n1_index++)
		{
			for(size_t n2_index = 0; n2_index < tree.size(); n2_index++) // looping through the entire tree.
			{
				if(tree.is_edge(n1_index, n2_index))
				{
					cout<<"\nanalysing edges "<<n1_index<<" "<<n2_index<<endl;
					Point n1 = tree[n1_index];
					Point n2 = tree[n2_index];

					#ifdef FRAMEPAINTING
					//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
					//code for frame by frame painting.						//n1_indiex-->ni_index.....n2_index-->nj_index
						for(size_t ni_index=0;ni_index<tree.size(); ni_index++)
						{
							for(size_t nj_index = 0; nj_index < tree.size(); nj_index++)
							{
								if(tree.is_edge(ni_index, nj_index))
								{
									Point n1 = tree[ni_index];
									Point n2 = tree[nj_index];
									cvLine(image,cvPoint(n1.x+200,n1.y+200),cvPoint(n2.x+200,n2.y+200),cvScalar(255,255,0));
								}
							}
						}
						for(int j=0;j<NO_OF_OBSTACLES+2;j++)
						{
							cvCircle(image, cvPoint(obstacle[j].x+200, obstacle[j].y+200), 2, cvScalar(255,0,0));
							cvCircle(image, cvPoint(obstacle[j].x+200, obstacle[j].y+200), obstacle[j].obstacle_radius , cvScalar(255,0,0));
						}
						cvCircle(image, cvPoint(n1.x+200, n1.y+200), 2, cvScalar(255,255,255), 2);
						cvCircle(image, cvPoint(n2.x+200, n2.y+200), 2, cvScalar(0,0,255), 2);
						cvCircle(image, cvPoint(start.x + 200, start.y + 200), 2, cvScalar(0,255,0));
						cvCircle(image, cvPoint(ball.x+200, ball.y+200), 2, cvScalar(255,0,255));
						cvCircle(image, cvPoint(goal.x+200, goal.y+200), 10, cvScalar(255,0,255));
						cvShowImage("Field", image);
						cvZero(image);
					 	cvWaitKey();
					//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
					#endif
					if(tree.is_onCircle(n1_index, n2_index)==false)
					{
						switch(main_function(n1,n2))
						{

							case normal_intersection:
							{
								flag = 1;
								tree.remove_edge(n1_index, n2_index); //removing the edges for which there is an obstacle in b/w.
								n1a.parent_id=n1_index;							//
								size_t n1a_index = tree.add_vertex(n1a);			//
								n1b.parent_id=n1_index;
								size_t n1b_index = tree.add_vertex(n1b);			//
								n2a.parent_id=n1_index;									//adding the four new nodes
								size_t n2a_index = tree.add_vertex(n2a);			//
								n2b.parent_id=n1_index;								//
								size_t n2b_index = tree.add_vertex(n2b);			//
								tree.add_edge(n1_index, n1a_index,cost_calculation(n1_index,n1a_index,tree));
								if(closest_intersecting_obstacle(tree[n1.parent_id],tree[n1a_index])==-1)//check the obstacle intersection from n1's parent to the point n1a...n1b.....
								{

									n1a.parent_id=n1.parent_id;
									tree.update_vertex(n1a,n1a_index);
									tree.add_edge(n1.parent_id,n1a_index,cost_calculation(n1.parent_id,n1a_index,tree));//add edge b/w the parent point and the point n1a,nb...
								}
								tree.add_edge(n1_index, n1b_index,cost_calculation(n1_index,n1b_index,tree));
								if(closest_intersecting_obstacle(tree[n1.parent_id],tree[n1b_index])==-1)//check the obstacle intersection from n1's parent to the point n1a...n1b.....
								{
									n1b.parent_id=n1.parent_id;
									tree.update_vertex(n1b,n1b_index);
									tree.add_edge(n1.parent_id,n1b_index,cost_calculation(n1.parent_id,n1b_index,tree));//add edge b/w the parent point and the point n1a,nb...

								}
								tree.add_edge(n2a_index, n2_index,cost_calculation(n2a_index,n2_index,tree));
								if(closest_intersecting_obstacle(tree[n2a.parent_id],tree[n2_index])==-1)//check the obstacle intersection from n1's parent to the point n1a...n1b.....
								{
									n2.parent_id=n2a.parent_id;
									tree.update_vertex(n2,n2_index);
									tree.add_edge(n2a.parent_id,n2_index,cost_calculation(n2a.parent_id,n2_index,tree));//add edge b/w the parent point and the point n1a,nb...
								}
								tree.add_edge(n2b_index, n2_index,cost_calculation(n2b_index,n2_index,tree));
								if(closest_intersecting_obstacle(tree[n2b.parent_id],tree[n2_index])==-1)//check the obstacle intersection from n1's parent to the point n1a...n1b.....
								{
									n2.parent_id=n2b.parent_id;
									tree.update_vertex(n2,n2_index);
									tree.add_edge(n2b.parent_id,n2_index,cost_calculation(n2b.parent_id,n2_index,tree));//add edge b/w the parent point and the point n1a,nb...
								}
								if(same_side_flag)
								{
									tree.add_edge(n1a_index, n2a_index,cost_calculation(n1a_index,n2a_index,tree), true);
									tree.add_edge(n1b_index, n2b_index,cost_calculation(n1b_index,n2b_index,tree), true);
								}
								else
								{
									tree.add_edge(n1a_index, n2b_index,cost_calculation(n1a_index,n2b_index,tree), true);
									tree.add_edge(n1b_index, n2a_index,cost_calculation(n1b_index,n2a_index,tree), true);
								}
								break;
							}
							case special_intersection:
							{
								if(midpoint_flag==false)
								{
									flag=1;
									//the special function returns n1a and n1b
									tree.remove_edge(n1_index,n2_index);
									n1a.parent_id=n1_index;
									n2a.parent_id=n1_index;
									size_t n1a_index = tree.add_vertex(n1a);
									size_t n2a_index = tree.add_vertex(n2a);
									tree.add_edge(n1_index, n1a_index,cost_calculation(n1_index,n1a_index,tree), true);
									for(std::size_t i=0; i<tree.size();i++)
									{
										if(tree.is_edge(n2_index,i))
										{
											if(tree.is_onCircle(n2_index,i))
												tree.add_edge(n2a_index, i,cost_calculation(n2a_index,i,tree), true);
											else
												tree.add_edge(n2a_index, i,cost_calculation(n2a_index,i,tree));
											tree.remove_edge(n2_index,i);
										}
									}
									tree.add_edge(n1a_index, n2a_index,cost_calculation(n1a_index,n2a_index,tree));


									// tree.add_edge(n2a_index, n2_index,cost_calculation(n2a_index,n2_index,tree),true);
									// tree.remove_edge(n1a_index,n2a_index);
									// tree.remove_edge(n2_index,n2a_index);
									// tree.add_edge(n1a_index, n2a_index,cost_calculation(n1a_index,n2a_index,tree), true);
									//note the circles on which n1a and n2a are lying.....
									break;
								}
								else
									break;

							}
							case no_intersection:
							{
								break;
							}
							case final_orientation_intersection:
							{

								flag=1;
								tree.remove_edge(n1_index,n2_index);
								size_t n1a_index = tree.add_vertex(n1a);
								size_t n1b_index = tree.add_vertex(n1b);
								tree.add_edge(n1_index, n1a_index,cost_calculation(n1_index,n1a_index,tree));
								if(closest_intersecting_obstacle(tree[n1.parent_id],tree[n1a_index])==-1)//check the obstacle intersection from n1's parent to the point n1a...n1b.....
								{
									n1a.parent_id=n1.parent_id;
									tree.update_vertex(n1a,n1a_index);
									tree.add_edge(n1.parent_id,n1a_index,cost_calculation(n1.parent_id,n1a_index,tree));//add edge b/w the parent point and the point n1a,nb...
								}
								tree.add_edge(n1_index, n1b_index,cost_calculation(n1_index,n1b_index,tree));
								if(closest_intersecting_obstacle(tree[n1.parent_id],tree[n1b_index])==-1)//check the obstacle intersection from n1's parent to the point n1a...n1b.....
								{
									n1b.parent_id=n1.parent_id;
									tree.update_vertex(n1b,n1b_index);
									tree.add_edge(n1.parent_id,n1b_index,cost_calculation(n1.parent_id,n1b_index,tree));//add edge b/w the parent point and the point n1a,nb...
								}
								tree.add_edge(n1a_index,n2_index,cost_calculation(n1a_index,n2_index,tree),true);
								tree.add_edge(n1b_index,n2_index,cost_calculation(n1b_index,n2_index,tree),true);
							}
						}
							if(flag)
							break;
					}
				}
			}
			if(flag)
				break;
		}
	}
	removed_index to_be_removed[tree.size()];
	for(std::size_t i=1;i<tree.size();i++)
	{
		to_be_removed[i].connected_count=0;
		to_be_removed[i].remove_count=0;
	}


#ifdef OLDDELETION
	for(std::size_t i=2;i<tree.size();i++) //code for final orientation i.e deleting the node from whose side we do not orient.
	{
		if(tree.is_edge(i,1)) //i is the point connected to 1
		{
			cout<<"\n\n\n\n\n\n\n\n\n";
			cout<<" node "<<i<<" is connected to 1\n";
			for(std::size_t j =0 ; j < tree.size() ; j++)
			{
				if(tree.is_edge(j,i)) //j is the point connected to i
				{
					cout<<"node "<<j<< " is further connected to this "<<i<<endl;
					cout<<"increasing the connected count \n";
					to_be_removed[i].connected_count+=1;
					if(!side_check_orientation(tree[j],tree[i]))
					{
						cout<<"side check orientation passed increasing the remove conunt\n";
						to_be_removed[i].remove_count+=1;
					}
					else
						cout<<"oops side check orientaion failed for the nodes "<<i<<" and "<<j<<endl;
				}

			}
		}
	}
	cout<<"\nto be removed remove count "<<to_be_removed[11].remove_count<<" "<<"to be removed connected count "<<to_be_removed[11].connected_count<<endl;
#endif

#ifdef NEWDELETION
	for(std::size_t i=2;i<tree.size();i++) //code for final orientation i.e deleting the node from whose side we do not orient.
	{
		if(tree.is_edge(i,1)) //i is the point connected to 1
		{
			if(!new_point_orientation(i))
			{
				tree.remove_edge(i,1);
			}
		}
	}

#endif

	for(std::size_t i=1;i<tree.size();i++)
	{
		if(to_be_removed[i].remove_count==to_be_removed[i].connected_count && to_be_removed[i].remove_count!=0)
		{
			tree.remove_edge(i,1);
			cout<<"\nremoving edge  "<<i<<endl;;
		}
	}
	for(size_t n1_index=0;n1_index<tree.size(); n1_index++)
	{
		for(size_t n2_index = 0; n2_index < tree.size(); n2_index++)
		{
			if(tree.is_edge(n1_index, n2_index))
			{
				Point n1 = tree[n1_index];
				Point n2 = tree[n2_index];
				cvLine(image,cvPoint(n1.x+200,n1.y+200),cvPoint(n2.x+200,n2.y+200),cvScalar(255,255,0));
			}
		}
	}
	//////cout<<"\nCalling Dijkstras\n";
	tree.dijkstra();
	std::size_t a,b;
	a=1;
	//////cout<<"Before return path point 1";
	b=tree.returnPathPoint(1);
	while(b!=0)
	{
		cvLine(image,cvPoint(tree[b].x+200,tree[b].y+200),cvPoint(tree[a].x+200,tree[a].y+200),cvScalar(0,0,255));
		a=b;
		b=tree.returnPathPoint(b);
	}
	path_completed_flag=true;
	for(int i=0;i<NO_OF_OBSTACLES+2;i++)
	{
		cvCircle(image, cvPoint(obstacle[i].x + 200, obstacle[i].y+200), obstacle[i].obstacle_radius, cvScalar(255,0,0));
	}
	cvCircle(image, cvPoint(start.x + 200, start.y + 200), 2, cvScalar(0,255,0));
	cvCircle(image, cvPoint(ball.x+200, ball.y+200), 2, cvScalar(255,0,255));
	cvCircle(image, cvPoint(goal.x+200, goal.y+200), 10, cvScalar(255,0,255));
	cvLine(image,cvPoint(tree[0].x+200,tree[0].y+200),cvPoint(tree[a].x+200,tree[a].y+200),cvScalar(0,0,255));
	cvShowImage("Field", image);
	//--->> calculating the next points r and theta.
	if(tree.is_onCircle(0, a))
	{
		next.r=(10*180/3.1415926)*obstacle[tree[a].obstacle_id].obstacle_radius;
		double m_prime= (tree[a].y - obstacle[tree[a].obstacle_id].y)/(tree[a].x - obstacle[tree[a].obstacle_id].x);
		double m=-1/m_prime;
		next.theta=rad2deg(atan(m));
		//////cout<<"\nnext.r "<<next.r;
		//////cout<<"\nnext.theta "<<next.theta;
	}
	else
	{
		next.r = sqrt(pow((tree[a].x-tree[0].x),2)+pow((tree[a].y-tree[0].y),2));
		next.theta=rad2deg((atan2((tree[a].y-tree[0].y),(tree[a].x-tree[0].x))));
		//////cout<<"\nnext.r "<<next.r;
		//////cout<<"\nnext.theta "<<next.theta;
	}
	// std::size_t init=1;
	// curvearray_index[0]=init;
	// len_curvearray_index=1;
	// while(init!=0)
	// {
	// 	curvearray_index[len_curvearray_index]=tree.returnPathPoint[init];
	// 	init=tree.returnPathPoint[init];
	// }
	// len_curvearray_index++;
	// for(int i=0;i<len_curvearray_index;i++)
	// {
	// 	cout<<"curvearray_index "<<curvearray_index[i]<<"\n";
	// }
















//copying it to an array
	std::size_t c,d;
	d=1;
	curve[0]=1;
	cout<<curve[0]<<"\t\t";
	for(int i=0;i<tree.size();i++)
	{
		curve[i+1]=tree.returnPathPoint(d);
		d=curve[i+1];
		cout<<curve[i+1]<<"\t\t";
		if(d==0)
		{
			len_curve=i+2;
			break;
		}
	}
	//cout<<"\nlen_curve: "<<len_curve<<"\n";
	cout<<endl;
	std::size_t tmp=0;
	len_curvenext=0;
	for(int i=0,j=len_curve-1;i<len_curve/2,j>=len_curve/2;i++,j--)
	{
			tmp=curve[i];
			curve[i]=curve[j];
			curve[j]=tmp;
	}

	for(int i=0,j=0;i<len_curve-1;i++)
	{
		cout<<"\n.............................\nanalysing "<<curve[i]<<" "<<curve[i+1]<<endl<<"...\n";
		if(!tree.is_onCircle(curve[i],curve[i+1]))
		{
			cout<<"straight line\n";
			curvenext[j].r=sqrt(pow((tree[curve[i+1]].x-tree[curve[i]].x),2)+pow((tree[curve[i+1]].y-tree[curve[i]].y),2));
			curvenext[j].theta=rad2deg((atan2((tree[curve[i+1]].y-tree[curve[i]].y),(tree[curve[i+1]].x-tree[curve[i]].x))));
			cout<<"curvenext["<<j<<"].r "<<curvenext[j].r<<endl;
			cout<<"curvenext["<<j<<"].theta "<<curvenext[j].theta<<endl;
			j++;

			//cout<<"\nlen_curvenext "<<len_curvenext++;
		}

		else
		{
			cout<<"on curve \n";
			sticks_r=STEPLENGTH;
			cout<<"\nsticks_r "<<sticks_r<<endl;
			sticks_theta=2*asin(STEPLENGTH/(2*obstacle[tree[curve[i]].obstacle_id].obstacle_radius));
			cout<<"\nsticks_theta "<<sticks_theta<<endl;
			b=sqrt(pow((tree[curve[i+1]].x-tree[curve[i]].x),2)+pow((tree[curve[i+1]].y-tree[curve[i]].y),2));
			no_of_sticks=(int)2/sticks_theta*asin(b/(2*obstacle[tree[curve[i]].obstacle_id].obstacle_radius));
			cout<<"\nno_of_sticks "<<no_of_sticks<<endl;
			while(no_of_sticks--)
			{
				curvenext[j].r=sticks_r;
				cout<<"curvenext["<<j<<"].r "<<curvenext[j].r<<endl;
				if(((atan2((tree[curve[i+1]].y-tree[curve[i]].y),(tree[curve[i+1]].x-tree[curve[i]].x))))>0)
				{
					curvenext[j].theta=rad2deg(sticks_theta);
					cout<<"\ncurvenext[j].theta "<<curvenext[j].theta<<endl;;
				}
				else
				{
					curvenext[j].theta=rad2deg(-sticks_theta);
					cout<<"\ncurvenext[j].theta "<<curvenext[j].theta<<endl;;
				}
				cout<<"curvenext["<<j<<"].theta "<<curvenext[j].theta<<endl;
				j++;
				len_curvenext++;
			}
		}
	 }
	cout<<"\nlen_curvenext "<<len_curvenext;
	cout<<"\ncurvenext.r\n";
	for(int i=0;i<len_curvenext;i++)
	{
		cout<<curvenext[i].r<<"\t";
	}
	cout<<"\ncurvenext.theta\n";
	for(int i=0;i<len_curvenext;i++)
	{
		cout<<curvenext[i].theta<<"\t";
	}
	//cout<<"\ncurvearray normally..\n";


#ifdef NEWENCIRCLING
//^^^^^^^^^^^^^^^^^^^^^^^^^^^new encircling and kicking and positioning^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	if(car2pol(ball.x,ball.y)>2*ORIENTATION_RADIUS)
	{
		len_curvearray=len_curvenext;
		for(int i=0;i<len_curvearray;i++)
		{
			curvearray[i].r=curvenext[i].r*10;
			curvearray[i].theta=rad2deg(curvenext[i].theta);
			cout<<curvearray[i].r<<"\t"<<curvearray[i].theta<<"\n";
		}

	}
	if(car2pol(ball.x,ball.y)<2*ORIENTATION_RADIUS)
	{
		//usleep(5000000);
		cout<<"\n\n\n\n\n\n\n\n\n\n\n\n\n..............calling DOORIENT........................\n\n\n\n\n\n\n\n\n\n\n\n\n";
		cout<<"\ncurvearray that is passed \n";
		for(int i=0;i<len_curvearray;i++)
		{
			cout<<curvearray[i].r<<"\t"<<curvearray[i].theta<<"\n";
		}
		cout<<"\n\n\n\n\n\n\n\n";
		//cvWaitKey();
		return DOORIENT;
	}

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#endif

	for(int i=0;i<NO_OF_OBSTACLES+2;i++)
	{
		cvCircle(image, cvPoint(obstacle[i].x + 200, obstacle[i].y+200), obstacle[i].obstacle_radius, cvScalar(255,0,0));
	}
	cvCircle(image, cvPoint(start.x + 200, start.y + 200), 2, cvScalar(0,255,0));
	cvCircle(image, cvPoint(ball.x+200, ball.y+200), 2, cvScalar(255,0,255));
	cvCircle(image, cvPoint(goal.x+200, goal.y+200), 10, cvScalar(255,0,255));
	cvLine(image,cvPoint(tree[0].x+200,tree[0].y+200),cvPoint(tree[a].x+200,tree[a].y+200),cvScalar(0,0,255));
	cvShowImage("Field", image);
	if(tree.no_path_flag==-1)
	{
		tree.no_path_flag=0;
		return NOPATH;
	}
	cout<<"\npath returning \n\nDOWALK\n\n  theta "<<next.theta<< " distance "<<next.r<<endl;

    return DOWALK;
}

void Path::updatePathPacket()
{
	if(path_completed_flag)
	{

		pathpackvar.id=com_id;
		com_id=com_id+1;
		std::size_t b;
		b=tree.returnPathPoint(1);
		pathpackvar.finalpath[0].x=ball.x;
		pathpackvar.finalpath[0].y=ball.y;
		assert(tree.size()<30);
		for(int i=1;i<30;i++)
		{
			if(b==0)
				break;
			pathpackvar.finalpath[i].x=tree[b].x;
			pathpackvar.finalpath[i].y=tree[b].y;
			b=tree.returnPathPoint(b);
			pathpackvar.no_of_points=i;
		}
		////////cout<<"after for in update path packet\n";
		if(pathpackvar.no_of_points<29)
		{
			pathpackvar.finalpath[pathpackvar.no_of_points+1].x=-1;
			pathpackvar.finalpath[pathpackvar.no_of_points+1].y=-1;
		}
	}
}
