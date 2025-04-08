/* graph_applications.c */
#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "graph_applications.h"

/* --- Détection de cycle (graphe non orienté) --- */
static int DFSUtilDetectCycle(Graph *graph, int v, int parent, int *visited)
{
    visited[v] = 1;
    AdjListNode *adj = graph->array[v].head;
    while (adj)
    {
        int w = adj->dest;
        if (!visited[w])
        {
            if (DFSUtilDetectCycle(graph, w, v, visited))
                return 1;
        }
        else if (w != parent)
        {
            return 1;
        }
        adj = adj->next;
    }
    return 0;
}

int detectCycle(Graph *graph)
{
    int *visited = calloc(graph->V, sizeof(int));
    if (!visited)
        return 0;
    int cycleFound = 0;
    for (int i = 0; i < graph->V; i++)
    {
        if (!visited[i] && graph->nodes[i].name != NULL)
        {
            if (DFSUtilDetectCycle(graph, i, -1, visited))
            {
                cycleFound = 1;
                break;
            }
        }
    }
    free(visited);
    return cycleFound;
}

/* --- Vérification d'accessibilité (BFS) --- */
int isReachable(Graph *graph, int start, int target)
{
    if (start < 0 || start >= graph->V || target < 0 || target >= graph->V)
        return 0;
    int *visited = calloc(graph->V, sizeof(int));
    if (!visited)
        return 0;
    int *queue = malloc(graph->V * sizeof(int));
    if (!queue)
    {
        free(visited);
        return 0;
    }
    int front = 0, rear = 0;
    visited[start] = 1;
    queue[rear++] = start;
    int reachable = 0;
    while (front < rear)
    {
        int current = queue[front++];
        if (current == target)
        {
            reachable = 1;
            break;
        }
        AdjListNode *adj = graph->array[current].head;
        while (adj)
        {
            if (!visited[adj->dest])
            {
                visited[adj->dest] = 1;
                queue[rear++] = adj->dest;
            }
            adj = adj->next;
        }
    }
    free(queue);
    free(visited);
    return reachable;
}

/* --- Composantes connexes via DFS --- */
static void DFSUtilCC(Graph *graph, int v, int compIndex, int *visited, int *components)
{
    visited[v] = 1;
    components[v] = compIndex;
    AdjListNode *adj = graph->array[v].head;
    while (adj)
    {
        if (!visited[adj->dest] && graph->nodes[adj->dest].name != NULL)
            DFSUtilCC(graph, adj->dest, compIndex, visited, components);
        adj = adj->next;
    }
}

int findConnectedComponents(Graph *graph, int *components)
{
    int *visited = calloc(graph->V, sizeof(int));
    if (!visited)
        return 0;
    int compIndex = 0;
    for (int i = 0; i < graph->V; i++)
    {
        if (!visited[i] && graph->nodes[i].name != NULL)
        {
            compIndex++;
            DFSUtilCC(graph, i, compIndex, visited, components);
        }
    }
    free(visited);
    return compIndex;
}

/* --- Points d'articulation (Tarjan simplifié) --- */
static int timeGlobal;
static void articulationDFS(Graph *graph, int u, int *visited, int *disc, int *low, int *parent, int *ap)
{
    visited[u] = 1;
    disc[u] = low[u] = ++timeGlobal;
    int children = 0;
    AdjListNode *adj = graph->array[u].head;
    while (adj)
    {
        int v = adj->dest;
        if (!visited[v])
        {
            children++;
            parent[v] = u;
            articulationDFS(graph, v, visited, disc, low, parent, ap);
            if (low[v] < low[u])
                low[u] = low[v];
            if (parent[u] == -1 && children > 1)
                ap[u] = 1;
            if (parent[u] != -1 && low[v] >= disc[u])
                ap[u] = 1;
        }
        else if (v != parent[u])
        {
            if (disc[v] < low[u])
                low[u] = disc[v];
        }
        adj = adj->next;
    }
}

void findArticulationPoints(Graph *graph, int *artPoints)
{
    int *visited = calloc(graph->V, sizeof(int));
    int *disc = calloc(graph->V, sizeof(int));
    int *low = calloc(graph->V, sizeof(int));
    int *parent = calloc(graph->V, sizeof(int));
    for (int i = 0; i < graph->V; i++)
    {
        visited[i] = 0;
        parent[i] = -1;
        artPoints[i] = 0;
    }
    timeGlobal = 0;
    for (int i = 0; i < graph->V; i++)
    {
        if (!visited[i] && graph->nodes[i].name != NULL)
            articulationDFS(graph, i, visited, disc, low, parent, artPoints);
    }
    free(visited);
    free(disc);
    free(low);
    free(parent);
}

/* --- Statistiques de connectivité --- */
void computeConnectivityStats(Graph *graph)
{
    int *visited = calloc(graph->V, sizeof(int));
    int *compSize = calloc(graph->V, sizeof(int));
    if (!visited || !compSize)
        return;
    int compCount = 0;
    for (int i = 0; i < graph->V; i++)
    {
        if (!visited[i] && graph->nodes[i].name != NULL)
        {
            compCount++;
            int stackSize = 0;
            int *stack = malloc(graph->V * sizeof(int));
            if (!stack)
                break;
            int top = -1;
            stack[++top] = i;
            visited[i] = 1;
            while (top >= 0)
            {
                int v = stack[top--];
                stackSize++;
                AdjListNode *adj = graph->array[v].head;
                while (adj)
                {
                    if (!visited[adj->dest])
                    {
                        visited[adj->dest] = 1;
                        stack[++top] = adj->dest;
                    }
                    adj = adj->next;
                }
            }
            free(stack);
            compSize[compCount - 1] = stackSize;
        }
    }
    printf("\n--- Statistiques de connectivité ---\n");
    printf("Nombre de composantes connexes : %d\n", compCount);
    for (int c = 0; c < compCount; c++)
    {
        printf("  Composante %d - Taille: %d\n", c + 1, compSize[c]);
    }
    free(compSize);
    free(visited);
}
