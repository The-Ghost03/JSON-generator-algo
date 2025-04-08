/* parser.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"

#define INITIAL_TOKEN_CAPACITY 100

/* ===============================
   Fonctions utilitaires communes
   =============================== */
const char *skip_whitespace(const char *s)
{
    while (*s && isspace((unsigned char)*s))
        s++;
    return s;
}

/* ===============================
   Module de parsing JSON
   =============================== */
typedef enum
{
    TOKEN_BEGIN_OBJECT,
    TOKEN_END_OBJECT,
    TOKEN_BEGIN_ARRAY,
    TOKEN_END_ARRAY,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_STRING,
    TOKEN_NUMBER,
    TOKEN_BOOLEAN,
    TOKEN_NULL,
    TOKEN_EOF,
    TOKEN_UNKNOWN
} TokenType;

typedef struct
{
    TokenType type;
    char *value;
} Token;

static Token create_token(TokenType type, const char *value)
{
    Token t;
    t.type = type;
    t.value = value ? strdup(value) : NULL;
    return t;
}

static Token parse_string(const char **s)
{
    (*s)++; // Skip opening quote
    char *buffer = malloc(256);
    int bufsize = 256, pos = 0;
    while (**s && **s != '"')
    {
        if (**s == '\\')
        {
            (*s)++;
            if (**s == 'n')
                buffer[pos++] = '\n';
            else if (**s == 't')
                buffer[pos++] = '\t';
            else
                buffer[pos++] = **s;
            (*s)++;
        }
        else
        {
            buffer[pos++] = **s;
            (*s)++;
        }
        if (pos >= bufsize - 1)
        {
            bufsize *= 2;
            buffer = realloc(buffer, bufsize);
        }
    }
    buffer[pos] = '\0';
    Token t = create_token(TOKEN_STRING, buffer);
    free(buffer);
    if (**s == '"')
        (*s)++; // Skip closing quote
    return t;
}

static Token parse_number(const char **s)
{
    const char *start = *s;
    while (**s && (isdigit((unsigned char)**s) || **s == '.' || **s == '-' ||
                   **s == '+' || **s == 'e' || **s == 'E'))
        (*s)++;
    int len = (int)(*s - start);
    char *numStr = malloc(len + 1);
    strncpy(numStr, start, len);
    numStr[len] = '\0';
    Token t = create_token(TOKEN_NUMBER, numStr);
    free(numStr);
    return t;
}

static void free_tokens(Token *tokens, int count)
{
    for (int i = 0; i < count; i++)
    {
        free(tokens[i].value);
    }
    free(tokens);
}

static Token *tokenize(const char *input, int *token_count)
{
    int capacity = INITIAL_TOKEN_CAPACITY, count = 0;
    Token *tokens = malloc(sizeof(Token) * capacity);
    const char *p = input;
    while (*p != '\0')
    {
        p = skip_whitespace(p);
        if (*p == '\0')
            break;
        if (*p == '{')
        {
            tokens[count++] = create_token(TOKEN_BEGIN_OBJECT, NULL);
            p++;
        }
        else if (*p == '}')
        {
            tokens[count++] = create_token(TOKEN_END_OBJECT, NULL);
            p++;
        }
        else if (*p == '[')
        {
            tokens[count++] = create_token(TOKEN_BEGIN_ARRAY, NULL);
            p++;
        }
        else if (*p == ']')
        {
            tokens[count++] = create_token(TOKEN_END_ARRAY, NULL);
            p++;
        }
        else if (*p == ':')
        {
            tokens[count++] = create_token(TOKEN_COLON, NULL);
            p++;
        }
        else if (*p == ',')
        {
            tokens[count++] = create_token(TOKEN_COMMA, NULL);
            p++;
        }
        else if (*p == '"')
        {
            tokens[count++] = parse_string(&p);
        }
        else if (isdigit((unsigned char)*p) || *p == '-' || *p == '+')
        {
            tokens[count++] = parse_number(&p);
        }
        else if (strncmp(p, "true", 4) == 0)
        {
            tokens[count++] = create_token(TOKEN_BOOLEAN, "true");
            p += 4;
        }
        else if (strncmp(p, "false", 5) == 0)
        {
            tokens[count++] = create_token(TOKEN_BOOLEAN, "false");
            p += 5;
        }
        else if (strncmp(p, "null", 4) == 0)
        {
            tokens[count++] = create_token(TOKEN_NULL, "null");
            p += 4;
        }
        else
        {
            tokens[count++] = create_token(TOKEN_UNKNOWN, NULL);
            p++;
        }
        if (count >= capacity)
        {
            capacity *= 2;
            tokens = realloc(tokens, sizeof(Token) * capacity);
        }
    }
    tokens[count++] = create_token(TOKEN_EOF, NULL);
    *token_count = count;
    return tokens;
}

