#ifndef __UTILS_COMMON_H__
#define __UTILS_COMMON_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

/* MAX LENGTH OF DIFFERENT MESSAGE ELEMENTS */
#define OP_CODE_SIZE 3 // uint8_t : 0 to 255
#define CLIENT_PIPE_PATH_SIZE 256
#define BOX_NAME_SIZE 32
#define RETURN_CODE_SIZE 10 // int32_t : -2,147,483,648 to 2,147,483,647
#define ERROR_MESSAGE_SIZE 1024
#define LAST_BYTE_SIZE 3 // uint64_t : 0 to 255
#define BOX_BYTES_SIZE 20 // uint64_t : 0 to 18,446,744,073,709,551,615
#define N_PUBLISHERS_SIZE 20 // uint64_t : 0 to 18,446,744,073,709,551,615
#define N_SUBSCRIBERS_SIZE 20 // uint64_t : 0 to 18,446,744,073,709,551,615
#define MESSAGE_SIZE 1024
#define MAX_ANSWER_SIZE (OP_CODE_SIZE + RETURN_CODE_SIZE + ERROR_MESSAGE_SIZE + 2)

/* Max size of each message */
/* Note: the additional added numbers are for the "|" separators */
#define MESSAGE_CODE_1_SIZE (OP_CODE_SIZE + CLIENT_PIPE_PATH_SIZE + BOX_NAME_SIZE + 2)
#define MESSAGE_CODE_2_SIZE (OP_CODE_SIZE + CLIENT_PIPE_PATH_SIZE + BOX_NAME_SIZE + 2)
#define MESSAGE_CODE_3_SIZE (OP_CODE_SIZE + CLIENT_PIPE_PATH_SIZE + BOX_NAME_SIZE + 2)
#define MESSAGE_CODE_4_SIZE (OP_CODE_SIZE + RETURN_CODE_SIZE + ERROR_MESSAGE_SIZE + 2)
#define MESSAGE_CODE_5_SIZE (OP_CODE_SIZE + CLIENT_PIPE_PATH_SIZE + BOX_NAME_SIZE + 2)
#define MESSAGE_CODE_6_SIZE (OP_CODE_SIZE + RETURN_CODE_SIZE + ERROR_MESSAGE_SIZE + 2)
#define MESSAGE_CODE_7_SIZE (OP_CODE_SIZE + CLIENT_PIPE_PATH_SIZE + 1)
#define MESSAGE_CODE_8_SIZE (OP_CODE_SIZE + LAST_BYTE_SIZE + BOX_NAME_SIZE + N_PUBLISHERS_SIZE + N_SUBSCRIBERS_SIZE + 4)
#define MESSAGE_CODE_9_SIZE (OP_CODE_SIZE + MESSAGE_SIZE + 1)
#define MESSAGE_CODE_10_SIZE (OP_CODE_SIZE + MESSAGE_SIZE + 1)

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
    OP_CODE_SERVER_PUBLISHER_MESSAGE = 10
};

// USE THE CONSTANTS ABOVE FOR THE SWITCH CASES
// MAKE CONSTANTS FOR THE SIZE OF MESSAGE ELEMENTS

 /*
  * Receives an array of strings with the different elements that will compose
  * the message, and returns a string with the composed message.
  */
char *construct_message(char **msg_elements);

 /*
  * Receives a message and returns an array of strings composed of each element
  * of the message.
  */
char **deconstruct_message(char *message);

#endif // __UTILS_COMMON_H__