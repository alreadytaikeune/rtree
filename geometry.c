#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <assert.h>
#include "geometry.h"
#include "utils.h"


/*
	a + DBL_MAX = DBL_MAX
	a - DBL_MAX = -DBL_MAX
	a*DBL_MAX = DBL_MAX
	a/DBL_MAX = 0 // should never happen though
	DBL_MAX - DBL_MAX ... for our intents and purposes 
	I think we can equate this to 0.
*/

int rec_alloc(Rectangle** rp, unsigned dim, const COORD_TYPE* coords){
	*rp = (Rectangle*) malloc(sizeof(Rectangle));
	Rectangle* r = *rp;
	if(r == NULL){
		return -1;
	}
	r->dim = dim;
	r->coords = (COORD_TYPE*) malloc(sizeof(COORD_TYPE)*dim);

	if(r->coords == NULL){
		return -1;
	}
	if(coords != NULL){
		int i;
		for(i=0;i<dim;i++){
			memcpy(r->coords[i], coords[i], sizeof(COORD_TYPE));
		}
			
	}
	else{
		int i;
		for(i=0; i<dim; i++){
			(r->coords)[i][0] = -DBL_MAX;
			(r->coords)[i][1] = DBL_MAX;
		}
	}
	return 0;
}

int rec_overlap(Rectangle* r1, Rectangle* r2){
	if(r1 == NULL || r2 == NULL){
		printf("Warning: one rectangle is null\n");
		return 0;
	}

	if(r1->dim != r2->dim){
		printf("Incompatible dimensions\n");
		return 0;
	}
		
	int i;
	for(i=0; i<r1->dim;i++){
		double* l1 = *(r1->coords+i);
		double* h1 = l1;
		double* l2 = *(r2->coords+i);
		double* h2 = l2;

		// sets l1 to the lowest value of the interval in this dimension
		((r1->coords)[i][0] > (r1->coords)[i][1]) ? l1++ : h1++;
		// sets l2 to the lowest value of the interval in this dimension
		((r2->coords)[i][0] > (r2->coords)[i][1]) ? l2++ : h2++;

		int j = ((r1->coords)[i][0] > (r1->coords)[i][1]);
		//printf("%d\n", j);
		//printf("%p %p\n", l1, h1);
		j = ((r2->coords)[i][0] > (r2->coords)[i][1]);
		//printf("%d\n", j);
		//printf("%p %p\n", l2, h2);

		//printf("%lf, %lf\n", *l1, *h1);
		//printf("%lf, %lf\n", *l2, *h2);
		if((*h1 < *l2) || (*l1 > *h2)){
			return 0;
		}
	}
	return 1;
}


void rec_get_encompassing(const Rectangle* r1, const Rectangle* r2, Rectangle* r3){
    if(r1 == NULL || r2 == NULL || r3 == NULL){
		printf("null Rectangle argument for rec_get_encompassing\n");
		exit(1);
	}
	if(! (r1->dim == r2->dim && r1->dim == r3->dim)){
		printf("dimension mismatch for rec_get_encompassing\n");
		exit(1);
	}
	
	int i;
	for(i=0; i<r1->dim;i++){
		double* l1 = *(r1->coords+i);
		double* h1 = l1;
		double* l2 = *(r2->coords+i);
		double* h2 = l2;

		// sets l1 to the lowest value of the interval in this dimension
		((r1->coords)[i][0] > (r1->coords)[i][1]) ? l1++ : h1++;
		// sets l2 to the lowest value of the interval in this dimension
		((r2->coords)[i][0] > (r2->coords)[i][1]) ? l2++ : h2++;

		(r3->coords)[i][0] = min(*l1, *l2);
		(r3->coords)[i][1] = max(*h1, *h2);
	}
	r3->dim = r1->dim;
}


void rec_adjust_to_fit(Rectangle* r1, const Rectangle* r2){
    assert(r1 != NULL);
    assert(r2 != NULL);
    assert(r1->dim == r2->dim);

    printf("Adjusting %p to %p\n", r1, r2);
    rec_print(r1);
    rec_print((Rectangle *) r2);

    int i;
    for(i=0; i<r1->dim;i++){
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
    rec_print(r1);
    printf("\n");
}


void rec_print(Rectangle *r){
	int i;
	for(i=0;i<r->dim;i++){
		if((r->coords)[i][0] == DBL_MAX){
			printf("+INF");
		}
		else if((r->coords)[i][0] == -DBL_MAX){
			printf("-INF");
		}
		else{
			printf("%lf", (r->coords)[i][0]);
		}
		printf(", ");
		if((r->coords)[i][1] == DBL_MAX){
			printf("+INF");
		}
		else if((r->coords)[i][1] == -DBL_MAX){
			printf("-INF");
		}
		else{
			printf("%lf", (r->coords)[i][1]);
		}
		printf("\n");
	}
	
}


void rec_copy(Rectangle **rp, Rectangle* r){
    if(rec_alloc(rp, r->dim, (const COORD_TYPE*) r->coords) < 0){
        printf("Error allocating");
        exit(1);
    }
}

double rec_get_surface(Rectangle* r){
	double s = 1;
	int i = 0;
	for(i=0;i<r->dim;i++){
		double su = sub((r->coords)[i][1], (r->coords)[i][0]);
		s=mult(s, absv(su));
		if(is_limit(s))
			break;
	}
	return s;
}



void rec_free(Rectangle* r){
	int i;
	free(r->coords);
	free(r);
	r = NULL;
}