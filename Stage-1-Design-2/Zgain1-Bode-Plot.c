/*  Zgain1-Bode-Plot.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_complex_math.h>
#include "gnuplot_i.h"

#define MEGA 1000000
#define KILO 1000
#define MILLI 0.001
#define PICO 0.000000000001

const double Rc = 250;
const double f1 = 1000 * MEGA;
const double f2 = 100 * MEGA;
const double Il = 0;

const double Cjc = 4 * PICO;
const double Csigma = 16 * 60 * PICO;
const double Vt = 25 * MILLI;
const double Vcc = 5;
const double BetaAve = 100;
const double Vbe = 0.85;
const double Re = 10;
const double Vmargin = 0.3;

int main (int argc, char *argv[]) {

  // Get the Frequency Range from the User
  double fstart, fend, fscale;
  if (argc != 4) {
    printf ("  Usage: %s startFreq endFreq freqScalingFactor \n", argv[0]);
    printf ("Example: %s 0.1 100 2.0\n", argv[0]);
    exit (EXIT_FAILURE);
  }
  
  fstart = atof (argv[1]);
  fend = atof (argv[2]);
  fscale = atof (argv[3]);
  if (fstart == 0 || fend == 0 || fscale == 0) {
    printf ("Example Usage$ %s 0.1 100 100\n", argv[0]);
    exit (EXIT_FAILURE);
  }
  if (fstart >= fend || fscale <= 1) {
    printf ("startFreq must be less than endFreq.\n");
    exit (EXIT_FAILURE);
  }

  int Npoints = 0;
  double temp = fstart;
  while (temp < fend) {
    Npoints = Npoints + 1;
    temp = temp * fscale;
  }
    
  // Set up the Frequency and Gain Arrays
  double *freq;
  gsl_complex *gain;
  freq = malloc (Npoints * sizeof (*freq));
  gain = malloc (Npoints * sizeof (*gain));
  if (freq == NULL || gain == NULL) {
    printf ("Malloc() failure.\n");
    exit (EXIT_FAILURE);
  }
  freq[0] = fstart;
  for (int i=1; i<Npoints; i++)
    freq[i] = fscale * freq[i-1];

  // Evaluate DC Values
  double Ic, rth, r0, r1, r2, r3, r4, omega1, omega2, A;
  Ic = 2 * M_PI * f2 * Csigma * Vt;
  rth= 1 / (2 * M_PI * f1 * Cjc);
  r4 = rth * Vcc / (Vcc - Ic*Re - 2*Vbe -Vmargin);
  r3 = rth * Vcc / (Ic*Re + 2*Vbe + Vmargin);
  r2 = (10/Ic) * (Vbe + Re*(Ic+Il+ (Vcc-Vbe+Vmargin)/(2*Rc)));
  r1 = 10*Re;
  r0 = (10*(Vcc-Vbe)/Ic) - r1 - r2;
  omega1 = 1 / (rth * Cjc);
  omega2 = Ic / (Csigma * Vt);
  A = 1 + Re*Ic / Vt;
  printf ("Re %f\n", Re);
  printf ("Rc %f\n", Rc);
  printf ("R0 %f\n", r0);
  printf ("R1 %f\n", r1);
  printf ("R2 %f\n", r2);
  printf ("R3 %f\n", r3);
  printf ("R4 %f\n", r4); 
  printf ("Rth = %f Ohms\n", rth);
  printf (" f1 = %f MHz\n", (omega1 / (2*MEGA*M_PI)));
  printf (" f2 = %f MHz\n", (omega2 / (2*MEGA*M_PI)));
  printf ("  A = %f (dimensionless)\n", A);

  // Evaluate the Transfer Function
  for (int i=0; i<Npoints; i++) {
    double omega, theta1, relmag1, relmag2;
    gsl_complex vec1, vec2, vec3, vec4, vec5, vec6;
    omega = 2 * M_PI * freq[i];
    theta1 = (-1) * (M_PI_4 + atan (omega / omega1));
    relmag1 = gsl_hypot (1, (omega1 / omega));
    relmag2 = gsl_hypot (1, (omega / omega1));	
    vec1 = gsl_complex_rect (1/relmag2, A/relmag1);
    vec2 = gsl_complex_polar (omega/omega2, theta1);
    vec3 = gsl_complex_mul (vec1, vec2);
    vec4 = gsl_complex_rect (1, 0);
    vec5 = gsl_complex_add (vec4, vec3);
    vec6 = gsl_complex_mul_real (vec5, (1/Rc));
    gain[i] = gsl_complex_inverse (vec6); 
  }
  // Print the Results to output.txt
  FILE *fp;
  fp = fopen ("gsl-output.txt", "w+");
  if (fp == NULL) {
    printf ("fopen() error.\n");
    exit (EXIT_FAILURE);
  }
  for (int i=0; i<Npoints; i++) {
    double dB = 20 * log10 (gsl_complex_abs (gain[i]));
    double phase = gsl_complex_arg (gain[i]);
    fprintf (fp, "%f\t%f\t%f\n", freq[i], dB, phase);
  }
  fclose (fp);

  // Plot the Frequency Response
  gnuplot_ctrl *mag_plot, *phase_plot;
  mag_plot = gnuplot_init ();
  phase_plot = gnuplot_init ();
  gnuplot_set_xlabel (mag_plot, "Frequency (Hz)");
  gnuplot_set_xlabel (phase_plot, "Frequency (Hz)");
  gnuplot_set_ylabel (mag_plot, "dB Gain");
  gnuplot_set_ylabel (phase_plot, "Phase (Degrees)");
  gnuplot_cmd (mag_plot, "set logscale x");
  gnuplot_cmd (phase_plot, "set logscale x");
  gnuplot_cmd (mag_plot, "set title 'Transimpedance Stage'");
  gnuplot_cmd (phase_plot, "set title 'Transimpedance Stage'");
  gnuplot_cmd (mag_plot, "plot 'gsl-output.txt' using 1:2 title 'Theory' with lines, \\");
  gnuplot_cmd (mag_plot, "'spice-output.data' using 1:2 title 'SPICE' with lines");
  gnuplot_cmd (phase_plot, "plot 'gsl-output.txt' using 1:3 title 'Theory' with lines, \\");
  gnuplot_cmd (phase_plot, "'spice-output.data' using 3:4 title 'SPICE' with lines");
  printf ("Return to Exit -> ");
  int stay_open = getc (stdin);
  gnuplot_close (mag_plot);
  gnuplot_close (phase_plot);

  // Release Memory
  free (freq);
  free (gain);    

}
