/******************************************************************************************************/
/* ECEN 5623 RTES Summer 2019 - Exercise-4 Real-Time Applications									  */
/* Problem #5 Code						                                    						  */
/*                                                                          						  */
/* Student Name: Rushi James Macwan, July 19th, 2019                         						  */
/******************************************************************************************************/
// This code has been re-used, modified and adapted according to the needs of the problem. 
//
// The original code is owned by Dr. Siewert and the major modifications if any have been highlighted 
// in this code with the source number. I duly credit Dr. Siewert for his original work.
// 
// The original code owned by Dr. Siewert can be found by visiting the below links:
// Link [1]: http://ecee.colorado.edu/%7Eecen5623/ecen/ex/Linux/computer-vision/simple-capture/
// Link [2]: http://ecee.colorado.edu/%7Eecen5623/ecen/ex/Linux/computer-vision/sharpen-psf/sharpen.c
// Link [3]: http://ecee.colorado.edu/%7Eecen5623/ecen/ex/Linux/code/RT-Clock/posix_clock.c
//
// In addition to the above reference, this code has been reused, modified and adapted using an external
// source code available on the internet. Major modifications, if any, have been duly highlighted in this
// code with the source number.
// Link [3]: 
// Link [4]:
//
// To regurgitate, this code is not entirely my original work. I duly credit all the above cited
// original owners and their original work. My work is based on the work created by them with due
// re-use, modifications, and adapatations. However, major changes may be highlighted or commented
// as per the need keeping in mind the availability of time.
//
/******************************************************************************************************/
/*
* Adapted by Sam Siewert for use with UVC web cameras and Bt878 frame
* grabber NTSC cameras to acquire digital video from a source,
* time-stamp each frame acquired, save to a PGM or PPM file.
*
* The original code adapted was open source from V4L2 API and had the
* following use and incorporation policy:
* 
* This program can be used and distributed without restrictions.
*
* This program is provided with the V4L2 API
* see http://linuxtv.org/docs.php for more information
*/
/******************************************************************************************************/
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
* $ sudo nano rushi.c //Enter this code in this file and save it
* $ gcc rushi.c -o exefile -lpthread -lrt //Compile the code
* $ ./exefile //Running the executable file just created
*
*/
/******************************************************************************************************/

/** BRIEF EXPLANATION ON THE PROBLEM
 *
 * 
 *
 * ASSUMPTIONS MADE:
 *
 *
 *
 * @file rushi.c 
 * @brief Problem Heading
 * 
 * File operation brief explanation 
 * @author  Rushi James Macwan 
 * @date July 19th, 2019 
 *
 * PLEASE NOTE:
 *
 * Best effort has been made to include comments wherever necessary to increase the redability of
 * this code. However, in best interests with the tight timing constraint on the exam, some elements
 * may be inadequately introduced and/or explained. Also, in addition to some redundancies arising out
 * of lack of time to eliminate them, one or more part of the code may be redundant but does not
 * undermine the execution and the response of the code to the intended problem objectives.
 */ 

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INCLUSION OF HEADER FILES
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/* Please Note:
 * Although not required, this code may contain some additional header files (for simplicity) that may 
 * create some redundancy and overhead of the code execution and system respone in an entirely 
 * RT-environment.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>             /* getopt_long() */
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <time.h>
#include <syslog.h>


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MACRO DEFINITIONS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// CLEAR (x) clears the memory locations specified
// by the memset function and pads zeros at these
// locations

// The memset function is used to fill 
// a block of memory with a particular data. 
// Its arguments include a pointer to a memory
// location (*ptr), an integer value (x) that
// needs to be stored at the locations the memory
// pointer (*ptr) points to and n represents the 
// number of bytes to be filled starting at the 
// memory location specified by the pointer (*ptr).
// 
// Function syntax: void *memset(void *ptr, int x, size_t n);
// Reference: https://www.geeksforgeeks.org/memset-c-example/

#define CLEAR(x) memset(&(x), 0, sizeof(x))

// PRINTF for debugging
// PRINTF2 for internal fprintf and snprintf executions
//syslog enabled by DEFAULT)

#define DEBUG	// for storing the output to an external file for histogram collection
//#define PRINTF
#define PRINTF2

#define COLOR_CONVERT
//#define GRAY_CONVERT
//#define SHARP_CONVERT

//#define DUMP1 //For RGB
//#define DUMP2 //FOR GRAYSCALE
//#define DUMP  //For SHARPENING

#ifdef SHARP_CONVERT
	#define DUMP1
#endif

////////////////////////////////////////////////////////////
//--------------------COLOR CONVERSION--------------------//
//	ifdef			COLOR_CONVERT	  	Color Conversion  //
//	ifdef			GRAY_CONVERT	  	Gray Conversion   //
//	ifdef			SHARP_CONVERT	  	Sharp Conversion  //	
////////////////////////////////////////////////////////////

// Image Sharpening (Continuous Transformation) Constant
#define K 4.0

#define RES_1	// Resolutions 1-5 in 4:3 common resolution ratio

////////////////////////////////////////////////////////////
//--------------------RESOLUTION TABLE--------------------//
//	RESOLUTION NAME		RESOLUTION		ASPECT RATIO	  //	
//--------------------------------------------------------//
//		RES_1			640 X 480			4:3			  //
//		RES_2			320 X 240			4:3			  //
//		RES_3			160 X 120			4:3			  //
//		RES_4			800 X 600			4:3			  //
//		RES_5			960 X 720			4:3			  //
////////////////////////////////////////////////////////////

#ifdef RES_1	// 640 x 480 Resolution

// Macro for the horizontal resolution in terms of pixels
#define HRES 640
// Macro for the vertical resolution in terms of pixels
#define VRES 480 

// Macro for the string containing the resolution
#define HRES_STR "640"
#define VRES_STR "480"

