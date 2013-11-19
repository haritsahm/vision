/*! \file example2s_XM_linux.cpp
    \brief Example file for CMT level 2 serial branch.
    
    This file contains an example of the CMT level 2 serial interface.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>

#include "cmtdef.h"
#include "xsens_time.h"
#include "xsens_list.h"
#include "cmtscan.h"
#include "cmt3.h"
#include "imu.h"

using namespace xsens;

#define EXIT_ERROR(loc) {printf("Error %d occurred during " loc ": %s\n", serial.getLastResult(), xsensResultText(serial.getLastResult())); return false; }

bool Imu::init()
{
    if(initialized)
        return true;
    XsensResultValue res;
    List<CmtPortInfo> portInfo;
    unsigned long portCount = 0;
    int mtCount;
    
    printf("Scanning for connected Xsens devices...");
    xsens::cmtScanPorts(portInfo);
    portCount = portInfo.length();
    printf("done\n");

    if (portCount == 0) {
        printf("No MotionTrackers found\n\n");
        return false;
    }

    for(int i = 0; i < (int)portCount; i++) {   
        printf("Using COM port %s at ", portInfo[i].m_portName);
        
        switch (portInfo[i].m_baudrate) {
        case B9600  : printf("9k6");   break;
        case B19200 : printf("19k2");  break;
        case B38400 : printf("38k4");  break;
        case B57600 : printf("57k6");  break;
        case B115200: printf("115k2"); break;
        case B230400: printf("230k4"); break;
        case B460800: printf("460k8"); break;
        case B921600: printf("921k6"); break;
        default: printf("0x%u", portInfo[i].m_baudrate);
        }
        printf(" baud\n\n");
    }

    printf("Opening ports...");
    //open the port which the device is connected to and connect at the device's baudrate.

    // char portname[32];
    // printf("Enter the port name you wish to connect to (default: /dev/ttyUSB0): ");
    // scanf("%s", portname);

    //     if (strlen(portname) == 0)
    //             sprintf(portname, "/dev/ttyUSB0");

    if (serial.open(portInfo[0].m_portName, B115200) != XRV_OK)
        EXIT_ERROR("open");
    
    msg.setMessageId(CMT_MID_GOTOCONFIG);
    printf("Putting MT in config mode\n");
    if (serial.writeMessage(&msg))
        EXIT_ERROR("goto config");
    if (serial.waitForMessage(&reply, CMT_MID_GOTOCONFIGACK, 0,  1) != XRV_OK)
        EXIT_ERROR("goto config ack");
    printf("MT now in config mode\n");

    // msg.setMessageId(CMT_MID_SETPERIOD);
    // msg.setDataShort(1152);
    // if (serial.writeMessage(&msg))
    //     EXIT_ERROR("set period");
    // if (serial.waitForMessage(&reply, CMT_MID_SETPERIODACK, 0,  1) != XRV_OK)
    //     EXIT_ERROR("set period ack");
    // printf("Period is now set to 100Hz\n");

    msg.setMessageId(CMT_MID_SETOUTPUTMODE);
    msg.setDataShort(CMT_OUTPUTMODE_ORIENT);
    if (serial.writeMessage(&msg))
        EXIT_ERROR("set output mode");
    if (serial.waitForMessage(&reply, CMT_MID_SETOUTPUTMODEACK, 0,  1) != XRV_OK)
        EXIT_ERROR("set output mode ack");
    printf("Output mode is now set to orientation\n");

    msg.setMessageId(CMT_MID_SETOUTPUTSETTINGS);
    msg.setDataLong(CMT_OUTPUTSETTINGS_ORIENTMODE_EULER | CMT_OUTPUTSETTINGS_TIMESTAMP_SAMPLECNT);
    if (serial.writeMessage(&msg))
        EXIT_ERROR("set output settings");
    if (serial.waitForMessage(&reply, CMT_MID_SETOUTPUTSETTINGSACK, 0,  1) != XRV_OK)
        EXIT_ERROR("set output settings ack");
    printf("Output settings now set to euler + timestamp\n");

    msg.setMessageId(CMT_MID_GOTOMEASUREMENT);
    msg.resizeData(0);
    if (serial.writeMessage(&msg))
        EXIT_ERROR("goto measurement");
    if (serial.waitForMessage(&reply, CMT_MID_GOTOMEASUREMENTACK, 0,  1) != XRV_OK)
        EXIT_ERROR("goto measurement ack");
    printf("Now in measurement mode, Time of day: %u\n", getTimeOfDay());


    initialized = true;
    return true;
}

void Imu::update()
{
    if(!initialized)
        return;
    if (serial.waitForMessage(&reply, 0, 0, 1) != XRV_OK)
    {
        printf("Error reading from IMU\n");
        return;
    }

     msgCount++;
     // printf("%3ld  SC: %hu  TS: %u  Roll %+6.1f  Pitch %+6.1f  Yaw %+6.1f         \r",
     //     msgCount,
     //     reply.getDataShort(3*4),           // sample counter
     //     getTimeOfDay(),                     // timestamp
     //     (double) reply.getDataFloat(0*4),  // roll
     //     (double) reply.getDataFloat(1*4),  // pitch
     //     (double) reply.getDataFloat(2*4)   // yaw
     //     );
    roll = reply.getDataFloat(0*4);
    pitch = reply.getDataFloat(1*4);
    yaw = reply.getDataFloat(2*4);
}

double Imu::return_yaw()
{
	 if(!initialized)
        return -1;
    if (serial.waitForMessage(&reply, 0, 0, 1) != XRV_OK)
    {
        printf("Error reading from IMU\n");
        return -1;
    }

    yaw = reply.getDataFloat(2*4);
    return yaw;
}