/* Forward declarations for JSON parsing functions */
static JsonValue *parse_json(Token *tokens, int *i, int token_count);
static JsonValue *parse_array(Token *tokens, int *i, int token_count);
static JsonValue *parse_object(Token *tokens, int *i, int token_count);

static JsonValue *parse_array(Token *tokens, int *i, int token_count)
{
    (*i)++; // Skip '['
    JsonValue *value = malloc(sizeof(JsonValue));
    value->type = JSON_ARRAY;
    size_t capacity = 4;
    value->as.array.items = malloc(capacity * sizeof(JsonValue *));
    value->as.array.count = 0;
    while (*i < token_count && tokens[*i].type != TOKEN_END_ARRAY)
    {
        if (tokens[*i].type == TOKEN_COMMA)
        {
            (*i)++;
            continue;
        }
        JsonValue *item = parse_json(tokens, i, token_count);
        if (item)
        {
            if (value->as.array.count >= capacity)
            {
                capacity *= 2;
                value->as.array.items = realloc(value->as.array.items, capacity * sizeof(JsonValue *));
            }
            value->as.array.items[value->as.array.count++] = item;
        }
    }
    if (*i < token_count && tokens[*i].type == TOKEN_END_ARRAY)
        (*i)++;
    return value;
}

static JsonValue *parse_object(Token *tokens, int *i, int token_count)
{
    (*i)++; // Skip '{'
    JsonValue *value = malloc(sizeof(JsonValue));
    value->type = JSON_OBJECT;
    size_t capacity = 4;
    value->as.object.entries = malloc(capacity * sizeof(JsonObjectEntry));
    value->as.object.count = 0;
    while (*i < token_count && tokens[*i].type != TOKEN_END_OBJECT)
    {
        if (tokens[*i].type == TOKEN_COMMA)
        {
            (*i)++;
            continue;
        }
        if (tokens[*i].type == TOKEN_STRING)
        {
            char *key = strdup(tokens[*i].value);
            (*i)++; // Skip key
            if (*i < token_count && tokens[*i].type == TOKEN_COLON)
                (*i)++;
            JsonValue *item = parse_json(tokens, i, token_count);
            if (value->as.object.count >= capacity)
            {
                capacity *= 2;
                value->as.object.entries = realloc(value->as.object.entries, capacity * sizeof(JsonObjectEntry));
            }
            value->as.object.entries[value->as.object.count].key = key;
            value->as.object.entries[value->as.object.count].value = item;
            value->as.object.count++;
        }
        else
        {
            (*i)++;
        }
    }
    if (*i < token_count && tokens[*i].type == TOKEN_END_OBJECT)
        (*i)++;
    return value;
}

