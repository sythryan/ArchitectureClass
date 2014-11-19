/*  Programmer:  <PUT YOUR NAME HERE>
    File:        hw6.c
    Compiled by: gcc -o hw6 hw6.c -lpthread -lm
    Run by:  ./hw6 <matrix size> <number of threads> <threshold>
    Description:  2D over-relaxation program written using POSIX threads.
     Using barrier we implemented with condition variable and a mutex.
*/
#include <math.h>
#include <stdio.h>
#include <limits.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include "timer.h"

#define MAXTHREADS 32	/* max. # threads */

/* Prototypes that mostly use global variables */
void * thread_main(void *);
void InitializeData();
void barrier();

pthread_mutex_t update_lock;
pthread_mutex_t barrier_lock;	/* mutex for the barrier */
pthread_cond_t all_here;	/* condition variable for barrier */
int count=0;			/* counter for barrier */

/* Global Variables */
int n, t;
double threshold;
double **val, **new; /* pointers to the 2D array of values */
                     /* val contains the current values with new used
                        to calculate the next iterations values */ 
double delta = 0.0;
double deltaNew = 0.0;

/* Command line args: matrix size, number of threads, threshold */
int main(int argc, char * argv[])
{
  /* thread ids and attributes */
  pthread_t tid[MAXTHREADS];
  pthread_attr_t attr;
  long i, j;
  double startTime, endTime,seqTime,parTime;
  float myThreshold;
  
  /* set global thread attributes */
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
  
  /* initial mutex and condition variable */
  pthread_mutex_init(&update_lock, NULL);
  pthread_mutex_init(&barrier_lock, NULL);
  pthread_cond_init(&all_here, NULL);
  
  /* read command line arguments */
  if (argc != 4) {
    printf("usage: %s <matrix size> <number of threads> <threshold>\n",
	   argv[0]);
    exit(1);
  } // end if
  
  sscanf(argv[1], "%d", &n);
  sscanf(argv[2], "%d", &t);
  sscanf(argv[3], "%f", &myThreshold);
  threshold = (double) myThreshold;

  /* Initial val and new 2D arrays with 0s and 1s. */  
  InitializeData();
  printf("InitializeData done\n");
  GET_TIME(startTime);
  for(i=0; i<t; i++) {
    pthread_create(&tid[i], &attr, thread_main, (void *) i);
  } // end for
  
  for (i=0; i < t; i++) {
    pthread_join(tid[i], NULL);
  } // end for  
  
  GET_TIME(endTime);
  printf("Parallel 2D SOR on %d x %d using %d threads: Time = %1.5f seconds\n", 
		  n, n, t, endTime-startTime);
  printf("maximum difference:  %e\n", delta);
  
} // end main


void* thread_main(void * arg) {
  long id=(long) arg;

  /* YOUR CODE GOES HERE */

} // end thread_main

void InitializeData() {
  int i, j;
  
  new = (double **) malloc((n+2)*sizeof(double *));
  val = (double **) malloc((n+2)*sizeof(double *));
  
  for (i = 0; i < n+2; i++) {
    new[i] = (double *) malloc((n+2)*sizeof(double));
    val[i] = (double *) malloc((n+2)*sizeof(double));
  } // end for i
  
  /* initialize to 0.0 except to 1.0 along the left boundary */
  for (i = 0; i < n+2; i++) {
    val[i][0] = 1.0;
    new[i][0] = 1.0;
  } // end for i
  for (i = 0; i < n+2; i++) {
    for (j = 1; j < n+2; j++) {
      val[i][j] = 0.0;
      new[i][j] = 0.0;
    } // end for j
  } // end for i
} // end InitializeData

void barrier(long id) {
  pthread_mutex_lock(&barrier_lock);
  count++;
  // printf("count %d, id %d\n", count, id);
  if (count == t) {
    count = 0;
    pthread_cond_broadcast(&all_here);
  } else {
    while(pthread_cond_wait(&all_here, &barrier_lock) != 0);
  } // end if
  pthread_mutex_unlock(&barrier_lock);
} // end barrier

