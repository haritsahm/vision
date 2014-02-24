#include "camcontrol.h"


CamControl::CamControl(CamCapture &cam) : IMAGE_WIDTH(cam.width()), IMAGE_HEIGHT(cam.height()), 
CENTRE_RECT_X1(IMAGE_WIDTH/4),	
CENTRE_RECT_Y1 (IMAGE_HEIGHT/4),
CENTRE_RECT_X2 ((IMAGE_WIDTH*4)/4),
CENTRE_RECT_Y2 ((IMAGE_HEIGHT*4)/4)
{
	pass_counter = 0;	
	state_of_motion = 0;
	prev_img_flag = 0;	
	
}



//TODO: Why were 30 and 70 degrees chosen as angles? (this is in findball after adding lower/upper)
//Think of what angles might be best and use those. Also give reason
void CamControl::moveSearch(HeadMotor &hm, int lower, int upper)
{
	hm.speed_motor(HeadMotor::MOTOR_SPEED,700);
	if(state_of_motion==0)
	{
		state_of_motion = 1;
		hm.write_motor(upper, LEFT_LIMIT);
	}
	else
	{
		state_of_motion = 0;
		hm.write_motor(lower, RIGHT_LIMIT);
	}
	return;
}



void CamControl::search(HeadMotor &hm)
{
	if(!hm.ismoving_motor())
		moveSearch(hm, 65,65);
}



bool CamControl::inCentreRect(int x, int y, int x1, int y1, int x2, int y2)
{
	if((x>=x1)&&(x<=x2))
	{ 
		if((y>=y1)&&(y<=y2))
		{
			return true;
		}
	}
	return false;
}



//Returns 0 on success.	-2: can't move along x, -4: unable to read, -10: turn left, -20: turn right
// OLD: -1 : cant move along x, -2: can't move along y, -3: can't move along x or y, -4: unable to read
//TODO: Should probably enum it, but I'm leaving it as it is right now..
//TODO: try using pid to get ball in centre
int CamControl::moveToPoint(int x, int y, HeadMotor &hm)
{
	// float hori = 0;
	// float vert = 0;
	float theta_x, theta_y;
	int motor_X,motor_Y;
	int return_value = 0;

	hm.speed_motor(700,700);//HeadMotor::MOTOR_SPEED,700);
	
	hm.read_motor(theta_x, theta_y,motor_X,motor_Y);
/*	
	FIXME
	Earlier, *probably* camera was getting stuck in front of ball because
	x and y were in integer operations, and had not been typecasted
	to float
	So, if ball was on edge of the rectangle, the camera got stuck, as 
	integer operations reduced values of less than 1 to 0
	Maybe problem will still come, if so, try rounding up over here,
	or changing the 5 and 8 to larger values.
	If problem persists, read whole code again..
*/

	theta_x = theta_x + ((IMAGE_HEIGHT/2-(float)y)*15.0/(IMAGE_HEIGHT/2));//15
	theta_y = theta_y + (((float)x-(IMAGE_WIDTH/2))*20.0/(IMAGE_WIDTH/2));//30	


	if((theta_x <= 1)||(theta_x >= 89))	//Max vertical limits. Maybe not hardcode them?
		return -2;

	if(theta_y <=LEFT_LIMIT +1)
		return_value = -10;
	if(theta_y >=RIGHT_LIMIT-1)
		return_value = -20;
			

	if(theta_x < 0 )
	{
		theta_x = 0;
	}
	else if(theta_x > 90)
	{
		theta_x = 90;
	}
			
	if(theta_y < LEFT_LIMIT)
	{
		theta_y = LEFT_LIMIT;
	}
	else if(theta_y > RIGHT_LIMIT)
	{
		theta_y = RIGHT_LIMIT;
	}
	
	hm.write_motor(theta_x, theta_y);
	
	return return_value;
} 



//Refer findBall.mm in Dropbox/ for flowchart
//TODO: prev_ballx is compared with 320. Replace it by image-width/2 if possible, also same thing in rest of code
//TODO: there are constant 4 passes for finding ball. Replace by default variable possibly?
BallReturns CamControl::findBall(FeatureDetection &fd, HeadMotor &hm) // new findball function
{
	if(fd.ballFound()==true)  // function to return ball coordinates if found
	{


		prev_img_flag=1; // flag to check whether prev img has ball in it or not. to be used ltr.
		prev_ballx=fd.ballX();
		prev_bally=fd.ballY();
		float ballx_center=fd.ballX()-(IMAGE_WIDTH/2);
		float bally_center=fd.ballY()-(IMAGE_HEIGHT/2);

		if(inCentreRect(fd.ballX(), fd.ballY(), CENTRE_RECT_X1, CENTRE_RECT_Y1, CENTRE_RECT_X2, CENTRE_RECT_Y2)) // function which checks whether ball in centre or not
		{
			//ball is found and it is in centre
			pass_counter=0;
			prev_img_flag = 0;
			return BALLFOUND;
			//distance and angle stored in ball.r and ball.theta
		}


		else // if ball not in centre
		{
			switch(moveToPoint(fd.ballX(), fd.ballY(), hm))  /*fuction which controls motor movements and 
													Returns 0 on success.	-2: can't move along x, -4: unable to read, -10: turn left, -20: turn right*/
			{
				case 0:
				case -2:
				case -4:
					return BALLFINDING;
				case -10:
					return TURNLEFT;
				case -20:
					return TURNRIGHT;
			}

		}

	}


	else	// if ball not found
	{
		if(prev_img_flag==1)// if ball in prev img then w'll move motors accordingly...
		{
			prev_img_flag=0;
			if(prev_ballx>(IMAGE_WIDTH/2))//for turning right
			{
				state_of_motion = 0;
				
			}
			else//for turning left
			{
				state_of_motion = 1;
				
			}
			moveSearch(hm, 45, 60);

		}
		else// ball not in prev. img either....
		{
			if(!hm.ismoving_motor(18))//if motor is not moving 
			{
				moveSearch(hm, 45, 60);
				pass_counter++;
				if(pass_counter>1)
				{
					pass_counter=0;
					return TURNRIGHT;
				}
			}
//hm.read_motor(thetaX, thetaY);
		}
 		return BALLFINDING;
	}
}
