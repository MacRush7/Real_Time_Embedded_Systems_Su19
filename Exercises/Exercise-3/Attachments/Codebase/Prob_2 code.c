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
* Apart from the above code, I have referred to the below code links for getting a representable
* code timing output using the available functions:
* http://www.cs.tufts.edu/comp/111/examples/Time/clock_gettime.c
* https://www.epochconverter.com/programming/c
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
*
*/


#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include <time.h> 
#include <sys/time.h>   
#include <sys/resource.h>

// NUMBER OF THREADS
#define NUMBER_OF_THREADS 2

//Iterations required in the thread-1 execution
unsigned int iteration=0;

//Structure used by thread-1 with the navigation data
struct dataset
{
        int x, y, z;
        int Acceleration;
        int Roll;
        int Pitch;
        int Yaw_rate;
} data;

//Timespec structure
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

/*
* MODIFICATIONS HAVE BEEN MADE HERE
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
}
}

if(pthread_params->threadID == 2) //Thread_2 counts the sum of numbers 100...199
{
        printf("x = %d, y = %d, z = %d \n", data.x, data.y, data.z);
        printf("Acc = %d\n", data.Acceleration);
        printf("Roll = %d\n", data.Roll);
        printf("Pitch = %d\n", data.Pitch);
        printf("Yaw = %d\n", data.Yaw_rate);
        //printf("sample_time = %ld\n", sample_time.tv_sec);

        long int x = sample_time.tv_sec;

	/* This section has been taken as it was presented in the epoch time conversion example. I duly
	 * credit the resource link mentioned at the beginning of this code as the original owner of this
	 * code.
	 */
	 
	//Completely reused fragment from external link for time conversion: 
    time_t rawtime = x;
    struct tm  ts;
    char       buf[80];
    // Format time, "ddd yyyy-mm-dd hh:mm:ss zzz"
    ts = *localtime(&rawtime);
    strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);
    printf("%s\n", buf);
}

}

int main (int argc, char *argv[])
{

int i;

while(1)
{

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


printf("TEST COMPLETED SUCCESSFULLY\n");
}
}