#include "record.h"
#include <stdlib.h>
#include <string.h>

Record* Record_create(char *name){
    Record *r = malloc(sizeof(Record));
    r->next = NULL;
    strcpy(r->name, name);
    r->votes = 1; /* Creation of record means at least 1 */
    return r;
}

void Record_set_votes(Record *r, int votes){
    r->votes = votes;
}

void Record_destroy(Record *r){
    free(r);
}
