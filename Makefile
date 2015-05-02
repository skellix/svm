SHELL = /bin/sh
.SUFFIXES: .c .o
CC = gcc
srcdir = .


svm.o: svm.c *.h
	$(CC) -g -std=c99 -O0 -c $< -o $@

svm: svm.o
	gcc -g -I$(srcdir) -L/usr/lib -lgc -lm $< -o $@
