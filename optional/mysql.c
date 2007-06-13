#include "mysql.h"
#include "../ext.h"
#define CONN(v) (LK_MYCONN(v)->data)
#define RES(v) (LK_MYQUERY(v)->res)

/* ext map */
static LK_OBJECT_DEFFREEFUNC(free__myconn) {
    if(CONN(self) != NULL) mysql_close(CONN(self));
}
static LK_OBJECT_DEFMARKFUNC(mark__myquery) {
    mark(LK_O(LK_MYQUERY(self)->conn));
    mark(LK_O(LK_MYQUERY(self)->query));
}
static LK_OBJECT_DEFFREEFUNC(free__myquery) {
    if(RES(self) != NULL) mysql_free_result(RES(self));
}
static LK_EXT_DEFCFUNC(init__myconn_str_str_str_str_fi_str) {
    CONN(self) = mysql_init(NULL);
    if(mysql_real_connect(CONN(self),
    /* host   */ pt_list_tocstr(LIST(ARG(0))),
    /* user   */ pt_list_tocstr(LIST(ARG(1))),
    /* passwd */ pt_list_tocstr(LIST(ARG(2))),
    /* db     */ pt_list_tocstr(LIST(ARG(3))),
    /* port   */                 INT(ARG(4)),
    /* socket */ pt_list_tocstr(LIST(ARG(5))),
    /* opts   */ 0
    ) != NULL) RETURN(self);
    else {
        fprintf(stderr, "unable to connect to db");
        abort();
    }
}
static LK_EXT_DEFCFUNC(prepare__myconn_str) {
    lk_myquery_t *q = LK_MYQUERY(lk_object_alloc(
    LK_VM_GETGLOBAL(VM, MySQL_Query)));
    q->conn = LK_MYCONN(self);
    q->query = LK_STRING(ARG(0));
    RETURN(q);
}
static LK_EXT_DEFCFUNC(execute__myquery) {
    MYSQL *conn = CONN(LK_MYQUERY(self)->conn);
    pt_list_t *q = LIST(LK_MYQUERY(self)->query);
    if(RES(self) != NULL) {
        mysql_free_result(RES(self));
        RES(self) = NULL;
    }
    if(mysql_real_query(conn, pt_list_tocstr(q), PT_LIST_COUNT(q)) == 0) {
        RES(self) = mysql_store_result(conn);
        if(RES(self) != NULL) {
            LK_MYQUERY(self)->fields = mysql_fetch_fields(RES(self));
            LK_MYQUERY(self)->fcount = mysql_num_fields(RES(self));
        }
    } else {
        fprintf(stderr, "unable to execute query");
        abort();
    }
    RETURN(self);
}
static LK_EXT_DEFCFUNC(fetch__myquery) {
    if(RES(self) != NULL) {
        MYSQL_ROW row;
        if((row = mysql_fetch_row(RES(self))) != NULL) {
            unsigned int i;
            unsigned long *lens = mysql_fetch_lengths(RES(self));
            lk_list_t *kcrow = lk_list_new(VM);
            for(i = 0; i < LK_MYQUERY(self)->fcount; i ++) {
                pt_list_pushptr(LIST(kcrow),
                lk_string_newfromdata(VM, row[i], lens[i]));
            }
            RETURN(kcrow);
        } else {
            mysql_free_result(RES(self));
            RES(self) = NULL;
            RETURN(N);
        }
    } else {
        fprintf(stderr, "must execute query before fetching");
        abort();
    }
}
LK_VM_DEFGLOBAL(MySQL_Query);
LK_EXT_DEFINIT(lk_mysql_extinit) {
    lk_object_t *obj = vm->t_object, *str = vm->t_string, *fi = vm->t_fi;
    lk_object_t *myconn = lk_object_allocwithsize(obj, sizeof(lk_myconn_t));
    lk_object_t *myquery = lk_object_allocwithsize(obj, sizeof(lk_myquery_t));
    lk_object_setfreefunc(myconn, free__myconn);
    lk_object_setmarkfunc(myquery, mark__myquery);
    lk_object_setfreefunc(myquery, free__myquery);
    lk_ext_global("MySQL", myconn);
    lk_ext_cfunc(myconn, "init", init__myconn_str_str_str_str_fi_str, str, str, str, str, fi, str, NULL);
    lk_ext_cfunc(myconn, "prepare", prepare__myconn_str, str, NULL);
    LK_VM_SETGLOBAL(vm, MySQL_Query, myquery);
    lk_ext_set(myconn, "Query", myquery);
    lk_ext_cfunc(myquery, "execute", execute__myquery, NULL);
    lk_ext_cfunc(myquery, "fetch", fetch__myquery, NULL);
}
