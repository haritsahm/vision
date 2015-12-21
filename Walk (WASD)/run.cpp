#include "gui.h"
#include "common.h"
#include <pthread.h>
#include "gui_thread.h"
#include <unistd.h>

using namespace std;

pthread_t thread_id_gui;
pthread_mutex_t buffer_lock = PTHREAD_MUTEX_INITIALIZER;
GUI gui;

int main(int argc, char const *argv[])
{
	pthread_create(&thread_id_gui,NULL,gui_thread,NULL);
	char input;
	while(1)
	{
		usleep(500000);
		if(gui.isEmpty())
			continue;
		pthread_mutex_lock(&buffer_lock);
		input = gui.getDirection();
		pthread_mutex_unlock(&buffer_lock);
		printf("%c\n", input);
	}
	return 0;
}