#include <stdio.h>
#include <float.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include "rtree.h"
#include "utils.h"
#include "list.h"

inline int set_coords2(COORD_TYPE* c, double a1, double a2, double a3, double a4){
	c[0][0] = a1;
	c[0][1] = a2;
	c[1][0] = a3;
	c[1][1] = a4;
}


int test_operations(){
	int success=1;
	double a1 = DBL_MAX;
	double a2 = -DBL_MAX;

	success = success & (sgn(a1) == sgn(-a2));
	if(!success){
		printf("Test failed, got %d and %d, but expected 1 and 1\n", sgn(a1), sgn(a2));
	}

	success = success & ((is_limit(a1) == 1) && (is_limit(a2) == 1));
	if(!((is_limit(a1) == 1) && (is_limit(a2) == 1))){
		printf("Test failed, got %d and %d, but expected 1 and 1\n",is_limit(a1), is_limit(a2));
	}
	double r = sub(a1, a2);
	success = success & (r == DBL_MAX);
	if(!(r == DBL_MAX)){
		printf("Test failed, got %lf, expected +INF\n", r);
	}

	a1 = 1;
	a2 = -DBL_MAX;
	r = sub(a1, a2);
	success = success & (r == DBL_MAX);
	if(!(r == DBL_MAX)){
		printf("Test failed, got %lf, expected +INF\n", r);
	}

	return success;
}

int test_surface(){
	int success = 1;
	double s;
	Rectangle* r1, *r2;
	COORD_TYPE c1[2];
	COORD_TYPE c2[2];
	set_coords2(c1, 1, 2, 1, 2);
	set_coords2(c2, 1.5, 3, 1, 4);

	if(rec_alloc(&r1, 2, (const COORD_TYPE*) c1) < 0){
		printf("Error allocating");
		exit(1);
	}
	if(rec_alloc(&r2, 2, (const COORD_TYPE*) c2) < 0){
		printf("Error allocating");
		exit(1);
	}

	s = rec_get_surface(r1);

	success = success & (s == 1);
	if(!success){
		printf("Test failed 1, result is %lf, and expected %lf\n", s, 1.);
	}

	s = rec_get_surface(r2);

	success = success & (s == 4.5);
	if(!success){
		printf("Test failed 2, result is %lf, and expected %lf\n", s, 4.5);
	}

	rec_free(r1);
	rec_free(r2);

	set_coords2(c1, -DBL_MAX, 2, 1, 2);
	set_coords2(c2, 1.5, -DBL_MAX, 1, DBL_MAX);

	if(rec_alloc(&r1, 2, (const COORD_TYPE*) c1) < 0){
		printf("Error allocating");
		exit(1);
	}

	if(rec_alloc(&r2, 2, (const COORD_TYPE*) c2) < 0){
		printf("Error allocating");
		exit(1);
	}

	s = rec_get_surface(r1);

	success = success & (s == DBL_MAX);
	if(!success){
		printf("Test failed 3, result is %lf, and expected +INF\n", s);
	}

	s = rec_get_surface(r2);
	success = success & (s == DBL_MAX);
	if(!success){
		printf("Test failed 4, result is %lf, and expected +INF\n", s);
	}

    rec_free(r1);
    rec_free(r2);
    set_coords2(c1, -7, 39, 37, -1);
    if(rec_alloc(&r1, 2, (const COORD_TYPE*) c1) < 0){
        printf("Error allocating");
        exit(1);
    }


    s = rec_get_surface(r1);

    success = success & (s == 62*12);
    if(!success){
        printf("Test failed 3, result is %lf, and expected %lf\n", s, 62*12.);
    }
    

	return success;
}


int test_list(){
    int success=1;
    List* l;
    list_alloc(&l);
    int i=1;
    int i2=2;
    int i3=3;
    list_append(l, (void *) &i);
    list_append(l, (void *) &i2);
    list_append(l, (void *) &i3);

    ListNode* ln = l->first;
    while(ln != NULL){
        printf("%d->", *((unsigned*) ln->data));
        ln = ln->next;
    }
    printf("END\n");

    unsigned* popped;
    popped = (unsigned*) list_pop(l);
    printf("%d\n", *popped);
    popped = (unsigned*) list_pop(l);
    printf("%d\n", *popped);
    popped = (unsigned*) list_pop(l);
    printf("%d\n", *popped);
    popped = (unsigned*) list_pop(l);
    success = (popped == NULL);
    return success;
}



