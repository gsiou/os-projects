#ifndef LL_H
#define LL_H
#include "types.h"

typedef struct Customer{
    long            custid;
    char            *name;
    char            *surname;
    char            *street;
    int             number;
    char            *city;
    char            *postal;
    float           money;
    struct Customer *next;
    struct Customer *prev;
} Customer;

typedef struct List{
    Customer        *start;
    Customer        *end;
    int             size;
} List;

/* List Functions */
List*     List_create          ();
void      List_addCustomer     (List* l,Customer * c);
void      List_print           (List *l);
void      List_print_top       (List *l, int k);
void      List_print_bot       (List *l, int k);
void      List_print_from_to   (List *l, Customer *c1, Customer *c2);
void      List_reinsert        (List* l, Customer *c);
float     List_avg             (List *l);
float     List_percentile      (List *l, long custid);
void      List_destroy         (List *l);
Customer* Customer_create      (long custid, char *name, char *surname, char *street,
			      int number, char *city, char *postal, float money);
void      Customer_print       (Customer *c);
void      Customer_update      (Customer *c, float money);
void      Customer_destroy     (Customer *c);
#endif
