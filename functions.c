#include "repository.h"
#include "commit.h"
#include "staging_area.h"
#include <sys/stat.h>
#include <openssl/sha.h>
#include <dirent.h>  

// список команд для пользователя
void help(){
    printf("Available commands:\n");

    printf("init: \ninitializes an empty repository in the directory and creates an empty initial commit\n");

    printf("add file_name: \nadds the file to the list of files that should be included in the next commit\n");
    printf("\n");

    printf("remove file_name: \nadds the to the list of files that should be included in the nextcommit, with a note that the it has been deleted\n");
    printf("\n");

    printf("commit message: \ncommits the state of files added by the add and remove commands, creating a commit\n");
    printf("\n");

    printf("log commit --n num: \nchain output before the first commit\n");
    printf("\n");

    printf("diff commit: \ncompares the specified commit with the current one and shows the modified files\n");
    printf("\n");

    printf("status: \nshows a list of files that will be included in the next commit\n");
    printf("\n");
    
    printf("checkout commit file_name: \ntakes the file from the specified commit and replaces it in the current directory\n");
}

void get_file_hash(char* content, char* file_hash){
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1((unsigned char*)content, strlen(content), hash);
    
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        snprintf(file_hash + i * 2, 3, "%02x", hash[i]);
    }
    file_hash[40] = '\0';
}

void func_add(repository* R, char* filename){
    struct stat st;
    if(stat(filename, &st) == -1){
        printf("File %s was not found\n", filename); 
        return;
    }
    if (!S_ISREG(st.st_mode)){
        printf("Error: %s is not a regular file (directory or special file)\n", filename);
        return;
    }
    
    char* contain = read_file(filename);
    if (!contain) {
        printf("Error: Could not read file %s\n", filename);
        return;
    }
    
    char file_hash[41];
    get_file_hash(contain, file_hash);
    add_SA(R->staging, filename, contain);  
    printf("Successfully added: %s (hash: %s)\n", filename, file_hash);
    free(contain);
}

// добавляет указанный файл в список файлов, которые должны попасть в следующий
// коммит с пометкой, что указанный файл удален.
void func_remove(repository* R, char* filename) {
    int found_in_staged = search_SA(R->staging, filename);
    int found_in_head = 0;

    if (R->head) {
        for (int i = 0; i < R->head->files_cnt; i++) {
            if (strcmp(R->head->files[i], filename) == 0) {
                found_in_head = 1;
                break;
            }
        }
    }

    if (found_in_staged != -1) {
        if (R->staging->contain[found_in_staged] != NULL) {
             add_SA(R->staging, filename, NULL); 
             printf("Successfully removed: %s\n", filename);
        } else {
             printf("File '%s' is already marked for removal\n", filename);
        }
    } 
    else if (found_in_head) {
        add_SA(R->staging, filename, NULL);
        printf("Successfully removed: %s\n", filename);
    } 
    else {
        printf("File '%s' was not found\n", filename);
    }
}

