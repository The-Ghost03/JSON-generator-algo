/* graph.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"

Graph *createGraph(int V)
{
    Graph *graph = malloc(sizeof(Graph));
    if (!graph)
    {
        fprintf(stderr, "Erreur d'allocation pour Graph.\n");
        exit(EXIT_FAILURE);
    }
    graph->V = V;
    graph->nodes = malloc(V * sizeof(Node));
    if (!graph->nodes)
    {
        fprintf(stderr, "Erreur d'allocation pour les nodes.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < V; i++)
    {
        graph->nodes[i].id = i + 1;
        graph->nodes[i].name = NULL;
        graph->nodes[i].type = strdup("undefined");
        graph->nodes[i].coordinates[0] = 0.0f;
        graph->nodes[i].coordinates[1] = 0.0f;
        graph->nodes[i].capacity = 0;
        graph->nodes[i].congestion_morning = 1.0f;
        graph->nodes[i].congestion_afternoon = 1.0f;
        graph->nodes[i].congestion_night = 1.0f;
    }
    graph->array = malloc(V * sizeof(AdjList));
    if (!graph->array)
    {
        fprintf(stderr, "Erreur d'allocation pour les listes d'adjacence.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < V; i++)
    {
        graph->array[i].head = NULL;
    }
    return graph;
}

void addEdgeToGraph(Graph *graph, int src, int dest, EdgeAttr attr)
{
    if (src < 0 || src >= graph->V || dest < 0 || dest >= graph->V)
    {
        fprintf(stderr, "Indices de nœud invalides dans addEdgeToGraph.\n");
        return;
    }
    AdjListNode *newNode = malloc(sizeof(AdjListNode));
    if (!newNode)
    {
        fprintf(stderr, "Erreur d'allocation pour AdjListNode.\n");
        exit(EXIT_FAILURE);
    }
    newNode->dest = dest;
    newNode->attr = attr;
    newNode->next = graph->array[src].head;
    graph->array[src].head = newNode;
}

void removeEdgeFromGraph(Graph *graph, int src, int dest)
{
    if (src < 0 || src >= graph->V)
        return;
    AdjListNode *curr = graph->array[src].head;
    AdjListNode *prev = NULL;
    while (curr)
    {
        if (curr->dest == dest)
        {
            if (prev == NULL)
                graph->array[src].head = curr->next;
            else
                prev->next = curr->next;
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

void addNode(Graph **graphPtr, const char *name, float cong_morning, float cong_afternoon, float cong_night)
{
    Graph *graph = *graphPtr;
    int newV = graph->V + 1;
    Node *newNodes = realloc(graph->nodes, newV * sizeof(Node));
    if (!newNodes)
    {
        fprintf(stderr, "Erreur de réallocation pour les nodes.\n");
        return;
    }
    graph->nodes = newNodes;
    AdjList *newAdj = realloc(graph->array, newV * sizeof(AdjList));
    if (!newAdj)
    {
        fprintf(stderr, "Erreur de réallocation pour les listes d'adjacence.\n");
        return;
    }
    graph->array = newAdj;
    graph->nodes[newV - 1].id = newV;
    graph->nodes[newV - 1].name = strdup(name);
    graph->nodes[newV - 1].type = strdup("undefined");
    graph->nodes[newV - 1].coordinates[0] = 0.0f;
    graph->nodes[newV - 1].coordinates[1] = 0.0f;
    graph->nodes[newV - 1].capacity = 0;
    graph->nodes[newV - 1].congestion_morning = cong_morning;
    graph->nodes[newV - 1].congestion_afternoon = cong_afternoon;
    graph->nodes[newV - 1].congestion_night = cong_night;
    graph->array[newV - 1].head = NULL;
    graph->V = newV;
}

void removeNode(Graph *graph, int node)
{
    if (node < 0 || node >= graph->V)
        return;
    AdjListNode *curr = graph->array[node].head;
    while (curr)
    {
        AdjListNode *temp = curr;
        curr = curr->next;
        free(temp);
    }
    graph->array[node].head = NULL;
    for (int i = 0; i < graph->V; i++)
    {
        if (i == node)
            continue;
        removeEdgeFromGraph(graph, i, node);
    }
    free(graph->nodes[node].name);
    free(graph->nodes[node].type);
    graph->nodes[node].name = NULL;
}

void freeGraph(Graph *graph)
{
    if (!graph)
        return;
    for (int i = 0; i < graph->V; i++)
    {
        AdjListNode *cur = graph->array[i].head;
        while (cur)
        {
            AdjListNode *temp = cur;
            cur = cur->next;
            free(temp);
        }
        if (graph->nodes[i].name)
        {
            free(graph->nodes[i].name);
            free(graph->nodes[i].type);
        }
    }
    free(graph->nodes);
    free(graph->array);
    free(graph);
}
