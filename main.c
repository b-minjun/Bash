#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 128

char username[MAX_INPUT_SIZE];
char password[MAX_INPUT_SIZE];
char hostname[MAX_INPUT_SIZE];
char input[MAX_INPUT_SIZE];
char current_path[MAX_INPUT_SIZE];
char *tokens[MAX_ARGS];

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
void tokenize(char *input);
void clear_tokens();
Command* parse_input();
Command* parse_command(int *pos);
Command* parse_pipeline(int *pos);
Command* parse_sequence(int *pos);
void print_command_tree(Command *cmd, int depth);
void get_user_info();
void pwd();
void cd(char *input);
void ls(int show_all);
void cat(char *input);
void free_command(Command *cmd);

// tokenize input
void tokenize(char *input) {
    int i = 0, j = 0;

    while (input[i] != '\0') {
        if (input[i] == ' ' || input[i] == '\t' || input[i] == '\n') {
            i++;
            continue;
        }

        if (strchr("|&;", input[i])) {
            char op[3] = {0};
            op[0] = input[i];
            if ((input[i] == '|' || input[i] == '&') && input[i + 1] == input[i]) {
                op[1] = input[i];
                i += 2;
            } else {
                i++;
            }
            tokens[j] = strdup(op);
            j++;
            continue;
        }

        int start = i;
        while (input[i] != '\0' &&
                !strchr(" \t\n|&;", input[i])) {
            i++;
        }
        int len = i - start;
        char *word = malloc(len + 1);
        strncpy(word, &input[start], len);
        word[len] = '\0';
        tokens[j++] = word;
    }
}

// clear tokens
void clear_tokens() {
    for (int i = 0; i < MAX_ARGS; i++) {
        if (tokens[i]) {
            free(tokens[i]);
            tokens[i] = NULL;
        }
    }
}

// parse input
Command* parse_input() {
    int pos = 0;
    Command *cmd = parse_sequence(&pos);
    return cmd;
}

// parse command
Command* parse_command(int *pos) {
    if (tokens[*pos] == NULL) return NULL;

    Command *cmd = malloc(sizeof(Command));
    cmd->type = CMD_NORMAL;
    cmd->left = cmd->right = NULL;
    cmd->is_background = 0;

    if (strcmp(tokens[*pos], "&") == 0) {
        cmd->is_background = 1;
        (*pos)++;
    }

    int i = 0;
    while (tokens[*pos] && strcmp(tokens[*pos], "|") != 0 &&
            strcmp(tokens[*pos], "&&") != 0 &&
            strcmp(tokens[*pos], "||") != 0 &&
            strcmp(tokens[*pos], ";") != 0 &&
            strcmp(tokens[*pos], "&") != 0) {
        cmd->argv[i++] = tokens[(*pos)++];
    }

    if (tokens[*pos] != NULL && strcmp(tokens[*pos], "&") == 0) {
        cmd->is_background = 1;
        (*pos)++;
    }

    cmd->argv[i] = NULL;
    return cmd;
}

// parse pipeline
Command* parse_pipeline(int *pos) {
    Command *left = parse_command(pos);
    while (tokens[*pos] && strcmp(tokens[*pos], "|") == 0) {
        (*pos)++;
        Command *right = parse_command(pos);

        Command *pipe_cmd = malloc(sizeof(Command));
        pipe_cmd->type = CMD_PIPELINE;
        pipe_cmd->left = left;
        pipe_cmd->right = right;
        left = pipe_cmd;
    }
    return left;
}

// parse sequence, and, or
Command* parse_sequence(int *pos) {
    Command *left = parse_pipeline(pos);
    while (tokens[*pos]) {
        CommandType type;
        if (strcmp(tokens[*pos], ";") == 0) type = CMD_SEQUENCE;
        else if (strcmp(tokens[*pos], "&&") == 0) type = CMD_AND;
        else if (strcmp(tokens[*pos], "||") == 0) type = CMD_OR;
        else break;

        (*pos)++;
        Command *right = parse_pipeline(pos);

        Command *cmd = malloc(sizeof(Command));
        cmd->type = type;
        cmd->left = left;
        cmd->right = right;
        left = cmd;
    }
    return left;
}

