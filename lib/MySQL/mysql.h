#ifndef LK_MYSQL_H
#define LK_MYSQL_H
#include <lk/types.h>
#include <mysql.h>

/* type */
typedef struct {
    struct lk_common  o;
    MYSQL            *conn;
    lk_str_t         *query;
    MYSQL_RES        *result;
    MYSQL_FIELD      *fields;
    unsigned int      nfields;
} lk_mysql_t;
#define LK_MYSQL(obj) ((lk_mysql_t *)(obj))

/* init */
void lk_mysql_libinit(lk_vm_t *vm);
#endif
