#include <stdio.h>
#include <stdlib.h>

// Structure pour les attributs d'une arête
typedef struct EdgeAttr
{
    float distance;
    float baseTime;
    float cost;
    int roadType;
    float reliability;
    int restrictions;
} EdgeAttr;

// Structure d'un nœud de la liste d'adjacence
typedef struct AdjListNode
{
    int dest;
    EdgeAttr attr;
    struct AdjListNode *next;
} AdjListNode;

// Structure pour la liste d'adjacence
typedef struct AdjList
{
    AdjListNode *head;
} AdjList;

// Structure du graphe
typedef struct Graph
{
    int V;
    AdjList *array;
} Graph;

// Fonction DFS récursive
void DFS(Graph *graph, int vertex, int visited[])
{
    visited[vertex] = 1;
    printf("Visite du sommet %d\n", vertex);

    // Parcourir les voisins du sommet actuel
    AdjListNode *temp = graph->array[vertex].head;
    while (temp)
    {
        int adjVertex = temp->dest;
        if (!visited[adjVertex])
        {
            DFS(graph, adjVertex, visited);
        }
        temp = temp->next;
    }
}

// Fonction pour initialiser le DFS
void depthFirstSearch(Graph *graph, int startVertex)
{
    int *visited = (int *)calloc(graph->V, sizeof(int));
    DFS(graph, startVertex, visited);
    free(visited);
}
