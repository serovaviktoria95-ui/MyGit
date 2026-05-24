#pragma once
#include <time.h>
#include <stdlib.h>
typedef struct repository repository;
// структура коммита 
typedef struct commit {
    char hash[41]; // в Git длина хэша 40 шестнадцатеричных символов + '\0'
    time_t creation_time; // время создания
    char* message; // сообщение коммита
    struct commit* parent; // указатель на родительский коммит
    char parent_hash[41];
    char** files; // список файлов в коммите
    int files_cnt; // их количество
} commit;
// читает содержимое файла filename
char* read_file(char* filename);
// создание коммита
commit* create_commit(repository* R, char* message, char** files, int files_cnt); 
// сохранение коммита на диск
void save_commit(commit* c);
// загрузка коммита с диска
commit* load_commit(char* hash);
// SHA-1 кодирование
void generate_hash(commit* c);
// освобождение памяти коммита
void free_commit(commit* c);
// ленивая загрузка родителя, используем только для log
commit* load_parent(commit* c);
// формирует безопасный путь к объекту, заменяя '/' на '%'
void get_object_path(char* obj_path, size_t size, const char* commit_hash, const char* filename);