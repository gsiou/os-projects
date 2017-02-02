#ifndef RECORD_H
#define RECORD_H

#define MAX_NAME 50

typedef struct Record{
    char name[MAX_NAME];
    int votes;
    struct Record *next;
} Record;

/* Allocates memory and creates record */
Record* Record_create(char *name);

/* Sets record's votes */
void Record_set_votes(Record *r, int votes);

/* Destroys record and deallocates memory */
void Record_destroy(Record *r);

#endif
