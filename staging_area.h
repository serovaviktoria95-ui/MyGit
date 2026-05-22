#pragma once
#include <string.h>
typedef struct staging_area{
    char** files;
    char** contain;
    int cnt;
} staging_area;
// создание staging area
staging_area* create_SA();
// очистка памяти
void free_SA(staging_area* SA);
// добавление или удаление в случае NULL
void add_SA(staging_area* SA, char* filename, char* contain);
// поиск заданного файла в SA
int search_SA(staging_area* SA, char* filename);