#endif

#ifdef RES_2	// 320 x 240 Resolution

// Macro for the horizontal resolution in terms of pixels
#define HRES 320
// Macro for the vertical resolution in terms of pixels
#define VRES 240

// Macro for the string containing the resolution
#define HRES_STR "320"
#define VRES_STR "240"

#endif

#ifdef RES_3	// 160 x 120 Resolution

// Macro for the horizontal resolution in terms of pixels
#define HRES 160
// Macro for the vertical resolution in terms of pixels
#define VRES 120

// Macro for the string containing the resolution
#define HRES_STR "160"
#define VRES_STR "120"

#endif

#ifdef RES_4	// 800 x 600 Resolution

// Macro for the horizontal resolution in terms of pixels
#define HRES 800
// Macro for the vertical resolution in terms of pixels
#define VRES 600

// Macro for the string containing the resolution
#define HRES_STR "800"
#define VRES_STR "600"

#endif

#ifdef RES_5	// 960 x 720 Resolution

// Macro for the horizontal resolution in terms of pixels
#define HRES 960
// Macro for the vertical resolution in terms of pixels
#define VRES 720

// Macro for the string containing the resolution
#define HRES_STR "960"
#define VRES_STR "720"

#endif

#define NSEC_PER_SEC (1000000000)
#define USEC_PER_SEC (1000000)
#define MSEC_PER_SEC (1000)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES & DATA
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Format is used by a number of functions, so made as a file global.
static struct v4l2_format fmt;

// Enumerating the IO operations like Reading, Memory-Map Access and User Pointer execution.
enum io_method 
{
	IO_METHOD_READ,
	IO_METHOD_MMAP,
	IO_METHOD_USERPTR,
};

// Structure defined for the buffer used for storing frames of images specified by its starting
// location (by a pointer) and the length of the buffer.
struct buffer 
{
	void   *start;
	size_t  length;
};

// Static Global Variables

static char            	*dev_name;
static enum 			io_method   io = IO_METHOD_MMAP;
static int              fd = -1;
struct 					buffer      *buffers;
static unsigned int     n_buffers;
static int              out_buf;
static int              force_format=1;
static int              frame_count = 1; // No. of frames created by the program execution
static const char 		short_options[] = "d:hmruofc:";

static const struct option
long_options[] = {
	{ "device", required_argument, NULL, 'd' },
	{ "help",   no_argument,       NULL, 'h' },
	{ "mmap",   no_argument,       NULL, 'm' },
	{ "read",   no_argument,       NULL, 'r' },
	{ "userp",  no_argument,       NULL, 'u' },
	{ "output", no_argument,       NULL, 'o' },
	{ "format", no_argument,       NULL, 'f' },
	{ "count",  required_argument, NULL, 'c' },
	{ 0, 0, 0, 0 }
};

// Other Global Variables

char 					ppm_header[]="P6\n#9999999999 sec 9999999999 msec \n"HRES_STR" "VRES_STR"\n255\n";
char 					ppm_dumpname[]="test00000000.ppm";
char 					pgm_header[]="P5\n#9999999999 sec 9999999999 msec \n"HRES_STR" "VRES_STR"\n255\n";
char 					pgm_dumpname[]="test00000000.pgm";
unsigned int 			framecnt=0; // Frame counter global variable
unsigned char 			bigbuffer[(1280*960)];

// Other Global Typdef Declarations

typedef double 					FLOAT;
typedef unsigned int 			UINT32;
typedef unsigned long long int 	UINT64;
typedef unsigned char 			UINT8;

// PPM Edge Enhancement Code
UINT8 header[22];
UINT8 R[VRES*HRES];
UINT8 G[VRES*HRES];
UINT8 B[VRES*HRES];
UINT8 convR[VRES*HRES];
UINT8 convG[VRES*HRES];
UINT8 convB[VRES*HRES];

FLOAT PSF[9] = {-K/8.0, -K/8.0, -K/8.0, -K/8.0, K+1.0, -K/8.0, -K/8.0, -K/8.0, -K/8.0};

static struct timespec rtclk_dt = {0, 0};
static struct timespec rtclk_start_time = {0, 0};
static struct timespec rtclk_stop_time = {0, 0};

long int tvd;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION DECLARATIONS (PROTOTYPES)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* FUNCTION PROPERTIES
 * 
 * FUNC RETURN 		FUNC NAME			FUNC ARGUMENT
 * TYPE
 */

static void 		errno_exit			(const char *s);
static int 			xioctl				(int fh, int request, void *arg);
static void 		dump_ppm			(const void *p, int size, unsigned int tag, struct timespec *time);
static void 		dump_pgm			(const void *p, int size, unsigned int tag, struct timespec *time);
void 				yuv2rgb_float		(float y, float u, float v, unsigned char *r, unsigned char *g, unsigned char *b);
void 				yuv2rgb				(int y, int u, int v, unsigned char *r, unsigned char *g, unsigned char *b);
static void 		process_image		(const void *p, int size);
static int 			read_frame			(void);
static void 		mainloop			(void);
static void 		stop_capturing		(void);
static void 		start_capturing		(void);
static void 		uninit_device		(void);
static void 		init_read			(unsigned int buffer_size);
static void 		init_mmap			(void);
static void 		init_userp			(unsigned int buffer_size);
static void 		init_device			(void);
static void 		close_device		(void);
static void 		open_device			(void);
static void 		usage				(FILE *fp, int argc, char **argv);
void 				delta_t				(struct timespec *stop, struct timespec *start, struct timespec *delta_t);
void				timestamp			(void);

#ifdef SHARP_CONVERT
void 				ppm_sharpen			(void);
#endif

// Useful information on the function usage is provided in the FUNCTION DEFINITIONS SECTION


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MAIN FUNCTION
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/** 
 * @brief Function brief
 * 
 * Function overview
 * @param variable explanation
 * @return  void. 
 */

