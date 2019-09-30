////////////////////////////////////////////////////////////////////////
/////////ECEN 5623 Exercise-2 Problem-3 Feasibility Test Code///////////
////////////////////////////////////////////////////////////////////////

/*
* This code has been modified by Rushi James Macwan for ECEN 5623 RTES
* Exercise-2 submission. The code is originally owned by Dr. Sam Siewert
* and is duly credited for his original work.
*
* The modifications performed to this code has been highlighted by the 
* statements pointing out where the modification begins and ends. Also,
* the modifications essentially include 5 other examples on the feasibility
* tests. 
*/

// Include Statements


#include <math.h>
#include <stdio.h>

// Macros

#define TRUE 1
#define FALSE 0
#define U32_T unsigned int

/*
* 	Service periods and worst-case execution times
*/

// U=0.7333
U32_T ex0_period[] = {1000, 1000};
U32_T ex0_wcet[] = {60, 15};

// U=0.9857
U32_T ex1_period[] = {100, 100};
U32_T ex1_wcet[] = {60, 15};

// Introducing the two feasibility test functions

int completion_time_feasibility(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[]);
int scheduling_point_feasibility(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[]);


int main(void)
{ 
    int i;
	U32_T numServices;
    
	/*
	* COMPLETION TEST FEASIBILITY CHECK
	*/
	
    printf("******** Completion Test Feasibility Example\n");
   
    printf("@1Hz U=%4.2f (C1=60, C2=15; T1=1000, T2=1000; T=D): ",
		   ((65.0/1000.0) + (15.0/1000.0)));
	numServices = sizeof(ex0_period)/sizeof(U32_T);
    if(completion_time_feasibility(numServices, ex0_period, ex0_wcet, ex0_period) == TRUE)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");

    printf("@10Hz U=%4.2f (C1=60, C2=15; T1=100, T2=100; T=D): ", 
		   ((60.0/100.0) + (15.0/100.0)));
	numServices = sizeof(ex1_period)/sizeof(U32_T);
    if(completion_time_feasibility(numServices, ex1_period, ex1_wcet, ex1_period) == TRUE)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");


	/*
	* SCHEDULING POINT FEASIBILITY CHECK
	*/

	printf("\n\n");
    printf("******** Scheduling Point Feasibility Example\n");

    printf("@1Hz U=%4.2f (C1=60, C2=15; T1=1000, T2=1000; T=D): ",
		   ((65.0/1000.0) + (15.0/1000.0)));
	numServices = sizeof(ex0_period)/sizeof(U32_T);
    if(scheduling_point_feasibility(numServices, ex0_period, ex0_wcet, ex0_period) == TRUE)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");

    printf("@10Hz U=%4.2f (C1=60, C2=15; T1=100, T2=100; T=D): ", 
		   ((60.0/100.0) + (15.0/100.0)));
	numServices = sizeof(ex1_period)/sizeof(U32_T);
    if(scheduling_point_feasibility(numServices, ex1_period, ex1_wcet, ex1_period) == TRUE)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");

}

/*
* Function to perform completion time feasibility test for a given service
*/

int completion_time_feasibility(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[])
{
  int i, j;
  U32_T an, anext;
  
  // assume feasible until we find otherwise
  int set_feasible=TRUE;
   
  //printf("numServices=%d\n", numServices);
  
  for (i=0; i < numServices; i++)
  {
       an=0; anext=0;
       
       for (j=0; j <= i; j++)
       {
           an+=wcet[j];
       }
       
	   //printf("i=%d, an=%d\n", i, an);

       while(1)
       {
             anext=wcet[i];
	     
             for (j=0; j < i; j++)
                 anext += ceil(((double)an)/((double)period[j]))*wcet[j];
		 
             if (anext == an)
                break;
             else
                an=anext;

			 //printf("an=%d, anext=%d\n", an, anext);
       }
       
	   //printf("an=%d, deadline[%d]=%d\n", an, i, deadline[i]);

       if (an > deadline[i])
       {
          set_feasible=FALSE;
       }
  }
  
  return set_feasible;
}

/*
* Function to perform scheduling point feasibility test for a given service
*/

int scheduling_point_feasibility(U32_T numServices, U32_T period[], 
								 U32_T wcet[], U32_T deadline[])
{
   int rc = TRUE, i, j, k, l, status, temp;

   for (i=0; i < numServices; i++) // iterate from highest to lowest priority
   {
      status=0;

      for (k=0; k<=i; k++) 
      {
          for (l=1; l <= (floor((double)period[i]/(double)period[k])); l++)
          {
               temp=0;

               for (j=0; j<=i; j++) temp += wcet[j] * ceil((double)l*(double)period[k]/(double)period[j]);

               if (temp <= (l*period[k]))
			   {
				   status=1;
				   break;
			   }
           }
           if (status) break;
      }
      if (!status) rc=FALSE;
   }
   return rc;
}