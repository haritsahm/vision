#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <ftdi.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include <iomanip>

#include "cmtdef.h"
#include "xsens_time.h"
#include "xsens_list.h"
#include "cmtscan.h"
#include "cmt3.h"
#include "imu.h"

using namespace std;

	struct yaw
	{
		int time;
		int motor;
		double yaw;
	}yaw;

typedef unsigned char byte;
#define serialusb2d "A900fDpz"//"A800d2dg"//"A7003N1d"// "AD025JOH"//"A4007rXR"//"A7003N1d"//"A4007sgG"//"A900fDpz"//"A7003N1d"////"A900fDhp"//"A900fDpA"//"A900fDhp"//"A900fDhp"

struct ftdi_context ftdic1;

int bootup_files()
{
	int ret;
    if (ftdi_init(&ftdic1) < 0)
    {
        fprintf(stderr, "ftdi_init failed\n");
        return 0;
    }
    
    if ((ret = ftdi_usb_open_desc (&ftdic1, 0x0403, 0x6001, NULL, serialusb2d)) < 0)
    {
        fprintf(stderr, "unable to open ftdi device: %d (%s)\n", ret, ftdi_get_error_string(&ftdic1));
        printf("unable to open USB2D");
        return 0;
    }
    else
    printf("USB2D ftdi successfully open\n");

    ftdi_set_baudrate(&ftdic1,1000000);
}

void set_goal_pos(int id,int n)
{
    int lb,hb;
    if((n<0)||(n>1023))
    {
        printf("INVALID VALUE\n");
        return;
    }
    lb=n%256;
    hb=n/256;
    byte pack[]={0xff,0xff,id,0x05,0x03,0x1E,lb,hb,0x00};
    pack[8]=id+5+3+30+lb+hb;
    pack[8]=0xff-pack[8];
    if(ftdi_write_data(&ftdic1,pack,pack[3]+4)<0)
    printf("GENORAI\n");
}

int read_position(int id)
{
    byte reader[1000]={0,0};
    byte pack[]={0xff,0xff,id,0x05,0x02,0x24,0x02,0x00};
    pack[7]=id+5+2+36+2;
    pack[7]=0xff-pack[7];
    if(ftdi_write_data(&ftdic1,pack,pack[3]+4)<0)
    printf("GENORAI\n");
    int x=ftdi_read_data(&ftdic1,reader,1000);
    if(x<0)
    printf("ERROR IN READING DATA\n");
    else if(x==0)
    printf("NO DATA READ\n");
    else
    {
        int i;
        for(i=0;i<x-1;i++)
        {
            if((reader[i]==0xff)&&(reader[i+1]==0xff))
            {
                int ch_sum=0;
                for(int j=i+2;j<=i+7;j++)
                ch_sum+=reader[j];
                if(ch_sum==0xff)
                break;
            }
        }
        if(i==x-1)
        {
            printf("WRONG DATA\n");
            return -1;
        }
        int ans,lb,hb;
        lb=reader[i+5];
        hb=reader[i+6];
        ans=(hb*256)+lb;
        printf("CURRENT POSITION = %d\n",ans);
        return ans;
    }
    return -1;
}

void set_moving_speed(int id,int n)
{
    int lb,hb;

    if((n<0)||(n>1023))
    {
        printf("INVALID VALUE\n");
        return;
    }
    lb=n%256;
    hb=n/256;
    byte pack[]={0xff,0xff,id,0x05,0x03,0x20,lb,hb,0x00};
    pack[8]=id+5+3+32+lb+hb;
    pack[8]=0xff-pack[8];

    if(ftdi_write_data(&ftdic1,pack,pack[3]+4)<0)
    printf("GENORAI\n");
    else 
    printf("DATA SENT\n");  
}

int main()
{
	bootup_files();
	Imu imu;
	imu.init();

	// double yaw_angle;

	ofstream binary;
	ofstream text;

	binary.open("yaw.dat",ios::binary|ios::app);
	text.open("yaw.txt");

	int id=1;
	set_moving_speed(id,100);

	while(1)
	{
		time_t t1 = time(0);
		float time_1 = float(t1);
		float time_2;

		while((time_2-time_1)<900)
		{
			usleep(1000000);
			imu.update();
			double yaw_angle = imu.yaw;
			
			time_t now = time(0) - t1;
			float time_now = float(now);

			yaw.yaw=yaw_angle;
			yaw.time=time_now;
			yaw.motor=read_position(id);

			cout<<"PRINTING"<<setprecision(5)<<yaw.time<<"\t\t\t"<<yaw.motor<<"\t\t\t"<<setprecision(5)<<yaw.yaw<<"\n";
			cout<<"\nWRITING...";
			binary.write((char*)&yaw,sizeof(yaw));
			text<<setprecision(5)<<yaw.time<<"\t\t\t"<<yaw.motor<<"\t\t\t"<<setprecision(5)<<yaw.yaw<<"\n";
			
			time_t t2 = time(0);
			time_2 = float(t2);
		}
		cout<<"\n15 MINUTES OVER";
		if(read_position(id)>1024-103)
			{
				set_goal_pos(id,0);
				cout<<"\nRESETTING...";
				usleep(1000000);
			}

		set_goal_pos(id,read_position(id)+102);
		usleep(1000000);
	}

	binary.close();
	text.close();
	return 0;
}