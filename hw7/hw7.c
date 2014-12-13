/* NOTE:  You only need to complete the perform2D_SOR function.
 * The main program already initializes the global "2D" array, ScatterV's it
 * to all MPI processes, and GatherV's the resulting parts back together at 
 * the RootProcess.  
 * 
 * The perform2D_SOR function should be implemented as outlined in class.
 */

/*  Programmer:  Mark Fienup
    File:        hw7.c
    Compiled by: mpicc -o hw7 hw7.c -lm
    Run using "qsub qsub.hw7" where qsub.hw7 file similar to:

#!/bin/bash
#PBS -N mpi
#PBS -l nodes=8:ppn=1
#PBS -l cput=5:00
##PBS -m be
#
echo "-"
NUMPROC=`wc -l ${PBS_NODEFILE} | awk '{print $1}'`
#
# Put the full pathname to the executable below
# NOTE:  YOU'LL NEED TO REPLACE fienup BY YOUR LOG-IN NAME
time mpiexec -np ${NUMPROC} /home/ryansag/hw7/hw7 1024 0.0001

    Description: 2D over-relaxation program written using MPI.
*/
#define RootProcess 0

#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <mpi.h>

// Prototypes
void initializeData(double * val, int n);
void printArray(double * val, int n);
double perform2D_SOR(int myID, int numProcs, double **resultValPtr, int n, 
		   int startRow, int endRow, double threshold, MPI_Comm comm );
/* Command-line args: matrix size, threshold */
int main(int argc, char * argv[])
{
  int myID, numProcs, n, rowsPerProcess, i;
  int startRow, endRow, recvCount;
  double threshold;
  double *val;      /* 1D array holds 2D (n+2)x(n+2) array in row-major order */
  double *localVal;
  int * sendCounts;
  int * displacements;
  double globalDelta;
  float myThreshold;
  
  MPI_Status status;
  
  MPI_Init(&argc, &argv);  /* Initialize MPI */
  MPI_Comm_size(MPI_COMM_WORLD, &numProcs); /* Get rank */
  MPI_Comm_rank(MPI_COMM_WORLD, &myID); /* Get rank */

  // all processes have access to argc and argv
  sscanf(argv[1], "%d", &n);
  sscanf(argv[2], "%f", &myThreshold);
  threshold = (double) myThreshold;
  rowsPerProcess = n/numProcs;


  sendCounts = (int *) malloc(numProcs*sizeof(int));
  displacements = (int *) malloc(numProcs*sizeof(int));
  displacements[0] = 0;
  sendCounts[0] = (rowsPerProcess+1)*(n+2);
  for (i = 1; i < numProcs - 1; i++) {
    sendCounts[i] = (rowsPerProcess)*(n+2);
    displacements[i] = displacements[i-1] + sendCounts[i-1];
  } // end for
  displacements[numProcs-1] =  displacements[numProcs-2] + sendCounts[numProcs-2];
  sendCounts[numProcs-1] = (n+2)*(n+2) - displacements[numProcs-1];
  
  
  if ( myID == RootProcess ) {       /* Generate initial array val */

    printf("n=%d numProcs=%d threshold=%f\n",n, numProcs, threshold);      
    val = (double *) malloc((n+2)*(n+2)*sizeof(double));

    initializeData(val, n);
    if (n <= 16) {
      printf("Initial val:\n");
      printArray(val, n);
      
      for (i = 0; i < numProcs; i++) {
	printf("%d: sendCnt=%d displacements=%d\n",i,sendCounts[i], displacements[i]);
      } // end for
    } // end if
    
  } // end if

  startRow = 1;
  endRow = rowsPerProcess; 
  if (myID == numProcs - 1) {
    endRow = n - rowsPerProcess * (numProcs-1);
  } // end if
  recvCount = sendCounts[myID];

  if (myID == numProcs-1 || myID == 0) {
    recvCount += n + 2;
  } // end if

  localVal = (double *) malloc((endRow - startRow + 3)*(n+2)*sizeof(double));

  if (myID == 0) {
    MPI_Scatterv(val,sendCounts,displacements, MPI_DOUBLE, 
		 localVal, recvCount, MPI_DOUBLE, RootProcess, MPI_COMM_WORLD);
  } else {
    MPI_Scatterv(val,sendCounts,displacements, MPI_DOUBLE, 
		 localVal+(n+2), recvCount, MPI_DOUBLE, RootProcess, MPI_COMM_WORLD);
  } // end if

  globalDelta = perform2D_SOR(myID, numProcs, &localVal, n, startRow, endRow, threshold, MPI_COMM_WORLD);

  if (myID == numProcs - 1) {
    MPI_Gatherv(localVal + (n+2), (endRow-startRow+2)*(n+2), MPI_DOUBLE, val,
		sendCounts, displacements, MPI_DOUBLE, RootProcess,
		MPI_COMM_WORLD);
    
  } else if (myID == RootProcess) {
    MPI_Gatherv(localVal, (endRow-startRow+2)*(n+2), MPI_DOUBLE, val,
		sendCounts, displacements, MPI_DOUBLE, RootProcess,
		MPI_COMM_WORLD);
    
  } else {
    MPI_Gatherv(localVal + (n+2), (endRow-startRow+1)*(n+2), MPI_DOUBLE, val,
		sendCounts, displacements, MPI_DOUBLE, RootProcess,
		MPI_COMM_WORLD);
  } // end if

  if ( myID == RootProcess ) {       /* Print results */
    
    if (n <= 16) {
      printf("Final val:\n");
      printArray(val, n);
    } // end if
    printf("maximum difference:  %1.5f\n", globalDelta);
  } // end if
  
  MPI_Finalize();
  return 0;
  
} // end main

