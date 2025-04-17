#ifndef PARSER_XML_H
#define PARSER_XML_H

/* Pour le type Graph */
#include "graph/graph.h"

/* Charge un graphe depuis un fichier XML, renvoie NULL en cas d’erreur */
Graph *parser_xml_load(const char *filename);

#endif /* PARSER_XML_H */
