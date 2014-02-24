#include "camcapture.h"
using namespace std; ////
    HIDS hCam;
//pass true as parameter to load luts, false to skip loading luts
CamCapture::CamCapture(bool param, int percent, int percent2)
{
    hCam = 1;
    imgPointer = NULL;
	isInit = false;
    doLUT = param;
    small_percent = percent;
    small_percent2 = percent2;
    width_var = (640*percent)/100;
    height_var = (480*percent)/100;
    width_var_small = (640*percent2)/100;
    height_var_small = (480*percent2)/100;
    width_var_full = 640;
    height_var_full = 480;
}



CamCapture::~CamCapture()
{
	if(isInit==false)
		return;

	
    cvReleaseImage(&rgbimg);
    // Stop capturing images

    int nRet = is_ExitCamera (hCam);
    if(nRet != IS_SUCCESS)
    {
        printf("Could not exit camera \n");
    }
}


CamError CamCapture::init()
{
    if(isInit==true)
        return CAM_SUCCESS;

    //Loading lookup table
    if(doLUT == true)
    {
        printf("Loading lookup tables...");
        if(loadLUT(REDC)==false||loadLUT(BLUEC)==false||loadLUT(YELLOWC)==false||loadLUT(GREENC)==false||loadLUT(WHITEC)==false||loadLUT(BLACKC)==false)
        {
            printf("Unable to open LUT\n");
            return CAM_FAILURE;
        }
        printf("Loaded.\n");
    }
    //Initializing camera
	
    char* errMsg = (char*)malloc(sizeof(char)*200);
    int err = 0;
    
    int nRet = is_InitCamera (&hCam, NULL);
    
    if (nRet != IS_SUCCESS)
    {
        is_GetError (hCam, &err, &errMsg);
        printf("Camera Init Error %d: %s\n",err,errMsg);
        return CAM_FAILURE;
    }

    nRet = is_SetColorMode(hCam, IS_CM_BGR8_PACKED);
    if (nRet != IS_SUCCESS) 
    {
            is_GetError (hCam, &err, &errMsg);
            printf("Color Mode Error %d: %s\n",err,errMsg);
            return CAM_FAILURE;  
    }

    nRet = is_SetHardwareGain(hCam, 100, 4, 0, 13);
    if (nRet != IS_SUCCESS) 
    {
            is_GetError (hCam, &err, &errMsg);
            printf("Hardware Gain Error %d: %s\n",err,errMsg);
            return CAM_FAILURE;
    }

    nRet = is_AllocImageMem(hCam, 752, 480, 24, &imgPointer, &imgMemPointer);
    if(nRet != IS_SUCCESS)
    {
        is_GetError (hCam, &err, &errMsg);
        printf("MemAlloc Unsuccessful %d: %s\n",err,errMsg);
        return CAM_FAILURE;
    }

    nRet = is_SetImageMem (hCam, imgPointer, imgMemPointer);
    if(nRet != IS_SUCCESS)
    {
        is_GetError (hCam, &err, &errMsg);
        printf("Could not set/activate image memory %d: %s\n",err, errMsg);
        return CAM_FAILURE;
    }

    double newFPS;
    nRet = is_SetFrameRate(hCam, 60.0, &newFPS);
    printf("FPS is set to %lf\n", newFPS);
    if(nRet != IS_SUCCESS)
    {
        is_GetError (hCam, &err, &errMsg);
        printf("Setting framerate Unsuccessful %d: %s\n",err,errMsg);

        return CAM_FAILURE;
    }
    free(errMsg);

    originalImg = cvCreateImage(cvSize(752,480),8,3);
    rgbimg_full = cvCreateImage(cvSize(640, 480), 8, 3);
    rgbimg = cvCreateImage(cvSize((640*small_percent)/100, (480*small_percent)/100), 8, 3);
    rgbimg_small = cvCreateImage(cvSize((640*small_percent2)/100, (480*small_percent2)/100), 8, 3);

#ifdef CAMCAPTURE_DEBUG
    showSeg = cvCreateImage(cvSize(rgbimg->width, rgbimg->height), 8, 3);
#endif
    isInit = true;
    return CAM_SUCCESS;

}



