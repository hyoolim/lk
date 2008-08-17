#include "socket.h"
#include "ext.h"
#include "fixnum.h"
#include "list.h"
#define IPADDR (LK_IPADDR(self))
#define SOCKET (LK_SOCKET(self))

/* non-ansi */
FILE *fdopen(int fildes, const char *mode);
int inet_aton(const char *cp, struct in_addr *pin);

/* ext map - ip addr */
LK_LIBRARY_DEFINECFUNCTION(alloc__ip_str) {
    inet_aton(darray_toCString(LIST(ARG(0))), &IPADDR->addr);
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(to_string__ip) {
    RETURN(lk_string_newfromcstr(VM, inet_ntoa(IPADDR->addr)));
}

/* ext map - socket */
static LK_OBJ_DEFALLOCFUNC(alloc__sock) {
    int yes = 1;
    SOCKET->fd = socket(AF_INET, SOCK_STREAM, 0);
    SOCKET->in = fdopen(SOCKET->fd, "r");
    SOCKET->out = fdopen(SOCKET->fd, "w");
    if(setsockopt(SOCKET->fd,
    SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }
}
static LK_OBJ_DEFFREEFUNC(free__sock) {
    if(LK_SOCKET(self)->in != NULL) fclose(LK_SOCKET(self)->in);
    if(LK_SOCKET(self)->out != NULL) fclose(LK_SOCKET(self)->out);
}
LK_LIBRARY_DEFINECFUNCTION(acce_sock) {
    struct sockaddr remote;
    socklen_t len = sizeof(struct sockaddr);
    lk_socket_t *conn = LK_SOCKET(lk_object_alloc(VM->t_socket));
    conn->fd = accept(SOCKET->fd, &remote, &len);
    conn->in = fdopen(conn->fd, "r");
    conn->out = fdopen(conn->fd, "w");
    RETURN(conn);
}
LK_LIBRARY_DEFINECFUNCTION(bind__sock_ip_fi) {
    struct sockaddr_in my;
    my.sin_family = AF_INET;
    my.sin_port = htons((uint16_t)INT(ARG(1)));
    my.sin_addr = LK_IPADDR(ARG(0))->addr;
    memset(&(my.sin_zero), 0x0, 8);
    if(bind(SOCKET->fd,
    (struct sockaddr *)&my, sizeof(struct sockaddr)) == -1) {
        lk_vm_raisecstr(VM, "Cannot bind");
    }
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(connect__sock_ip_fi) {
    struct sockaddr_in remote;
    remote.sin_family = AF_INET;
    remote.sin_port = htons((uint16_t)INT(ARG(1)));
    remote.sin_addr = LK_IPADDR(ARG(0))->addr;
    memset(&(remote.sin_zero), 0x0, 8);
    if(connect(SOCKET->fd,
    (struct sockaddr *)&remote, sizeof(struct sockaddr)) == -1) {
        lk_vm_raisecstr(VM, "Cannot connect");
    }
    RETURN(self);
}
LK_LIBRARY_DEFINECFUNCTION(listen__sock) {
    if(listen(SOCKET->fd, 10) == -1) {
        lk_vm_raisecstr(VM, "Cannot listen");
    }
    RETURN(self);
}
LK_EXT_DEFINIT(lk_socket_extinit) {
    lk_object_t *obj = vm->t_obj, *str = vm->t_string, *fi = vm->t_fi;
    lk_object_t *ip = lk_object_allocwithsize(obj, sizeof(lk_ipaddr_t));
    lk_object_t *sock = lk_object_allocwithsize(obj, sizeof(lk_socket_t));
    lk_object_setallocfunc(sock, alloc__sock);
    lk_object_setfreefunc(sock, free__sock);
    /* */
    lk_library_setGlobal("IpAddress", ip);
    lk_library_set(ip, "ANY", lk_object_alloc(ip));
    lk_library_setCFunction(ip, "init", alloc__ip_str, str, NULL);
    lk_library_setCFunction(ip, "toString", to_string__ip, NULL);
    /* */
    lk_library_setGlobal("Socket", vm->t_socket = sock);
    lk_library_setCFunction(sock, "accept", acce_sock, NULL);
    lk_library_setCFunction(sock, "bind", bind__sock_ip_fi, ip, fi, NULL);
    lk_library_setCFunction(sock, "connect", connect__sock_ip_fi, ip, fi, NULL);
    lk_library_setCFunction(sock, "listen", listen__sock, NULL);
}
