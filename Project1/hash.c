#include "hash.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

Cell* Cell_create(long id, CellData content){
    Cell *c = malloc(sizeof(Cell));
    c->id = id;
    c->content = content;
    c->next = NULL;
    return c;
}

void Cell_destroy(Cell *c){
    free(c);
}

Bucket* Bucket_create(int bid, int bucket_size){
    Bucket *b = malloc(sizeof(Bucket));
    b->bucket_id = bid;
    b->bucket_size = bucket_size;
    b->cell_count = 0;
    b->first_cell = b->last_cell = NULL;
    return b;
}

void Bucket_addCell(Bucket *b, Cell *c){
    c->next = NULL;
    if(b->cell_count == 0){
	b->first_cell = b->last_cell = c;
    }
    else{
	b->last_cell->next = c;
	b->last_cell = c;
    }
    b->cell_count++;
}

Cell* Bucket_empty(Bucket *b){
    Cell *temp = b->first_cell;
    b->first_cell = NULL;
    b->last_cell = NULL;
    b->cell_count = 0;
    return temp;
}

void Bucket_destroy(Bucket *b){
    Cell *c = b->first_cell;
    Cell *temp;
    while(c != NULL){
	temp = c;
	c = c->next;
	Cell_destroy(temp);
    }
    free(b);
}

LinearHash* LinearHash_create(int bucket_size, float max_load_factor){
    LinearHash *h = malloc(sizeof(LinearHash));
    h->round = 0;
    h->load_factor = 0;
    h->max_load_factor = max_load_factor;
    h->current = 0;
    h->buckets = NULL;
    h->bucket_count = 0;
    h->bucket_size = bucket_size;
    int i;
    for(i = 0; i < START_BUCKETS; i++){
	LinearHash_addBucket(h);
    }
    h->milestone = 2 * START_BUCKETS;
    return h;
}

void LinearHash_destroy(LinearHash *h){
    int i;
    for(i = 0; i < h->bucket_count; i++){
	Bucket_destroy(h->buckets[i]);
    }
    free(h->buckets);
    free(h);
}

void LinearHash_addBucket(LinearHash *h){
    Bucket *b = Bucket_create(h->bucket_count, h->bucket_size);
    if(h->bucket_count == 0){
	h->buckets = malloc(sizeof(Bucket*));
    }
    else{
        h->buckets = realloc(h->buckets, sizeof(Bucket*) * (h->bucket_count + 1));
    }
    h->buckets[h->bucket_count] = b;
    h->bucket_count++;
}

void LinearHash_add(LinearHash *h, CellData data, BOOL use_next_hash){
    int id = data->custid;
    Cell *c = Cell_create(id, data);
    int b_index;
    if(use_next_hash){
	b_index = hash(h->round + 1, START_BUCKETS, id);
    }
    else{
	b_index = hash(h->round, START_BUCKETS, id);
	if(b_index < h->current){
	    b_index = hash(h->round + 1, START_BUCKETS, id);
	}
    }
    Bucket *b = h->buckets[b_index];
    Bucket_addCell(b, c);
    h->load_factor = (float) LinearHash_population(h) / (h->bucket_count * h->bucket_size);
    if(h->load_factor > h->max_load_factor){
	LinearHash_split(h);
	if(h->bucket_count >= h->milestone){
	    h->round++;
	    h->current = 0;
	    h->milestone = 2 * h->bucket_count;
	}
    }
}

void LinearHash_split(LinearHash *h){
    int bucket_id = h->current;
    int i = h->round;
    int m = h->bucket_count;
    int p = h->current;
    Bucket *b = h->buckets[bucket_id];

    /* Adding a new bucket */
    LinearHash_addBucket(h);
    
    /* Emptying bucket of all content
     * Preserving the cells */
    Cell *c;
    c = Bucket_empty(b);

    /* Rehashing all previous content with h(i+1) */
    Cell *temp = c;
    while(c != NULL){
	LinearHash_add(h, c->content, TRUE);
	temp = c;
	c = c->next;
	Cell_destroy(temp);
    }
    h->current++;
}

int LinearHash_population(LinearHash *h){
    int i, pop = 0;
    for(i = 0; i < h->bucket_count; i++){
	pop += h->buckets[i]->cell_count;
    }
    return pop;
}

void LinearHash_print(LinearHash *h){
    printf("Round: %d \n", h->round);
    printf("Number of buckets: %d \n", h->bucket_count);
    int i;
    Cell *c;
    for(i = 0; i < h->bucket_count; i++){
	printf("Bucket ID: %d\n", i);
	printf("Cells: %d\n", h->buckets[i]->cell_count);
	c = h->buckets[i]->first_cell;
	while(c != NULL){
	    printf("--->Cell ID: %ld \n", c->id);
	    c = c->next;
	}
    }
}

Customer* LinearHash_search_id(LinearHash *h, long id){
    int hashed = hash(h->round, START_BUCKETS, id);
    if(hashed < h->current){
	hashed = hash(h->round + 1, START_BUCKETS, id);
    }
    Cell *c = h->buckets[hashed]->first_cell;
    while(c != NULL){
	if(c->id == id){
	    return c->content;
	}
	c = c->next;
    }
    return NULL;
}

int hash(int i, int m, int x){
    return x % (((int)pow(2,i)) * m);
}
