#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include "rtree.h"
#include "geometry.h"
#include "utils.h"
#include "list.h"

#define debug 1

int rtree_alloc(Rtree** tp, unsigned m, unsigned M, unsigned dim){
    assert(m <= M/2);
	*tp = (Rtree*) malloc(sizeof(Rtree));
	Rtree *t = *tp;
	if(t==NULL){
		return -1;
	}
	t->m = m;
	t->M = M;
	t->dim = dim;
	if(node_alloc_no_rec(&(t->root), M, dim) < 0){
        return -1;
    }

    t->root->parent = t->root;
    return 0;
}

int node_alloc(Node** np, unsigned M, unsigned dim){
	*np = (Node*) malloc(sizeof(Node));
	Node* n = *np;
	if(n == NULL){
		return -1;
	}

	n->entries = (Entry**) calloc(M+1, sizeof(Entry*)); // +1 for extra space used when split
	n->nb_entries = 0;
    n->r = NULL;
	return rec_alloc(&(n->r), dim, NULL);
}

int node_alloc_no_rec(Node** np, unsigned M, unsigned dim){
    *np = (Node*) malloc(sizeof(Node));
    Node* n = *np;
    if(n == NULL){
        return -1;
    }

    n->entries = (Entry**) calloc(M+1, sizeof(Entry*)); // +1 for extra space used when split
    n->nb_entries = 0;
    n->r = NULL;
    return 0;
}

void node_free(Node* n){
    free(n->entries);
    free(n);
}

void node_free_deep(Node* n){
    if(debug)
        printf("Freeing node at %p\n", n);
    int i;
    for(i=0;i<n->nb_entries;i++){
        rec_free(n->entries[i]->r);
        free(n->entries[i]);
    }
    free(n->entries);
    if(debug)
        printf("Freeing rec %p\n", n->r);
    free(n);
}

void node_print(Node* n){
	if(n != NULL){
		printf("Node at %p:\n", n);
		printf("\tNumber entries: %d\n", n->nb_entries);
		if(n->r != NULL){
			rec_print(n->r);
		}
		else{
			printf("Rectangle is NULL\n");
		}
	}
	
}

int node_isleaf(Rtree* t, Node* n){
	if(n == NULL || t == NULL){
		printf("Error: parameters can't be NULL is node is leaf\n");
		exit(1);
	}
	if(n->nb_entries >= t->m){
		if(n->entries == NULL){
			printf("ERROR: nb entries > 0 but entries pointer is null\n");
			exit(1);
		}
		return n->entries[0]->node == NULL;
	}
	else{
		return n == t->root;
	}

	// Suppose that if no entries it is an empty root considered as leaf
	return 0;
}


int node_isroot(Node* n){
    return n == n->parent;
}

int node_insert(const Rtree* t, Node* n, Entry *e){
    assert(n->nb_entries <= t->M);
    //printf("Inserting entry %p at index %d in node %p, rectangle is %p\n", e, n->nb_entries, n, e->r);
    n->entries[n->nb_entries] = e;
    n->nb_entries++;
    if(e->node != NULL){
        e->node->parent = n;
    }
    return n->nb_entries == t->M+1 ? 0 : 1;
}




/*
	Node is normally null when called by user. It used for recursion 
	when descending down the tree.
	QuerySet should already have been allocated. The job of this function
	is only to fill it, possibly by allocating List objects. You are therefore
	responsible for the allocation of the creation of the QuerySet object 
	and deallocation via the method queryset_free when done with it, to 
	ensure that no memory is leaked.

*/
void search(Rtree* t, Node* n, Rectangle* r, QuerySet* entries){
    assert(t != NULL);
	Node* cur_n;
	if(n == NULL){
		cur_n = t->root;
	}
    else{
        cur_n = n;
    }
    assert(cur_n != NULL);
	int i=0;
	if(node_isleaf(t, cur_n)){
		for(i=0; i<cur_n->nb_entries;i++){
			if(rec_overlap(r, (cur_n->entries)[i]->r)){
				queryset_insert(entries, (cur_n->entries)[i]->id);
			}
		}
	}
	else{
		for(i=0; i<cur_n->nb_entries;i++){
			if(rec_overlap(r, (cur_n->entries)[i]->r)){
				search(t, (cur_n->entries)[i]->node, r, entries);
			}         
		}
	}
}


