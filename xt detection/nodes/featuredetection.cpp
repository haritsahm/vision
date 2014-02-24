#include "featuredetection.h"
//NOTE: THIS FILE ASSUMES IN SEVERAL PLACES THAT rgbimg_small IS HALF THE SIZE OF rgbimg!!!!!!!!!!!!!!!!!!!
//1. Someplace in ballFind, i have multiplied ballX_var by 2 assuming main image is double the size of image used here
//2. in finding ballRatio, same assumption has been made
//3. in line detection etc. 1/4th of image used here is used(in the end assuming it was half of main image, as 1/8 of main image is needed)
using namespace cvb;
using namespace tbb;
using namespace LOCALIZE_INTERNALS;

FeatureDetection::FeatureDetection(CamCapture &cam): IMAGE_HEIGHT(cam.height_small()), IMAGE_WIDTH(cam.width_small())
{
    seg_red = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), 8, 1);
    seg_blue = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), 8, 1);
    seg_yellow = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), 8, 1);
    labelImg = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_LABEL, 1);
    labelImg_small = cvCreateImage(cvSize(IMAGE_WIDTH/4, IMAGE_HEIGHT/4), IPL_DEPTH_LABEL, 1);
    seg_white_count = cvCreateImage(cvSize(IMAGE_WIDTH/4, IMAGE_HEIGHT/4), 8, 1);
    seg_white = cvCreateImage(cvSize(IMAGE_WIDTH/4, IMAGE_HEIGHT/4), 8, 1);
    seg_black = cvCreateImage(cvSize(IMAGE_WIDTH/4, IMAGE_HEIGHT/4), 8, 1);
    seg_green = cvCreateImage(cvSize(IMAGE_WIDTH/4, IMAGE_HEIGHT/4), 8, 1);
    ballRatio=1.0;
    ballFound_var = false;
    tempnLand = 0;
    tempnObstacle = 0;
    o.clear();
    l.clear();
    constants.open("Source/lut/constants.dat",ios::binary);
    cvZero(seg_yellow);
    cvZero(seg_red);
}



//TODO: re-write this function to make it readable and maybe more efficient.
//TODO: document how it works (if documenting somewhere else, give a link)
//returns angle in degrees
// void FeatureDetection::findReal(int x,int y, double &objdis, double &objangdeg, HeadMotor &hm)
// {
//  double s=1,focal=533.33;
//  double thetaX = hm.thetaX();
//  double thetaY = hm.thetaY();
//  thetaX += forwardTiltCorrection*(PI/180.);
//  objdis=(((IMAGE_HEIGHT/2-y)+(focal/s)*tan(thetaX))/(1-(s/focal)*(IMAGE_HEIGHT/2-y)*tan(thetaX)));
//  double perpend=(x-(IMAGE_WIDTH/2))*((s/focal)*(objdis)*sin(thetaX)+cos(thetaX))*pix2cm;
//  objdis=pix2cm*(objdis-(focal/s)*tan(thetaX)) + (s_height+(neck_len/sin(thetaX)))*tan(thetaX);
//  //printf("%lf %lf %lf %lf\n",perpend,objdis,rad2deg(atan2(perpend,objdis)),rad2deg(thetaY));
// //   printf("thetaX is %lf\t",rad2deg(thetaY));
// //   printf("x %lf y %lf\n", rad2deg(thetaX), rad2deg(thetaY));
//  objangdeg=rad2deg(thetaY) - 150 + rad2deg(atan2(perpend,objdis));
//  objdis=sqrt(objdis*objdis+perpend*perpend);
    
// }
void FeatureDetection::undistort(int xd, int yd, int* xu, int* yu)
{
    double ax = -7.1e-06;
    double r2 = xd*xd +yd*yd;    
    *xu = xd/(1+ax*r2);
    *yu = yd/(1+ax*r2);
    #ifndef ALL_PRINTING_OFF
    printf("xd = %d \t\t\t yd = %d\n", xd, yd);
    printf("xu = %d \t\t\t yu = %d\n", *xu, *yu);
    #endif
}

void FeatureDetection::findReal(int X,int Y, double &objdis, double &objangdeg, HeadMotor &hm)
{
    int x,y;
    X = X - IMAGE_WIDTH/2;
    Y = IMAGE_HEIGHT/2 - Y;
    undistort(X,Y,&x,&y);
    x = x + IMAGE_WIDTH/2;
    y = IMAGE_HEIGHT/2 - y;
    double s = 1.0;
    double motorX = hm.motorX();
    double thetaX = hm.thetaX();
    double thetaY = hm.thetaY();

    parameters entry;
    // parameters temp;
    // constants.seekg(0,ios::beg);
    // while(1)
    // {
    //     if(constants.eof())
    //     {
    //         constants.close();
    //         printf("MOTOR POS NOT FOUND\n");
    //         break;
    //     }
    //     constants.read((char*)&temp,sizeof(temp));
    //     if(temp.motor_pos == motorX)
    //     {
    //         entry = temp;
    //         printf("FOUND\n");
    //         break;
    //     }
    // }
    entry.angle = 37.5;
    entry.focal = 198.74;
    entry.pix2cmy = 1.25;
    entry.pix2cmx = 1.09589;
    entry.s_view_compensation = 187.0;

    //correct    // objdis=(((IMAGE_HEIGHT/2-y)+(entry.focal/s)*tan(entry.angle))/(1-(s/entry.focal)*(IMAGE_HEIGHT/2-y)*tan(entry.angle)));
        // double perpend= - (x-(IMAGE_WIDTH/2))*((s/entry.focal)*(objdis)*sin(entry.angle)+cos(entry.angle))*entry.pix2cmx; //NEGATiVE VALUE!!
        // objdis=entry.pix2cmy*(objdis) + entry.s_view_compensation;

    objdis= (((IMAGE_HEIGHT/2-y)+(entry.focal/s)*tan(entry.angle))/(1-(s/entry.focal)*(IMAGE_HEIGHT/2-y)*tan(entry.angle)));
    float perpend= (x-(IMAGE_WIDTH/2))*((s/entry.focal)*(objdis)*sin(entry.angle)+cos(entry.angle))*entry.pix2cmx; //NEGATiVE VALUE!!(removed)
    objdis=entry.pix2cmy*(objdis) + entry.s_view_compensation;
    objangdeg=rad2deg(thetaY) + rad2deg(atan2(perpend,objdis));
    // objangdeg = 0.0 + rad2deg(atan2(perpend,objdis));
    objdis=sqrt(objdis*objdis+perpend*perpend);

    // objdis=(((IMAGE_HEIGHT/2-y)+(31.641/s)*tan(31.641))/(1-(s/98.44)*(IMAGE_HEIGHT/2-y)*tan(31.641)));
    // double perpend=(x-(IMAGE_WIDTH/2))*((s/98.44)*(objdis)*sin(31.641)+cos(31.641))*0.66667;
    // objdis=1.0211*(objdis) + 240;
    #ifndef ALL_PRINTING_OFF
    printf("PERPEND : \t\t\t%lf\n", perpend);
    printf("OBJDIS: \t\t\t%lf\n\n\n\n", objdis);
    #endif
    // objangdeg=rad2deg(thetaY) - 150 + rad2deg(atan2(perpend,objdis));
    // objdis=sqrt(objdis*objdis+perpend*perpend);
    
}



