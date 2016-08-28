#include <stdlib.h>
#include <stdio.h>
#include "list.h"


void list_init(List* l){
    l->first = NULL;
    l->last = &(l->first);
}

void list_alloc(List** l){
    *l = (List *) malloc(sizeof(List));
    list_init(*l);
}



ListNode* listnode_new(){
    ListNode* ln = (ListNode *) malloc(sizeof(ListNode));
    return ln;
}

void listnode_free(ListNode* l){
    free(l);
}

void list_append(List* l, void* data){
    ListNode* ln = listnode_new();
    ln->data = data;
    ln->next = NULL;
    *(l->last) = ln;
    l->last = &(ln->next);
}

void* list_pop(List* l){
    if(l->first == NULL){
        return NULL;
    }

    void* out = l->first->data;
    ListNode* old = l->first;
    l->first = l->first->next;
    if(l->first == NULL){
        l->last = &(l->first);
    }
    free(old);
    return out;
}

void list_print(List *l){
    ListNode* ln = l->first;
    while(ln != NULL){
        printf("%p->", ln->data);
        ln = ln->next;
    }
    printf("END\n");
}

void list_free(List* l){
    ListNode* ln = l->first;
    ListNode* tmp;
    while(ln != NULL){
        tmp = ln;
        ln = ln->next;
        listnode_free(tmp);
    }

    free(l);
}