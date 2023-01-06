#include "logging.h"

static int pipe_client;
static int pipe_server;
static int session_id;

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    fprintf(stderr, "usage: sub <register_pipe_name> <box_name>\n");
    WARN("unimplemented"); // TODO: implement
    return -1;
}
