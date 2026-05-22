#include "functions.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    char* command = argv[1];
    if (argc < 2) {
        printf("Not enough arguments\n");
        printf("Run './mygit --help to get the instruction'\n");
        return 1;
    }

    if (strcmp(command, "--help") == 0 || strcmp(command, "help") == 0){
        help();
        return 0;
    }
    
    repository* R = NULL;
    if (strcmp(command, "init") == 0) {
        R = init(".");
        if (!R){
            printf("Failed to initialize repository\n");
            return 1;
        }
        printf("Successfully initialized\n");
        free_repository(R);
        return 0;
    }
    R = load_repository(".");
    // если пользователь сначала вызывает add без init, программа выведет ошибку
    if (!R) {
    printf("Run './mygit init' first\n");
    return 1;
}
    // выполняем команду
    if (strcmp(command, "add") == 0) {
        if (argc < 3) {
            printf("Not enough arguments\n");
            printf("Run './mygit --help to get the instruction'\n");
        } else {
            func_add(R, argv[2]);
        }
    } 
    else if (strcmp(command, "remove") == 0) {
        if (argc < 3) {
            printf("Not enough arguments\n");
            printf("Run './mygit --help to get the instruction'\n");
        } else {
            func_remove(R, argv[2]);
        }
    } 
    else if (strcmp(command, "commit") == 0) {
        if (argc < 3) {
            printf("Not enough arguments\n");
            printf("Run './mygit --help to get the instruction'\n");
        } else {
            func_commit(R, argv[2]);
        }
    } 
    else if (strcmp(command, "log") == 0) {
        func_log(R, argc, argv);
    } 
    else if (strcmp(command, "diff") == 0) {
        if (argc < 3) {
            printf("Not enough arguments\n");
            printf("Run './mygit --help to get the instruction'\n");
        } else {
            func_diff(R, argv[2]);
        }
    } 
    else if (strcmp(command, "status") == 0) {
        func_status(R);
    } 
    else if (strcmp(command, "checkout") == 0) {
        if (argc < 4) {
            printf("Not enough arguments\n");
            printf("Run './mygit --help to get the instruction'\n");
        } else {
            func_checkout(R, argv[2], argv[3]);
        }
    } 
    else {
        printf("Unknown command: %s\n", command);
        printf("Run './mygit --help to get the instruction'\n");
    }
    save_repository(R);
    free_repository(R);
    return 0;
}