int test_adjust_to_fit(){
    Rectangle* r1, *r2;
    COORD_TYPE c1[2];
    COORD_TYPE c2[2];
    set_coords2(c1, 1, 2, -2, 2);
    set_coords2(c2, -1, 0, 1, 5);

    if(rec_alloc(&r1, 2, (const COORD_TYPE*) c1) < 0){
        printf("Error allocating");
        exit(1);
    }
    if(rec_alloc(&r2, 2, (const COORD_TYPE*) c2) < 0){
        printf("Error allocating");
        exit(1);
    }

    rec_adjust_to_fit(r1, (const Rectangle *) r2);

    rec_print(r1);

    return 1;
}


int test_overlap(){
	int success = 1;
	int ov;
	Rectangle* r1, *r2;
	COORD_TYPE c1[2];
	COORD_TYPE c2[2];
	set_coords2(c1, 1, 2, 1, 2);
	set_coords2(c2, 1.5, 3, 1, 4);

	if(rec_alloc(&r1, 2, (const COORD_TYPE*) c1) < 0){
		printf("Error allocating");
		exit(1);
	}
	if(rec_alloc(&r2, 2, (const COORD_TYPE*) c2) < 0){
		printf("Error allocating");
		exit(1);
	}

	ov = rec_overlap(r1, r2);

	success = success & (ov == 1);
	if(!success){
		printf("Test failed, result is %d, and expected 1\n", ov);
	}
	rec_free(r1);
	rec_free(r2);

	set_coords2(c1, 1, 2, 1, 2);
	set_coords2(c2, 3, 5, -10, -8);

	if(rec_alloc(&r1, 2, (const COORD_TYPE*) c1) < 0){
		printf("Error allocating");
		exit(1);
	}
	if(rec_alloc(&r2, 2, (const COORD_TYPE*) c2) < 0){
		printf("Error allocating");
		exit(1);
	}

	ov = rec_overlap(r1, r2);

	success = success & (ov == 0);
	if(!success){
		printf("Test failed, result is %d, and expected 0\n", ov);
	}


	rec_free(r1);
	rec_free(r2);

	set_coords2(c1, 1, 2, 1, 2);
	set_coords2(c2, -DBL_MAX, DBL_MAX, -DBL_MAX, DBL_MAX);

	if(rec_alloc(&r1, 2, (const COORD_TYPE*) c1) < 0){
		printf("Error allocating");
		exit(1);
	}
	if(rec_alloc(&r2, 2, (const COORD_TYPE*) c2) < 0){
		printf("Error allocating");
		exit(1);
	}

	ov = rec_overlap(r1, r2);

	success = success & (ov == 1);
	if(!success){
		printf("Test failed, result is %d, and expected 1\n", ov);
	}
	rec_free(r1);
	rec_free(r2);

	return success;

}


double dbmax(){
	return DBL_MAX;
}


/*
    TODO: add rtree_free() when implemented
    Blatant memory leak but whatever for now
*/
int test_insert(){
    int success = 1;
    COORD_TYPE c[2];
    
    set_coords2(c, 1, 2, 1, 2);
    Rtree* t;
    rtree_alloc(&t, 1, 2, 2);
    
    int j=0;
    for(j=0;j<2;j++){
        Rectangle* r;
        Entry* e;
        e = (Entry *) malloc(sizeof(Entry));
        e->node = NULL;
        if(rec_alloc(&r, 2, (const COORD_TYPE*) c) < 0){
            printf("Error allocating");
            exit(1);
        }
        e->r = r;
        e->id = j;
        int i = insert(t, e);
        Node* n = choose_leaf(t, NULL, e->r);
        printf("%d\n", i);
        success = success && ((n == t->root) && (i == 1));
    }


    Rectangle* r;
    Entry* e;
    e = (Entry *) malloc(sizeof(Entry));
    e->node = NULL;
    if(rec_alloc(&r, 2, (const COORD_TYPE*) c) < 0){
        printf("Error allocating");
        exit(1);
    }
    e->r = r;
    e->id = j;
    int i = insert(t, e);
    printf("%d\n", i);

    Node* n = choose_leaf(t, NULL, e->r);
    success = success && ((n != t->root) && (i == 0));
    
    rtree_print(t);

    return success;
}


int test_insert2(){
    int N = 15;
    COORD_TYPE c[2];
    Rtree* t;
    rtree_alloc(&t, 1, 2, 2);
    srand(time(NULL));
    int j=0;
    for(j=0;j<N;j++){
        int i11, i12, i21, i22;
        i11 = rand() % 100-50;
        i12 = rand() % 100-50;
        i21 = rand() % 100-50;
        i22 = rand() % 100-50;
        set_coords2(c, i11, i12, i21, i22);
        Rectangle* r;
        Entry* e;
        e = (Entry *) malloc(sizeof(Entry));
        e->node = NULL;
        if(rec_alloc(&r, 2, (const COORD_TYPE*) c) < 0){
            printf("Error allocating");
            exit(1);
        }
        printf("Rectangle: %p\n", r);
        printf("%d %d %d %d\n", i11, i12, i21, i22);
        double s = rec_get_surface(r);
        double ss = absv((i11 - i12)*(i22-i21));
        assert(s == ss);
        
        e->r = r;
        e->id = j;
        int i = insert(t, e);
        //rtree_print(t);
    }
    rtree_print(t);
    rtree_free(t);
    return 1;
}