//For parallel segmenting
class SegmentImages {
    IplImage* my_seg_yellow;
    IplImage* my_seg_blue;
    IplImage* my_seg_red;
    IplImage* my_seg_green;
    CamCapture* my_cam;
public:
    void operator()( const blocked_range2d<size_t>& r ) const {
        for( size_t x=r.rows().begin(); x!=r.rows().end(); ++x )
        {
            for( size_t y=r.cols().begin(); y!=r.cols().end(); ++y ) 
            {
                if(my_cam->isYellow_small(x, y))
                    returnPixel1C(my_seg_yellow, x, y) = 255;
                else
                    returnPixel1C(my_seg_yellow, x, y) = 0;

                // if(my_cam->isGreen_small(x, y))
                //     returnPixel1C(my_seg_green, x, y) = 255;
                // else
                //     returnPixel1C(my_seg_green, x, y) = 0;


                // if(my_cam->isBlue_small(x, y))
                //  returnPixel1C(my_seg_blue, x, y) = 255;
                // else
                //  returnPixel1C(my_seg_blue, x, y) = 0;

                if(my_cam->isRed_small(x, y))
                    returnPixel1C(my_seg_red, x, y) = 255;
                else
                    returnPixel1C(my_seg_red, x, y) = 0;
            }
        }
    }
    SegmentImages(IplImage* &seg_yellow, IplImage* &seg_blue, IplImage* &seg_red,IplImage* &seg_green, CamCapture &cam) :
        my_seg_yellow(seg_yellow), my_seg_blue(seg_blue), my_seg_red(seg_red), my_seg_green(seg_green), my_cam(&cam)
    {}
};



//TODO: Morphology operation being applied, so that spurious yellow pixels dont come
//Think of a way to remove that
void FeatureDetection::getBlobs(CamCapture &cam)
{
    parallel_for( blocked_range2d<size_t>(0, IMAGE_WIDTH, 16, 0, IMAGE_HEIGHT, 32),     
                  SegmentImages(seg_yellow,seg_blue,seg_red,seg_green,cam) );
    // IplConvKernel *morphkernel = cvCreateStructuringElementEx(3,3,0,0,CV_SHAPE_RECT);
    // cvMorphologyEx(seg_yellow, seg_yellow, NULL, morphkernel, CV_MOP_OPEN, 1);

    getInGreen(cam);
    // cvLabel(seg_yellow, labelImg, blobs_yellow);
    // cvLabel(seg_blue, labelImg, blobs_blue);
    cvLabel(seg_red, labelImg, blobs_red);
    //getInGreen should have been called before this
    // cvLabel(seg_black, labelImg_small, blobs_black);
    // cvFilterByArea(blobs_yellow, 100, 1000000);
    // cvFilterByArea(blobs_blue, 100, 1000000);
    cvFilterByArea(blobs_red, 10, 1000000);
    // printf("After filter\n");
    //minimum obstacle area defined here
    // cvFilterByArea(blobs_black, 50, 10000);
    int i = 0;

    // i= 0;
    // for (CvBlobs::const_iterator it=blobs_black.begin(); it!=blobs_black.end(); ++it, i++)
    // {
    //   std::cout << "Black #" << i << ": Area=" << it->second->area << ", Centroid=(" << it->second->centroid.x << ", " << it->second->centroid.y << ")" << std::endl;
    // }

    // i= 0;
    // for (CvBlobs::const_iterator it=blobs_yellow.begin(); it!=blobs_yellow.end(); ++it, i++)
    // {
    //   std::cout << "Yellow #" << i << ": Area=" << it->second->area << ", Centroid=(" << it->second->centroid.x << ", " << it->second->centroid.y << ")" << std::endl;
    // }
}



