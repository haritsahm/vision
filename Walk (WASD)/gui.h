#ifndef GUI_H
#define GUI_H

#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <vector>
#include <string>
#include <pthread.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>

using namespace std;
using namespace cv;

class GUI
{
public:
	string buffer;
	char direction;
	char getDirection();
	void putDirection(char dir);
	bool isEmpty();
};

#endif