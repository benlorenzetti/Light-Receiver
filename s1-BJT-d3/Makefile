default:	s1_BJT_d3

s1_BJT_d3: s1_BJT_d3.o gnuplot_i.o 
	gcc -static s1_BJT_d3.o gnuplot_i.o -lgsl -lgslcblas -lm -o s1_BJT_d3

s1_BJT_d3.o: s1_BJT_d3.c
	gcc s1_BJT_d3.c -std=c99 -c

gnuplot_i.o: gnuplot_i.c gnuplot_i.h
	gcc -O3 -c gnuplot_i.c
