#include <cstdio>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include "camcapture.h"
#include "imgproc.h"

using namespace std;

typedef unsigned char uchar;

struct ColourData
{
    int yminv, ymaxv, uminv, umaxv, vminv, vmaxv;
};

inline int doCheck3C(int a, int b, int c, int mina, int maxa, int minb, int maxb, int minc, int maxc)
{
	if((a<=maxa)&&(a>=mina)&&(b<=maxb)&&(b>=minb)&&(c<=maxc)&&(c>=minc))
		return 1; 
	return 0;
}

uchar* marker_create_lut(ColourData &c)
{
    uchar* lut = new unsigned char [256*256*256];
    float r,g,b;
    float Ys, Crs, Cbs;
    int value;

    for(r=0.0; r<256.0; r++)
    for(g=0.0; g<256.0; g++)
    for(b=0.0; b<256.0; b++)
    {
		Ys = 0.299*r + 0.587*g + 0.114*b;
		Crs = (r-Ys)*0.713 + 128.0;
		Cbs = (b-Ys)*0.564 + 128.0;
		if(doCheck3C(Ys, Crs, Cbs, c.yminv, c.ymaxv, c.uminv, c.umaxv, c.vminv, c.vmaxv))
			lut[(uchar)r + (((uchar)g)<<8) + (((uchar)b)<<16)] = 255;
		else
			lut[(uchar)r + (((uchar)g)<<8) + (((uchar)b)<<16)] = 0;
    }
    return lut;
}

int main(int argc, char const *argv[])
{
	CamCapture cam(false);
	cam.init();

	int yminval = 0;
	int ymaxval = 255;
	int uminval = 0;
	int umaxval = 255;
	int vminval = 0;
	int vmaxval = 255;

	int red = false;
	int black = false;
	int blue = false;
	int yellow = false;
	int white = false;
	int green = false;

	cvNamedWindow("sliders", CV_WINDOW_NORMAL);
	cvCreateTrackbar("yminval","sliders",&yminval,255,0);
	cvCreateTrackbar("ymaxval","sliders",&ymaxval,255,0);
	cvCreateTrackbar("uminval","sliders",&uminval,255,0);
	cvCreateTrackbar("umaxval","sliders",&umaxval,255,0);
	cvCreateTrackbar("vminval","sliders",&vminval,255,0);
	cvCreateTrackbar("vmaxval","sliders",&vmaxval,255,0);

	cvNamedWindow("LUT Color (Choose 1 only)", CV_WINDOW_NORMAL);
	cvCreateTrackbar("red", "LUT Color (Choose 1 only)", &red, 1, 0);
	cvCreateTrackbar("black", "LUT Color (Choose 1 only)", &black, 1, 0);
	cvCreateTrackbar("blue", "LUT Color (Choose 1 only)", &blue, 1, 0);
	cvCreateTrackbar("yellow", "LUT Color (Choose 1 only)", &yellow, 1, 0);
	cvCreateTrackbar("white", "LUT Color (Choose 1 only)", &white, 1, 0);
	cvCreateTrackbar("green", "LUT Color (Choose 1 only)", &green, 1, 0);

	while(1)
	{
		cam.init();
		cam.getImage();
		IplImage *threshy = cvCreateImage(cvSize(cam.width(), cam.height()), 8, 1);
		for (int i = 0; i < cam.width(); ++i)
		{
			for (int j = 0; j < cam.height(); ++j)
			{
				int rgb[3];
				pixelColor3C(cam.rgbimg, i, j, rgb);
				float y, u, v;
				y=0.299*rgb[0]+0.587*rgb[1]+0.114*rgb[2];
				u=(rgb[0]-y)*0.713+128.0;
				v=(rgb[2]-y)*0.564+128.0;
				if(y>=yminval&&y<=ymaxval&&u>=uminval&&u<=umaxval&&v>=vminval&&v<=vmaxval)
					returnPixel1C(threshy,i,j)=255;
				else
					returnPixel1C(threshy,i,j)=0;
			}
		}
		cvShowImage("frame", cam.rgbimg);
		cvShowImage("threshy", threshy);
		cvMoveWindow("frame", 100, 100);
		cvMoveWindow("threshy", 100 + cam.width(), 100);
		int c = cvWaitKey(10);
		if(c == 27)
			exit(0);
		if(c == 's' || c == 'S')
		{
			ColourData c;
			c.yminv = yminval;
			c.ymaxv = ymaxval;
			c.uminv = uminval;
			c.umaxv = umaxval;
			c.vminv = vminval;
			c.vmaxv = vmaxval;
			if(red)
			{
				uchar *mylut = marker_create_lut(c);
				FILE *fp = fopen("./lut_out/red.lut", "wb");
				fwrite(mylut,sizeof(uchar),256*256*256,fp);
    			fclose(fp);
			}
			if(black)
			{
				uchar *mylut = marker_create_lut(c);
				FILE *fp = fopen("./lut_out/black.lut", "wb");
				fwrite(mylut,sizeof(uchar),256*256*256,fp);
    			fclose(fp);
			}
			if(blue)
			{
				uchar *mylut = marker_create_lut(c);
				FILE *fp = fopen("./lut_out/blue.lut", "wb");
				fwrite(mylut,sizeof(uchar),256*256*256,fp);
    			fclose(fp);
			}
			if(yellow)
			{
				uchar *mylut = marker_create_lut(c);
				FILE *fp = fopen("./lut_out/yellow.lut", "wb");
				fwrite(mylut,sizeof(uchar),256*256*256,fp);
    			fclose(fp);
			}
			if(white)
			{
				uchar *mylut = marker_create_lut(c);
				FILE *fp = fopen("./lut_out/white.lut", "wb");
				fwrite(mylut,sizeof(uchar),256*256*256,fp);
    			fclose(fp);
			}
			if(green)
			{
				uchar *mylut = marker_create_lut(c);
				FILE *fp = fopen("./lut_out/green.lut", "wb");
				fwrite(mylut,sizeof(uchar),256*256*256,fp);
    			fclose(fp);
			}
		}
	}

	return 0;
}