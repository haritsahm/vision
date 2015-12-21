#include <iostream>
#include "stdio.h"
#include "math.h"
#include "ftdi.h"

#include <ueye.h>

#ifdef WIN32
#include <cv.h>
#include <highgui.h>
#else
#include <opencv/cv.h>
#include <opencv/highgui.h>
#endif

//#include <cvblob.h>
#include <termios.h>
#include <fcntl.h>   /* File control definitions */
//#define FT1 "A7003NDp"
//#define FT1 "A600cBa7"
#define FT1 "A600cURy"
#define FT2 "A800ctep"
#define get_lb(x)	x&0xff
#define	get_hb(x)	x>>8
#define deg2rad(x) x*(3.14159/180.0)
#define rad2deg(x) x*(180.0/3.14159)

typedef unsigned char byte;
typedef struct
{
	int radial_distance;
	int ANGLE;
	int GOAL_ANGLE;
}obj_pos;
obj_pos object_position;

//using namespace cvb;

float theta_ydeg=30.0,theta_xdeg=150.0,radius=10.0,s_height=84.7,neck_len=6.5;
float constanty=15.0,rangey=5.0,constantx=20.0,rangex=16.0,rminx=70.0,rmaxx=230.0,rchangex=7.0,rminy=5.0,rmaxy=88.0,rchangey=10.0,box_x=20.0,box_y=15.0,prevchangey=0;
float thetay=0.0,height =s_height+(neck_len*sin(theta_ydeg))-radius,thetax=0.0,theta=deg2rad(theta_ydeg),cen_dist=(height)*tan(theta),len=cen_dist/sin(theta);
int offsety=210,offsetx=-77,stop_search=0,max_noat=5,disable_noat=0,mltplr_x=1,search_dir_var=20,check_goal_after_nof=10;//FOR DISABLING NOAT SET disable_noat VALUE TO (max_noat-1) ELSE SET IT TO 0
//int fd;
////////////GOAL VARS//////////////////////
float goal_box_x=50.0,goal_box_y=80.0,no_frame_check=10.0,GOAL_ANGLE_X=150.0,GOAL_ANGLE_Y=60.0,R_miny_goal=40.0;
///////////////////////////////////////////

/////////////LOCATION VARS/////////////////
float GOAL_POS=1000.0,BALL_POS=200.0,CENTER_POS=theta_xdeg,MOVE_THETA=1000.0;
///////////////////////////////////////////

struct ftdi_context ft1,ft2;

byte tx_packet[128]={0xff,0xff,0xfe,0x0a,0x83,0x1e,0x02,17,0,0,18};

typedef struct
{
	unsigned int BlobId;
	unsigned int Area;
	int Minx;
	int Miny;
	int Maxx;
	int Maxy;
	CvPoint2D32f Centroid;
}AcyutBlob;

int init()
{
	int ret=0;
	if(ftdi_init(&ft1)<0)
	{
		fprintf(stderr,"ftdi_init failed\n");
		ret+=10;
	}
	if((ret=ftdi_usb_open_desc(&ft1,0x0403,0x6001,NULL,FT1))<0)
	{
		fprintf(stderr,"unable to open ftdi device: %d (%s)\n",ret,ftdi_get_error_string(&ft1));
		ret+=100;
	}
	else	ftdi_set_baudrate(&ft1,1000000);
	
	/*if(ftdi_init(&ft2))
	{
		fprintf(stderr,"ftdi_init failed\n");
		ret+=5;
	}
	if((ret=ftdi_usb_open_desc(&ft2,0x0403,0x6001,NULL,FT2))<0)
	{
		fprintf(stderr,"unable to open ftdi device: %d (%s)\n",ret,ftdi_get_error_string(&ft2));
		ret+=50;
	}
	else	ftdi_set_baudrate(&ft2,115200);
	*/
	/*struct termios options;
	struct timeval start, end;
	long mtime, seconds, useconds;
	if((fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY))==-1)
	{
		printf("Serial Port unavailable\n");
		ret=+1000;
	}
	else
	{
		tcgetattr(fd, &options);
		cfsetispeed(&options, B115200);
		cfsetospeed(&options, B115200);
		options.c_cflag |= (CLOCAL | CREAD);
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		options.c_cflag &= ~CSIZE;
		options.c_cflag |= CS8;
		options.c_oflag |= OPOST;
		options.c_oflag |= ONOCR;
		options.c_oflag |= FF0;
		options.c_oflag &= ~FF1;
		options.c_oflag &= ~ONLCR;
		options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
		options.c_cflag &= ~CRTSCTS;
		tcsetattr(fd, TCSANOW, &options);
		fcntl(fd, F_SETFL, FNDELAY);
		
		char buf[20];
		strcpy(buf, "This is a test\n");
		int nbytes = strlen(buf);
		int bytes_written = write(fd, buf, nbytes);
	}*/
	return ret;
}

