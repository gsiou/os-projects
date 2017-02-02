#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "types.h"

int main(int argc, char *argv[]) {
    if (argc != 5) {
        exit(-1);
    }

    char ballotfile[100], outfilename[100];
    int period, shmid;
    strcpy(ballotfile, argv[1]);
    period = atoi(argv[2]);
    strcpy(outfilename, argv[3]);
    shmid = atoi(argv[4]);

    FILE *choicefile = fopen(ballotfile, "r");
    FILE *outfile = fopen(outfilename, "w");
    if(choicefile == NULL){
        perror("Counter choice file error");
    }
    if(outfile == NULL){
        perror("Counter out file error");
    }
    //attach shared memory
    void *sharedmem = shmat(shmid, NULL, 0);
    SharedData *sd;
    sd = (SharedData*) sharedmem;

    // create table with results
    int results[sd->number_of_choices];
    int i;
    for(i = 0; i < sd->number_of_choices; i++){
        results[i] = 0;
    }

    // say to the world that we are ready
    sem_post(&(sd->c_voter));
    int wait_time;
    srand(time(NULL));
    while(1){
        sem_wait(&(sd->counter));
        if(sd->finished_voters == sd->number_of_voters){
            break;
        }
        sem_post(&(sd->list2));
        printf("->Counter got woken up by %d. \n", sd->counter_id);
        if(sd->counter_choice < sd->number_of_choices){
            results[sd->counter_choice]++;
        }
        wait_time = rand() % period;
        usleep(wait_time * 1000000);
        sem_post(&(sd->counter_job)); //wake up voter
        sem_wait(&(sd->counter)); // we block till voter updates status
        sem_post(&(sd->c_voter));
    }
    printf("Counter finished and will prepare the results \n");

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int count = 0;

    fprintf(outfile, "CHOICES: \n");
    while((read = getline(&line, &len, choicefile)) != -1){
        fprintf(outfile, "[%d] %s", count, line);
        count++;
    }
    if(line) free(line);
    
    fprintf(outfile, "RESULTS: \n");
    for(i = 0; i < sd->number_of_choices; i++){
        fprintf(outfile, "Choice %d: %d\n", i, results[i]);
    }
    fprintf(outfile, "Number of registered voters: %d\n", sd->registered_voters);
    fprintf(outfile, "Number of unregistered voters: %d \n", sd->unregistered_voters);
    Registry *reg;
    for(i = 0; i < sd->number_of_registered; i++){
        reg = sharedmem + sizeof(SharedData) + i * sizeof(Registry);
        if(reg->vote_count > 1){
            fprintf(outfile, "Voter %d tried to vote %d times.", reg->reg_number, reg->vote_count);
            fprintf(outfile, "First attempt at: %s", ctime(&(reg->vote_time)));
        }
    }

    fclose(choicefile);
    int err;
    err = shmdt(sharedmem);
    if (err == -1) perror("Could not detach.");
    printf("Results are available at %s \n", outfilename);
    printf("Press enter to exit.\n");
}
