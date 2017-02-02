#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>
#include "list.h"

#define BUFFER_SIZE 50
#define READ 0
#define WRITE 1
#define PIPE_SIZE 50
#define FILENAME_SIZE 100

volatile sig_atomic_t sigcounter = 0;

void handler(int signum){
    sigcounter++;
}

void print_usage(char *program_name){
    printf("Usage: %s -i TextFile -l numOfSMs -d Depth -p Percentile -o OutputFile \n", program_name);
    printf("-b and -f flags are required, the others are optional \n");
}


int main(int argc, char *argv[]){
    /* Read command line arguments */
    if(argc < 6){
	printf("Insufficient parameters. \n");
	print_usage(argv[0]);
	return 1;
    }

    int current = 1;
    int args_left = argc - 1;

    char  TextFileName[FILENAME_SIZE];
    char  OutFileName[FILENAME_SIZE];
    int   LParam;
    int   DParam;
    float Percentile;

    /* We are reading arguments in pairs */
    while(args_left >= 2){
	if(strcmp(argv[current], "-i") == 0){
	    current++;
	    args_left--;
	    strcpy(TextFileName, argv[current]);
	}
	else if(strcmp(argv[current], "-l") == 0){
	    current++;
	    args_left--;
	    LParam = atoi(argv[current]);
	}
	else if(strcmp(argv[current], "-d") == 0){
	    current++;
	    args_left--;
	    DParam = atoi(argv[current]);
	}
	else if(strcmp(argv[current], "-p") == 0){
	    current++;
	    args_left--;
	    Percentile = (float) strtod(argv[current], NULL);
	}
	else if(strcmp(argv[current], "-o") == 0){
	    current++;
	    args_left--;
	    strcpy(OutFileName, argv[current]);
	}
	else{
	    printf("%s is not a correct option. \n", argv[current]);
	    print_usage(argv[0]);
	    return 1;
	}
	current++;
	args_left--;
    }
    
    signal(SIGUSR1, handler);
    //signal(SIGRTMIN+1, handler);
    
    int l, d;
    l = LParam;
    d = DParam;
    char filename[FILENAME_SIZE];
    strcpy(filename, TextFileName);
    int pid, i;
    int rootid = getpid();
    /* Count lines of file to split it */
    int lines = 0, ch;
    FILE *f = fopen(TextFileName, "r");
    if(f == NULL){
	perror("Can't open file with votes.");
	exit(-1);
    }
    while((ch = fgetc(f)) != EOF){
	if(ch == '\n') lines++;
    }
    printf("File has %d lines. \n", lines);
    fclose(f);
    int split = lines / l;
    int rem = lines % l; /* Remainder will be sent to the last child */
    int fd[l];
    int fd_temp[2];
    int fd_write;
    printf("Root id: %d \n", rootid);
    for(i = 0; i < l; i++){
	/* Opening pipe for children */
	if(pipe(fd_temp) == -1){
	    perror("Pipe creation error");
	    exit(-1);
	}
	fd[i] = fd_temp[READ];
	fd_write = fd_temp[WRITE];
	pid = fork();
	if(pid == 0){
	    break;
	}
	
    }
    int n_child = i; /* which child we are on TODO */
    if(pid > 0){
	//parent
        while(waitpid(-1, NULL, WNOHANG) == 0){
	    sleep(1);
	}
	if(sigcounter != pow(l,d)){
	    printf("Received %d signals.\n", sigcounter);
	    exit(-1);
	}
	int i;
	char buf[50];
	char tmp[50];
	char rec[100]; 
	int count;
	
	/* When lists is 0 we read candidates */
	/* When lists is 1 we read vote places */
	int lists = 0;
	int min;
	List *list = List_create();
	List *places = List_create();
	int votes[l];
	bool B[l];
	bool done[l];
	char (*P)[PIPE_SIZE];
	int total_votes = 0;
	P = malloc(sizeof(char *) * l);
	while(lists < 2){
	    count = 0;
	    for(i = 0; i < l; i++){
		B[i] = TRUE;
		done[i] = FALSE;
	    }
	    while(count < l){
		/* Reading pipes */
		for(i = 0; i < l; i++){
		    if(B[i] && !done[i]){
			read(fd[i], buf, PIPE_SIZE);
			strcpy(rec, buf);
			sscanf(rec, "%s %d", P[i], &votes[i]);
			B[i] = FALSE; /* Blocking */
			if(strcmp(P[i], "$") == 0){
			    done[i] = TRUE;
			}
		    }
		}
		/* Finding min to merge */
		min = 0;
		for(i = 1; i < l; i++){
		    if(strcmp(P[min], "$") == 0 || (strcmp(P[i], P[min]) < 0 && strcmp(P[i], "$") != 0)){
			min = i;
		    }
		}
		B[min] = TRUE; /* Unblocking */

		/* Finding duplicates */
		for(i = 0; i < l; i++){
		    if(strcmp(P[i], P[min]) == 0 && i != min
		       && strcmp(P[i], "$") != 0)
		    {
			B[i] = TRUE;
			votes[min] += votes[i];
		    }
		}
		/* Adding data read to lists */
		if(strcmp(P[min], "$") != 0){
		    if(lists == 0){
			List_add_votes(list, P[min], votes[min]);
		    }
		    else{
			total_votes += votes[min];
			List_add_votes(places, P[min], votes[min]);
		    }
		}
		else{
		    count++;
		}
		if(count == l) lists++;
	    }
	}

	printf("Total number of votes: %d \n", total_votes);
	
	/* Read number of invalids */
	int invalids = 0, n;
	for(i = 0; i < l; i++){
	    n = read(fd[i], buf, PIPE_SIZE);
	    if(n < 0){
		perror("Cant read");
		exit(-1);
	    }
	    invalids += atoi(buf);
	}
	printf("Number of invalid votes: %d \n", invalids);

	/* Read times */
	double time;
	int type;
	int c_pid;
	for(i = 0; i < l; i++){;
	    do{
		n = read(fd[i], buf, PIPE_SIZE);
		if(n < 0){
		    perror("Cant read");
		    exit(-1);
		}

		if(strcmp(buf, "$") != 0){
		    sscanf(buf, "%d %d %lf", &c_pid, &type, &time);
		    if(type == WORKER){
			printf("WORKER WITH PID:%d TIME:%lf \n", c_pid, time);
		    }
		    else{
			printf("MERGER WITH PID:%d TIME:%lf \n", c_pid, time);
		    }
		}
	    } while(strcmp(buf, "$") != 0);
	}

	/* Print data to files */
	FILE *f = fopen(OutFileName, "w");
	fprintf(f, "|CANDIDATE RESULTS|\n");
	List_print(list, f);
	fprintf(f, "\n|ELECTION PLACES STATISTICS|\n");
	List_print(places, f);
	fprintf(f, "\n|ELECTION PLACES WITH ABOUT %f VOTES|\n", Percentile);
	count = 0;
	int votesn = 0;
	while(List_next(places, buf, &votesn) != -1){
	    count += votesn;
	    float perc = (float) count / (float) total_votes;
	    fprintf(f, "%d %s \n", votesn, buf);
	    if(perc > Percentile) break;
	}
	printf("done \n");
	exit(0);
    }
    else if(pid == -1){
	perror("ERROR!");
	exit(-1);
    }
    else{
	char buf1[BUFFER_SIZE], buf2[BUFFER_SIZE],
	     buf3[BUFFER_SIZE], buf4[BUFFER_SIZE],
	     buf5[BUFFER_SIZE], buf6[BUFFER_SIZE];
	/* we reduce 1 from d per level created */
	sprintf(buf1, "%d", d - 1);
	sprintf(buf2, "%d", l);
	sprintf(buf3, "%d", i * split); 
	if(i != l - 1){
	    sprintf(buf4, "%d", split);
	}
	else{
	    /* last child gets the remainder */
	    sprintf(buf4, "%d", split + rem);
	}
	sprintf(buf5, "%d", fd_write);
	sprintf(buf6, "%d", rootid);
	char *args[] = {"merger", buf1, buf2, buf3, buf4, filename, buf5, buf6, NULL};
	if(execvp("./merger", args) == -1){
	    perror("Exec failed \n");
	    exit(-1);
	}
    }
    exit(0);
}
