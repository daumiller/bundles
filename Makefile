objects : bundle.o \
          wide.o

bundle.o : bundle.c
	gcc -c bundle.c -o bundle.o -std=c99

wide.o : wide.c
	gcc -c wide.c -o wide.o -std=c99

test : test.c objects mxml.a hashtable.a
	rm -f test.exe
	gcc -c test.c -o test.o -std=c99
	gcc wide.o bundle.o test.o mxml.a hashtable.a -o test.exe

clean :
	rm -f *.o
	rm -f *.exe