// Use this wrapper to dynamically allocate temp vars once and 
// for all, and reuse them for each step of the recurrence instead
// of re-allocating them every time before freeing them.
Node* choose_leaf(Rtree* t, Node* n, Rectangle* r){

	Rectangle* r_fit;
	Rectangle* r_fit2;
	rec_alloc(&r_fit, r->dim, NULL);
	rec_alloc(&r_fit2, r->dim, NULL);

	Node* out = _choose_leaf(t, n, r, &r_fit, &r_fit2);

	rec_free(r_fit);
	rec_free(r_fit2);
    if(debug)
        printf("Chosen leaf is: %p\n", out);
	return out;

}
/*
	Chooses a leaf in which to insert the Rectangle r. Should 
	first be invoked with n=NULL.
*/
Node* _choose_leaf(Rtree* t, Node* n, Rectangle* r, Rectangle** r_best, Rectangle** r_t){
	if(n == NULL){
		n = t->root;
	}

    if(node_isleaf(t, n)){
        return n;
    }

	int i = 0;

    assert(n->nb_entries > 0);
	if(n->nb_entries == 0){
		// Should never happen, non-leaf node with 0 entry...
        printf("WARNING: non-leaf node with 0 entry");
        return NULL;
		
	}
	double s, e; // temporary variables used in the loop
	int index = 0;
	Rectangle* r_cur = (n->entries)[0]->r;
	rec_get_encompassing(r, r_cur, *r_best);
	double min_surface = rec_get_surface(*r_best);
	double enlargement = min_surface - rec_get_surface(r_cur);

	for(i=1;i<n->nb_entries;i++){
		r_cur = (n->entries)[i]->r;
		rec_get_encompassing(r, r_cur, *r_t);
		s = rec_get_surface(*r_t);
		if(s < min_surface){
			min_surface = s;
			swap((void **) &r_best, (void **) &r_t);
			index = i;
		}
		else if(s == min_surface){ 
			// choose the one that needs least enlargement
			e = min_surface - rec_get_surface(r_cur);
			if(e < enlargement){
				// choose r_cur
				enlargement = e;
				index = i;
				swap((void **) &r_best, (void **) &r_t);
			}
		}
	}

	return _choose_leaf(t, (n->entries)[index]->node, r, r_best, r_t);
}


int entry_alloc(Entry** e, int id, Node* n){
    *e = (Entry*) malloc(sizeof(Entry));
    if(*e == NULL){
        return -1;
    }
    (*e)->id = id;
    (*e)->node = n;
    (*e)->r = n->r;
    return 0;
}


void node_adjust(Node* n){
    Rectangle* r1 = n->r;
    Rectangle* r2;
    int i;
    for(i=0;i<n->nb_entries;i++){
        r2 = (n->entries)[i]->r;
        double* l1 = *(r1->coords+i);
        double* h1 = l1;
        double* l2 = *(r2->coords+i);
        double* h2 = l2;

        // sets l1 to the lowest value of the interval in this dimension
        ((r1->coords)[i][0] > (r1->coords)[i][1]) ? l1++ : h1++;
        // sets l2 to the lowest value of the interval in this dimension
        ((r2->coords)[i][0] > (r2->coords)[i][1]) ? l2++ : h2++;

        (r1->coords)[i][0] = min(*l1, *l2);
        (r1->coords)[i][1] = max(*h1, *h2);
    }
}


