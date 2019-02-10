# Jonathan Chang																																																																																																																																																																																																																																																																																																																																																																																												# Jonathan Chang
# CS 3013 C02
# Project 2

CFLAGS = -Wall -g

all: pvn

pvn: pvn.o
	gcc $(CFLAGS) -pthread pvn.o -o pvn -lm

pvn.o: pvn.c pvn.h
	gcc $(CFLAGS) -c pvn.c

clean:
	rm -f *.o pvn