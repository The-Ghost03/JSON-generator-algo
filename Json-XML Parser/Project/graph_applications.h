/* graph_applications.h */
#ifndef GRAPH_APPLICATIONS_H
#define GRAPH_APPLICATIONS_H

#include "graph.h"

/* Détecte les cycles dans le graphe */
int detectCycle(Graph *graph);

/* Vérifie si le nœud target est accessible depuis le nœud start */
int isReachable(Graph *graph, int start, int target);

/* Identifie les composantes connexes.
   Le tableau 'components' doit être alloué par l'appelant et contiendra l'indice de composante pour chaque nœud. */
int findConnectedComponents(Graph *graph, int *components);

/* Détecte les points d'articulation et place 1 dans artPoints[i] si le nœud i en est un. */
void findArticulationPoints(Graph *graph, int *artPoints);

/* Calcule et affiche des statistiques sur la connectivité du graphe */
void computeConnectivityStats(Graph *graph);

#endif /* GRAPH_APPLICATIONS_H */
