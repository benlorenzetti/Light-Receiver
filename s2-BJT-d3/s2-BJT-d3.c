/*  s2-BJT-d3.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_complex_math.h>
#include "gnuplot_i.h"

#define MEGA 1000000
#define KILO 1000
#define MILLI 0.001
#define MICRO 0.000001
#define NANO 0.000000001
#define PICO 0.000000000001

const double Ic1 = 16.6 * MILLI;
const double Re1 = 10;
const double Vc1= 3.075;
const double q3beta = 100;

const char ptsPerDecade[] = "100";
const char Fstart[] = "10";
const char Fend[] = "1000Meg";

const double Cjc = 4 * PICO;
const double Vt = 25 * MILLI;
const double Vcc = 5;
const double Vbe = 0.85;
const double Vmargin = 0.3;

int main (int argc, char *argv[]) {

  // Get Rc1 and the Pole locations from the User
  double Rc1, f3, f4, f5, f6;
  if (argc != 6) {
    printf ("  Usage: %s Rc f3 f4 f5 f6\n", argv[0]);
    printf ("Example: %s 250 1000000000 100 1000 1000000000\n", argv[0]);
    exit (EXIT_FAILURE);
  }

  Rc1 = atof (argv[1]);
  f3 = atof (argv[2]);
  f4 = atof (argv[3]);
  f5 = atof (argv[4]);
  f6 = atof (argv[5]);
  
  // Evaluate DC Values
  double omega3, omega4, omega5, omega6, Ic3, Re3, Rc4, R5, R7, R8, Ce3, C5, L6, Vmirror;
  omega3 = 2 * M_PI * f3;
  omega4 = 2 * M_PI * f4;
  omega5 = 2 * M_PI * f5 / 43;
  omega6 = 2 * M_PI * f6;
  Ic3 = Vt * Cjc * omega3;
  Re3 = Re1 * (Ic1 / Ic3);
  Rc4 = (3*Vbe + 2*Vmargin) / (2 * Ic3);
  R5 = (q3beta/Ic3) * (Vc1 - 2*Vbe - Vmargin);
  R7 = Vt*q3beta*Vcc / (10 * Ic3 * (3*Vbe + 2*Vmargin));
  R8 = Vt*q3beta*Vcc / (10 * Ic3 * (Vcc - 3*Vbe - 2*Vmargin));
  Ce3 = Ic3 / (omega4 * (Vc1-2*Vbe-Vmargin));
  C5 = (43 * omega3 * Vt * Cjc) / (q3beta * omega5 * (Vc1-2*Vbe-Vmargin));
  L6 = 1 / (C5 * omega6 * omega6);
  Vmirror = Ic1 * Re1 + Vbe;
  printf ("f3 = %f MHz\nf4 = %f Hz\nf5 = %f Hz\nf6 = %f MHz\n", f3/MEGA, f4, f5, f6/MEGA);
  printf ("Ic3= %f mA\nL6 = %f nF\nCe3=%f uF\nC5 =%f uF\n", Ic3/MILLI, L6/NANO, Ce3/MICRO, C5/MICRO);

  // Prepare SPICE Netlist
  char netlist_name[1000];
  strcpy (netlist_name, argv[0]);
  strcat (netlist_name, ".cir"); 
  FILE *fp;
  fp = fopen (netlist_name, "w+");
  if (fp == NULL) {
    printf ("fopen() error.\n");
    exit (EXIT_FAILURE);
  }
  
  fprintf (fp, "*** %s ***\n", argv[0]);
  fprintf (fp, "*f3=%-li, f4=%-li, f5=%-li, f6=%-li\n", (long)f3, (long)f4, (long)f5, (long)f6);
  fprintf (fp, "Vcc cc gnd DC 5\n");
  fprintf (fp, "Vc1 indc gnd DC %f\n", Vc1);
  fprintf (fp, "Vin in indc 1m AC 1 SIN (0 1 10Meg)\n");
  fprintf (fp, "Vmirror mirror gnd DC %f\n", Vmirror);
  fprintf (fp, "Rc1 c1 in %f\n", Rc1);
  fprintf (fp, "L6 c1 n5 %fn\n", L6/NANO);
  fprintf (fp, "R5 n5 b3 %fk\n", R5/KILO);
  fprintf (fp, "C5 n5 b3 %fn\n", C5/NANO);
  fprintf (fp, "R7 cc b4 %f\n", R7);
  fprintf (fp, "R8 b4 gnd %f\n", R8);
  fprintf (fp, "Rc4 cc out %f\n", Rc4);
  fprintf (fp, "Re3 me3 gnd %f\n", Re3);
  fprintf (fp, "Ce3 e3 gnd %fu\n", Ce3/MICRO);
  fprintf (fp, "\nQ3 c3 b3 e3 2n3904\n");
  fprintf (fp, "Qm3 e3 mirror me3 2n3904\n");
  fprintf (fp, "Q4 out b4 c3 2n3904\n");
  fprintf (fp, ".model 2n3904 npn (bf=%f vaf=200 cje=18p cjc=4p)\n", q3beta);
  fprintf (fp, ".control\n");
  fprintf (fp, "ac dec %s %s %s\n", ptsPerDecade, Fstart, Fend);
  fprintf (fp, "plot vdb(out)\nplot vp(out)\n");
  fprintf (fp, "wrdata spice-output vdb(out) vp(out)\n");
  
  fclose (fp);

  // Run NGSPICE
  char ngspice_call[1000];
  strcpy (ngspice_call, "ngspice ");
  strcat (ngspice_call, netlist_name);
  if (-1 == system (ngspice_call)) {
    printf ("Error system() returned -1\n");
    exit (EXIT_FAILURE);
  }
}
