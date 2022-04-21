#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "server/hashmap.h"
#include "server/utils.h"

hashmap_t* init_hashmap_malloc(int (*hash_func)(const char* str), size_t size)
{
    hashmap_t* hashmap;
    hashmap = malloc(sizeof(hashmap));
    hashmap->hash_func = hash_func;
    hashmap->list = malloc(size * sizeof(int));

    return hashmap;
}

int add_key(hashmap_t* hashmap, const char* key, int value)
{
    int code = hashmap->hash_func(key) % HASHMAP_SIZE;
    mapnode_t** pp = &(hashmap->list[code]);
    mapnode_t* new_node;

    while (*pp != NULL) {
        if (strcmp((*pp)->key, key) == 0) {
            return FALSE;
        }
        pp = &(*pp)->next;
    }

    new_node = malloc(sizeof(mapnode_t));
    new_node->value = value;
    new_node->key = malloc(strlen(key) + 1);
    strcpy(new_node->key, key);

    *pp = new_node;

    return TRUE;
}

int remove_key(hashmap_t* hashmap, const char* key)
{
    int code = hashmap->hash_func(key) % HASHMAP_SIZE;
    mapnode_t** pp = &hashmap->list[code];

    while (*pp != NULL) {
        if (strcmp((*pp)->key, key) == 0) {
            mapnode_t* next = (*pp)->next;
            free(*pp);
            *pp = next;

            return TRUE;
        }
        pp = &(*pp)->next;
    }

    return FALSE;
}

int get(hashmap_t* hashmap, const char* key)
{
    int code = hashmap->hash_func(key) % HASHMAP_SIZE;
    mapnode_t* p = hashmap->list[code];

    while (p != NULL) {
        if (strcmp(p->key, key) == 0) {
            return p->value;
        }
        p = p->next;
    }

    return -1;
}

void destroy_hashmap(hashmap_t* hashmap)
{
    size_t i;

    for (i = 0; i < HASHMAP_SIZE; ++i) {
        mapnode_t* node = hashmap->list[i];
        while (node != NULL) {
            mapnode_t* next = node->next;

            free(node->key);
            free(node);
            
            node = next;
        }
    }

    free(hashmap->list);
    free(hashmap);
}

void print_keys(hashmap_t* hashmap)
{
    size_t i;
    printf("[ ");
    for (i = 0; i < HASHMAP_SIZE; ++i) {
        mapnode_t* p = hashmap->list[i];
        while (p != NULL) {
            printf("%s, ", p->key);
            p = p->next;
        }
    }
    printf("]\n");
}