static JsonValue *parse_json(Token *tokens, int *i, int token_count)
{
    if (*i >= token_count)
        return NULL;
    Token current = tokens[*i];
    if (current.type == TOKEN_NULL)
    {
        JsonValue *val = malloc(sizeof(JsonValue));
        val->type = JSON_NULL;
        (*i)++;
        return val;
    }
    else if (current.type == TOKEN_BOOLEAN)
    {
        JsonValue *val = malloc(sizeof(JsonValue));
        val->type = JSON_BOOL;
        val->as.boolValue = (strcmp(current.value, "true") == 0);
        (*i)++;
        return val;
    }
    else if (current.type == TOKEN_NUMBER)
    {
        JsonValue *val = malloc(sizeof(JsonValue));
        val->type = JSON_NUMBER;
        val->as.numberValue = atof(current.value);
        (*i)++;
        return val;
    }
    else if (current.type == TOKEN_STRING)
    {
        JsonValue *val = malloc(sizeof(JsonValue));
        val->type = JSON_STRING;
        val->as.stringValue = strdup(current.value);
        (*i)++;
        return val;
    }
    else if (current.type == TOKEN_BEGIN_ARRAY)
    {
        return parse_array(tokens, i, token_count);
    }
    else if (current.type == TOKEN_BEGIN_OBJECT)
    {
        return parse_object(tokens, i, token_count);
    }
    else
    {
        (*i)++;
        return NULL;
    }
}

void print_json(const JsonValue *value, int indent)
{
    if (!value)
        return;
    for (int i = 0; i < indent; i++)
        printf("  ");
    switch (value->type)
    {
    case JSON_NULL:
        printf("null\n");
        break;
    case JSON_BOOL:
        printf("%s\n", value->as.boolValue ? "true" : "false");
        break;
    case JSON_NUMBER:
        printf("%f\n", value->as.numberValue);
        break;
    case JSON_STRING:
        printf("\"%s\"\n", value->as.stringValue);
        break;
    case JSON_ARRAY:
        printf("[\n");
        for (size_t i = 0; i < value->as.array.count; i++)
        {
            print_json(value->as.array.items[i], indent + 1);
        }
        for (int i = 0; i < indent; i++)
            printf("  ");
        printf("]\n");
        break;
    case JSON_OBJECT:
        printf("{\n");
        for (size_t i = 0; i < value->as.object.count; i++)
        {
            for (int j = 0; j < indent + 1; j++)
                printf("  ");
            printf("\"%s\": ", value->as.object.entries[i].key);
            print_json(value->as.object.entries[i].value, indent + 1);
        }
        for (int i = 0; i < indent; i++)
            printf("  ");
        printf("}\n");
        break;
    }
}

void free_json(JsonValue *value)
{
    if (!value)
        return;
    switch (value->type)
    {
    case JSON_STRING:
        free(value->as.stringValue);
        break;
    case JSON_ARRAY:
        for (size_t i = 0; i < value->as.array.count; i++)
            free_json(value->as.array.items[i]);
        free(value->as.array.items);
        break;
    case JSON_OBJECT:
        for (size_t i = 0; i < value->as.object.count; i++)
        {
            free(value->as.object.entries[i].key);
            free_json(value->as.object.entries[i].value);
        }
        free(value->as.object.entries);
        break;
    default:
        break;
    }
    free(value);
}

char *read_file(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        perror("Erreur ouverture fichier");
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    rewind(fp);
    char *buffer = malloc(filesize + 1);
    if (!buffer)
    {
        fclose(fp);
        return NULL;
    }
    size_t read_size = fread(buffer, 1, filesize, fp);
    buffer[read_size] = '\0';
    fclose(fp);
    return buffer;
}

JsonValue *parse_json_file(const char *fileContent)
{
    int token_count = 0;
    Token *tokens = tokenize(fileContent, &token_count);
    int index = 0;
    JsonValue *root = parse_json(tokens, &index, token_count);
    free_tokens(tokens, token_count);
    return root;
}

/* ===============================
   Conversion du JSON en Graph
   =============================== */
