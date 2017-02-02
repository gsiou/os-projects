#include "twa.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

Twa* Twa_create(int hash_bucket_size, float hash_max_load_factor){
    /* Creates twa structure
     * Allocates memory 
     * Returns pointer to it */
    Twa *t = malloc(sizeof(Twa));
    t->list = List_create();
    t->hash = LinearHash_create(hash_bucket_size, hash_max_load_factor);
    return t;
}

int Twa_load(Twa *t, char* path){
    /* Reads from file at path 
     * Inserts records in the struct 
     * Returns number of records inserted 
     * or -1 if file was not found */

    /* Define temporary record for retrieving data from files */
    struct Record{
	long long custid; /* long long for 32 bit support */
	char 	  surname[FILE_BUFF_SIZE];
	char 	  name[FILE_BUFF_SIZE];
	char	  street[FILE_BUFF_SIZE];
	int 	  number;	
	char	  city[FILE_BUFF_SIZE];
	char 	  postal[FILE_BUFF_SIZE2];
	float  	  money;
    } rec;
	
    FILE *f;
    long size;
    int n;
    f = fopen(path, "rb");
    if(f == NULL){
	printf("Error opening file! \n");
	return -1;
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    rewind(f);
    n = (int) size/sizeof(rec);
    int i;
    Customer *c;
    for(i = 0; i < n; i++){
	fread(&rec, sizeof(rec), 1, f);
	Twa_insert(t,(long) rec.custid, rec.surname, rec.name, rec.street, rec.number, rec.city, rec.postal, rec.money);
    }
    fclose(f);
    return n;
}

int Twa_insert(Twa *t, long custid, char *name, char *surname, char *street,
		int number, char *city, char *postal, float money){
    /* Returns 1 if insertion was successful ,2 if id existed and was updated 
     * or 0 if there was an error. */
    
    /* First check if we have all the data */
    BOOL error = FALSE;
    if(custid < 0 || money < 0 || number < 0)
	error = TRUE;
    else if(strcmp(name, "") == 0 || strcmp(surname, "") == 0 ||
	    strcmp(street, "") == 0 || strcmp(postal, "") == 0 ||
	    strcmp(city, "") == 0)
	error = TRUE;

    if(error){
	return 0; /* Error printing is handled outside */
    }

    /* Check if customer id already exists */
    Customer *temp;
    temp = LinearHash_search_id(t->hash, custid);
    if(temp != NULL){
	/* Customer already in twa */
	/* Updating money field and re-inserting */
	Customer_update(temp, money);
	List_reinsert(t->list, temp);
	//printf("Record updated \n\n");
	return 2;
    }
    
    Customer *c = Customer_create(custid, name, surname, street, number, city, postal, money);
    List_addCustomer(t->list, c);
    LinearHash_add(t->hash, c, FALSE);
    return 1;
}

void Twa_print(Twa *t){
    /* Prints information about current status of twa (for debugging) */
    List_print(t->list);
    LinearHash_print(t->hash);
}

void Twa_query(Twa *t, long id){
    /* Finds customer with id in O(1) complexity 
     * If he exists prints the record
     * if not, error message */
    Customer *c = LinearHash_search_id(t->hash, id);
    if(c != NULL)
        Customer_print(c);
    else
	printf("record with id:%ld does not exist\n", id);
}

void Twa_top(Twa *t, int k){
    /* Makes the list print top k records */
    List_print_top(t->list, k);
}

void Twa_bottom(Twa *t, int k){
    /* Makes the lsit print the last k records */
    List_print_bot(t->list, k);
}

void Twa_range(Twa *t, long id1, long id2){
    /* Prints records between id1 and id2 */
    
    /* Check if we have correct data */
    if(id1 < 0 || id2 < 0)
	printf("Wrong input! \n");
    
    Customer *c1 = LinearHash_search_id(t->hash, id1);
    Customer *c2 = LinearHash_search_id(t->hash, id2);
    Customer *temp;
    
    if(c1->money < c2->money){
	/* Sort them */
	temp = c1;
	c1 = c2;
	c2 = temp;
    }
    
    if(c1 == NULL){
	printf("Customer with custid %ld does not exist!\n", id1);
	return ;
    }
    if(c2 == NULL){
	printf("Customer with custid %ld does not exist!\n", id2);
	return ;
    }
    
    List_print_from_to(t->list, c1->next, c2);
}

float Twa_average(Twa *t){
    /* Returns average money amount from list */
    return List_avg(t->list);
}

float Twa_percentile(Twa *t, long custid){
    /* Prints percentile of records
     * Returns -1 if customer does not exists  or 
     * the percentile if everything is fine */
    Customer *c = LinearHash_search_id(t->hash, custid);
    if(c == NULL){
	return -1;
    }
    else{
	float p = List_percentile(t->list, custid);
	return p;
    }
}

void Twa_destroy(Twa *t){
    /* Destroys list, hash and frees the pointer */
    LinearHash_destroy(t->hash);
    List_destroy(t->list);
    free(t);
}