//TODO: if opposite color blob found nearby, then it is not a goalpost
void FeatureDetection::getGoals(CamCapture &cam, HeadMotor &hm)
{
    int nGoals = 0;
    IplImage* histogram_y = cvCreateImage(cvSize(IMAGE_WIDTH,1),8,1);   //image colours (histogram_y)
    // IplImage* histogram_x = cvCreateImage(cvSize(1, IMAGE_HEIGHT),8,1);   //image colours (histogram_y)
    int max_x=0,max_y = 0;
    int threshold;  //threshold for peaks

    for(int x=0;x<IMAGE_WIDTH;++x)
    {
        cvSetImageROI(seg_yellow,cvRect(x,0,1,IMAGE_HEIGHT));
        int n = cvCountNonZero(seg_yellow);
        cvResetImageROI(seg_yellow);
        if(max_y<=n)
            max_y=n;
        returnPixel1C(histogram_y,x,0) = n;
    }

    cvLabel(seg_yellow, labelImg, blobs_yellow);
    cvFilterByArea(blobs_yellow, 100, 1000000);
    cvNamedWindow("YELLOW");
    cvMoveWindow("YELLOW",400,600);
    cvShowImage("YELLOW",seg_yellow);
    int max = IMAGE_HEIGHT;
    for (CvBlobs::const_iterator it=blobs_yellow.begin(); it!=blobs_yellow.end(); ++it)
    {
        if(it->second->miny <= max)
            max = it->second->miny;
    } 

    threshold = max_y/5;
    // cvSmooth(histogram_x,histogram_x);
    cvSmooth(histogram_y,histogram_y);
    int gpx1 = 0,gpx2 = 0;                  //x COORDINATE FOR GOAL POSTS
    int pixels_at_gpx1 = 0,pixels_at_gpx2 = 0;
    for(int i=0;i<IMAGE_WIDTH;++i)
    {
        if(returnPixel1C(histogram_y,i,0)>threshold)
        {
            int peak_x = 0;
            int pixels_at_x = 0;                        
            while(1)
            {
                if(i>=IMAGE_WIDTH)
                    break;
                if(returnPixel1C(histogram_y,i,0)<threshold)
                    break;
                if(returnPixel1C(histogram_y,i,0)>pixels_at_x)
                {
                    peak_x = i;
                    pixels_at_x = returnPixel1C(histogram_y,i,0);
                }
                ++i;
            }
            if(pixels_at_x >= pixels_at_gpx1)     //COMPARING COLOR SO THAT ONLY THE TWO PEAKS WITH MAX WHITE REGION ARE CONSIDERED AS GOAL POSTS
            {
                gpx2 = gpx1;
                pixels_at_gpx2 = pixels_at_gpx1;
                gpx1 = peak_x;
                pixels_at_gpx1 = pixels_at_x;
            }

            else if (pixels_at_x < pixels_at_gpx1 && pixels_at_x >=pixels_at_gpx2)
            {
                gpx2 = peak_x;
                pixels_at_gpx2 = pixels_at_x;
            }
        }
    }
    cvSetImageROI(seg_yellow,cvRect(0,max-10,IMAGE_WIDTH,20));
    cvZero(seg_yellow);
    cvResetImageROI(seg_yellow);
    // cvLine(seg_yellow,cvPoint(0,max),cvPoint(IMAGE_WIDTH,max),cvScalar(255,0,0));
    // cvErode(seg_yellow,seg_yellow);
    cvDilate(seg_yellow,seg_yellow);
    cvLabel(seg_yellow, labelImg, blobs_yellow);
    cvFilterByArea(blobs_yellow, 200, 1000000);
    // cout<<"height = "<<seg_yellow->height<<"\n\n\n \n\n";
    CvPoint gp = cvPoint(0,0);

    for (CvBlobs::const_iterator it=blobs_yellow.begin(); it!=blobs_yellow.end(); ++it)
    {
        if(((it->second->maxy - it->second->miny)/(it->second->maxx - it->second->minx)) >= 2)
        {
            //Check for closeby blue
            bool blueFlag = false;
            for (CvBlobs::const_iterator it2=blobs_blue.begin(); it2!=blobs_blue.end(); ++it2)
            {
                if(getDistance(it->second->centroid, it2->second->centroid) < 50.0)
                {
                    blueFlag = true;
                    break;
                }
            }
            if(blueFlag==true)
                continue;

            //Check if on image edge
            if(isOnImageEdge((it->second->maxx + it->second->minx)/2, it->second->maxy)==true)
                continue;

            if(it->second ->maxx >= gpx1 && it->second ->minx <= gpx1)  //MATCHING BLOBS TO GPX1 AND GPX2
                gp = cvPoint(gpx1,it->second->maxy);
            if(it->second ->maxx >= gpx2 && it->second ->minx <= gpx2)
                gp = cvPoint(gpx2,it->second->maxy);
            #ifndef ALL_PRINTING_OFF
            printf("Found GPY\n");
            #endif
            #ifdef PLOT_LANDMARKS
                CvScalar color = {255,255,0};
                #ifndef ALL_PRINTING_OFF
                printf("gp.x :%d\tgp.y :%d\n\n\n\n", gp.x*2,gp.y*2);
                #endif
                cvCircle(cam.rgbimg, cvPoint(gp.x*2,gp.y*2), 2, color, 2);
            #endif
            findReal(gp.x,gp.y, templ[tempnLand].distance, templ[tempnLand].angle, hm);
            CvFont font;
            cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.3, 0.3, 0, 1, 8);
            char A[100] = "  DISTANCE : ";
            char B[100] = "  ANGLE : ";
            ostringstream s1;
            s1<<templ[tempnLand].distance;
            const std::string tmp1 = s1.str();
            const char* cs1 = tmp1.c_str();
            strcat(A,cs1);
            ostringstream s2;
            s2<<templ[tempnLand].angle;
            const std::string tmp2 = s2.str();
            const char* cs2 = tmp2.c_str();
            strcat(B,cs2);

            cvPutText(cam.rgbimg,A,cvPoint(gp.x*2,gp.y*2),&font,cvScalar(255,255,255));
            cvPutText(cam.rgbimg,B,cvPoint(gp.x*2,gp.y*2+10),&font,cvScalar(255,255,255));

            templ[tempnLand].type = LAND_GPY;
            tempnLand++;
            nGoals++;
            if(nGoals>=2) break;
        }
    }
    nGoals = 0;
    for (CvBlobs::const_iterator it=blobs_blue.begin(); it!=blobs_blue.end(); ++it)
    {
        if(((it->second->maxy - it->second->miny)/(it->second->maxx - it->second->minx)) >= 2)
        {
            //Check for closeby yellow
            bool yellowFlag = false;
            for (CvBlobs::const_iterator it2=blobs_yellow.begin(); it2!=blobs_yellow.end(); ++it2)
            {
                if(getDistance(it->second->centroid, it2->second->centroid) < 50.0)
                {
                    yellowFlag = true;
                    break;
                }
            }
            if(yellowFlag==true)
                continue;

            //Check if on image edge
            if(isOnImageEdge((it->second->maxx + it->second->minx)/2, it->second->maxy)==true)
                continue;

            #ifndef ALL_PRINTING_OFF
            printf("Found GPB\n");
            #endif
            findReal((it->second->maxx + it->second->minx)/2, it->second->maxy, templ[tempnLand].distance, templ[tempnLand].angle, hm);
            #ifndef ALL_PRINTING_OFF
            printf("GPB distance %lf angle %lf\n", templ[tempnLand].distance, templ[tempnLand].angle);
            #endif
            templ[tempnLand].type = LAND_GPB;
            tempnLand++;
            nGoals++;
            if(nGoals>=2) break;
        }
    }
    cvReleaseImage(&histogram_y);
}



