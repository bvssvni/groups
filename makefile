all:
	mkdir -p obj
	gcc -c bitstream.c	-o obj/bitstream.o
	gcc -c boolean.c 	-o obj/boolean.o
	gcc -c crashtest.c 	-o obj/crashtest.o
	gcc -c errorhandling.c -o obj/errorhandling.o
	gcc -c gcstack.c 	-o obj/gcstack.o
	gcc -c groups.c 	-o obj/groups.o
	gcc -c hashtable.c 	-o obj/hashtable.o
	gcc -c parsing.c 	-o obj/parsing.o
	gcc -c sorting.c 	-o obj/sorting.o
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
