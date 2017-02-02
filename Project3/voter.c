#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "types.h"

int main(int argc, char *argv[]) {
    if (argc != 4){
        return -1;
    }

    int regnum, choice, shmid;
    regnum = atoi(argv[1]);
    choice = atoi(argv[2]);
    shmid = atoi(argv[3]);
        
    printf("[%d]: Hello! I support [%d]\n", regnum, choice);

    void *sharedmem = shmat(shmid, NULL, 0);
    SharedData *sd = (SharedData *)sharedmem;

    // wait on list semaphore
    sem_wait(&(sd->list1));

    printf("Voter %d got in the list.\n", regnum);

    // find available checker
    sem_t *checker;
    sem_t *voter;
    bool *voter_valid;
    bool i_am_valid;
    sem_wait(&(sd->mutex));
    if (sd->checker1_id == 0) {
        sd->checker1_id = regnum;
        checker = &(sd->checker1);
        voter = &(sd->ch1_voter);
        voter_valid = &(sd->valid1);
    }
    else if (sd->checker2_id == 0) {
        sd->checker2_id = regnum;
        checker = &(sd->checker2);
        voter = &(sd->ch2_voter);
        voter_valid = &(sd->valid2);
    }
    else if (sd->checker3_id == 0){
        sd->checker3_id = regnum;
        checker = &(sd->checker3);
        voter = &(sd->ch3_voter);
        voter_valid = &(sd->valid3);
    }
    else{       
        printf("I should not be here \n");
    }
    sem_post(&(sd->mutex));

    // wake checker and wait for him to finish
    sem_post(checker);
    sem_wait(voter);
        
    sem_wait(&(sd->mutex));
    printf("(%d): Im done with checker and I am %d\n", regnum, *voter_valid);
    i_am_valid = *voter_valid;
    sem_post(&(sd->mutex));
    sem_post(checker); // we are done with checker
    if(i_am_valid){
        sem_wait(&(sd->c_voter));
        printf("I got picked by counter \n");
        sd->counter_id = regnum;
        sd->counter_choice = choice;
        sem_post(&(sd->counter));
        sem_wait(&(sd->counter_job));
        // we are done so we let other procs know 
        sem_wait(&(sd->mutex));
        sd->finished_voters++;
        sem_post(&(sd->mutex));
        sem_post(&(sd->counter));
    }
    else{
        printf("I was invalid :( \n");
        // we are done so we let other procs know 
        sem_wait(&(sd->mutex));
        sd->finished_voters++;
        sem_post(&(sd->mutex));
    }

    int err = shmdt(sharedmem);
    if (err == -1) perror("Could not detach.");
    return 0;
}