//Bug fix: earlier, x of blobs wasn't being checked, so yby/byb pattern with very different x was also getting detected
void FeatureDetection::getLPs(CamCapture &cam, HeadMotor &hm)
{
    int breakflag = 0;
    for (CvBlobs::const_iterator it=blobs_yellow.begin(); it!=blobs_yellow.end(); ++it)
    {
        if(((it->second->maxy - it->second->miny)/(it->second->maxx - it->second->minx)) > 4) continue; //If not goal post
        //Search for blue blob
        for (CvBlobs::const_iterator it2=blobs_blue.begin(); it2!=blobs_blue.end(); ++it2)
        {
            if(((it2->second->maxy - it2->second->miny)/(it2->second->maxx - it2->second->minx)) > 4) continue; //If not goal post
            if((((int)it2->second->miny - (int)it->second->maxy > 20)||((int)it2->second->miny - (int)it->second->maxy < -20))||(((int)it2->second->centroid.x - (int)it->second->centroid.x > 20)||((int)it2->second->centroid.x - (int)it->second->centroid.x < -20)))    continue;
            //Search for yellow blob
            for (CvBlobs::const_iterator it3=blobs_yellow.begin(); it3!=blobs_yellow.end(); ++it3)
            {
                if(((it3->second->maxy - it3->second->miny)/(it3->second->maxx - it3->second->minx)) > 4) continue; //If not goal post
                if((((int)it3->second->miny - (int)it2->second->maxy > 20)||((int)it3->second->miny - (int)it2->second->maxy < -20))||(((int)it3->second->centroid.x - (int)it2->second->centroid.x > 20)||((int)it3->second->centroid.x - (int)it2->second->centroid.x < -20)))    continue;
                //Found landmark
                //Check if on edge
                if(isOnImageEdge((it3->second->maxx + it3->second->minx)/2, it3->second->maxy)==true)
                    continue;
                #ifndef ALL_PRINTING_OFF
                printf("Found YBY\n");
                #endif
                #ifdef PLOT_LANDMARKS
                CvScalar color = {255,0,0};
                cvCircle(cam.rgbimg, cvPoint((it3->second->maxx + it3->second->minx), it3->second->maxy*2), 2, color, 2);
                #endif
                findReal((it3->second->maxx + it3->second->minx)/2, it3->second->maxy, templ[tempnLand].distance, templ[tempnLand].angle, hm);
                templ[tempnLand].type = LAND_YBY;
                tempnLand++;
                breakflag = 1;
                break;  
            }
            if(breakflag)
                break;
        }
        if(breakflag)
            break;
    }

    breakflag = 0;
    for (CvBlobs::const_iterator it=blobs_blue.begin(); it!=blobs_blue.end(); ++it)
    {
        if(((it->second->maxy - it->second->miny)/(it->second->maxx - it->second->minx)) > 4) continue; //If not goal post
        //Search for yellow blob
        for (CvBlobs::const_iterator it2=blobs_yellow.begin(); it2!=blobs_yellow.end(); ++it2)
        {
            if(((it2->second->maxy - it2->second->miny)/(it2->second->maxx - it2->second->minx)) > 4) continue; //If not goal post
            if((((int)it2->second->miny - (int)it->second->maxy > 20)||((int)it2->second->miny - (int)it->second->maxy < -20))||(((int)it2->second->centroid.x - (int)it->second->centroid.x > 20)||((int)it2->second->centroid.x - (int)it->second->centroid.x < -20)))    continue;
            //Search for blue blob
            for (CvBlobs::const_iterator it3=blobs_blue.begin(); it3!=blobs_blue.end(); ++it3)
            {
                if(((it3->second->maxy - it3->second->miny)/(it3->second->maxx - it3->second->minx)) > 4) continue; //If not goal post
                if((((int)it3->second->miny - (int)it2->second->maxy > 20)||((int)it3->second->miny - (int)it2->second->maxy < -20))||(((int)it3->second->centroid.x - (int)it2->second->centroid.x > 20)||((int)it3->second->centroid.x - (int)it2->second->centroid.x < -20)))    continue;
                //Found landmark
                //Check if on edge
                if(isOnImageEdge((it3->second->maxx + it3->second->minx)/2, it3->second->maxy)==true)
                    continue;
                #ifndef ALL_PRINTING_OFF
                printf("Found BYB\n");
                #endif
                #ifdef PLOT_LANDMARKS
                CvScalar color = {255,0,0};
                cvCircle(cam.rgbimg, cvPoint((it3->second->maxx + it3->second->minx), it3->second->maxy*2), 2, color, 2);
                #endif
                findReal((it3->second->maxx + it3->second->minx)/2, it3->second->maxy, templ[tempnLand].distance, templ[tempnLand].angle, hm);
                templ[tempnLand].type = LAND_BYB;
                tempnLand++;
                breakflag = 1;
                break;
            }
            if(breakflag)
                break;
        }
        if(breakflag)
            break;
    }
    return;
}


