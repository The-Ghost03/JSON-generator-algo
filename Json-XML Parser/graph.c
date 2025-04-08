#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define INF 1e9
#define INITIAL_TOKEN_CAPACITY 100

/**********************************
 * MODULE DE PARSING JSON
 **********************************/
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
    (*s)++; // saut du guillemet ouvrant
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
        (*s)++; // saut du guillemet fermant
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

/**********************************
 * STRUCTURES JSON GÉNÉRIQUES
 **********************************/
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
        break;
    }
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
        break;
    }
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

/**********************************
 * MODULE DE PARSING XML (minimal)
 **********************************/
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
    {
        (*p)++;
    }
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

/**********************************
 * LECTURE DE FICHIER
 **********************************/
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

/**********************************
 * STRUCTURES DU GRAPHE (NOUVELLES STRUCTURES)
 **********************************/
typedef struct
{
    int id;               // Identifiant unique du nœud
    char *name;           // Nom du nœud (ex: "Hub Abidjan")
    char *type;           // Type de nœud ("hub", "relay", "delivery", etc.)
    float coordinates[2]; // Coordonnées [latitude, longitude]
    int capacity;         // Capacité du nœud
    // Facteurs de congestion pour moduler le temps de parcours
    float congestion_morning;
    float congestion_afternoon;
    float congestion_night;
} Node;

typedef struct
{
    float distance;    // Distance en km
    float baseTime;    // Temps de parcours de base en minutes
    float cost;        // Coût de l'arête
    int roadType;      // Type de route (ex: 0: asphalte, 1: latérite, etc.)
    float reliability; // Fiabilité (0..1)
    int restrictions;  // Restrictions (par exemple en bits)
    int weatherType;   // Type de météo (0 = normal, 1 = pluie, 2 = vent, ...)
} EdgeAttr;

typedef struct AdjListNode
{
    int dest;
    EdgeAttr attr;
    struct AdjListNode *next;
} AdjListNode;

typedef struct
{
    AdjListNode *head;
} AdjList;

typedef struct
{
    int V;
    Node *nodes;
    AdjList *array;
} Graph;

/**********************************
 * FONCTIONS DE MANIPULATION DU GRAPHE
 **********************************/
