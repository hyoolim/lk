#ifndef LK_PQSQL_H
#define LK_PQSQL_H
#include "../vm.h"
#include <libpq-fe.h>
#include <libpq/libpq-fs.h>

/* type */
typedef struct lk_pgconn {
    struct lk_common  co;
    PGconn            *data;
} lk_pgconn_t;
#define LK_PGCONN(v) ((lk_pgconn_t *)(v))
typedef struct lk_pgquery {
    struct lk_common  co;
    lk_pgconn_t      *conn;
    lk_string_t      *query;
    PGresult         *res;
    char              prepared;
    int               fcount;
    int               tcount;
    int               tcurrent;
} lk_pgquery_t;
#define LK_PGQUERY(v) ((lk_pgquery_t *)(v))

/* ext map */
LK_VM_DEFGLOBAL_PROTO(PostgreSQL_Query);
LK_EXT_DEFINIT(lk_postgres_extinit);
#endif
