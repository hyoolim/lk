#include "socket.h"
#include "lib.h"
#include "list.h"
#include "num.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

// non-ansi
FILE *fdopen(int fildes, const char *mode);
int inet_aton(const char *cp, struct in_addr *pin);

// helpers
static void close_sock_file(void *ptr) {
    fclose((FILE *)ptr);
}

static void sock_setup_from_fd(lk_obj_t *self, int fd) {
    lk_vm_t *vm = LK_VM(self);

    lk_obj_set_slot_by_cstr(self, "fd", NULL, LK_OBJ(lk_num_new(vm, fd)));
    lk_obj_set_slot_by_cstr(self, "in", NULL, LK_OBJ(lk_cptr_new(vm, fdopen(fd, "r"), close_sock_file)));
    lk_obj_set_slot_by_cstr(self, "out", NULL, LK_OBJ(lk_cptr_new(vm, fdopen(fd, "w"), close_sock_file)));
}

static int sock_fd(lk_obj_t *self) {
    return (int)CNUMBER(lk_obj_get_value_by_cstr(self, "fd"));
}

// ext map - ip addr
static void alloc_ip_str(lk_obj_t *self, lk_scope_t *local) {
    lk_vm_t *vm = LK_VM(self);
    struct in_addr *addr = mem_alloc(sizeof(struct in_addr));

    inet_aton(vec_str_tocstr(VEC(ARG(0))), addr);
    lk_obj_set_slot_by_cstr(self, "addr", NULL, LK_OBJ(lk_cptr_new(vm, addr, mem_free)));
    RETURN(self);
}

static void to_str_ip(lk_obj_t *self, lk_scope_t *local) {
    lk_cptr_t *cptr = LK_CPTR(lk_obj_get_value_by_cstr(self, "addr"));

    RETURN(lk_str_new_from_cstr(VM, inet_ntoa(*(struct in_addr *)cptr->ptr)));
}

// ext map - socket
static void init_sock(lk_obj_t *self, lk_scope_t *local) {
    int yes = 1;
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd == -1) {
        perror("socket");
        exit(1);
    }

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    sock_setup_from_fd(self, fd);
    RETURN(self);
}

static void acce_sock(lk_obj_t *self, lk_scope_t *local) {
    struct sockaddr remote;
    socklen_t len = sizeof(struct sockaddr);
    int fd = accept(sock_fd(self), &remote, &len);
    lk_obj_t *conn = lk_obj_alloc(VM->t_socket);

    if (fd == -1)
        lk_vm_raise_cstr(VM, "Cannot accept");

    sock_setup_from_fd(conn, fd);
    RETURN(conn);
}

static void bind_sock_ip_num(lk_obj_t *self, lk_scope_t *local) {
    struct sockaddr_in my;
    lk_cptr_t *cptr = LK_CPTR(lk_obj_get_value_by_cstr(ARG(0), "addr"));

    my.sin_family = AF_INET;
    my.sin_port = htons((uint16_t)CNUMBER(ARG(1)));
    my.sin_addr = *(struct in_addr *)cptr->ptr;
    memset(&(my.sin_zero), 0x0, 8);

    if (bind(sock_fd(self), (struct sockaddr *)&my, sizeof(struct sockaddr)) == -1)
        lk_vm_raise_cstr(VM, "Cannot bind");

    RETURN(self);
}

static void connect_sock_ip_num(lk_obj_t *self, lk_scope_t *local) {
    struct sockaddr_in remote;
    lk_cptr_t *cptr = LK_CPTR(lk_obj_get_value_by_cstr(ARG(0), "addr"));

    remote.sin_family = AF_INET;
    remote.sin_port = htons((uint16_t)CNUMBER(ARG(1)));
    remote.sin_addr = *(struct in_addr *)cptr->ptr;
    memset(&(remote.sin_zero), 0x0, 8);

    if (connect(sock_fd(self), (struct sockaddr *)&remote, sizeof(struct sockaddr)) == -1)
        lk_vm_raise_cstr(VM, "Cannot connect");

    RETURN(self);
}

static void listen_sock(lk_obj_t *self, lk_scope_t *local) {
    if (listen(sock_fd(self), 10) == -1)
        lk_vm_raise_cstr(VM, "Cannot listen");

    RETURN(self);
}

void lk_socket_ext_init(lk_vm_t *vm) {
    lk_obj_t *obj = vm->t_obj, *str = vm->t_str, *num = vm->t_num;
    lk_obj_t *ip = lk_obj_alloc_type(obj, sizeof(lk_obj_t));
    lk_obj_t *sock = lk_obj_alloc_type(obj, sizeof(lk_obj_t));
    lk_obj_t *any = lk_obj_alloc(ip);
    struct in_addr *zero_addr = mem_alloc(sizeof(struct in_addr));

    memset(zero_addr, 0, sizeof(struct in_addr));
    lk_obj_set_slot_by_cstr(any, "addr", NULL, LK_OBJ(lk_cptr_new(vm, zero_addr, mem_free)));
    lk_global_set("IpAddress", ip);
    lk_object_set(ip, "ANY", any);
    lk_obj_set_cfunc_lk(ip, "init!", alloc_ip_str, str, NULL);
    lk_obj_set_cfunc_lk(ip, "toString", to_str_ip, NULL);

    lk_global_set("Socket", vm->t_socket = sock);
    lk_obj_set_cfunc_lk(sock, "init!", init_sock, NULL);
    lk_obj_set_cfunc_lk(sock, "accept", acce_sock, NULL);
    lk_obj_set_cfunc_lk(sock, "bind", bind_sock_ip_num, ip, num, NULL);
    lk_obj_set_cfunc_lk(sock, "connect", connect_sock_ip_num, ip, num, NULL);
    lk_obj_set_cfunc_lk(sock, "listen", listen_sock, NULL);
}
