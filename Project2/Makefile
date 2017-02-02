OBJS_ROOT = root.o list.o record.o
SOURCE_ROOT = root.c list.c record.c
OUT_ROOT = tally

OBJS_MERGER = merger.o
SOURCE_MERGER = merger.c
OUT_MERGER = merger

OBJS_WORKER = worker.o list.o record.o
SOURCE_WORKER = worker.c list.c record.c
OUT_WORKER = worker

CC = gcc
FLAGS = -g -c
LIBS = -lm

all: root merger worker

root: $(OBJS_ROOT)
	$(CC) -g $(OBJS_ROOT) -o $(OUT_ROOT) $(LIBS)
merger: $(OBJS_MERGER)
	$(CC) -g $(OBJS_MERGER) -o $(OUT_MERGER)
worker:	$(OBJS_WORKER)
	$(CC) -g $(OBJS_WORKER) -o $(OUT_WORKER)
root.o: root.c
	$(CC) $(FLAGS) root.c
list.o: list.c
	$(CC) $(FLAGS) list.c
record.o: record.c
	$(CC) $(FLAGS) record.c
merger.o: merger.c
	$(CC) $(FLAGS) merger.c
worker.o: worker.c
	$(CC) $(FLAGS) worker.c
