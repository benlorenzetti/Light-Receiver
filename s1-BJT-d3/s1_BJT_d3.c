/*  s1_BJT_d3.c
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_complex_math.h>
#include "gnuplot_i.h"

#define MEGA 1000000
#define KILO 1000
#define MILLI 0.001
#define MICRO 0.000001
#define NANO 0.000000001
#define PICO 0.000000000001

#define START_FREQ 100
#define END_FREQ 1000000000

/* Constant Physical/Design Parameters */
const double Vout1 = 2.5;
const double Beta = 100;
const double Cc = 4 * PICO;
const double Ce = 18 * PICO;
const double Csigma = 960 * PICO;
const double Vt = 25 * MILLI;
const double Vcc = 5;
const double Vbe = 0.85;
const double Vb1 = 2;
const double Vmirror = 1;

/* Pole Locations in Gain Equation */
double f0, f1, f2, f3, f33;
/* Coefficients which are on the order of 1 */
double A, D, E, F, G, H, I, J, K, alpha;
/* Design Values */
double Ic1, I0, I1;
/* Component Values */
double R0, R1, R2, Re0, Re2, Rc1, C1;


int main (int argc, char *argv[]) {

  /* Get Desired Pole Locations from User */
  if (argc != 3) {
    printf ("\nUsage: %s (R1+R2) f0 f3 \n", argv[0]);
    printf ("Where f0 is the dominant high pole related to Csigma and Ic,\n");
    printf ("and f3 is a second high pole related to");
    printf (" output capacitance of BJT.\n\n");
    exit (EXIT_FAILURE);
  }
  
  f0 = atof (argv[1]);
  f3 = atof (argv[2]);

  /* Calculate Design & Component Values */
  A = D = E = F = G = H = K = 1;

  alpha = 1 - Csigma / (Beta * Ce);
  Ic1= (2*M_PI*f0) * sqrt ((Cc*Csigma*Vt*Vout1) / H);
  Rc1= (Vcc - Vout1) / Ic1;
  f1 = (H * Ic1) / (2 * M_PI * Cc * Vt);
  f2 = 1 / (2 * M_PI * Rc1 * Csigma);
  C1 = 1 / (K * Rc1 * 2 * M_PI * f3);
  f33= f3 * (K*alpha / G);
  Re2= (Vmirror - Vbe) / Ic1;
  I0 = Ic1 / 10;
  Re0= (Vmirror - Vbe) / I0;
  R0 = (Vcc - Vmirror) / I0;
  I1 = Ic1 / 10;
  R2 = (Vcc / I1) * (Vb1 / Vcc);
  R1 = (Vcc / I1) - R2;

  printf ("Ic1 (mA)  = %f\n", Ic1/MILLI);
  printf ("alpha ()  = %f\n", alpha);
  printf ("C1  (pF)  = %f\n", C1/PICO);
  printf ("f0  (MHz) = %f\n", f0/MEGA);
  printf ("f1  (MHz) = %f\n", f1/MEGA);
  printf ("f2  (MHz) = %f\n", f2/MEGA);
  printf ("f3  (MHz) = %f\n", f3/MEGA);
  printf ("f33 (MHz) = %f\n", f33/MEGA);
   
  /* Generate SPICE netlist */
  char netlist_name[1000];
  strcpy (netlist_name, argv[0] + 2);
  strcat (netlist_name, ".cir");
  FILE *fp;
  fp = fopen (netlist_name, "w+");
  if (fp == NULL)
  {
    printf ("fopen() error.\n");
    exit (EXIT_FAILURE);
  }
  printf ("Generating netlist %s\n", netlist_name);
  printf ("Nodes are: in out1 cc b1 mirror e2 e0\n");

  fprintf (fp, "*** %s ***\n", netlist_name);
  fprintf (fp, "Vcc cc gnd DC 5\n\n");
  fprintf (fp, "Isignal in gnd AC 1 SIN (0 1n 10Meg)\n");
  fprintf (fp, "Iambient in gnd DC 1u\n");
  fprintf (fp, "Csigma in gnd %fp\n\n", Csigma/PICO);
  fprintf (fp, "R0 cc mirror %d\n", (int) R0);
  fprintf (fp, "Re0 e0 gnd %d\n", (int) Re0);
  fprintf (fp, "Re2 e2 gnd %d\n", (int) Re2);
  fprintf (fp, "Rc1 cc out1 %d\n", (int) Rc1);
  fprintf (fp, "R1 cc b1 %d\n", (int) R1);
  fprintf (fp, "R2 b1 gnd %d\n", (int) R2);
  fprintf (fp, "C1 b1 gnd %fp\n\n", C1/PICO);
  fprintf (fp, "Q0 mirror mirror e0 model1\n");
  fprintf (fp, "Q1 out1 b1 in model1\n");
  fprintf (fp, "Q2 in mirror e2 model1\n");
  fprintf (fp, ".model model1 npn (bf=%d vaf=200 cje=%fp cjc=%fp)\n", (int) Beta, Ce/PICO, Cc/PICO);
  fprintf (fp, "\n.control\n");
  fprintf (fp, "ac dec 100 %d %d\n", START_FREQ, END_FREQ);
  fprintf (fp, "wrdata spice_output vdb(out1) vp(out1)\n");

  fclose (fp);

  /* Run NGSPICE */
  char ngspice_call[1000];
  strcpy (ngspice_call, "ngspice ");
  strcat (ngspice_call, netlist_name);
  if (-1 == system (ngspice_call))
  {
    printf ("Error system() returned -1.\n");
    exit (EXIT_FAILURE);
  }
 
  // Set up the Frequency and Gain Arrays
  int  Npoints;
  double f_domain;
  f_domain = START_FREQ;
  Npoints = 1;
  while (f_domain < END_FREQ)
  {
    f_domain = (START_FREQ * pow (1.1, Npoints));
    Npoints = 1 + Npoints;
  }
  double *freq;
  gsl_complex *Z;
  freq = malloc (Npoints * sizeof (*freq));
  Z = malloc (Npoints * sizeof (*Z));
  if (freq == NULL || Z == NULL) {
    printf ("Malloc() failure.\n");
    exit (EXIT_FAILURE);
  }

  freq[0] = START_FREQ;
  int i;
  for (i=1; i<Npoints; i++)
  {
    freq[i] = 1.1 * freq[i-1];
  }

  // Evaluate the Transfer Function(s)
  for (i=0; i<Npoints; i++) {
    double mag1, phase1;
    gsl_complex num1, num2, num, den1, den;
    mag1 = sqrt ( (1+(freq[i]/f1)*(freq[i]/f1)) * (1+(freq[i]/f33)*(freq[i]/f33)) );
    mag1 = mag1 * (H*alpha*Ce) / (F*C1);
    phase1 = (M_PI/4) + atan (f1/freq[i]) + atan (freq[i]/f33);
    num1 = gsl_complex_polar (mag1, phase1);
    num2 = gsl_complex_rect (Rc1, 0);
    num  = gsl_complex_add (num1, num2);
    den1 = gsl_complex_rect (1 - (freq[i]/f0)*(freq[i]/f0), freq[i]/f3);
    den  = gsl_complex_add (den1, num1);
    Z[i] = gsl_complex_div (num, den);
  }

  // Print the Results to output.txt
  fp = fopen ("gsl_output.txt", "w+");
  if (fp == NULL) {
    printf ("fopen() error.\n");
    exit (EXIT_FAILURE);
  }
  for (i=0; i<Npoints; i++) {
    double dB = 20 * log10 (gsl_complex_abs (Z[i]));
    double phase = gsl_complex_arg (Z[i]);
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
  gnuplot_cmd (mag_plot, "plot 'gsl_output.txt' using 1:2 title 'Theory' with lines, \\");
  gnuplot_cmd (mag_plot, "'spice_output.data' using 1:2 title 'SPICE' with lines");
  gnuplot_cmd (phase_plot, "plot 'gsl_output.txt' using 1:3 title 'Theory' with lines, \\");
  gnuplot_cmd (phase_plot, "'spice_output.data' using 3:4 title 'SPICE' with lines");
  printf ("Return to Exit -> ");
  int stay_open = getc (stdin);
  gnuplot_close (mag_plot);
  gnuplot_close (phase_plot);

  // Release Memory
  free (freq);
  free (Z);    

}
