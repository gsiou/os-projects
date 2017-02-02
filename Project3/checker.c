#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include "types.h"
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        exit(-1);
    }
    int period;
    int shmid;
    int myindex;
    period = atoi(argv[1]);
    shmid = atoi(argv[2]);
    myindex = atoi(argv[3]);

    printf("I am checker %d.\n", myindex);

    // attach shared memory 
    void *sharedmem = shmat(shmid, NULL, 0);
    sem_t *mysem, *votersem;
    int *voterid;
    bool *voter_valid;
    SharedData *sd = (SharedData*)sharedmem;
    Registry *reg;
    if (myindex == 0){
        mysem = &(sd->checker1);
        votersem = &(sd->ch1_voter);
        voter_valid = &(sd->valid1);
    }
    else if (myindex == 1) {
        mysem = &(sd->checker2);
        votersem = &(sd->ch2_voter);
        voter_valid = &(sd->valid2);
    }
    else {
        mysem = &(sd->checker3);
        votersem = &(sd->ch3_voter);
        voter_valid = &(sd->valid3);
    }
    int wait_time;
    srand(time(NULL));
    while (1) {
        //wait on my checker
        sem_wait(mysem);
        if(sd->finished_voters == sd->number_of_voters){
            break;
        }
        sem_wait(&(sd->mutex));
        if (myindex == 0) {
            voterid = &(sd->checker1_id);
        }
        else if (myindex == 1) {
            voterid = &(sd->checker2_id);
        }
        else {
            voterid = &(sd->checker3_id);
        }
        printf("Checker %d: I got woken up by: %d\n", myindex, *voterid);
        
        // search if voter is registered 
        // and make sure he has never voted again
        int i;
        bool registered = false;
        *voter_valid = false;
        for(i = 0;i < sd->number_of_registered; i++){
            reg = sharedmem + sizeof(SharedData) + i * sizeof(Registry);
            if(reg->reg_number == *voterid){

                // we got a valid id, check for vote count
                if(reg->vote_count == 0){
                    // we are good to go
                    *voter_valid = true;
                    reg->vote_time = time(NULL);
                }
                else{
                    printf("%d tried to vote again! \n", *voterid);
                }
                registered = true;
                reg->vote_count++;
                break;
            }
        }
        if(registered){
            sd->registered_voters++;
        }
        else{
            sd->unregistered_voters++;
        }
        //*voterid = 0;
        sem_post(&(sd->mutex));
        wait_time = rand() % period;
        usleep(wait_time * 1000000);
        if(*voter_valid == true){
            sem_wait(&(sd->list2));
        }
        sem_wait(&(sd->mutex));
        *voterid = 0;
        sem_post(&(sd->mutex));
        sem_post(votersem); // wake up voter
        sem_wait(mysem); // we block until voter gets the data
        
        sem_post(&(sd->list1));
    }
    // detach shared memory 
    int err = shmdt(sharedmem);
    if (err == -1) perror("Could not detach.");
}
