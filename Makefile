SHELL = /bin/sh
.SUFFIXES: .c .o
CC = gcc
srcdir = bin
includeDir = include
debug = -g
std = -std=c99


svm: $(srcdir)/svm.c
	rm -rf /usr/bin/$@
	$(CC) $(debug) $(std) -c $< -o $@.o -I$(srcdir) -I$(includeDir)
	$(CC) $(debug) $(std) $@.o -o $@ -I$(srcdir) -I$(includeDir) -L/usr/lib -ldl -lsvm
	mv $@ /usr/bin

libsvm: $(srcdir)/stdfuncs.c $(srcdir)/stack.c $(srcdir)/interrupt.c $(srcdir)/error.c
	rm -rf /usr/lib/$@.so
	$(CC) -fPIC $(debug) $(std) -c $^ -I$(srcdir) -I$(includeDir) -L/usr/lib -ldl
	$(CC) -shared -Wl,-soname,$@.so -o $@.so stdfuncs.o stack.o interrupt.o error.o
	mv $@.so /usr/lib

libsvm.bytecode: $(srcdir)/bytecode.new.c
	rm -rf /usr/lib/$@.so
	$(CC) -fPIC $(debug) $(std) -c $^ -I$(srcdir) -I$(includeDir) -L/usr/lib -ldl -lsvm
	$(CC) -shared -Wl,-soname,$@.so -o $@.so bytecode.new.o
	mv $@.so /usr/lib

libsvm.debug: $(srcdir)/debug.c
	rm -rf /usr/lib/$@.so
	$(CC) -fPIC $(debug) $(std) -c $^ -I$(srcdir) -I$(includeDir) -L/usr/lib -ldl -lsvm
	$(CC) -shared -Wl,-soname,$@.so -o $@.so debug.o
	mv $@.so /usr/lib

libsvm.io: $(srcdir)/io.c
	rm -rf /usr/lib/$@.so
	$(CC) -fPIC $(debug) $(std) -c $^ -I$(srcdir) -I$(includeDir) -L/usr/lib -ldl -lsvm
	$(CC) -shared -Wl,-soname,$@.so -o $@.so io.o
	mv $@.so /usr/lib

clean:
	rm -rf *.o

install: libsvm libsvm.bytecode libsvm.debug libsvm.io svm
	echo done
