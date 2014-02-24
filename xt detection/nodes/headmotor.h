#ifndef HEADMOTOR_H
#define HEADMOTOR_H////
//IMPORTANT: implementation of this class has changed!
//Now, only writing is done, that too only when update function is called(has to be called in loop)
//TODO: add speed control. Right now, it is random, depends on frame rate
//usage of timers may make it better
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <ftdi.h>
#include "../common/defines.h"
//Name was changed from WRITE to WRITE_FTDI as it is a global #define.
//Any way to make this local to headmotor class?
#define WRITE_FTDI ftdi_write_data(&ftdic1_camera,pack,pack[3]+4)
#define serialusb2d_camera "A4007rX0"//"A900fDpA"//"A4007rXO"//"A800d2dg"//"A900fDpz"//"A7003N1d" //"A4007sgE"//"A900fDpz"//"A700eSSZ"//"A4007sgG"//"A900fDpz"//A700eSSZ"//"A4007sgG"//"A900fDhp"// "A900fDpA"//"A900fDpA"

typedef unsigned char byte;

class HeadMotor
{
private:
        static const int NO_OF_TRIES = 20;
        static const int offsetx=210,offsety=-90;
        static const int ERROR_MOTORS = -900;
        static const int FPS = 120;
        static const int MAX_STEP = 30;
        int STEP_SIZE_17;
        int STEP_SIZE_18;        
        struct ftdi_context ftdic1_camera;
        int global_id;

        int checksum(byte*);
        void set_gp(int id,int gp);
        void set_gp(int gp);
        void set_speed(int id, int sp);
        void sync_write_gp(int id[], int gp[], int n);
        void sync_write_gp();
        void scurve(int id[],int final[], float time);
        int swrite(float thetax, float thetay, float ttime=1.0);
        float thetaX_var, thetaY_var;
        int motorX_var, motorY_var;
        int current_pos[2];
        int goal_pos[2];
        bool initPos;
        //0 is Y-axis motor, 1 is X-axis (in goal and current pos) everywhere

public:
        static const int MOTOR_SPEED = 50;
        int read_pos(int id);        //Actually reads position from motor. Should not be used in current implementation. Only for testing
        HeadMotor(bool param = true);
        int bootup_files();
        int ismoving_motor(int id = 0);        //TODO id not being used right now, both motors being checked (17 and 18)
        void speed_motor(int speed1, int speed2);
        void write_motor(float thetax, float thetay);
        int read_motor(float &thetax, float &thetay,int &motorX_var,int &motorY_var);
        int stop_motor();
        int go2state(int state);
        void update();
        inline float thetaX(){ return thetaX_var;}
        inline float thetaY(){ return thetaY_var;}
        inline int motorX(){ return motorX_var;}
        inline int motorY(){ return motorY_var;}
};



#endif