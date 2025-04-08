/* graph_applications.h */
#ifndef GRAPH_APPLICATIONS_H
#define GRAPH_APPLICATIONS_H

#include "graph.h"

/* Détecte les cycles dans le graphe (graphe non orienté) */
int detectCycle(Graph *graph);

/* Vérifie si le nœud target est accessible à partir du nœud start */
int isReachable(Graph *graph, int start, int target);

/* Remplit le tableau components (alloué par l'appelant) avec l'indice de composante de chaque nœud.
   Retourne le nombre de composantes connexes. */
int findConnectedComponents(Graph *graph, int *components);

/* Détecte et marque les points d'articulation (nœuds critiques) dans le tableau artPoints */
void findArticulationPoints(Graph *graph, int *artPoints);

/* Calcule et affiche des statistiques sur la connectivité du graphe */
void computeConnectivityStats(Graph *graph);

#endif /* GRAPH_APPLICATIONS_H */