char* imgPointer = NULL;
int imgMemPointer;


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

int set_motor_value()
{
	int ret=0;
	int lb17=8,lb18=11,chksm=tx_packet[3]+3,m_val,m_valx;
	m_val=((((300.0-theta_ydeg))/300.0)*1023.0);
	m_valx=((((300.0-theta_xdeg))/300.0)*1023.0);
	//printf("mvalue=%d\n",m_valx);
	m_val=m_val-offsety;
	//printf("m_val%d\n",m_val);
	m_valx=m_valx-offsetx;
	//printf("m_val=%d, theta+offsety=%f\n",m_val,theta_ydeg+offsety*1023/300);
	tx_packet[lb18]=get_lb(m_valx);
	tx_packet[lb18+1]=get_hb(m_valx);
	tx_packet[lb17]=get_lb(m_val);
	tx_packet[lb17+1]=get_hb(m_val);
	int iterator1=0;
	tx_packet[chksm]=0;
	for(iterator1=2;iterator1<tx_packet[3]+3;iterator1++)
	{
		tx_packet[chksm]+=tx_packet[iterator1];
		//printf("%d\n",tx_packet[iterator1]);
		//printf("chksm=%d\n",tx_packet[chksm]);
	}
	//printf("!checksum%d\n",tx_packet[iterator1]);
	tx_packet[chksm]=255-tx_packet[chksm];
	//printf("chksm%d\n",tx_packet[chksm]);
	ftdi_write_data(&ft1,tx_packet,tx_packet[3]+4);
	//usleep(10000);
	return 0;
}

int read_motor_value()
{
	int ret=0,id=0,chksm=7,iterator1=0,noat=0,flag=0,m_val17=0,m_val18=0;
	byte data=0,snd_read[40]={0xff,0xff,0x00,0x04,0x02,0x24,0x02,0x00},rcv_data[5],checksum=0;
	for(id=17;id<=18;id++)
	{
		snd_read[2]=id;
		//printf("id%d\n",id);
		snd_read[chksm]=0;
		for(iterator1=2;iterator1<snd_read[3]+3;iterator1++)
			snd_read[chksm]+=snd_read[iterator1];
		snd_read[chksm]=255-snd_read[chksm];
		/*for(iterator1=0;iterator1<snd_read[3]+4;iterator1++)
			printf("%d\n",snd_read[iterator1]);
		*/ftdi_write_data(&ft1,snd_read,snd_read[3]+4);
		//printf("attempting to read\n");
		//usleep(10000);
		flag=0;
		for(noat=disable_noat;noat<max_noat;noat++)
		{
			if(ftdi_read_data(&ft1,&data,1))
			{
				noat=0;
				//printf("packetff = %dflag=%d\n",data,flag);
				//usleep(10000);
				if(data==255)
				{
					flag=flag+1;
					if(flag==2)
						break;
				}else
					flag=0;
			}
		}
		if(flag==2)
		{
			iterator1=0;
			checksum=0;
			for(noat=disable_noat;noat<max_noat;noat++)
			{
				if(ftdi_read_data(&ft1,&data,1))
				{
					noat=0;
					//printf("packet = %d\n",data);
					//usleep(10000);
					rcv_data[iterator1++]=data;
					//printf("no %d data =%d\n",iterator1,data);
					if(iterator1>=6)
						break;
					checksum+=data;
				}
			}
			if(noat<max_noat)
			{
				checksum=255-checksum;
				//printf("checksum:%d\n",checksum);
				if(checksum==rcv_data[5])
				{
					switch(rcv_data[0])
					{
						case 17:m_val17=rcv_data[3]+rcv_data[4]*256;
								break;
						case 18:m_val18=rcv_data[3]+rcv_data[4]*256;
								break;
						default:return 3;
								break;
					}
				}
				else return 2;
			}else return 1;
		}
		else
			return 1;
	}
	//printf("theta =%f\n",theta_ydeg);
	//printf("m_val17%d\n",m_val17);
	theta_ydeg=300.0-((m_val17+offsety)*300.0/1023.0);
	//printf("theta_ydeg=%f\n",theta_ydeg);
	//printf("theta =%f\n",theta_ydeg);
	return 0;
}