CamError CamCapture::getImage()
{
	// if(isInit==false)
 //        {
 //        printf("CAMERA NOT INIT.\n");
 //        sleep
	// 	return CAM_FAILURE;
 //        }

        while(isInit==false)
                {
                    printf("Waiting for camera to init\n");
                    usleep(500000);
                }
    // Start capturing images
    

    char* errMsg = (char*)malloc(sizeof(char)*200);
    int err = 0;
    // printf("stage1 getImage\n");
    int nRet = is_FreezeVideo (hCam,IS_DONT_WAIT);
    // printf("stage2 getImage\n");
    // if(nRet != IS_SUCCESS)
    // {
    //     printf("nRet is %d\n",nRet);
    //     is_GetError (hCam, &err, &errMsg);
    //     printf("Could not grab image %d: %s\n",err,errMsg);
    //     printf("lol\n");

    //     return CAM_FAILURE;
    // }
    
    // if(nRet != IS_SUCCESS)
    //     {
    //         this->~CamCapture();
    //         printf("Camera instance destroyed");
    //         return CAM_REINITIALIZE;
    //     }



    //fill in the OpenCV imaga data 
    memcpy(originalImg->imageData, imgPointer, 752*480 * 3);
    //originalImg->imageData = imgPointer;


    cvSetImageROI(originalImg, cvRect(56, 0,640,480));
        
    cvCopy(originalImg, rgbimg_full, NULL);
    cvResetImageROI(originalImg);


    if(small_percent==100)
        cvCopy(rgbimg_full, rgbimg);
    else
        cvResize(rgbimg_full, rgbimg, CV_INTER_NN); //nearest neighbour. fastest but doesn't look very good
    
    if(small_percent2==100)
        cvCopy(rgbimg_full, rgbimg_small);
    else
    {
        if(small_percent2==small_percent)
            cvCopy(rgbimg, rgbimg_small);
        else
            cvResize(rgbimg_full, rgbimg_small, CV_INTER_NN);
    }
    free(errMsg);

#ifdef CAMCAPTURE_DEBUG
    if(doLUT==true)
        showSegmentation();
#endif

    return CAM_SUCCESS;
}



bool CamCapture::loadLUT(int color)
{
    //Loads lut in variable lut
    FILE *fp;
    uchar** lut_address;
    char file[100];
    switch(color)
    {
        case REDC: 
        lut_address = &lut_red;
        strcpy(file, "Source/lut/red.lut");
        break;

        case BLUEC:
        lut_address = &lut_blue;
        strcpy(file, "Source/lut/blue.lut");
        break;

        case YELLOWC:
        lut_address = &lut_yellow;
        strcpy(file, "Source/lut/yellow.lut");
        break;

        case GREENC:
        lut_address = &lut_green;
        strcpy(file, "Source/lut/green.lut");
        break;

        case WHITEC:
        lut_address = &lut_white;
        strcpy(file, "Source/lut/white.lut");
        break;

        case BLACKC:
        lut_address = &lut_black;
        strcpy(file, "Source/lut/black.lut");
        break;        

    }
    fp = fopen(file,"rb");
    if(!fp)
        return false;

    *lut_address = new uchar [256*256*256];
    if(!fread(*lut_address,sizeof(uchar),256*256*256,fp))
        return false;

    fclose(fp);

    return true;
}



void CamCapture::showSegmentation()
{
    CvScalar pixel; 
    for (int x = 0; x < width_var; ++x)
    {
        for (int y = 0; y < height_var; ++y)
        {
            
            if(isRed(x,y))
            {
                setPixel3C(pixel,0,0,255);
            }
            else if(isBlue(x, y))
            {
                setPixel3C(pixel,255,0,0);
            }
            else if(isYellow(x, y))
            {
                setPixel3C(pixel,0,255,255);
            }
            else if(isGreen(x, y))
            {
                setPixel3C(pixel,0,255,0);
            }
            else if(isWhite(x, y))
            {
                setPixel3C(pixel, 255, 255, 255);
            }
            else if(isBlack(x, y))
            {
                setPixel3C(pixel, 127, 127, 127);   
            }
            else
            {
                setPixel3C(pixel,0,0,0);
            }

            cvSet2D(showSeg, y, x, pixel);
        }
    }
}
// void CamCapture::makeinfoimg(IplImage* color, uchar* lut)
// {

//     for(int i=0; i<color->width; i++)
//     for(int j=0; j<color->height; j++)
//     {
//         returnPixel1C(infoimg, i, j) = lut[returnPixel3C(color, i, j, RED)
//                                         |(returnPixel3C(color, i, j, GREEN)<<8)
//                                         |(returnPixel3C(color, i, j, BLUE)<<16)];
//     }

// }
