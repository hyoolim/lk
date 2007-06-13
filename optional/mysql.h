#ifndef LK_MYSQL_H
#define LK_MYSQL_H
#include "../vm.h"
#include <mysql.h>

/* type */
typedef struct lk_myconn {
    struct lk_common  co;
    MYSQL            *data;
} lk_myconn_t;
#define LK_MYCONN(v) ((lk_myconn_t *)(v))
typedef struct lk_myquery {
    struct lk_common  co;
    lk_myconn_t      *conn;
    lk_string_t      *query;
    MYSQL_RES        *res;
    MYSQL_FIELD      *fields;
    unsigned int      fcount;
} lk_myquery_t;
#define LK_MYQUERY(v) ((lk_myquery_t *)(v))

/* ext map */
LK_VM_DEFGLOBAL_PROTO(MySQL_Query);
LK_EXT_DEFINIT(lk_mysql_extinit);
#endif