int main(int argc, char **argv)
{
	int f;
	
    if(argc > 1)
	{
        dev_name = argv[1];
    }
	else
	{	
        dev_name = "/dev/video0";
	}
	
    for (;;)
    {
        int idx;
        int c;

        c = getopt_long(argc, argv, short_options, long_options, &idx);

        if (-1 == c)
		{
            break;
		}
		
        switch (c)
        {
            case 0: /* getopt_long() flag */
                break;

            case 'd':
                dev_name = optarg;
                break;

            case 'h':
                usage(stdout, argc, argv);
                exit(EXIT_SUCCESS);

            case 'm':
                io = IO_METHOD_MMAP;
                break;

            case 'r':
                io = IO_METHOD_READ;
                break;

            case 'u':
                io = IO_METHOD_USERPTR;
                break;

            case 'o':
                out_buf++;
                break;

            case 'f':
                force_format++;
                break;

            case 'c':
                errno = 0;
                frame_count = strtol(optarg, NULL, 0);
                if (errno)
                        errno_exit(optarg);
                break;

            default:
                usage(stderr, argc, argv);
                exit(EXIT_FAILURE);
        }
    }

#ifndef SHARP_CONVERT

	open_device();
	init_device();
	start_capturing();
	
	for(f=1; f<=100; f++)
	{		
		clock_gettime(CLOCK_REALTIME, &rtclk_start_time);
		mainloop();
		clock_gettime(CLOCK_REALTIME, &rtclk_stop_time);
		
		timestamp();
	}	
	
	stop_capturing();
	uninit_device();
	close_device();

	#ifdef PRINTF2
		fprintf(stderr, "\n");
	#endif

#endif


#ifdef SHARP_CONVERT

	open_device();
	init_device();
	start_capturing();
	mainloop();
	stop_capturing();
	uninit_device();
	close_device();

	#ifdef PRINTF2
		fprintf(stderr, "\n");
	#endif

	for(f=1; f<=100; f++)
	{		
		clock_gettime(CLOCK_REALTIME, &rtclk_start_time);
		ppm_sharpen();
		clock_gettime(CLOCK_REALTIME, &rtclk_stop_time);
		
		timestamp();
	}
	
#endif

	syslog(LOG_CRIT, "TEST COMPLETED\n"); 	
    return 0;
} 


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION DEFINITIONS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

 
/** 
 * @brief Function brief
 * 
 * Function overview
 * @param variable explanation
 * @return  void. 
 */ 

static void errno_exit(const char *s)
{
#ifdef PRINTF2
	fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
#endif
	exit(EXIT_FAILURE);
}


/** 
 * @brief Function brief
 * 
 * Function overview
 * @param variable explanation
 * @return  void. 
 */ 

static int xioctl(int fh, int request, void *arg)
{
	int r;

	do 
	{
		r = ioctl(fh, request, arg);
	} while (-1 == r && EINTR == errno);

	return r;
}


/** 
 * @brief Function brief
 * 
 * Function overview
 * @param variable explanation
 * @return  void. 
 */ 
 
static void dump_ppm(const void *p, int size, unsigned int tag, struct timespec *time)
{
    int written, i, total, dumpfd;
#ifdef PRINTF2
    snprintf(&ppm_dumpname[4], 9, "%08d", tag);
#endif
    strncat(&ppm_dumpname[12], ".ppm", 5);
    dumpfd = open(ppm_dumpname, O_WRONLY | O_NONBLOCK | O_CREAT, 00666);
#ifdef PRINTF2
    snprintf(&ppm_header[4], 11, "%010d", (int)time->tv_sec);
#endif
    strncat(&ppm_header[14], " sec ", 5);
#ifdef PRINTF2
    snprintf(&ppm_header[19], 11, "%010d", (int)((time->tv_nsec)/1000000));
#endif
    strncat(&ppm_header[29], " msec \n"HRES_STR" "VRES_STR"\n255\n", 19);
    written=write(dumpfd, ppm_header, sizeof(ppm_header));

    total = 0;

    do
    {
        written = write(dumpfd, p, size);
        total += written;
    } while(total < size);
#ifdef PRINTF
    printf("wrote %d bytes\n", total);
#endif
	syslog(LOG_CRIT, "wrote %d bytes\n", total);
    close(dumpfd);
    
}


/** 
 * @brief Function brief
 * 
 * Function overview
 * @param variable explanation
 * @return  void. 
 */ 
 
static void dump_pgm(const void *p, int size, unsigned int tag, struct timespec *time)
{
    int written, i, total, dumpfd;
#ifdef PRINTF2   
    snprintf(&pgm_dumpname[4], 9, "%08d", tag);
#endif
    strncat(&pgm_dumpname[12], ".pgm", 5);
    dumpfd = open(pgm_dumpname, O_WRONLY | O_NONBLOCK | O_CREAT, 00666);

#ifdef PRINTF2
    snprintf(&pgm_header[4], 11, "%010d", (int)time->tv_sec);
#endif

    strncat(&pgm_header[14], " sec ", 5);
	
#ifdef PRINTF2
    snprintf(&pgm_header[19], 11, "%010d", (int)((time->tv_nsec)/1000000));
#endif

    strncat(&pgm_header[29], " msec \n"HRES_STR" "VRES_STR"\n255\n", 19);
    written=write(dumpfd, pgm_header, sizeof(pgm_header));

    total=0;

    do
    {
        written=write(dumpfd, p, size);
        total+=written;
    } while(total < size);
#ifdef PRINTF
    printf("wrote %d bytes\n", total);
#endif
	syslog(LOG_CRIT, "wrote %d bytes\n", total);
    close(dumpfd);
    
}


/** 
 * @brief Function brief
 * 
 * Function overview
 * @param variable explanation
 * @return  void. 
 */ 
 
