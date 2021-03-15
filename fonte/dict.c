#include "dict.h"

static int jenkins_one_at_a_time_hash(const uint8_t* key, size_t length) {
    size_t i = 0;
    uint32_t hash = 0;
    while (i != length) {
        hash += key[i++];
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    // convertendo para hashkey do dict
    //long long int a = (long long int)hash;
    //int hashKey = a % DICT_SIZE;
    //return hashKey;
    return (int)(hash % DICT_SIZE);
}

DictInfo *createDict() {
    DictInfo *d = (DictInfo*)calloc(1, sizeof(DictInfo));
    return d;
}

void dictInsert(DictInfo *d, Info *pi) {
    int keyHash = jenkins_one_at_a_time_hash(pi->key, strlen(pi->key));
    append(&d->content[keyHash], pi);
}

Info *dictLocate(DictInfo *d, char *key) {
    int keyHash = jenkins_one_at_a_time_hash(key, strlen(key));
    return search(&d->content[keyHash], key);
}

void printDict(DictInfo *d) {
    for(int i = 0; i < DICT_SIZE; i++) {
        printf("Dict hash %d: ", i);
        printList(&d->content[i]);
        printf("\n");
    }
}
/*
void freeDict(DictInfo *d){
    for(int i = 0; i < DICT_SIZE; i++){
        freeList(&d->content[i]);
    }
    free(d);
}
*/
