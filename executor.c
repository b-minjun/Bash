#include "shell.h"

// execute command
void execute_command(Command *cmd, char *current_path) {
    if (cmd == NULL) return;

    switch (cmd->type) {
        case CMD_NORMAL: {
            // exit
            if (cmd->argv[0] && strcmp(cmd->argv[0], "exit") == 0) {
                printf("logout\n");
                exit(0);
            }
            // cd
            if (cmd->argv[0] && strcmp(cmd->argv[0], "cd") == 0) {
                cd(cmd->argv[1], current_path);
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
            execute_command(cmd->left, current_path);
            execute_command(cmd->right, current_path);
            break;
        }

        case CMD_AND: {
            int status = 1;
            if (cmd->left->type == CMD_NORMAL && strcmp(cmd->left->argv[0], "cd") == 0) {
                status = cd(cmd->left->argv[1], current_path);
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
                execute_command(cmd->right, current_path);
            }
            break;
        }

        case CMD_OR: {
            int status = 1;
            if (cmd->left->type == CMD_NORMAL && strcmp(cmd->left->argv[0], "cd") == 0) {
                status = cd(cmd->left->argv[1], current_path);
            } else {
                pid_t pid = fork();
                if (pid == 0) {
                    execvp(cmd->left->argv[0], cmd->left->argv);
                    perror("execvp failed");
                    exit(EXIT_FAILURE);
                } else if (pid > 0) {
                    int wstatus;
                    waitpid(pid, &wstatus, 0);
                    if (!(WIFEXITED(wstatus))) {
                        status = WEXITSTATUS(wstatus);
                    }
                }
            }
            if (status != 0) {
                execute_command(cmd->right, current_path);
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
int cd(char *input, char *current_path) {
    char *path = input;
    if(path[0] == '~'){
        const char *home = getenv("HOME");
        
    }
    if(chdir(path) == 0){   // chdir : 성공 시 0 반환 / 실패 시 -1 반환
        getcwd(current_path, MAX_INPUT_SIZE);
        return 0;
    } else {
        perror("cd error");
        return 1;
    }
}

// ls
void ls(int is_background, int show_all, char *current_path) {
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