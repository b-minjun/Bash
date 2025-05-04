#include "shell.h"

// main
int main(){
    char username[MAX_INPUT_SIZE] = {0};
    char password[MAX_INPUT_SIZE] = {0};
    char hostname[MAX_INPUT_SIZE] = {0};
    char input[MAX_INPUT_SIZE] = {0};
    char current_path[MAX_INPUT_SIZE] = {0};
    char *tokens[MAX_ARGS] = {0};

    // get user information
    get_user_info(username, password, hostname, current_path);

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
        if (strlen(input) == 0) {
            continue;
        }

        int print_tokens = 0;
        int print_tree = 0;

        // tokenize input
        tokenize(input, tokens);

        // parse input(make command tree)
        Command *cmd = parse_input(tokens);

        int is_multi = is_multi_command(tokens);

        // print tokens
        if(print_tokens == 1){
            printf("Tokens:\n");
            print_command_token(tokens);
            printf("\n");
        }

        // print command tree
        if(print_tree == 1){
            printf("Command Tree:\n");
            print_command_tree(cmd, 0);
            printf("\n");
        }
        
        // exit
        if (is_multi == 0 && cmd->argv[0] && strcmp(cmd->argv[0], "exit") == 0) {
            printf("logout\n");
            break;
        }

        // pwd
        else if(is_multi == 0 && cmd->argv[0] && strcmp(cmd->argv[0], "pwd") == 0){
            pwd(cmd->is_background);
        }

        // cd
        else if(is_multi == 0 && cmd->argv[0] && strcmp(cmd->argv[0], "cd") == 0){
            cd(cmd->argv[1], current_path);
        }

        // ls
        else if(is_multi == 0 && cmd->argv[0] && strcmp(cmd->argv[0], "ls") == 0){
            if(cmd->argv[1] && strstr(cmd->argv[1], "-a")){
                ls(cmd->is_background, 1, current_path);
            } else{
                ls(cmd->is_background, 0, current_path);
            }
        }

        // cat
        else if(is_multi == 0 && cmd->argv[0] && strcmp(cmd->argv[0], "cat") == 0)  {
            cat(cmd->is_background, cmd->argv[1]);
        }

        // execute command exept for built-in commands
        else{
            execute_command(cmd, current_path);
        }

        // free tokens
        free_tokens(tokens);
        // free command tree
        free_command(cmd);
    }
    return 0;
}