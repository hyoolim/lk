#ifndef LK_SOCKET_H
#define LK_SOCKET_H

/* type */
typedef struct lk_ipaddr lk_ipaddr_t;
typedef struct lk_socket lk_socket_t;
#include "vm.h"
#include "file.h"
#include "string.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
struct lk_socket {
    struct lk_common  obj;
    lk_string_t      *path;
    lk_string_t      *mode;
    int               fd;
    FILE             *in;
    FILE             *out;
};
struct lk_ipaddr {
    struct lk_common obj;
    struct in_addr   addr;
};
#define LK_IPADDR(v) ((lk_ipaddr_t *)(v))
#define LK_SOCKET(v) ((lk_socket_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_socket_extinit);
#endif
