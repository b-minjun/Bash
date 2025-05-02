#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/wait.h>

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
int is_multi_command();
Command* parse_input();
Command* parse_command(int *pos);
Command* parse_pipeline(int *pos);
Command* parse_sequence(int *pos);
void execute_command(Command *cmd);
void print_command_tree(Command *cmd, int depth);
void get_user_info();
void pwd(int is_background);
int cd(char *input);
void ls(int is_background, int show_all);
void cat(int is_background, char *input);
void free_command(Command *cmd);
void clear_tokens();

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

int is_multi_command() {
    for (int i = 0; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], ";") == 0 ||
            strcmp(tokens[i], "&&") == 0 ||
            strcmp(tokens[i], "||") == 0) {
            return 1;
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

// execute command
void execute_command(Command *cmd) {
    if (cmd == NULL) return;

    switch (cmd->type) {
        case CMD_NORMAL: {
            // exit
            if (strcmp(cmd->argv[0], "exit") == 0) {
                printf("logout\n");
                exit(0);
            }
            // cd
            if (strcmp(cmd->argv[0], "cd") == 0) {
                cd(cmd->argv[1]);
                return;
            }// not cd
            pid_t pid = fork();
            if (pid == 0) {
                execvp(cmd->argv[0], cmd->argv);
                perror("execvp failed");
                exit(EXIT_FAILURE);
            } else if (pid > 0) {
                if (!cmd->is_background) {
                    waitpid(pid, NULL, 0);
                }
            } else {
                perror("fork failed");
            }
            break;
        }

        case CMD_SEQUENCE: {
            execute_command(cmd->left);
            execute_command(cmd->right);
            break;
        }

        case CMD_AND: {
            int status = 1;
            if (cmd->left->type == CMD_NORMAL && strcmp(cmd->left->argv[0], "cd") == 0) {
                status = cd(cmd->left->argv[1]);
            } else {    
                pid_t pid = fork();
                if (pid == 0) {
                    execvp(cmd->left->argv[0], cmd->left->argv);
                    perror("execvp failed");
                    exit(EXIT_FAILURE);
                } else if (pid > 0) {
                    int wstatus;
                    waitpid(pid, &wstatus, 0);
                    if (WIFEXITED(wstatus)) {
                        status = WEXITSTATUS(wstatus);
                    }
                }
            }

            if (status == 0) {
                execute_command(cmd->right);
            }
            break;
        }

        case CMD_OR: {
            int status = 1;
            if (cmd->left->type == CMD_NORMAL && strcmp(cmd->left->argv[0], "cd") == 0) {
                status = cd(cmd->left->argv[1]);
            } else {
                pid_t pid = fork();
                if (pid == 0) {
                    execvp(cmd->left->argv[0], cmd->left->argv);
                    perror("execvp failed");
                    exit(EXIT_FAILURE);
                } else if (pid > 0) {
                    int wstatus;
                    waitpid(pid, &wstatus, 0);
                    if (!(WIFEXITED(status))) {
                        status = WEXITSTATUS(status);
                    }
                }
            }
            if (status != 0) {
                execute_command(cmd->right);
            }
            break;
        }

        case CMD_PIPELINE: {
            int fd[2];
            pipe(fd);

            pid_t left_pid = fork();
            if (left_pid == 0) {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[0]);
                close(fd[1]);
                execvp(cmd->left->argv[0], cmd->left->argv);
                perror("execvp left failed");
                exit(EXIT_FAILURE);
            }

            pid_t right_pid = fork();
            if (right_pid == 0) {
                dup2(fd[0], STDIN_FILENO);
                close(fd[1]);
                close(fd[0]);
                execvp(cmd->right->argv[0], cmd->right->argv);
                perror("execvp right failed");
                exit(EXIT_FAILURE);
            }

            close(fd[0]);
            close(fd[1]);
            if (!cmd->left->is_background) {
                waitpid(left_pid, NULL, 0);
            }
            if (!cmd->right->is_background) {
                waitpid(right_pid, NULL, 0);
            }
            break;
        }
    }
}