/*
    k: the node from which we are adjusting. 
    l and ll are not NULL if k has been splitted and the adjustment aims at 
    adding them to the parent node, and freeing k.

    Allocations:
        * Allocates entry in the parent node when adding a new node created 
        from a split. An entry is freed when the tree is, or when it is 
        deleted. 
        * Allocates two nodes when inserting in parent node needs splitting.
        These nodes are deallocated when they are themselves splitted or when 
        the tree is freed.

    Deallocations:
        * Old nodes are freed when splitted. It is not a deep deallocation 
        because the entries are still valid and must not be freed. The 
        corresponding rectangle is nevertheless deallocated as we don't need
        it anymore.

*/
void adjust_tree(Rtree* t, Node* k, Node* l, Node* ll){
    if(debug)
        printf("Adjusting node %p %p %p\n", k, l, ll);
    if(l != NULL){
        rec_print(l->r);
        rec_print(ll->r);
    }
    Node* n = k;
    Node* p;
    if(node_isroot(k)){
        p = k;
        if(l == NULL){
            rec_adjust_to_fit(p->r, (const Rectangle *) n->r);
            return;
        }
        // The root was splitted
        if(debug)
            printf("Growing the root %p\n", t->root);
        assert(ll != NULL);
        rec_adjust_to_fit(p->r, (const Rectangle *) ll->r);
        rec_adjust_to_fit(p->r, (const Rectangle *) l->r);
        Node* new_root;
        assert(node_alloc_no_rec(&new_root, t->M, 2) == 0);
        Entry* new_e1;
        Entry* new_e2;
        assert(entry_alloc(&new_e1, -1, l) == 0);
        assert(entry_alloc(&new_e2, -1, ll) == 0);

        node_insert((const Rtree *) t, new_root, new_e1);
        node_insert((const Rtree *) t, new_root, new_e2);
        new_root->r = p->r; // That's why we adjusted the rectangle
        // associated with the soon-to-be-deleted root. This way we don't
        // have to free it and allocate a new one. We simply re-use it.

        node_free(p);
        t->root = new_root;
        new_root->parent = new_root;
        if(debug)
            printf("New root is %p\n", new_root);
        return;
    }


    p = k->parent;
    int i;
    for(i=0;i<p->nb_entries;i++){
        if(p->entries[i]->node == n){
            break;
        }
    }
    assert(i<p->nb_entries);

    if(l != NULL){
        // the node was split, l will replace k in the corresponding parent
        // slot and ll will be added to the parent. 
        assert(ll != NULL);
        node_free(k);
        n = l;
        l->parent = p;

        // Update the area of the parent's entry corresponding to the the Node
        Entry* e = p->entries[i];
        if(e->r != n->r){
            // The area for the child node has been updated. Since there has
            // been an insertion, it is very likely that these rectangles are
            // different in terms of coordinates.
            rec_free(e->r);
            e->r = n->r;
            e->node = n;
        }
    }
    else{
        // Either l and ll are NULL, or none is.
        assert(ll == NULL);
        rec_adjust_to_fit(p->r, (const Rectangle *) n->r);
        // There is no new splitted node to add, we just need to
        // propagate the area adjustment up.
        return adjust_tree(t, p, NULL, NULL);
    }

    // Now only concerns splitting cases


    // We adjust the area for the parent Node, no new allocation is needed
    rec_adjust_to_fit(p->r, (const Rectangle *) n->r);

    // Try to insert the second node resulting from the previous split
    // Create to new entry to put the node to.
    Entry* new_entry;
    assert(entry_alloc(&new_entry, -1, ll) == 0);
    if(!node_insert((const Rtree *) t, p, new_entry)){
        // The parent node was full... split it and proceed samewise.
        Node* n1;
        Node* n2;
        // Nodes allocated here are deleted when splitted or when 
        // the tree is freed
        assert(node_alloc_no_rec(&n1, t->M, 2) == 0); 
        assert(node_alloc_no_rec(&n2, t->M, 2) == 0);

        // Split the parent Node, we adjust the area for efficiency in order
        // to re-use the rectangle if we need to regenerate a new root.
        quadratic_split((const Rtree*) t, p, n1, n2);

        // If we have splitted the root, create a new root with the 
        // two resulting Nodes.
        if(node_isroot(p)){
            if(debug)
                printf("Growing the root\n");
            assert(t->root == p); // For debug purpose. Should eventually be deleted
            rec_adjust_to_fit(p->r, (const Rectangle *) ll->r);
            Node* new_root;
            assert(node_alloc_no_rec(&new_root, t->M, 2) == 0);
            Entry* new_e1;
            Entry* new_e2;
            assert(entry_alloc(&new_e1, -1, n1) == 0);
            assert(entry_alloc(&new_e2, -1, n2) == 0);

            node_insert((const Rtree *) t, new_root, new_e1);
            node_insert((const Rtree *) t, new_root, new_e2);
            new_root->r = p->r; // That's why we adjusted the rectangle
            // associated with the soon-to-be-deleted root. This way we don't
            // have to free it and allocate a new one. We simply re-use it.

            node_free(p);
            t->root = new_root;
            new_root->parent = new_root;
            return;
        }

        // Now, this is not the root we have just splitted. We propagate 
        // the adjustement with the insertion of the two new nodes one step
        // higher.

        return adjust_tree(t, p, n1, n2);

    }
    
    // The second Node has been correclty inserted.
    // We adjust the area of the parent Node for the newly inserted entry.
    // Maybe we should move the adjustment logic to the node_insert 
    // function. 

    rec_adjust_to_fit(p->r, (const Rectangle *) ll->r);

    return adjust_tree(t, p, NULL, NULL);

}  


