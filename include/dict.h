#ifndef _DICT_H_
#define _DICT_H_

#include <list.h>

#define N_ITEMS 50




typedef struct{
    struct list_head root;
    int len;
    /* int (* hash_fun)(void * key); // potser m'he semat una mica */
    /* int (* eq_fun)(void * key_a, void * key_b) */
}dict_info;

// len(dict_info) <= len(dict_item)

typedef struct{
    struct list_head lh;
    int key; /* void * key */
    void *value;
}dict_item;

typedef union {
    dict_item item;
    dict_info info; 
}dict;

extern struct list_head used_items;
extern struct list_head free_items;
extern dict pull_dict[N_ITEMS];


// python like functions 
dict *new_dict();

int contains(dict *d, int key);

int del_item(dict * d, int key);

void set_item(dict *d, int key, void *value);

void *get_item(dict *d, int key);

#endif  /* __SCHED_H__ */