void yuv2rgb_float(float y, float u, float v, unsigned char *r, unsigned char *g, unsigned char *b)
{
    float r_temp, g_temp, b_temp;

    // R = 1.164(Y-16) + 1.1596(V-128)
    r_temp = 1.164*(y-16.0) + 1.1596*(v-128.0);  
    *r = r_temp > 255.0 ? 255 : (r_temp < 0.0 ? 0 : (unsigned char)r_temp);

    // G = 1.164(Y-16) - 0.813*(V-128) - 0.391*(U-128)
    g_temp = 1.164*(y-16.0) - 0.813*(v-128.0) - 0.391*(u-128.0);
    *g = g_temp > 255.0 ? 255 : (g_temp < 0.0 ? 0 : (unsigned char)g_temp);

    // B = 1.164*(Y-16) + 2.018*(U-128)
    b_temp = 1.164*(y-16.0) + 2.018*(u-128.0);
    *b = b_temp > 255.0 ? 255 : (b_temp < 0.0 ? 0 : (unsigned char)b_temp);
}


/** 
 * @brief Function brief
 * 
 * Function overview
 * @param variable explanation
 * @return  void. 
 */ 
 
void yuv2rgb(int y, int u, int v, unsigned char *r, unsigned char *g, unsigned char *b)
{
	
// This is probably the most acceptable conversion from camera YUYV to RGB
//
// Wikipedia has a good discussion on the details of various conversions and cites good references:
// http://en.wikipedia.org/wiki/YUV
//
// Also http://www.fourcc.org/yuv.php
//
// What's not clear without knowing more about the camera in question is how often U & V are sampled compared
// to Y.
//
// E.g. YUV444, which is equivalent to RGB, where both require 3 bytes for each pixel
//      YUV422, which we assume here, where there are 2 bytes for each pixel, with two Y samples for one U & V,
//              or as the name implies, 4Y and 2 UV pairs
//      YUV420, where for every 4 Ys, there is a single UV pair, 1.5 bytes for each pixel or 36 bytes for 24 pixels
	
   int r1, g1, b1;

   // replaces floating point coefficients
   int c = y-16, d = u - 128, e = v - 128;       

   // Conversion that avoids floating point
   r1 = (298 * c           + 409 * e + 128) >> 8;
   g1 = (298 * c - 100 * d - 208 * e + 128) >> 8;
   b1 = (298 * c + 516 * d           + 128) >> 8;

   // Computed values may need clipping.
   if (r1 > 255) r1 = 255;
   if (g1 > 255) g1 = 255;
   if (b1 > 255) b1 = 255;

   if (r1 < 0) r1 = 0;
   if (g1 < 0) g1 = 0;
   if (b1 < 0) b1 = 0;

   *r = r1 ;
   *g = g1 ;
   *b = b1 ;
}


/** 
 * @brief Function brief
 * 
 * Function overview
 * @param variable explanation
 * @return  void. 
 */ 
 
static void process_image(const void *p, int size)
{
    int 			i;
	int 			newi;
	int 			newsize=0;
	int 			y_temp; 
	int 			y2_temp; 
	int 			u_temp;
	int 			v_temp;
    struct 			timespec frame_time;
    unsigned char	*pptr = (unsigned char *)p;

    // record when process was called
    clock_gettime(CLOCK_REALTIME, &frame_time);    

    framecnt++;
#ifdef PRINTF
    printf("frame %d: ", framecnt);
#endif
	syslog(LOG_CRIT, "frame %d: ", framecnt);

	// DUMPING AND STORAGE OF IMAGE FILES HERE.....

    // This just dumps the frame to a file now, but you could replace with whatever image
    // processing you wish.
    //

    if(fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_GREY)
    {
#ifdef PRINTF
        printf("Dump graymap as-is size %d\n", size);
#endif
		syslog(LOG_CRIT, "Dump graymap as-is size %d\n", size);
		
#ifdef DUMP2		
        dump_pgm(p, size, framecnt, &frame_time);
#endif
    }

    else if(fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV)
    {

#ifdef COLOR_CONVERT
#ifdef PRINTF
	printf("Dump YUYV converted to RGB size %d\n", size);
#endif
	syslog(LOG_CRIT, "Dump YUYV converted to RGB size %d\n", size);
	// Pixels are YU and YV alternating, so YUYV which is 4 bytes
	// We want RGB, so RGBRGB which is 6 bytes
	//
	for(i=0, newi=0; i<size; i=i+4, newi=newi+6)
	{
		y_temp=(int)pptr[i]; u_temp=(int)pptr[i+1]; y2_temp=(int)pptr[i+2]; v_temp=(int)pptr[i+3];
		yuv2rgb(y_temp, u_temp, v_temp, &bigbuffer[newi], &bigbuffer[newi+1], &bigbuffer[newi+2]);
		yuv2rgb(y2_temp, u_temp, v_temp, &bigbuffer[newi+3], &bigbuffer[newi+4], &bigbuffer[newi+5]);
	}
	
#ifdef DUMP1

	dump_ppm(bigbuffer, ((size*6)/4), framecnt, &frame_time);
	
#endif
	
#endif

#ifdef GRAY_CONVERT
#ifdef PRINTF
	printf("Dump YUYV converted to YY size %d\n", size);
#endif
	syslog(LOG_CRIT, "Dump YUYV converted to YY size %d\n", size);
	// Pixels are YU and YV alternating, so YUYV which is 4 bytes
	// We want Y, so YY which is 2 bytes
	//
	for(i=0, newi=0; i<size; i=i+4, newi=newi+2)
	{
		// Y1=first byte and Y2=third byte
		bigbuffer[newi]=pptr[i];
		bigbuffer[newi+1]=pptr[i+2];
	}
#ifdef DUMP2
	dump_pgm(bigbuffer, (size/2), framecnt, &frame_time);
#endif

#endif

    }

    else if(fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_RGB24)
    {
#ifdef PRINTF
        printf("Dump RGB as-is size %d\n", size);
#endif
		syslog(LOG_CRIT, "Dump RGB as-is size %d\n", size);

#ifdef DUMP1
        dump_ppm(p, size, framecnt, &frame_time);
#endif
    }
	
    else
    {
#ifdef PRINTF
        printf("ERROR - unknown dump format\n");
#endif
		syslog(LOG_CRIT, "ERROR - unknown dump format\n");
    }

    fflush(stderr);
    //fprintf(stderr, ".");
    fflush(stdout);
}


