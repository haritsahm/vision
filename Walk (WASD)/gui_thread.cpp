#include "gui_thread.h"

void* gui_thread(void*)
{
	IplImage* dir = cvCreateImage(cvSize(500, 500), 8, 3);
	CvFont font1;
	cvInitFont(&font1,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, 5.0,5.0,0,1);
	CvFont font2;
	cvInitFont(&font2,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, 1.0,1.0,0,1);
	while(1)
	{
		int c = -1;
		c = cvWaitKey(30);
		if(c == 27)
			exit(0);
		if(c == -1)
		{
			cvZero(dir);
			if(gui.isEmpty())
			{
				cvShowImage("gui", dir);
				continue;
			}
			string str;
			pthread_mutex_lock(&buffer_lock);
			str.push_back(toupper(gui.direction));
			pthread_mutex_unlock(&buffer_lock);
			cvPutText(dir, str.c_str(), cvPoint(200, 400), &font1, cvScalar(255, 255, 255));
			pthread_mutex_lock(&buffer_lock);
			cvPutText(dir, gui.buffer.c_str(), cvPoint(20, 20), &font2, cvScalar(255, 255, 255));
			pthread_mutex_unlock(&buffer_lock);
			cvShowImage("gui", dir);
			continue;
		}
		switch(c)
		{
			case 'w':
				pthread_mutex_lock(&buffer_lock);
				gui.putDirection('w');
				pthread_mutex_unlock(&buffer_lock);
				cvZero(dir);
				cvPutText(dir, "W", cvPoint(200, 400), &font1, cvScalar(255, 255, 255));
				pthread_mutex_lock(&buffer_lock);
				cvPutText(dir, gui.buffer.c_str(), cvPoint(20, 20), &font2, cvScalar(255, 255, 255));
				pthread_mutex_unlock(&buffer_lock);
				cvShowImage("gui", dir);
				break;
			case 'a':
				pthread_mutex_lock(&buffer_lock);
				gui.putDirection('a');
				pthread_mutex_unlock(&buffer_lock);
				cvZero(dir);
				cvPutText(dir, "A", cvPoint(200, 400), &font1, cvScalar(255, 255, 255));
				pthread_mutex_lock(&buffer_lock);
				cvPutText(dir, gui.buffer.c_str(), cvPoint(20, 20), &font2, cvScalar(255, 255, 255));
				pthread_mutex_unlock(&buffer_lock);
				cvShowImage("gui", dir);
				break;
			case 's':
				pthread_mutex_lock(&buffer_lock);
				gui.putDirection('s');
				pthread_mutex_unlock(&buffer_lock);
				cvZero(dir);
				cvPutText(dir, "S", cvPoint(200, 400), &font1, cvScalar(255, 255, 255));
				pthread_mutex_lock(&buffer_lock);
				cvPutText(dir, gui.buffer.c_str(), cvPoint(20, 20), &font2, cvScalar(255, 255, 255));
				pthread_mutex_unlock(&buffer_lock);
				cvShowImage("gui", dir);
				break;
			case 'd':
				pthread_mutex_lock(&buffer_lock);
				gui.putDirection('d');
				pthread_mutex_unlock(&buffer_lock);
				cvZero(dir);
				cvPutText(dir, "D", cvPoint(200, 400), &font1, cvScalar(255, 255, 255));
				pthread_mutex_lock(&buffer_lock);
				cvPutText(dir, gui.buffer.c_str(), cvPoint(20, 20), &font2, cvScalar(255, 255, 255));
				pthread_mutex_unlock(&buffer_lock);
				cvShowImage("gui", dir);
				break;
			default:
				cvShowImage("gui", dir);
		}
	}
}