int insert(Rtree* t, Entry* e){
    assert(t != NULL);
    assert(e != NULL);
    Node* n = choose_leaf(t, NULL, e->r);
    assert(n != NULL);
    if(debug){
        printf("Inserting rectangle %p\n", e->r);
        rec_print(e->r);
    }
    
    // First insertion
    if(n == t->root && t->root->r == NULL){
        rec_copy(&(t->root->r), e->r);
    }

    if(!node_insert(t, n, e)){
        // We have to split the Node !
        Node* n1;
        Node* n2;
        // Nodes allocated here are deleted when splitted or when 
        // the tree is freed
        assert(node_alloc_no_rec(&n1, t->M, 2) == 0); 
        assert(node_alloc_no_rec(&n2, t->M, 2) == 0);

        quadratic_split((const Rtree*) t, (const Node* ) n, n1, n2);

        adjust_tree(t, n, n1, n2);
        return 0;
    }
    rec_adjust_to_fit(n->r, (const Rectangle *) e->r);
    adjust_tree(t, n, NULL, NULL);

    return 1;
}


void pick_seeds(const Node* n, Entry** e1, Entry** e2){
    assert(n != NULL);
    int i,j;
    double d, tmp;
    d = -DBL_MAX;
    Rectangle* enc;
    assert(rec_alloc(&enc, 2, NULL) >= 0);
    *e1 = NULL;
    *e2 = NULL;
    for(i=0;i<n->nb_entries-1;i++){
        for(j=i+1;j<n->nb_entries;j++){ // to nb_entries + 1 for extra space
            Entry* e1_tmp = n->entries[i];
            Entry* e2_tmp = n->entries[j];


            assert(e1_tmp != NULL);
            assert(e2_tmp != NULL);
            /*printf("%d\n", j);
            printf("%p\n",e1_tmp->r);
            printf("%p\n",e2_tmp->r);*/
            rec_get_encompassing((const Rectangle*) e1_tmp->r, 
                (const Rectangle*) e2_tmp->r, enc);
/*            printf("Encompassing:\n");
            rec_print(enc);
            printf("Surface: %lf\n", rec_get_surface(enc));
            printf("Surface e1: %lf\n", rec_get_surface(e1_tmp->r));
            printf("Surface e2: %lf\n", rec_get_surface(e2_tmp->r));*/
            tmp = rec_get_surface(enc) - rec_get_surface(e1_tmp->r) - 
                    rec_get_surface(e2_tmp->r);
            if(tmp > d){
                d = tmp;
                *e1 = e1_tmp;
                *e2 = e2_tmp;
            }
        }
    }
    assert(*e1 != NULL);
    assert(*e2 != NULL);
    rec_free(enc);
}



void qs_insert(const Rtree* t, Node* n, Entry* e, int* score, int* remaining){
    node_insert(t, n, e);
    (*score)++;
    (*remaining)--;
}