/** 
 * @brief Function brief
 * 
 * Function overview
 * @param variable explanation
 * @return  void. 
 */ 
 
static int read_frame(void)
{
    struct v4l2_buffer buf;
    unsigned int i;

    switch (io)
    {
        case IO_METHOD_READ:
        if (-1 == read(fd, buffers[0].start, buffers[0].length))
        {
			switch (errno)
			{

				case EAGAIN:
					return 0;

				case EIO:
					/* Could ignore EIO, see spec. */

					/* fall through */

				default:
					errno_exit("read");
			}
        }

		process_image(buffers[0].start, buffers[0].length);
		break;

        case IO_METHOD_MMAP:
            CLEAR(buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;

		if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
		{
			switch (errno)
			{
				case EAGAIN:
					return 0;

				case EIO:
					/* Could ignore EIO, but drivers should only set for serious errors, although some set for
					   non-fatal errors too.
					 */
					return 0;


				default:
#ifdef PRINTF
					printf("mmap failure\n");
#endif
					syslog(LOG_CRIT, "mmap failure\n");
					errno_exit("VIDIOC_DQBUF");
			}
		}

		assert(buf.index < n_buffers);

		process_image(buffers[buf.index].start, buf.bytesused);

		if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
				errno_exit("VIDIOC_QBUF");
		break;

        case IO_METHOD_USERPTR:
            CLEAR(buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_USERPTR;

		if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
		{
			switch (errno)
			{
				case EAGAIN:
					return 0;

				case EIO:
					/* Could ignore EIO, see spec. */

					/* fall through */

				default:
					errno_exit("VIDIOC_DQBUF");
			}
		}

		for (i = 0; i < n_buffers; ++i)
				if (buf.m.userptr == (unsigned long)buffers[i].start
					&& buf.length == buffers[i].length)
						break;

		assert(i < n_buffers);

		process_image((void *)buf.m.userptr, buf.bytesused);

		if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
				errno_exit("VIDIOC_QBUF");
		break;
    }
	
    //printf("R");
    return 1;
}


/** 
 * @brief Function brief
 * 
 * Function overview
 * @param variable explanation
 * @return  void. 
 */ 
 
static void mainloop(void)
{
    unsigned int count;
    struct timespec read_delay;
    struct timespec time_error;

    read_delay.tv_sec=0;
    read_delay.tv_nsec=30000;

    count = frame_count;

    while (count > 0)
    {
        for (;;)
        {
            fd_set fds;
            struct timeval tv;
            int r;

            FD_ZERO(&fds);
            FD_SET(fd, &fds);

            /* Timeout. */
            tv.tv_sec = 2;
            tv.tv_usec = 200000;

            r = select(fd + 1, &fds, NULL, NULL, &tv);

            if (-1 == r)
            {
                if (EINTR == errno)
                    continue;
                errno_exit("select");
            }

            if (0 == r)
            {
#ifdef PRINTF2
                fprintf(stderr, "select timeout\n");
#endif
                exit(EXIT_FAILURE);
            }

            if (read_frame())
            {
                if(nanosleep(&read_delay, &time_error) != 0)
                    perror("nanosleep");
                else
#ifdef PRINTF
                    printf("time_error.tv_sec=%ld, time_error.tv_nsec=%ld\n", time_error.tv_sec, time_error.tv_nsec);
#endif
					syslog(LOG_CRIT, "time_error.tv_sec=%ld, time_error.tv_nsec=%ld\n", time_error.tv_sec, time_error.tv_nsec);
                
				count--;
                break;
            }

            /* EAGAIN - continue select loop unless count done. */
            if(count <= 0) break;
        }
		
        if(count <= 0) break;
    }
}


/** 
 * @brief Function brief
 * 
 * Function overview
 * @param variable explanation
 * @return  void. 
 */ 
 
static void stop_capturing(void)
{
	enum v4l2_buf_type type;

	switch (io) 
	{
        case IO_METHOD_READ:
			/* Nothing to do. */
			break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
			type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
					errno_exit("VIDIOC_STREAMOFF");
		break;
    }
}


/** 
 * @brief Function brief
 * 
 * Function overview
 * @param variable explanation
 * @return  void. 
 */
 
static void start_capturing(void)
{
	unsigned int i;
	enum v4l2_buf_type type;

	switch (io) 
	{

	case IO_METHOD_READ:
		/* Nothing to do. */
		break;

	case IO_METHOD_MMAP:
		for (i = 0; i < n_buffers; ++i) 
		{
#ifdef PRINTF
			printf("allocated buffer %d\n", i);
#endif
			syslog(LOG_CRIT, "allocated buffer %d\n", i);
			struct v4l2_buffer buf;

			CLEAR(buf);
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = i;

			if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
			{
				errno_exit("VIDIOC_QBUF");
			}
		}
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
		{
			errno_exit("VIDIOC_STREAMON");
		}
		break;

	case IO_METHOD_USERPTR:
		for (i = 0; i < n_buffers; ++i) {
			struct v4l2_buffer buf;

			CLEAR(buf);
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_USERPTR;
			buf.index = i;
			buf.m.userptr = (unsigned long)buffers[i].start;
			buf.length = buffers[i].length;

			if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
			{
				errno_exit("VIDIOC_QBUF");
			}
		}
		
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		
		if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
		{
			errno_exit("VIDIOC_STREAMON");
		}
		break;
		
	}
}


/** 
 * @brief Function brief
 * 
 * Function overview
 * @param variable explanation
 * @return  void. 
 */ 
 
static void uninit_device(void)
{
	unsigned int i;

	switch (io) {
	case IO_METHOD_READ:
			free(buffers[0].start);
			break;

	case IO_METHOD_MMAP:
			for (i = 0; i < n_buffers; ++i)
			if (-1 == munmap(buffers[i].start, buffers[i].length))
			{
				errno_exit("munmap");
			}
			break;

	case IO_METHOD_USERPTR:
			for (i = 0; i < n_buffers; ++i)
			{
				free(buffers[i].start);
			}
			break;
	}

	free(buffers);
}


/** 
 * @brief Function brief
 * 
 * Function overview
 * @param variable explanation
 * @return  void. 
 */ 
 
static void init_read(unsigned int buffer_size)
{
	buffers = calloc(1, sizeof(*buffers));

	if (!buffers) 
	{
#ifdef PRINTF2
		fprintf(stderr, "Out of memory\n");
#endif
		exit(EXIT_FAILURE);
	}

	buffers[0].length = buffer_size;
	buffers[0].start = malloc(buffer_size);

	if (!buffers[0].start) 
	{
#ifdef PRINTF2
		fprintf(stderr, "Out of memory\n");
#endif
		exit(EXIT_FAILURE);
	}
}


/** 
 * @brief Function brief
 * 
 * Function overview
 * @param variable explanation
 * @return  void. 
 */ 

static void init_mmap(void)
{
	struct v4l2_requestbuffers req;

	CLEAR(req);

	req.count = 6;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) 
	{
		if (EINVAL == errno) 
		{
#ifdef PRINTF2
			fprintf(stderr, "%s does not support memory mapping\n", dev_name);
#endif
			exit(EXIT_FAILURE);
		} 
		else 
		{
			errno_exit("VIDIOC_REQBUFS");
		}
	}

	if (req.count < 2) 
	{
#ifdef PRINTF2
		fprintf(stderr, "Insufficient buffer memory on %s\n", dev_name);
#endif
		exit(EXIT_FAILURE);
	}

	buffers = calloc(req.count, sizeof(*buffers));

	if (!buffers) 
	{
#ifdef PRINTF2
		fprintf(stderr, "Out of memory\n");
#endif
		exit(EXIT_FAILURE);
	}

	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) 
	{
		struct v4l2_buffer buf;

		CLEAR(buf);

		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = n_buffers;

		if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
		{		
			errno_exit("VIDIOC_QUERYBUF");
		}

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start =
									mmap(NULL /* start anywhere */,
									buf.length,
									PROT_READ | PROT_WRITE /* required */,
									MAP_SHARED /* recommended */,
									fd, buf.m.offset);

		if (MAP_FAILED == buffers[n_buffers].start)
		{
			errno_exit("mmap");
		}
	}
}


