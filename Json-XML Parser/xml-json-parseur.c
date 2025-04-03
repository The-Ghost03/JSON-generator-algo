#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define INF 99999999.0
#define INITIAL_TOKEN_CAPACITY 100

//------------------------------------
// PARTIE 1 : PARSING JSON
//------------------------------------
typedef enum
{
    TOKEN_BEGIN_OBJECT, // {
    TOKEN_END_OBJECT,   // }
    TOKEN_BEGIN_ARRAY,  // [
    TOKEN_END_ARRAY,    // ]
    TOKEN_COLON,        // :
    TOKEN_COMMA,        // ,
    TOKEN_STRING,       // "..."
    TOKEN_NUMBER,       // 123, -3.14, etc.
    TOKEN_BOOLEAN,      // true ou false
    TOKEN_NULL,         // null
    TOKEN_EOF,          // fin de l'entrée
    TOKEN_UNKNOWN       // token non reconnu
} TokenType;

typedef struct
{
    TokenType type;
    char *value; // Pour STRING, NUMBER, BOOLEAN ou NULL
} Token;

static Token create_token(TokenType type, const char *value)
{
    Token t;
    t.type = type;
    t.value = value ? strdup(value) : NULL;
    return t;
}

static void free_tokens(Token *tokens, int count)
{
    for (int i = 0; i < count; i++)
        free(tokens[i].value);
    free(tokens);
}

