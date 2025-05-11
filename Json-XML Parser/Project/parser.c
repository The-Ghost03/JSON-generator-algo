/* parser.c */
#include "parser.h"
#include "graph.h"
#include "CJSON.h" // votre CJSON.c/.h "vendored"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef INFINITY
#define INFINITY (1.0f / 0.0f)
#endif

#ifdef USE_LIBXML
#include <libxml/parser.h>
#include <libxml/tree.h>
#endif

// forward declaration
static char *read_file(const char *filename);

// -----------------------------------------------------------------------------
// parse_graph : choix du parseur selon extension
// -----------------------------------------------------------------------------
int parse_graph(const char *filename, Graph **out_graph)
{
    const char *ext = strrchr(filename, '.');
    if (!ext)
    {
        fprintf(stderr, "parser: pas d'extension sur %s\n", filename);
        return -1;
    }
    if (strcasecmp(ext, ".json") == 0)
    {
        return parse_graph_from_json(filename, out_graph);
    }
#ifdef USE_LIBXML
    else if (strcasecmp(ext, ".xml") == 0)
    {
        return parse_graph_from_xml(filename, out_graph);
    }
#else
    else if (strcasecmp(ext, ".xml") == 0)
    {
        fprintf(stderr, "parser: XML support désactivé (recompilez avec -DUSE_LIBXML)\n");
        return -1;
    }
#endif
    else
    {
        fprintf(stderr, "parser: extension non supportée '%s'\n", ext);
        return -1;
    }
}

// -----------------------------------------------------------------------------
// JSON parser
// -----------------------------------------------------------------------------
int parse_graph_from_json(const char *filename, Graph **out_graph)
{
    char *text = read_file(filename);
    if (!text)
        return -1;

    cJSON *root = cJSON_Parse(text);
    free(text);
    if (!root)
    {
        fprintf(stderr, "JSON Error: %s\n", cJSON_GetErrorPtr());
        return -1;
    }

    cJSON *nodes = cJSON_GetObjectItem(root, "nodes");
    if (!cJSON_IsArray(nodes))
    {
        fprintf(stderr, "JSON: 'nodes' manquant ou non-array\n");
        cJSON_Delete(root);
        return -1;
    }
    int V = cJSON_GetArraySize(nodes);
    Graph *g = create_graph(V);
    if (!g)
    {
        cJSON_Delete(root);
        return -1;
    }

    // init distances
    for (int i = 0; i < V; i++)
        for (int j = 0; j < V; j++)
            g->dist[i][j] = (i == j ? 0.0f : INFINITY);

    cJSON *edges = cJSON_GetObjectItem(root, "edges");
    if (!cJSON_IsArray(edges))
    {
        fprintf(stderr, "JSON: 'edges' manquant ou non-array\n");
        free_graph(g);
        cJSON_Delete(root);
        return -1;
    }

    cJSON *e = NULL;
    cJSON_ArrayForEach(e, edges)
    {
        cJSON *js_src = cJSON_GetObjectItem(e, "source");
        cJSON *js_dst = cJSON_GetObjectItem(e, "destination");
        cJSON *js_dist = cJSON_GetObjectItem(e, "distance");
        cJSON *js_time = cJSON_GetObjectItem(e, "baseTime");
        if (!cJSON_IsNumber(js_src) || !cJSON_IsNumber(js_dst) || !cJSON_IsNumber(js_dist) || !cJSON_IsNumber(js_time))
        {
            fprintf(stderr, "JSON: champ edge manquant ou incorrect\n");
            continue;
        }
        int u = js_src->valueint, v = js_dst->valueint;
        EdgeAttr attr = {
            .distance = (float)js_dist->valuedouble,
            .baseTime = (float)js_time->valuedouble,
            .cost = 0.0f,
            .roadType = 0,
            .reliability = 1.0f,
            .restrictions = 0};
        // champs optionnels
        cJSON *tmp;
        if ((tmp = cJSON_GetObjectItem(e, "cost")) && cJSON_IsNumber(tmp))
            attr.cost = tmp->valuedouble;
        if ((tmp = cJSON_GetObjectItem(e, "roadType")) && cJSON_IsNumber(tmp))
            attr.roadType = tmp->valueint;
        if ((tmp = cJSON_GetObjectItem(e, "reliability")) && cJSON_IsNumber(tmp))
            attr.reliability = tmp->valuedouble;
        if ((tmp = cJSON_GetObjectItem(e, "restrictions")) && cJSON_IsNumber(tmp))
            attr.restrictions = tmp->valueint;
        // timeVariation.morning
        if ((tmp = cJSON_GetObjectItem(e, "timeVariation")) && cJSON_IsObject(tmp))
        {
            cJSON *m = cJSON_GetObjectItem(tmp, "morning");
            if (cJSON_IsNumber(m))
                attr.baseTime *= (float)m->valuedouble;
        }
        add_edge(g, u, v, &attr);
    }

    cJSON_Delete(root);
    *out_graph = g;
    return 0;
}

#ifdef USE_LIBXML
// -----------------------------------------------------------------------------
// XML parser (activé seulement si -DUSE_LIBXML)
// -----------------------------------------------------------------------------
int parse_graph_from_xml(const char *filename, Graph **out_graph)
{
    xmlDocPtr doc = xmlReadFile(filename, NULL, 0);
    if (!doc)
    {
        fprintf(stderr, "XML: impossible d'ouvrir %s\n", filename);
        return -1;
    }
    xmlNode *root = xmlDocGetRootElement(doc);
    if (!root)
    {
        fprintf(stderr, "XML: doc vide\n");
        xmlFreeDoc(doc);
        return -1;
    }
    // même logique que précédemment pour JSON, mais en XML...
    // (votre code existant ici)
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return -1; // remplacer par implémentation complète
}
#endif

// -----------------------------------------------------------------------------
// cleanup
// -----------------------------------------------------------------------------
void free_parsed_graph(Graph *g)
{
    free_graph(g);
}

// -----------------------------------------------------------------------------
// utils
// -----------------------------------------------------------------------------
static char *read_file(const char *filename)
{
    FILE *f = fopen(filename, "rb");
    if (!f)
    {
        perror("read_file:fopen");
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(sz + 1);
    if (!buf)
    {
        fclose(f);
        return NULL;
    }
    if (fread(buf, 1, sz, f) != (size_t)sz)
    {
        perror("read_file:fread");
        free(buf);
        fclose(f);
        return NULL;
    }
    buf[sz] = 0;
    fclose(f);
    return buf;
}
