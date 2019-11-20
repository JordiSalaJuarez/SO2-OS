#include <dict.h>


struct list_head used_items;
struct list_head free_items;
dict pull_dict[N_ITEMS];


dict *new_dict(){
    if(!list_empty(&free_items)){
        struct list_head *d = list_first(&free_items);
        list_del(d);
        INIT_LIST_HEAD(d);
        dict_info *info = (dict_info *) d;
        info->len = 0;
        return (dict *) info;
    }else{
        return -1;
    }
}

int contains(dict * d, int key){
    struct list_head *item_lh;
    dict_item *it;
    list_for_each(item_lh, &(d->info.root)){
        it = (dict_item *) item_lh;
        if (it->key == key)return 1;
    }
    return 0;
}


void *get_item(dict * d, int key){
    struct list_head *item_lh;
    dict_item *it;
    list_for_each(item_lh, &(d->info.root)){
        it = (dict_item *) item_lh;
        if (it->key == key)return it->value;
    }
    // 'cointains' should be called before 'get_item' 
    return -1;  // throw keyError
}

int del_item(dict * d, int key){
    struct list_head *item_lh;
    dict_item *it;
    list_for_each(item_lh, &(d->info.root)){
        it = (dict_item *) item_lh;
        if (it->key == key){
            list_del(item_lh);
            list_add_tail(item_lh, &free_items);
            --d->info.len;
            return 1;
        }
    }
    return 0;
}

void set_item(dict * d, int key, void *value){
    struct list_head *item_lh;
    dict_item *it;
    list_for_each(item_lh, &(d->info.root)){
        it = (dict_item *) item_lh;
        if (it->key == key){
            it->value = value;
            return;
        }
    }
    struct list_head *new = list_first(&free_items);
    list_del(new);
    list_add_tail(new, &(d->info.root));
    dict_item *new_item = new;
    new_item->key = key;
    new_item->value = value;
    ++d->info.len;
}