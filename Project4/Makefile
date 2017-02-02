##########################################
#					 #
#	Makefile for the twa project.	 #
#					 #
##########################################

OBJS	= proc.o files.o utils.o main.o
SOURCE	= proc.c files.c utils.c main.c
HEADER	= proc.h files.h utils.h types.h
MISC    = README Makefile
TAR     = release.tar
OUT 	= myfsinfo
CC	= gcc
FLAGS 	= -c

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT)

main.o: main.c
	$(CC) $(FLAGS) main.c

proc.o: proc.c
	$(CC) $(FLAGS) proc.c

files.o: files.c
	$(CC) $(FLAGS) files.c

main.o: main.c
	$(CC) $(FLAGS) main.c

utils.o: utils.c
	$(CC) $(FLAGS) utils.c

clean:
	rm -f $(OBJS) $(OUT)

loc:
	wc -l $(SOURCE) $(HEADER)

release:
	tar -cvf $(TAR) $(SOURCE) $(HEADER) $(MISC)
