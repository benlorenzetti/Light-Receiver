default:	op_amp nyquist

op_amp:	op_amp.o gnuplot_i.o
	gcc -static op_amp.o gnuplot_i.o -lgsl -lgslcblas -lm -o op_amp

op_amp.o: op_amp.c
	gcc op_amp.c -std=c99 -c

nyquist: nyquist.o gnuplot_i.o
	gcc -static nyquist.o gnuplot_i.o -lgsl -lgslcblas -lm -o nyquist

nyquist.o: nyquist.c
	gcc nyquist.c -std=c99 -c

gnuplot_i.o:
	gcc -O3 -c gnuplot_i.c

clean:
	rm gnuplot_i.o op_amp.o op_amp
