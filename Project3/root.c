#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "types.h"

void print_usage(){
    printf("Usage: ./root -i register_file -n number_of_voters ");
    printf("-l list_size -o outfile -d1 checker_max_time -d2 counter_max_time ");
    printf("-c ballot_file\n");
}

int main(int argc, char *argv[]){
    //read parameters from command line
    if(argc != 15){
        printf("Parameter syntax is wrong! \n");
        print_usage();
        return -1;
    }

    int current = 1;
    int args_left = argc - 1;

    int   numvoters, L;
    char  fname_out[100];
    char  fname_ballot[100];
    char  fname_reg[100];
    float d1, d2;

    // reading args in pairs
    
    while(args_left >= 2){
        if(strcmp(argv[current], "-i") == 0){
            current++;
            args_left--;
            strcpy(fname_reg, argv[current]);
        }
        else if(strcmp(argv[current], "-l") == 0){
            current++;
            args_left--;
            L = atoi(argv[current]);
        }
        else if(strcmp(argv[current], "-n") == 0){
            current++;
            args_left--;
            numvoters = atoi(argv[current]);
        }
        else if(strcmp(argv[current], "-d1") == 0){
            current++;
            args_left--;
            d1 = (float) strtod(argv[current], NULL);
        }
        else if(strcmp(argv[current], "-d2") == 0){
            current++;
            args_left--;
            d2 = (float) strtod(argv[current], NULL);
        }
        else if(strcmp(argv[current], "-o") == 0){
            current++;
            args_left--;
            strcpy(fname_out, argv[current]);
        }
        else if(strcmp(argv[current], "-c") == 0){
            current++;
            args_left--;
            strcpy(fname_ballot, argv[current]);
        }
        else{
            printf("%s is not a correct option. \n", argv[current]);
            print_usage(argv[0]);
            return -1;
        }
        current++;
        args_left--;
    }

    // open regfile and ballot and count lines
    int linesreg, linesballot = 0;
    FILE *filereg = fopen(fname_reg, "r");
    FILE *fileballot = fopen(fname_ballot, "r");
    if(filereg == NULL){
        perror("File error.");
        exit(-1);
    }
    if(fileballot == NULL){
        perror("File error.");
        exit(-1);
    }
    fseek(filereg, 0, SEEK_END);
    linesreg = ftell(filereg) / 12;
    char ch;
    while (!feof(fileballot))
    {
        ch = fgetc(fileballot);
        if (ch == '\n')
        {
            linesballot++;
        }
    }

    // create shared segment 
    int id;
    id = shmget(IPC_PRIVATE, sizeof(SharedData) + linesreg * sizeof(Registry), 0666);
    if (id == -1) {
        perror("Failed to allocate shared segm.\n");
    }
    else {
        printf("Allocated shared segm with id: %d.\n", (int)id);
    }

    // store registry in shared memory segm
    rewind(filereg);
    void *sharedmem;
    int err, i;
    SharedData *sd;
    Registry *registry;
    sharedmem = shmat(id, NULL, 0);
    sd = (SharedData *)sharedmem;
    sd->number_of_registered = linesreg;
    sd->number_of_choices = linesballot;
    sd->number_of_voters = numvoters;
    sd->finished_voters = 0;
    sd->unregistered_voters = 0;
    sd->registered_voters = 0;
    sem_init(&(sd->mutex), 1, 1);
    sem_init(&(sd->checker1), 1, 0);
    sem_init(&(sd->checker2), 1, 0);
    sem_init(&(sd->checker3), 1, 0);
    sem_init(&(sd->list1), 1, 3);
    sem_init(&(sd->list2), 1, L);
    sem_init(&(sd->ch1_voter), 1, 0);
    sem_init(&(sd->ch2_voter), 1, 0);
    sem_init(&(sd->ch3_voter), 1, 0);
    sem_init(&(sd->c_voter), 1, 0);
    sem_init(&(sd->counter), 1, 0);
    sem_init(&(sd->counter_job), 1, 0);
    sd->checker1_id = 0;
    sd->checker2_id = 0;
    sd->checker3_id = 0;

    for (i = 0; i < linesreg; i++) {
        registry = sharedmem + sizeof(SharedData) + i * sizeof(Registry);
        registry->vote_count = 0;
        registry->vote_time = 0;
        fseek(filereg, 12 * i, SEEK_SET);
        fscanf(filereg, "%d", &(registry->reg_number));
    }

    // create counter
    int pid;
    pid = fork();
    if(pid == 0){
        char buf_period[50];
        char buf_outfile[50];
        char buf_shmid[50];
        sprintf(buf_period, "%f", d2);
        sprintf(buf_shmid, "%d", id);
        char *args[] = { "counter", fname_ballot, buf_period, fname_out, 
                         buf_shmid, NULL};
        execvp("./counter", args);
    }

    // create 3 checkers
    for (i = 0; i < 3; i++) {
        pid = fork();
        if (pid == 0) {
            // create checker and assign args
            char buf_per[50];
            char buf_shid[50];
            char buf_index[50];
            sprintf(buf_per, "%f", d1);
            sprintf(buf_shid, "%d", id);
            sprintf(buf_index, "%d", i);
            char *args[] = { "checker", buf_per, buf_shid, buf_index, NULL };
            execvp("./checker", args);
        }
    }

    // create some voters
    for (i = 0; i < numvoters; i++) {
        pid = fork();
        if (pid == 0) {
            // create worker and assign random values on args
            time_t t;
            srand((unsigned)time(&t) * 100 * i);
            char buf_reg[50];
            char buf_choice[50];
            char buf_shid[50];
            char buf_line[100];
            int reg, choice, regline, hasvoted;
            if ((rand() % 2) == 0) {
                //.5 chance of getting correct registry number
                regline = rand() % linesreg;
                fseek(filereg, 12 * regline, SEEK_SET);
                fscanf(filereg, "%d %d", &reg, &hasvoted);
                sprintf(buf_reg, "%d", reg);
            }
            else {
                sprintf(buf_reg, "%d", rand() % 1000000000 + 99999999);
            }
            choice = rand() % linesballot;
            sprintf(buf_choice, "%d", choice);
            sprintf(buf_shid, "%d", id);
            char *args[] = { "voter", buf_reg, buf_choice, buf_shid, NULL };
            execvp("./voter", args);
        }
    }

    while(1){
        //check if we are done
        //printf("Done so far: %d \n", sd->finished_voters);
        if(sd->finished_voters == sd->number_of_voters){
            printf("We are done.Thank you everybody. \n");
            // wake counter and checkers who are stuck waiting
            sem_post(&(sd->counter));
            sem_post(&(sd->checker1));
            sem_post(&(sd->checker2));
            sem_post(&(sd->checker3));
            break;
        }
        sleep(1);
    }
    err = shmdt(sharedmem);
    if (err == -1) perror("Could not detach.");

    // remove the shared segment
    err = shmctl(id, IPC_RMID, 0);
    if(err == -1) perror ("Could not remove.");

    // close files
    fclose(filereg);
    fclose(fileballot);

    getchar();
    return 0;
}
