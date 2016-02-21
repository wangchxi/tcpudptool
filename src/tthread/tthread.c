#include "tthread.h"



void* tut_create_thread(void* func, void* param)
{
    pthread_t *thread;
    thread = (pthread_t *)malloc(sizeof(pthread_t));

    if(thread)
    {
        pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_create(thread, &attr, (void *(*)(void *))func, param);
    }

    return thread;
}

void tut_release_thread(void* thread)
{
    if(thread)
    {
        usleep(1);
        pthread_kill(*(pthread_t*)thread, 9);
        free(thread);
    }
}


