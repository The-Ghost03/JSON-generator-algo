/* graph.c */
#include "graph.h"
#include <stdlib.h>
#include <stdio.h>

/* Crée et initialise un graphe vide à V sommets */
Graph *create_graph(int V)
{
    if (V <= 0)
        return NULL;
    Graph *g = malloc(sizeof(*g));
    if (!g)
        return NULL;
    g->V = V;

    /* allouer la matrice des distances */
    g->dist = malloc(V * sizeof(*g->dist));
    if (!g->dist)
    {
        free(g);
        return NULL;
    }
    for (int i = 0; i < V; i++)
    {
        g->dist[i] = malloc(V * sizeof(*g->dist[i]));
        if (!g->dist[i])
        {
            /* rollback en cas d'erreur */
            for (int k = 0; k < i; k++)
                free(g->dist[k]);
            free(g->dist);
            free(g);
            return NULL;
        }
    }

    /* allouer la matrice des attributs */
    g->attrs = malloc(V * sizeof(*g->attrs));
    if (!g->attrs)
    {
        for (int i = 0; i < V; i++)
            free(g->dist[i]);
        free(g->dist);
        free(g);
        return NULL;
    }
    for (int i = 0; i < V; i++)
    {
        g->attrs[i] = calloc(V, sizeof(*g->attrs[i]));
        if (!g->attrs[i])
        {
            /* rollback */
            for (int k = 0; k < i; k++)
                free(g->attrs[k]);
            free(g->attrs);
            for (int k = 0; k < V; k++)
                free(g->dist[k]);
            free(g->dist);
            free(g);
            return NULL;
        }
    }

    /* init dist : 0 sur diag, INF ailleurs */
    for (int i = 0; i < V; i++)
    {
        for (int j = 0; j < V; j++)
        {
            g->dist[i][j] = (i == j ? 0.0f : INF);
            /* attrs[i][j] laissé à zero / calloc */
        }
    }

    return g;
}

/* Ajoute ou met à jour l’arête u → v */
void add_edge(Graph *g, int u, int v, const EdgeAttr *attr)
{
    if (!g || u < 0 || v < 0 || u >= g->V || v >= g->V || !attr)
        return;
    g->dist[u][v] = attr->distance;
    g->attrs[u][v] = *attr; /* copie du struct */
}

/* Accès à la distance u → v */
float get_edge_distance(const Graph *g, int u, int v)
{
    if (!g || u < 0 || v < 0 || u >= g->V || v >= g->V)
        return INF;
    return g->dist[u][v];
}

/* Accès aux attributs u → v */
const EdgeAttr *get_edge_attr(const Graph *g, int u, int v)
{
    if (!g || u < 0 || v < 0 || u >= g->V || v >= g->V)
        return NULL;
    return &g->attrs[u][v];
}

/* Libère toute la mémoire associée */
void free_graph(Graph *g)
{
    if (!g)
        return;
    for (int i = 0; i < g->V; i++)
    {
        free(g->dist[i]);
        free(g->attrs[i]);
    }
    free(g->dist);
    free(g->attrs);
    free(g);
}
