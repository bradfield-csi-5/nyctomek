/*
Naive code for multiplying two matrices together.

There must be a better way!
*/

#include <stdio.h>
#include <stdlib.h>

/*
  A naive implementation of matrix multiplication.

  DO NOT MODIFY THIS FUNCTION, the tests assume it works correctly, which it
  currently does
*/
void matrix_multiply(double **C, double **A, double **B, int a_rows, int a_cols,
                     int b_cols) {
  for (int i = 0; i < a_rows; i++) {
    for (int j = 0; j < b_cols; j++) {
      C[i][j] = 0;
      for (int k = 0; k < a_cols; k++)
        C[i][j] += A[i][k] * B[k][j];
    }
  }
}

void fast_matrix_multiply(double **C, double **A, double **B, int a_rows,
                          int a_cols, int b_cols) {
  for(int i = 0; i < a_rows; i++) {
    for(int j = 0; j < a_cols; j++) {
      C[i][j] = 0;
    }
  } 

  for (int i = 0; i < a_rows; i++) {
    for (int j = 0; j < b_cols; j++) {
      for (int k = 0; k < a_cols; k++) {
        C[i][k] += A[i][j] * B[j][k];
      }
    }
  }
}
