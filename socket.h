#ifndef LK_SOCKET_H
#define LK_SOCKET_H

/* type */
typedef struct lk_Ipaddr lk_Ipaddr_t;
typedef struct lk_Socket lk_Socket_t;
#include "vm.h"
#include "file.h"
#include "string.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
struct lk_Socket {
    struct lk_Common  obj;
    lk_String_t      *path;
    lk_String_t      *mode;
    int               fd;
    FILE             *in;
    FILE             *out;
};
struct lk_Ipaddr {
    struct lk_Common obj;
    struct in_addr   addr;
};
#define LK_IPADDR(v) ((lk_Ipaddr_t *)(v))
#define LK_SOCKET(v) ((lk_Socket_t *)(v))

/* ext map */
LK_EXT_DEFINIT(lk_Socket_extinit);
#endif
