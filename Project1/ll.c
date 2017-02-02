#include "ll.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

List* List_create(){
    List *l = malloc(sizeof(List));
    l->start = NULL;
    l->end = NULL;
    l->size = 0;
    return l;
}

void List_addCustomer(List *l,Customer * c){
    if(l->size == 0){
	l->start = c;
	l->end = c;
    }
    else if(c->money >= l->start->money){
	c->next = l->start;
	l->start->prev = c;
	l->start = c;
    }
    else if(c->money <= l->end->money){
	c->prev = l->end;
	l->end->next = c;
	l->end = c;
    }
    else{
	Customer *pointer = l->start;
	Customer *temp;
	while(pointer != NULL){
	    if(c->money >= pointer->money){
		c->next = pointer;
		c->prev = pointer->prev;
		pointer->prev->next = c;
		pointer->prev = c;
		break;
	    }
	    pointer = pointer->next;
	}
    }
    l->size++;
}

void List_print(List *l){
    printf("List size: %d\n", l->size);
    Customer *current = l->start;
    while(current != NULL){
	Customer_print(current);
	current = current->next;
    }
}

void List_reinsert(List *l, Customer *c){
    if(l->size == 0){
	return ;
    }
    else if(l->start == c){
	l->start = l->start->next;
	l->start->prev = NULL;
    }
    else if(l->end == c){
	l->end = l->end->prev;
	l->end->next = NULL;
    }
    else{
	c->prev->next = c->next;
	c->next->prev = c->prev;
    }
    l->size--;
    c->next = NULL;
    c->prev = NULL;
    List_addCustomer(l, c);
}

void List_print_top(List *l, int k){
    Customer *c = l->start;
    int i = 0;
    while(i < k && c != NULL){
	Customer_print(c);
	c = c->next;
	i++;
    }
}

void List_print_bot(List *l, int k){
    Customer *c = l->end;
    int i = 0;
    while(i < k && c != NULL){
	Customer_print(c);
	c = c->prev;
	i++;
    }
}

void List_print_from_to(List *l, Customer *c1, Customer *c2){
    Customer *current = c1;
    while(current != NULL && current != c2){
	Customer_print(current);
	current = current->next;
    }
}

float List_avg(List *l){
    float res;
    Customer *c = l->start;
    while(c != NULL){
	res += c->money;
	c = c->next;
    }
    res = res / l->size;
    return res;
}

float List_percentile(List *l, long custid){
    Customer *c = l->end;
    int i = 1;
    while(c != NULL){
	if(c->custid == custid){
	    return ((float) i) * 100 / l->size;
	}
	i++;
	c = c->prev;
    }
}

void List_destroy(List* l){
    /* Destroy customers first */
    Customer *c = l->start;
    Customer *temp;
    while(c != NULL){
	temp = c;
	c = c->next;
	Customer_destroy(temp);
    }

    /* Free list memory */
    free(l);
}

Customer* Customer_create(long custid, char *name, char *surname, char *street,
			  int number, char *city, char *postal, float money){
    Customer* c = malloc(sizeof(Customer));
    c->custid = custid;
    c->number = number;
    c->money = money;
    c->next = NULL;
    c->prev = NULL;
    c->name = malloc(sizeof(char) * strlen(name) +1);
    strcpy(c->name, name);
    c->surname = malloc(sizeof(char) * strlen(surname) +1);
    strcpy(c->surname, surname);
    c->street = malloc(sizeof(char) * strlen(street) +1);
    strcpy(c->street, street);
    c->postal = malloc(sizeof(char) * strlen(postal) +1);
    strcpy(c->postal, postal);
    c->city = malloc(sizeof(char) * strlen(city) +1);
    strcpy(c->city, city);
    return c;
}

void Customer_print(Customer *c){
    printf("%ld~%s~%s~%s~%d~%s~%s~%f \n",
	   c->custid,
	   c->surname,
	   c->name,
	   c->street,
	   c->number,
	   c->city,
	   c->postal,
	   c->money);
}

void Customer_update(Customer *c, float money){
    if(c != NULL){;
	c->money += money;
    }
}

void Customer_destroy(Customer *c){
    free(c->name);
    free(c->surname);
    free(c->street);
    free(c->city);
    free(c->postal);
    free(c);
}
