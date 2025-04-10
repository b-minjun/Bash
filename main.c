#include <stdio.h>
#include <unistd.h>
#include <string.h>

char username[100];
char password[100];
char hostname[100];
char prompt[100];

int main(){
    // get user information
    printf("Welcome to my Bash Shell!\n");
    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);
    gethostname(hostname, sizeof(hostname));

    while(1){
        // prompt
        printf("%s@%s:~$ ", username, hostname);
        scanf("%s", prompt);

        // exit
        if(strcmp(prompt, "exit") == 0){
            printf("logout\n");
            break;
        }
    }
    return 0;
    
}