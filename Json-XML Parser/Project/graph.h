/* graph.h */
#ifndef GRAPH_H
#define GRAPH_H

#include <stddef.h>

/**
 * Valeur utilisée pour représenter l'absence de connexion directe
 * entre deux nœuds (i ≠ j) dans la matrice des distances.
 */
#ifndef INF
#define INF (1.0f / 0.0f)
#endif

/**
 * Attributs d’une arête orientée u → v :
 */
typedef struct
{
    float distance;    /**< longueur (km, m, etc.) */
    float baseTime;    /**< temps de parcours de base (min, h…) */
    float cost;        /**< coût associé (carburant, péage…) */
    float reliability; /**< fiabilité (0.0 = pas fiable, 1.0 = fiable) */
    int roadType;      /**< code du type de route (enum/bitfield) */
    int restrictions;  /**< code des restrictions (poids, hauteur…) */
} EdgeAttr;

/**
 * Graphe à V sommets numérotés 0..V-1.
 *
 * - dist[i][j]  = distance de i → j, ou INF si pas d’arête
 * - attrs[i][j] = attributs complets de l’arête i → j
 */
typedef struct
{
    int V;            /**< nombre de sommets */
    float **dist;     /**< matrice V×V des distances */
    EdgeAttr **attrs; /**< matrice V×V des attributs */
} Graph;

/**
 * Crée un graphe à V sommets :
 *  - alloue dist (float**) et attrs (EdgeAttr**)
 *  - initialise dist[i][i]=0, dist[i][j]=INF pour i≠j
 *  - zero attrs (à remplir via add_edge)
 *
 * @param V nombre de sommets
 * @return pointeur sur Graph, ou NULL en erreur
 */
Graph *create_graph(int V);

/**
 * Ajoute ou remplace l’arête u → v :
 *  - met dist[u][v] = attr->distance
 *  - copie *attr dans attrs[u][v]
 *
 * @param g    graphe
 * @param u    sommet source (0 ≤ u < g->V)
 * @param v    sommet destination
 * @param attr attributs de l’arête
 */
void add_edge(Graph *g, int u, int v, const EdgeAttr *attr);

/**
 * Renvoie la distance i → j (INF si pas d’arête).
 */
float get_edge_distance(const Graph *g, int i, int j);

/**
 * Renvoie pointeur vers EdgeAttr i → j (la distance INF indique l'absence).
 */
const EdgeAttr *get_edge_attr(const Graph *g, int i, int j);

/**
 * Libère toute la mémoire allouée au graphe.
 */
void free_graph(Graph *g);

#endif /* GRAPH_H */