/** 
 * @brief Function brief
 * 
 * Function overview
 * @param variable explanation
 * @return  void. 
 */ 

static void init_userp(unsigned int buffer_size)
{
	struct v4l2_requestbuffers req;

	CLEAR(req);

	req.count  = 4;
	req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_USERPTR;

	if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) 
	{
		if (EINVAL == errno) 
		{
#ifdef PRINTF2
			fprintf(stderr, "%s does not support user pointer i/o\n", dev_name);
#endif
			exit(EXIT_FAILURE);
		} 
		else 
		{
			errno_exit("VIDIOC_REQBUFS");
		}
	}

	buffers = calloc(4, sizeof(*buffers));

	if (!buffers) 
	{
#ifdef PRINTF2
		fprintf(stderr, "Out of memory\n");
#endif
		exit(EXIT_FAILURE);
	}

	for (n_buffers = 0; n_buffers < 4; ++n_buffers) 
	{
		buffers[n_buffers].length = buffer_size;
		buffers[n_buffers].start = malloc(buffer_size);

		if (!buffers[n_buffers].start) 
		{
#ifdef PRINTF2
			fprintf(stderr, "Out of memory\n");
#endif
			exit(EXIT_FAILURE);
		}
	}
}


/** 
 * @brief Function brief
 * 
 * Function overview
 * @param variable explanation
 * @return  void. 
 */ 
 
