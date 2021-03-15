#include "list.h"

List *createList() {
	List *l = (List*)malloc(sizeof(List));
	l->sig = List_SIG;
    l->first = NULL;
    return l;
}

void freeList(List *l){
	if (!l) { return; }
	Info *pi = l->first;
	while(pi != NULL){
		Info *nextNode = pi->dict_next;
		free(pi);
		pi = nextNode;
	}
	free(l);
}


void append(List *l, Info *pi) {
	pi->dict_next = l->first;
	l->first = pi;
}

Info *getFirstInfo(List *l) {
	if (!l || !l->first) { return NULL; }
	Info *pi = l->first;
	
	l->first = pi->dict_next;
	return pi;
}

Info *search(List *l, char *key) {
	if (!l) { return NULL; }
	for (Info *p = l->first; p; p = p->dict_next) {
		if (strcmp(p->key, key) == 0) { return p; }
	}
	return NULL;
}

void printList(List *l) {
	Info *pi = l->first;
	for (Info *p = l->first; p; p = p->dict_next) {
		printf("%s(%d) ", pi->key, pi->counter);
	}
	printf("\n");
}