Graph *buildGraphFromJson(const JsonValue *root)
{
    if (!root || root->type != JSON_OBJECT)
        return NULL;
    JsonValue *nodesVal = NULL, *edgesVal = NULL;
    for (size_t i = 0; i < root->as.object.count; i++)
    {
        if (strcmp(root->as.object.entries[i].key, "nodes") == 0)
            nodesVal = root->as.object.entries[i].value;
        else if (strcmp(root->as.object.entries[i].key, "edges") == 0)
            edgesVal = root->as.object.entries[i].value;
    }
    if (!nodesVal || nodesVal->type != JSON_ARRAY)
    {
        printf("Erreur : champ 'nodes' manquant ou invalide.\n");
        return NULL;
    }
    int nodeCount = (int)nodesVal->as.array.count;
    Graph *graph = createGraph(nodeCount);
    for (int i = 0; i < nodeCount; i++)
    {
        JsonValue *nodeObj = nodesVal->as.array.items[i];
        if (!nodeObj || nodeObj->type != JSON_OBJECT)
            continue;
        int id = i + 1;
        JsonValue *idField = NULL;
        for (size_t j = 0; j < nodeObj->as.object.count; j++)
        {
            if (strcmp(nodeObj->as.object.entries[j].key, "id") == 0)
                idField = nodeObj->as.object.entries[j].value;
        }
        if (idField && idField->type == JSON_NUMBER)
            id = (int)idField->as.numberValue;
        char *name = strdup("Unnamed");
        JsonValue *nameField = NULL;
        for (size_t j = 0; j < nodeObj->as.object.count; j++)
        {
            if (strcmp(nodeObj->as.object.entries[j].key, "nom") == 0)
                nameField = nodeObj->as.object.entries[j].value;
        }
        if (nameField && nameField->type == JSON_STRING)
        {
            free(name);
            name = strdup(nameField->as.stringValue);
        }
        char *type = strdup("undefined");
        JsonValue *typeField = NULL;
        for (size_t j = 0; j < nodeObj->as.object.count; j++)
        {
            if (strcmp(nodeObj->as.object.entries[j].key, "type") == 0)
                typeField = nodeObj->as.object.entries[j].value;
        }
        if (typeField && typeField->type == JSON_STRING)
        {
            free(type);
            type = strdup(typeField->as.stringValue);
        }
        int capacity = 0;
        JsonValue *capField = NULL;
        for (size_t j = 0; j < nodeObj->as.object.count; j++)
        {
            if (strcmp(nodeObj->as.object.entries[j].key, "capacity") == 0)
                capField = nodeObj->as.object.entries[j].value;
        }
        if (capField && capField->type == JSON_NUMBER)
            capacity = (int)capField->as.numberValue;
        float coord[2] = {0.0f, 0.0f};
        JsonValue *coordField = NULL;
        for (size_t j = 0; j < nodeObj->as.object.count; j++)
        {
            if (strcmp(nodeObj->as.object.entries[j].key, "coordinates") == 0)
                coordField = nodeObj->as.object.entries[j].value;
        }
        if (coordField && coordField->type == JSON_ARRAY && coordField->as.array.count >= 2)
        {
            JsonValue *latField = coordField->as.array.items[0];
            JsonValue *lonField = coordField->as.array.items[1];
            if (latField && latField->type == JSON_NUMBER)
                coord[0] = (float)latField->as.numberValue;
            if (lonField && lonField->type == JSON_NUMBER)
                coord[1] = (float)lonField->as.numberValue;
        }
        float cong_morning = 1.0f, cong_afternoon = 1.0f, cong_night = 1.0f;
        JsonValue *morningField = NULL, *afternoonField = NULL, *nightField = NULL;
        for (size_t j = 0; j < nodeObj->as.object.count; j++)
        {
            if (strcmp(nodeObj->as.object.entries[j].key, "congestion_morning") == 0)
                morningField = nodeObj->as.object.entries[j].value;
            else if (strcmp(nodeObj->as.object.entries[j].key, "congestion_afternoon") == 0)
                afternoonField = nodeObj->as.object.entries[j].value;
            else if (strcmp(nodeObj->as.object.entries[j].key, "congestion_night") == 0)
                nightField = nodeObj->as.object.entries[j].value;
        }
        if (morningField && morningField->type == JSON_NUMBER)
            cong_morning = (float)morningField->as.numberValue;
        if (afternoonField && afternoonField->type == JSON_NUMBER)
            cong_afternoon = (float)afternoonField->as.numberValue;
        if (nightField && nightField->type == JSON_NUMBER)
            cong_night = (float)nightField->as.numberValue;
        int index = id - 1;
        if (index < 0 || index >= nodeCount)
            index = i;
        graph->nodes[index].id = id;
        graph->nodes[index].name = name;
        graph->nodes[index].type = type;
        graph->nodes[index].capacity = capacity;
        graph->nodes[index].coordinates[0] = coord[0];
        graph->nodes[index].coordinates[1] = coord[1];
        graph->nodes[index].congestion_morning = cong_morning;
        graph->nodes[index].congestion_afternoon = cong_afternoon;
        graph->nodes[index].congestion_night = cong_night;
    }
    if (edgesVal && edgesVal->type == JSON_ARRAY)
    {
        for (size_t i = 0; i < edgesVal->as.array.count; i++)
        {
            JsonValue *edgeObj = edgesVal->as.array.items[i];
            if (!edgeObj || edgeObj->type != JSON_OBJECT)
                continue;
            int src = -1, dest = -1;
            EdgeAttr attr = {0};
            JsonValue *srcField = NULL, *destField = NULL;
            for (size_t j = 0; j < edgeObj->as.object.count; j++)
            {
                if (strcmp(edgeObj->as.object.entries[j].key, "source_id") == 0)
                    srcField = edgeObj->as.object.entries[j].value;
                else if (strcmp(edgeObj->as.object.entries[j].key, "destination_id") == 0)
                    destField = edgeObj->as.object.entries[j].value;
            }
            if (srcField && srcField->type == JSON_NUMBER)
                src = (int)srcField->as.numberValue - 1;
            if (destField && destField->type == JSON_NUMBER)
                dest = (int)destField->as.numberValue - 1;
            JsonValue *distField = NULL, *timeField = NULL, *costField = NULL,
                      *typeFieldEdge = NULL, *fiabField = NULL, *restField = NULL, *weatherField = NULL;
            for (size_t j = 0; j < edgeObj->as.object.count; j++)
            {
                if (strcmp(edgeObj->as.object.entries[j].key, "distance") == 0)
                    distField = edgeObj->as.object.entries[j].value;
                else if ((strcmp(edgeObj->as.object.entries[j].key, "temps_base") == 0) ||
                         (strcmp(edgeObj->as.object.entries[j].key, "baseTime") == 0))
                    timeField = edgeObj->as.object.entries[j].value;
                else if ((strcmp(edgeObj->as.object.entries[j].key, "cout") == 0) ||
                         (strcmp(edgeObj->as.object.entries[j].key, "cout_monetaire") == 0))
                    costField = edgeObj->as.object.entries[j].value;
                else if (strcmp(edgeObj->as.object.entries[j].key, "type_route") == 0)
                    typeFieldEdge = edgeObj->as.object.entries[j].value;
                else if (strcmp(edgeObj->as.object.entries[j].key, "fiabilite") == 0)
                    fiabField = edgeObj->as.object.entries[j].value;
                else if ((strcmp(edgeObj->as.object.entries[j].key, "restrictions") == 0) ||
                         (strcmp(edgeObj->as.object.entries[j].key, "restrictions_bitmask") == 0))
                    restField = edgeObj->as.object.entries[j].value;
                else if (strcmp(edgeObj->as.object.entries[j].key, "weatherType") == 0)
                    weatherField = edgeObj->as.object.entries[j].value;
            }
            if (distField && distField->type == JSON_NUMBER)
                attr.distance = (float)distField->as.numberValue;
            if (timeField && timeField->type == JSON_NUMBER)
                attr.baseTime = (float)timeField->as.numberValue;
            if (costField && costField->type == JSON_NUMBER)
                attr.cost = (float)costField->as.numberValue;
            if (typeFieldEdge && typeFieldEdge->type == JSON_NUMBER)
                attr.roadType = (int)typeFieldEdge->as.numberValue;
            if (fiabField && fiabField->type == JSON_NUMBER)
                attr.reliability = (float)fiabField->as.numberValue;
            if (restField && restField->type == JSON_NUMBER)
                attr.restrictions = (int)restField->as.numberValue;
            attr.weatherType = 0;
            if (weatherField && weatherField->type == JSON_NUMBER)
                attr.weatherType = (int)weatherField->as.numberValue;
            if (src >= 0 && src < graph->V && dest >= 0 && dest < graph->V)
                addEdgeToGraph(graph, src, dest, attr);
        }
    }
    return graph;
}

