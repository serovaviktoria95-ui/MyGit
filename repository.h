#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "commit.h"
#include "staging_area.h"
// структура репозитория
typedef struct repository {
    char* path; // путь к репозиторию
    commit* head; // указатель на текущий коммит
    staging_area* staging; // информация о том, что должно войти в след. коммит
} repository;
// сохранение репозитория на диск
void save_repository(repository* R);
// загрузка репозитория
repository* load_repository(char* path);
// освобождение памяти репозитория
void free_repository(repository* R);
// создание директории
void create_directory();
// проверка наличия репозитория
int check_existence(char* path);
// функция инициализации репозитория
repository* init(char* path);