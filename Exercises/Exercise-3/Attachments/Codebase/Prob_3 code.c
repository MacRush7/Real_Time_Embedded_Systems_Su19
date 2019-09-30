/*
* This code has been re-used and modified according to the needs of the
* problem. This code has been originally owned by Dr. Siewert and the
* code has modified on several instances. I therefore duly credit the resource
* links available in this citation document as the original work which I have
* reused and modified for completing this code.
*
* I duly credit Dr. Siewert for his original work based on which I have
* prepared my code through reuse and some modifications. Thanks.
*
* The original code as authored by Dr. Siewert can be found at the below
* link:
* http://ecee.colorado.edu/%7Eecen5623/ecen/ex/Linux/simplethread/pthread.c
*
* 
*/
/*
* INFORMATION ON RUNNING THE BELOW CODE ON R-PI 3B+
*
* To run the below modified and reused code on the R-Pi 3B+ platform,
* please run the below commands on the Command Line Interface (CLI)
* (as root for each core) for setting up the
* board after a fresh reboot:
*
* $ sudo -i // To enter the root mode
*
* $ echo userspace > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
* $ cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq > /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq
* $ cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq > /sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed
* $ echo 1 > /sys/devices/system/cpu/cpu0/online
*
* In addition to the above commands, as root, also run the below command:
*
* $ sysctl kernel.sched_rt_runtime_us=-1
*
* Now, the R-Pi 3B+ platform is ready for the below code execution.
*
* TO RUN THE CODE BELOW, PLEASE ENTER THE BELOW COMMANDS USING THE CLI:
*
* $ sudo nano test_code.c //Enter this code in this file and save it
* $ gcc test_code.c -o exefile -lpthread //Compile the code
* $ ./exefile //Running the executable file just created
*
*/

#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include <time.h>
#include <stdlib.h>

// NUMBER OF THREADS
#define NUM_THREADS 2
#define THREAD_1 1
#define THREAD_2 2

typedef struct
{
    int threadIdx;
} threadParams_t;


pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];

struct sched_param nrt_param;

pthread_mutex_t rsrcA, rsrcB;

volatile int rsrcACnt=0, rsrcBCnt=0, noWait=0;


void *grabRsrcs(void *threadp)
{
   threadParams_t *threadParams = (threadParams_t *)threadp;
   int threadIdx = threadParams->threadIdx;

   int i;
   
   if(threadIdx == THREAD_1)
   {
     printf("THREAD 1 grabbing resources\n");
     //The thread checks if both critical resources are available and
	 //performs a random back-off if they are unavailable.
	 
	 while(rsrcBCnt)
     {
       pthread_mutex_unlock(&rsrcB);
       pthread_mutex_unlock(&rsrcA);
       usleep(10000);
     }
     pthread_mutex_lock(&rsrcA);
     rsrcACnt++;
     if(!noWait) usleep(1000000);
     printf("THREAD 1 got A, trying for B\n");
     pthread_mutex_lock(&rsrcB);
     rsrcBCnt++;
     printf("THREAD 1 got A and B\n");
     pthread_mutex_unlock(&rsrcB);
     pthread_mutex_unlock(&rsrcA);
     printf("THREAD 1 done\n");
     rsrcACnt = 0;
     rsrcBCnt = 0;
   }
   else
   {
     printf("THREAD 2 grabbing resources\n");
     while(rsrcACnt)
     //The thread checks if both critical resources are available and
	 //performs a random back-off if they are unavailable.
	 {
       pthread_mutex_unlock(&rsrcA);
       pthread_mutex_unlock(&rsrcB);
       usleep(25000);
     }
     pthread_mutex_lock(&rsrcB);
     rsrcBCnt++;
     if(!noWait) usleep(1000000);
     printf("THREAD 2 got B, trying for A\n");
     pthread_mutex_lock(&rsrcA);
     rsrcACnt++;
     printf("THREAD 2 got B and A\n");
     pthread_mutex_unlock(&rsrcA);
     pthread_mutex_unlock(&rsrcB);
     printf("THREAD 2 done\n");
   }
   pthread_exit(NULL);
}

int main (int argc, char *argv[])
{
   int rc, safe=0;

   rsrcACnt=0, rsrcBCnt=0, noWait=0;

   if(argc < 2)
   {
     printf("Will set up unsafe deadlock scenario\n");
   }
   else if(argc == 2)
   {
     if(strncmp("safe", argv[1], 4) == 0)
       safe=1;
     else if(strncmp("race", argv[1], 4) == 0)
       noWait=1;
     else
       printf("Will set up unsafe deadlock scenario\n");
   }
   else
   {
     printf("Usage: deadlock [safe|race|unsafe]\n");
   }

   // Set default protocol for mutex
   pthread_mutex_init(&rsrcA, NULL);
   pthread_mutex_init(&rsrcB, NULL);

   printf("Creating thread %d\n", THREAD_1);
   threadParams[THREAD_1].threadIdx=THREAD_1;
   rc = pthread_create(&threads[0], NULL, grabRsrcs, (void *)&threadParams[THREAD_1]);
   if (rc) {printf("ERROR; pthread_create() rc is %d\n", rc); perror(NULL); exit(-1);}
   printf("Thread 1 spawned\n");

   if(safe) // Make sure Thread 1 finishes with both resources first
   {
     if(pthread_join(threads[0], NULL) == 0)
       printf("Thread 1: %x done\n", (unsigned int)threads[0]);
     else
       perror("Thread 1");
   }

   printf("Creating thread %d\n", THREAD_2);
   threadParams[THREAD_2].threadIdx=THREAD_2;
   rc = pthread_create(&threads[1], NULL, grabRsrcs, (void *)&threadParams[THREAD_2]);
   if (rc) {printf("ERROR; pthread_create() rc is %d\n", rc); perror(NULL); exit(-1);}
   printf("Thread 2 spawned\n");

   printf("rsrcACnt=%d, rsrcBCnt=%d\n", rsrcACnt, rsrcBCnt);
   printf("will try to join CS threads unless they deadlock\n");

   if(!safe)
   {
     if(pthread_join(threads[0], NULL) == 0)
       printf("Thread 1: %x done\n", (unsigned int)threads[0]);
     else
       perror("Thread 1");
   }

   if(pthread_join(threads[1], NULL) == 0)
     printf("Thread 2: %x done\n", (unsigned int)threads[1]);
   else
     perror("Thread 2");

   if(pthread_mutex_destroy(&rsrcA) != 0)
     perror("mutex A destroy");

   if(pthread_mutex_destroy(&rsrcB) != 0)
     perror("mutex B destroy");

   printf("All done\n");

   exit(0);
}
