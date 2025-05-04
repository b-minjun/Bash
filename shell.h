#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 128

// CommandType
typedef enum CommandType {
    CMD_NORMAL,
    CMD_PIPELINE,
    CMD_SEQUENCE,
    CMD_AND,
    CMD_OR
} CommandType;

// Command
typedef struct Command {
    CommandType type;
    struct Command *left;
    struct Command *right;
    char *argv[MAX_ARGS];
    int is_background;
} Command;

// 함수 선언

// parser
void tokenize(char *input, char *tokens[]);
int is_multi_command(char *tokens[]);
Command* parse_input(char *tokens[]);
Command* parse_command(int *pos, char *tokens[]);
Command* parse_pipeline(int *pos, char *tokens[]);
Command* parse_sequence(int *pos, char *tokens[]);

// executor
void execute_command(Command *cmd, char *current_path);
void pwd(int is_background);
int cd(char *input, char *current_path);
void ls(int is_background, int show_all, char *current_path);
void cat(int is_background, char *input);

// utils
void get_user_info(char *username, char *password, char *hostname, char *current_path);
void print_command_token(char *tokens[]);
void print_command_tree(Command *cmd, int depth);
void free_command(Command *cmd);
void free_tokens();

#endif