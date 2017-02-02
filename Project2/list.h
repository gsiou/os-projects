#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include "types.h"
#include "record.h" 

typedef struct{
    char type;
    Record *start;
    Record **current;
    int size;
} List;

/* Allocates memory for a list, returns pointer to it */
List* List_create();

/*
  Adds a record to the list sorted alphabetically
  by name.
  Returns 1 in case of success, -1 otherwise.
*/
int List_add_name(List *l, char *name);

/*
  Adds a record to the list sorted by votes.
  Returns 1 in case of success, -1 otherwise.
*/
int List_add_votes(List *l, char *name, int votes);

/* Destroys list (deallocates memory) */
void List_destroy(List *l);

/* Prints list data (for debug) */
void List_print(List *l, FILE *f);

/*
  Copies current records name and votes to
  respective parameters.
  Returns -1 if reached end of list.
*/
int List_next(List *l, char *name, int *votes);

/*
  Returns size of list 
*/
int List_size(List *l);

#endif
