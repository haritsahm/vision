#ifndef IMGPROC_H//
#define IMGPROC_H
//WARNING: imgproc contains dangerous #defines, must be places after all other headers, otherwise causes errors
#include <opencv2/opencv.hpp>

typedef unsigned char uchar;

#define NOCOLS 7 //Plz update this!
#define YELLOWC 4
#define GREENC 2
#define BLUEC 1
#define ORANGEC 8
#define BLACKC 16
#define REDC 32
#define WHITEC 64

#define returnPixel1C(image, x, y) ((uchar*)(image->imageData + image->widthStep*(y)))[x]
#define returnPixel3C(image, x, y, color) ((uchar*)(image->imageData + image->widthStep*(y)))[(x)*3 + color]


//BGR
inline void setPixel3C(CvScalar &pixel, uchar blue, uchar green, uchar red)
{
	pixel.val[0] = blue;
	pixel.val[1] = green;
	pixel.val[2] = red;
	return;
}

inline uchar pixelColor3C(IplImage* image, int x, int y, int color)
{
	return ((uchar*)(image->imageData + image->widthStep*y))[x*3 + color];
}

inline void pixelColor3C(IplImage* image, int x, int y, int *rgb)
{
	rgb[0] = ((uchar*)(image->imageData + image->widthStep*y))[x*3 + 2];
	rgb[1] = ((uchar*)(image->imageData + image->widthStep*y))[x*3 + 1];
	rgb[2] = ((uchar*)(image->imageData + image->widthStep*y))[x*3 + 0];
	return;
}

inline uchar pixelColor1C(IplImage* image, int x, int y)
{
	return ((uchar*)(image->imageData + image->widthStep*y))[x];
}

//Deprecated. Now, there isn't any single dataimg (infoimg) in camcapture. Instead, each color has it's own lookup table
inline bool chkcolor(IplImage* dataimg,int hori,int vert,int colorcode)
{
	int rawcode =(int)((uchar*)(dataimg->imageData + vert * dataimg->widthStep))[hori];
	return (bool)((rawcode/colorcode)%2);
}





#endif