/* ===============================
   Module de parsing XML (minimal)
   =============================== */

/* On ne conserve ici qu'un unique ensemble de définitions XML */
static void xml_skip_whitespace(const char **p)
{
    while (**p && isspace((unsigned char)**p))
        (*p)++;
}

static char *xml_parse_tag(const char **p)
{
    xml_skip_whitespace(p);
    const char *start = *p;
    while (**p && !isspace((unsigned char)**p) && **p != '>' && **p != '/')
        (*p)++;
    int len = (int)(*p - start);
    char *tag = malloc(len + 1);
    strncpy(tag, start, len);
    tag[len] = '\0';
    return tag;
}

static char *xml_parse_text(const char **p)
{
    const char *start = *p;
    while (**p && **p != '<')
        (*p)++;
    int len = (int)(*p - start);
    char *text = malloc(len + 1);
    strncpy(text, start, len);
    text[len] = '\0';
    return text;
}

static XmlAttribute *xml_parse_attributes(const char **p, size_t *attr_count)
{
    size_t capacity = 4;
    XmlAttribute *attrs = malloc(capacity * sizeof(XmlAttribute));
    *attr_count = 0;
    xml_skip_whitespace(p);
    while (**p && **p != '>' && **p != '/')
    {
        const char *start = *p;
        while (**p && !isspace((unsigned char)**p) && **p != '=')
            (*p)++;
        int len = (int)(*p - start);
        char *key = malloc(len + 1);
        strncpy(key, start, len);
        key[len] = '\0';
        xml_skip_whitespace(p);
        if (**p == '=')
        {
            (*p)++;
            xml_skip_whitespace(p);
        }
        char quote = **p;
        char *value = NULL;
        if (quote == '"' || quote == '\'')
        {
            (*p)++;
            const char *valStart = *p;
            while (**p && **p != quote)
                (*p)++;
            int vlen = (int)(*p - valStart);
            value = malloc(vlen + 1);
            strncpy(value, valStart, vlen);
            value[vlen] = '\0';
            if (**p == quote)
                (*p)++;
        }
        if (*attr_count >= capacity)
        {
            capacity *= 2;
            attrs = realloc(attrs, capacity * sizeof(XmlAttribute));
        }
        attrs[*attr_count].key = key;
        attrs[*attr_count].value = value;
        (*attr_count)++;
        xml_skip_whitespace(p);
    }
    return attrs;
}

