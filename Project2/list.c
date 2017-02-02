#include "list.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

List* List_create(){
    List *l = malloc(sizeof(List));
    l->size = 0;
    l->start = NULL;
    l->current = NULL;
    return l;
}

int List_add_name(List *l, char *name){
    /* If list is empty, just add record */
    if(l->size == 0){
	Record *r = Record_create(name);
	if(r == NULL) return -1;
	l->start = r;
	l->current = &(l->start);
	l->size++;
	return 1;
    }
    Record *curr;
    Record *prev;
    curr = l->start;
    prev = NULL;
    while(prev != NULL || curr != NULL){
	if(curr == NULL){
	    /* Add it to the end */
	    Record *r = Record_create(name);
	    if(r == NULL) return -1;
	    prev->next = r;
	    l->size++;
	    return 1;
	}
	else if(strcmp(name, curr->name) == 0){
	    /* Name exists, increase votes */
	    curr->votes++;
	    return 1;
	}
	else if(strcmp(name, curr->name) < 0){
	    /* Add new record */
	    Record *r = Record_create(name);
	    if(r == NULL) return -1;
	    if(prev != NULL)
		prev->next = r;
	    else
		l->start = r;
	    r->next = curr;
	    l->size++;
	    return 1;
	}
	prev = curr;
	curr = curr->next;
    }
    return -1;
}

int List_add_votes(List *l, char *name, int votes){
    /* If list is empty, just add record */
    if(l->size == 0){
	Record *r = Record_create(name);
	Record_set_votes(r, votes);
	if(r == NULL) return -1;
	l->start = r;
	l->current = &(l->start);
	l->size++;
	return 1;
    }
    Record *curr;
    Record *prev;
    curr = l->start;
    prev = NULL;
    while(prev != NULL || curr != NULL){
	if(curr == NULL){
	    /* Add it to the end */
	    Record *r = Record_create(name);
	    Record_set_votes(r, votes);
	    if(r == NULL) return -1;
	    prev->next = r;
	    l->size++;
	    return 1;
	}
	//else if(strcmp(name, curr->name) < 0){
	else if(votes > curr->votes){
	    /* Add new record */
	    Record *r = Record_create(name);
	    Record_set_votes(r, votes);
	    if(r == NULL) return -1;
	    if(prev != NULL)
		prev->next = r;
	    else
		l->start = r;
	    r->next = curr;
	    l->size++;
	    return 1;
	}
	prev = curr;
	curr = curr->next;
    }
    return -1;
}

void List_destroy(List *l){
    int i;
    Record *curr = l->start;
    Record *temp;
    for(i = 0; i < l->size; i++){
	temp = curr;
	curr = curr->next;
	Record_destroy(temp);
    }
    free(l);
}
void List_print(List *l, FILE *f){
    int i;
    if(f == NULL){
	printf("Error opening file \n");
	return;
    }
    Record *curr = l->start;
    for(i = 0;i < l->size; i++){
	fprintf(f, "%d %s \n", curr->votes, curr->name);
	curr = curr->next;
    }
}

int List_next(List *l, char *name, int *votes){
    if(*(l->current) == NULL){
	l->current = &(l->start); /* Reset current */
	return -1;
    }
    strcpy(name, ((*(l->current))->name));
    *votes = (*(l->current))->votes;
    l->current = &((*(l->current))->next);
    return 0;
}

int List_size(List *l){
    return l->size;
}

/*
int main(){
    List *l = List_create();
    List_add(l, "Manolis");
    List_add(l, "Manolis");
    List_add(l, "Giorgos");
    List_add(l, "Narima");
    List_add(l, "Giorgos");
    List_add(l, "Manolis");
    List_add(l, "Vertis");
    List_add(l, "Narima");
    List_print(l);
    char buf[50];
    int votes;
    while(List_next(l, buf, &votes) != -1){
	printf("-> %s %d \n", buf, votes);
    }
    List_destroy(l);
}
*/
