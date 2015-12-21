#ifndef COMMON_H
#define COMMON_H

#include <pthread.h>
#include "gui.h"

extern pthread_t thread_id_gui;
extern pthread_mutex_t buffer_lock;
extern GUI gui;

#endif