#include "scenario.h"
#include <stdlib.h>
#include <stdio.h>

struct Scenario
{
    // vide
};

Scenario *load_scenario(const char *name)
{
    (void)name;
    return NULL;
}

void apply_scenario(Scenario *sc, Graph *g)
{
    (void)sc;
    (void)g;
    printf("[apply_scenario stub]\n");
}

void free_scenario(Scenario *sc)
{
    free(sc);
}
