/* graph_applications.c */
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include "graph_applications.h"
#include "graph.h"

#ifndef INF
#define INF (1.0f / 0.0f)
#endif

//--- 1. Détection de cycle (DFS sur matrice) ---
static void dfs_cycle(const Graph *g, int u, int parent, int *vis, int *found)
{
    vis[u] = 1;
    for (int v = 0; v < g->V && !*found; v++)
    {
        if (g->dist[u][v] != INF)
        {
            if (!vis[v])
                dfs_cycle(g, v, u, vis, found);
            else if (v != parent)
                *found = 1;
        }
    }
}

int detectCycle(Graph *g)
{
    int *vis = calloc(g->V, sizeof(int));
    int found = 0;
    for (int i = 0; i < g->V && !found; i++)
        if (!vis[i])
            dfs_cycle(g, i, -1, vis, &found);
    free(vis);
    return found;
}

//--- 2. Accessibilité (BFS sur matrice) ---
int isReachable(Graph *g, int src, int tgt)
{
    if (src < 0 || tgt < 0 || src >= g->V || tgt >= g->V)
        return 0;
    int *vis = calloc(g->V, sizeof(int));
    int *queue = malloc(g->V * sizeof(int));
    int front = 0, rear = 0;
    vis[src] = 1;
    queue[rear++] = src;
    while (front < rear)
    {
        int u = queue[front++];
        if (u == tgt)
        {
            free(queue);
            free(vis);
            return 1;
        }
        for (int v = 0; v < g->V; v++)
        {
            if (g->dist[u][v] != INF && !vis[v])
            {
                vis[v] = 1;
                queue[rear++] = v;
            }
        }
    }
    free(queue);
    free(vis);
    return 0;
}

//--- 3. Composantes connexes (DFS) ---
static void dfs_cc(const Graph *g, int u, int cid, int *vis, int *comp)
{
    vis[u] = 1;
    comp[u] = cid;
    for (int v = 0; v < g->V; v++)
        if (g->dist[u][v] != INF && !vis[v])
            dfs_cc(g, v, cid, vis, comp);
}

int findConnectedComponents(Graph *g, int *comp)
{
    int *vis = calloc(g->V, sizeof(int));
    int cid = 0;
    for (int i = 0; i < g->V; i++)
        if (!vis[i])
            dfs_cc(g, i, ++cid, vis, comp);
    free(vis);
    return cid;
}

//--- 4. Points d’articulation (Tarjan) ---
static void tarjanAP(const Graph *g, int u, int *disc, int *low, int *par,
                     int *vis, int *ap, int *tt)
{
    vis[u] = 1;
    disc[u] = low[u] = ++(*tt);
    int children = 0;
    for (int v = 0; v < g->V; v++)
    {
        if (g->dist[u][v] == INF)
            continue;
        if (!vis[v])
        {
            children++;
            par[v] = u;
            tarjanAP(g, v, disc, low, par, vis, ap, tt);
            low[u] = (low[v] < low[u] ? low[v] : low[u]);
            if ((par[u] == -1 && children > 1) ||
                (par[u] != -1 && low[v] >= disc[u]))
                ap[u] = 1;
        }
        else if (v != par[u])
        {
            low[u] = (disc[v] < low[u] ? disc[v] : low[u]);
        }
    }
}

void findArticulationPoints(Graph *g, int *art)
{
    int V = g->V, time = 0;
    int *disc = calloc(V, sizeof(int));
    int *low = calloc(V, sizeof(int));
    int *par = calloc(V, sizeof(int));
    int *vis = calloc(V, sizeof(int));
    memset(art, 0, V * sizeof(int));
    for (int i = 0; i < V; i++)
        par[i] = -1;
    for (int i = 0; i < V; i++)
        if (!vis[i])
            tarjanAP(g, i, disc, low, par, vis, art, &time);
    free(disc);
    free(low);
    free(par);
    free(vis);
}

//--- 5. Statistiques de connectivité ---
void computeConnectivityStats(Graph *g)
{
    int V = g->V;
    int *comp = malloc(V * sizeof(int));
    int cnt = findConnectedComponents(g, comp);
    int *sz = calloc(cnt, sizeof(int));
    for (int i = 0; i < V; i++)
        sz[comp[i] - 1]++;
    printf("Composantes: %d\n", cnt);
    for (int i = 0; i < cnt; i++)
        printf("  #%d : %d nœuds\n", i + 1, sz[i]);
    free(sz);
    free(comp);
}

//--- 6. Floyd–Warshall (tous-pairs shortest paths) ---
void floydWarshall(const Graph *g, float **outDist)
{
    int V = g->V;
    // copier la matrice
    for (int i = 0; i < V; i++)
        for (int j = 0; j < V; j++)
            outDist[i][j] = g->dist[i][j];
    // DP
    for (int k = 0; k < V; k++)
        for (int i = 0; i < V; i++)
            if (outDist[i][k] != INF)
                for (int j = 0; j < V; j++)
                    if (outDist[k][j] != INF &&
                        outDist[i][j] > outDist[i][k] + outDist[k][j])
                        outDist[i][j] = outDist[i][k] + outDist[k][j];
}

//--- 7. Bellman–Ford (origine unique) ---
int bellmanFord(const Graph *g, int src, float *dist, int *pred)
{
    int V = g->V;
    // init
    for (int i = 0; i < V; i++)
    {
        dist[i] = INF;
        pred[i] = -1;
    }
    dist[src] = 0;
    // relaxation
    for (int it = 1; it < V; it++)
    {
        int updated = 0;
        for (int u = 0; u < V; u++)
        {
            for (int v = 0; v < V; v++)
                if (g->dist[u][v] != INF)
                {
                    float w = g->dist[u][v];
                    if (dist[u] != INF && dist[v] > dist[u] + w)
                    {
                        dist[v] = dist[u] + w;
                        pred[v] = u;
                        updated = 1;
                    }
                }
        }
        if (!updated)
            break;
    }
    // détection de cycle négatif
    for (int u = 0; u < V; u++)
        for (int v = 0; v < V; v++)
            if (g->dist[u][v] != INF &&
                dist[u] != INF && dist[v] > dist[u] + g->dist[u][v])
                return -1;
    return 0;
}

//--- 8. Squelettes TSP et GA (à compléter) ---
float solveTSP_DP(const Graph *g, int start, int **outTour)
{
    // TODO: implémenter DP (bitmask + memo)
    return -1.0f;
}
float solveTSP_Greedy(const Graph *g, int start, int *outTour)
{
    // TODO: implémenter heuristique nearest-neighbor
    return -1.0f;
}
void geneticTSP(const Graph *g, int popSize, int generations, GA_Indiv *outBest)
{
    // TODO: initialiser population, boucler cross/mutation, sélectionner
}
