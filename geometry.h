#ifndef GEOMETRY_H
#define GEOMETRY_H
#include <stdlib.h>
#include <stddef.h>


typedef double COORD_TYPE[2];



struct Rectangle
{
	unsigned dim;
	COORD_TYPE* coords;
};

typedef struct Rectangle Rectangle;


int rec_alloc(Rectangle** r, unsigned dim, const COORD_TYPE* coords);

void rec_free(Rectangle* r);

int rec_overlap(Rectangle* r1, Rectangle* r2);

//int rec_equals(Rectangle* r1, Rectangle * r2);


/*
	Stores in r3 (already allocated) the smallest rectangle that contains both
	r1 and r2;
*/
void rec_get_encompassing(const Rectangle* r1, const Rectangle* r2, Rectangle* r3);


// Adjust r1 to encompass old r1 and r2
void rec_adjust_to_fit(Rectangle* r1, const Rectangle* r2);

void rec_print(Rectangle *r);

double rec_get_surface(Rectangle* r);

#endif 