all:
	mkdir -p obj
	gcc -std=c99 -c bitstream.c	-o obj/bitstream.o
	gcc -std=c99 -c boolean.c 	-o obj/boolean.o
	gcc -std=c99 -c crashtest.c 	-o obj/crashtest.o
	gcc -std=c99 -c errorhandling.c -o obj/errorhandling.o
	gcc -std=c99 -c gcstack.c 	-o obj/gcstack.o
	gcc -std=c99 -c groups.c 	-o obj/groups.o
	gcc -std=c99 -c hashtable.c 	-o obj/hashtable.o
	gcc -std=c99 -c parsing.c 	-o obj/parsing.o
	gcc -std=c99 -c sorting.c 	-o obj/sorting.o
	mkdir -p bin
	ar rcs bin/libmemgroups.a 	\
		obj/bitstream.o	 	\
		obj/boolean.o 		\
		obj/crashtest.o 	\
		obj/errorhandling.o 	\
		obj/gcstack.o 		\
		obj/groups.o 		\
		obj/hashtable.o		\
		obj/parsing.o		\
		obj/sorting.o