void print_entry(Entry* e){
    printf("Entry at %p:\n", e);
    printf("\tid: %d\n", e->id);
    printf("\tnode: %p\n", e->node);
    rec_print(e->r);
}


/*
    We could re-implement this function where, instead of allocating two 
    new nodes, we would reuse the splitted node as a shell for one of the
    two new nodes. We would this way avoid one node allocation, but at the
    expense of clarity. 

    This function allocates two rectangles. They are freed when adjusting 
    the tree during subsequent splits or when freeing the tree. 
*/
void quadratic_split(const Rtree* t, const Node* n, Node* n1, Node* n2){
    if(debug)
        printf("Splitting node %p with %d entries\n", n, n->nb_entries);
    Entry* e1;
    Entry* e2;
    pick_seeds(n, &e1, &e2);
    int m = t->m;
    int nb = n->nb_entries;
    int i = 0; // loop variable
    int a1 = 1; // added to Node n1, there's already the seed of the group
    int a2 = 1;
    int e1_or_e2 = 0; // counter of e1 or e2 occurrence
    //a2 = i - e1_or_e2 - a1 at any point in the loop

    int remaining = nb - 2; // entries remaining after i loops : nb - (m - e1_or_e2) - i
    double d1, d2, s1, s2, s1_tmp, s2_tmp;
    Entry* e;
    Rectangle* r_tmp1;
    Rectangle* r_tmp2;
    Rectangle* r1; // Pointer to the rectangle that encompasses group 1 at loop i
    Rectangle* r2; // Pointer to the trectangle that encompasses group 2 at loop i
    assert(rec_alloc(&r_tmp1, 2, NULL) >= 0);
    assert(rec_alloc(&r_tmp2, 2, NULL) >= 0);
    assert(rec_alloc(&r1, 2, (const COORD_TYPE*) e1->r->coords) >= 0); 
    assert(rec_alloc(&r2, 2, (const COORD_TYPE*) e2->r->coords) >= 0); 
    s1 = rec_get_surface(e1->r);
    s2 = rec_get_surface(e2->r);
    node_insert(t, n1, e1);
    node_insert(t, n2, e2);

    for(i=0;i<nb;i++){
        e = n->entries[i];
        if(e == e1 || e == e2){
            e1_or_e2++;
            continue;
        }
        printf("%d %d %d %d\n", m, a1, a2, remaining);
        if(m-a1 == remaining){ 
            // everything entry left should be added to a1 to 
            // reach the minimum of m entries
            qs_insert(t, n1, e, &a1, &remaining);
            continue;
        }
        
        else if(m-a2 == remaining){
            // Same for n2
            qs_insert(t, n2, e, &a2, &remaining);
            continue;
        }
        
        rec_get_encompassing((const Rectangle *) e->r, 
                             (const Rectangle *) r1, 
                             r_tmp1);
        rec_get_encompassing((const Rectangle *) e->r, 
                             (const Rectangle *) r2, 
                             r_tmp2);
        s1_tmp = rec_get_surface(r_tmp1);
        d1 = s1_tmp - s1;
        s2_tmp = rec_get_surface(r_tmp2);
        d2 = s2_tmp - s2;
        // Compare the increase in sufaces
        if(d1 < d2){
            // Add to group 1
            s1 = s1_tmp;
            swap((void **) &r1, (void **) &r_tmp1);
            qs_insert(t, n1, e, &a1, &remaining);
        }
        else if(d2 < d1){
            // Add to group 2
            s2 = s2_tmp;
            swap((void **) &r2, (void **) &r_tmp2);
            qs_insert(t, n2, e, &a2, &remaining);
        }
        else{
            // Compare the surfaces
            if(s1_tmp < s2_tmp){
                s1 = s1_tmp;
                swap((void **) &r1, (void **) &r_tmp1);
                qs_insert(t, n1, e, &a1, &remaining);
            }
            else if(s2_tmp < s1_tmp){
                s2 = s2_tmp;
                swap((void **) &r2, (void **) &r_tmp2);
                qs_insert(t, n2, e, &a2, &remaining);
            }
            else{
                // Compare group sizes
                if(n1->nb_entries < n2->nb_entries){
                    s1 = s1_tmp;
                    swap((void **) &r1, (void **) &r_tmp1);
                    qs_insert(t, n1, e, &a1, &remaining);
                }
                else{
                    // In case of equality in the number of 
                    // entries we default to group 2 (chosen by a fair 
                    // coin toss :) )
                    s2 = s2_tmp;
                    swap((void **) &r2, (void **) &r_tmp2);
                    qs_insert(t, n2, e, &a2, &remaining);
                }
            }
        }

    }

    n1->r = r1;
    n2->r = r2;
    rec_free(r_tmp1);
    rec_free(r_tmp2);
    if(debug){
        printf("Node %p has been splitted into nodes %p and %p\n", 
        n, n1, n2);
        printf("The %d entries of %p were splitted into %d and %d entries\n", 
            n->nb_entries, n, n1->nb_entries, n2->nb_entries);
    }
    

}


