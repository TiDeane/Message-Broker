#include "common.h"

void send_msg(int pipe, char const *msg) {
    size_t len = strlen(msg);
    size_t written = 0;

    while (written < len) {
        ssize_t ret = write(pipe, msg + written, len - written);
        if (ret < 0) {
            fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        written += (size_t) ret;
    }
}
