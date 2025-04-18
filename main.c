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

// CommandType
typedef enum CommandType {
    CMD_NORMAL,
    CMD_PIPELINE,
    CMD_SEQUENCE,
    CMD_AND,
    CMD_OR,
    CMD_BACKGROUND
} CommandType;

// Command
typedef struct Command {
    CommandType type;
    struct Command *left;
    struct Command *right;
    char *argv[MAX_ARGS];
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

        // ls
        else if(strncmp(input, "ls", 2) == 0){
            if(strstr(input, "-a")){
                ls(1);
            } else{
                ls(0);
            }
        }

        // cat
        else if(strncmp(input, "cat ", 4) == 0){
            cat(input);
        }

    }
    return 0;
}