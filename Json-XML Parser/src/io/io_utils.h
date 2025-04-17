#ifndef PARSER_JSON_H
#define PARSER_JSON_H

#include "../graph/graph.h"

/** Charge un graphe depuis un JSON. Stub, retourne toujours NULL. */
Graph *parser_json_load(const char *filename);

#endif // PARSER_JSON_H
#ifndef IO_UTILS_H
#define IO_UTILS_H

/** Lit tout le fichier en mémoire (malloc), retourne NULL si erreur. */
char *read_file(const char *filename);

/** Retourne le pointeur passé en avance jusqu’au premier caractère non‑blanc. */
const char *skip_whitespace(const char *s);

#endif // IO_UTILS_H
