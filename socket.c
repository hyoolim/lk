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
static LK_EXT_DEFCFUNC(alloc__ip_str) {
    inet_aton(list_tocstr(LIST(ARG(0))), &IPADDR->addr);
    RETURN(self);
}
static LK_EXT_DEFCFUNC(to_string__ip) {
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
static LK_EXT_DEFCFUNC(acce_sock) {
    struct sockaddr remote;
    socklen_t len = sizeof(struct sockaddr);
    lk_socket_t *conn = LK_SOCKET(lk_obj_alloc(VM->t_socket));
    conn->fd = accept(SOCKET->fd, &remote, &len);
    conn->in = fdopen(conn->fd, "r");
    conn->out = fdopen(conn->fd, "w");
    RETURN(conn);
}
static LK_EXT_DEFCFUNC(bind__sock_ip_fi) {
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
static LK_EXT_DEFCFUNC(connect__sock_ip_fi) {
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
static LK_EXT_DEFCFUNC(listen__sock) {
    if(listen(SOCKET->fd, 10) == -1) {
        lk_vm_raisecstr(VM, "Cannot listen");
    }
    RETURN(self);
}
LK_EXT_DEFINIT(lk_socket_extinit) {
    lk_obj_t *obj = vm->t_obj, *str = vm->t_string, *fi = vm->t_fi;
    lk_obj_t *ip = lk_obj_allocwithsize(obj, sizeof(lk_ipaddr_t));
    lk_obj_t *sock = lk_obj_allocwithsize(obj, sizeof(lk_socket_t));
    lk_obj_setallocfunc(sock, alloc__sock);
    lk_obj_setfreefunc(sock, free__sock);
    /* */
    lk_ext_global("IpAddress", ip);
    lk_ext_set(ip, "ANY", lk_obj_alloc(ip));
    lk_ext_cfunc(ip, "init", alloc__ip_str, str, NULL);
    lk_ext_cfunc(ip, "to string", to_string__ip, NULL);
    /* */
    lk_ext_global("Socket", vm->t_socket = sock);
    lk_ext_cfunc(sock, "accept", acce_sock, NULL);
    lk_ext_cfunc(sock, "bind", bind__sock_ip_fi, ip, fi, NULL);
    lk_ext_cfunc(sock, "connect", connect__sock_ip_fi, ip, fi, NULL);
    lk_ext_cfunc(sock, "listen", listen__sock, NULL);
}
