#include "postgres.h"
#include "../ext.h"
#include "../dict.h"
#define CONN(v) (LK_PGCONN(v)->data)
#define RES(v) (LK_PGQUERY(v)->res)

/* ext map */
static LK_OBJECT_DEFFREEFUNC(free__pgconn) {
    if(CONN(self) != NULL) PQfinish(CONN(self));
    CONN(self) = NULL;
}
static LK_EXT_DEFCFUNC(init__pgconn_str) {
    CONN(self) = PQconnectdb(pt_list_tocstr(LIST(ARG(0))));
    if(CONN(self) != NULL) {
        RETURN(self);
    } else {
        fprintf(stderr, "unable to connect to db");
        abort();
    }
}
static LK_OBJECT_DEFMARKFUNC(mark__pgquery) {
    mark(LK_O(LK_PGQUERY(self)->conn));
    mark(LK_O(LK_PGQUERY(self)->query));
}
static LK_OBJECT_DEFFREEFUNC(free__pgquery) {
    if(RES(self) != NULL) PQclear(RES(self));
    RES(self) = NULL;
}
static LK_EXT_DEFCFUNC(prepare__pgconn_str) {
    lk_pgquery_t *q = LK_PGQUERY(lk_object_alloc(
        LK_VM_GETGLOBAL(VM, PostgreSQL_Query)));
    q->conn = LK_PGCONN(self);
    q->query = LK_STRING(ARG(0));
    q->prepared = 1;
    q->fcount = 0;
    q->tcount = 0;
    q->tcurrent = 0;
    PQprepare(CONN(self), "", pt_list_tocstr(LIST(q->query)), 0, NULL);
    RETURN(q);
}
static LK_EXT_DEFCFUNC(query__pgconn_str) {
    lk_pgquery_t *q = LK_PGQUERY(lk_object_alloc(
        LK_VM_GETGLOBAL(VM, PostgreSQL_Query)));
    q->conn = LK_PGCONN(self);
    q->query = LK_STRING(ARG(0));
    q->prepared = 0;
    q->fcount = 0;
    q->tcount = 0;
    q->tcurrent = 0;
    RES(q) = NULL;
    RETURN(q);
}
static LK_EXT_DEFCFUNC(execute__pgquery) {
    PGconn *conn = CONN(LK_PGQUERY(self)->conn);
    pt_list_t *q = LIST(LK_PGQUERY(self)->query);
    if(env->argc > 0) {
        int h, c = env->argc;
        char **params = pt_memory_alloc(sizeof(char *) * (c + 1));
        for(h = 0; h < c; h++) {
            params[h] = (char *)pt_list_tocstr(LIST(ARG(h)));
        }
        if((LK_PGQUERY(self)->prepared) == 0) {
            RES(self) = PQexecParams(conn, pt_list_tocstr(q), c, NULL,
                    (const char * const *) params, NULL, NULL, 0);
        } else {
            RES(self) = PQexecPrepared(conn, "", c,
                    (const char * const *)params, NULL, NULL, 0);
        }
        pt_memory_free(params);
    } else {
        if((LK_PGQUERY(self)->prepared) == 0) {
            RES(self) = PQexec(conn, pt_list_tocstr(q));
        } else {
            RES(self) = PQexecPrepared(conn, "", 0,
                    NULL, NULL, NULL, 0);
        }
    }
    if(RES(self) != NULL ) {
        switch(PQresultStatus(RES(self))) {
            case PGRES_COMMAND_OK:
                LK_PGQUERY(self)->tcount = 0;
                LK_PGQUERY(self)->tcurrent = 0;
                LK_PGQUERY(self)->fcount = 0;
                break;
            case PGRES_TUPLES_OK:
                LK_PGQUERY(self)->tcount = PQntuples(RES(self));
                LK_PGQUERY(self)->tcurrent = 0;
                LK_PGQUERY(self)->fcount = PQnfields(RES(self));
                break;
            default:
                fprintf(stderr, "%s: %s", PQresStatus(PQresultStatus(RES(self))),
                    PQresultErrorMessage(RES(self)));
                abort();
                break;
        }
    } else {
        fprintf(stderr, "%s: unable to execute query",
            PQresStatus(PGRES_FATAL_ERROR));
        abort();
    }
    RETURN(self);
}
static LK_EXT_DEFCFUNC(fetch__pgquery) {
    if(RES(self) == NULL) {
        PGconn *conn = CONN(LK_PGQUERY(self)->conn);
        pt_list_t *q = LIST(LK_PGQUERY(self)->query);
        if((LK_PGQUERY(self)->prepared) == 0) {
            RES(self) = PQexec(conn, pt_list_tocstr(q));
        } else {
            RES(self) = PQexecPrepared(conn, "", 0,
                    NULL, NULL, NULL, 0);
        }
        if(RES(self) != NULL ) {
            switch(PQresultStatus(RES(self))) {
                case PGRES_COMMAND_OK:
                    LK_PGQUERY(self)->tcount = 0;
                    LK_PGQUERY(self)->tcurrent = 0;
                    LK_PGQUERY(self)->fcount = 0;
                    break;
                case PGRES_TUPLES_OK:
                    LK_PGQUERY(self)->tcount = PQntuples(RES(self));
                    LK_PGQUERY(self)->tcurrent = 0;
                    LK_PGQUERY(self)->fcount = PQnfields(RES(self));
                    break;
                default:
                    fprintf(stderr, "%s: %s", PQresStatus(PQresultStatus(RES(self))),
                        PQresultErrorMessage(RES(self)));
                    abort();
                    break;
            }
        } else {
            fprintf(stderr, "%s: unable to execute query",
                PQresStatus(PGRES_FATAL_ERROR));
            abort();
        }
    }
    if( LK_PGQUERY(self)->tcurrent < LK_PGQUERY(self)->tcount ) {
        int h = LK_PGQUERY(self)->tcurrent;
        int i;
        lk_dict_t *kcrow = lk_dict_new(VM);
        for(i = 0; i < LK_PGQUERY(self)->fcount; i++ ) {
            lk_dict_setbycstr( LK_DICT(kcrow),
                (const char *)PQfname( RES(self), i ),
                LK_O(lk_string_newfromdata( VM, 
                    (const char *)PQgetvalue( RES(self), h, i ),
                    PQgetlength( RES(self), h, i )
                ))
            );
        }
        LK_PGQUERY(self)->tcurrent++;
        RETURN(kcrow);
    } else {
        PQclear(RES(self));
        RES(self) = NULL;
        RETURN(N);
    }
}
LK_VM_DEFGLOBAL(PostgreSQL_Query);
LK_EXT_DEFINIT(lk_postgres_extinit) {
    lk_object_t *obj = vm->t_object, *str = vm->t_string, *fi = vm->t_fi;
    lk_object_t *pgconn = lk_object_allocwithsize(obj, sizeof(lk_pgconn_t));
    lk_object_t *pgquery = lk_object_allocwithsize(obj, sizeof(lk_pgquery_t));
    lk_object_setfreefunc(pgconn, free__pgconn);
    lk_object_setmarkfunc(pgquery, mark__pgquery);
    lk_object_setfreefunc(pgquery, free__pgquery);
    lk_ext_global("PostgreSQL", pgconn);
    lk_ext_cfunc1(pgconn, "init", init__pgconn_str, str);
    lk_ext_cfunc1(pgconn, "query", query__pgconn_str, str);
    lk_ext_cfunc1(pgconn, "prepare", prepare__pgconn_str, str);
    LK_VM_SETGLOBAL(vm, PostgreSQL_Query, pgquery);
    lk_ext_set(pgconn, "Query", pgquery);
    lk_ext_cfuncany(pgquery, "execute", execute__pgquery);
    lk_ext_cfunc0(pgquery, "fetch", fetch__pgquery);
}
