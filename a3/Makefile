# Makefile for CSC209 A3 Project

CC = gcc
FLAGS = -Wall -g

.PHONY: all clean

all: a3

a3: main.o jobs.o msg.o node.o parent.o
	$(CC) $(FLAGS) -o a3 main.o jobs.o msg.o node.o parent.o

%.o: %.c ring.h
	$(CC) $(FLAGS) -c $<

clean:
	rm -f *.o a3