bool FeatureDetection::getObstacles(CamCapture &cam, HeadMotor &hm)
{
    bool obstacleOnEdge = false;
    for (CvBlobs::const_iterator it=blobs_black.begin(); it!=blobs_black.end(); ++it)
    {
        if(((it->second->maxy - it->second->miny)/(it->second->maxx - it->second->minx)) >= 1)
        {
            
            //Check if on image edge
            if(isOnImageEdgeObstacle((it->second->maxx + it->second->minx)*2, it->second->maxy*4)==true)
              continue;
            #ifndef ALL_PRINTING_OFF
            printf("Found Obstacle\n");
            #endif
            #ifdef PLOT_LANDMARKS
                CvScalar color = {255,255,255};
                cvCircle(cam.rgbimg, cvPoint((it->second->maxx + it->second->minx)*4, it->second->maxy*8), 2, color, 2);
            #endif
            findReal((it->second->maxx + it->second->minx)*2, it->second->maxy*4, tempo[tempnObstacle].distance, tempo[tempnObstacle].angle, hm);
            tempnObstacle++;
        }
    }
    return obstacleOnEdge;
}

struct Point
{
    int x;
    int y;
};

struct Node
{
    int degree;
    Point n[5]; //5 Maximum connections?
};

struct NodeXY
{
    int degree;
    Point n[5]; //5 Maximum connections?
    Point p;
};


bool isPerpendicular(CvPoint* line1, CvPoint* line2, double angle_threshold = CV_PI*50.0/180.0)
{
    double angle1 = atan2(line1[1].y - line1[0].y, line1[1].x - line1[0].x);
    double angle2 = atan2(line2[1].y - line2[0].y, line2[1].x - line2[0].x);
    double diff = fabs(angle1 - angle2);
    if((diff > angle_threshold)&&(diff < CV_PI - angle_threshold))
        return true;
    else
        return false;
}


double dist2(CvPoint v, CvPoint w)
{
    return (v.x - w.x)*(v.x - w.x) + (v.y - w.y)*(v.y - w.y);
}

double distToSegmentSquared(CvPoint v, CvPoint w, CvPoint p)
{
    double l2 = dist2(v, w);
    if(l2==0.0) return dist2(p, v);
    double t = ((p.x - v.x) * (w.x - v.x) + (p.y - v.y) * (w.y - v.y)) / l2;
    if (t < 0) return dist2(p, v);
    if (t > 1) return dist2(p, w);
    CvPoint temp;
    temp.x = v.x + t * (w.x - v.x);
    temp.y = v.y + t * (w.y - v.y);
    return dist2(p, temp);
}


double distToSegment(CvPoint* line, CvPoint point)
{
    return sqrt(distToSegmentSquared(line[0], line[1], point));
}


//checks if an end point of line1 is close 
int isClose(CvPoint* line1, CvPoint* line2)
{
    if(distToSegment(line2, line1[0]) < 3.0)
        return 0;
    if(distToSegment(line2, line1[1]) < 3.0)
        return 1;
    return -1;
}


