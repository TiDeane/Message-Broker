#include "common.h"

char* construct_message(char **msg_elements) {
    uint8_t op_code;

    // 4 : "op_code[2]|"
    char *message = malloc(4 * sizeof(char));

    // There are always at least 2 arguments
    strcat(message, msg_elements[0]);
    strcat(message, "|");

    if (isdigit(message[0]) > 0)
        if (strlen(msg_elements[0]) == 2)
            op_code = (uint8_t) atoi(msg_elements[0]);
        else
            op_code = (uint8_t) (message[0] - '0');
    else
        return NULL;

    char *temp;
    // This assumes message structure is correct
    switch (op_code) {
        case 1:
            // 292 : "op_code[1]|client_pipe_path[256]|box_name[32]"
            temp = realloc(message, 292 * sizeof(char));
            if (temp != NULL)
                message = temp;
            else
                return NULL;
            strcat(message, msg_elements[1]);
            strcat(message, "|");
            strcat(message, msg_elements[2]);

            break;
        case 2:
            // 292 : "op_code[1]|client_pipe_path[256]|box_name[32]"
            temp = realloc(message, 292 * sizeof(char));
            if (temp != NULL)
                message = temp;
            else
                return NULL;
            strcat(message, msg_elements[1]);
            strcat(message, "|");
            strcat(message, msg_elements[2]);
            
            break;
        case 3:
            // 292 : "op_code[1]|client_pipe_path[256]|box_name[32]"
            temp = realloc(message, 292 * sizeof(char));
            if (temp != NULL)
                message = temp;
            else
                return NULL;
            strcat(message, msg_elements[1]);
            strcat(message, "|");
            strcat(message, msg_elements[2]);
            
            break;
        case 4: 
            // 1037 : "op_code[1]|return_code[10]|error_message[1024]"
            temp = realloc(message, 1037 * sizeof(char));
            if (temp != NULL)
                message = temp;
            else
                return NULL;
            strcat(message, msg_elements[1]);
            strcat(message, "|");
            strcat(message, msg_elements[2]);
            
            break;
        case 5:
            // 292 : "op_code[1]|client_pipe_path[256]|box_name[32]"
            temp = realloc(message, 292 * sizeof(char));
            if (temp != NULL)
                message = temp;
            else
                return NULL;
            strcat(message, msg_elements[1]);
            strcat(message, "|");
            strcat(message, msg_elements[2]);
            
            break;
        case 6:
            // 1037 : "op_code[1]|return_code[10]|error_message[1024]"
            temp = realloc(message, 1037 * sizeof(char));
            if (temp != NULL)
                message = temp;
            else
                return NULL;
            strcat(message, msg_elements[1]);
            strcat(message, "|");
            strcat(message, msg_elements[2]);
            
            break;
        case 7:
            // 259 : "op_code[1]|client_pipe_path[256]"
            temp = realloc(message, 259 * sizeof(char));
            if (temp != NULL)
                message = temp;
            else
                return NULL;
            strcat(message, msg_elements[1]);

            break;
        case 8:
            // 81 : "op_code[1]|last[1]|box_name[32]|box_size[20]|n_publishers[1]|n_subscribers[20]"
            temp = realloc(message, 81 * sizeof(char));
            if (temp != NULL)
                message = temp;
            else
                return NULL;
            strcat(message, msg_elements[1]);
            strcat(message, "|");
            strcat(message, msg_elements[2]);
            strcat(message, "|");
            strcat(message, msg_elements[3]);
            strcat(message, "|");
            strcat(message, msg_elements[4]);
            strcat(message, "|");
            strcat(message, msg_elements[5]);

            break;
        case 9:
            // 1027 : "op_code[1]|message [1024]"
            temp = realloc(message, 1027 * sizeof(char));
            if (temp != NULL)
                message = temp;
            else
                return NULL;
            strcat(message, msg_elements[1]);

            break;
        case 10:
            // 1028 : "op_code[2]|message [1024]"
            temp = realloc(message, 1028 * sizeof(char));
            if (temp != NULL)
                message = temp;
            else
                return NULL;
            strcat(message, msg_elements[1]);

            break;
        default:
            return NULL;
    }
    return message;
}