static void init_device(void)
{
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    unsigned int min;

    if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap))
    {
        if (EINVAL == errno) 
		{
#ifdef PRINTF2
            fprintf(stderr, "%s is no V4L2 device\n", dev_name);
#endif
            exit(EXIT_FAILURE);
        }
        else
        {
			errno_exit("VIDIOC_QUERYCAP");
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
#ifdef PRINTF2
        fprintf(stderr, "%s is no video capture device\n", dev_name);
#endif
        exit(EXIT_FAILURE);
    }

    switch (io)
    {
        case IO_METHOD_READ:
            if (!(cap.capabilities & V4L2_CAP_READWRITE))
            {
#ifdef PRINTF2
                fprintf(stderr, "%s does not support read i/o\n",
                         dev_name);
#endif						 
                exit(EXIT_FAILURE);
            }
            break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
            if (!(cap.capabilities & V4L2_CAP_STREAMING))
            {
#ifdef PRINTF2	
                fprintf(stderr, "%s does not support streaming i/o\n", dev_name);
#endif
                exit(EXIT_FAILURE);
            }
            break;
    }


    /* Select video input, video standard and tune here. */


    CLEAR(cropcap);

    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap))
    {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /* reset to default */

        if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop))
        {
            switch (errno)
            {
                case EINVAL:
                    /* Cropping not supported. */
                    break;
                default:
                    /* Errors ignored. */
					break;
            }
        }

    }
    else
    {
        /* Errors ignored. */
    }


    CLEAR(fmt);

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (force_format)
    {
#ifdef PRINTF
        printf("FORCING FORMAT\n");
#endif
		syslog(LOG_CRIT, "FORCING FORMAT\n");
        fmt.fmt.pix.width       = HRES;
        fmt.fmt.pix.height      = VRES;

        // Specify the Pixel Coding Formate here

        // This one work for Logitech C200
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;

        //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;
        //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_VYUY;

        // Would be nice if camera supported
        //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
        //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;

        //fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
        fmt.fmt.pix.field       = V4L2_FIELD_NONE;

        if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
			errno_exit("VIDIOC_S_FMT");

        /* Note VIDIOC_S_FMT may change width and height. */
    }
    else
    {
#ifdef PRINTF
        printf("ASSUMING FORMAT\n");
#endif
		syslog(LOG_CRIT, "ASSUMING FORMAT\n");
        /* Preserve original settings as set by v4l2-ctl for example */
        if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt))
        {
			errno_exit("VIDIOC_G_FMT");
		}
    }

    /* Buggy driver paranoia. */
    min = fmt.fmt.pix.width * 2;
    
	if (fmt.fmt.pix.bytesperline < min)
	{
		fmt.fmt.pix.bytesperline = min;
		min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    }
	
	if (fmt.fmt.pix.sizeimage < min)
	{
		fmt.fmt.pix.sizeimage = min;
	}
	
    switch (io)
    {
        case IO_METHOD_READ:
            init_read(fmt.fmt.pix.sizeimage);
            break;

        case IO_METHOD_MMAP:
            init_mmap();
            break;

        case IO_METHOD_USERPTR:
            init_userp(fmt.fmt.pix.sizeimage);
            break;
    }
}


/** 
 * @brief Function brief
 * 
 * Function overview
 * @param variable explanation
 * @return  void. 
 */ 
 
static void close_device(void)
{
	if (-1 == close(fd))
	{
		errno_exit("close");
		fd = -1;
	}
}


/** 
 * @brief Function brief
 * 
 * Function overview
 * @param variable explanation
 * @return  void. 
 */ 
 
static void open_device(void)
{
	struct stat st;

	if (-1 == stat(dev_name, &st)) 
	{
#ifdef PRINTF2
		fprintf(stderr, "Cannot identify '%s': %d, %s\n", dev_name, errno, strerror(errno));
#endif
		exit(EXIT_FAILURE);
	}

	if (!S_ISCHR(st.st_mode)) 
	{
#ifdef PRINTF2
		fprintf(stderr, "%s is no device\n", dev_name); exit(EXIT_FAILURE);
#endif
	}

	fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

	if (-1 == fd) 
	{
#ifdef PRINTF2
		fprintf(stderr, "Cannot open '%s': %d, %s\n", dev_name, errno, strerror(errno));
#endif
		exit(EXIT_FAILURE);
	}
}


/** 
 * @brief Function brief
 * 
 * Function overview
 * @param variable explanation
 * @return  void. 
 */ 
 
static void usage(FILE *fp, int argc, char **argv)
{
#ifdef PRINTF2
	fprintf(fp,
		 "Usage: %s [options]\n\n"
		 "Version 1.3\n"
		 "Options:\n"
		 "-d | --device name   Video device name [%s]\n"
		 "-h | --help          Print this message\n"
		 "-m | --mmap          Use memory mapped buffers [default]\n"
		 "-r | --read          Use read() calls\n"
		 "-u | --userp         Use application allocated buffers\n"
		 "-o | --output        Outputs stream to stdout\n"
		 "-f | --format        Force format to 640x480 GREY\n"
		 "-c | --count         Number of frames to grab [%i]\n"
		 "",
		 argv[0], dev_name, frame_count);
#endif
}

#ifdef SHARP_CONVERT


/** 
 * @brief Function brief
 * 
 * Function overview
 * @param variable explanation
 * @return  void. 
 */
 