XmlNode *parse_xml_element(const char **p)
{
    xml_skip_whitespace(p);
    if (**p != '<')
        return NULL;
    (*p)++; // Skip '<'
    if (**p == '?')
    {
        while (**p && strncmp(*p, "?>", 2) != 0)
            (*p)++;
        if (**p)
            (*p) += 2;
        return parse_xml_element(p);
    }
    if (**p == '/')
        return NULL;
    char *tag = xml_parse_tag(p);
    XmlNode *node = malloc(sizeof(XmlNode));
    node->tag = tag;
    node->attributes = NULL;
    node->attribute_count = 0;
    node->text = NULL;
    node->children = NULL;
    node->child_count = 0;
    xml_skip_whitespace(p);
    if (**p != '>' && **p != '/')
        node->attributes = xml_parse_attributes(p, &node->attribute_count);
    if (**p == '/')
    {
        (*p)++;
        if (**p == '>')
            (*p)++;
        return node;
    }
    if (**p == '>')
        (*p)++;
    size_t children_capacity = 4;
    node->children = malloc(children_capacity * sizeof(XmlNode *));
    node->child_count = 0;
    while (**p)
    {
        xml_skip_whitespace(p);
        if (**p == '<')
        {
            if ((*p)[1] == '/')
                break;
            XmlNode *child = parse_xml_element(p);
            if (child)
            {
                if (node->child_count >= children_capacity)
                {
                    children_capacity *= 2;
                    node->children = realloc(node->children, children_capacity * sizeof(XmlNode *));
                }
                node->children[node->child_count++] = child;
            }
        }
        else
        {
            char *text = xml_parse_text(p);
            if (text && strlen(text) > 0)
            {
                if (node->text == NULL)
                    node->text = text;
                else
                {
                    size_t oldLen = strlen(node->text);
                    size_t newLen = oldLen + strlen(text) + 1;
                    node->text = realloc(node->text, newLen);
                    strcat(node->text, text);
                    free(text);
                }
            }
            else
            {
                free(text);
            }
        }
        xml_skip_whitespace(p);
    }
    if (**p == '<' && (*p)[1] == '/')
    {
        (*p) += 2; // Skip "</"
        char *closeTag = xml_parse_tag(p);
        free(closeTag);
        xml_skip_whitespace(p);
        if (**p == '>')
            (*p)++;
    }
    return node;
}

