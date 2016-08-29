#ifndef RTREE_H
#define RTREE_H
#include "geometry.h"
#include <stddef.h>
#include "list.h"

#define MAX_NODES 100
#define MIN_NODES MAX_NODES/2
#define DIM 2

/*
	Structures definitions
*/

typedef struct Node Node;
typedef struct QuerySet QuerySet;

struct QuerySet{
	List* values;
	unsigned size;
	
};


typedef struct Entry
{
	unsigned id;
	Node* node;
	Rectangle* r;
} Entry;

typedef struct Node
{
	unsigned nb_entries; // Either number of subnodes or number of entries
	Entry** entries;
	Rectangle* r; // Don't know if relevant. May be set to point to 
				  // parent node corresponding entry ?
    Node* parent;
} Node;


typedef struct Rtree {
	Node* root;
	unsigned m;
	unsigned M;
	unsigned dim;
} Rtree;


/*
	Rtree methods
*/

int rtree_alloc(Rtree** t, unsigned m, unsigned M, unsigned d);

int node_alloc(Node** n, unsigned M, unsigned dim);

int node_alloc_no_rec(Node** n, unsigned M, unsigned dim);

void node_adjust(Node* n);

void rtree_free(Rtree* t);

void node_free(Node *);

void node_free_deep(Node* n);

int node_isleaf(Rtree* t, Node* n);

int node_isroot(Node *);

int node_insert(const Rtree* t, Node* n, Entry *e);

void search(Rtree* t, Node* n, Rectangle* r, QuerySet* entries);

int insert(Rtree* t, Entry* e);

Node* choose_leaf(Rtree* t,Node* n, Rectangle* r);

Node* _choose_leaf(Rtree* t, Node* n, Rectangle* r, Rectangle** r_best, Rectangle** r_t);

void adjust_tree(Rtree* t, Node* k, Node* l, Node* ll);

//void split_node(Rtree* t, Node* n);

void node_print(Node* n);

void quadratic_split(const Rtree* t, const Node* n, Node* n1, Node* n2);

//void (*split_node)(const Node*, Node*, Node*) = quadratic_split;

void pick_seeds(const Node* n, Entry** e1, Entry** e2);


int entry_alloc(Entry** e, int id, Node* n);

void print_entry(Entry* e);

void rtree_print(Rtree* t);


/*
	QuerySet methods
*/

int queryset_alloc(QuerySet* q);

int queryset_insert(QuerySet* q, unsigned data);



#endif