Graph *createGraph(int V)
{
    Graph *graph = malloc(sizeof(Graph));
    if (!graph)
    {
        fprintf(stderr, "Erreur d'allocation pour Graph.\n");
        exit(EXIT_FAILURE);
    }
    graph->V = V;
    graph->nodes = malloc(V * sizeof(Node));
    if (!graph->nodes)
    {
        fprintf(stderr, "Erreur d'allocation pour nodes.\n");
        exit(EXIT_FAILURE);
    }
    // Initialisation des nœuds avec valeurs par défaut
    for (int i = 0; i < V; i++)
    {
        graph->nodes[i].id = i + 1;
        graph->nodes[i].name = NULL;
        graph->nodes[i].type = strdup("undefined");
        graph->nodes[i].coordinates[0] = 0.0f;
        graph->nodes[i].coordinates[1] = 0.0f;
        graph->nodes[i].capacity = 0;
        graph->nodes[i].congestion_morning = 1.0f;
        graph->nodes[i].congestion_afternoon = 1.0f;
        graph->nodes[i].congestion_night = 1.0f;
    }
    graph->array = malloc(V * sizeof(AdjList));
    if (!graph->array)
    {
        fprintf(stderr, "Erreur d'allocation pour listes d'adjacence.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < V; i++)
    {
        graph->array[i].head = NULL;
    }
    return graph;
}

void addEdgeToGraph(Graph *graph, int src, int dest, EdgeAttr attr)
{
    if (src < 0 || src >= graph->V || dest < 0 || dest >= graph->V)
    {
        fprintf(stderr, "Indices de nœud invalides dans addEdgeToGraph.\n");
        return;
    }
    AdjListNode *newNode = malloc(sizeof(AdjListNode));
    if (!newNode)
    {
        fprintf(stderr, "Erreur d'allocation pour AdjListNode.\n");
        exit(EXIT_FAILURE);
    }
    newNode->dest = dest;
    newNode->attr = attr;
    newNode->next = graph->array[src].head;
    graph->array[src].head = newNode;
}

void removeEdgeFromGraph(Graph *graph, int src, int dest)
{
    if (src < 0 || src >= graph->V)
        return;
    AdjListNode *curr = graph->array[src].head;
    AdjListNode *prev = NULL;
    while (curr)
    {
        if (curr->dest == dest)
        {
            if (prev == NULL)
                graph->array[src].head = curr->next;
            else
                prev->next = curr->next;
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

void addNode(Graph **graphPtr, const char *name, float cong_morning, float cong_afternoon, float cong_night)
{
    Graph *graph = *graphPtr;
    int newV = graph->V + 1;
    Node *newNodes = realloc(graph->nodes, newV * sizeof(Node));
    if (!newNodes)
    {
        fprintf(stderr, "Erreur de réallocation pour nodes.\n");
        return;
    }
    graph->nodes = newNodes;
    AdjList *newAdj = realloc(graph->array, newV * sizeof(AdjList));
    if (!newAdj)
    {
        fprintf(stderr, "Erreur de réallocation pour listes d'adjacence.\n");
        return;
    }
    graph->array = newAdj;
    // Initialisation du nouveau nœud
    graph->nodes[newV - 1].id = newV;
    graph->nodes[newV - 1].name = strdup(name);
    graph->nodes[newV - 1].type = strdup("undefined");
    graph->nodes[newV - 1].coordinates[0] = 0.0f;
    graph->nodes[newV - 1].coordinates[1] = 0.0f;
    graph->nodes[newV - 1].capacity = 0;
    graph->nodes[newV - 1].congestion_morning = cong_morning;
    graph->nodes[newV - 1].congestion_afternoon = cong_afternoon;
    graph->nodes[newV - 1].congestion_night = cong_night;
    graph->array[newV - 1].head = NULL;
    graph->V = newV;
}

void removeNode(Graph *graph, int node)
{
    if (node < 0 || node >= graph->V)
        return;
    // Libérer la liste d'adjacence du nœud
    AdjListNode *curr = graph->array[node].head;
    while (curr)
    {
        AdjListNode *temp = curr;
        curr = curr->next;
        free(temp);
    }
    graph->array[node].head = NULL;
    // Supprimer les arêtes entrantes dans les autres nœuds
    for (int i = 0; i < graph->V; i++)
    {
        if (i == node)
            continue;
        removeEdgeFromGraph(graph, i, node);
    }
    free(graph->nodes[node].name);
    free(graph->nodes[node].type);
    graph->nodes[node].name = NULL;
}

/**********************************
 * CONVERSION DU JSON/VERS UN GRAPHE
 **********************************/
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

Graph *buildGraphFromJson(const JsonValue *root)
{
    if (!root || root->type != JSON_OBJECT)
        return NULL;
    JsonValue *nodesVal = get_json_field(root, "nodes");
    JsonValue *edgesVal = get_json_field(root, "edges");
    if (!nodesVal || nodesVal->type != JSON_ARRAY)
    {
        printf("Erreur : champ 'nodes' manquant ou invalide.\n");
        return NULL;
    }
    int nodeCount = (int)nodesVal->as.array.count;
    Graph *graph = createGraph(nodeCount);
    // Traitement des nœuds
    for (int i = 0; i < nodeCount; i++)
    {
        JsonValue *nodeObj = nodesVal->as.array.items[i];
        if (!nodeObj || nodeObj->type != JSON_OBJECT)
            continue;
        // Extraction du champ id
        int id = i + 1;
        JsonValue *idField = get_json_field(nodeObj, "id");
        if (idField && idField->type == JSON_NUMBER)
            id = (int)idField->as.numberValue;
        // Extraction du nom
        char *name = strdup("Unnamed");
        JsonValue *nameField = get_json_field(nodeObj, "nom");
        if (nameField && nameField->type == JSON_STRING)
        {
            free(name);
            name = strdup(nameField->as.stringValue);
        }
        // Extraction du type
        char *type = strdup("undefined");
        JsonValue *typeField = get_json_field(nodeObj, "type");
        if (typeField && typeField->type == JSON_STRING)
        {
            free(type);
            type = strdup(typeField->as.stringValue);
        }
        // Extraction de la capacité
        int capacity = 0;
        JsonValue *capField = get_json_field(nodeObj, "capacity");
        if (capField && capField->type == JSON_NUMBER)
            capacity = (int)capField->as.numberValue;
        // Extraction des coordonnées (attendu comme array avec au moins 2 nombres)
        float coord[2] = {0.0f, 0.0f};
        JsonValue *coordField = get_json_field(nodeObj, "coordinates");
        if (coordField && coordField->type == JSON_ARRAY && coordField->as.array.count >= 2)
        {
            JsonValue *latField = coordField->as.array.items[0];
            JsonValue *lonField = coordField->as.array.items[1];
            if (latField && latField->type == JSON_NUMBER)
                coord[0] = (float)latField->as.numberValue;
            if (lonField && lonField->type == JSON_NUMBER)
                coord[1] = (float)lonField->as.numberValue;
        }
        // Extraction des indices de congestion
        float cong_morning = 1.0f, cong_afternoon = 1.0f, cong_night = 1.0f;
        JsonValue *morningField = get_json_field(nodeObj, "congestion_morning");
        if (morningField && morningField->type == JSON_NUMBER)
            cong_morning = (float)morningField->as.numberValue;
        JsonValue *afternoonField = get_json_field(nodeObj, "congestion_afternoon");
        if (afternoonField && afternoonField->type == JSON_NUMBER)
            cong_afternoon = (float)afternoonField->as.numberValue;
        JsonValue *nightField = get_json_field(nodeObj, "congestion_night");
        if (nightField && nightField->type == JSON_NUMBER)
            cong_night = (float)nightField->as.numberValue;
        // Utilisation de l'indice basé sur id si cohérent
        int index = id - 1;
        if (index < 0 || index >= nodeCount)
            index = i;
        // Affectation dans le graphe
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
            JsonValue *costField = get_json_field(edgeObj, "cout");
            if (!costField)
                costField = get_json_field(edgeObj, "cout_monetaire");
            if (costField && costField->type == JSON_NUMBER)
                attr.cost = (float)costField->as.numberValue;
            JsonValue *typeFieldEdge = get_json_field(edgeObj, "type_route");
            if (typeFieldEdge && typeFieldEdge->type == JSON_NUMBER)
                attr.roadType = (int)typeFieldEdge->as.numberValue;
            JsonValue *fiabField = get_json_field(edgeObj, "fiabilite");
            if (fiabField && fiabField->type == JSON_NUMBER)
                attr.reliability = (float)fiabField->as.numberValue;
            JsonValue *restField = get_json_field(edgeObj, "restrictions");
            if (!restField)
                restField = get_json_field(edgeObj, "restrictions_bitmask");
            if (restField && restField->type == JSON_NUMBER)
                attr.restrictions = (int)restField->as.numberValue;
            attr.weatherType = 0;
            JsonValue *weatherField = get_json_field(edgeObj, "weatherType");
            if (weatherField && weatherField->type == JSON_NUMBER)
                attr.weatherType = (int)weatherField->as.numberValue;

            if (src >= 0 && src < graph->V && dest >= 0 && dest < graph->V)
            {
                addEdgeToGraph(graph, src, dest, attr);
            }
        }
    }
    return graph;
}

/**********************************
 * CONVERSION DU XML VERS UN GRAPHE
 **********************************/
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

Graph *buildGraphFromXml(const XmlNode *root)
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
        printf("Erreur : Balise <nodes> absente dans le fichier XML.\n");
        return NULL;
    }
    int nodeCount = (int)nodesNode->child_count;
    Graph *graph = createGraph(nodeCount);
    // Traitement des nœuds
    for (int i = 0; i < nodeCount; i++)
    {
        XmlNode *nodeObj = nodesNode->children[i];
        if (!nodeObj || strcmp(nodeObj->tag, "node") != 0)
            continue;
        int id = i + 1;
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
        float coord[2] = {0.0f, 0.0f};
        char *latText = get_xml_child_text(nodeObj, "latitude");
        char *lonText = get_xml_child_text(nodeObj, "longitude");
        if (latText)
            coord[0] = atof(latText);
        if (lonText)
            coord[1] = atof(lonText);
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

/**********************************
 * FONCTIONS DE PARCOURS : DFS et BFS
 **********************************/
static void DFSUtil(Graph *graph, int v, int *visited)
{
    visited[v] = 1;
    printf("DFS visite nœud %d: %s\n", v + 1, graph->nodes[v].name);
    AdjListNode *adj = graph->array[v].head;
    while (adj)
    {
        if (!visited[adj->dest])
            DFSUtil(graph, adj->dest, visited);
        adj = adj->next;
    }
}

static void DFS(Graph *graph, int start)
{
    int *visited = calloc(graph->V, sizeof(int));
    if (!visited)
    {
        fprintf(stderr, "Erreur d'allocation pour visited (DFS).\n");
        return;
    }
    printf("\n--- Parcours DFS ---\n");
    DFSUtil(graph, start, visited);
    free(visited);
}

static void BFS(Graph *graph, int start)
{
    int *visited = calloc(graph->V, sizeof(int));
    if (!visited)
    {
        fprintf(stderr, "Erreur d'allocation pour visited (BFS).\n");
        return;
    }
    int *queue = malloc(graph->V * sizeof(int));
    if (!queue)
    {
        free(visited);
        fprintf(stderr, "Erreur d'allocation pour la file (BFS).\n");
        return;
    }
    int front = 0, rear = 0;
    queue[rear++] = start;
    visited[start] = 1;
    printf("\n--- Parcours BFS ---\n");
    while (front < rear)
    {
        int current = queue[front++];
        printf("BFS visite nœud %d: %s\n", current + 1, graph->nodes[current].name);
        AdjListNode *adj = graph->array[current].head;
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

/**********************************
 * MATRICE DE DISTANCES & FLOYD-WARSHALL
 **********************************/
static double **createDistanceMatrix(const Graph *graph)
{
    int n = graph->V;
    double **matrix = malloc(n * sizeof(double *));
    for (int i = 0; i < n; i++)
    {
        matrix[i] = malloc(n * sizeof(double));
        for (int j = 0; j < n; j++)
        {
            matrix[i][j] = (i == j) ? 0.0 : INF;
        }
        AdjListNode *edge = graph->array[i].head;
        while (edge)
        {
            int j = edge->dest;
            // Pour un temps effectif, appliquer ici congestion et météo si besoin
            double cost = edge->attr.baseTime;
            if (cost < matrix[i][j])
                matrix[i][j] = cost;
            edge = edge->next;
        }
    }
    return matrix;
}

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

/**********************************
 * MAIN : Fonctionnement du module
 **********************************/
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

    Graph *graph = NULL;
    // Détection du format d'entrée
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
        printf("Échec de la conversion en graphe.\n");
        system("pause");
        return 1;
    }

    // Affichage du graphe initial
    printf("\n--- Graphe converti ---\n");
    printf("Nombre de nœuds : %d\n", graph->V);
    for (int i = 0; i < graph->V; i++)
    {
        if (graph->nodes[i].name)
            printf("Nœud %d : %s, Type: %s\n", i + 1, graph->nodes[i].name, graph->nodes[i].type);
        AdjListNode *adj = graph->array[i].head;
        while (adj)
        {
            printf("  -> Vers nœud %d | Distance: %.2f, Temps: %.2f, Coût: %.2f, WeatherType: %d\n",
                   adj->dest + 1, adj->attr.distance, adj->attr.baseTime, adj->attr.cost, adj->attr.weatherType);
            adj = adj->next;
        }
    }

    // Ajout d'un nouveau nœud et d'une arête vers ce nœud
    addNode(&graph, "Nœud 6", 1.0f, 1.0f, 1.0f);
    EdgeAttr attr = {500.0f, 400.0f, 8000.0f, 0, 1.0f, 1, 1}; // weatherType = 1 (ex : pluie)
    addEdgeToGraph(graph, 0, graph->V - 1, attr);

    printf("\n--- Graphe modifié ---\n");
    printf("Nombre de nœuds : %d\n", graph->V);
    for (int i = 0; i < graph->V; i++)
    {
        if (graph->nodes[i].name)
            printf("Nœud %d : %s, Type: %s\n", i + 1, graph->nodes[i].name, graph->nodes[i].type);
        AdjListNode *adj = graph->array[i].head;
        while (adj)
        {
            printf("  -> Vers nœud %d | Distance: %.2f, Temps: %.2f, Coût: %.2f, WeatherType: %d\n",
                   adj->dest + 1, adj->attr.distance, adj->attr.baseTime, adj->attr.cost, adj->attr.weatherType);
            adj = adj->next;
        }
    }

    // Suppression d'une arête (exemple : supprimer l'arête de nœud 1 vers nœud 4)
    removeEdgeFromGraph(graph, 0, 3);
    // Suppression d'un nœud (exemple : supprimer le nœud 2)
    removeNode(graph, 1);

    DFS(graph, 0);
    BFS(graph, 0);

    double **distanceMatrix = createDistanceMatrix(graph);
    floydWarshall(graph->V, distanceMatrix);
    printDistanceMatrix(graph->V, distanceMatrix);
    freeDistanceMatrix(graph->V, distanceMatrix);

    // Libération du graphe
    for (int i = 0; i < graph->V; i++)
    {
        if (graph->nodes[i].name)
        {
            free(graph->nodes[i].name);
            free(graph->nodes[i].type);
        }
    }
    free(graph->nodes);
    for (int i = 0; i < graph->V; i++)
    {
        AdjListNode *cur = graph->array[i].head;
        while (cur)
        {
            AdjListNode *temp = cur;
            cur = cur->next;
            free(temp);
        }
    }
    free(graph->array);
    free(graph);

    printf("Appuyez sur une touche pour continuer...\n");
    system("pause");
    return 0;
}
