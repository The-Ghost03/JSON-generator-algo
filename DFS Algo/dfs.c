#include <stdio.h>
#include <stdlib.h>

// Structure pour les attributs d'une arête
typedef struct EdgeAttr
{
    float distance;    // en kilomètres
    float baseTime;    // temps de base en minutes
    float cost;        // coût monétaire
    int roadType;      // type de route (0: asphalte, 1: latérite, etc.)
    float reliability; // fiabilité de la route
    int restrictions;  // restrictions codées en bits
} EdgeAttr;

// Structure pour un nœud de la liste d'adjacence
typedef struct AdjListNode
{
    int dest;                 // ID du nœud de destination
    EdgeAttr attr;            // Attributs de l'arête
    struct AdjListNode *next; // Pointeur vers le nœud suivant dans la liste
} AdjListNode;

// Structure principale du graphe logistique
typedef struct
{
    int nodeCount;                // Nombre total de nœuds
    AdjListNode **adjacencyLists; // Tableau de listes d'adjacence
    char **nodeNames;             // Noms des nœuds pour affichage
} LogisticsGraph;

/*
 * Fonction DFSUtil
 * Parcourt récursivement le graphe à partir du nœud 'v' et marque les nœuds visités.
 */
void DFSUtil(LogisticsGraph *graph, int v, int *visited)
{
    visited[v] = 1;
    printf("Visite du nœud %d: %s\n", v, graph->nodeNames[v]);

    // Parcours de la liste d'adjacence du nœud actuel
    AdjListNode *adjNode = graph->adjacencyLists[v];
    while (adjNode != NULL)
    {
        if (!visited[adjNode->dest])
        {
            DFSUtil(graph, adjNode->dest, visited);
        }
        adjNode = adjNode->next;
    }
}

/*
 * Fonction DFS
 * Initialisation du tableau de visites et démarrage du parcours DFS à partir du nœud 'start'.
 */
void DFS(LogisticsGraph *graph, int start)
{
    int *visited = (int *)calloc(graph->nodeCount, sizeof(int));
    if (!visited)
    {
        fprintf(stderr, "Erreur d'allocation mémoire pour le tableau 'visited'\n");
        return;
    }

    printf("Début du parcours DFS à partir du nœud %d\n", start);
    DFSUtil(graph, start, visited);

    free(visited);
}

/* Exemple d'utilisation du module DFS */
int main(void)
{
    // Exemple minimal de création d'un graphe avec 3 nœuds
    int n = 3;
    LogisticsGraph graph;
    graph.nodeCount = n;

    // Allocation du tableau des listes d'adjacence et des noms
    graph.adjacencyLists = malloc(n * sizeof(AdjListNode *));
    graph.nodeNames = malloc(n * sizeof(char *));
    for (int i = 0; i < n; i++)
    {
        graph.adjacencyLists[i] = NULL;
    }
    // Noms des nœuds (par exemple)
    graph.nodeNames[0] = strdup("A");
    graph.nodeNames[1] = strdup("B");
    graph.nodeNames[2] = strdup("C");

    // Création manuelle des arêtes pour l'exemple :
    // A -> B
    AdjListNode *nodeAB = malloc(sizeof(AdjListNode));
    nodeAB->dest = 1;
    nodeAB->attr.distance = 10.0;
    nodeAB->attr.baseTime = 15.0;
    nodeAB->attr.cost = 500.0;
    nodeAB->attr.roadType = 0;
    nodeAB->attr.reliability = 0.9;
    nodeAB->attr.restrictions = 0;
    nodeAB->next = NULL;
    graph.adjacencyLists[0] = nodeAB;

    // B -> C
    AdjListNode *nodeBC = malloc(sizeof(AdjListNode));
    nodeBC->dest = 2;
    nodeBC->attr.distance = 20.0;
    nodeBC->attr.baseTime = 30.0;
    nodeBC->attr.cost = 1000.0;
    nodeBC->attr.roadType = 1;
    nodeBC->attr.reliability = 0.95;
    nodeBC->attr.restrictions = 1;
    nodeBC->next = NULL;
    graph.adjacencyLists[1] = nodeBC;

    // C -> A (pour former un cycle)
    AdjListNode *nodeCA = malloc(sizeof(AdjListNode));
    nodeCA->dest = 0;
    nodeCA->attr.distance = 25.0;
    nodeCA->attr.baseTime = 35.0;
    nodeCA->attr.cost = 1200.0;
    nodeCA->attr.roadType = 0;
    nodeCA->attr.reliability = 0.85;
    nodeCA->attr.restrictions = 0;
    nodeCA->next = NULL;
    graph.adjacencyLists[2] = nodeCA;

    // Lancement du parcours DFS à partir du nœud 0 (A)
    DFS(&graph, 0);

    // Libération de la mémoire du graphe
    for (int i = 0; i < n; i++)
    {
        free(graph.nodeNames[i]);
        // Libération de la liste d'adjacence
        AdjListNode *cur = graph.adjacencyLists[i];
        while (cur)
        {
            AdjListNode *temp = cur;
            cur = cur->next;
            free(temp);
        }
    }
    free(graph.nodeNames);
    free(graph.adjacencyLists);

    return 0;
}
