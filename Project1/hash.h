#ifndef HASH_H
#define HASH_H

#include "ll.h"
#include "types.h"
#define START_BUCKETS 37

typedef Customer* CellData;

typedef struct Cell{
    long          id;
    CellData      content;
    struct Cell   *next;
} Cell;

typedef struct Bucket{
    Cell          *first_cell;
    Cell          *last_cell;
    int           bucket_id;
    int           bucket_size;
    int           cell_count;
} Bucket;

typedef struct LinearHash{
    int           round;
    int           bucket_size;
    int           bucket_count;
    int           milestone;
    int           current;
    float         load_factor;
    float         max_load_factor;
    Bucket        **buckets;
} LinearHash;

Cell*       Cell_create           (long id, CellData content);
void        Cell_destroy          (Cell *c);
Bucket*     Bucket_create         (int bid, int bucket_size);
void        Bucket_addCell        (Bucket *b, Cell *c);
Cell*       Bucket_empty          (Bucket *b);
void        Bucket_destroy        (Bucket *b);
LinearHash* LinearHash_create     (int bucket_size, float max_load_factor);
void        LinearHash_addBucket  (LinearHash *h);
void        LinearHash_add        (LinearHash *h, CellData data, BOOL use_next_hash);
int         LinearHash_population (LinearHash *h);
void        LinearHash_print      (LinearHash *h);
void        LinearHash_split      (LinearHash *h);
void        LinearHash_destroy    (LinearHash *h);
Customer*   LinearHash_search_id  (LinearHash *h, long id);
int         hash                  (int i, int m, int x);
#endif
