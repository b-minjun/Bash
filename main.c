#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#define MAX_INPUT_SIZE 4096

char username[MAX_INPUT_SIZE];
char password[MAX_INPUT_SIZE];
char hostname[MAX_INPUT_SIZE];
char input[MAX_INPUT_SIZE];
char current_path[MAX_INPUT_SIZE];


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
            if(getcwd(cwd, sizeof(cwd)) != NULL){// cwd : 성공 시 cwd의 주소 반환 / 실패 시 NULL 반환
                printf("%s\n", cwd);
            } else {
                printf("pwd error\n");
            }
            return;
}

// cd
void cd(char *input) {
    char *path = input + 3;
    if(chdir(path) != 0){// chdir : 성공 시 0 반환 / 실패 시 -1 반환
        printf("cd error\n");
    } else {
        getcwd(current_path, sizeof(current_path));
    }
    return;
}

int main(){
    // get user information
    get_user_info();

    while(1){
        // prompt 
        printf("%s@%s:%s$ ", username, hostname, current_path);
        fgets(input, MAX_INPUT_SIZE, stdin);
        input[strcspn(input, "\n")] = 0;

        // exit
        if(strcmp(input, "exit") == 0){
            printf("logout\n");
            break;
        }

        // pwd
        else if(strcmp(input, "pwd") == 0){
            pwd();
        }

        // cd
        else if(strncmp(input, "cd ", 3) == 0){
            cd(input);
        }

    }
    return 0;
}