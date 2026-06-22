#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdarg.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)
//#define MAX_NUM_THREADS (10)

void *threadfunc(void *thread_param)
//void *threadfunc(void* thread_data *thread_param)
//void *threadfunc(void* arg)
{
int wait_lock, wait_unlock;
struct thread_data thread_param;
pthread_mutex_t my_mutex; 

    thread_data *thread_func_args = (thread_data *)thread_param;
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    printf("Karl, Thread is running.\n");
    wait_lock = 1000*thread_param->wait_to_obtain_ms;
    usleep(wait_lock);
    // obtain mutex
    pthread_mutex_lock(&my_mutex);
    //clock_t start_time = clock();
    //while (clock() < start_time + wait_to_release_ms);
    wait_unlock = 1000*thread_param->wait_to_release_ms;
    usleep(wait_unlock);
    // release mutex
    pthread_mutex_unlock(&my_mutex);
    printf("Karl, Thread is finished.\n");
    return thread_param;
}

bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
pthread_mutex_t my_mutex;
struct thread_data thread_func_args;

    pthread_mutex_init(&my_mutex, NULL);
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    pthread_t threadID;  // Declare a pthread thread descriptor
    int index;
    //thread_func_args.threadID = thread;
    thread_func_args.mutex = mutex;
    thread_func_args.wait_to_obtain_ms = wait_to_obtain_ms;
    thread_func_args.wait_to_release_ms = wait_to_release_ms;
    thread_func_args.thread_complete_success = false;
    index = 0;
    // Create a new thread
    printf("Karl, Creating new thread\n");
    int threadfunctreturn = pthread_create(&threadID,   // pointer to thread descriptor
                            NULL,                       // use SPECIFIC SECHED_FIFO attributes
                            threadfunc,                 // thread function entry point
                            //*(*threadfunc) (void *),
                            //void *(*threadfunc)(void*),
                            //void*(*threadfunc)(void*),                 // thread function entry point
                            //void*(*threadfunc),                 // thread function entry point
                            //thread_func_args);   // parameters to pass in
                            (void *)&(thread_func_args));   // parameters to pass in
    if (threadfunctreturn != 0) {
        printf("Error creating thread\n");
        return 1;
    }
    pthread_mutex_destroy(&my_mutex);
    // Wait for the thread to finish
    pthread_join(threadID, NULL);
    printf("Karl, whole thing has finished.\n");
    if (threadfunctreturn == threadID) return true;
    else return false;
    
} 
