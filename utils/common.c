#include "common.h"

// Returns NULL in case of error
char* construct_message(char **msg_elements) {
    uint8_t op_code;

    char *message = malloc((OP_CODE_SIZE + 1) * sizeof(char));
    if (message == NULL)
        return NULL;

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
        case OP_CODE_REG_PUBLISHER:
            temp = realloc(message, (MESSAGE_CODE_1_SIZE + 1) * sizeof(char));
            if (temp != NULL)
                message = temp;
            else
                return NULL;

            strcat(message, msg_elements[1]);
            strcat(message, "|");
            strcat(message, msg_elements[2]);

            break;
        case OP_CODE_REG_SUBSCRIBER:
            temp = realloc(message, (MESSAGE_CODE_2_SIZE + 1) * sizeof(char));
            if (temp != NULL)
                message = temp;
            else
                return NULL;

            strcat(message, msg_elements[1]);
            strcat(message, "|");
            strcat(message, msg_elements[2]);
            
            break;
        case OP_CODE_CREAT_MAILBOX:
            temp = realloc(message, (MESSAGE_CODE_3_SIZE + 1) * sizeof(char));
            if (temp != NULL)
                message = temp;
            else
                return NULL;

            strcat(message, msg_elements[1]);
            strcat(message, "|");
            strcat(message, msg_elements[2]);
            
            break;
        case OP_CODE_CREAT_MAILBOX_ANS:
            temp = realloc(message, (MESSAGE_CODE_4_SIZE + 1) * sizeof(char));
            if (temp != NULL)
                message = temp;
            else
                return NULL;

            strcat(message, msg_elements[1]);
            strcat(message, "|");
            strcat(message, msg_elements[2]);
            
            break;
        case OP_CODE_RM_MAILBOX:
            temp = realloc(message, (MESSAGE_CODE_5_SIZE + 1) * sizeof(char));
            if (temp != NULL)
                message = temp;
            else
                return NULL;

            strcat(message, msg_elements[1]);
            strcat(message, "|");
            strcat(message, msg_elements[2]);
            
            break;
        case OP_CODE_RM_MAILBOX_ANS:
            temp = realloc(message, (MESSAGE_CODE_6_SIZE + 1) * sizeof(char));
            if (temp != NULL)
                message = temp;
            else
                return NULL;

            strcat(message, msg_elements[1]);
            strcat(message, "|");
            strcat(message, msg_elements[2]);
            
            break;
        case OP_CODE_LIST_MAILBOX:
            temp = realloc(message, (MESSAGE_CODE_7_SIZE + 1) * sizeof(char));
            if (temp != NULL)
                message = temp;
            else
                return NULL;

            strcat(message, msg_elements[1]);

            break;
        case OP_CODE_LIST_MAILBOX_ANS:
            temp = realloc(message, (MESSAGE_CODE_8_SIZE + 1) * sizeof(char));
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
        case OP_CODE_PUBLISHER_SERVER_MESSAGE:
            temp = realloc(message, (MESSAGE_CODE_9_SIZE + 1) * sizeof(char));
            if (temp != NULL)
                message = temp;
            else
                return NULL;

            strcat(message, msg_elements[1]);

            break;
        case OP_CODE_SERVER_PUBLISHER_MESSAGE:
            temp = realloc(message, (MESSAGE_CODE_10_SIZE + 1) * sizeof(char));
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

// Returns NULL in case of error
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
        case OP_CODE_REG_PUBLISHER:
            msg_elements = malloc(3 * sizeof(char*));
            if (msg_elements == NULL)
                return NULL;

            msg_elements[0] = calloc(OP_CODE_SIZE + 1, sizeof(char));
            msg_elements[1] = calloc(CLIENT_PIPE_PATH_SIZE + 1, sizeof(char));
            msg_elements[2] = calloc(BOX_NAME_SIZE + 1, sizeof(char));
            if (msg_elements[0] == NULL || msg_elements[1] == NULL || msg_elements[2] == NULL)
                return NULL;

            strcpy(msg_elements[0], token);
            strcpy(msg_elements[1], strtok(NULL, "|"));
            strcpy(msg_elements[2], strtok(NULL, "|"));

            break;
        case OP_CODE_REG_SUBSCRIBER:
            msg_elements = malloc(3 * sizeof(char*));
            if (msg_elements == NULL)
                return NULL;

            msg_elements[0] = calloc(OP_CODE_SIZE + 1, sizeof(char));
            msg_elements[1] = calloc(CLIENT_PIPE_PATH_SIZE + 1, sizeof(char));
            msg_elements[2] = calloc(BOX_NAME_SIZE + 1, sizeof(char));
            if (msg_elements[0] == NULL || msg_elements[1] == NULL || msg_elements[2] == NULL)
                return NULL;

            strcpy(msg_elements[0], token);
            strcpy(msg_elements[1], strtok(NULL, "|"));
            strcpy(msg_elements[2], strtok(NULL, "|"));

            break;
        case OP_CODE_CREAT_MAILBOX:
            msg_elements = malloc(3 * sizeof(char*));
            if (msg_elements == NULL)
                return NULL;

            msg_elements[0] = calloc(OP_CODE_SIZE + 1, sizeof(char));
            msg_elements[1] = calloc(CLIENT_PIPE_PATH_SIZE + 1, sizeof(char));
            msg_elements[2] = calloc(BOX_NAME_SIZE + 1, sizeof(char));
            if (msg_elements[0] == NULL || msg_elements[1] == NULL || msg_elements[2] == NULL)
                return NULL;

            strcpy(msg_elements[0], token);
            strcpy(msg_elements[1], strtok(NULL, "|"));
            strcpy(msg_elements[2], strtok(NULL, "|"));

            break;
        case OP_CODE_CREAT_MAILBOX_ANS:
            msg_elements = malloc(3 * sizeof(char*));
            if (msg_elements == NULL)
                return NULL;

            msg_elements[0] = calloc(OP_CODE_SIZE+1, sizeof(char));
            msg_elements[1] = calloc(RETURN_CODE_SIZE+1, sizeof(char));
            msg_elements[2] = calloc(ERROR_MESSAGE_SIZE+1, sizeof(char));
            if (msg_elements[0] == NULL || msg_elements[1] == NULL || msg_elements[2] == NULL)
                return NULL;

            strcpy(msg_elements[0], token);
            strcpy(msg_elements[1], strtok(NULL, "|"));
            strcpy(msg_elements[2], strtok(NULL, "|"));

            break;
        case OP_CODE_RM_MAILBOX:
            msg_elements = malloc(3 * sizeof(char*));
            if (msg_elements == NULL)
                return NULL;

            msg_elements[0] = calloc(OP_CODE_SIZE + 1, sizeof(char));
            msg_elements[1] = calloc(CLIENT_PIPE_PATH_SIZE + 1, sizeof(char));
            msg_elements[2] = calloc(BOX_NAME_SIZE + 1, sizeof(char));
            if (msg_elements[0] == NULL || msg_elements[1] == NULL || msg_elements[2] == NULL)
                return NULL;

            strcpy(msg_elements[0], token);
            strcpy(msg_elements[1], strtok(NULL, "|"));
            strcpy(msg_elements[2], strtok(NULL, "|"));

            break;
        case OP_CODE_RM_MAILBOX_ANS:
            msg_elements = malloc(3 * sizeof(char*));
            if (msg_elements == NULL)
                return NULL;

            msg_elements[0] = calloc(OP_CODE_SIZE + 1, sizeof(char));
            msg_elements[1] = calloc(RETURN_CODE_SIZE + 1, sizeof(char));
            msg_elements[2] = calloc(ERROR_MESSAGE_SIZE + 1, sizeof(char));
            if (msg_elements[0] == NULL || msg_elements[1] == NULL || msg_elements[2] == NULL)
                return NULL;

            strcpy(msg_elements[0], token);
            strcpy(msg_elements[1], strtok(NULL, "|"));
            strcpy(msg_elements[2], strtok(NULL, "|"));

            break;
        case OP_CODE_LIST_MAILBOX:
            msg_elements = malloc(2 * sizeof(char*));
            if (msg_elements == NULL)
                return NULL;

            msg_elements[0] = calloc(OP_CODE_SIZE + 1, sizeof(char));
            msg_elements[1] = calloc(CLIENT_PIPE_PATH_SIZE + 1, sizeof(char));
            if (msg_elements[0] == NULL || msg_elements[1] == NULL)
                return NULL;

            strcpy(msg_elements[0], token);
            strcpy(msg_elements[1], strtok(NULL, "|"));

            break;
        case OP_CODE_LIST_MAILBOX_ANS:
            msg_elements = malloc(6 * sizeof(char*));
            if (msg_elements == NULL)
                return NULL;

            msg_elements[0] = calloc(OP_CODE_SIZE + 1, sizeof(char));
            msg_elements[1] = calloc(LAST_BYTE_SIZE + 1, sizeof(char));
            msg_elements[2] = calloc(BOX_NAME_SIZE + 1, sizeof(char));
            msg_elements[3] = calloc(BOX_BYTES_SIZE + 1, sizeof(char));
            msg_elements[4] = calloc(N_PUBLISHERS_SIZE + 1, sizeof(char));
            msg_elements[5] = calloc(N_SUBSCRIBERS_SIZE + 1, sizeof(char));
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
        case OP_CODE_PUBLISHER_SERVER_MESSAGE:
            msg_elements = malloc(2 * sizeof(char*));
            if (msg_elements == NULL)
                return NULL;

            msg_elements[0] = calloc(OP_CODE_SIZE + 1, sizeof(char));
            msg_elements[1] = calloc(MESSAGE_SIZE + 1, sizeof(char));
            if (msg_elements[0] == NULL || msg_elements[1] == NULL)
                return NULL;

            strcpy(msg_elements[0], token);
            strcpy(msg_elements[1], strtok(NULL, "|"));

            break;
        case OP_CODE_SERVER_PUBLISHER_MESSAGE:
            msg_elements = malloc(2 * sizeof(char*));
            if (msg_elements == NULL)
                return NULL;

            msg_elements[0] = calloc(OP_CODE_SIZE + 1, sizeof(char));
            msg_elements[1] = calloc(MESSAGE_SIZE + 1, sizeof(char));
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