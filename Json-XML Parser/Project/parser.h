/* parser.h */
#ifndef PARSER_H
#define PARSER_H

#include "graph.h"

/**
 * Détecte automatiquement le format (JSON ou XML) puis parse.
 * @param filename   Chemin vers le fichier .json ou .xml
 * @param out_graph  Adresse du pointeur Graph* à renseigner
 * @return 0 en cas de succès, <0 en cas d'erreur
 */
int parse_graph(const char *filename, Graph **out_graph);

/**
 * Parse un graphe depuis un fichier JSON.
 * @see parse_graph()
 */
int parse_graph_from_json(const char *filename, Graph **out_graph);

/**
 * Parse un graphe depuis un fichier XML.
 * @see parse_graph()
 */
int parse_graph_from_xml(const char *filename, Graph **out_graph);

/**
 * Libère le graphe alloué par l'un des parseurs.
 */
void free_parsed_graph(Graph *g);

#endif /* PARSER_H */
