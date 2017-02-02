#define true 1
#define false 0

typedef int bool;

typedef struct SharedData{
    int number_of_registered;
    int number_of_choices;
    int number_of_voters;
    int finished_voters;
    int unregistered_voters;
    int registered_voters;
    sem_t mutex;
    sem_t checker1;
    sem_t checker2;
    sem_t checker3;
    sem_t ch1_voter;
    sem_t ch2_voter;
    sem_t ch3_voter;
    sem_t list1;
    sem_t list2;
    sem_t counter;
    sem_t c_voter;
    sem_t counter_job;
    int checker1_id;
    int checker2_id;
    int checker3_id;
    bool valid1;
    bool valid2;
    bool valid3;
    int counter_id;
    int counter_choice;
} SharedData;

typedef struct Registry{
    int reg_number;
    int vote_count;
    time_t vote_time;
} Registry;
