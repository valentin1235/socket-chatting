#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdio.h>

#define HASHMAP_SIZE (10)

typedef struct node {
    int host;
    int port;
    struct node* next;
} mapnode_t;

typedef struct hashmap {
    int (*hash_func)(const char* str);
    mapnode_t** list;
} hashmap_t;


hashmap_t* init_hashmap_malloc(int (*hash_func)(const char* str), size_t size);
int add_key(hashmap_t* hashmap, const char* key, int value);
int remove_key(hashmap_t* hashmap, const char* key);
int get(hashmap_t* hashmap, const char* key);
void destroy_hashmap(hashmap_t* hashmap);
void print_keys(hashmap_t* hashmap);

#endif /* HASHMAP_H */
