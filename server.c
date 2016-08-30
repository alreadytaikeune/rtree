#include <onion/onion.h>
#include <onion/request.h>
#include <onion/response.h>
#include <onion/handler.h>
#include <onion/log.h> // ONION_INFO

#include <string.h>
#include <signal.h> // SIGINT, SIGTERM
#include <stdio.h> // sscanf
#include <assert.h>


#include "geometry.h"
#include "rtree.h"
#include "list.h"
#include "utils.h"

Rtree *t;
onion *o=NULL;


typedef struct RtreeRequestData {
    Rectangle* r;
    double zoom;
} RtreeRequestData;


inline int set_coords2(COORD_TYPE* c, double a1, double a2, double a3, double a4){
    c[0][0] = a1;
    c[0][1] = a2;
    c[1][0] = a3;
    c[1][1] = a4;
}


void init_rtree(){
    int N = 15;
    COORD_TYPE c[2];
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
        rtree_print(t);
    }
    rtree_print(t);
}


int rtree_request_handler(RtreeRequestData *d, onion_request *request, 
    onion_response *response){

    if (onion_request_get_flags(request)&OR_POST){
        if(d == NULL){
            d = (RtreeRequestData*) malloc(sizeof(RtreeRequestData));
        }
        const char * coords = onion_request_get_post(request,"r");

        printf("%s \n", coords);

        COORD_TYPE c[2];
        sscanf(onion_request_get_post(request,"r"), "%lf,%lf,%lf,%lf", &(c[0][0]), 
            &(c[0][1]), &(c[1][0]), &(c[1][1]));
        
        double zoom;
        sscanf(onion_request_get_post(request,"z"), "%lf", &zoom);

        rec_alloc(&(d->r), 2, (const COORD_TYPE*) c);
        rec_print(d->r);
    
        QuerySet q;
        list_alloc(&(q.values));
        q.size = 0;

        search(t, NULL, d->r, &q);
        printf("Queryset size is %d \n", q.size);
        list_free(q.values);

    }
    return OCS_PROCESSED;

}

void rtree_request_free(RtreeRequestData *d){
    if(d != NULL){
        rec_free(d->r);
        free(d);
    }
}


void onexit(int _){
    ONION_INFO("Exit");
    if (o)
        onion_listen_stop(o);
}


int main(int argc, char **argv){
    rtree_alloc(&t, 2, 4, 2);
    init_rtree();
    signal(SIGTERM, onexit);    
    signal(SIGINT, onexit);
    o=onion_new(O_THREADED);
    onion_set_port(o, "8080");
    onion_set_hostname(o, "0.0.0.0");
    onion_url *url=onion_root_url(o);
    onion_url_add(url, "tree", rtree_request_handler);
    onion_url_add_static(url, "version", "0.0.1", 200);
    onion_listen(o);
    onion_free(o);
    return 0;
}