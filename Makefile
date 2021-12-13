CC=gcc -g 
cc=gcc -g

sp-O: misc.o display.o linear.o first.o plane.o xyz.o main.o graphics.o poly.o sig.o
	gcc -o sp-O sig.o misc.o display.o linear.o first.o plane.o xyz.o main.o poly.o graphics.o -lm -L/usr/X11R6/lib -lX11

sig.o:       sig.c
display.o:   display.cc  sp1.h spprocs.h
first.o:     first.cc    sp1.h spprocs.h
graphics.o:  graphics.cc sp1.h spprocs.h graphics.h polyicon.h
linear.o:    linear.cc   sp1.h spprocs.h
main.o:      main.cc     sp1.h spprocs.h graphics.h
misc.o:      misc.cc     sp1.h spprocs.h
plane.o:     plane.cc    sp1.h spprocs.h
poly.o:      poly.cc     sp1.h spprocs.h
xyz.o:       xyz.cc      sp1.h spprocs.h

.cc.o:
	$(CC) -c $*.cc

.c.o:
	$(cc) -c $*.c