void ppm_sharpen(void)
{ 
    int fdin, fdout, bytesRead=0, bytesLeft, i, j;
    UINT64 microsecs=0, millisecs=0;
    FLOAT temp;

	if((fdin = open("test00000001.ppm", O_RDONLY, 0644)) < 0)
	{
#ifdef PRINTF
		printf("Please CAPTURE COLOR frames before requesting for SHARPENING...\n");
#endif
		syslog(LOG_CRIT, "Please CAPTURE COLOR frames before requesting for SHARPENING...\n");
		exit(-1);
	}
	if((fdout = open("sharp01.ppm", (O_RDWR | O_CREAT), 0666)) < 0)
	{
#ifdef PRINTF
		printf("Please CAPTURE COLOR frames before requesting for SHARPENING...\n");
#endif
		syslog(LOG_CRIT, "Please CAPTURE COLOR frames before requesting for SHARPENING...\n");
		exit(-1);
	}

	bytesLeft=21;
	//printf("Reading header\n");

	do
	{
		//printf("bytesRead=%d, bytesLeft=%d\n", bytesRead, bytesLeft);
		bytesRead=read(fdin, (void *)header, bytesLeft);
		bytesLeft -= bytesRead;
	} while(bytesLeft > 0);

	header[21]='\0';

	//printf("header = %s\n", header); 

	// Read RGB data
	for(i=0; i<VRES*HRES; i++)
	{
		read(fdin, (void *)&R[i], 1); convR[i]=R[i];
		read(fdin, (void *)&G[i], 1); convG[i]=G[i];
		read(fdin, (void *)&B[i], 1); convB[i]=B[i];
	}

	// Skip first and last row, no neighbors to convolve with
	for(i=1; i<((VRES)-1); i++)
	{
	   // Skip first and last column, no neighbors to convolve with
		for(j=1; j<((HRES)-1); j++)
		{
			temp=0;
			temp += (PSF[0] * (FLOAT)R[((i-1)*HRES)+j-1]);
			temp += (PSF[1] * (FLOAT)R[((i-1)*HRES)+j]);
			temp += (PSF[2] * (FLOAT)R[((i-1)*HRES)+j+1]);
			temp += (PSF[3] * (FLOAT)R[((i)*HRES)+j-1]);
			temp += (PSF[4] * (FLOAT)R[((i)*HRES)+j]);
			temp += (PSF[5] * (FLOAT)R[((i)*HRES)+j+1]);
			temp += (PSF[6] * (FLOAT)R[((i+1)*HRES)+j-1]);
			temp += (PSF[7] * (FLOAT)R[((i+1)*HRES)+j]);
			temp += (PSF[8] * (FLOAT)R[((i+1)*HRES)+j+1]);
			if(temp<0.0) temp=0.0;
			if(temp>255.0) temp=255.0;
			convR[(i*HRES)+j]=(UINT8)temp;

			temp=0;
			temp += (PSF[0] * (FLOAT)G[((i-1)*HRES)+j-1]);
			temp += (PSF[1] * (FLOAT)G[((i-1)*HRES)+j]);
			temp += (PSF[2] * (FLOAT)G[((i-1)*HRES)+j+1]);
			temp += (PSF[3] * (FLOAT)G[((i)*HRES)+j-1]);
			temp += (PSF[4] * (FLOAT)G[((i)*HRES)+j]);
			temp += (PSF[5] * (FLOAT)G[((i)*HRES)+j+1]);
			temp += (PSF[6] * (FLOAT)G[((i+1)*HRES)+j-1]);
			temp += (PSF[7] * (FLOAT)G[((i+1)*HRES)+j]);
			temp += (PSF[8] * (FLOAT)G[((i+1)*HRES)+j+1]);
			if(temp<0.0) temp=0.0;
			if(temp>255.0) temp=255.0;
			convG[(i*HRES)+j]=(UINT8)temp;

			temp=0;
			temp += (PSF[0] * (FLOAT)B[((i-1)*HRES)+j-1]);
			temp += (PSF[1] * (FLOAT)B[((i-1)*HRES)+j]);
			temp += (PSF[2] * (FLOAT)B[((i-1)*HRES)+j+1]);
			temp += (PSF[3] * (FLOAT)B[((i)*HRES)+j-1]);
			temp += (PSF[4] * (FLOAT)B[((i)*HRES)+j]);
			temp += (PSF[5] * (FLOAT)B[((i)*HRES)+j+1]);
			temp += (PSF[6] * (FLOAT)B[((i+1)*HRES)+j-1]);
			temp += (PSF[7] * (FLOAT)B[((i+1)*HRES)+j]);
			temp += (PSF[8] * (FLOAT)B[((i+1)*HRES)+j+1]);
			if(temp<0.0) temp=0.0;
			if(temp>255.0) temp=255.0;
			convB[(i*HRES)+j]=(UINT8)temp;
		}
	}

#ifdef PRINTF
	printf("sharpened file...\n");
#endif
	syslog(LOG_CRIT, "sharpened file...\n");

#ifdef DUMP
	write(fdout, (void *)header, 21);

	// Write RGB data
	for(i=0; i<VRES*HRES; i++)
	{
		write(fdout, (void *)&convR[i], 1);
		write(fdout, (void *)&convG[i], 1);
		write(fdout, (void *)&convB[i], 1);
	}

#endif
	close(fdin);
	close(fdout);

#ifdef PRINTF
	printf("TEST COMPLETED\n"); 
#endif
	syslog(LOG_CRIT, "TEST COMPLETED\n"); 	
	//exit(0);
	return;
}

#endif


/** 
 * @brief Function brief
 * 
 * Function overview
 * @param variable explanation
 * @return  void. 
 */

void delta_t(struct timespec *stop, struct timespec *start, struct timespec *delta_t)
{
  int dt_sec=stop->tv_sec - start->tv_sec;
  int dt_nsec=stop->tv_nsec - start->tv_nsec;

  if(dt_sec >= 0)
  {
    if(dt_nsec >= 0)
    {
      delta_t->tv_sec=dt_sec;
      delta_t->tv_nsec=dt_nsec;
    }
    else
    {
      delta_t->tv_sec=dt_sec-1;
      delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
    }
  }
  else
  {
    if(dt_nsec >= 0)
    {
      delta_t->tv_sec=dt_sec;
      delta_t->tv_nsec=dt_nsec;
    }
    else
    {
      delta_t->tv_sec=dt_sec-1;
      delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
    }
  }

  return;
}


/** 
 * @brief Function brief
 * 
 * Function overview
 * @param variable explanation
 * @return  void. 
 */

void timestamp(void)
{
	delta_t(&rtclk_stop_time, &rtclk_start_time, &rtclk_dt);

#ifdef PRINTF
	printf("RT clock DT seconds = %ld, nanoseconds = %ld\n", rtclk_dt.tv_sec, rtclk_dt.tv_nsec);
#endif

	syslog(LOG_CRIT, "RT clock DT seconds = %ld, nanoseconds = %ld\n", rtclk_dt.tv_sec, rtclk_dt.tv_nsec);
	
	tvd = (long int) ((rtclk_dt.tv_sec * (MSEC_PER_SEC)) + (rtclk_dt.tv_nsec/(USEC_PER_SEC)));

#ifdef DEBUG
	printf("%ld\n", tvd);
#endif

	/*
	 * MAIN TIMING TEST PERFORMED BY THIS SYSLOG COMMAND
	 */
	
	syslog(LOG_CRIT, "%ld\n", tvd);

}