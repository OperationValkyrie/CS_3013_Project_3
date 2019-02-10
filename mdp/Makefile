# Jonathan Chang																																																																																																																																																																																																																																																																																																																																																																																												# Jonathan Chang
# CS 3013 C02
# Project 2

CFLAGS = -Wall -g

all: mdp

mdp: mdp.o
	gcc $(CFLAGS) -pthread mdp.o -o mdp -lm

mdp.o: mdp.c mdp.h
	gcc $(CFLAGS) -c mdp.c

clean:
	rm -f *.o mdp