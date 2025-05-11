#include <stdio.h>
#include <stdlib.h>
#include <time.h> // Pour mesurer le temps d'exécution
#include "graph.h"
#include "parser.h"
#include "graph_applications.h"
#include "optimize.h" // Ce header contient également les définitions de Delivery et Vehicle.

void start_timer(clock_t *start)
{
    *start = clock();
}

void end_timer(const char *process_name, clock_t start)
{
    clock_t end = clock();
    double elapsed_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("[Temps d'exécution] %s : %.6f secondes\n", process_name, elapsed_time);
}

int main(void)
{
    clock_t timer;
    char input_filename[256];
    printf("Entrez le nom du fichier d'entree (ex : input.json ou input.xml) : ");
    if (scanf("%255s", input_filename) != 1)
    {
        fprintf(stderr, "Erreur lors de la lecture du nom du fichier.\n");
        return 1;
    }

    /* Lecture du fichier d'entrée */
    start_timer(&timer);
    char *file_content = read_file(input_filename);
    end_timer("Lecture du fichier", timer);

    if (!file_content)
    {
        fprintf(stderr, "Erreur lors de la lecture du fichier %s\n", input_filename);
        return 1;
    }

    /* Conserver le pointeur d'origine pour la libération */
    char *original_content = file_content;

    /* Gestion du BOM UTF-8 pour JSON, si présent */
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
        start_timer(&timer);
        JsonValue *json_root = parse_json_file(file_content);
        end_timer("Parsing JSON", timer);

        if (json_root)
        {
            printf("\n--- Structure JSON construite ---\n");
            print_json(json_root, 0);

            start_timer(&timer);
            graph = buildGraphFromJson(json_root);
            end_timer("Construction du graphe depuis JSON", timer);

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
        start_timer(&timer);
        XmlNode *xml_root = parse_xml_element(&p);
        end_timer("Parsing XML", timer);

        if (xml_root)
        {
            printf("\n--- Structure XML construite ---\n");
            print_xml(xml_root, 0);

            start_timer(&timer);
            graph = buildGraphFromXml(xml_root);
            end_timer("Construction du graphe depuis XML", timer);

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
    start_timer(&timer);
    double **distMatrix = createDistanceMatrix(graph);
    floydWarshall(graph->V, distMatrix);
    end_timer("Floyd-Warshall", timer);

    printDistanceMatrix(graph->V, distMatrix);
    freeDistanceMatrix(graph->V, distMatrix);

    // 2. Bellman-Ford adapté (exemple avec source = noeud 0)
    start_timer(&timer);
    bellman_ford_time_aware(graph, 0);
    end_timer("Bellman-Ford", timer);

    // 3. Algorithme génétique pour le TSP
    start_timer(&timer);
    tsp_genetic_solution(graph);
    end_timer("Algorithme génétique pour TSP", timer);

    // 4. Planification journalière gloutonne
    Delivery sampleDeliveries[2] = {
        {.id = 1, .origin = 0, .destination = 12, .volume = 20, .deadline = 300, .livre = 0},
        {.id = 2, .origin = 30, .destination = 7, .volume = 700, .deadline = 1800, .livre = 0}};
    Vehicle sampleVehicle = {.capacity = 1000, .dispo_debut = 480, .dispo_fin = 1020, .cost_per_km = 0.7, .position = 0};

    start_timer(&timer);
    planification_gloutonne(graph, sampleDeliveries, 2, &sampleVehicle);
    end_timer("Planification journalière gloutonne", timer);

    /* Appel aux fonctions du module d'applications sur le graphe */
    start_timer(&timer);
    if (detectCycle(graph))
        printf("\nCycle detecte dans le graphe.\n");
    else
        printf("\nAucun cycle detecte dans le graphe.\n");
    end_timer("Détection de cycle", timer);

    start_timer(&timer);
    if (isReachable(graph, 0, 4))
        printf("Le noeud 5 est accessible depuis le noeud 1.\n");
    end_timer("Accessibilité entre noeuds", timer);

    int *components = calloc(graph->V, sizeof(int));
    start_timer(&timer);
    int nbComp = findConnectedComponents(graph, components);
    end_timer("Recherche de composantes connexes", timer);
    printf("Nombre de composantes connexes : %d\n", nbComp);
    free(components);

    int *artPoints = calloc(graph->V, sizeof(int));
    start_timer(&timer);
    findArticulationPoints(graph, artPoints);
    end_timer("Recherche de points d'articulation", timer);
    for (int i = 0; i < graph->V; i++)
    {
        if (artPoints[i])
            printf("Noeud %d est un point d'articulation.\n", i + 1);
    }
    free(artPoints);

    start_timer(&timer);
    computeConnectivityStats(graph);
    end_timer("Calcul des statistiques de connectivité", timer);

    /* Libération du graphe */
    start_timer(&timer);
    freeGraph(graph);
    end_timer("Libération du graphe", timer);

    printf("Appuyez sur une touche pour continuer...\n");
    system("pause");
    return 0;
}