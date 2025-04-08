/* parser.h */
#ifndef PARSER_H
#define PARSER_H

#include "graph.h"

/* ===============================
   SECTION JSON
   =============================== */
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

JsonValue *parse_json_file(const char *fileContent);
void print_json(const JsonValue *value, int indent);
void free_json(JsonValue *value);

Graph *buildGraphFromJson(const JsonValue *root);

/* ===============================
   SECTION XML
   =============================== */
typedef struct XmlAttribute
{
    char *key;
    char *value;
} XmlAttribute;

typedef struct XmlNode
{
    char *tag;
    XmlAttribute *attributes;
    size_t attribute_count;
    char *text;
    struct XmlNode **children;
    size_t child_count;
} XmlNode;

XmlNode *parse_xml_element(const char **p);
void print_xml(const XmlNode *node, int indent);
void free_xml(XmlNode *node);

Graph *buildGraphFromXml(const void *xmlRoot);

/* ===============================
   FONCTIONS UTILITAIRES COMMUNES
   =============================== */
char *read_file(const char *filename);
const char *skip_whitespace(const char *s);

#endif /* PARSER_H */
