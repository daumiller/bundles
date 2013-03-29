cflags  = -I. -I./bundle -I./depends -std=c99
statics = depends/mxml.a depends/hashtable.a
objects = bundle.o wide.o

bundle.o : bundle/bundle.c
	gcc $(cflags) -c bundle/bundle.c -o bundle.o

wide.o : bundle/wide.c
	gcc $(cflags) -c bundle/wide.c -o wide.o

launch : programs/Bundle_Launch.c $(objects) $(statics)
	gcc $(cflags) -c programs/Bundle_Launch.c -o Bundle_Launch.o
	gcc Bundle_Launch.o $(objects) $(statics) -o Bundle_Launch.exe -mwindows

icon : programs/Bundle_ApplyIcon.c $(objects) $(statics)
	gcc $(cflags) -c programs/Bundle_ApplyIcon.c -o Bundle_ApplyIcon.o
	gcc Bundle_ApplyIcon.o $(objects) $(statics) -o Bundle_ApplyIcon.exe -mwindows

assoc : programs/Bundle_Association.c $(objects) $(statics)
	gcc $(cflags) -c programs/Bundle_Association.c -o Bundle_Association.o
	gcc Bundle_Association.o $(objects) $(statics) -o Bundle_Association.exe -mwindows

test : programs/test.c $(objects) $(statics)
	gcc $(cflags) -c programs/test.c -o test.o
	gcc test.o $(objects) $(statics) -o test.exe

clean :
	rm -f *.o
	rm -f *.exe
