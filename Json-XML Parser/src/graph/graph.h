#ifndef GRAPH_H
#define GRAPH_H

/** Structure opaque du graphe. */
typedef struct Graph Graph;

/** Libère toute la mémoire du graphe. */
void free_graph(Graph *g);

/** Affiche le graphe en mode texte (stub). */
void graph_print(const Graph *g);

#endif // GRAPH_H
