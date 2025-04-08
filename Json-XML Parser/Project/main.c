/* main.c */
#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "parser.h"
#include "graph_applications.h"
#include "optimize.h" // Ce header contient également les définitions de Delivery et Vehicle.

int main(void)
{
    char input_filename[256];
    printf("Entrez le nom du fichier d'entree (ex : input.json ou input.xml) : ");
    if (scanf("%255s", input_filename) != 1)
    {
        fprintf(stderr, "Erreur lors de la lecture du nom du fichier.\n");
        return 1;
    }

    /* Lecture du fichier d'entrée */
    char *file_content = read_file(input_filename);
    if (!file_content)
    {
        fprintf(stderr, "Erreur lors de la lecture du fichier %s\n", input_filename);
        return 1;
    }

    /* Conserver le pointeur d'origine pour la libération */
    char *original_content = file_content;

    /* Gestion du BOM UTF-8 pour JSON, si présent.
       On avance file_content sans modifier original_content */
    if ((unsigned char)file_content[0] == 0xEF &&
        (unsigned char)file_content[1] == 0xBB &&
        (unsigned char)file_content[2] == 0xBF)
    {
        file_content += 3;
    }

    const char *p = skip_whitespace(file_content);
    Graph *graph = NULL;

    /* Construction du graphe à partir du fichier (JSON ou XML) */
    if (*p == '{' || *p == '[')
    {
        /* Parsing JSON */
        JsonValue *json_root = parse_json_file(file_content);
        if (json_root)
        {
            printf("\n--- Structure JSON construite ---\n");
            print_json(json_root, 0);
            graph = buildGraphFromJson(json_root);
            free_json(json_root);
        }
        else
        {
            fprintf(stderr, "Erreur de parsing JSON.\n");
        }
    }
    else if (*p == '<')
    {
        /* Parsing XML */
        XmlNode *xml_root = parse_xml_element(&p);
        if (xml_root)
        {
            printf("\n--- Structure XML construite ---\n");
            print_xml(xml_root, 0);
            graph = buildGraphFromXml(xml_root);
            free_xml(xml_root);
        }
        else
        {
            fprintf(stderr, "Erreur de parsing XML.\n");
        }
    }
    else
    {
        printf("Format de fichier non reconnu.\n");
        free(original_content);
        return 1;
    }

    /* Libération du contenu original du fichier */
    free(original_content);

    if (!graph)
    {
        printf("Echec de la conversion en graphe.\n");
        return 1;
    }

    /* Affichage du graphe construit */
    printf("\n--- Graphe converti ---\n");
    printf("Nombre de noeuds : %d\n", graph->V);
    for (int i = 0; i < graph->V; i++)
    {
        if (graph->nodes[i].name)
            printf("Noeud %d : %s, Type: %s\n", i + 1, graph->nodes[i].name, graph->nodes[i].type);
        AdjListNode *adj = graph->array[i].head;
        while (adj)
        {
            printf("  -> Vers noeud %d | Distance: %.2f, Temps: %.2f, Cout: %.2f, WeatherType: %d\n",
                   adj->dest + 1, adj->attr.distance, adj->attr.baseTime,
                   adj->attr.cost, adj->attr.weatherType);
            adj = adj->next;
        }
    }

    /* Appel aux modules d'optimisation */
    // 1. Utilisation de Floyd-Warshall pour les plus courts chemins
    double **distMatrix = createDistanceMatrix(graph);
    floydWarshall(graph->V, distMatrix);
    printDistanceMatrix(graph->V, distMatrix);
    freeDistanceMatrix(graph->V, distMatrix);

    // 2. Bellman-Ford adapté (exemple avec source = noeud 0)
    bellman_ford_time_aware(graph, 0);

    // 3. Algorithme génétique pour le TSP
    tsp_genetic_solution(graph);

    // 4. Planification journalière gloutonne - exemple avec 2 livraisons et 1 véhicule
    Delivery sampleDeliveries[2] = {
        {.id = 1, .origin = 0, .destination = 12, .volume = 20, .deadline = 300, .livre = 0},
        {.id = 2, .origin = 30, .destination = 7, .volume = 700, .deadline = 1800, .livre = 0}};
    Vehicle sampleVehicle = {.capacity = 1000, .dispo_debut = 480, .dispo_fin = 1020, .cost_per_km = 0.7, .position = 0};
    planification_gloutonne(graph, sampleDeliveries, 2, &sampleVehicle);

    // 5. Planification multi-jours (à compléter selon vos données réelles)
    // multi_day_scheduling(deliveries, nbDeliveries, vehicles, nbVehicles, num_days, graph);

    /* Appel aux fonctions du module d'applications sur le graphe */
    if (detectCycle(graph))
        printf("\nCycle detecte dans le graphe.\n");
    else
        printf("\nAucun cycle detecte dans le graphe.\n");

    if (isReachable(graph, 0, 4))
        printf("Le noeud 5 est accessible depuis le noeud 1.\n");

    int *components = calloc(graph->V, sizeof(int));
    int nbComp = findConnectedComponents(graph, components);
    printf("Nombre de composantes connexes : %d\n", nbComp);
    free(components);

    int *artPoints = calloc(graph->V, sizeof(int));
    findArticulationPoints(graph, artPoints);
    for (int i = 0; i < graph->V; i++)
    {
        if (artPoints[i])
            printf("Noeud %d est un point d'articulation.\n", i + 1);
    }
    free(artPoints);

    computeConnectivityStats(graph);

    /* Libération du graphe */
    freeGraph(graph);

    printf("Appuyez sur une touche pour continuer...\n");
    system("pause");
    return 0;
}
