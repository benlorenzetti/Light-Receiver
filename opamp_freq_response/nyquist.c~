/* nyquist.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_complex_math.h>
#include "gnuplot_i.h"

#define GIGA  1000000000
#define MEGA  1000000
#define KILO  1000
#define MILLI 0.001
#define MICRO 0.000001
#define NANO  0.000000001
#define PICO  0.000000000001


double GB, R0, R1, R2, R3, C0, C1, C2, C3;
double w0, w1, w2, w3, w4;
double B, D;
double w, f, mag, dB, phase;
gsl_complex L, F, Av;

int main (int argc, char *argv[])
{
  /* Get Component Values from User */
  if (argc != 9)
  {
    printf ("Usage: %s |GB|(MHz) R1(k) R2(k) R3(k) ", argv[0]);
    printf ("C0(pF) C1(nF) C2(pf) C3(nF)\n");
    exit (EXIT_FAILURE);
  } else {
    GB = atof (argv[1]);
    R1 = KILO * atof (argv[2]);
    R2 = KILO * atof (argv[3]);
    R3 = KILO * atof (argv[4]);
    C0 = PICO * atof (argv[5]);
    C1 = NANO * atof (argv[6]);
    C2 = PICO * atof (argv[7]);
    C3 = NANO * atof (argv[8]);
  }

  /* Calculate Poles/Zeros */
  w0 = 2*M_PI*GB*R2 / R1; /* Note: w0 to large for double, so w0 in prog is w0/MEGA */
  w1 = 1 / (R1*C1);
  w2 = 1 / (R2*(MEGA*C2)); /* Note: w2 may be too large, so          */
  w3 = 1 / (R3*C3);        /*       w2 in memory is actually w2/MEGA */
  w4 = 1 / (R2*(MEGA*C0)); /* Note: w4 in memory = w4/MEGA */
  printf ("w0 = %f MHz\nw1 = %d Hz\nw2 = %f MHz\n", w0, (int)w1, w2);
  printf ("w3 = %d Hz\nw4 = %f MHz\n", (int)w3, w4);
}

