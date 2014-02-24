
#include "headmotor.h"////


//NOTE: goal pos and current pos set here must be same as that set in bootup_files()!!! they both also must be same
//and must be equal to the initialize value you want
HeadMotor::HeadMotor(bool param)	//param is there to see if need to initialize position
{
	global_id = 1;
	initPos = param;
	goal_pos[0] = (300.0 - 150)*(1023.0/300.0) - offsety;
	goal_pos[1] = (300.0 - 50)*(1023.0/300.0) - offsetx;
	// goal_pos[0] = 512;		//testing for loc tp
	// goal_pos[1] = 640;
	current_pos[0] = goal_pos[0];
	current_pos[1] = goal_pos[1];
	speed_motor(MOTOR_SPEED,700);
}



int HeadMotor::bootup_files()
{
    int ret;
    if (ftdi_init(&ftdic1_camera) < 0)
    {
        fprintf(stderr, "ftdi_init failed\n");
        return -1;
    }

    if ((ret = ftdi_usb_open_desc (&ftdic1_camera, 0x0403, 0x6001, NULL, serialusb2d_camera)) < 0)
    {
        fprintf(stderr, "unable to open ftdi device: %d (%s)\n", ret, ftdi_get_error_string(&ftdic1_camera));
        printf("unable to open USB2D_camera\n");
        sleep(1);
        return -1;
    }
    else
    {
    	printf("USB2D ftdi successfully open\n");
		if(initPos==true)
		{
			set_speed(18, 0);
			set_speed(17, 0);
			int id[] = {18,17};
			int gp[2];
			gp[0] = (300.0 - 150)*(1023.0/300.0) - offsety;
			gp[1] = (300.0 - 80)*(1023.0/300.0) - offsetx; 
			// sync_write_gp(id, gp, 2);
		}
	}

    ftdi_set_baudrate(&ftdic1_camera,1000000);
    return ret;
}



int HeadMotor::checksum(byte *pack)
{
	byte checksum = 0;
	int i;
	for (i=2; i<pack[3]+3; i++)
	{
		checksum +=pack[i];
	}
	checksum = 255 - checksum;
	pack[pack[3]+3]=checksum;
	return (int)checksum;
}



int HeadMotor::read_pos(int id)
{
  byte pack[]={0xff,0xff,id,0x04,0x02,0x24,0x02,0x00};
  byte c1 = 0, c2=0, lpos = 0, hpos = 0;
  int j=0, pos=0;
  long i = 0;
  checksum(pack);
  ftdi_usb_purge_buffers(&ftdic1_camera);
  WRITE_FTDI;
  usleep(1000);
  while((c1!=255)||(c2!=255))
  {
  	i++;
 	c2=c1;
	ftdi_read_data(&ftdic1_camera,&c1,1);
 	if(i==NO_OF_TRIES)
 	break;
  }

  if(c1==255&&c2==255)
  {
  	while(ftdi_read_data(&ftdic1_camera,&c1,1)==1)
  	{
  		pack[j++] = c1;
  		if(j>=8)
  		{
  			break;
  		}

  	}

  	if (pack[2]==0)
  	{
  		lpos = pack[3];
  		hpos = pack[4];
  		pos = lpos + hpos*256;
  	//	printf("Position is %d ",pos);
  		return pos;
  	}
  	else
  	{
  		printf("Error (pos): %d",pack[2]);
  		return -pack[2];
  	}

  }
  else
  {
  	printf("Unable to read (pos)  %d  %d\n",c1,c2);
  	return -1;
  }

}



//id not being used here, this just checks if both motors are still
int HeadMotor::ismoving_motor(int id)
{
	if((current_pos[0] == goal_pos[0])&&(current_pos[1] == goal_pos[1]))
	{
		return 0;
	}
	else
	{
		return 1;
	}
}



void HeadMotor::set_gp(int id,int gp)
{
	byte gpl, gph;
	gpl = gp%256;
	gph = gp/256;

  	byte pack[] = {0xff, 0xff, id,0x05 ,0x03, 0x1e, gpl,gph,0x00};
  	checksum(pack);

	WRITE_FTDI;
}



void HeadMotor::set_gp(int gp)
{
	set_gp(global_id,gp);
}



void HeadMotor::set_speed(int id, int sp)
{
	byte sl, sh;
	sl=sp%256;
	sh=sp/256;
	byte pack[] = {0xff, 0xff, id, 0x05 ,0x03, 0x20,sl,sh,0x00};
	checksum(pack);
	WRITE_FTDI;
}


//Motor IDS hardcoded
//Shouldn't be a problem as they are a standard

void HeadMotor::speed_motor(int speed1, int speed2)
{
	// STEP_SIZE_18 = 1;
	// STEP_SIZE_17 = 5;
	if(speed1==0)
	{
		STEP_SIZE_18 = MAX_STEP;
	}
	else
	{
		STEP_SIZE_18 = (speed1*MAX_STEP)/1024;
	}

	if(speed2==0)
	{
		STEP_SIZE_17 = MAX_STEP;
	}
	else
	{
		STEP_SIZE_17 = (speed2*MAX_STEP)/1024;
	}

}



void HeadMotor::sync_write_gp(int id[], int gp[], int n)
{
	int i, c=7;
	byte pack[3*n+8];
	pack[0] = pack[1] = 0xff;
	pack[2]=0xfe;
	pack[3]=3*n+4;
	pack[4]=0x83;
	pack[5]=0x1e;
	pack[6]=2;

	for(i=0;i<n;i++)
	{
		pack[c++]=id[i];
		pack[c++]=gp[i]%256;
		pack[c++]=gp[i]/256;

	}

	checksum(pack);
//	for (i=0; i<(3*n + 8);i++)
//	{
//		printf("%d ", pack[i]);
//	}
	WRITE_FTDI;
}



