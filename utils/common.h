#ifndef __UTILS_COMMON_H__
#define __UTILS_COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>

/* MAX LENGTH OF DIFFERENT MESSAGE ELEMENTS */
#define CLIENT_PIPE_PATH_SIZE (256)
#define BOX_NAME_SIZE (32)
#define ERROR_MESSAGE_SIZE (1024)
#define MESSAGE_SIZE (1024)

/* Operation codes */
enum {
    OP_CODE_REG_PUBLISHER = 1,
    OP_CODE_REG_SUBSCRIBER = 2,
    OP_CODE_CREAT_MAILBOX = 3,
    OP_CODE_CREAT_MAILBOX_ANS = 4,
    OP_CODE_RM_MAILBOX = 5,
    OP_CODE_RM_MAILBOX_ANS = 6,
    OP_CODE_LIST_MAILBOX = 7,
    OP_CODE_LIST_MAILBOX_ANS = 8,
    OP_CODE_PUBLISHER_SERVER_MESSAGE = 9,
    OP_CODE_SERVER_PUBLISHER_MESSAGE = 10,
    OP_CODE_REG_PUBLISHER_ANS = 11,
    OP_CODE_REG_SUBSCRIBER_ANS = 12
};

typedef struct {
    uint8_t last;
    char box_name[BOX_NAME_SIZE];
    uint64_t box_size;
    uint64_t n_publishers;
    uint64_t n_subscribers;
} box_listing;

typedef struct {                                /* Request (client --> server) */
    uint8_t op_code;                            /* Operation code */

    union {
        char client_pipe_path[CLIENT_PIPE_PATH_SIZE]; /* op_code 1, 2, 3, 5 or 7 */
        struct {};                                    /* op_code 9 */
    } u_client_pipe_path;

    union {
        char box_name[BOX_NAME_SIZE];           /* op_code 1, 2, 3 or 5 */
        struct {};                              /* op_code 7 or 9 */
    } u_box_name;

    union {
        char message[MESSAGE_SIZE];             /* op_code 9 */
        struct {};                              /* op_code 1, 2, 3, 5 or 7 */
    } u_publisher_message;
} request;

typedef struct {                                /* Response (server --> client) */
    uint8_t op_code;                            /* Operation code */

    union {
        char message[MESSAGE_SIZE];             /* op_code 9 or 10 */
        char error_message[ERROR_MESSAGE_SIZE]; /* op_code 4, 6, 11 or 12 */
        struct {};                              /* op_code 8 */
    } u_response_message;

    union {
        int32_t return_code;                    /* op_code 4, 6, 11 or 12*/
        struct {};                              /* op_code 8, 9 or 10 */
    } u_return_code;

    union {
        box_listing box_listing;                /* op_code 8 */
        struct {};                              /* op_code 4, 6, 9, 10, 11 or 12 */
    } u_box_listing;
} response;

// Helper function to send messages
// Retries to send whatever was not sent in the beginning
void send_msg(int pipe, char const *msg);

#endif // __UTILS_COMMON_H__