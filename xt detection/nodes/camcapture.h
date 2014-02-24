#ifndef CAMCAPTURE_H
#define CAMCAPTURE_H ////

#include <opencv2/opencv.hpp>
#include "imgproc.h"
#include <cstring>
#include <cstdio>
#include <ueye.h>

// #define CAMCAPTURE_DEBUG
/*
HOW TO USE THIS CLASS
1. Make object of class.
2. Call init().
3. When you need image, call getImage().
   It will store the images in
   rgbimg, bayerimg
4. ** IF YOU DON'T WANT LOOKUP TABLES, SEND false AS ARGUMENT TO CONSTRUCTOR. THEN U CANNOT USE isRed() etc. **
There are 3 images:
rgbimg - default one, to be used normally. it may be reduced or not.
rgbimg_full - full image, direct from camera. does not have any functions to check colors (no lut lookup functions for this)
rgbimg_small - 2nd reduced image. to be used for blob detection. associated lut functions have _small in front of them
*/

//TODO:
//Add feature such that u can pass to constructor what video mode u want
//Maybe enum it for ease of use


enum CamError {CAM_SUCCESS = 1, CAM_FAILURE = 0, CAM_REINITIALIZE=11};
class CamCapture
{
private:

    char* imgPointer;
    int imgMemPointer;
    bool isInit;
    bool doLUT;
    unsigned char* lut_red;
    unsigned char* lut_yellow;
    unsigned char* lut_blue;
    unsigned char* lut_green;
    unsigned char* lut_white;
    unsigned char* lut_black;
    int width_var;
    int height_var;
    int width_var_full;
    int height_var_full;
    int width_var_small;
    int height_var_small;
    int small_percent;
    int small_percent2;
    bool loadLUT(int color);
    void showSegmentation();
    //TODO:
    //makeinfoimg is our own implementation of a lut
    //OpenCV has a function called LUT that does this job
    //Maybe look into it?
 //   void makeinfoimg(IplImage*, unsigned char*);

public:
    IplImage* originalImg;
    IplImage* rgbimg;
    IplImage* rgbimg_full;
    IplImage* rgbimg_small;
    IplImage* showSeg;
    inline bool isRed(int i, int j)
    {
        if(lut_red[returnPixel3C(rgbimg, i, j, 2)
            |(returnPixel3C(rgbimg, i, j, 1)<<8)
            |(returnPixel3C(rgbimg, i, j, 0)<<16)])
            return true;
        else
            return false;

    }
    inline bool isBlue(int i, int j)
    {
        if(lut_blue[returnPixel3C(rgbimg, i, j, 2)
            |(returnPixel3C(rgbimg, i, j, 1)<<8)
            |(returnPixel3C(rgbimg, i, j, 0)<<16)])
            return true;
        else
            return false;

    }
    inline bool isYellow(int i, int j)
    {
        if(lut_yellow[returnPixel3C(rgbimg, i, j, 2)
            |(returnPixel3C(rgbimg, i, j, 1)<<8)
            |(returnPixel3C(rgbimg, i, j, 0)<<16)])
            return true;
        else
            return false;

    }
    inline bool isGreen(int i, int j)
    {
        if(lut_green[returnPixel3C(rgbimg, i, j, 2)
            |(returnPixel3C(rgbimg, i, j, 1)<<8)
            |(returnPixel3C(rgbimg, i, j, 0)<<16)])
            return true;
        else
            return false;

    }
    inline bool isWhite(int i, int j)
    {
        if(lut_white[returnPixel3C(rgbimg, i, j, 2)
            |(returnPixel3C(rgbimg, i, j, 1)<<8)
            |(returnPixel3C(rgbimg, i, j, 0)<<16)])
            return true;
        else
            return false;
    }
    inline bool isBlack(int i, int j)
    {
        if(lut_black[returnPixel3C(rgbimg, i, j, 2)
            |(returnPixel3C(rgbimg, i, j, 1)<<8)
            |(returnPixel3C(rgbimg, i, j, 0)<<16)])
            return true;
        else
            return false;
    }

        inline bool isRed_small(int i, int j)
    {
        if(lut_red[returnPixel3C(rgbimg_small, i, j, 2)
            |(returnPixel3C(rgbimg_small, i, j, 1)<<8)
            |(returnPixel3C(rgbimg_small, i, j, 0)<<16)])
            return true;
        else
            return false;

    }
    inline bool isBlue_small(int i, int j)
    {
        if(lut_blue[returnPixel3C(rgbimg_small, i, j, 2)
            |(returnPixel3C(rgbimg_small, i, j, 1)<<8)
            |(returnPixel3C(rgbimg_small, i, j, 0)<<16)])
            return true;
        else
            return false;

    }
    inline bool isYellow_small(int i, int j)
    {
        if(lut_yellow[returnPixel3C(rgbimg_small, i, j, 2)
            |(returnPixel3C(rgbimg_small, i, j, 1)<<8)
            |(returnPixel3C(rgbimg_small, i, j, 0)<<16)])
            return true;
        else
            return false;

    }
    inline bool isGreen_small(int i, int j)
    {
        if(lut_green[returnPixel3C(rgbimg_small, i, j, 2)
            |(returnPixel3C(rgbimg_small, i, j, 1)<<8)
            |(returnPixel3C(rgbimg_small, i, j, 0)<<16)])
            return true;
        else
            return false;

    }
    inline bool isWhite_small(int i, int j)
    {
        if(lut_white[returnPixel3C(rgbimg_small, i, j, 2)
            |(returnPixel3C(rgbimg_small, i, j, 1)<<8)
            |(returnPixel3C(rgbimg_small, i, j, 0)<<16)])
            return true;
        else
            return false;
    }
    inline bool isBlack_small(int i, int j)
    {
        if(lut_black[returnPixel3C(rgbimg_small, i, j, 2)
            |(returnPixel3C(rgbimg_small, i, j, 1)<<8)
            |(returnPixel3C(rgbimg_small, i, j, 0)<<16)])
            return true;
        else
            return false;
    }
    CamCapture(bool param = true, int percent = 100, int percent2 = 50);
    ~CamCapture();
    CamError init();
    CamError getImage();
    inline int width(){     return width_var;}
    inline int height(){    return height_var;}
    inline int width_full(){     return width_var_full;}
    inline int height_full(){    return height_var_full;}
    inline int width_small(){     return width_var_small;}
    inline int height_small(){    return height_var_small;}
};
#endif