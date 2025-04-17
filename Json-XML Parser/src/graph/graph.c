#include <stdlib.h>
#include <stdio.h>
#include "graph.h"

struct Graph
{
    // Aucune donnée pour l'instant
};

void free_graph(Graph *g)
{
    free(g);
}

void graph_print(const Graph *g)
{
    (void)g;
    printf("[graph_print stub]\n");
}
