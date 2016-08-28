#ifndef UTILS_H
#define UTILS_H
#include <float.h>

static inline double max(double a1, double a2){
	return a1 >= a2 ? a1 : a2;
}

static inline double min(double a1, double a2){
	return a1 <= a2 ? a1 : a2;
}

static inline double absv(double a){
	return a >= 0 ? a : -a;
}

static inline swap(void* p1, void* p2){
	void* tmp = p1;
	p1 = p2;
	p2 = tmp;
}

static inline int is_limit(double a){
	return a == DBL_MAX || a == -DBL_MAX;
}

static inline int sgn(double a){
	return a >= 0 ? 1 : -1; 
}

/*
	Overflow safe operations.
*/
static inline double add(double a1, double a2){
	if(is_limit(a1) && is_limit(a2)){
		//printf("Both are limits %d, %d, %lf\n", sgn(a1), sgn(a2), sgn(a1)*DBL_MAX);
		//printf("%lf\n", sgn(a1) != sgn(a2) ? 0 : sgn(a1)*DBL_MAX);
        return (sgn(a1) != sgn(a2) ? 0 : sgn(a1)*DBL_MAX);
	}
	else if(is_limit(a1)){
		return sgn(a1)*DBL_MAX;
	}
	else if(is_limit(a2)){
		return sgn(a2)*DBL_MAX;
	}
	else{
		if(sgn(a1) == 1 && sgn(a2) == 1){
			if(a2 >= DBL_MAX - a1){
				return DBL_MAX;
            }
		}
		else if(sgn(a1) == -1 && sgn(a2) == -1){
			if(a2  <= -DBL_MAX - a1){
				return -DBL_MAX;
            }
		}
		return a1 + a2;
	}
}

static inline double sub(double a1, double a2){
	return add(a1, -a2);
}

static inline double mult(double a1, double a2){
	if(is_limit(a1) && is_limit(a2)){
		return sgn(a2)*sgn(a1)*DBL_MAX;
	}
	else if(is_limit(a1)){
		return sgn(a1)*DBL_MAX;
	}
	else if(is_limit(a2)){
		return sgn(a2)*DBL_MAX;
	}
	else{
		if(DBL_MAX / absv(a1) < absv(a2)){
			return sgn(a1)*sgn(a2)*DBL_MAX;
		}

		return a1*a2;
	}
}

#endif