void print_xml(const XmlNode *node, int indent)
{
    if (!node)
        return;
    for (int i = 0; i < indent; i++)
        printf("  ");
    printf("<%s", node->tag);
    for (size_t i = 0; i < node->attribute_count; i++)
    {
        printf(" %s=\"%s\"", node->attributes[i].key, node->attributes[i].value);
    }
    if (node->child_count == 0 && (!node->text || strlen(node->text) == 0))
    {
        printf("/>\n");
        return;
    }
    printf(">");
    if (node->text)
        printf("%s", node->text);
    if (node->child_count > 0)
    {
        printf("\n");
        for (size_t i = 0; i < node->child_count; i++)
        {
            print_xml(node->children[i], indent + 1);
        }
        for (int i = 0; i < indent; i++)
            printf("  ");
    }
    printf("</%s>\n", node->tag);
}

void free_xml(XmlNode *node)
{
    if (!node)
        return;
    free(node->tag);
    for (size_t i = 0; i < node->attribute_count; i++)
    {
        free(node->attributes[i].key);
        free(node->attributes[i].value);
    }
    free(node->attributes);
    if (node->text)
        free(node->text);
    for (size_t i = 0; i < node->child_count; i++)
    {
        free_xml(node->children[i]);
    }
    free(node->children);
    free(node);
}

/* ===============================
   Conversion du XML en Graph
   =============================== */
static char *get_xml_child_text(const XmlNode *node, const char *childTag)
{
    if (!node || !childTag)
        return NULL;
    for (size_t i = 0; i < node->child_count; i++)
    {
        if (node->children[i]->tag && strcmp(node->children[i]->tag, childTag) == 0)
        {
            if (node->children[i]->text)
                return node->children[i]->text;
        }
    }
    return NULL;
}

