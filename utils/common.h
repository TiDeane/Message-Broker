#ifndef __UTILS_COMMON_H__
#define __UTILS_COMMON_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
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

 /*
  * Receives an array of strings with the different elements that will compose
  * the message, and returns a string with the composed message.
  */
char *construct_message(char **msg_elements);

 /*
  * Receives a message and returns an array of strings composed of each element
  * of the message.
  */
//char **deconstruct_message(char *message);

#endif // __UTILS_COMMON_H__