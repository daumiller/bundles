statics = mxml.a hashtable.a

objects = bundle.o wide.o

bundle.o : bundle.c
	gcc -c bundle.c -o bundle.o -std=c99

wide.o : wide.c
	gcc -c wide.c -o wide.o -std=c99

launch : Bundle_Launch.c $(objects) $(statics)
	rm -f Bundle_Launch.exe
	gcc -c Bundle_Launch.c -o Bundle_Launch.o -std=c99
	gcc Bundle_Launch.o $(objects) $(statics) -o Bundle_Launch.exe -mwindows

icon : Bundle_ApplyIcon.c $(objects) $(statics)
	rm -f Bundle_ApplyIcon.exe
	gcc -c Bundle_ApplyIcon.c -o Bundle_ApplyIcon.o -std=c99
	gcc Bundle_ApplyIcon.o $(objects) $(statics) -o Bundle_ApplyIcon.exe -mwindows

test : test.c $(objects) $(statics)
	rm -f test.exe
	gcc -c test.c -o test.o -std=c99
	gcc test.o $(objects) $(statics) -o test.exe

clean :
	rm -f *.o
	rm -f *.exe
