#ifndef MBROKER_H
#define MBROKER_H

#include "common.h"

void register_publisher(request req);

void register_subscriber(request req);

void op_create_mailbox(request req);

void op_remove_mailbox(request req);

void op_list_mailbox(request req);

void publish_message(request req);

#endif // OPERATIONS_H