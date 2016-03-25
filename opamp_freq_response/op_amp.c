/* op_amp.c
 *
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

const double C0 = 20 * PICO;
const double R0 = 20;
const double GB = 125 * MEGA;

double R1, R2, R3, C1, C2, C3;
double w1, w2, w3;
double w, f, mag, dB, phase;
gsl_complex Z1, Z2, Z3, Av, x1, x2, x3, x4, temp;

int main (int argc, char *argv[])
{
  /* Get Component Values from User */
  if (argc != 7)
  {
    printf ("Usage: %s R1(k) R2(k) R3(k) C1(nF) C2(pf) C3(nF)\n", argv[0]);
    exit (EXIT_FAILURE);
  } else {
    R1 = KILO * atof (argv[1]);
    R2 = KILO * atof (argv[2]);
    R3 = KILO * atof (argv[3]);
    C1 = NANO * atof (argv[4]);
    C2 = PICO * atof (argv[5]);
    C3 = NANO * atof (argv[6]);
  }

  /* Calculate Poles/Zeros */
  w1 = 1 / (R1*C1);
  w2 = 1 / (R2*C2);
  w3 = 1 / (R3*C3);

  /* Open Data File */
  FILE *fp;
  fp = fopen ("theory.data", "w");
  if (fp == NULL)
  {
    printf ("fopen() error.\n");
    exit (EXIT_FAILURE);
  }

  /* Calculate Theoretical Data */
  f = 1;
  while (f < GIGA)
  {
    w = 2 * M_PI * f;
    Z1 = gsl_complex_rect (R1, -R1*w1/w);

    Z2 = gsl_complex_rect (1, w/w2);
    temp = gsl_complex_rect (R2, 0);
    Z2 = gsl_complex_div (temp, Z2);

    Z3 = gsl_complex_rect (R3, -R3*w3/w);

    x1 = gsl_complex_mul (Z1, Z2);
    temp = gsl_complex_add (Z1, Z2);
    x1 = gsl_complex_div (x1, temp);

    x2 = gsl_complex_mul_real (x1, w*C0);
    temp = gsl_complex_rect (0, -1);
    x2 = gsl_complex_mul (x1, temp);
    temp = gsl_complex_rect (1, 0);
    x2 = gsl_complex_add (x1, temp);

    x3 = gsl_complex_rect (0, -R0*f/GB);
    x3 = gsl_complex_div (x3, Z2);
    temp = gsl_complex_rect (1, 0);
    x3 = gsl_complex_add (temp, x3);

    x4 = gsl_complex_rect (0, f*R0/GB);
    x4 = gsl_complex_div (x4, x1);
    temp = gsl_complex_add (Z2, Z3);
    x4 = gsl_complex_mul (x4, temp);
    x4 = gsl_complex_div (x4, Z3);

    Av = gsl_complex_div (x2, x3);
    Av = gsl_complex_mul (Av, x4);
    temp = gsl_complex_rect (1, 0);
    Av = gsl_complex_add (Av, temp);
    temp = gsl_complex_rect (1, 0);
    Av = gsl_complex_div (temp, Av);
    Av = gsl_complex_mul (Av, Z2);
    Av = gsl_complex_mul_real (Av, -1);
    Av = gsl_complex_div (Av, Z1);

    mag = gsl_complex_abs (Av);
    dB = 20 * log10 (mag);
    phase = gsl_complex_arg (Av);
    fprintf (fp, "%f\t%f\t%f\t%f\n", f, dB, f, phase);

    f = f * 1.1;
  }

  /* Close Theory Data File and Open Spice Netlist File */
  fclose (fp);
  char netlist_name[1000];
  strcpy (netlist_name, argv[0]);
  strcat (netlist_name, ".cir");
  fp = fopen (netlist_name, "w");
  if (fp == NULL)
  {
    printf ("fopen() error.\n");
    exit (EXIT_FAILURE);
  }

  /* Prepare SPICE Netlist */
  strcpy (netlist_name, "*** ");
  strcat (netlist_name, (2 + argv[0]));
  strcat (netlist_name, ".cir ***");
  fprintf (fp, "%s\n\n", netlist_name);
  fprintf (fp, "Vcc cc 0 DC 5\n");
  fprintf (fp, "Vref 4 0 DC 2.5\n");
  fprintf (fp, "Vin in 0 DC 0 AC 1 PULSE (0mV 1uV 0s 1ns 1n2 50us 100us)\n");
  fprintf (fp, "Rin 1 in 100\n\n");
  fprintf (fp, "C1 1 2 %fn\n", C1/NANO);
  fprintf (fp, "R1 2 3 %fk\n", R1/KILO);
  fprintf (fp, "R2 3 6 %fk\n", R2/KILO);
  fprintf (fp, "C2 3 6 %fp\n", C2/PICO);
  fprintf (fp, "R3 7 0 %fk\n", R3/KILO);
  fprintf (fp, "C3 6 7 %fn\n", C3/NANO);
  fprintf (fp, "\n.include ./lm7171.cir\n");
  fprintf (fp, "X1 4 3 cc 0 6 LM7171A\n");
  fprintf (fp, "\n.control\n");
  fprintf (fp, "ac dec 100 1 100G\n");
  fprintf (fp, "*plot vdb(6)\n*plot vp(6)\n");
  fprintf (fp, "wrdata spice vdb(6) vp(6)\n");
  fprintf (fp, "*tran 1us 1000us\n*plot v(1) v(6)\n");

  /* Close SPICE Netlist File */
  fclose (fp);

  /* Run NGSPICE */
  char ngspice_call[1000];
  strcpy (ngspice_call, "ngspice ");
  strcat (ngspice_call, argv[0]);
  strcat (ngspice_call, ".cir");
  if (-1 == system (ngspice_call))
  {
    printf ("system() call error.\n");
    exit (EXIT_FAILURE);
  }

  /* Plot the Frequency Response */
  gnuplot_ctrl *mag_plot, *phase_plot;
  mag_plot = gnuplot_init ();
  phase_plot = gnuplot_init ();
  gnuplot_set_xlabel (mag_plot, "Frequency (Hz)");
  gnuplot_set_xlabel (phase_plot, "Frequency (Hz)");
  gnuplot_set_ylabel (mag_plot, "Magnitude Gain (dB)");
  gnuplot_set_ylabel (phase_plot, "Phase Shift (Degrees)");
  gnuplot_cmd (mag_plot, "set logscale x");
  gnuplot_cmd (phase_plot, "set logscale x");
  gnuplot_cmd (mag_plot, "set title 'Op-Amp Stage'");
  gnuplot_cmd (phase_plot, "set title 'Op-Amp Stage'");
  gnuplot_cmd (mag_plot, "plot 'theory.data' using 1:2 title 'Theory' with lines, \\");
  gnuplot_cmd (mag_plot, "'spice.data' using 1:2 title 'SPICE' with lines");
  gnuplot_cmd (phase_plot, "plot 'theory.data' using 3:4 title 'Theory' with lines, \\");
  gnuplot_cmd (phase_plot, "'spice.data' using 3:4 title 'SPICE' with lines");
  printf ("Return to Exit -> ");
  int stay_open = getc (stdin);
  gnuplot_close (mag_plot);
  gnuplot_close (phase_plot);
}
