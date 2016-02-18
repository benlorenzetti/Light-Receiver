default:	G2

G2: G2.o gnuplot_i.o 
	gcc -static G2.o gnuplot_i.o -lgsl -lgslcblas -lm -o G2

G2.o: G2.c
	gcc G2.c -std=c99 -c

gnuplot_i.o: gnuplot_i.c gnuplot_i.h
	gcc -O3 -c gnuplot_i.c
clean:
	rm gnuplot_i.o G2.o G2
