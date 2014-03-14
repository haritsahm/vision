#include <inttypes.h>
#include <ftdi.h>
#include <unistd.h>	//for usleep() function
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <sstream>  //include this to use string streams
#include <string>
#include <stdlib.h>

#include <cvblob.h>
#include <opencv/cv.h>
#include <opencv/highgui.h> 

#include <ueye.h>

using namespace std;
using namespace cvb;
using namespace cv;

// #define HEIGHT 480
// #define WIDTH 752
#define PI 3.14159265359
char* imgPointer = NULL;
int imgMemPointer;

typedef unsigned char byte;
#define serialusb2d "A4007rXO"//"A800d2dg"//"A7003N1d"// "AD025JOH"//"A4007rXR"//"A7003N1d"//"A4007sgG"//"A900fDpz"//"A7003N1d"////"A900fDhp"//"A900fDpA"//"A900fDhp"//"A900fDhp"

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

int initializeCam(HIDS hCam)
{   
    char* errMsg = (char*)malloc(sizeof(char)*200);
    int err = 0;

    int nRet = is_InitCamera (&hCam, NULL);
    if (nRet != IS_SUCCESS)
    {
        is_GetError (hCam, &err, &errMsg);
        printf("Camera Init Error %d: %s\n",err,errMsg);
        return EXIT_FAILURE;
    }

    nRet = is_SetColorMode(hCam, IS_CM_BGR8_PACKED);
    if (nRet != IS_SUCCESS) 
    {
            is_GetError (hCam, &err, &errMsg);
            printf("Color Mode Error %d: %s\n",err,errMsg);
            return EXIT_FAILURE;  
    }

    nRet = is_SetHardwareGain(hCam, 100, 4, 0, 13);
    if (nRet != IS_SUCCESS) 
    {
            is_GetError (hCam, &err, &errMsg);
            printf("Hardware Gain Error %d: %s\n",err,errMsg);
            return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int setImgMem(HIDS hCam)
{
    char* errMsg = (char*)malloc(sizeof(char)*200);
    int err = 0;

    int nRet = is_AllocImageMem(hCam, 752, 480, 24, &imgPointer, &imgMemPointer);
    if(nRet != IS_SUCCESS)
    {
        is_GetError (hCam, &err, &errMsg);
        printf("MemAlloc Unsuccessful %d: %s\n",err,errMsg);
        return EXIT_FAILURE;
    }

    nRet = is_SetImageMem (hCam, imgPointer, imgMemPointer);
    if(nRet != IS_SUCCESS)
    {
        is_GetError (hCam, &err, &errMsg);
        printf("Could not set/activate image memory %d: %s\n",err, errMsg);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void getFrame(HIDS hCam,IplImage* frame)
{
    char* errMsg = (char*)malloc(sizeof(char)*200);
    int err = 0;


    int nRet = is_FreezeVideo (hCam, IS_WAIT) ;
    if(nRet != IS_SUCCESS)
    {
        is_GetError (hCam, &err, &errMsg);
        printf("Could not grab image %d: %s\n",err,errMsg);
        //return EXIT_FAILURE;
    }
        
    //fill in the OpenCV imaga data 
    //IplImage* img_rgb = cvCreateImage(cvSize(752,480),8,3);
    memcpy(frame->imageData, imgPointer, 752*480 * 3);
    //img_rgb->imageData = imgPointer;
    //return img_rgb;
}

int exitCam(HIDS hCam)
{
    int nRet = is_ExitCamera (hCam);
    if(nRet != IS_SUCCESS)
    {
        printf("Could not exit camera \n");
        return EXIT_FAILURE;
    }
}

void set_id(int n)
{
    byte pack[]={0xff,0xff,0xfe,0x04,0x03,0x03,n,0x00};
    pack[7]=254+4+3+3+n;
    pack[7]=0xff-pack[7];

    if(ftdi_write_data(&ftdic1,pack,pack[3]+4)<0)
    printf("GENORAI\n");
    else 
    printf("DATA SENT\n");  
}

/*reads position from motor*/

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

/*sets goal position*/

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

/*sets motor movement speed so that there are no jerks while camera is mounted on it*/

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

    //INITIALIZE CAMERA
    HIDS hCam = 1;
    initializeCam(hCam);
    setImgMem(hCam);

    int id=17;
    int f=read_position(id);					//some useless error while reading position from motor

    IplImage* bgr_frame=cvCreateImage(cvSize(752, 480), 8, 3);//fisheye image
    IplImage* bgr_half=cvCreateImage(cvSize(320, 240), 8, 3);//fisheye image

    set_moving_speed(id,100);    

    for(int i=609;i<810;++i)			//for loop moves motor for each and every motor location and capture pictures
    {									//that will be later used calculate the constants for the fisheye lens for 
        // set_moving_speed(id,100);
        set_goal_pos(id,512);        	//for each and every angle the head motor will be allowed to move
        usleep(3000000);
        // set_moving_speed(id,100);        
        set_goal_pos(id,i);        
        usleep(3000000);
        int g=read_position(id);
              
		char imgcounter[50]="";
		string motorstring = static_cast<ostringstream*>( &(ostringstream() << read_position(id)) )->str();
		const char * d = motorstring.c_str();
		strcat(imgcounter,d);            
		strcat(imgcounter,".bmp");
        
        getFrame(hCam, bgr_frame);
        cvSetImageROI(bgr_frame, cvRect(56, 0,640,480));
        cvResize(bgr_frame, bgr_half, CV_INTER_NN);
        if(cvSaveImage(imgcounter,bgr_half))
            printf("IMAGE %s SAVED\n", imgcounter);
        
        usleep(1000000);
    }

    return 0;
}