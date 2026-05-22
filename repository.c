#include "repository.h"
#include "commit.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h> // для работы с директориями
void create_directory() {
    system("mkdir -p .mygit/objects");
}

int check_existence(char* path){
    char full_path[256];
    snprintf(full_path, sizeof(full_path), "%s/.mygit", path);
    struct stat st;
    return stat(full_path, &st) == 0;
}

// init для репозитория (только в памяти)
repository* init(char* path) {
    // проверяем, существует ли такой репозиторий 
    if (check_existence(path) == 1){
        printf("Repository already exists\n");
        return NULL;
    }
    create_directory();
    repository* R = malloc(sizeof(repository));    
    R->path = strdup(path);
    R->head = NULL;
    R->staging = create_SA();
    
    char* message = "Initial commit";

    commit* initial = create_commit(R, message, NULL, 0);
    R->head = initial;
    // сохраняем новый репозиторий на диск
    save_repository(R);
    return R;
}

// сохранение репозитория на диск
void save_repository(repository* R) {
    if (!R) return;
    // сохраняем HEAD - файл с хэшом текущего коммита
    FILE* output = fopen(".mygit/HEAD", "w");
    if (output && R->head) {
        fprintf(output, "%s\n", R->head->hash);
        fclose(output);
    }    
    // сохраняем staging area
    output = fopen(".mygit/staging", "w");
    if (output && R->staging) {
        for ( int i = 0; i < R->staging->cnt; i++){
            char status = R->staging->contain[i] ? 'A' : 'R';
            fprintf(output, "%c %s\n", status, R->staging->files[i]);
        }
        fclose(output);
    }
}
// загрузка репозитория
repository* load_repository(char* path) {
    if (!check_existence(path)) {
        return NULL;  // репозиторий не существует
    }
    repository* R = malloc(sizeof(repository));    
    R->path = strdup(".");
    R->head = NULL;
    R->staging = create_SA();
    
    char buf[256];
    
    // загружаем HEAD
    FILE* input = fopen(".mygit/HEAD", "r");
    if (input != NULL) {
        if (fgets(buf, sizeof(buf), input)) {
            buf[strcspn(buf, "\n")] = '\0';
            R->head = load_commit(buf);
        }
        fclose(input);
    }
    
    // загружаем staging area
    input = fopen(".mygit/staging", "r");
    if (input) {
        char status; 
        char filename[256]; 
        while (fscanf(input, " %c %255s", &status, filename) == 2) {
            if (status == 'A') { 
                char* content = read_file(filename); 
                add_SA(R->staging, filename, content); 
                if (content) free(content); 
            } else if (status == 'R') {
                add_SA(R->staging, filename, NULL);
            }
        }
        fclose(input); 
    }
    return R;
}

// освобождение памяти репозитория
void free_repository(repository* R) {
    if (!R) return;
    if (R->staging) free_SA(R->staging);
    if (R->path) free(R->path);
    free(R);
}