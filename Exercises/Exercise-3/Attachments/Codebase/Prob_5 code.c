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
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <time.h> 
#include <sys/time.h>   
#include <sys/resource.h>

// MODIFIED NUMBER OF THREADS
#define NUMBER_OF_THREADS 2 // Number of threads used in the code
#define THREAD_1 1
#define THREAD_2 2
#define MUTEX

unsigned int flag;      //flag for ans-5 if flag=0 here then it will generate mutex condition

unsigned int iteration=0;

struct dataset
{
        int x, y, z;
        int Acceleration;
        int Roll;
        int Pitch;
        int Yaw_rate;
} data;

struct timespec sample_time;

typedef struct
{
int threadID;
} pthread_params_t;

/*
* Declarations for pthreads
*/

pthread_t threads[NUMBER_OF_THREADS];
pthread_params_t pthread_params[NUMBER_OF_THREADS];

////////////////////////changes

struct sched_param nrt_param;

pthread_mutex_t rsrcA, rsrcB;

volatile int rsrcACnt=0, rsrcBCnt=0, noWait=0;

#ifdef MUTEX

void *grabRsrcs(void *threadp)
{
   pthread_params_t *pthread_params = (pthread_params_t *)threadp;
   int threadID = pthread_params->threadID;

   int i;
   
   if(threadID == THREAD_1)
   {
     printf("THREAD 1 grabbing resources\n");
     while(rsrcBCnt)
     {
		flag=0;
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
     /*
	 while(rsrcACnt)
     {
      flag=0;
     }
	 */
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

#endif
////////////////////////changes

/*
* MODIFICATIONS HAVE BEEN MADE HERE
* Some global variables have been declared here
*/

void *pthread_counter(void *pthread_p)
{
int i;
pthread_params_t *pthread_params = (pthread_params_t *)pthread_p;

for(i=1; i < (pthread_params->threadID)+1; i++)

/*
* MODIFICATIONS HAVE BEEN MADE HERE
* The thread operations have been changed as per the need
*/

if(pthread_params->threadID == 1) //Thread_1 counts the sum of numbers 1...99
{

if(!iteration)
{
        data.x = -150;
        data.y = 200;
        data.z = 3;
        data.Acceleration = 10;
        data.Roll = 50;
        data.Pitch = 0;
        data.Yaw_rate = 215;

        clock_gettime(CLOCK_REALTIME, &sample_time);
        usleep(100000);

//      flag++;
        iteration++;
}

else
{
        data.x+=10;
        data.y-=1;
        data.z+=3;
        data.Acceleration+=5;
        data.Roll-=2;
        data.Pitch+=7;
        data.Yaw_rate-=13;

        clock_gettime(CLOCK_REALTIME, &sample_time);
        usleep(100000);
//      flag=0;
}
}

if(pthread_params->threadID == 2) //Thread_2 counts the sum of numbers 100...199
{
    if(flag)
        {
                printf("x = %d, y = %d, z = %d \n", data.x, data.y, data.z);
        printf("Acc = %d\n", data.Acceleration);
        printf("Roll = %d\n", data.Roll);
        printf("Pitch = %d\n", data.Pitch);
        printf("Yaw = %d\n", data.Yaw_rate);
        //printf("sample_time = %ld\n", sample_time.tv_sec);

        long int x = sample_time.tv_sec;

                time_t rawtime = x;
                struct tm  ts;
                char       buf[80];

                // Format time, "ddd yyyy-mm-dd hh:mm:ss zzz"
                ts = *localtime(&rawtime);
                strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);
                printf("%s\n", buf);
        }

        else
        {
				printf("A MUTEX deadlock has occurred... The system will wait for 10 seconds before a auto-reset.\n");
                usleep(10000000);
                if(!flag)
                {
                        clock_gettime(CLOCK_REALTIME, &sample_time);
                        printf("No new data available at the below time...\n");
                        long int x = sample_time.tv_sec;
                        time_t rawtime = x;
                        struct tm  ts;
                        char       buf[80];
                        // Format time, "ddd yyyy-mm-dd hh:mm:ss zzz"
                        ts = *localtime(&rawtime);
                        strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);
                        printf("%s\n", buf);
                        flag++;
						pthread_mutex_unlock(&rsrcA);
						pthread_mutex_unlock(&rsrcB);
						printf("Deadlock resolved...\n");
                }
        }
}
}

int main (int argc, char *argv[])
{

//struct dataset data;
//struct timespec sample_time;

int i;

while(1)
{

//flag = 0;

for(i=1; i <= NUMBER_OF_THREADS; i++)
{
pthread_params[i].threadID=i;

pthread_create(&threads[i], // pointer pthread descriptor
(void *)0, // Feeding in default attributes
pthread_counter, // pthread entry point
(void *)&(pthread_params[i]) // parameters passed in
);
}

for(i=0;i<NUMBER_OF_THREADS;i++)
pthread_join(threads[i], NULL);

/*
* MODIFICATIONS HAVE BEEN MADE HERE
* The final output of the main function has been modified as per
* the objectives of the code problem.
*/


//printf("TEST COMPLETED SUCCESSFULLY\n");
}
}