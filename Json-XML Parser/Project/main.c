/* main.c */
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"             // parse_graph, free_parsed_graph
#include "graph.h"              // Graph
#include "graph_applications.h" // floydWarshall, bellmanFord, etc.
#include "output.h"             // allocMatrix, printMatrix, writeCsv, etc.

int main(void)
{
    char filename[256];
    printf("Entrez le fichier d'entrée (ex: input.json ou input.xml) : ");
    if (scanf("%255s", filename) != 1)
    {
        fprintf(stderr, "Erreur de saisie\n");
        return 1;
    }

    // 1) Charger le graphe (JSON ou XML suivant l'extension)
    Graph *g = NULL;
    if (parse_graph(filename, &g) != 0)
    {
        fprintf(stderr, "Échec du parsing de '%s'\n", filename);
        return 1;
    }
    int V = g->V;
    printf("\nGraphe chargé : %d nœud(s)\n", V);

    // 2) Floyd–Warshall
    float **fwDist = allocMatrix(V);
    if (!fwDist)
    {
        fprintf(stderr, "Erreur allocMatrix\n");
        free_parsed_graph(g);
        return 1;
    }
    floydWarshall(g, fwDist);
    printMatrix(fwDist, V, "Distances (Floyd–Warshall)");
    writeCsv("floyd.csv", fwDist, V);

    // 3) Bellman–Ford depuis la source 0
    float *bfDist = malloc(V * sizeof(float));
    int *bfPred = malloc(V * sizeof(int));
    if (!bfDist || !bfPred)
    {
        fprintf(stderr, "Erreur malloc\n");
    }
    else
    {
        if (bellmanFord(g, 0, bfDist, bfPred) == 0)
        {
            printVector(bfDist, V, "Distances (Bellman–Ford depuis 0)");
            writeCsv1d("bellman.csv", bfDist, V);
        }
        else
        {
            printf("Cycle à poids négatif détecté par Bellman–Ford\n");
        }
    }

    // 4) Détection de cycle
    printf("\nCycle détecté : %s\n",
           detectCycle(g) ? "oui" : "non");

    // 5) Accessibilité (ex : nœud 1 → nœud 2)
    if (V >= 2)
    {
        printf("Nœud 1 → nœud 2 reachable ? %s\n",
               isReachable(g, 0, 1) ? "oui" : "non");
    }

    // 6) Composantes connexes
    int *components = malloc(V * sizeof(int));
    if (components)
    {
        int nComp = findConnectedComponents(g, components);
        printf("\nNombre de composantes connexes : %d\n", nComp);
        printComponents(components, V);
        free(components);
    }

    // 7) Points d'articulation
    int *artPoints = malloc(V * sizeof(int));
    if (artPoints)
    {
        findArticulationPoints(g, artPoints);
        printArtPoints(artPoints, V);
        free(artPoints);
    }

    // 8) Statistiques
    computeConnectivityStats(g);

    // 9) Nettoyage
    freeMatrix(fwDist);
    free(bfDist);
    free(bfPred);
    free_parsed_graph(g);

    return 0;
}
