/* graph.h */
#ifndef GRAPH_H
#define GRAPH_H

/* Structure représentant un nœud du graphe */
typedef struct
{
    int id;               // Identifiant unique
    char *name;           // Nom (ex: "Hub Abidjan")
    char *type;           // Type ("hub", "relay", "delivery", etc.)
    float coordinates[2]; // [latitude, longitude]
    int capacity;         // Capacité du nœud
    /* Facteurs de congestion pour moduler le temps de parcours */
    float congestion_morning;
    float congestion_afternoon;
    float congestion_night;
} Node;

/* Structure représentant les attributs d'une arête */
typedef struct
{
    float distance;    // Distance (km)
    float baseTime;    // Temps de parcours de base (minutes)
    float cost;        // Coût
    int roadType;      // Type de route (0 = asphalte, 1 = latérite, etc.)
    float reliability; // Fiabilité (entre 0 et 1)
    int restrictions;  // Restrictions (codées, ex en bits)
    int weatherType;   // Type de météo (0 = normal, 1 = pluie, 2 = vent, etc.)
} EdgeAttr;

/* Structures pour la liste d'adjacence */
typedef struct AdjListNode
{
    int dest;                 // Indice de destination
    EdgeAttr attr;            // Attributs de l'arête
    struct AdjListNode *next; // Pointeur vers l'élément suivant
} AdjListNode;

typedef struct
{
    AdjListNode *head; // Tête de la liste d'adjacence
} AdjList;

/* Structure du graphe */
typedef struct
{
    int V;          // Nombre de nœuds
    Node *nodes;    // Tableau des nœuds
    AdjList *array; // Tableau des listes d'adjacence
} Graph;

/* Prototypes de manipulation du graphe */
Graph *createGraph(int V);
void addEdgeToGraph(Graph *graph, int src, int dest, EdgeAttr attr);
void removeEdgeFromGraph(Graph *graph, int src, int dest);
void addNode(Graph **graphPtr, const char *name, const char *type, float cong_morning, float cong_afternoon, float cong_night);
void removeNode(Graph *graph, int node);
void freeGraph(Graph *graph);

#endif /* GRAPH_H */
