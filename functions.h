#pragma once
#include "repository.h"
// список команд для пользователя
void help();
// вычилсяет хэш для содержимого файлов
void get_file_hash(char* content, char* file_hash);
// добавляет указанный файл в список файлов, которые должны попасть в следующий коммит
void func_add(repository* R, char* filename);
// добавляет указанный файл в список файлов, которые должны попасть в следующий коммит с пометкой на удаление
void func_remove(repository* R, char* filename);
// фиксирует состояние файлов, добавленных командой add и remove, создавая коммит
void func_commit(repository* R, char* message);
// вывод цепочки до начального коммита
void func_log(repository* R, int argc, char** argv);
// сравнивает указанный коммит с текущим и выводит измененные файлы
void func_diff(repository* R, char* commit_hash);
// выводит список файлов, которые попадают в следующий коммит
void func_status(repository* R);
// берет указанный файл из указанного коммита и заменяет в текущей директории
void func_checkout(repository* R, char* commit_hash, char* filename);