static const char *skip_whitespace(const char *s)
{
    while (*s && isspace((unsigned char)*s))
        s++;
    return s;
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

static Token parse_string(const char **s)
{
    (*s)++; // saut du guillemet d'ouverture
    const char *start = *s;
    while (**s && **s != '"')
    {
        if (**s == '\\')
            (*s)++; // saut de l'échappement
        (*s)++;
    }
    int len = (int)(*s - start);
    char *strVal = malloc(len + 1);
    strncpy(strVal, start, len);
    strVal[len] = '\0';
    Token t = create_token(TOKEN_STRING, strVal);
    free(strVal);
    if (**s == '"')
        (*s)++; // saut du guillemet de fermeture
    return t;
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

// Structures pour JSON générique
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

static JsonValue *parse_json(Token *tokens, int *i, int token_count);
static JsonValue *parse_array(Token *tokens, int *i, int token_count);
static JsonValue *parse_object(Token *tokens, int *i, int token_count);

static JsonValue *parse_array(Token *tokens, int *i, int token_count)
{
    (*i)++; // saute '['
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
    (*i)++; // saute '{'
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
            (*i)++; // passe la clé
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

static void print_json(const JsonValue *value, int indent)
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
    {
        printf("[\n");
        for (size_t i = 0; i < value->as.array.count; i++)
        {
            print_json(value->as.array.items[i], indent + 1);
        }
        for (int i = 0; i < indent; i++)
            printf("  ");
        printf("]\n");
    }
    break;
    case JSON_OBJECT:
    {
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
    }
    break;
    }
}

static void free_json(JsonValue *value)
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
        {
            free_json(value->as.array.items[i]);
        }
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

//------------------------------------
// PARTIE 2 : PARSING XML (minimal)
//------------------------------------
typedef struct
{
    char *key;
    char *value;
} XmlAttribute;

typedef struct XmlNode
{
    char *tag;
    XmlAttribute *attributes;
    size_t attribute_count;
    char *text; // contenu textuel (optionnel)
    struct XmlNode **children;
    size_t child_count;
} XmlNode;

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

static XmlNode *parse_xml_element(const char **p)
{
    xml_skip_whitespace(p);
    if (**p != '<')
        return NULL;
    (*p)++; // saute '<'
    if (**p == '?')
    { // prologue <?xml ... ?>
        while (**p && strncmp(*p, "?>", 2) != 0)
            (*p)++;
        if (**p)
            (*p) += 2;
        return parse_xml_element(p);
    }
    if (**p == '/')
        return NULL; // balise fermante
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
    {
        node->attributes = xml_parse_attributes(p, &node->attribute_count);
    }
    if (**p == '/')
    { // balise auto-fermante
        (*p)++;
        if (**p == '>')
            (*p)++;
        return node;
    }
    if (**p == '>')
    {
        (*p)++;
    }
    // Lecture du contenu jusqu'à la balise fermante
    size_t children_capacity = 4;
    node->children = malloc(children_capacity * sizeof(XmlNode *));
    node->child_count = 0;
    while (**p)
    {
        xml_skip_whitespace(p);
        if (**p == '<')
        {
            if ((*p)[1] == '/')
                break; // balise fermante
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
                {
                    node->text = text;
                }
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
        (*p) += 2; // saute "</"
        char *closeTag = xml_parse_tag(p);
        free(closeTag);
        xml_skip_whitespace(p);
        if (**p == '>')
            (*p)++;
    }
    return node;
}

static void print_xml(const XmlNode *node, int indent)
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

static void free_xml(XmlNode *node)
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

//------------------------------------
// PARTIE 3 : LECTURE DE FICHIER
//------------------------------------
static char *read_file(const char *filename)
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

//------------------------------------
// PARTIE 4 : STRUCTURE DE GRAPHE (LogisticsGraph)
//------------------------------------
typedef struct EdgeAttr
{
    float distance;
    float baseTime;
    float cost;
    int roadType;
    float reliability;
    int restrictions;
} EdgeAttr;

typedef struct AdjListNode
{
    int dest;                 // Indice du nœud destination (0-indexé)
    EdgeAttr attr;            // Attributs de l'arête
    struct AdjListNode *next; // Prochain nœud dans la liste
} AdjListNode;

typedef struct
{
    int nodeCount;
    AdjListNode **adjacencyLists; // Tableau de listes d'adjacence
    char **nodeNames;             // Noms des nœuds
} LogisticsGraph;

static void addEdge(LogisticsGraph *graph, int src, int dest, EdgeAttr attr)
{
    AdjListNode *newNode = malloc(sizeof(AdjListNode));
    newNode->dest = dest;
    newNode->attr = attr;
    newNode->next = graph->adjacencyLists[src];
    graph->adjacencyLists[src] = newNode;
}

//------------------------------------
// PARTIE 5 : CONVERSION DU JSON VERS UN GRAPHE
//------------------------------------
static JsonValue *get_json_field(const JsonValue *obj, const char *field)
{
    if (!obj || obj->type != JSON_OBJECT)
        return NULL;
    for (size_t i = 0; i < obj->as.object.count; i++)
    {
        if (strcmp(obj->as.object.entries[i].key, field) == 0)
            return obj->as.object.entries[i].value;
    }
    return NULL;
}

static LogisticsGraph *buildGraphFromJson(const JsonValue *root)
{
    if (!root || root->type != JSON_OBJECT)
        return NULL;

    JsonValue *nodesVal = get_json_field(root, "nodes");
    JsonValue *edgesVal = get_json_field(root, "edges");
    if (!nodesVal || nodesVal->type != JSON_ARRAY)
    {
        printf("Erreur : Le champ \"nodes\" est manquant ou mal formaté.\n");
        return NULL;
    }

    int nodeCount = (int)nodesVal->as.array.count;
    LogisticsGraph *graph = malloc(sizeof(LogisticsGraph));
    graph->nodeCount = nodeCount;
    graph->adjacencyLists = malloc(nodeCount * sizeof(AdjListNode *));
    graph->nodeNames = malloc(nodeCount * sizeof(char *));
    for (int i = 0; i < nodeCount; i++)
    {
        graph->adjacencyLists[i] = NULL;
    }

    // Traitement des nœuds : on suppose que chaque nœud est un objet avec "id" et "nom"
    for (int i = 0; i < nodeCount; i++)
    {
        JsonValue *nodeObj = nodesVal->as.array.items[i];
        if (!nodeObj || nodeObj->type != JSON_OBJECT)
            continue;
        int id = i;
        char *name = strdup("Unnamed");
        JsonValue *idField = get_json_field(nodeObj, "id");
        if (idField && idField->type == JSON_NUMBER)
        {
            id = (int)idField->as.numberValue;
        }
        JsonValue *nameField = get_json_field(nodeObj, "nom");
        if (nameField && nameField->type == JSON_STRING)
        {
            free(name);
            name = strdup(nameField->as.stringValue);
        }
        int index = id - 1;
        if (index < 0 || index >= nodeCount)
            index = i;
        graph->nodeNames[index] = name;
    }

    // Traitement des arêtes
    if (edgesVal && edgesVal->type == JSON_ARRAY)
    {
        for (size_t i = 0; i < edgesVal->as.array.count; i++)
        {
            JsonValue *edgeObj = edgesVal->as.array.items[i];
            if (!edgeObj || edgeObj->type != JSON_OBJECT)
                continue;
            int src = -1, dest = -1;
            EdgeAttr attr = {0};
            JsonValue *srcField = get_json_field(edgeObj, "source_id");
            JsonValue *destField = get_json_field(edgeObj, "destination_id");
            if (srcField && srcField->type == JSON_NUMBER)
                src = (int)srcField->as.numberValue - 1;
            if (destField && destField->type == JSON_NUMBER)
                dest = (int)destField->as.numberValue - 1;
            JsonValue *distField = get_json_field(edgeObj, "distance");
            if (distField && distField->type == JSON_NUMBER)
                attr.distance = (float)distField->as.numberValue;
            JsonValue *timeField = get_json_field(edgeObj, "temps_base");
            if (!timeField)
                timeField = get_json_field(edgeObj, "baseTime");
            if (timeField && timeField->type == JSON_NUMBER)
                attr.baseTime = (float)timeField->as.numberValue;
            JsonValue *coutField = get_json_field(edgeObj, "cout");
            if (!coutField)
                coutField = get_json_field(edgeObj, "cout_monetaire");
            if (coutField && coutField->type == JSON_NUMBER)
                attr.cost = (float)coutField->as.numberValue;
            JsonValue *typeField = get_json_field(edgeObj, "type_route");
            if (typeField && typeField->type == JSON_NUMBER)
                attr.roadType = (int)typeField->as.numberValue;
            JsonValue *fiabField = get_json_field(edgeObj, "fiabilite");
            if (fiabField && fiabField->type == JSON_NUMBER)
                attr.reliability = (float)fiabField->as.numberValue;
            JsonValue *restField = get_json_field(edgeObj, "restrictions");
            if (!restField)
                restField = get_json_field(edgeObj, "restrictions_bitmask");
            if (restField && restField->type == JSON_NUMBER)
                attr.restrictions = (int)restField->as.numberValue;

            if (src >= 0 && src < graph->nodeCount && dest >= 0 && dest < graph->nodeCount)
            {
                addEdge(graph, src, dest, attr);
            }
        }
    }

    return graph;
}

//------------------------------------
// PARTIE 5bis : CONVERSION DU XML VERS UN GRAPHE
//------------------------------------
// On suppose que le fichier XML suit la structure suivante :
// <reseau>
//   <nodes>
//      <node>
//         <id>1</id>
//         <nom>Base</nom>
//         ... autres informations ...
//      </node>
//      <node> ... </node>
//   </nodes>
//   <edges>
//      <edge>
//         <source_id>1</source_id>
//         <destination_id>2</destination_id>
//         <distance>375.0</distance>
//         <temps_base>300.0</temps_base>
//         <cout>10000.0</cout>
//         <type_route>0</type_route>
//         <fiabilite>1.0</fiabilite>
//         <restrictions>2</restrictions>
//      </edge>
//      <edge> ... </edge>
//   </edges>
// </reseau>
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

static LogisticsGraph *buildGraphFromXml(const XmlNode *root)
{
    if (!root)
        return NULL;
    XmlNode *nodesNode = NULL, *edgesNode = NULL;
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
        printf("Erreur : Pas de noeud <nodes> dans le fichier XML.\n");
        return NULL;
    }
    int nodeCount = (int)nodesNode->child_count;
    LogisticsGraph *graph = malloc(sizeof(LogisticsGraph));
    graph->nodeCount = nodeCount;
    graph->adjacencyLists = malloc(nodeCount * sizeof(AdjListNode *));
    graph->nodeNames = malloc(nodeCount * sizeof(char *));
    for (int i = 0; i < nodeCount; i++)
    {
        graph->adjacencyLists[i] = NULL;
        graph->nodeNames[i] = strdup("Unnamed");
    }
    // Traitement des noeuds : on suppose que chaque noeud est un <node>
    for (int i = 0; i < nodeCount; i++)
    {
        XmlNode *nodeObj = nodesNode->children[i];
        if (!nodeObj || strcmp(nodeObj->tag, "node") != 0)
            continue;
        int id = i;
        char *name = strdup("Unnamed");
        char *idText = get_xml_child_text(nodeObj, "id");
        if (idText)
            id = atoi(idText);
        char *nameText = get_xml_child_text(nodeObj, "nom");
        if (nameText)
        {
            free(name);
            name = strdup(nameText);
        }
        int index = id - 1;
        if (index < 0 || index >= nodeCount)
            index = i;
        graph->nodeNames[index] = name;
    }
    // Traitement des arêtes
    if (edgesNode)
    {
        for (size_t i = 0; i < edgesNode->child_count; i++)
        {
            XmlNode *edgeObj = edgesNode->children[i];
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
            char *timeText = get_xml_child_text(edgeObj, "temps_base");
            if (!timeText)
                timeText = get_xml_child_text(edgeObj, "baseTime");
            if (timeText)
                attr.baseTime = atof(timeText);
            char *coutText = get_xml_child_text(edgeObj, "cout");
            if (!coutText)
                coutText = get_xml_child_text(edgeObj, "cout_monetaire");
            if (coutText)
                attr.cost = atof(coutText);
            char *typeText = get_xml_child_text(edgeObj, "type_route");
            if (typeText)
                attr.roadType = atoi(typeText);
            char *fiabText = get_xml_child_text(edgeObj, "fiabilite");
            if (fiabText)
                attr.reliability = atof(fiabText);
            char *restText = get_xml_child_text(edgeObj, "restrictions");
            if (!restText)
                restText = get_xml_child_text(edgeObj, "restrictions_bitmask");
            if (restText)
                attr.restrictions = atoi(restText);
            if (src >= 0 && src < graph->nodeCount && dest >= 0 && dest < graph->nodeCount)
            {
                addEdge(graph, src, dest, attr);
            }
        }
    }
    return graph;
}

//------------------------------------
// PARTIE 6 : DFS DU GRAPHE
//------------------------------------
static void DFSUtil(LogisticsGraph *graph, int v, int *visited)
{
    visited[v] = 1;
    printf("Visite DFS du nœud %d: %s\n", v + 1, graph->nodeNames[v]);
    AdjListNode *adj = graph->adjacencyLists[v];
    while (adj)
    {
        if (!visited[adj->dest])
            DFSUtil(graph, adj->dest, visited);
        adj = adj->next;
    }
}

static void DFS(LogisticsGraph *graph, int start)
{
    int *visited = calloc(graph->nodeCount, sizeof(int));
    if (!visited)
    {
        fprintf(stderr, "Erreur d'allocation pour visited (DFS)\n");
        return;
    }
    printf("\n--- Parcours DFS ---\n");
    DFSUtil(graph, start, visited);
    free(visited);
}

//------------------------------------
// PARTIE 7 : BFS DU GRAPHE
//------------------------------------
static void BFS(LogisticsGraph *graph, int start)
{
    int *visited = calloc(graph->nodeCount, sizeof(int));
    if (!visited)
    {
        fprintf(stderr, "Erreur d'allocation pour visited (BFS)\n");
        return;
    }
    int *queue = malloc(graph->nodeCount * sizeof(int));
    if (!queue)
    {
        free(visited);
        fprintf(stderr, "Erreur d'allocation pour la file (BFS)\n");
        return;
    }
    int front = 0, rear = 0;

    queue[rear++] = start;
    visited[start] = 1;

    printf("\n--- Parcours BFS ---\n");
    while (front < rear)
    {
        int current = queue[front++];
        printf("Visite BFS du nœud %d: %s\n", current + 1, graph->nodeNames[current]);
        AdjListNode *adj = graph->adjacencyLists[current];
        while (adj)
        {
            if (!visited[adj->dest])
            {
                visited[adj->dest] = 1;
                queue[rear++] = adj->dest;
            }
            adj = adj->next;
        }
    }

    free(visited);
    free(queue);
}

//------------------------------------
// PARTIE 8 : CONVERSION DU GRAPHE EN MATRICE DE DISTANCES
//------------------------------------
static double **createDistanceMatrix(const LogisticsGraph *graph)
{
    int n = graph->nodeCount;
    double **matrix = malloc(n * sizeof(double *));
    for (int i = 0; i < n; i++)
    {
        matrix[i] = malloc(n * sizeof(double));
        for (int j = 0; j < n; j++)
        {
            matrix[i][j] = (i == j) ? 0.0 : INF;
        }
        AdjListNode *edge = graph->adjacencyLists[i];
        while (edge)
        {
            int j = edge->dest;
            double cost = edge->attr.baseTime; // Utilisation de baseTime comme coût
            if (cost < matrix[i][j])
                matrix[i][j] = cost;
            edge = edge->next;
        }
    }
    return matrix;
}

//------------------------------------
// PARTIE 9 : FLOYD WARSHALL
//------------------------------------
static void floydWarshall(int n, double **matrix)
{
    for (int k = 0; k < n; k++)
    {
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                double newDist = matrix[i][k] + matrix[k][j];
                if (newDist < matrix[i][j])
                    matrix[i][j] = newDist;
            }
        }
    }
}