// void HeadMotor::sync_write_gp()
// {
// 	int id[]={1,2};
// 	int n = 2;
// 	int gp[2];
// 	printf("Enter gp1 and gp2 : ");
// 	scanf("%d %d",&gp[0],&gp[1]);
// 	sync_write_gp(id, gp, n);
// }



void HeadMotor::write_motor(float thetax, float thetay)
{
	goal_pos[0] = (300.0 - thetay)*(1023.0/300.0) - offsety;
	goal_pos[1] = (300.0 - thetax)*(1023.0/300.0) - offsetx;

//	sync_write_gp(id, gp, 2);
//	set_gp(18, (300.0 - thetay)*(1023.0/300.0) - offsety);
//	set_gp(17, (300.0 - thetax)*(1023.0/300.0) - offsetx);
}



int HeadMotor::read_motor(float &thetax, float &thetay, int &motorX_var, int &motorY_var)
{
	motorX_var = current_pos[1];
	motorY_var = current_pos[0];
	thetay = 300.0 - ((current_pos[0]+offsety)*300.0/1023.0);
	thetax = 300.0 - ((current_pos[1]+offsetx)*300.0/1023.0);
	return 1;
}



int HeadMotor::stop_motor()
{
	goal_pos[0] = current_pos[0];
	goal_pos[1] = current_pos[1];
	return 1;
}



void HeadMotor::scurve(int id[],int final[], float time)
{
	#define vmax 3
	#define Smoo(x) 3*x*x - 2*x*x*x;//vmax*x + x*x*x -vmax*x*x;//(vmax*((x)+((x)*(x)*(x)/vmax)-(x)*(x)))
	float x,y;
	float t = 0;
	float init[2];
	int gp[2];
	init[0] = read_pos(id[0]);
	init[1] = read_pos(id[1]);
	t = time;
	if (t<0)
		t = -t;


	x = 1/(FPS*t);
	for (; x<=1; x = x+ 1/(FPS*t))
	{
		y= Smoo(x);
		gp[0]= y*final[0] + (1-y)*init[0];
		gp[1]= y*final[1] + (1-y)*init[1];
		sync_write_gp(id,gp,2);
		usleep(1000000.0/FPS);
	}

	#undef Smoo
	#undef vmax
}



int HeadMotor::swrite(float thetax, float thetay, float ttime)
{
	int id[2] = {17,18};
	int final[2] = {(300.0 - thetax)*(1023.0/300.0) - offsetx,(300.0 - thetay)*(1023.0/300.0) - offsety};
	scurve(id, final, ttime);
	return 1;
}



void HeadMotor::update()
{
	int id[2] = {18,17};

	thetaY_var = deg2rad(300.0 - ((current_pos[0]+offsety)*300.0/1023.0));
	thetaX_var = deg2rad(300.0 - ((current_pos[1]+offsetx)*300.0/1023.0));

	if(!ismoving_motor())
		return;

	if(goal_pos[0]!=current_pos[0])
	{
		if(current_pos[0] < goal_pos[0])
		{
			current_pos[0] += STEP_SIZE_18;
			if(current_pos[0] > goal_pos[0])
			{
				current_pos[0] = goal_pos[0];
			}
		}
		else
		{
			current_pos[0] -= STEP_SIZE_18;
			if(current_pos[0] < goal_pos[0])
			{
				current_pos[0] = goal_pos[0];
			}
		}
	}

	if(goal_pos[1]!=current_pos[1])
	{
		if(current_pos[1] < goal_pos[1])
		{
			current_pos[1] += STEP_SIZE_17;
			if(current_pos[1] > goal_pos[1])
			{
				current_pos[1] = goal_pos[1];
			}
		}
		else
		{
			current_pos[1] -= STEP_SIZE_17;
			if(current_pos[1] < goal_pos[1])
			{
				current_pos[1] = goal_pos[1];
			}
		}
	}
	
	// printf("Current: %d Goal: %d\n", current_pos[0], goal_pos[0]);
	// printf("Current: %d Goal: %d\n", current_pos[1], goal_pos[1]);
	sync_write_gp(id, current_pos, 2);
	return;
}

/*
int swrite(int thetax, int thetay, int ttime=2)
{
	#define Smoo(x) (3*(x)*(x)-2*(x)*(x)*(x))
	ttime=30*ttime;
	int dtx=thetax-deg2rad(thetaX);
	int dty=thetay-deg2rad(thetaY);
	thetax=deg2rad(thetaX);
	thetay=deg2rad(thetaY);
	for(int ctime=0; ctime < ttime; ctime++)
	{
		write_motor((int)(Smoo((float)ctime/ttime)*dtx)+thetax,(int)(Smoo((float)ctime/ttime)*dty)+thetay);
		usleep(33333);
	}
	#undef Smoo
	return 1;
}
*/



int HeadMotor::go2state(int state)
{
	state=state%6;
	int c[2];
	switch(state)
	{
		case 2:
		c[0] = 60;
		c[1] = 90;
		break;
		case 1:

		c[0] = 60;
		c[1] = 150;
		break;
		case 0:

		c[0] = 60;
		c[1] = 210;
		break;
		case 3:

		c[0] = 45;
		c[1] = 90;
		break;
		case 4:

		c[0] = 45;
		c[1] = 150;
		break;
		case 5:

		c[0] = 45;
		c[1] = 210;
		break;

		default: return -1;
	}
	write_motor(c[0], c[1]);
//	write_motor(c[0], c[1]);

	return 0;
}