int test_insert3(){
    COORD_TYPE c[2];
    Rtree* t;
    rtree_alloc(&t, 1, 2, 2);
    Rectangle* r;
    Entry* e;

    // First rectangle
    set_coords2(c, -3, -1, -2, 0);
    e = (Entry *) malloc(sizeof(Entry));
    e->node = NULL;
    if(rec_alloc(&r, 2, (const COORD_TYPE*) c) < 0){
        printf("Error allocating");
        exit(1);
    }
    e->r = r;
    e->id = 0;
    insert(t, e);


    // Second rectangle
    set_coords2(c, 1, 2, -2, 2);
    e = (Entry *) malloc(sizeof(Entry));
    e->node = NULL;
    if(rec_alloc(&r, 2, (const COORD_TYPE*) c) < 0){
        printf("Error allocating");
        exit(1);
    }
    e->r = r;
    e->id = 1;
    insert(t, e);
    rtree_print(t);

    // Third rectangle
    set_coords2(c, -2, 1, -1, 0);
    e = (Entry *) malloc(sizeof(Entry));
    e->node = NULL;
    if(rec_alloc(&r, 2, (const COORD_TYPE*) c) < 0){
        printf("Error allocating");
        exit(1);
    }
    e->r = r;
    e->id = 2;
    insert(t, e);
    rtree_print(t);


    // Fourth rectangle
    set_coords2(c, 1, 2, 1, 0);
    e = (Entry *) malloc(sizeof(Entry));
    e->node = NULL;
    if(rec_alloc(&r, 2, (const COORD_TYPE*) c) < 0){
        printf("Error allocating");
        exit(1);
    }
    e->r = r;
    e->id = 3;
    insert(t, e);
    rtree_print(t);


    // Fifth rectangle
    set_coords2(c, -2, 0, 1, 2);
    e = (Entry *) malloc(sizeof(Entry));
    e->node = NULL;
    if(rec_alloc(&r, 2, (const COORD_TYPE*) c) < 0){
        printf("Error allocating");
        exit(1);
    }
    e->r = r;
    e->id = 4;
    insert(t, e);
    rtree_print(t);


    // Sixth rectangle
    set_coords2(c, -2, 1, 2, 3);
    e = (Entry *) malloc(sizeof(Entry));
    e->node = NULL;
    if(rec_alloc(&r, 2, (const COORD_TYPE*) c) < 0){
        printf("Error allocating");
        exit(1);
    }
    e->r = r;
    e->id = 5;
    insert(t, e);
    printf("***6***");
    rtree_print(t);


    // Seventh rectangle
    set_coords2(c, -1, 0, 1, 5);
    e = (Entry *) malloc(sizeof(Entry));
    e->node = NULL;
    if(rec_alloc(&r, 2, (const COORD_TYPE*) c) < 0){
        printf("Error allocating");
        exit(1);
    }
    e->r = r;
    e->id = 6;
    insert(t, e);
    printf("***7***");
    rtree_print(t);
    test_search(t);
    rtree_free(t);
    return 1;

}




int test_search(Rtree* t){
    QuerySet q;
    list_alloc(&(q.values));
    q.size = 0;

    Rectangle* r;
    COORD_TYPE c[2];
    set_coords2(c, 1, 2, -2, 2);
    if(rec_alloc(&r, 2, (const COORD_TYPE*) c) < 0){
        printf("Error allocating");
        exit(1);
    }
    rec_print(r);
    printf("Testing search\n");
    search(t, NULL, r, &q);
    List* l = q.values;
    unsigned* datap;
    printf("Queryset size is %d, it contains: ", q.size);
    while((datap = list_pop(l)) != NULL){
        printf("%d ", *datap);
    }
    printf("\n");

    rec_free(r);
    list_free(q.values);
    return q.size;
}


int main(){

	//printf("Test operations: %d\n", test_operations());
	//printf("Test surface: %d\n", test_surface());
	//printf("Test overlap: %d\n", test_overlap());
    //test_adjust_to_fit();
    //printf("Test insert: %d\n", test_insert());
    //printf("Test list: %d\n", test_list());

    //test_insert2();
    test_insert3();

	return 0;
}