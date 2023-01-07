#ifndef __UTILS_COMMON_H__
#define __UTILS_COMMON_H__

#include <stdint.h>

/* operation codes */
enum {
    OP_CODE_REG_PUBLISHER = 1,
    OP_CODE_REG_SUBSCRIBER = 2,
    OP_CODE_CREAT_MAILBOX = 3,
    OP_CODE_CREAT_MAILBOX_ANS = 4,
    OP_CODE_RM_MAILBOX = 5,
    OP_CODE_RM_MAILBOX_ANS = 6,
    OP_CODE_LIST_MAILBOX = 7,
    OP_CODE_LIST_MAILBOX_ANS = 8
};

// PROTOTYPE
typedef struct {
    uint8_t op_code;
    char *client_pipe_path;
    char *box_name;
} message_to_server;

// PROTOTYPE
typedef struct {
    uint8_t op_code;
    int32_t return_code;
    char *error_message;
} message_to_client;

// PROTOTYPE
typedef struct {
    uint8_t op_code;
    uint8_t last;
    char *box_name;
    uint64_t box_size;
    uint64_t n_publishers;
    uint64_t n_subscribers;
} box_list_message;

#endif // __UTILS_COMMON_H__