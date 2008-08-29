#include "socket.h"
#include "lib.h"
#include "num.h"
#include "list.h"
#define IPADDR (LK_IPADDR(self))
#define SOCKET (LK_SOCKET(self))

/* non-ansi */
FILE *fdopen(int fildes, const char *mode);
int inet_aton(const char *cp, struct in_addr *pin);

/* ext map - ip addr */
static void alloc_ip_str(lk_obj_t *self, lk_scope_t *local) {
    inet_aton(darray_tocstr(DARRAY(ARG(0))), &IPADDR->addr);
    RETURN(self);
}
static void to_str_ip(lk_obj_t *self, lk_scope_t *local) {
    RETURN(lk_str_new_fromcstr(VM, inet_ntoa(IPADDR->addr)));
}

/* ext map - socket */
static void alloc_sock(lk_obj_t *self, lk_obj_t *parent) {
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
static void free_sock(lk_obj_t *self) {
    if(LK_SOCKET(self)->in != NULL) fclose(LK_SOCKET(self)->in);
    if(LK_SOCKET(self)->out != NULL) fclose(LK_SOCKET(self)->out);
}
static void acce_sock(lk_obj_t *self, lk_scope_t *local) {
    struct sockaddr remote;
    socklen_t len = sizeof(struct sockaddr);
    lk_socket_t *conn = LK_SOCKET(lk_obj_alloc(VM->t_socket));
    conn->fd = accept(SOCKET->fd, &remote, &len);
    conn->in = fdopen(conn->fd, "r");
    conn->out = fdopen(conn->fd, "w");
    RETURN(conn);
}
static void bind_sock_ip_num(lk_obj_t *self, lk_scope_t *local) {
    struct sockaddr_in my;
    my.sin_family = AF_INET;
    my.sin_port = htons((uint16_t)CNUMBER(ARG(1)));
    my.sin_addr = LK_IPADDR(ARG(0))->addr;
    memset(&(my.sin_zero), 0x0, 8);
    if(bind(SOCKET->fd,
    (struct sockaddr *)&my, sizeof(struct sockaddr)) == -1) {
        lk_vm_raisecstr(VM, "Cannot bind");
    }
    RETURN(self);
}
static void connect_sock_ip_num(lk_obj_t *self, lk_scope_t *local) {
    struct sockaddr_in remote;
    remote.sin_family = AF_INET;
    remote.sin_port = htons((uint16_t)CNUMBER(ARG(1)));
    remote.sin_addr = LK_IPADDR(ARG(0))->addr;
    memset(&(remote.sin_zero), 0x0, 8);
    if(connect(SOCKET->fd,
    (struct sockaddr *)&remote, sizeof(struct sockaddr)) == -1) {
        lk_vm_raisecstr(VM, "Cannot connect");
    }
    RETURN(self);
}
static void listen_sock(lk_obj_t *self, lk_scope_t *local) {
    if(listen(SOCKET->fd, 10) == -1) {
        lk_vm_raisecstr(VM, "Cannot listen");
    }
    RETURN(self);
}
void lk_socket_extinit(lk_vm_t *vm) {
    lk_obj_t *obj = vm->t_obj, *str = vm->t_str, *num = vm->t_num;
    lk_obj_t *ip = lk_obj_alloc_withsize(obj, sizeof(lk_ipaddr_t));
    lk_obj_t *sock = lk_obj_alloc_withsize(obj, sizeof(lk_socket_t));
    lk_obj_setallocfunc(sock, alloc_sock);
    lk_obj_setfreefunc(sock, free_sock);
    /* */
    lk_global_set("IpAddress", ip);
    lk_object_set(ip, "ANY", lk_obj_alloc(ip));
    lk_obj_set_cfunc_lk(ip, "init!", alloc_ip_str, str, NULL);
    lk_obj_set_cfunc_lk(ip, "toString", to_str_ip, NULL);
    /* */
    lk_global_set("Socket", vm->t_socket = sock);
    lk_obj_set_cfunc_lk(sock, "accept", acce_sock, NULL);
    lk_obj_set_cfunc_lk(sock, "bind", bind_sock_ip_num, ip, num, NULL);
    lk_obj_set_cfunc_lk(sock, "connect", connect_sock_ip_num, ip, num, NULL);
    lk_obj_set_cfunc_lk(sock, "listen", listen_sock, NULL);
}
