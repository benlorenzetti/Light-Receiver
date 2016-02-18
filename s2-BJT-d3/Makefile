default:	s2-BJT-d3

s2-BJT-d3: s2-BJT-d3.o gnuplot_i.o 
	gcc -static s2-BJT-d3.o gnuplot_i.o -lgsl -lgslcblas -lm -o s2-BJT-d3

s2-BJT-d3.o: s2-BJT-d3.c
	gcc s2-BJT-d3.c -std=c99 -c

gnuplot_i.o: gnuplot_i.c gnuplot_i.h
	gcc -O3 -c gnuplot_i.c
clean:
	rm gnuplot_i.o s2-BJT-d3.o s2-BJT-d3
