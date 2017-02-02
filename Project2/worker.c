#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/times.h>
#include <unistd.h>
#include "list.h"
#include "types.h"

#define LINE_SIZE 100
#define PIPE_SIZE 50
#define NAME_SIZE 50
#define BUFFER_SIZE 50

int main(int argc, char *argv[]){
    /* Set the timer */
    double t1, t2;
    struct tms tb1, tb2;
    double ticspersec;
    ticspersec = (double) sysconf(_SC_CLK_TCK);
    t1 = (double) times(&tb1);

    /* Start job */
    int fd = atoi(argv[1]);
    int line_start, lines_to_read;
    List *l = List_create(); /* List to sort candidate data */
    List *places = List_create(); /* List to sort voting places */
    line_start = atoi(argv[2]);
    lines_to_read = atoi(argv[3]);
    /* open file and read our lines */
    FILE *f = fopen(argv[4], "r");
    if(f == NULL){
	perror("Worker can't open file");
	exit(-1);
    }
    char buffer[LINE_SIZE];
    /* Skip the first lines */
    int i;
    for(i = 0; i < line_start; i++){
	fgets(buffer, LINE_SIZE, f);
    }
    /* Start reading and adding to list */
    int invalids_n = 0;
    for(i = line_start; i < line_start + lines_to_read; i++){
	fgets(buffer, LINE_SIZE, f);
	/* parse string */
	char name[NAME_SIZE];
	char place_id[NAME_SIZE];
        int valid;
	sscanf(buffer, "%s %s %d\n", name, place_id, &valid);
	List_add_name(places, place_id);
	if(valid == 0){
	    /* handle invalid votes here */
	    invalids_n++;
	}
	else{
	    List_add_name(l, name);
	}
    }
    fclose(f);
    
    char buf[PIPE_SIZE];
    char msg[PIPE_SIZE];
    int votes;
    
    /* Send candidates */
    while(List_next(l, buf, &votes) != -1){
	sprintf(msg, "%s %d", buf, votes);
	if(write(fd, msg, sizeof(msg)) == -1){
	    perror("Cant write");
	    exit(-1);
	}
    }
    if(write(fd, "$", PIPE_SIZE) == -1){
	perror("Cant write");
	exit(-1);
    }

    /* Send voting place statistics */
    while(List_next(places, buf, &votes) != -1){
	sprintf(msg, "%s %d", buf, votes);
	if(write(fd, msg, sizeof(msg)) == -1){
	    perror("Cant write");
	    exit(-1);
	}
    }
    if(write(fd, "$", PIPE_SIZE) == -1){
	perror("Cant write");
	exit(-1);
    }

    /* Send number of invalid votes */
    sprintf(msg, "%d", invalids_n);
    if(write(fd, msg, sizeof(msg)) == -1){
	perror("Cant write");
	exit(-1);
    }

    /* Stop the timer */
    t2 = (double) times(&tb2);
    sprintf(msg, "%d %d %lf", getpid(), WORKER, (t2-t1) / ticspersec);
    if(write(fd, msg, sizeof(msg)) == -1){
	perror("Cant write");
	exit(-1);
    }
    if(write(fd, "$", sizeof(msg)) == -1){
	perror("Cant write");
	exit(-1);
    }
    /* Send signal to root */
    kill(atoi(argv[6]), SIGUSR1);
    //kill(atoi(argv[6]), SIGRTMIN+1);
    
    List_destroy(l);
    List_destroy(places);
    close(fd);
    exit(0);
}