void update_height()
{
	height=s_height+(neck_len*sin(deg2rad(theta_ydeg)))-radius;
}



int findblob(AcyutBlob *genblob,IplImage *src,IplImage *dstn,float min_area)
{
	printf("in findblob fncn\n");
	CvSize srcSize=cvGetSize(src);
	int step      = src->widthStep/sizeof(uchar);
	uchar* datasrc    = (uchar *)src->imageData;
	uchar* datadstn    = (uchar *)dstn->imageData;
	unsigned int NUMBER,curr_no,prev_no;
	int height=480,width=640,i,j,i2,j2;
	//int arrayii[height][width],arrayi[height][width];
	NUMBER=0;
	for(i=1;i<srcSize.height;i++)
	{
		for(j=1;j<(srcSize.width-1);j++)
		{
			if(datasrc[i*step+j]!=0)
			{
				//printf("p\n");
				if(datadstn[i*step+(j-1)]!=0)//(arrayii[i-1][j-1]!=0||arrayii[i-1][j+1]!=0||arrayii[i-1][j]!=0)
				{
					//printf("p1\n");
					datadstn[i*step+j]=datadstn[i*step+(j-1)];
					genblob[datadstn[i*step+j]].Area++;
					genblob[datadstn[i*step+j]].Centroid.y=(i+genblob[datadstn[i*step+j]].Centroid.y*(genblob[datadstn[i*step+j]].Area-1.0));
					genblob[datadstn[i*step+j]].Centroid.y=genblob[datadstn[i*step+j]].Centroid.y/(float)genblob[datadstn[i*step+j]].Area;
					genblob[datadstn[i*step+j]].Centroid.x=(j+genblob[datadstn[i*step+j]].Centroid.x*(genblob[datadstn[i*step+j]].Area-1.0));
					genblob[datadstn[i*step+j]].Centroid.x=genblob[datadstn[i*step+j]].Centroid.x/(float)genblob[datadstn[i*step+j]].Area;
					//printf("p1\n");
				}//printf("p1\n");
				if(datadstn[(i-1)*step+(j-1)]!=0)
				{
					//printf("p2\n");
					if(datadstn[i*step+j]!=0&&datadstn[i*step+j]!=datadstn[(i-1)*step+(j-1)])
					{
						if(datadstn[i*step+j]<datadstn[(i-1)*step+(j-1)])
						{
							//printf("X\n");
							prev_no=datadstn[(i-1)*step+(j-1)];
							curr_no=datadstn[i*step+j];
							for(i2=1;i2<=i;i2++)
								for(j2=1;j2<(srcSize.width-1);j2++)
									if(datadstn[i2*step+j2]==prev_no)
									{
										datadstn[i2*step+j2]=curr_no;
									}
							genblob[curr_no].Centroid.y=(genblob[curr_no].Centroid.y*(float)(genblob[curr_no].Area)+genblob[prev_no].Centroid.y*(float)(genblob[prev_no].Area));
							genblob[curr_no].Centroid.y=genblob[curr_no].Centroid.y/(genblob[prev_no].Area+genblob[curr_no].Area);
							genblob[curr_no].Centroid.x=(genblob[curr_no].Centroid.x*(genblob[curr_no].Area)+genblob[prev_no].Centroid.x*(genblob[prev_no].Area));
							genblob[curr_no].Centroid.x=genblob[curr_no].Centroid.x/(genblob[prev_no].Area+genblob[curr_no].Area);
							genblob[curr_no].Area=genblob[prev_no].Area+genblob[curr_no].Area;
							genblob[prev_no].Area=0;
							prev_no=0;
							curr_no=0;
							//printf("X\n");
						}else
						{	
							//printf("X1\n");
							prev_no=datadstn[i*step+j];
							curr_no=datadstn[(i-1)*step+(j-1)];
							for(i2=1;i2<=i;i2++)
								for(j2=1;j2<(srcSize.width-1);j2++)
									if(datadstn[i2*step+j2]==prev_no)
									{
										datadstn[i2*step+j2]=curr_no;
									}
							//printf("X1\n");
							genblob[curr_no].Centroid.y=(genblob[curr_no].Centroid.y*(genblob[curr_no].Area)+genblob[prev_no].Centroid.y*(genblob[prev_no].Area));
							genblob[curr_no].Centroid.y=genblob[curr_no].Centroid.y/(genblob[prev_no].Area+genblob[curr_no].Area);
							genblob[curr_no].Centroid.x=(genblob[curr_no].Centroid.x*(genblob[curr_no].Area)+genblob[prev_no].Centroid.x*(genblob[prev_no].Area));
							genblob[curr_no].Centroid.x=genblob[curr_no].Centroid.x/(genblob[prev_no].Area+genblob[curr_no].Area);
							genblob[curr_no].Area=genblob[prev_no].Area+genblob[curr_no].Area;
							genblob[prev_no].Area=0;
							prev_no=0;
							curr_no=0;
						}
					}else
					{
						datadstn[i*step+j]=datadstn[(i-1)*step+(j-1)];
						genblob[datadstn[i*step+j]].Area++;
						genblob[datadstn[i*step+j]].Centroid.y=(i+genblob[datadstn[i*step+j]].Centroid.y*(genblob[datadstn[i*step+j]].Area-1));
						genblob[datadstn[i*step+j]].Centroid.y=genblob[datadstn[i*step+j]].Centroid.y/genblob[datadstn[i*step+j]].Area;
						genblob[datadstn[i*step+j]].Centroid.x=(j+genblob[datadstn[i*step+j]].Centroid.x*(genblob[datadstn[i*step+j]].Area-1));
						genblob[datadstn[i*step+j]].Centroid.x=genblob[datadstn[i*step+j]].Centroid.x/genblob[datadstn[i*step+j]].Area;
					}
					//printf("p2\n");
				}//printf("p1\n");
				if(datadstn[(i-1)*step+j]!=0)
				{
					//printf("p3\n");
					if(datadstn[i*step+j]!=0&&datadstn[i*step+j]!=datadstn[(i-1)*step+j])
					{
						if(datadstn[i*step+j]<datadstn[(i-1)*step+j])
						{
							//printf("Y\n");
							prev_no=datadstn[(i-1)*step+j];
							curr_no=datadstn[i*step+j];
							for(i2=1;i2<=i;i2++)
								for(j2=1;j2<(srcSize.width-1);j2++)
									if(datadstn[i2*step+j2]==prev_no)
									{
										datadstn[i2*step+j2]=curr_no;
									}
							genblob[curr_no].Centroid.y=(genblob[curr_no].Centroid.y*(genblob[curr_no].Area)+genblob[prev_no].Centroid.y*(genblob[prev_no].Area));
							genblob[curr_no].Centroid.y=genblob[curr_no].Centroid.y/(genblob[prev_no].Area+genblob[curr_no].Area);
							genblob[curr_no].Centroid.x=(genblob[curr_no].Centroid.x*(genblob[curr_no].Area)+genblob[prev_no].Centroid.x*(genblob[prev_no].Area));
							genblob[curr_no].Centroid.x=genblob[curr_no].Centroid.x/(genblob[prev_no].Area+genblob[curr_no].Area);
							genblob[curr_no].Area=genblob[prev_no].Area+genblob[curr_no].Area;
							genblob[prev_no].Area=0;
							prev_no=0;
							curr_no=0;
							//printf("Y\n");
						}else
						{
							//printf("Y1\n");
							prev_no=datadstn[i*step+j];
							curr_no=datadstn[(i-1)*step+j];
							for(i2=1;i2<=i;i2++)
								for(j2=1;j2<(srcSize.width-1);j2++)
									if(datadstn[i2*step+j2]==prev_no)
									{
										datadstn[i2*step+j2]=curr_no;
									}
							genblob[curr_no].Centroid.y=(genblob[curr_no].Centroid.y*(genblob[curr_no].Area)+genblob[prev_no].Centroid.y*(genblob[prev_no].Area));
							genblob[curr_no].Centroid.y=genblob[curr_no].Centroid.y/(genblob[prev_no].Area+genblob[curr_no].Area);
							genblob[curr_no].Centroid.x=(genblob[curr_no].Centroid.x*(genblob[curr_no].Area)+genblob[prev_no].Centroid.x*(genblob[prev_no].Area));
							genblob[curr_no].Centroid.x=genblob[curr_no].Centroid.x/(genblob[prev_no].Area+genblob[curr_no].Area);
							genblob[curr_no].Area=genblob[prev_no].Area+genblob[curr_no].Area;
							genblob[prev_no].Area=0;
							prev_no=0;
							curr_no=0;
							//printf("Y1\n");
						}
					}else
					{
						datadstn[i*step+j]=datadstn[(i-1)*step+j];
						genblob[datadstn[i*step+j]].Area++;
						genblob[datadstn[i*step+j]].Centroid.y=(i+genblob[datadstn[i*step+j]].Centroid.y*(genblob[datadstn[i*step+j]].Area-1));
						genblob[datadstn[i*step+j]].Centroid.y=genblob[datadstn[i*step+j]].Centroid.y/genblob[datadstn[i*step+j]].Area;
						genblob[datadstn[i*step+j]].Centroid.x=(j+genblob[datadstn[i*step+j]].Centroid.x*(genblob[datadstn[i*step+j]].Area-1));
						genblob[datadstn[i*step+j]].Centroid.x=genblob[datadstn[i*step+j]].Centroid.x/genblob[datadstn[i*step+j]].Area;
					}
					//printf("p3\n");
				}//printf("p1\n");
				if(datadstn[(i-1)*step+(j+1)]!=0)
				{
					//printf("p4\n");
					if(datadstn[i*step+j]!=0&&datadstn[i*step+j]!=datadstn[(i-1)*step+(j+1)])
					{
						if(datadstn[i*step+j]<datadstn[(i-1)*step+(j+1)])
						{
							//printf("Z\n");
							prev_no=datadstn[(i-1)*step+(j+1)];
							curr_no=datadstn[i*step+j];
							for(i2=1;i2<=i;i2++)
								for(j2=1;j2<(srcSize.width-1);j2++)
									if(datadstn[i2*step+j2]==prev_no)
									{
										datadstn[i2*step+j2]=curr_no;
									}
							genblob[curr_no].Centroid.y=(genblob[curr_no].Centroid.y*(genblob[curr_no].Area)+genblob[prev_no].Centroid.y*(genblob[prev_no].Area));
							genblob[curr_no].Centroid.y=genblob[curr_no].Centroid.y/(genblob[prev_no].Area+genblob[curr_no].Area);
							genblob[curr_no].Centroid.x=(genblob[curr_no].Centroid.x*(genblob[curr_no].Area)+genblob[prev_no].Centroid.x*(genblob[prev_no].Area));
							genblob[curr_no].Centroid.x=genblob[curr_no].Centroid.x/(genblob[prev_no].Area+genblob[curr_no].Area);
							genblob[curr_no].Area=genblob[prev_no].Area+genblob[curr_no].Area;
							genblob[prev_no].Area=0;
							prev_no=0;
							curr_no=0;
							//printf("Z\n");
						}else
						{
							//printf("z1\n");
							prev_no=datadstn[i*step+j];
							curr_no=datadstn[(i-1)*step+(j+1)];
							for(i2=1;i2<=i;i2++)
								for(j2=1;j2<srcSize.width-1;j2++)
									if(datadstn[i2*step+j2]==prev_no)
									{
										datadstn[i2*step+j2]=curr_no;
									}
							genblob[curr_no].Centroid.y=(genblob[curr_no].Centroid.y*(genblob[curr_no].Area)+genblob[prev_no].Centroid.y*(genblob[prev_no].Area));
							genblob[curr_no].Centroid.y=genblob[curr_no].Centroid.y/(genblob[prev_no].Area+genblob[curr_no].Area);
							genblob[curr_no].Centroid.x=(genblob[curr_no].Centroid.x*(genblob[curr_no].Area)+genblob[prev_no].Centroid.x*(genblob[prev_no].Area));
							genblob[curr_no].Centroid.x=genblob[curr_no].Centroid.x/(genblob[prev_no].Area+genblob[curr_no].Area);
							genblob[curr_no].Area=genblob[prev_no].Area+genblob[curr_no].Area;
							genblob[prev_no].Area=0;
							prev_no=0;
							curr_no=0;
							//printf("z1\n");
						}
					}else
					{
						datadstn[i*step+j]=datadstn[(i-1)*step+(j+1)];
						genblob[datadstn[i*step+j]].Area++;
						genblob[datadstn[i*step+j]].Centroid.y=(i+genblob[datadstn[i*step+j]].Centroid.y*(genblob[datadstn[i*step+j]].Area-1));
						genblob[datadstn[i*step+j]].Centroid.y=genblob[datadstn[i*step+j]].Centroid.y/genblob[datadstn[i*step+j]].Area;
						genblob[datadstn[i*step+j]].Centroid.x=(j+genblob[datadstn[i*step+j]].Centroid.x*(genblob[datadstn[i*step+j]].Area-1));
						genblob[datadstn[i*step+j]].Centroid.x=genblob[datadstn[i*step+j]].Centroid.x/genblob[datadstn[i*step+j]].Area;
					}
					//printf("p4\n");
				}//printf("p1\n");
				if(!(datadstn[i*step+(j-1)]!=0||datadstn[(i-1)*step+(j-1)]!=0||datadstn[(i-1)*step+(j+1)]!=0||datadstn[(i-1)*step+j]!=0))
				{
					//printf("p5\n");
					NUMBER++;
					if(NUMBER>254)
						return -1;
					datadstn[i*step+j]=NUMBER;
					genblob[NUMBER].BlobId=NUMBER;
					genblob[NUMBER].Centroid=cvPoint2D32f(j,i);
					genblob[NUMBER].Area=1;
					//printf("p5\n");
				}
			//printf("p\n");
			}
		}
	}
	//printf("Number=%d\n",NUMBER);
	int amit=1,aval=-1;
	float max_A=min_area;
	for(;amit<=NUMBER;amit++)
	{
		if(genblob[amit].Area>=max_A)
		{
			max_A=genblob[amit].Area;
			aval=amit;
			printf("id=%darea=%dcentroid(%f,%f)\n",amit,genblob[amit].Area,genblob[amit].Centroid.x-(srcSize.width/2.0),(srcSize.height/2.0)-genblob[amit].Centroid.y);
		}
	}
	//printf("%f%d\n",max_A,aval);
	for(i=0;i<srcSize.height;i++)
	for(j=0;j<srcSize.width;j++)
	if(datadstn[i*step+j]==aval)
	{
	cvSet2D(dstn,i,j,CV_RGB(255,255,255));
	//datadstn[i*step+j]=250;
	//printf("changed\n");
	}
	if(NUMBER>0)
	cvSet2D(dstn,genblob[aval].Centroid.y,genblob[aval].Centroid.x,CV_RGB(255,255,255));
	return aval;
}