// print command tree
void print_command_tree(Command *cmd, int depth) {
    if (cmd == NULL) return;
    
    for (int i = 0; i < depth; i++) printf("\t");

    switch (cmd->type) {
        case CMD_NORMAL:
            printf("CMD_NORMAL (is_background: %d): ",cmd->is_background);
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
void pwd(int is_background) {
    char cwd[MAX_INPUT_SIZE];
            if(getcwd(cwd, sizeof(cwd)) != NULL){   // getcwd : 성공 시 cwd의 주소 반환 / 실패 시 NULL 반환
                if(is_background == 0){
                    printf("%s\n", cwd);
                }
            } else {
                perror("pwd error");
            }
            return;
}

// cd
int cd(char *input) {
    char *path = input;
    if(path[0] == '~'){
        const char *home = getenv("HOME");
        
    }
    if(chdir(path) == 0){   // chdir : 성공 시 0 반환 / 실패 시 -1 반환
        getcwd(current_path, sizeof(current_path));
        return 0;
    } else {
        perror("cd error");
        return 1;
    }
}

// ls
void ls(int is_background, int show_all) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(current_path);    // opendir : 성공 시 dir의 주소 반환 / 실패 시 NULL 반환
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL && is_background == 0) {    // readdir : 성공 시 dirent 구조체의 주소 반환 / 실패 시 NULL 반환
        if (show_all || entry->d_name[0] != '.') {  // show_all : 1이면 숨김 파일도 출력 / 0이면 숨김 파일 제외
            printf("%s\n", entry->d_name);
        }
    }

    closedir(dir);
    return;
}

// cat
void cat(int is_background, char *input) {
    if (is_background == 1){
        return;
    }
    char *filename = input;
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

// clear tokens
void clear_tokens() {
    for (int i = 0; i < MAX_ARGS; i++) {
        if (tokens[i]) {
            free(tokens[i]);
            tokens[i] = NULL;
        }
    }
}

// main
int main(){
    // get user information
    get_user_info();

    while(1){
        // prompt 
        const char *home = getenv("HOME");
        if (strncmp(current_path, home, strlen(home)) == 0) { 
            printf("%s@%s:~%s$ ", username, hostname, current_path + strlen(home));
        } else {
            printf("%s@%s:%s$ ", username, hostname, current_path);
        }
        fgets(input, MAX_INPUT_SIZE, stdin);
        input[strcspn(input, "\n")] = 0;

        int print_tokens = 0;
        int print_tree = 0;

        // tokenize input
        tokenize(input);

        // parse input(make command tree)
        Command *cmd = parse_input();

        int is_multi = is_multi_command();

        // print tokens
        if(print_tokens == 1){
            printf("Tokens:\n");
            for (int i = 0; tokens[i] != NULL; i++) {
                printf("%s \n", tokens[i]);
            }
            printf("\n");
        }

        // print command tree
        if(print_tree == 1){
            printf("Command Tree:\n");
            print_command_tree(cmd, 0);
            printf("\n");
        }
        
        // exit
        if (is_multi == 0 && strcmp(cmd->argv[0], "exit") == 0) {
            printf("logout\n");
            break;
        }

        // pwd
        else if(is_multi == 0 && strcmp(cmd->argv[0], "pwd") == 0){
            pwd(cmd->is_background);
        }

        // cd
        else if(is_multi == 0 && strcmp(cmd->argv[0], "cd") == 0){
            cd(cmd->argv[1]);
        }

        // ls
        else if(is_multi == 0 && strcmp(cmd->argv[0], "ls") == 0){
            if(cmd->argv[1] && strstr(cmd->argv[1], "-a")){
                ls(cmd->is_background, 1);
            } else{
                ls(cmd->is_background, 0);
            }
        }

        // cat
        else if(is_multi == 0 && strcmp(cmd->argv[0], "cat") == 0)  {
            cat(cmd->is_background, cmd->argv[1]);
        }

        // execute command exept for built-in commands
        else{
            execute_command(cmd);
        }

        // free tokens
        clear_tokens();
        // free command tree
        free_command(cmd);
    }
    return 0;
}