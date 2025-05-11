/* graph_applications.h */
#ifndef GRAPH_APPLICATIONS_H
#define GRAPH_APPLICATIONS_H

#include "graph.h"

/* Parcours de graphe de base */
int detectCycle(Graph *g);
int isReachable(Graph *g, int start, int target);
int findConnectedComponents(Graph *g, int *components);
void findArticulationPoints(Graph *g, int *artPoints);
void computeConnectivityStats(Graph *g);

/* Programmation dynamique */
void floydWarshall(const Graph *g, float **outDist);
int bellmanFord(const Graph *g, int src, float *outDist, int *outPred);

/* Voyageur de commerce (DP exact ou heuristique) */
float solveTSP_DP(const Graph *g, int start, int **outTour);
float solveTSP_Greedy(const Graph *g, int start, int *outTour);

/* Algorithme génétique basique pour TSP */
typedef struct
{
   int *chromosome; /* séquence de noeuds */
   float fitness;   /* distance totale ou coût */
} GA_Indiv;
void geneticTSP(const Graph *g, int popSize, int generations, GA_Indiv *outBest);

#endif /* GRAPH_APPLICATIONS_H */
