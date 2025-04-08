/* main.c */
#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "parser.h"
#include "graph_applications.h"

int main(void)
{
    char input_filename[256];
    printf("Entrez le nom du fichier d'entree (ex: data.json ou data.xml) : ");
    scanf("%255s", input_filename);

    // Lecture du fichier d'entrée
    char *file_content = malloc(1); // allocation initiale (vous utilisez read_file dans parser.c)
    file_content = read_file(input_filename);
    if (!file_content)
    {
        fprintf(stderr, "Erreur lors de la lecture du fichier %s\n", input_filename);
        return 1;
    }

    // Selon le premier caractère, détecter JSON ou XML
    const char *p = file_content;
    p = skip_whitespace(p);

    Graph *graph = NULL;
    if (*p == '{' || *p == '[')
    {
        JsonValue *json_root = parse_json_file(file_content);
        if (json_root)
        {
            graph = buildGraphFromJson(json_root);
            free_json(json_root);
        }
    }
    else if (*p == '<')
    {
        // Implémenter le parsing XML dans parser.c si nécessaire.
        // Exemple :
        // XmlNode *xml_root = parse_xml_element(&p);
        // graph = buildGraphFromXml(xml_root);
        // free_xml(xml_root);
    }
    else
    {
        printf("Format de fichier non reconnu.\n");
        free(file_content);
        return 1;
    }

    free(file_content);

    if (!graph)
    {
        printf("Échec de la conversion en graphe.\n");
        return 1;
    }

    /* Utilisation des modules d'applications de parcours */
    if (detectCycle(graph))
        printf("Cycle détecté dans le graphe.\n");
    else
        printf("Aucun cycle détecté.\n");

    if (isReachable(graph, 0, 4))
        printf("Le nœud 5 est accessible depuis le nœud 1.\n");

    int *components = calloc(graph->V, sizeof(int));
    int nbComp = findConnectedComponents(graph, components);
    printf("Nombre de composantes connexes: %d\n", nbComp);
    free(components);

    int *artPoints = calloc(graph->V, sizeof(int));
    findArticulationPoints(graph, artPoints);
    for (int i = 0; i < graph->V; i++)
    {
        if (artPoints[i])
            printf("Nœud %d est un point d'articulation.\n", i + 1);
    }
    free(artPoints);

    computeConnectivityStats(graph);

    /* Libération du graphe */
    freeGraph(graph);
    printf("Appuyez sur une touche pour continuer...\n");
    system("pause");
    return 0;
}
