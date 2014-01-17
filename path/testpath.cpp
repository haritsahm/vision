#include "path.hpp"

PathStructure ps; //Coming from main cognition.
PathPacket pathpackvar; //The class for writing the files.
void call_main() //Function made to replicate path calling from the cognition module.
{
	srand(time(NULL));
	

	ps.n_obstacles = 2;
	ps.absObstacles[0].x=31; 
	ps.absObstacles[0].y=-75;
	ps.absObstacles[1].x=7;
	ps.absObstacles[1].y=67;
	ps.absObstacles[2].x=-40;
	ps.absObstacles[2].y=-80;

	ps.ball.x =92;
	ps.ball.y =73;

	ps.goal.x=-39;
	ps.goal.y=-90;
	for(int i=0;i<ps.n_obstacles;i++)
	 {
	// 	ps.absObstacles[i].x=rand() % 200 - 100;
	// 	ps.absObstacles[i].y=rand() % 200 - 100;
		std::cout<<"obstacle "<<i<<" x value "<<ps.absObstacles[i].x<<std::endl;
		std::cout<<"obstacle "<<i<<" y value "<<ps.absObstacles[i].y<<std::endl;
	 }
	// ps.ball.x=rand() % 200 - 100;
	// ps.ball.y=rand() % 200 - 100;	
	// ps.goal.x = rand() % 200 - 100;
	// ps.goal.y = rand() % 200 - 100;
	std::cout<<"ball "<<ps.ball.x<<" "<<ps.ball.y<<std::endl;	
	std::cout<<"goal "<<ps.goal.x<<" "<<ps.goal.y<<std::endl;	

	int r=200;
	int theta=100;
	
	Path p;
	// for(int r=50;r>0;r--)
	// 	{		
	// 		ps.ball.x=r*cos(deg2rad(theta));
	// 		ps.ball.y=r*sin(deg2rad(theta));
	// 		std:cout<<"\n\nr...."<<r<<"\n\n\n";
	// 		p.path_return(ps); // the main function that is called which returns the final path.
	// 		cvWaitKey();
	// 	}
	p.path_return(ps);
	p.updatePathPacket();//true if file writing to be on...else off.
	cvWaitKey();
}
int main()
{
	call_main();
	return 0;
}

