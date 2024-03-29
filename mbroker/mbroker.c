#include "logging.h"
#include "common.h"
#include "mbroker.h"
#include "operations.h"
#include "state.h"

static int pipe_server;

static char *named_server_pipe;
static char *publisher_pipe_path;

// Array that stores all mailboxes
static mailbox *mailboxes;

// free_mailboxes[i] is true if mailboxes[i] is currently free/empty
static bool *free_mailboxes;

// Current maximum length of the array
static long unsigned mailbox_alloc = 10;

static void sig_handler(int sig) {

    // UNSAFE: This handler uses non-async-signal-safe functions (printf(),
    // exit();)
    if (sig == SIGINT) {
    // In some systems, after the handler call the signal gets reverted
    // to SIG_DFL (the default action associated with the signal).
    // So we set the signal handler back to our function after each trap.
    //
        tfs_destroy();
        unlink(named_server_pipe);

        if (signal(SIGINT, sig_handler) == SIG_ERR) {
            exit(EXIT_FAILURE);
        }
        return; // Resume execution at point of interruption
    }

    // Must be SIGQUIT - print a message and terminate the process
    tfs_destroy();
    
    exit(EXIT_SUCCESS);
}


int main(int argc, char **argv) {

    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        exit(EXIT_FAILURE);
    }
     
    if (argc < 2) {
        printf("Please specify the pathname of the server's pipe.\n");
        return 1;
    }

    tfs_init(NULL);

    mailboxes = malloc(mailbox_alloc * sizeof(mailbox));
    free_mailboxes = malloc(mailbox_alloc * sizeof(bool));
    for (unsigned long i = 0; i < mailbox_alloc; i++)
        free_mailboxes[i] = true;

    request req;

    named_server_pipe = argv[1];

    // Remove server pipe if it exists
    if (unlink(named_server_pipe) != 0 && errno != ENOENT) {
        fprintf(stderr, "[ERR]: server unlink(%s) failed: %s\n", argv[2],
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Creates the server's named pipe
    if (mkfifo(named_server_pipe, 0640) != 0) {
        fprintf(stderr, "[ERR]: server mkfifo failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    
    while (true) {
        // Opens server pipe for reading
        if ((pipe_server = open(named_server_pipe, O_RDONLY)) == -1) {
            fprintf(stderr, "[ERR]: server open failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (read(pipe_server, &req, sizeof(request)) != sizeof(request)) {
            fprintf(stderr, "[ERR]: server read failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        switch (req.op_code) {
            case OP_CODE_REG_PUBLISHER:
                register_publisher(req);
                break;
            case OP_CODE_REG_SUBSCRIBER:
                register_subscriber(req);
                break;
            case OP_CODE_CREAT_MAILBOX:
                op_create_mailbox(req);
                break;
            case OP_CODE_RM_MAILBOX:
                op_remove_mailbox(req);
                break;
            case OP_CODE_LIST_MAILBOX:
                op_list_mailbox(req);
                break;
            default:
                break;
        }
    }
}

void register_publisher(request req) {
    int fhandle = 0;
    int pipe_publisher;
    ssize_t ret;
    ssize_t bytes_written;
    unsigned long box_index = mailbox_alloc; // impossible value
    response resp;

    publisher_pipe_path = req.u_client_pipe_path.client_pipe_path;

    for (unsigned long i = 0; i < mailbox_alloc; i++) {
        if (free_mailboxes[i] == true)
            continue;
        else {
            if (strcmp(req.u_box_name.box_name, mailboxes[i].box_name) == 0) {
                box_index = i;
                break;
            }
        }
    }

    resp.op_code = OP_CODE_REG_PUBLISHER_ANS;

    if (box_index == mailbox_alloc ) {
        resp.u_return_code.return_code = -1;
        strcpy(resp.u_response_message.error_message, "publisher's mailbox does not exist");
    } else if (mailboxes[box_index].n_publishers == 1) {
        resp.u_return_code.return_code = -1;
        strcpy(resp.u_response_message.error_message, "mailbox already has a publisher");
    } else {
        char box_name_path[33] = "/";
        strcat(box_name_path, req.u_box_name.box_name);

        if ((fhandle = tfs_open(box_name_path, TFS_O_APPEND)) == -1) {
            resp.u_return_code.return_code = -1;
            strcpy(resp.u_response_message.error_message, "publisher could not find the mailbox's file");
        } else {
            resp.u_return_code.return_code = 0;
            memset(resp.u_response_message.error_message, '\0', ERROR_MESSAGE_SIZE);
        }
    }
    if ((pipe_publisher = open(publisher_pipe_path, O_WRONLY)) == -1) {
        fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (write(pipe_publisher, &resp, sizeof(response)) != sizeof(response)) {
        fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (resp.u_return_code.return_code == -1) {
        close(pipe_publisher);
        return;
    }
    close(pipe_publisher);
    
    mailboxes[box_index].n_publishers = 1;

    
    
    if ((pipe_publisher = open(publisher_pipe_path, O_RDONLY)) == -1) {
        fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    while (true) {
        if (strcmp(mailboxes[box_index].box_name, "") == 0)
            // Box has beem removed by manager
            break;
        if ((ret = read(pipe_publisher, &req, sizeof(request))) != sizeof(request)) {
            if (ret != 0) {
                fprintf(stderr, "[ERR]: read failed: %s\n", strerror(errno));
            }
            // ret == 0 indicates EOF
            break;
        } 
        strcat(req.u_publisher_message.message,"\n");
        bytes_written = tfs_write(fhandle, req.u_publisher_message.message, strlen(req.u_publisher_message.message));
        mailboxes[box_index].box_size += (uint64_t) bytes_written;
        mailboxes[box_index].n_messages += 1;

        // Notifies with condition variable that a new message was written
        mailboxes[box_index].new_message = 1;
        mailboxes[box_index].new_message = 0;
    }
    mailboxes[box_index].n_publishers = 0;
    tfs_close(fhandle);
    close(pipe_server);
}

void register_subscriber(request req) {
    int fhandle = 0;
    int pipe_subscriber = 0;
    unsigned long box_index = mailbox_alloc; // impossible value
    ssize_t ret;
    char message_buffer[MESSAGE_SIZE];
    response resp;

    for (unsigned long i = 0; i < mailbox_alloc; i++) {
        if (free_mailboxes[i] == true)
            continue;
        else {
            if (strcmp(req.u_box_name.box_name, mailboxes[i].box_name) == 0) {
                box_index = i;
                break;
            }
        }
    }
    resp.op_code = OP_CODE_REG_SUBSCRIBER_ANS;

    if (box_index == mailbox_alloc) {
        resp.u_return_code.return_code = -1;
        strcpy(resp.u_response_message.error_message, "subscriber's mailbox does not exist");
    } else {
        char box_name_path[33] = "/";
        strcat(box_name_path, req.u_box_name.box_name);

        if ((fhandle = tfs_open(box_name_path, 0)) == -1) {
            resp.u_return_code.return_code = -1;
            strcpy(resp.u_response_message.error_message, "subscriber could not find the mailbox's file");
        } else {
            resp.u_return_code.return_code = 0;
            memset(resp.u_response_message.error_message, '\0', ERROR_MESSAGE_SIZE);
        }
    }
    if ((pipe_subscriber = open(req.u_client_pipe_path.client_pipe_path, O_WRONLY)) == -1) {
        fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (write(pipe_subscriber, &resp, sizeof(response)) != sizeof(response)) {
            fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
    }
    if (resp.u_return_code.return_code == -1) {
        close(pipe_subscriber);
        return;
    }
    close(pipe_subscriber);

    mailboxes[box_index].n_subscribers += 1;

    if ((pipe_subscriber = open(req.u_client_pipe_path.client_pipe_path, O_WRONLY)) == -1) {
        fprintf(stderr, "[ERR]: sub open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    while (true) {

        ret = tfs_read(fhandle, message_buffer, sizeof(message_buffer)-1);
        if (ret == 0 || ret == -1) {
            // ret == -1 indicates EOF
            break;
        }
        
        // Removing this next line makes the program get stuck and exit mbroker
        // and we have no idea why...
        fprintf(stdout,"%s", message_buffer);

        strcpy(resp.u_response_message.message,message_buffer);
        
        if ((ret = write(pipe_subscriber, &resp, sizeof(response))) != sizeof(response)) {
            fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (ret == 0) {
            // ret == 0 indicates EOF
            break;
        } else if (ret == -1) {
            // ret == -1 indicates error
            fprintf(stderr, "[ERR]: read failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        
    }
    tfs_close(fhandle);
    close(pipe_subscriber);
}

void op_create_mailbox(request req) {
    int pipe_client;
    int fhandle;

    response resp;
    resp.op_code = OP_CODE_CREAT_MAILBOX_ANS;

    bool box_already_exists = false;
    long free_mailbox_index = -1;

    if ((pipe_client = open(req.u_client_pipe_path.client_pipe_path, O_WRONLY)) == -1) {
        fprintf(stderr, "[ERR]: client open failed: %s\n", strerror(errno));
        return;
    }

    for (unsigned long i = 0; i < mailbox_alloc; i++)
        if (free_mailboxes[i] == true) {
            if (free_mailbox_index == -1)
                // free_mailbox_index will be the first free spot
                free_mailbox_index = (long) i;
            continue;
        }
        else if (strcmp(mailboxes[i].box_name, req.u_box_name.box_name) == 0) {
            box_already_exists = true;
            break;
        }
    
    char box_name_path[33] = "/";
    strcat(box_name_path, req.u_box_name.box_name);

    if (box_already_exists) {
        resp.u_return_code.return_code = -1;
        strcpy(resp.u_response_message.error_message, "box already exists");
    }
    else if ((fhandle = tfs_open(box_name_path, TFS_O_CREAT)) == -1) {
        resp.u_return_code.return_code = -1;
        strcpy(resp.u_response_message.error_message, "could not create box");
    } else {
        /* The mailbox's file was successfully created */
        mailbox new_mailbox;
        strcpy(new_mailbox.box_name, req.u_box_name.box_name);
        new_mailbox.box_size = 0;
        new_mailbox.n_subscribers = 0;
        new_mailbox.n_publishers = 0;
        new_mailbox.n_messages = 0;
        new_mailbox.new_message = 0;

        // If there are no free spaces (the array "mailboxes" is full)
        if (free_mailbox_index == -1) {
            // After allocating more, the current value of mailbox_alloc will be
            // the next free spot
            free_mailbox_index = (long) mailbox_alloc;

            // Allocates memory for 10 more
            mailbox_alloc += 10;

            mailbox *temp = realloc(mailboxes, mailbox_alloc * sizeof(mailbox));
            if (temp == NULL) {
                printf("No memory\n");
                return;
            }
            mailboxes = temp;

            bool *temp2 = realloc(free_mailboxes, mailbox_alloc * sizeof(bool));
            if (temp2 == NULL) {
                printf("No memory\n");
                return;
            }
            free_mailboxes = temp2;

            // All new spots are free
            for (unsigned long i = mailbox_alloc - 10; i < mailbox_alloc; i++)
                free_mailboxes[i] = true;
        }

        // Adds the new mailbox to the array
        mailboxes[free_mailbox_index] = new_mailbox;
        free_mailboxes[free_mailbox_index] = false;

        resp.u_return_code.return_code = 0;
        memset(resp.u_response_message.error_message, '\0', ERROR_MESSAGE_SIZE);

        tfs_close(fhandle);
    }

    if (write(pipe_client, &resp, sizeof(response)) != sizeof(response)) {
        fprintf(stderr, "[ERR]: mbroker write failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    close(pipe_client);
}

void op_remove_mailbox(request req) {
    int pipe_client;

    long box_index = -1; // arbitrary value
    bool box_exists = false;

    response resp;
    resp.op_code = OP_CODE_RM_MAILBOX_ANS;

    if ((pipe_client = open(req.u_client_pipe_path.client_pipe_path, O_WRONLY)) == -1) {
        fprintf(stderr, "[ERR]: client open failed: %s\n", strerror(errno));
        return;
    }

    for (unsigned long i = 0; i < mailbox_alloc; i++)
        if (free_mailboxes[i] == true) {
            continue;
        }
        else if (strcmp(mailboxes[i].box_name, req.u_box_name.box_name) == 0) {
            box_index = (long) i;
            box_exists = true;
            break;
        }
    
    char box_name_path[33] = "/";
    strcat(box_name_path, req.u_box_name.box_name);

    if (box_exists == false) {
        resp.u_return_code.return_code = -1;
        strcpy(resp.u_response_message.error_message, "box doesn't exist");
    }
    else if (tfs_unlink(box_name_path) == -1) {
        resp.u_return_code.return_code = -1;
        strcpy(resp.u_response_message.error_message, "could remove box");
    } else {
        /* The mailbox's file was succesfully removed */

        memset(mailboxes[box_index].box_name, '\0', BOX_NAME_SIZE);   
        free_mailboxes[box_index] = true; // index can be used for a new box

        resp.u_return_code.return_code = 0;
        memset(resp.u_response_message.error_message, '\0', ERROR_MESSAGE_SIZE);
    }

    if (write(pipe_client, &resp, sizeof(response)) != sizeof(response)) {
        fprintf(stderr, "[ERR]: mbroker write failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    close(pipe_client);
}

void op_list_mailbox(request req) {
    int pipe_client;

    unsigned long box_amount = 0;
    unsigned long boxes_printed = 0;

    response resp;
    resp.op_code = OP_CODE_LIST_MAILBOX_ANS;

    if ((pipe_client = open(req.u_client_pipe_path.client_pipe_path, O_WRONLY)) == -1) {
        fprintf(stderr, "[ERR]: client open failed: %s\n", strerror(errno));
        return;
    }

    for (unsigned long i = 0; i < mailbox_alloc; i++)
        if (free_mailboxes[i] == false)
            box_amount += 1;

    box_listing listing;
    if (box_amount == 0) { // if there are no boxes
        strcpy(listing.box_name, "");
        listing.last = 1;

        resp.u_box_listing.box_listing = listing;

        if (write(pipe_client, &resp, sizeof(response)) != sizeof(response)) {
            fprintf(stderr, "[ERR]: mbroker write failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    else {
        for (unsigned long i = 0; i < mailbox_alloc; i++) {
            if (boxes_printed == box_amount)
                break;
            if (free_mailboxes[i] == false) { // if there is a box in this index
                if (boxes_printed == box_amount - 1) // checks if this is the last box
                    listing.last = 1;
                else
                    listing.last = 0;
                strcpy(listing.box_name, mailboxes[i].box_name);
                listing.box_size = mailboxes[i].box_size;
                listing.n_publishers = mailboxes[i].n_publishers;
                listing.n_subscribers = mailboxes[i].n_subscribers;
                
                resp.u_box_listing.box_listing = listing;

                if (write(pipe_client, &resp, sizeof(response)) != sizeof(response)) {
                    fprintf(stderr, "[ERR]: mbroker write failed: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }

                boxes_printed += 1;
            }
        }
    }

    close(pipe_client);
}