int main()
{
	int framecount=0;
	int err=0,blob_no=0,super_change=0,first_check=1,sudden_change=0;
	float change_dir=0.0,changey=0.0,changex=0.0,x=0.0,y=0.0,gcx=0.0,gcy=0.0,ball_ydeg=0.0,ball_xdeg=0.0;
	int marea=0,search=0,chklvl=0;
	unsigned char send_packet[7]={0xff,0xff};

	//INITIALIZE CAMERA
    HIDS hCam = 1;
    initializeCam(hCam);
    setImgMem(hCam);
	// if(init())
	// {
	// 	printf("Program not initalized\n");
	// 	return 0;
	// }
  if(set_motor_value())
  {
  	printf("Unable to change motor values\n");
	return 0;
  }
  //CvTracks tracks;
  AcyutBlob ballBlob[256];
//  cvNamedWindow("camera_stream", CV_WINDOW_AUTOSIZE);
//  cvNamedWindow("thresholded image",CV_WINDOW_AUTOSIZE);
  // CvCapture *capture = cvCaptureFromCAM(300);//firefly
 // CvCapture *capture = cvCaptureFromCAM(-1);//usb cam
  // while(!cvGrabFrame(capture))
  // {	printf("yemi ledu\n");}

  //firefly
  // IplImage *img_bw = cvRetrieveFrame(capture);
  IplImage *img_bw;
  getFrame(hCam, img_bw);
  CvSize imgSize = cvGetSize(img_bw);

  /*//usb cam
  IplImage *img = cvRetrieveFrame(capture);
  CvSize imgSize = cvGetSize(img);
  */
	printf("img width %d,height %d\n",imgSize.width,imgSize.height);

  //firefly
  IplImage *img = cvCreateImage(imgSize, img_bw->depth, 3);
  CvSize imgSize_col = cvGetSize(img);
  IplImage *frame = cvCreateImage(imgSize_col, img->depth, 3);
  
  IplConvKernel* morphKernel = cvCreateStructuringElementEx(5, 5, 1, 1, CV_SHAPE_RECT, NULL);

  unsigned int frameNumber = check_goal_after_nof;
object_position.radial_distance=1000.0;
	float prevDist=0,prevAng=0;
	int frameCount=0;
  
  while (1)
  {
  // cvReleaseCapture(&capture);
   // CvCapture *capture = cvCaptureFromCAM(300);
  // while(!cvGrabFrame(capture))
	// {
		// printf("yemi ledu\n");
	// }
  	//firefly  
    // IplImage *img_bw = cvRetrieveFrame(capture);
    IplImage *img_bw;
    getFrame(hCam, img_bw);
    cvCvtColor(img_bw,img, CV_BayerBG2BGR);
    prevchangey=theta_ydeg;
    //  cvSaveImage("teat.ppm",img_bw);
    //  cvSaveImage("tea1t.ppm",img);
    //usb cam
    //IplImage *img= cvRetrieveFrame(capture);
    
    cvConvertScale(img, frame, 1, 0);
	marea=0;
	search=stop_search;
    
    IplImage *segmentated = cvCreateImage(imgSize, 8, 1);
    
    // Detecting red pixels:
    // (This is very slow, use direct access better...)
    for (unsigned int j=0; j<imgSize.height; j++)
      for (unsigned int i=0; i<imgSize.width; i++)
      {
	CvScalar c = cvGet2D(frame, j, i);

	double b = ((double)c.val[0])/255.;
	double g = ((double)c.val[1])/255.;
	double r = ((double)c.val[2])/255.;
	unsigned char f = 255*((r>0.2 + g)&&(r>0.175+b));
//	unsigned char f = 255*((r>0.8)&&(g<0.75)&&(b<0.3));

	cvSet2D(segmentated, j, i, CV_RGB(f, f, f));
	
      }
    //printf("sr\n");

    cvMorphologyEx(segmentated, segmentated, NULL, morphKernel, CV_MOP_OPEN, 1);

    IplImage *labelImg = cvCreateImage(cvGetSize(frame), 8, 1);
	int ball_blobno=findblob(ballBlob,segmentated,labelImg,150.0);
    //CvBlobs blobs;
    //unsigned int result = cvLabel(segmentated, labelImg, blobs);
    /*
    cvFilterByArea(blobs, 150, 1000000);
    cvUpdateTracks(blobs, tracks, 200.0, 5);
    cvRenderTracks(tracks, frame, frame, CV_TRACK_RENDER_ID|CV_TRACK_RENDER_BOUNDING_BOX);
    *///printf("ao\n");
//	cvShowImage("thresholded image",segmentated);
//    cvShowImage("camera_stream", frame);
    thetax=0;
    thetay=0;
    //printf("asr\n");
	//for (CvBlobs::const_iterator it=blobs.begin(); it!=blobs.end(); ++it)
	if(ball_blobno>0)
	{
		//printf("xr\n");
		x=ballBlob[ball_blobno].Centroid.x-(imgSize.width/2),y=(imgSize.height/2)-ballBlob[ball_blobno].Centroid.y;
		//printf("\nBlob #%d : Area=%d, Centroid=(%4.2f,%4.2f)",it->second->label,it->second->area,x,y);
		//float r=HT/cos(theta);
		//printf("max area=%d\n",marea);
		//printf("ar\n");
		/*if(marea>(it->second->area))
		{
			//printf("max area=%d\n",marea);
			continue;
		}else marea=(it->second->area);
		*/thetax=(x/len);
		thetay=(y/len);
		//thetay=thetay*cos(theta)/constanty;
		//thetay=(((0.5+(0.5*(y*y)/(144*144)))*2)*thetay)/constanty;
		thetay=thetay/constanty;
		thetax=((((1/mltplr_x)+((1-(1/mltplr_x))*(x*x)/((imgSize.width/2)*(imgSize.width/2))))*mltplr_x)*thetax)/constantx;
		//printf("Centroid=(%4.2f,%4.2f)\n",x,y);
		if(frameCount==0)
  {
  	prevDist=object_position.radial_distance;
  	prevAng=object_position.ANGLE;
  }
 		if(abs(x)<box_x)
			thetax=0.0;
		if(abs(y)<box_y)
		{
			framecount++;
			thetay=0.0;
			//printf("Distance = %8.2f,Angle= %f\n",(height*tan(deg2rad(theta_ydeg)+thetay)),theta_xdeg-150);
			object_position.radial_distance=(height*tan(deg2rad(theta_ydeg)+thetay));
			object_position.ANGLE=theta_xdeg-150;
			//printf("--%f--\n",(prevDist-object_position.radial_distance)/prevDist);
    		if((prevDist-object_position.radial_distance)/prevDist>0.2 && object_position.radial_distance<400)//||object_position.radial_distance<70)
    		{
    		float prevAngRad= deg2rad(prevAng);
    		float currAngRad= deg2rad(object_position.ANGLE);
    		float projDist=object_position.radial_distance*prevDist*(cos(currAngRad)*sin(prevAngRad)-sin(currAngRad)*cos(prevAngRad))/(object_position.radial_distance*cos(currAngRad)-prevDist*cos(prevAngRad));
    		//printf("New algo ==%f==\n",projDist);
    		//if(object_position.radial_distance*prevDist*(cos(currAngRad)*sin(object_position.ANGLE)-sin(currAngRad)*cos(object_position.ANGLE))/(object_position.radial_distance*cos(currAngRad)-prevDist*cos(prevAngRad))<150)
    		
    		if(projDist<0 && projDist>-250 )
    		printf("FALL LEFT! %f\n",(prevDist-object_position.radial_distance)/prevDist);
    		else if(projDist>0 && projDist<250 )
    		printf("FALL RIGHT! %f\n",(prevDist-object_position.radial_distance)/prevDist);    		
      		} 
   			prevDist=object_position.radial_distance;
		  	prevAng=object_position.ANGLE;
			
			frameNumber++;
		}search=1;
	}
	//printf("search=%d\n",search);
	/*if(err=read_motor_value())
	{
		printf("error: %d\n",err);
	}*/
	changey=rad2deg(thetay);
	/*if(changey<(-0.3)&&changey>(-1.9))
	{
		changey=-1.9;
		usleep(10000);
	}*/
	changex=rad2deg(thetax);
	//    cvReleaseBlobs(blobs);
    cvReleaseImage(&labelImg);
    cvReleaseImage(&segmentated);
    cvReleaseImage(&img_bw);
  frameCount++;
  
    
	//printf("change init %f,change xinit%f\n",changey,changex);
	if(changey>=rangey)
	{
		changey=rangey;
		//printf("change finaly %f\n",changey);
	}
	if(changey<=-rangey)
	{
		changey=-rangey;
		//printf("change finaly %f\n",changey);
	}
	if(changex>=rangex)
	{
		changex=rangex;
		//printf("change finalx %f\n",changex);
	}
	if(changex<=-rangex)
	{
		changex=-rangex;
		//printf("change final x%f\n",changex);
	}
	//printf("height= %f\n",height);
	if(search==0)
	{
		changey=0;
		changex=0;
		if(abs(x)>search_dir_var)
			change_dir=x/abs(x);
		else change_dir=0.0;
		if(abs(change_dir)>0.1)
		{
			if(theta_xdeg<=rmaxx&&theta_xdeg>=rminx)
				changex=change_dir*rchangex;
			else
			{
				if(change_dir>=0.1)
					theta_xdeg=rminx;
				else
					theta_xdeg=rmaxx;
				chklvl=1;
			}
		}else if(theta_xdeg<=rmaxx)
			changex=rchangex;
		else
		{
			theta_xdeg=rminx;
			chklvl=1;
		}
		if(chklvl==1)
		{
			chklvl=0;
			//printf("hi\n");
			if(theta_ydeg<(rmaxy-(rchangey+1.0))&&theta_ydeg>=rminy)
			{
				changey=rchangey;
			//printf("hii\n");
			}
			else
			{	//printf("hi2\n");
				theta_ydeg=rminy;
				super_change=1;
				changey=0.0;
			}
		}
	}//printf("theta_ydeg+change=%ftheta_ydeg=%f\n",theta_ydeg+changey,theta_ydeg);
	//printf("theta%f\n",theta_xdeg);
	if(theta_ydeg+changey<(rmaxy-1.0)&&theta_ydeg+changey>=(rminy-1.0))
	{
		//printf("here\n");
		theta_ydeg=theta_ydeg+changey;
		//printf("theta_ydeg+change=%ftheta_ydeg=%f\n",theta_ydeg+changey,theta_ydeg);
		theta_xdeg=theta_xdeg+changex;
		//printf("theta_ydeg=%f\n",theta_ydeg);
		set_motor_value();
		if(super_change==1)
		{
			super_change=0;
			usleep(100000);
		}
 	}/*
 	if(result>0)
 	{
 		//printf("Motor change %f\n",(theta_ydeg-prevchangey)/prevchangey);
 		if((theta_ydeg-prevchangey)/prevchangey>0.1&&(theta_ydeg-prevchangey)/prevchangey<5&& object_position.radial_distance<400)
 		{
 		             
 		//printf("\nFALL MOTOR %f\n",(changey-prevchangey)/prevchangey);
 		
 		}
 	}*/
 	update_height();
    /*std::stringstream filename;
    filename << "redobject_" << std::setw(5) << std::setfill('0') << frameNumber << ".png";
    cvSaveImage(filename.str().c_str(), frame);*/
    if ((cvWaitKey(10)&0xff)==27)
      break;
   // frameNumber++;
  }
// cvReleaseCapture(&capture);
  cvReleaseStructuringElement(&morphKernel);
  cvReleaseImage(&frame);

  cvDestroyWindow("camera_stream");

  return 0;
}




