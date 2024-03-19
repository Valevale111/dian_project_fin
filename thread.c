#include <stdio.h>
#include <errno.h> // Error code head file(EBUSY)
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h> // Pthread head file

typedef struct Global_Memory{ // #define new struct type var with mutex lock and data also using-time.
    pthread_mutex_t *g_mutex; // The mutex lock variable define
    unsigned int Memory[10]; // The truly Data you will deal with.
    int using_time; // Using time setup.
}GMem;

GMem *GMemory = NULL; // #define the mutex-lock & Memory variable
void *thread_func1(void *arg); // #define the thread functions
void *thread_func2(void *arg);

int main(void)
{
    pthread_t th1,th2;
    GMemory = (GMem *)malloc(sizeof(GMem)); // Allocal the storage spcae for Memory Data and time_using.
    GMemory->g_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t)); // Allocal the memory space for the mutex variable.

    printf("Mutil_Thread_Sys Starting...\n"); // Show the Starting point.
    pthread_mutex_init(GMemory->g_mutex,NULL); // Initialize the mutex.
    pthread_create(&th1,NULL,thread_func1,NULL); // Create the Thread1 & Start the thread func1.
    pthread_create(&th2,NULL,thread_func2,NULL); // Create the Thread2 & Start the thread func2.

    while(1){ // Main Processing thread.
        printf("This is the Main Processing...\n");
        sleep(1); // Delay time cost 1second.
    }

    pthread_join(th2,NULL); // wait the Thread2 end.
    pthread_join(th1,NULL); // Wait the Thread1 end.
    
    printf("System Exit.\n"); // Show the Ending point.
    return 0;
}
//为什么可能出现2-1-2-2？


void *thread_func1(void *arg) // Func1 Point address Define.(void *)
{
    time_t t_s; // Define a time_t var for storage the thread ending timestamp.
    while(1){
        pthread_mutex_lock(GMemory->g_mutex); // try to get the mutex lock var, if success, the thread is going to run and deal with the Memory Data.
        printf("********************************************************************************Func1 Locked.\n"); // Print the mutex lock var's state.
        t_s = time(NULL) + 1; // setup the ending time of the thread.
        while(time(NULL) <= t_s){ // Control the ending time.
            GMemory->Memory[1]++; // Deal with the Memory Data.
            printf("This is thread_func1 Running:Memory[1]=%d  Memory[2]=%d\n",GMemory->Memory[1],GMemory->Memory[2]); // show the result the thread just did.
            usleep(1000); // us delay for testing the thread time control.
        }
        pthread_mutex_unlock(GMemory->g_mutex); // Unlock the mutex variable so that other thread could use the mutex lock variable.
        printf("********************************************************************************Func1 Unlocked.\n"); // Print the mutex lock var's state.
    }
}

void *thread_func2(void *arg)
{
    time_t t_s;
    while(1){
        // int ret = pthread_mutex_trylock(GMemory->g_mutex); // Use the Trylock function will make the func2 to be sleeped, Use lock to make sure the Func2 thread could be called.
        // if(EBUSY == ret) continue;
        pthread_mutex_lock(GMemory->g_mutex);
        printf("********************************************************************************Func2 Locked.\n");
        t_s = time(NULL) + 1;
        while(time(NULL) <= t_s){
            GMemory->Memory[2]++;
            printf("This is thread_func2 Running:Memory[1]=%d  Memory[2]=%d\n",GMemory->Memory[1],GMemory->Memory[2]);
            usleep(1000);
        }
        pthread_mutex_unlock(GMemory->g_mutex);
        printf("********************************************************************************Func2 Unlocked.\n");
    }
}