objects : bundle.o \
          wide.o   \
          test.o

test.o : test.c bundle.h
	gcc -c test.c -o test.o -std=c99

bundle.o : bundle.c
	gcc -c bundle.c -o bundle.o -std=c99

wide.o : wide.c
	gcc -c wide.c -o wide.o -std=c99

test.exe : objects mxml.a hashtable.a
	rm -f test.exe
	gcc wide.o bundle.o test.o mxml.a hashtable.a -o test.exe

clean :
	rm -f *.o
	rm -f *.exe