/*
// TODO: USE CONSTANTS FOR THE MALLOC VALUES
// Constants should be maximum size
// example: 1 < op_code < 255, so op_code shoud alloc 3+1
char **deconstruct_message(char *message) {
    uint8_t op_code;
    char **msg_elements;

    // Note: the first token is the operation code
    char *token = strtok(message, "|");

    if (isdigit(token[0]) > 0)
        if (strlen(token) == 2)
            op_code = (uint8_t) atoi(token);
        else
            op_code = (uint8_t) (token[0] - '0');
    else
        return NULL;

    switch (op_code) {
        case 1:
            msg_elements = malloc(3 * sizeof(char*));
            if (msg_elements == NULL)
                return NULL;

            msg_elements[0] = calloc(1+1, sizeof(char)); // op_code
            msg_elements[1] = calloc(256+1, sizeof(char)); // client_pipe_path
            msg_elements[2] = calloc(32+1, sizeof(char)); // box_size
            if (msg_elements[0] == NULL || msg_elements[1] == NULL || msg_elements[2] == NULL)
                return NULL;

            strcpy(msg_elements[0], token);
            strcpy(msg_elements[1], strtok(NULL, "|"));
            strcpy(msg_elements[2], strtok(NULL, "|"));

            break;
        case 2:
            msg_elements = malloc(3 * sizeof(char*));
            if (msg_elements == NULL)
                return NULL;

            msg_elements[0] = calloc(1+1, sizeof(char)); // op_code
            msg_elements[1] = calloc(256+1, sizeof(char)); // client_pipe_path
            msg_elements[2] = calloc(32+1, sizeof(char)); // box_size
            if (msg_elements[0] == NULL || msg_elements[1] == NULL || msg_elements[2] == NULL)
                return NULL;

            strcpy(msg_elements[0], token);
            strcpy(msg_elements[1], strtok(NULL, "|"));
            strcpy(msg_elements[2], strtok(NULL, "|"));

            break;
        case 3:
            msg_elements = malloc(3 * sizeof(char*));
            if (msg_elements == NULL)
                return NULL;

            msg_elements[0] = calloc(1+1, sizeof(char)); // op_code
            msg_elements[1] = calloc(256+1, sizeof(char)); // client_pipe_path
            msg_elements[2] = calloc(32+1, sizeof(char)); // box_size
            if (msg_elements[0] == NULL || msg_elements[1] == NULL || msg_elements[2] == NULL)
                return NULL;

            strcpy(msg_elements[0], token);
            strcpy(msg_elements[1], strtok(NULL, "|"));
            strcpy(msg_elements[2], strtok(NULL, "|"));

            break;
        case 4:
            msg_elements = malloc(3 * sizeof(char*));
            if (msg_elements == NULL)
                return NULL;

            msg_elements[0] = calloc(1+1, sizeof(char)); // op_code
            msg_elements[1] = calloc(10+1, sizeof(char)); // return_code
            msg_elements[2] = calloc(1024+1, sizeof(char)); // error message
            if (msg_elements[0] == NULL || msg_elements[1] == NULL || msg_elements[2] == NULL)
                return NULL;

            strcpy(msg_elements[0], token);
            strcpy(msg_elements[1], strtok(NULL, "|"));
            strcpy(msg_elements[2], strtok(NULL, "|"));

            break;
        case 5:
            msg_elements = malloc(3 * sizeof(char*));
            if (msg_elements == NULL)
                return NULL;

            msg_elements[0] = calloc(1+1, sizeof(char)); // op_code
            msg_elements[1] = calloc(256+1, sizeof(char)); // client_pipe_path
            msg_elements[2] = calloc(32+1, sizeof(char)); // box_size
            if (msg_elements[0] == NULL || msg_elements[1] == NULL || msg_elements[2] == NULL)
                return NULL;

            strcpy(msg_elements[0], token);
            strcpy(msg_elements[1], strtok(NULL, "|"));
            strcpy(msg_elements[2], strtok(NULL, "|"));

            break;
        case 6:
            msg_elements = malloc(3 * sizeof(char*));
            if (msg_elements == NULL)
                return NULL;

            msg_elements[0] = calloc(1+1, sizeof(char)); // op_code
            msg_elements[1] = calloc(10+1, sizeof(char)); // return_code (int32_t)
            msg_elements[2] = calloc(1024+1, sizeof(char)); // error_message
            if (msg_elements[0] == NULL || msg_elements[1] == NULL || msg_elements[2] == NULL)
                return NULL;

            strcpy(msg_elements[0], token);
            strcpy(msg_elements[1], strtok(NULL, "|"));
            strcpy(msg_elements[2], strtok(NULL, "|"));

            break;
        case 7:
            msg_elements = malloc(2 * sizeof(char*));
            if (msg_elements == NULL)
                return NULL;

            msg_elements[0] = calloc(1+1, sizeof(char)); // op_code
            msg_elements[1] = calloc(256+1, sizeof(char)); // client_pipe_path
            if (msg_elements[0] == NULL || msg_elements[1] == NULL)
                return NULL;

            strcpy(msg_elements[0], token);
            strcpy(msg_elements[1], strtok(NULL, "|"));

            break;
        case 8:
            msg_elements = malloc(6 * sizeof(char*));
            if (msg_elements == NULL)
                return NULL;

            msg_elements[0] = calloc(1+1, sizeof(char)); // op_code
            msg_elements[1] = calloc(1+1, sizeof(char)); // last
            msg_elements[2] = calloc(32+1, sizeof(char)); // box_name
            msg_elements[3] = calloc(20+1, sizeof(char)); // box_size
            msg_elements[4] = calloc(1+1, sizeof(char)); // n_publishers
            msg_elements[5] = calloc(20+1, sizeof(char)); // n_subscribers
            if (msg_elements[0] == NULL || msg_elements[1] == NULL || msg_elements[2] == NULL
                || msg_elements[3] == NULL || msg_elements[4] == NULL || msg_elements[5] == NULL)
                return NULL;

            strcpy(msg_elements[0], token);
            strcpy(msg_elements[1], strtok(NULL, "|"));
            strcpy(msg_elements[2], strtok(NULL, "|"));
            strcpy(msg_elements[3], strtok(NULL, "|"));
            strcpy(msg_elements[4], strtok(NULL, "|"));
            strcpy(msg_elements[5], strtok(NULL, "|"));

            break;
        case 9:
            msg_elements = malloc(2 * sizeof(char*));
            if (msg_elements == NULL)
                return NULL;

            msg_elements[0] = calloc(1+1, sizeof(char)); // op_code
            msg_elements[1] = calloc(1024+1, sizeof(char)); // message
            if (msg_elements[0] == NULL || msg_elements[1] == NULL)
                return NULL;

            strcpy(msg_elements[0], token);
            strcpy(msg_elements[1], strtok(NULL, "|"));

            break;
        case 10:
            msg_elements = malloc(2 * sizeof(char*));
            if (msg_elements == NULL)
                return NULL;

            msg_elements[0] = calloc(2+1, sizeof(char)); // op_code
            msg_elements[1] = calloc(1024+1, sizeof(char)); // message
            if (msg_elements[0] == NULL || msg_elements[1] == NULL)
                return NULL;

            strcpy(msg_elements[0], token);
            strcpy(msg_elements[1], strtok(NULL, "|"));

            break;
        default:
            return NULL;
    }

    free(message);

    return msg_elements;
}
*/