void rtree_print(Rtree* t){
    printf("=======================\n\n");
    printf("Printing tree at %p. Root is at %p. m=%d, M=%d\n",
        t, t->root, t->m, t->M);
    List* to_fill;
    List* to_empty;
    List* rects;
    list_alloc(&to_fill);
    list_alloc(&to_empty);
    list_alloc(&rects);
    int i;
    Node* cur_node = t->root;
    printf("Current node: %p (R: %p), nb entries: %d   ", cur_node, cur_node->r, cur_node->nb_entries);
    list_append(rects, cur_node->r);
    for(i=0;i<cur_node->nb_entries;i++){
        Entry* e = (cur_node->entries)[i];
        printf("R: %p, N: %p id: %d  ", e->r, e->node, e->id);
        list_append(to_empty, (void *) e->node);
        list_append(rects, e->r);
    }
    printf("\n");
    int b = node_isleaf(t, cur_node);
    int j=0;
    do{
        if(j > 10){
            printf("Infinte loop\n");
            break;
        }
        j++;
        while((cur_node = list_pop(to_empty)) != NULL){
            printf("Current node: %p (R: %p), nb entries: %d (parent %p)  ", cur_node, cur_node->r, 
                cur_node->nb_entries, cur_node->parent);
            list_append(rects, cur_node->r);
            b = b || node_isleaf(t, cur_node);
            for(i=0;i<cur_node->nb_entries;i++){
                Entry* e = (cur_node->entries)[i];
                printf("R: %p, N: %p id: %d  ", e->r, e->node, e->id);
                list_append(to_fill, (void *) e->node);
                list_append(rects, e->r);
            }
            printf(" || ");
        }
        printf("\n\n");
        List *tmp;
        tmp = to_fill;
        to_fill = to_empty;
        to_empty = tmp;

        /*printf("~~~~~\n");
        list_print(to_empty);
        list_print(to_fill);
        printf("~~~~~\n");*/
    }while(!b);

    /*ListNode* ln = rects->first;
    while(ln != NULL){
        Rectangle* r = (Rectangle*) ln->data;
        printf("Rectangle at %p\n", r);
        rec_print(r);
        ln = ln->next;
    }*/


    printf("\n\n=======================\n");

    list_free(to_fill);
    list_free(to_empty);
    list_free(rects);
}



void rtree_free(Rtree* t){
    List* to_free;
    list_alloc(&to_free);
    list_append(to_free, t->root);
    rec_free(t->root->r);
    Node* cur_node;
    while((cur_node = list_pop(to_free)) != NULL){
        int i;
        for(i=0;i<cur_node->nb_entries;i++){
            list_append(to_free, (cur_node->entries)[i]->node);
        }
        node_free_deep(cur_node);
    }
    list_free(to_free);
    free(t);
}


int queryset_alloc(QuerySet* q){
	q = (QuerySet*) malloc(sizeof(QuerySet));
	list_alloc(&(q->values));
	q->size = 0;
	return 0;
}


int queryset_insert(QuerySet* q, unsigned data){
    assert(q!=NULL);
    unsigned* datap = (unsigned *) malloc(sizeof(unsigned));
    *datap = data;
    list_append(q->values, (void *) datap);
	q->size++;
	return q->size;
}
