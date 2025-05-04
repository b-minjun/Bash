#include "shell.h"

// get user information
void get_user_info(char *username, char *password, char *hostname, char *current_path) {    
    printf("Welcome to my Bash Shell!\n");

    printf("Enter username: ");
    fgets(username, MAX_INPUT_SIZE, stdin);
    username[strcspn(username, "\n")] = 0;

    printf("Enter password: ");
    fgets(password, MAX_INPUT_SIZE, stdin);
    password[strcspn(password, "\n")] = 0;

    gethostname(hostname, MAX_INPUT_SIZE);

    getcwd(current_path, MAX_INPUT_SIZE);
}

// print command token
void print_command_token(char *tokens[]) {
    for (int i = 0; tokens[i] != NULL; i++) {
        printf("%s \n", tokens[i]);
    }
    return;
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

//free command tree
void free_command(Command *cmd) {
    if (cmd == NULL) return;

    if (cmd->left) free_command(cmd->left);
    if (cmd->right) free_command(cmd->right);

    free(cmd);
}

// free tokens
void free_tokens(char *tokens[]) {
    for (int i = 0; i < MAX_ARGS; i++) {
        if (tokens[i]) {
            free(tokens[i]);
            tokens[i] = NULL;
        }
    }
}