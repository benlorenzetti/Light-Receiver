default:	Zgain1-Bode-Plot

Zgain1-Bode-Plot: Zgain1-Bode-Plot.o gnuplot_i.o 
	gcc -static Zgain1-Bode-Plot.o gnuplot_i.o -lgsl -lgslcblas -lm -o Zgain1-Bode-Plot

Zgain1-Bode-Plot.o: Zgain1-Bode-Plot.c
	gcc Zgain1-Bode-Plot.c -std=c99 -c

gnuplot_i.o: gnuplot_i.c gnuplot_i.h
	gcc -O3 -c gnuplot_i.c
clean:
	rm gnuplot_i.o Zgain1-Bode-Plot.o Zgain1-Bode-Plot
