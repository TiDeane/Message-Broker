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
char **deconstruct_message(char *message) {
    // INCOMPLETE
    return NULL;
}
*/