// print command tree
void print_command_tree(Command *cmd, int depth) {
    if (cmd == NULL) return;

    for (int i = 0; i < depth; i++) printf("\t");

    switch (cmd->type) {
        case CMD_NORMAL:
            printf("CMD_NORMAL (%d): ",cmd->is_background);
            for (int i = 0; cmd->argv[i] != NULL; i++) {
                printf("%s ", cmd->argv[i]);
            }
            printf("\n");
            break;
        case CMD_PIPELINE:
            printf("CMD_PIPELINE\n");
            break;
        case CMD_SEQUENCE:
            printf("CMD_SEQUENCE (;)\n");
            break;
        case CMD_AND:
            printf("CMD_AND (&&)\n");
            break;
        case CMD_OR:
            printf("CMD_OR (||)\n");
            break;
        default:
            printf("UNKNOWN\n");
            break;
    }

    if (cmd->left) {
        print_command_tree(cmd->left, depth + 1);
    }
    if (cmd->right) {
        print_command_tree(cmd->right, depth + 1);
    }
}

// get user information
void get_user_info() {    
    printf("Welcome to my Bash Shell!\n");

    printf("Enter username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;

    printf("Enter password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;

    gethostname(hostname, sizeof(hostname));

    getcwd(current_path, sizeof(current_path));
}

// pwd
void pwd() {
    char cwd[MAX_INPUT_SIZE];
            if(getcwd(cwd, sizeof(cwd)) != NULL){   // getcwd : 성공 시 cwd의 주소 반환 / 실패 시 NULL 반환
                printf("%s\n", cwd);
            } else {
                perror("pwd error");
            }
            return;
}

// cd
void cd(char *input) {
    char *path = input + 3;
    if(path[0] == '~'){
        const char *home = getenv("HOME");
        
    }
    if(chdir(path) == 0){   // chdir : 성공 시 0 반환 / 실패 시 -1 반환
        getcwd(current_path, sizeof(current_path));
    } else {
        perror("cd error");
    }
    return;
}

// ls
void ls(int show_all) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(current_path);    // opendir : 성공 시 dir의 주소 반환 / 실패 시 NULL 반환
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {    // readdir : 성공 시 dirent 구조체의 주소 반환 / 실패 시 NULL 반환
        if (show_all || entry->d_name[0] != '.') {  // show_all : 1이면 숨김 파일도 출력 / 0이면 숨김 파일 제외
            printf("%s\n", entry->d_name);
        }
    }

    closedir(dir);
    return;
}

// cat
void cat(char *input) {
    char *filename = input + 4;
    FILE *file = fopen(filename, "r");   // fopen : 성공 시 file의 주소 반환 / 실패 시 NULL 반환 / r : 읽기 전용 모드
    if (file == NULL) {
        perror("cat error");
        return;
    }
    char c;
    while ((c = fgetc(file)) != EOF) {  // fgetc : 성공 시 문자 반환 / 실패 시 EOF 반환
        putchar(c);
    }
    fclose(file);
    printf("\n");   
    return;
}

//free command tree
void free_command(Command *cmd) {
    if (cmd == NULL) return;

    if (cmd->left) free_command(cmd->left);
    if (cmd->right) free_command(cmd->right);

    free(cmd);
}

// main
int main(){
    // get user information
    get_user_info();

    while(1){
        // prompt 
        const char *home = getenv("HOME");
        if(strncmp(current_path, home, strlen(home)) == 0){ // home directory = '~'
            printf("%s@%s:~%s$ ", username, hostname, current_path + strlen(home));
        } else{
            printf("%s@%s:%s$ ", username, hostname, current_path);
        }
        fgets(input, MAX_INPUT_SIZE, stdin);
        input[strcspn(input, "\n")] = 0;

        // tokenize input
        tokenize(input);

        for(int i = 0; tokens[i] != NULL; i++){
            printf("%s \n", tokens[i]);
        }
        printf("\n");

        // parse input(make command tree)
        Command *cmd = parse_input();

        // print command tree
        print_command_tree(cmd, 0);

        // parse input(make command tree)
        
        // print command tree

        // exit
        if(strcmp(input, "exit") == 0){
            printf("logout\n");
            break;
        }

        // // pwd
        // else if(strcmp(input, "pwd") == 0){
        //     pwd();
        // }

        // // cd
        // else if(strncmp(input, "cd ", 3) == 0){
        //     cd(input);
        // }

        // // ls
        // else if(strncmp(input, "ls", 2) == 0){
        //     if(strstr(input, "-a")){
        //         ls(1);
        //     } else{
        //         ls(0);
        //     }
        // }

        // // cat
        // else if(strncmp(input, "cat ", 4) == 0){
        //     cat(input);
        // }

        // free tokens
        clear_tokens();
        // free command tree
        free_command(cmd);
    }
    return 0;
}