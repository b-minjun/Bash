#include "shell.h"

// tokenize input
void tokenize(char *input, char *tokens[]) {
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

int is_multi_command(char *tokens[]) {
    for (int i = 0; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], ";") == 0 ||
            strcmp(tokens[i], "&&") == 0 ||
            strcmp(tokens[i], "||") == 0) {
            return 1;
        }
    }
    return 0;
}

// parse input
Command* parse_input(char *tokens[]) {
    int pos = 0;
    Command *cmd = parse_sequence(&pos, tokens);
    return cmd;
}

// parse command
Command* parse_command(int *pos, char *tokens[]) {
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
Command* parse_pipeline(int *pos, char *tokens[]) {
    Command *left = parse_command(pos, tokens);
    while (tokens[*pos] && strcmp(tokens[*pos], "|") == 0) {
        (*pos)++;
        Command *right = parse_command(pos, tokens);

        Command *pipe_cmd = malloc(sizeof(Command));
        pipe_cmd->type = CMD_PIPELINE;
        pipe_cmd->left = left;
        pipe_cmd->right = right;
        left = pipe_cmd;
    }
    return left;
}

// parse sequence, and, or
Command* parse_sequence(int *pos, char *tokens[]) {
    Command *left = parse_pipeline(pos, tokens);
    while (tokens[*pos]) {
        CommandType type;
        if (strcmp(tokens[*pos], ";") == 0) type = CMD_SEQUENCE;
        else if (strcmp(tokens[*pos], "&&") == 0) type = CMD_AND;
        else if (strcmp(tokens[*pos], "||") == 0) type = CMD_OR;
        else break;

        (*pos)++;
        Command *right = parse_pipeline(pos, tokens);

        Command *cmd = malloc(sizeof(Command));
        cmd->type = type;
        cmd->left = left;
        cmd->right = right;
        left = cmd;
    }
    return left;
}