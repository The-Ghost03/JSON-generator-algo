#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "io_utils.h"

char *read_file(const char *filename)
{
    (void)filename;
    // stub : pour l’instant, on retourne NULL pour toujours
    return NULL;
}

const char *skip_whitespace(const char *s)
{
    while (*s && isspace((unsigned char)*s))
        s++;
    return s;
}