Graph *buildGraphFromXml(const void *xmlRoot)
{
    const XmlNode *root = (const XmlNode *)xmlRoot;
    if (!root)
        return NULL;
    const XmlNode *nodesNode = NULL, *edgesNode = NULL;
    for (size_t i = 0; i < root->child_count; i++)
    {
        if (root->children[i]->tag)
        {
            if (strcmp(root->children[i]->tag, "nodes") == 0)
                nodesNode = root->children[i];
            else if (strcmp(root->children[i]->tag, "edges") == 0)
                edgesNode = root->children[i];
        }
    }
    if (!nodesNode)
    {
        printf("Erreur : Balise <nodes> absente dans le fichier XML.\n");
        return NULL;
    }
    int nodeCount = (int)nodesNode->child_count;
    Graph *graph = createGraph(nodeCount);
    /* Traitement des nœuds */
    for (int i = 0; i < nodeCount; i++)
    {
        const XmlNode *nodeObj = nodesNode->children[i];
        if (!nodeObj || strcmp(nodeObj->tag, "node") != 0)
            continue;
        int id = i + 1;
        char *idText = get_xml_child_text(nodeObj, "id");
        if (idText)
            id = atoi(idText);
        char *name = strdup("Unnamed");
        char *nameText = get_xml_child_text(nodeObj, "nom");
        if (nameText)
        {
            free(name);
            name = strdup(nameText);
        }
        char *type = strdup("undefined");
        char *typeText = get_xml_child_text(nodeObj, "type");
        if (typeText)
        {
            free(type);
            type = strdup(typeText);
        }
        int capacity = 0;
        char *capText = get_xml_child_text(nodeObj, "capacity");
        if (capText)
            capacity = atoi(capText);
        float coord[2] = {0.0f, 0.0f};
        char *coordText = get_xml_child_text(nodeObj, "coordinates");
        if (coordText)
        {
            char *token = strtok(coordText, ",");
            if (token)
            {
                coord[0] = atof(token);
                token = strtok(NULL, ",");
                if (token)
                    coord[1] = atof(token);
            }
        }
        float cong_morning = 1.0f, cong_afternoon = 1.0f, cong_night = 1.0f;
        char *morningText = get_xml_child_text(nodeObj, "congestion_morning");
        if (morningText)
            cong_morning = atof(morningText);
        char *afternoonText = get_xml_child_text(nodeObj, "congestion_afternoon");
        if (afternoonText)
            cong_afternoon = atof(afternoonText);
        char *nightText = get_xml_child_text(nodeObj, "congestion_night");
        if (nightText)
            cong_night = atof(nightText);
        int index = id - 1;
        if (index < 0 || index >= nodeCount)
            index = i;
        graph->nodes[index].id = id;
        graph->nodes[index].name = name;
        graph->nodes[index].type = type;
        graph->nodes[index].capacity = capacity;
        graph->nodes[index].coordinates[0] = coord[0];
        graph->nodes[index].coordinates[1] = coord[1];
        graph->nodes[index].congestion_morning = cong_morning;
        graph->nodes[index].congestion_afternoon = cong_afternoon;
        graph->nodes[index].congestion_night = cong_night;
    }
    /* Traitement des arêtes */
    if (edgesNode)
    {
        for (size_t i = 0; i < edgesNode->child_count; i++)
        {
            const XmlNode *edgeObj = edgesNode->children[i];
            if (!edgeObj || strcmp(edgeObj->tag, "edge") != 0)
                continue;
            int src = -1, dest = -1;
            EdgeAttr attr = {0};
            char *srcText = get_xml_child_text(edgeObj, "source_id");
            char *destText = get_xml_child_text(edgeObj, "destination_id");
            if (srcText)
                src = atoi(srcText) - 1;
            if (destText)
                dest = atoi(destText) - 1;
            char *distText = get_xml_child_text(edgeObj, "distance");
            if (distText)
                attr.distance = atof(distText);
            char *timeText = get_xml_child_text(edgeObj, "baseTime");
            if (!timeText)
                timeText = get_xml_child_text(edgeObj, "temps_base");
            if (timeText)
                attr.baseTime = atof(timeText);
            char *costText = get_xml_child_text(edgeObj, "cout");
            if (!costText)
                costText = get_xml_child_text(edgeObj, "cout_monetaire");
            if (costText)
                attr.cost = atof(costText);
            char *typeTextEdge = get_xml_child_text(edgeObj, "type_route");
            if (typeTextEdge)
                attr.roadType = atoi(typeTextEdge);
            char *fiabText = get_xml_child_text(edgeObj, "fiabilite");
            if (fiabText)
                attr.reliability = atof(fiabText);
            char *restText = get_xml_child_text(edgeObj, "restrictions");
            if (!restText)
                restText = get_xml_child_text(edgeObj, "restrictions_bitmask");
            if (restText)
                attr.restrictions = atoi(restText);
            attr.weatherType = 0;
            char *weatherText = get_xml_child_text(edgeObj, "weatherType");
            if (weatherText)
                attr.weatherType = atoi(weatherText);
            if (src >= 0 && src < graph->V && dest >= 0 && dest < graph->V)
                addEdgeToGraph(graph, src, dest, attr);
        }
    }
    return graph;
}
