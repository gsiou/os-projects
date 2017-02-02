#ifndef TWA_H
#define TWA_H

#define FILE_BUFF_SIZE 20
#define FILE_BUFF_SIZE2 6

#include "types.h"
#include "ll.h"
#include "hash.h"

typedef struct Twa{
    List       *list;
    LinearHash *hash;
} Twa;

Twa*      Twa_create     (int hash_bucket_size, float hash_max_load_factor);
int       Twa_load       (Twa *t, char* path);
int       Twa_insert     (Twa *t, long custid, char *name, char *surname, char *street,
			  int number, char *city, char *postal, float money);
void      Twa_print      (Twa *t);
void      Twa_query      (Twa *t, long id);
void      Twa_top        (Twa *t, int k);
void      Twa_bottom     (Twa *t, int k);
void      Twa_range      (Twa *t, long id1, long id2);
float     Twa_average    (Twa *t);
float     Twa_percentile (Twa *t, long custid);
void      Twa_destroy    (Twa *t);
#endif