static void printDistanceMatrix(int n, double **matrix)
{
    printf("\n--- Matrice des plus courts chemins (coût = baseTime) ---\n");
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if (matrix[i][j] >= INF)
                printf("%7s ", "INF");
            else
                printf("%7.2f ", matrix[i][j]);
        }
        printf("\n");
    }
}

static void freeDistanceMatrix(int n, double **matrix)
{
    for (int i = 0; i < n; i++)
        free(matrix[i]);
    free(matrix);
}

//------------------------------------
// PARTIE 10 : LIBÉRATION DU GRAPHE
//------------------------------------
static void freeGraph(LogisticsGraph *graph)
{
    if (!graph)
        return;
    for (int i = 0; i < graph->nodeCount; i++)
    {
        AdjListNode *cur = graph->adjacencyLists[i];
        while (cur)
        {
            AdjListNode *temp = cur;
            cur = cur->next;
            free(temp);
        }
        free(graph->nodeNames[i]);
    }
    free(graph->nodeNames);
    free(graph->adjacencyLists);
    free(graph);
}

//------------------------------------
// PARTIE 11 : MAIN
//------------------------------------
int main(void)
{
    char input_filename[256];
    printf("Entrez le nom du fichier d'entree (ex: data.json ou data.xml) : ");
    scanf("%255s", input_filename);

    char *file_content = read_file(input_filename);
    if (!file_content)
    {
        fprintf(stderr, "Erreur lors de la lecture du fichier %s\n", input_filename);
        system("pause");
        return 1;
    }

    // Gestion du BOM UTF-8 pour JSON
    if ((unsigned char)file_content[0] == 0xEF &&
        (unsigned char)file_content[1] == 0xBB &&
        (unsigned char)file_content[2] == 0xBF)
    {
        file_content += 3;
    }

    const char *p = file_content;
    p = skip_whitespace(p);

    LogisticsGraph *graph = NULL;

    // Détection du format : JSON si { ou [, XML si <
    if (*p == '{' || *p == '[')
    {
        int token_count = 0;
        Token *tokens = tokenize(file_content, &token_count);
        int index = 0;
        JsonValue *json_root = parse_json(tokens, &index, token_count);
        printf("\n--- Structure JSON construite ---\n");
        print_json(json_root, 0);
        graph = buildGraphFromJson(json_root);
        free_tokens(tokens, token_count);
        free_json(json_root);
    }
    else if (*p == '<')
    {
        // Traitement XML
        XmlNode *xml_root = parse_xml_element(&p);
        printf("\n--- Structure XML construite ---\n");
        print_xml(xml_root, 0);
        graph = buildGraphFromXml(xml_root);
        free_xml(xml_root);
    }
    else
    {
        printf("Format de fichier non reconnu ou non supporté pour la conversion en graphe.\n");
        system("pause");
        return 1;
    }

    if (!graph)
    {
        printf("Echec de la conversion en graphe.\n");
        system("pause");
        return 1;
    }

    // Affichage du graphe
    printf("\n--- Graphe converti ---\n");
    printf("Nombre de nœuds : %d\n", graph->nodeCount);
    for (int i = 0; i < graph->nodeCount; i++)
    {
        printf("Nœud %d : %s\n", i + 1, graph->nodeNames[i]);
        AdjListNode *adj = graph->adjacencyLists[i];
        while (adj)
        {
            printf("  -> Vers nœud %d | Distance: %.2f, Temps: %.2f, Cout: %.2f\n",
                   adj->dest + 1, adj->attr.distance, adj->attr.baseTime, adj->attr.cost);
            adj = adj->next;
        }
    }

    // Parcours DFS et BFS
    DFS(graph, 0);
    BFS(graph, 0);

    // Conversion en matrice de distances et application de Floyd Warshall
    double **distanceMatrix = createDistanceMatrix(graph);
    floydWarshall(graph->nodeCount, distanceMatrix);
    printDistanceMatrix(graph->nodeCount, distanceMatrix);
    freeDistanceMatrix(graph->nodeCount, distanceMatrix);

    // Nettoyage
    freeGraph(graph);

    printf("Appuyez sur une touche pour continuer...\n");
    system("pause");
    return 0;
}