/* The main already Scatterv'ed the whole array by rows with the parameter
 * resultVarPtr contains a pointer to the memory location containing a pointer 
 * to the 1D array containing our block of rows.  This is a pointer to a pointer because
 * the below wants pass back the resulting block of rows after the globalDelta < threshold.
 * We also return the globalDelta at the end.
 * The main Gatherv's the local chunks back together.  You don't do that here.* 
 * */

double perform2D_SOR(int myID, int numProcs, double **resultValPtr, int n, 
		   int startRow, int endRow, double threshold, MPI_Comm comm ) {
  double average, myDelta, globalDelta;
  double * myNew;
  double * myVal;
  double * temp;
  int i, j, step;
  int downNeighbor;
  int upNeighbor;
  MPI_Status status;
  
  myVal = *resultValPtr;

  downNeighbor = myID + 1;
  upNeighbor = myID - 1;

  myNew = (double *) malloc((n+2) * (n+2) * sizeof(double));

  for (i = 0; i <= (endRow + 1); ++i ) 
  {
    for (j = 0; j <= (n + 1); ++j )
    {
      myNew[i * (n + 2) + j] = myVal[i * (n + 2) + j];
    }
  }

  do
  {
    myDelta = 0.0;
    if (myID == 0)
    {
      MPI_Send( myVal + endRow * (n + 2) , n + 2, MPI_DOUBLE, downNeighbor, 0, comm);
      MPI_Recv( myVal + startRow * (n + 2) , n + 2, MPI_DOUBLE, downNeighbor, 0, comm, &status);
    } else if (myID == (numProcs - 1)){
      MPI_Send( myVal + startRow * (n + 2) , n + 2, MPI_DOUBLE, upNeighbor, 0, comm);
      MPI_Recv( myVal + endRow * (n + 2) , n + 2, MPI_DOUBLE, upNeighbor, 0, comm, &status);
    } else {
      MPI_Send( myVal + endRow * (n + 2) , n + 2, MPI_DOUBLE, upNeighbor, 0, comm);
      MPI_Send( myVal + startRow * (n + 2) , n + 2, MPI_DOUBLE, downNeighbor, 0, comm);
      MPI_Recv( myVal + endRow * (n + 2) , n + 2, MPI_DOUBLE, upNeighbor, 0, comm, &status);
      MPI_Recv( myVal + startRow * (n + 2) , n + 2, MPI_DOUBLE, downNeighbor, 0, comm, &status);
    } 

    for ( i = startRow; i <= endRow; ++i)
    {
      for ( j = 1; j <= n; ++j)
      {
        average = (myVal[(i * (n + 2) + j) + 1] + myVal[i * (n + 2) + j - 1] + myVal[((i - 1) * (n + 2) + j)] + myVal[((i + 1) * (n + 2) + j)]) / 4;
        myNew[(i * (n + 2) + j)] = average;

        if (fabs(average - myVal[(i * (n + 2) + j)]) > myDelta)
        {
          myDelta = fabs(average - myVal[(i * (n + 2) + j)]);
        }

      }
    }
    temp = myNew;
    myNew = myVal;
    myVal = temp;

    MPI_Allreduce(&myDelta, &globalDelta, 1, MPI_DOUBLE, MPI_MAX, comm);
  } while(globalDelta > threshold);
  
  *resultValPtr = myVal;
  return globalDelta;
} // end perform2D_SOR

void initializeData(double *val, int n) {
  int i;

  /* initialize to 0.0, except for 1.0 along the left boundary */
  for (i = 0; i < (n+2)*(n+2); i++) {
    val[i] = 0.0;
  } // end for i
  for (i = 0; i < (n+2)*(n+2); i = i + (n+2)) {
    val[i] = 1.0;
  } // end for i
} // end InitializeData


void printArray(double * val, int n) {

  int i;

  for( i = 0; i < (n+2)*(n+2); i++) {
    printf("%5.2f",val[i]);
    if ((i+1) % (n+2) == 0) {
      printf("\n");
    } // end if
  } // end for
} // end printArray


