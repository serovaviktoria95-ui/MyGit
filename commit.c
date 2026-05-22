#include "commit.h"
#include "repository.h"
#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <sys/stat.h>
// SHA-1 кодирование 
void generate_hash(commit* commit){
    char buf[2048];
    snprintf(buf, sizeof(buf), "%ld%s%s", commit->creation_time, commit->message,\
     commit->parent ? commit->parent->hash : "");
    unsigned char hash[SHA_DIGEST_LENGTH]; // 20 байт для SHA-1
    // вызываем функцию OpenSSL для вычисления SHA-1 хэша
    SHA1((unsigned char*)buf, strlen(buf), hash); 
    // unsigned char*, strlen(buf) - содержимое буфера + длина строки
    // hash - массив для хэша
    // преобразовываем 20-байтный бинарный хэш в 40-символьную строку
    for ( int i = 0; i < SHA_DIGEST_LENGTH; i++){
        snprintf(commit->hash + i * 2, 3, "%02x", hash[i]);
    }
    commit->hash[40] = '\0';
}
// функция чтения файлов
char* read_file(char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);

    string[fsize] = 0;
    return string;
}

commit* create_commit(repository* R, char* message, char** files, int cnt){
    commit* new_commit = malloc(sizeof(commit));
    new_commit->parent = R->head; // родителем будет текущий коммит
    new_commit->creation_time = time(NULL); // текущее время
    new_commit->message = strdup(message);
    if (files != NULL && cnt > 0) {
        new_commit->files = (char**)malloc(sizeof(char*) * cnt);
        new_commit->files_cnt = cnt;
        for (int i = 0; i < cnt; i++) {
            new_commit->files[i] = strdup(files[i]);
        }
    } else {
        new_commit->files = NULL;
        new_commit->files_cnt = 0;
    } 
    generate_hash(new_commit);
    R->head = new_commit;
    printf("Commit created: %s\n", new_commit->hash);
    // хранение содержимого файлов
    for (int i = 0; i < cnt; i++) {
        char* content = read_file(files[i]);
        char obj_path[512];
        snprintf(obj_path, sizeof(obj_path), ".mygit/objects/%s_%s", \
        new_commit->hash, files[i]);
        
        FILE* output = fopen(obj_path, "w");
        fprintf(output, "%s", content);
        fclose(output);
        free(content);
    }
    save_commit(new_commit);
    return new_commit;
}
// сохранение коммита на диск
void save_commit(commit* c){
    if (!c) return;
    char path[256];
    snprintf(path, sizeof(path), ".mygit/objects/%s", c->hash);
    FILE* output = fopen(path, "w");

    // сохраняем метаданные
    fprintf(output, "hash: %s\n", c->hash);
    fprintf(output, "time: %ld\n", (long)c->creation_time);
    fprintf(output, "message: %s\n", c->message ? c->message : "");
    if (c->parent) {
        fprintf(output, "parent: %s\n", c->parent->hash);
    } else if (strlen(c->parent_hash) > 0) {
        fprintf(output, "parent: %s\n", c->parent_hash);
    } else {
        fprintf(output, "parent: \n");
    }
    fprintf(output, "files_cnt: %d\n", c->files_cnt);
    for (int i = 0; i < c->files_cnt; i++) {
        fprintf(output, "file: %s\n", c->files[i]);
    }
    fclose(output);    
}
commit* load_commit(char* hash){
    if (!hash) return NULL;
    char path[256];
    snprintf(path, sizeof(path), ".mygit/objects/%s", hash);
    FILE* input = fopen(path, "r");
    commit* c = calloc(1, sizeof(commit));
    char str[1024];
    c->files = NULL;
    c->files_cnt = 0;
    int file_idx = 0;
    
    strcpy(c->hash, hash);
    c->parent = NULL; // родитель загружается лениво
    c->parent_hash[0] = '\0';

    while(fgets(str, sizeof(str), input)){
        str[strcspn(str, "\n")] = '\0';
        if (strncmp(str, "time:", 5) == 0){
            c->creation_time = (time_t)atol(str + 6);
        } else if (strncmp(str, "message:", 8) == 0){
            c->message = strdup(str + 9);
        } else if (strncmp(str, "parent:", 7) == 0) {
            strcpy(c->parent_hash, str + 8);
        } else if (strncmp(str, "files_cnt:", 10) == 0) {
            c->files_cnt = atoi(str + 11);
            if (c->files_cnt > 0){
                c->files = malloc(sizeof(char*) * c->files_cnt);
            }
        } else if (strncmp(str, "file:", 5) == 0 && c->files && file_idx < c->files_cnt){
            c->files[file_idx++] = strdup(str + 6);
        }
        //+1 из-за пробела
    }
    fclose(input);
    return c;
}

// функция для ленивой загрузки родителя
commit* load_parent(commit* c) {
    if (!c || c->parent != NULL) return c->parent;
    if (strlen(c->parent_hash) == 0) return NULL;
    
    c->parent = load_commit(c->parent_hash);
    return c->parent;
}

void free_commit(commit* c){
    if (!c) return;
    if (c->message) free(c->message);
    if (c->files){
        for ( int i = 0; i < c->files_cnt; i++){
            if (c->files[i]) free(c->files[i]);
        }
        free(c->files);
    }
    free(c);
}