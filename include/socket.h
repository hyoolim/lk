#ifndef LK_SOCKET_H
#define LK_SOCKET_H
#include "types.h"

/* type */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
struct lk_socket {
    struct lk_common o;
    lk_str_t *path;
    lk_str_t *mode;
    int fd;
    FILE *in;
    FILE *out;
};
struct lk_ipaddr {
    struct lk_common o;
    struct in_addr addr;
};

/* ext map */
void lk_socket_extinit(lk_vm_t *vm);
#endif
