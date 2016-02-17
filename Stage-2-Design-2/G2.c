/*  G2.c
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
#define MICRO 0.000001
#define PICO 0.000000000001

const double Rc1 = 250;
const double Ic1 = 16.6 * MILLI;
const double Vc1= 3.075;
const double Ic3= 1 * MILLI;
const double q3beta = 100;
const double f3 = 1 * KILO;
const double f4 = 100;

const double Cjc = 4 * PICO;
const double Vt = 25 * MILLI;
const double Vcc = 5;
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
  gsl_complex *gain, *high_gain;
  freq = malloc (Npoints * sizeof (*freq));
  gain = malloc (Npoints * sizeof (*gain));
  high_gain = malloc (Npoints * sizeof (*high_gain));
  if (freq == NULL || gain == NULL || high_gain == NULL) {
    printf ("Malloc() failure.\n");
    exit (EXIT_FAILURE);
  }
  freq[0] = fstart;
  int i;
  for (i=1; i<Npoints; i++)
    freq[i] = fscale * freq[i-1];

  // Evaluate DC Values
  double Re3, R4, Ce3, C4, Rc3, R5, R6, omega3, omega4, omega5, Vmirror;
  Re3 = Re * (Ic1 / Ic3);
  R4 = (q3beta/Ic3) * (Vc1 - 2*Vbe - Vmargin) - Rc1;
  Ce3 = (5*Ic3) / (M_PI*(Vc1-2*Vbe-Vmargin)*f3);
  C4 = (5*Ic3) / (M_PI * q3beta * (Vc1-2*Vbe-Vmargin) * f4);
  Rc3 = (3*Vbe + 2*Vmargin) / (2 * Ic3);
  R5 = Vt*q3beta*Vcc / (10 * Ic3 * (3*Vbe + 2*Vmargin));
  R6 = Vt*q3beta*Vcc / (10 * Ic3 * (Vcc - 3*Vbe - 2*Vmargin));
  omega3 = Ic3 / (Ce3 * (Vc1-2*Vbe-Vmargin));
  omega4 = 1 / (R4 * C4);
  omega5 = Ic3 / (Vt*Cjc);
  Vmirror = Ic1 * Re + Vbe;

  printf (".param q3beta=%f\n", q3beta);
  printf ("Vc1 in gnd DC %f\n", Vc1);
  printf ("Vmirror mirror gnd DC %f\n", Vmirror);
  printf ("Rc1 inac c1 %f\n", Rc1);
  printf ("R4 c1 b3 %fk\n", R4/KILO);
  printf ("C4 c1 b3 %fp\n", C4/PICO);
  printf ("Rc3 cc out %f\n", Rc3);
  printf ("Re3 me3 gnd %f\n", Re3);
  printf ("Ce3 e3 gnd %fu\n", Ce3/MICRO);
  printf ("R5 b4 cc %f\n", R5);
  printf ("R6 b4 gnd %f\n", R6);


  printf ("f3 = %f kHz\n", (omega3 / (2*M_PI)) / KILO);
  printf ("f4 = %f kHz\n", (omega4 / (2*M_PI)) / KILO);
  printf ("f5 = %f MHz\n", (omega5 / (2*M_PI)) / MEGA);

  // Evaluate the Transfer Function(s)
  for (i=0; i<Npoints; i++) {
    double omega, mag1;
    gsl_complex vec1, vec2, vec3, vec4, vec5, vec6;
    omega = 2 * M_PI * freq[i];
    // The Low Frequency Transfer Function
    vec1 = gsl_complex_polar (1/gsl_hypot (1, (omega/omega4)), (-1)*atan (omega/omega4));
    vec2 = gsl_complex_rect (0, -omega3/omega);
    vec3 = gsl_complex_add (vec1, vec2);
    vec4 = gsl_complex_mul_real (vec3, -(Vc1-2*Vbe-Vmargin) / (Ic3*Rc3));
    gain[i] = gsl_complex_inverse (vec4);
    // The High Frequency Transfer Function
    mag1 = (1 + Vt/(Rc1*Ic3)) * (gsl_hypot (1, omega/omega5));
    vec1 = gsl_complex_polar (-mag1, 2 * atan (omega/omega5));
    vec2 = gsl_complex_rect (1, 0);
    vec3 = gsl_complex_add (vec1, vec2);
    vec4 = gsl_complex_mul_real (vec3, Rc1 / Rc3);
    high_gain[i] = gsl_complex_inverse (vec4);
  }

  // Print the Results to output.txt
  FILE *fp;
  fp = fopen ("gsl-output.txt", "w+");
  if (fp == NULL) {
    printf ("fopen() error.\n");
    exit (EXIT_FAILURE);
  }
  for (i=0; i<Npoints; i++) {
    double dB = 20 * log10 (gsl_complex_abs (gain[i]));
    double phase = gsl_complex_arg (gain[i]);
    double dB_high = 20 * log10 (gsl_complex_abs (high_gain[i]));
    double phase_high = gsl_complex_arg (high_gain[i]);
    fprintf (fp, "%f\t%f\t%f\t%f\t%f\n", freq[i], dB, phase, dB_high, phase_high);
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
  gnuplot_cmd (mag_plot, "plot 'gsl-output.txt' using 1:2 title 'Theory (low freq)' with lines, \\");
  gnuplot_cmd (mag_plot, "'gsl-output.txt' using 1:4 title 'Theory (high freq)' with lines, \\");
  gnuplot_cmd (mag_plot, "'spice-output.data' using 1:2 title 'SPICE' with lines");
  gnuplot_cmd (phase_plot, "plot 'gsl-output.txt' using 1:3 title 'Theory (low freq)' with lines, \\");
  gnuplot_cmd (phase_plot, "'gsl-output.txt' using 1:5 title 'Theory (high freq)' with lines, \\");
  gnuplot_cmd (phase_plot, "'spice-output.data' using 3:4 title 'SPICE' with lines");
  printf ("Return to Exit -> ");
  int stay_open = getc (stdin);
  gnuplot_close (mag_plot);
  gnuplot_close (phase_plot);

  // Release Memory
  free (freq);
  free (gain);    

}