// фиксирует состояние файлов, добавленных командой add и remove, 
// создавая коммит
void func_commit(repository* R, char* message){
    if (R->staging->cnt == 0){
        printf("Nothing to commit\n");
        return;
    }
    int cnt = 0;
    for (int i = 0; i < R->staging->cnt; i++){
        if (R->staging->contain[i] != NULL) cnt++;
    }
    
    if (cnt == 0) {
        printf("No files to commit (all marked for deletion)\n");
        return;
    }
    
    char** files_to_commit = malloc(sizeof(char*) * cnt);
    int idx = 0;
    for (int i = 0; i < R->staging->cnt; i++){
        if (R->staging->contain[i] != NULL){  
            files_to_commit[idx++] = R->staging->files[i];
        }
    }

    commit* new_commit = create_commit(R, message, files_to_commit, cnt);
    // очищаем staging area
    free_SA(R->staging);
    // создаем пустую новую 
    R->staging = create_SA();
    printf("Successfully committed: %s\n", new_commit->hash);
    free(files_to_commit);
}
// вывод цепочки до начального коммита
void func_log(repository* R, int argc, char** argv) {
    if (!R || !R->head) {
        printf("No commits yet\n");
        return;
    }
    
    commit* cur = R->head;
    char* from = NULL;
    int max = -1;

    // парсим аргументы
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--n") == 0 && i + 1 < argc) {
            max = atoi(argv[i + 1]);
            i++;
        } else if (strlen(argv[i]) == 40) {
            from = argv[i];
        } else if (atoi(argv[i]) > 0){
            max = atoi(argv[i]);
        }
    }
    if (from) {
        // загружаем указанный коммит
        commit* temp = load_commit(from);
        if (!temp) {
            printf("Commit %s was not found\n", from);
            return;
        }
        cur = temp;
    }
    int cnt = 0;
    while (cur && (max == -1 || cnt < max)) {
        printf("Commit: %s\n", cur->hash);
        printf("Time: %ld\n", (long)cur->creation_time);
        printf("Message: %s\n", cur->message);
        printf("Files: %d\n", cur->files_cnt);
        commit* next = load_parent(cur);
        cur = next;
        cnt++;
    }
}
// сравнивает указанный коммит с текущим + вывод измененных файлов
void func_diff(repository* R, char* commit_hash){
    if (strcmp(commit_hash, R->head->hash) == 0){
        printf("Can't compare commit with itself\n");
        return;
    }
    
    commit* target = load_commit(commit_hash);
    if (!target){
        printf("Commit %s was not found\n", commit_hash);
        return;
    } 
    
    commit* cur = R->head;
    
    // сначала проверяем modified файлы 
    for (int i = 0; i < cur->files_cnt; i++) {
        for (int j = 0; j < target->files_cnt; j++) {
            if (strcmp(cur->files[i], target->files[j]) == 0) {
                // файл есть в обоих коммитах - проверяем содержимое
                char cur_path[512], target_path[512];
                
                get_object_path(cur_path, sizeof(cur_path), cur->hash, cur->files[i]);
                get_object_path(target_path, sizeof(target_path), target->hash, target->files[j]);
                
                char* cur_content = read_file(cur_path);
                char* target_content = read_file(target_path);

                if (strcmp(cur_content, target_content) != 0) {
                    // содержимое различается - файл изменен
                    char cur_hash[41], target_hash[41];
                    get_file_hash(cur_content, cur_hash);
                    get_file_hash(target_content, target_hash);
                    
                    printf("Modified: %s (hash: %s -> %s)\n", \
                     cur->files[i], target_hash, cur_hash);
                }
                free(cur_content);
                free(target_content);
                break;
            }
        }
    }
    
    // проверяем added файлы (только в cur)
    for (int i = 0; i < cur->files_cnt; i++) {
        int flag_add = 0;
        for (int j = 0; j < target->files_cnt; j++) {
            if (strcmp(cur->files[i], target->files[j]) == 0) {
                flag_add = 1;
                break;
            }
        }
        if (!flag_add) {
            char cur_path[512];
            get_object_path(cur_path, sizeof(cur_path),cur->hash, cur->files[i]);
            char* content = read_file(cur_path);
            char file_hash[41];
            get_file_hash(content, file_hash);
            printf("Added: %s (hash: %s)\n", cur->files[i], file_hash);
            free(content);
        }
    }
    
    // проверяем removed файлы (только в target)
    for (int i = 0; i < target->files_cnt; i++) {
        int flag_remove = 0;
        for (int j = 0; j < cur->files_cnt; j++) {
            if (strcmp(target->files[i], cur->files[j]) == 0) {
                flag_remove = 1;
                break;
            }
        }
        if (!flag_remove) {
            char target_path[512];
            get_object_path(target_path, sizeof(target_path), target->hash, target->files[i]);
            char* content = read_file(target_path);
            char file_hash[41];
            get_file_hash(content, file_hash);
            printf("Removed: %s (hash: %s)\n", target->files[i], file_hash);
            free(content);
        }
    }
    free_commit(target);
}

// выводит список файлов, которые попадают в следующий коммит
void func_status(repository* R){
    if (!R) return;
    if (!R->staging || R->staging->cnt == 0) {
        printf("Nothing staged for commit\n");
        return;
    } 
    commit* parent = R->head;
    for ( int i = 0; i< R->staging->cnt; i++){
        char* filename = R->staging->files[i];
        char* staged_content = R->staging->contain[i];

        if (staged_content == NULL){
            printf("Removed: %s\n", filename);
            continue;
        }

        if (parent == NULL){
            printf("Added: %s\n", filename);
            continue;
        }

        int found_in_parent = 0;
        for (int j = 0; j < parent->files_cnt; j++) {
            if (strcmp(parent->files[j], filename) == 0) {
                found_in_parent = 1;
                break;
            }
        } 
        if (!found_in_parent){
            printf("Added: %s\n", filename);
        } else {
            char parent_path[512];
            get_object_path(parent_path, sizeof(parent_path), parent->hash, filename);
            char* parent_content = read_file(parent_path);
            char cur_hash[41], staged_hash[41];
            get_file_hash(parent_content, cur_hash);
            get_file_hash(staged_content, staged_hash);
            if (strcmp(staged_content, parent_content) != 0){
                printf("Modified: %s\n", filename);
            } else {
                printf("Unchanged: %s\n", filename);
            }
            free(parent_content);  
        }
    }
}

// берет указанный файл из указанного
// коммита и заменяет в текущей директории
void func_checkout(repository* R, char* commit_hash, char* filename){
    commit* from = load_commit(commit_hash);
    if (!from){
        printf("Commit %s was not found\n", commit_hash);
        return;
    }
    char object_path[512];
    get_object_path(object_path, sizeof(object_path), commit_hash, filename);

    FILE* input = fopen(object_path, "r");
    if (!input){
        printf("No such file in commit: %s\n", filename);
        free_commit(from);
        return;
    }
    // копируем содержимое в текущую директорию
    FILE* output = fopen(filename, "w");
    char ch;
    while ((ch = fgetc(input)) != EOF){
        fputc(ch, output);
    }
    
    fclose(input);
    fclose(output);
    
    printf("Successfully restored %s from commit %s\n", filename, commit_hash);
    free_commit(from);
}