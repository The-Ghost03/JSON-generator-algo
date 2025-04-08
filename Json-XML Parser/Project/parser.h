/* parser.h */
#ifndef PARSER_H
#define PARSER_H

#include "graph.h"

/* Types et structures pour le parsing JSON */
typedef enum
{
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} JsonType;

typedef struct JsonValue JsonValue;
typedef struct
{
    char *key;
    JsonValue *value;
} JsonObjectEntry;

struct JsonValue
{
    JsonType type;
    union
    {
        int boolValue;
        double numberValue;
        char *stringValue;
        struct
        {
            JsonValue **items;
            size_t count;
        } array;
        struct
        {
            JsonObjectEntry *entries;
            size_t count;
        } object;
    } as;
};

/* Prototypes pour le parseur JSON */
JsonValue *parse_json_file(const char *fileContent);
void print_json(const JsonValue *value, int indent);
void free_json(JsonValue *value);

/* Prototypes pour la conversion en graphe */
Graph *buildGraphFromJson(const JsonValue *root);
Graph *buildGraphFromXml(const void *xmlRoot); // Note : Adapter selon votre parseur XML

#endif /* PARSER_H */
