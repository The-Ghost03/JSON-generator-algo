#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

//////////////////////////////
// PARTIE 1 : PARSING JSON
//////////////////////////////

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

#define INITIAL_TOKEN_CAPACITY 100

static Token create_token(TokenType type, const char *value)
{
    Token t;
    t.type = type;
    if (value != NULL)
        t.value = strdup(value);
    else
        t.value = NULL;
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
    int capacity = INITIAL_TOKEN_CAPACITY;
    Token *tokens = malloc(sizeof(Token) * capacity);
    int count = 0;
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

//////////////////////////////
// PARTIE 2 : PARSING XML (minimal)
// (Pour cet exemple, nous nous concentrons sur la conversion du JSON, mais le XML serait analogue)
//////////////////////////////
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

// (Les fonctions de parsing XML – xml_skip_whitespace, xml_parse_tag, etc. – sont identiques à celles présentées précédemment)
// Pour cet exemple, nous nous concentrerons sur la conversion du JSON vers le graphe.

//////////////////////////////
// PARTIE 3 : LECTURE DE FICHIER
//////////////////////////////
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

//////////////////////////////
// PARTIE 4 : STRUCTURE DE GRAPHE
//////////////////////////////

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

//////////////////////////////
// PARTIE 5 : CONVERSION DU JSON VERS UN GRAPHE
//////////////////////////////

// Fonction pour rechercher une entrée dans un objet JSON par clé
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

// Conversion du JSON en LogisticsGraph
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
        int id = i; // Par défaut, utilisation de l'indice
        char *name = strdup("Unnamed");
        // Lecture des champs du nœud
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
        // Stockage dans le graphe ; on suppose ici que les id sont consécutifs et 1-indexés
        int index = id - 1;
        if (index < 0 || index >= nodeCount)
            index = i; // Sinon, utiliser l'indice courant
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
            // Extraction des champs
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

//////////////////////////////
// PARTIE 6 : DFS DU GRAPHE
//////////////////////////////

static void DFSUtil(LogisticsGraph *graph, int v, int *visited)
{
    visited[v] = 1;
    printf("Visite du nœud %d: %s\n", v + 1, graph->nodeNames[v]);
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
        fprintf(stderr, "Erreur allocation mémoire pour visited\n");
        return;
    }
    printf("Début du DFS à partir du nœud %d: %s\n", start + 1, graph->nodeNames[start]);
    DFSUtil(graph, start, visited);
    free(visited);
}

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

//////////////////////////////
// PARTIE 7 : MAIN
//////////////////////////////
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

    // Gestion du BOM UTF-8
    if ((unsigned char)file_content[0] == 0xEF &&
        (unsigned char)file_content[1] == 0xBB &&
        (unsigned char)file_content[2] == 0xBF)
    {
        file_content += 3;
    }

    const char *p = file_content;
    p = skip_whitespace(p);

    // Pour cet exemple, nous nous concentrons sur le format JSON.
    // (La conversion XML serait analogue, en créant une fonction buildGraphFromXml.)
    if (*p != '{' && *p != '[')
    {
        printf("Format de fichier non reconnu ou non supporté pour la conversion en graphe.\n");
        system("pause");
        return 1;
    }

    int token_count = 0;
    Token *tokens = tokenize(file_content, &token_count);
    int index = 0;
    JsonValue *json_root = parse_json(tokens, &index, token_count);
    printf("\n--- Structure JSON construite ---\n");
    print_json(json_root, 0);

    // Conversion en graphe
    LogisticsGraph *graph = buildGraphFromJson(json_root);
    if (!graph)
    {
        printf("Echec de la conversion en graphe.\n");
        free_tokens(tokens, token_count);
        free_json(json_root);
        system("pause");
        return 1;
    }

    // Affichage des informations du graphe
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

    // Lancement du DFS sur le graphe (à partir du premier nœud)
    printf("\n--- Parcours DFS ---\n");
    DFS(graph, 0);

    // Nettoyage
    free_tokens(tokens, token_count);
    free_json(json_root);
    freeGraph(graph);

    printf("Appuyez sur une touche pour continuer...\n");
    system("pause");
    return 0;
}
