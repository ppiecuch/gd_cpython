char *PYTHONCASEOK = 0;

#include <string.h>
#include <stdlib.h>


int __putenv(const char *v)
{
    if (strcmp(v, "PYTHONCASEOK") == 0) {
        if(PYTHONCASEOK) free(PYTHONCASEOK);
        PYTHONCASEOK = calloc(1,1);
        return 1;
    }
    return putenv(v);
}

const char *__getenv(const char *v)
{
    if (strcmp(v, "PYTHONCASEOK") == 0 && PYTHONCASEOK) return PYTHONCASEOK;
    return getenv(v);
}
