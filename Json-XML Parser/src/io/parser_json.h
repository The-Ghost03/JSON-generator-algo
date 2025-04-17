#ifndef PARSER_JSON_H
#define PARSER_JSON_H

/* Pour le type Graph */
#include "graph/graph.h"

/* Charge un graphe depuis un fichier JSON, renvoie NULL en cas d’erreur */
Graph *parser_json_load(const char *filename);

#endif /* PARSER_JSON_H */
