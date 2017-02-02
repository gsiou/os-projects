OBJS_ROOT = root.o
SOURCE_ROOT = root.c
OUT_ROOT = root

OBJS_VOTER = voter.o
SOURCE_VOTER = voter.c
OUT_VOTER = voter

OBJS_CHECKER = checker.o
SOURCE_CHECKER = checker.c
OUT_CHECKER = checker

OBJS_COUNTER = counter.o
SOURCE_COUNTER = counter.c
OUT_COUNTER = counter


CC = gcc
FLAGS = -g -c
LIBS = -lpthread

all: root voter counter checker

root: $(OBJS_ROOT)
	$(CC) -g $(OBJS_ROOT) -o $(OUT_ROOT) $(LIBS)
voter: $(OBJS_VOTER)
	$(CC) -g $(OBJS_VOTER) -o $(OUT_VOTER) $(LIBS)
checker: $(OBJS_CHECKER)
	$(CC) -g $(OBJS_CHECKER) -o $(OUT_CHECKER) $(LIBS)
counter: $(OBJS_COUNTER)
	$(CC) -g $(OBJS_COUNTER) -o $(OUT_COUNTER) $(LIBS)
root.o: root.c
	$(CC) $(FLAGS) root.c
voter.o: voter.c
	$(CC) $(FLAGS) voter.c
checker.o: checker.c
	$(CC) $(FLAGS) checker.c
counter.o: counter.c
	$(CC) $(FLAGS) counter.c
