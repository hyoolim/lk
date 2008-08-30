#include <lk/lib.h>
#include "mysql.h"

/* new */
void lk_mysql_init(lk_mysql_t *self, lk_str_t *host, lk_str_t *user, lk_str_t *pw, lk_str_t *db, lk_num_t *port) {
    self->conn = mysql_init(NULL);
    if(mysql_real_connect(self->conn, CSTRING(host), CSTRING(user), CSTRING(pw), CSTRING(db), CNUMBER(port), NULL, 0) == NULL) {
        lk_vm_raisecstr(VM, "Unable to connect to the database: %s", lk_str_new_fromcstr(VM, mysql_error(self->conn)));
    }
}

/* info */
lk_mysql_t *lk_mysql_query(lk_mysql_t *self, lk_str_t *query) {
    self = LK_MYSQL(lk_obj_clone(LK_OBJ(self)));
    self->query = query;
    return self;
}
lk_obj_t *lk_mysql_fetch(lk_mysql_t *self) {
    if(self->result == NULL) {
        darray_t *query = DARRAY(self->query);
        if(mysql_real_query(self->conn, darray_tocstr(query), LIST_COUNT(query)) == 0) {
            self->result = mysql_store_result(self->conn);
            if(self->result != NULL) {
                self->fields = mysql_fetch_fields(self->result);
                self->nfields = mysql_num_fields(self->result);
            }
        } else {
            lk_vm_raisecstr(VM, "Unable to execute query: %s", lk_str_new_fromcstr(VM, mysql_error(self->conn)));
        }
    }
    if(self->result != NULL) {
        MYSQL_ROW fields = mysql_fetch_row(self->result);
        if(fields != NULL) {
            unsigned int i;
            unsigned long *lengths = mysql_fetch_lengths(self->result);
            lk_list_t *row = lk_list_new(VM);
            for(i = 0; i < self->nfields; i ++) {
                darray_pushptr(DARRAY(row), lk_str_new_fromdata(VM, fields[i], lengths[i]));
            }
            return LK_OBJ(row);
        } else {
            mysql_free_result(self->result);
            self->result = NULL;
        }
    }
    return NIL;
}

/* bind all c funcs to lk equiv */
static void alloc_mysql(lk_obj_t *self, lk_obj_t *parent) {
    LK_MYSQL(self)->conn = LK_MYSQL(parent)->conn;
    if(LK_MYSQL(self)->query != NULL) {
        LK_MYSQL(self)->query = LK_STRING(lk_obj_clone(LK_OBJ(LK_MYSQL(parent)->query)));
    }
}
static LK_OBJ_DEFMARKFUNC(mark_mysql) {
    mark(LK_OBJ(LK_MYSQL(self)->query));
}
static void free_mysql(lk_obj_t *self) {
    if(LK_MYSQL(self)->result != NULL) {
        mysql_free_result(LK_MYSQL(self)->result);
    }
    if(LK_MYSQL(self)->conn != NULL) {
        printf("closing\n");
        mysql_close(LK_MYSQL(self)->conn);
    }
}
void lk_mysql_libinit(lk_vm_t *vm) {
    lk_obj_t *obj = vm->t_obj, *str = vm->t_str, *num = vm->t_num;
    lk_obj_t *mysql = lk_obj_alloc_withsize(obj, sizeof(lk_mysql_t));
    lk_obj_setallocfunc(mysql, alloc_mysql);
    lk_obj_setmarkfunc(mysql, mark_mysql);
    lk_obj_setfreefunc(mysql, free_mysql);
    lk_global_set("MySQL", mysql);

    /* new */
    lk_obj_set_cfunc_cvoid(mysql, "init!", lk_mysql_init, str, str, str, str, num, NULL);

    /* info */
    lk_obj_set_cfunc_creturn(mysql, "query", lk_mysql_query, str, NULL);
    lk_obj_set_cfunc_creturn(mysql, "fetch", lk_mysql_fetch, NULL);
}
