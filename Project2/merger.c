#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include "record.h"
#include "types.h"

#define WRITE 1
#define READ 0
#define BUFFER_SIZE 100
#define PIPE_SIZE 50
int main(int argc, char *argv[]){
    int d, l, i, pid;
    int file_line_from, lines;
    int split, rem;
    int fd_temp[2];
    int fd_write; /* To pass on */
    file_line_from = atoi(argv[3]);
    lines = atoi(argv[4]);
    d = atoi(argv[1]);
    l = atoi(argv[2]);
    int *fd = malloc(sizeof(int) * l); /* Store file descriptors */
    split = lines / l;
    rem = lines % l;
    for(i = 0; i < l; i++){

	/* Create pipes for each child */
	if(pipe(fd_temp) == -1)
	{
	    perror("Pipe creation error");
	    exit(-1);
	}
	fd[i] = fd_temp[READ];
	fd_write = fd_temp[WRITE];
	pid = fork();
	if(pid == 0){
	    close(fd_temp[READ]);
	    break;
	}
	else{
	    close(fd_temp[WRITE]);
	    /* Wait for children to finish */
	    while(waitpid(-1, NULL, WNOHANG) == 0){
	    	sleep(0.3);
	    }
	}
    }
    int n_child = i; /* to know which child we are */
    if(pid == -1){
	perror("I failed \n");
	exit(-1);
    }
    else if(pid != 0){
        //parent
	int fd_pipe = atoi(argv[6]);
	int lists = 0; /* when 1 we start reading voting places */
	char buf[50];
	int n;
	int count;
	int votes[l];
	char rec[500]; //TODO
	bool B[l];
	bool done[l];
	char (*P)[PIPE_SIZE];
	int min;
	P = malloc(sizeof(char *) * l);

	/* Set the timer */
	double t1, t2;
	struct tms tb1, tb2;
	double ticspersec;
	ticspersec = (double) sysconf(_SC_CLK_TCK);
	t1 = (double) times(&tb1);

	/* Start job */
	while(lists < 2){
	    count = 0;
	    for(i = 0; i < l; i++) {
		B[i] = TRUE;
		done[i] = FALSE;
	    }
	    while(count < l){
		for(i = 0; i < l; i++){
		    if(B[i]  && !done[i]){
			n = read(fd[i], buf, PIPE_SIZE);
			strcpy(rec, buf);
			sscanf(rec, "%s %d", P[i], &votes[i]);
			B[i] = FALSE;
			if(strcmp(P[i], "$") == 0){
			    done[i] = TRUE;
			}
		    }
		}
		/* Find minimum name to merge and send first */
		min = 0;
		for(i = 1; i < l; i++){
		    if(strcmp(P[min], "$") == 0 || (strcmp(P[i], P[min]) < 0 && strcmp(P[i], "$") != 0)){
			min = i;
		    }
		}
		/* Allow position min to be read again */
		B[min] = TRUE;

	    
		/* check for duplicates */
		for(i = 0; i < l; i++){
		    if(strcmp(P[i], P[min]) == 0 && i != min
		       && strcmp(P[i], "$") != 0)
		    {
			/* add duplicate votes to min */
			B[i] = TRUE;
			votes[min] += votes[i];
		    }
		}
		/* write min to pipe */
		char buf[PIPE_SIZE];
		if(strcmp(P[min], "$") != 0){
		    sprintf(buf, "%s %d", P[min], votes[min]);
		    if(write(fd_pipe, buf, sizeof(buf)) == -1){
			perror("Cant write");
			exit(-1);
		    }
		}
		else{
		    count++;
		}
		if(count == l){
		    /* end of data, write it to pipe */
		    if(write(fd_pipe, "$", sizeof(buf)) == -1){
			perror("Cant write");
			exit(-1);
		    }
		    lists++;
		}
	    }
	}
	free(P);
	/* Read number of invalids */
	int invalids = 0;
	for(i = 0; i < l; i++){
	    n = read(fd[i], buf, PIPE_SIZE);
	    if(n < 0){
		perror("Cant read");
		exit(-1);
	    }
	    invalids += atoi(buf);
	}

	/* Write number of invalids */
	sprintf(buf, "%d", invalids);
	if(write(fd_pipe, buf, sizeof(buf)) == -1){
	    perror("Cant write");
	    exit(-1);
	}

	/* Read times from children */
	for(i = 0; i < l; i++){
	    do{
		n = read(fd[i], buf, PIPE_SIZE);
		//printf("=>%d %s \n", getpid(), buf);
		if(n < 0){
		    perror("Cant read");
		    exit(-1);
		}
		
		if(strcmp(buf, "$") != 0){
		    /* Write to parent */
		    if(write(fd_pipe, buf, sizeof(buf)) == -1){
			perror("Cant write");
			exit(-1);
		    }
		}
	    } while(strcmp(buf, "$") != 0);
	}

	/* End timer */
	t2 = (double) times(&tb2);
	sprintf(buf, "%d %d %lf", getpid(), MERGER, (t2-t1) / ticspersec);

	/* Send our own time */
	if(write(fd_pipe, buf, PIPE_SIZE) == -1){
	    perror("Cant write");
	    exit(-1);
	}
	
	/* We are done, let parent know */
	if(write(fd_pipe, "$", PIPE_SIZE) == -1){
	    perror("Cant write");
	    exit(-1);
	}
	
	
	//printf("Child with pid %d done\n",getpid());
	exit(0);
    }
    else{
	//child
	/* split lines */
	int new_from = file_line_from + n_child * split;
	if(d == 1){
	    /* we are at lowest level so become workers */
	    char buf1[BUFFER_SIZE], buf2[BUFFER_SIZE],
		 buf3[BUFFER_SIZE], buf4[BUFFER_SIZE];
	    sprintf(buf3, "%d", n_child);
	    sprintf(buf4, "%d", fd_write);
	    sprintf(buf1, "%d", new_from);
	    if(n_child != l - 1){
		sprintf(buf2, "%d", split);
	    }
	    else{
		/* last worker gets remainder */
		sprintf(buf2, "%d", split + rem);
	    }
	    char *args[] = {"./worker", "worker", buf4, buf1, buf2, argv[5], buf3, argv[7], NULL};
	    if(execvp(args[0], &args[1]) == -1){
		perror("Worker fail \n");
		exit(-1);
	    }
	}
	else{
	    /* creating another level of mergers */
	    char tmp1[BUFFER_SIZE], tmp2[BUFFER_SIZE],
		 tmp3[BUFFER_SIZE], tmp4[BUFFER_SIZE],
		 tmp5[BUFFER_SIZE];
	    sprintf(tmp1, "%d", d - 1);
	    sprintf(tmp2, "%d", l);

	    sprintf(tmp3, "%d", new_from);
	    if(n_child != l - 1){
		sprintf(tmp4, "%d", split);
	    }
	    else{
                /* last child gets the remainder */
		sprintf(tmp4, "%d", split + rem);
	    }
	    sprintf(tmp5, "%d", fd_write);
	    char *args[] = {"merger", tmp1, tmp2, tmp3, tmp4, argv[5], tmp5, argv[7], NULL};
	    if(execvp("./merger", args) == -1){
		perror("Exec failure \n");
		exit(-1);
	    }
	}
    }
    exit(0);
}

    
