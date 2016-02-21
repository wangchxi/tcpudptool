#ifndef _TTHREAD_H_
#define _TTHREAD_H_

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "../include/types.h"


//API of thread, wrap of pthread
void* tut_create_thread(void* func, void* param);



void  tut_release_thread(void* thread);


#endif

