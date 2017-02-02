##########################################
#					 #
#	Makefile for the twa project.	 #
#					 #
##########################################

OBJS	= hash.o ll.o main.o twa.o
SOURCE	= hash.c ll.c main.c twa.c
HEADER	= types.h ll.h hash.h twa.h
MISC    = README Makefile
TAR     = release.tar
OUT 	= twa
CC	= gcc
FLAGS 	= -g -c
LIBS 	= -lm

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LIBS)

main.o: main.c
	$(CC) $(FLAGS) main.c

ll.o: ll.c
	$(CC) $(FLAGS) ll.c

hash.o: hash.c
	$(CC) $(FLAGS) hash.c

twa.o: twa.c
	$(CC) $(FLAGS) twa.c

clean:
	rm -f $(OBJS) $(OUT)

loc:
	wc -l $(SOURCE) $(HEADER)

release:
	tar -cvf $(TAR) $(SOURCE) $(HEADER) $(MISC)
