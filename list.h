#ifndef LIST_H
#define LIST_H

typedef struct ListNode ListNode;
typedef struct List List;

struct ListNode
{
    void* data;
    ListNode* next;
};


struct List
{
    ListNode* first;
    ListNode** last;
};


void list_init(List* l);
void list_alloc(List** l);
ListNode* listnode_new();
void listnode_free(ListNode* l);
void list_append(List* l, void* data);
void* list_pop(List* l);
void list_print(List *l);
void list_free(List* l);
#endif