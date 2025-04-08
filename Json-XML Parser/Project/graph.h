/* graph.h */
#ifndef GRAPH_H
#define GRAPH_H

typedef struct
{
    int id;               // Identifiant unique du nœud
    char *name;           // Nom (ex: "Hub Abidjan")
    char *type;           // Type : "hub", "relay", "delivery", etc.
    float coordinates[2]; // [latitude, longitude]
    int capacity;         // Capacité du nœud
    // Indices de congestion, utilisés pour moduler le temps de parcours
    float congestion_morning;
    float congestion_afternoon;
    float congestion_night;
} Node;

typedef struct
{
    float distance;    // Distance en km
    float baseTime;    // Temps de parcours de base en minutes
    float cost;        // Coût associé à l’arête
    int roadType;      // Type de route (ex : 0 = asphalte, 1 = latérite, …)
    float reliability; // Fiabilité (entre 0 et 1)
    int restrictions;  // Restrictions (en bits, par exemple)
    int weatherType;   // Type de météo (0 = normal, 1 = pluie, 2 = vent, …)
} EdgeAttr;

typedef struct AdjListNode
{
    int dest;                 // Indice de destination
    EdgeAttr attr;            // Attributs de l’arête
    struct AdjListNode *next; // Pointe vers le nœud suivant dans la liste
} AdjListNode;

typedef struct
{
    AdjListNode *head; // Tête de la liste d’adjacence
} AdjList;

typedef struct
{
    int V;          // Nombre de nœuds
    Node *nodes;    // Tableau des nœuds
    AdjList *array; // Tableau des listes d’adjacence
} Graph;

/* Fonctions de manipulation du graphe */
Graph *createGraph(int V);
void addEdgeToGraph(Graph *graph, int src, int dest, EdgeAttr attr);
void removeEdgeFromGraph(Graph *graph, int src, int dest);
void addNode(Graph **graphPtr, const char *name, float cong_morning, float cong_afternoon, float cong_night);
void removeNode(Graph *graph, int node);
void freeGraph(Graph *graph);

#endif /* GRAPH_H */