//Fine tuning of constants might be required
void FeatureDetection::getCorners(CamCapture &cam, HeadMotor &hm)
{
    // Assumption: getInGreen has been called, so 
    // seg_white_count stores white in green with densities
    // seg_white stores white in green with 255

    IplImage* show_image = cvCreateImage(cvSize(IMAGE_WIDTH*2, IMAGE_HEIGHT*2), 8, 1);
    IplImage* src = seg_white_count;
    IplImage* dst = src;
    IplImage* color_dst = cvCreateImage( cvSize(src->width*8, src->height*8), 8, 1 );
    CvMemStorage* storage = cvCreateMemStorage(0);
    CvSeq* lines = 0;
    int i;
    cvResize(dst, color_dst);
    lines = cvHoughLines2( dst, storage, CV_HOUGH_PROBABILISTIC, 2, CV_PI/18, 35, 10, 1 );

    std::vector<CvPoint> Tlist;
    Tlist.clear();
    //Got lines. now need to check for roughly perpendicular lines that are intersecting
    //or will intersect when extended
    //line1 is the line whose end point is on T
    for (int i = 0; i < lines->total; ++i)
    {
        CvPoint* line1 = (CvPoint*)cvGetSeqElem(lines,i);
        for (int j = 0; j < lines->total; ++j)
        {
            if(i==j) continue;
            CvPoint* line2 = (CvPoint*)cvGetSeqElem(lines,j);   
            //checking if perpendicular
            if(isPerpendicular(line1, line2))
            {
                int temp = isClose(line1, line2);
                if(temp >= 0)
                {
                    // printf("%d is close to %d\n", i, j);
                    //line1[temp] stores where T intersection is.
                    //However, it could also be an L intersection
                    //Checking if it is L intersection, if so skipping
                    if(dist2(line1[temp], line2[0]) < 9.0 || dist2(line1[temp], line2[1]) < 9.0)
                    {
                        // printf("L removed\n");
                        continue;
                    }
                    Tlist.push_back(line1[temp]);
                }
            }
        }
    }

    // TODO Postprocessing:
    // 1. Remove duplicates
    // 2. Remove Ts close to goalposts
    // 3. If more than 2 remain, do something?

    //Removing duplicates
    #define intabs(x) ((x)>0?(x):-(x))
    std::vector<CvPoint> Tlist_noDuplicates;
    for (int i = 0; i < Tlist.size(); ++i)
    {
        bool flag = true;
        for (int j = i+1; j < Tlist.size(); ++j)
        {
            if(intabs(Tlist[i].x - Tlist[j].x) < 10 && intabs(Tlist[i].y - Tlist[j].y) < 10)
            {
                flag = false;
                break;
            }
        }
        if(flag)
        {
            Tlist_noDuplicates.push_back(Tlist[i]);
        }
    }
    #undef intabs

    //Getting list in real terms
    std::vector<Landmark> tReal;
    for (int i = 0; i < Tlist_noDuplicates.size(); ++i)
    {
        Landmark l;
        findReal(Tlist_noDuplicates[i].x*4, Tlist_noDuplicates[i].y*4, l.distance, l.angle, hm);
        tReal.push_back(l);
    }

    //Comparing with goalposts
    for (int i = 0; i < tempnLand; ++i)
    {
        if(templ[i].type == LAND_GPB || templ[i].type == LAND_GPY)
        {
            for(std::vector<Landmark>::iterator it = tReal.begin(); it != tReal.end();)
            {
                if(isCloseLandmark(templ[i], *it, 40.0, 90.0))
                {
                    // printf("Deleted a T close to goalpost\n");
                    it = tReal.erase(it);
                    //could continue over here, but this is probably safer?
                    //could continue because duplicates have already been removed
                }
                else
                {
                    ++it;
                }
            }
        }
    }

    //Ts are now stored in tReal. These need to be added to templ

    for (int i = 0; i < tReal.size(); ++i)
    {
        templ[tempnLand].distance = tReal[i].distance;
        templ[tempnLand].angle = tReal[i].angle;
        templ[tempnLand].type = LAND_T;
        tempnLand++;
    }

#ifdef PLOT_HOUGH_LINES
    for (int i = 0; i < Tlist_noDuplicates.size(); ++i)
    {
        cvCircle(color_dst, cvPoint(Tlist_noDuplicates[i].x*8, Tlist_noDuplicates[i].y*8), 10, cvScalar(255),10);
    }
    for( i = 0; i < lines->total; i++ )
    {
        CvPoint* line = (CvPoint*)cvGetSeqElem(lines,i);
        line[0].x *= 8.;
        line[1].x *= 8.;
        line[0].y *= 8.;
        line[1].y *= 8.;
        cvLine( color_dst, line[0], line[1], cvScalar(127.0), 3, 8 );
        cvCircle(color_dst, line[0], 5, cvScalar(64),5);
    }

    cvNamedWindow( "Hough", 1 );
    cvShowImage( "Hough", color_dst );
#endif

    // Clear the memory storage which was used before
    cvClearMemStorage(storage);

    // Release memory
    cvReleaseMemStorage(&storage);

 

    cvReleaseImage(&color_dst);
    cvReleaseImage(&show_image);
}



