#include "staging_area.h"
#include <stdlib.h>
staging_area* create_SA(){
    staging_area* SA = malloc(sizeof(staging_area));
    SA->files = NULL;
    SA->cnt = 0;
    SA->contain = NULL;
    return SA;
}

void free_SA(staging_area* SA){
    if (!SA) return;
    for ( int i = 0; i < SA->cnt; i++){
        free(SA->files[i]);
        if (SA->contain[i]) free(SA->contain[i]);
    }
    free(SA->files);
    free(SA->contain);
    SA->files = NULL;
    SA->contain = NULL;
    SA->cnt = 0;
}

int search_SA(staging_area* SA, char* filename){
    if (!SA) return -1;
    for (int i = 0; i < SA->cnt; i++) {
        if (strcmp(SA->files[i], filename) == 0) return i;
    }
    return -1;
}

void add_SA(staging_area* SA, char* filename, char* contain){
    int idx = search_SA(SA, filename);
    if (idx != -1) {
        // перезаписываем существующий
        if (SA->contain[idx]) free(SA->contain[idx]);
        SA->contain[idx] = contain ? strdup(contain) : NULL;
        // filename не меняем, он тот же
    } else {
        // добавляем новый
        SA->files = realloc(SA->files, sizeof(char*) * (SA->cnt + 1));
        SA->contain = realloc(SA->contain, sizeof(char*) * (SA->cnt + 1));
        SA->files[SA->cnt] = strdup(filename);
        SA->contain[SA->cnt] = contain ? strdup(contain) : NULL;
        SA->cnt++;
    }
}