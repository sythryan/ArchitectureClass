/*  Programmer:  Syth Ryan
    Partial Programmer:  Mark Fienup
    File:        mmult_sequential.c
    Compiled by: gcc -o mmult mmult_sequential.c
    Run by:  mmult 20
    Description:  Matrix Multiplication sequentially.
*/

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

// Prototype of functions
void printMatrix(double ** matrix, int rows, int columns);

int main(int argc, char * argv[]) {

  int n;
  double ** A;  // pointer to matrix A that's n x n
  double ** B;  // pointer to matrix B that's n x n
  double ** C;  // pointer to matrix C that's n x n

  int i, j, r, c, k;

  long startTime, endTime,seqTime;
  
  if (argc != 2) {
    printf("usage: %s <integer size of matrix>\n", argv[0]);
    exit(1);
  } // end ifs
    
  sscanf(argv[1], "%d", &n);
  
  srand(5);  // initialize random number generator 
  srand((long)time(NULL)); /* initialize rand() */

  // dynamically allocate matrices A, B, and C.  

  // I'LL DO A, BUT YOU NEED TO ALLOCATE B AND C.

  A = (double **) malloc(sizeof(double *)*n);
  for (r = 0; r < n; r++) {
    A[r] = (double *) malloc(sizeof(double)*n);
  } // end for r

  B = (double **) malloc(sizeof(double *)*n);
  for (r = 0; r < n; r++) {
    B[r] = (double *) malloc(sizeof(double)*n);
  }

  C = (double **) malloc(sizeof(double *)*n);
  for (r = 0; r < n; r++) {
    C[r] = (double *) malloc(sizeof(double)*n);
  }

  printf("after allocating matrices\n");

  /* initialize array A and B */
  for( r =  0; r < n; r++ ){
    for( c =  0; c < n; c++ ){
      A[r][c] = rand() / (double) RAND_MAX; 
      B[r][c] = rand() / (double) RAND_MAX; 
    } // for c
  } // end for r
  
  printf("after initializing matrices\n");
      
  time(&startTime);

  /* Calculate AND time sequential matrix-multiplication results */
  // ADD CODE HERE TO PERFORM C = A X B MATRIX MULTIPLICATION WITH n X n ARRAYS

  for( r = 0; r < n; r++ ) {
    for( c = 0; c < n; c++ ) {
      for( i = 0; i < n; i++ ) {
        C[r][c] = C[r][c] + A[r][c] * B[c][r];
      }
    }
  }

  time(&endTime);
  seqTime = endTime-startTime;
  printf("Seq time = %ld\n",seqTime);

// FOR DEBUGGING PURPOSES -- CHECK SMALL RESULTS BY HAND

  if (n < 5) { 

    printf("A:\n");
    printMatrix(A, n, n);

    printf("\nB:\n");
    printMatrix(B, n, n);

    printf("\nC:\n");
    printMatrix(C, n, n);

  } // end if


} // end main


void printMatrix(double ** matrix, int rows, int columns) {
  int r, c;

  for( r=0; r<rows ; r++ ){
    for( c=0; c<columns ; c++ ){
      printf("%10.5f   ",matrix[r][c]); 
    } // for c
    printf("\n" );
  } // end for r


} // end printMatrix