void FeatureDetection::getInGreen(CamCapture &cam)
{
    // Make reduced image
    int rowsum[IMAGE_HEIGHT/4];
    IplImage* binary_image = cvCreateImage(cvSize(IMAGE_WIDTH/4, IMAGE_HEIGHT/4), 8, 1);
    IplImage* prob_image = cvCreateImage(cvSize(IMAGE_WIDTH/4, IMAGE_HEIGHT/4), 8, 1);
    cvZero(binary_image);
    cvZero(prob_image);

    cvZero(seg_white);
    cvZero(seg_black);
    cvZero(seg_green);
    cvZero(seg_white_count);
 //New Begin
    const int tmin = 30;
    const int trow = IMAGE_WIDTH*4.5;
    const int tsum = 36*18;//30*18;//36*18;
    for(int y = 0; y < IMAGE_HEIGHT/4; y++)
    {
        rowsum[y] = 0;
        for(int x = 0; x < IMAGE_WIDTH/4; x++)
        {
            int wcount = 0;
            int gcount = 0;
            int bcount = 0;
            int rcount = 0;
            for(int xx = 0; xx < 4; xx++)
            for(int yy=0; yy < 4; yy++)
            {
                int tx = x*4 + xx;
                int ty = y*4 + yy;
                if(cam.isGreen_small(tx, ty))
                    gcount++;
                if(cam.isWhite_small(tx, ty))
                 wcount++;
                if(cam.isBlack_small(tx, ty))
                 bcount++;
                if(cam.isRed_small(tx, ty))
                 rcount++;
            }
            returnPixel1C(prob_image, x, y) = wcount + bcount + rcount*16 + gcount*8;
            if(gcount > 4)
                returnPixel1C(prob_image, x, y) += gcount*2;
            if(wcount)
            {
                if(wcount==16)
                    returnPixel1C(seg_white_count, x, y) = 255;
                else    
                    returnPixel1C(seg_white_count, x, y) = (wcount*16)%256;
                if(wcount>4)
                    returnPixel1C(seg_white, x, y) = 255;
            }
            if(gcount>4)
            {
                returnPixel1C(seg_green, x, y) = 255;
            }
            if(bcount > 4)
            {
                returnPixel1C(seg_black, x, y) = 255;
            }
            if(rcount)
            {
                returnPixel1C(seg_red, x, y) = 255;
            }
            else
            {
    //          returnPixel1C(seg_black, x, y) = 255;
            }
            rowsum[y] += pixelColor1C(prob_image, x, y);
        }
    }
    //New End

    for(int y = 0; y < IMAGE_HEIGHT/4; y++)
    {
        for(int x = 0; x < IMAGE_WIDTH/4; x++)
        {
            //check if point inside fisheye image
            if((x-IMAGE_WIDTH/8)*(x-IMAGE_WIDTH/8) + (-y+IMAGE_HEIGHT/8)*(-y+IMAGE_HEIGHT/8) > 120*120/16)  //radius of image is 120 pixels
                continue;
            //for each pixel, first check tmin
            if(pixelColor1C(prob_image, x, y) < tmin)
            {
                returnPixel1C(binary_image, x, y) = 0;
                continue;
            }

            //Now, check row sum is above certain threshold
            if(rowsum[y] > trow)
            {
                returnPixel1C(binary_image, x, y) = 255;
                continue;   
            }

            //Now do the expensive test
            int sum = 0;
            for(int i = -4; i <5; i++)
            {
                for(int j = -4; j < 5; j++)
                {
                    if((x + i > 0)&&(y + j > 0)&&(x + i < IMAGE_WIDTH/4)&&(y + j < IMAGE_HEIGHT/4))
                    {
                        sum = sum + pixelColor1C(prob_image, x +i, y+j);
                    }
                }
            }

            if(sum >= tsum)
            {
                returnPixel1C(binary_image, x, y) = 255;
                continue;
            }
            else
            {
                returnPixel1C(binary_image, x, y) = 0;
                continue;
            }
        }
    }


    IplImage* histogram = cvCreateImage(cvSize(IMAGE_WIDTH/4, 1), 8, 1);

    for(int x = 0; x < IMAGE_WIDTH/4; x++)
    {
        int y=0, count = 0;
        for(y = 0; y < IMAGE_HEIGHT/4; y++)
        {
            if(pixelColor1C(binary_image, x, y))
            {
                count++;
                if(count == 4)
                    break;
            }
            else
                count = 0;
        }
        if(count == 4)
            returnPixel1C(histogram, x, 0) = y;
        else
            returnPixel1C(histogram, x, 0) = IMAGE_HEIGHT/4 - 1;
    }


    cvSmooth(histogram, histogram, CV_GAUSSIAN, 3);
    cvSmooth(histogram, histogram, CV_MEDIAN, 15);

    // for(int x = 0; x < IMAGE_WIDTH/4; x++)
    // {
    //     // for(int y = 0; y < returnPixel1C(histogram, x, 0); y++)
    //     // {
    //         // returnPixel1C(seg_white, x, y) = 0;
    //         // returnPixel1C(seg_black, x, y) = 0;
    //         // returnPixel1C(seg_white_count, x, y) = 0;
            
    //     // }

      
    //     // Uncomment this to see histogram boundary in seg_black
    //     // Caution: uncommenting this will cause segmentation faults in some cases! Uncomment only when testing
    //     int temp =  returnPixel1C(histogram, x, 0);
    //     if(temp > 1)
    //       returnPixel1C(seg_black, x,(int) (temp-1)) = 127;
    // }

    // cvShowImage("boundary", seg_black);
    
    // for(int x = 0; x < IMAGE_WIDTH; x++)
    // {
    //     for(int y = 0; y < returnPixel1C(histogram, x/4, 0)*4; y++)
    //     {
    //         returnPixel1C(seg_red, x, y) = 0;
    //     }
    // }
    cvNamedWindow("RED");
    cvMoveWindow("RED",50,600);
    cvShowImage("RED", seg_red);

    cvReleaseImage(&histogram);
    // convex corner algo?

    // IplImage* show_image = cvCreateImage(cvSize(IMAGE_WIDTH*2, IMAGE_HEIGHT*2), 8, 1);
    // cvResize(seg_black, show_image);
    // cvShowImage("Black", show_image);
    
    
    // cvResize(seg_white, show_image);
    // cvShowImage("White", show_image);
    // cvResize(seg_green, show_image);
    // cvShowImage("Green", show_image);
    // cvResize(binary_image, show_image);
    // cvShowImage("Binary", show_image);
    // cvResize(prob_image, show_image);
    // cvShowImage("Prob", show_image);
    // cvReleaseImage(&show_image);

    //Segmentation done
    cvReleaseImage(&binary_image);
    cvReleaseImage(&prob_image);

}



void FeatureDetection::getLandmarks(CamCapture &cam, HeadMotor &hm, MotionModel &mm)
{
    tempnLand = 0;
    tempnObstacle = 0;
    //getInGreenKartik(cam);
    // getInGreen(cam);
    getBlobs(cam);
    //cam is passed for plotting
    getGoals(cam, hm);

#ifndef SYMMETRIC_LANDMARKS
    getLPs(cam, hm);
#endif
    //getCorners needs to be called after getGoals since it checks for goalposts close to Ts
    // getCorners(cam, hm);
    //getObstacle returns whether some obstacle on edge was ignored. If so, counters are not increased
    // bool obstacleReturn = getObstacles(cam, hm);
    
    
/* Landmark start */
    for(std::vector<Landmark>::iterator it = l.begin(); it != l.end();)
    {
        it->counter++;
        if(it->counter > LANDMARK_PERSISTENCE)
        {
            it = l.erase(it);
        }
        else
        {
            ++it;
        }
    }
    //Now assigning templ landmarks to main Landmark vector l
    for(int i = 0; i < tempnLand; i++)
    {
        bool flag = false;

        for(std::vector<Landmark>::iterator it = l.begin(); it != l.end(); ++it)
        {
            if(isSameLandmark(templ[i], *it)==true)
            {
                *it = templ[i];
                it->counter = 0;
                flag = true;
                break;
            }
        }
        if(flag==false)
        {
            //need to add new element now
            templ[i].counter = 0;
            l.push_back(templ[i]);
        }
    }
/* Landmark end */
/* Obstacle start */
    for(std::vector<Obstacle>::iterator it = o.begin(); it != o.end();)
    {
        it->counter++;
        if(it->counter > OBSTACLE_PERSISTENCE)
        {
            it = o.erase(it);
        }
        else
        {
            ++it;       
        }
    }
    //Now assigning tempo obstacles to main Obstacle vector o
    for(int i = 0; i < tempnObstacle; i++)
    {
        bool flag = false;

        for(std::vector<Obstacle>::iterator it = o.begin(); it != o.end(); ++it)
        {
            if(isSameObstacle(tempo[i], *it)==true)
            {
                *it = tempo[i];
                it->counter = 0;
                flag = true;
                break;
            }
        }
        if(flag==false)
        {
            //need to add new element now
            tempo[i].counter = 0;
            o.push_back(tempo[i]);
        }
    }
/* Obstacle end */
    getBall(cam, hm);
    int tempx = ballX_var - IMAGE_WIDTH;
    int tempy = ballY_var - IMAGE_HEIGHT;
    /*coordinates from center of image
        Ratio=(|x|/w + |y|/h)/2
        */
    double t1 = (tempx>0?tempx:-tempx)/((double)IMAGE_WIDTH);
    double t2 = (tempy>0?tempy:-tempy)/((double)IMAGE_HEIGHT);
    ballRatio = t1>t2? t1: t2;
    if(ballFound_var==false)
        ballRatio = 99999.0;
    // if(ballFound_var)
    //     printf("THERE IS A BALL!!!\n");

    #ifndef ALL_PRINTING_OFF
    for(std::vector<Landmark>::iterator it = l.begin(); it != l.end(); ++it)
    {
        #ifndef ALL_PRINTING_OFF
        printf("Type: %d Distance: %lf Angle: %lf Counter: %d\n",it->type, it->distance, it->angle, it->counter);
        #endif    
    }

    for(std::vector<Obstacle>::iterator it = o.begin(); it != o.end(); ++it)
    {
        #ifndef ALL_PRINTING_OFF
        printf("Obstacle Distance: %lf Angle: %lf Counter: %d\n",it->distance, it->angle, it->counter);
        #endif
    }
    #endif
}



void FeatureDetection::getBall(CamCapture &cam, HeadMotor &hm)
{
    int area = 0;
    CvBlobs::const_iterator it2;
    
    //Select blob of largest area
    for (CvBlobs::const_iterator it=blobs_red.begin(); it!=blobs_red.end(); ++it)
    {
        if((it->second->maxx - it->second->minx) == 0) continue;
        if((it->second->maxy - it->second->miny)/(it->second->maxx - it->second->minx)>=2) continue;
        if(it->second->area > area)
        {
            area = it->second->area;
            it2 = it;
        }
    }
    if(area==0) //Meaning no ball found
    {
        ballFound_var = false;
    }
    else
    {
        ballX_var = it2->second->centroid.x*2;
        ballY_var = it2->second->centroid.y*2;
        ballFound_var = true;
        findReal(it2->second->centroid.x, it2->second->centroid.y, ball.r, ball.theta, hm);
        // ball.theta +=10;
    }
    CvFont font;
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.3, 0.3, 0, 1, 8);
    char A[100] = "  DISTANCE : ";
    char B[100] = "  ANGLE : ";
    ostringstream s1;
    s1<<ball.r;
    const std::string tmp1 = s1.str();
    const char* cs1 = tmp1.c_str();
    strcat(A,cs1);
    ostringstream s2;
    s2<<ball.theta;
    const std::string tmp2 = s2.str();
    const char* cs2 = tmp2.c_str();
    strcat(B,cs2);
    cvCircle(cam.rgbimg, cvPoint(ballX_var,ballY_var), 2, cvScalar(255,255,0), 2);
    cvPutText(cam.rgbimg,A,cvPoint(ballX_var,ballY_var),&font,cvScalar(255,255,255));
    cvPutText(cam.rgbimg,B,cvPoint(ballX_var,ballY_var + 10),&font,cvScalar(255,255,255));


    /* Simple Begin */
    // int bx = 0, by = 0, count = 0;
    // for(int x = 0; x < IMAGE_WIDTH; x++)
    // {
    //  for(int y = 0; y < IMAGE_HEIGHT; y++)
    //  {
    //      if(cam.isRed_small(x,y))
    //      {
    //          count++;
    //          bx += x;
    //          by += y;
    //      }
    //  }
    // }

    // printf("Count = %d\n", count);
    // if(count > 50)
    // {
    //  bx = bx/count;
    //  by = by/count;
    //  ballX_var = bx*2;
    //  ballY_var = by*2;
    //  ballFound_var = true;
    //  findReal(bx, by, ball.r, ball.theta, hm);
    // }
    // else
    // {
    //  ballFound_var = false;
    // }
    /* Simple End */
}

void FeatureDetection::updatePacket(FeaturesPacket &fp)
{
    fp.n_obstacles = o.size();
    for(int i = 0; i < o.size(); i++)
    {
        fp.obstacles[i].r = o[i].distance;
        fp.obstacles[i].theta = o[i].angle;
    }

    fp.n_landmarks = l.size();
    for(int i = 0; i < l.size(); i++)
    {
        fp.landmarks[i] = l[i];
    }

    fp.ball = ball;
}
