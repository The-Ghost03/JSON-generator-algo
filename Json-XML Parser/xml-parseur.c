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
    {
        printf("  ");
    }
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

//////////////////////////////
// PARTIE 2 : PARSING XML (minimal)
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

static void xml_skip_whitespace(const char **p)
{
    while (**p && isspace((unsigned char)**p))
    {
        (*p)++;
    }
}

static char *xml_parse_tag(const char **p)
{
    xml_skip_whitespace(p);
    const char *start = *p;
    while (**p && !isspace((unsigned char)**p) && **p != '>' && **p != '/')
    {
        (*p)++;
    }
    int len = (int)(*p - start);
    char *tag = (char *)malloc(len + 1);
    strncpy(tag, start, len);
    tag[len] = '\0';
    return tag;
}

static XmlAttribute *xml_parse_attributes(const char **p, size_t *attr_count)
{
    size_t capacity = 4;
    XmlAttribute *attrs = (XmlAttribute *)malloc(capacity * sizeof(XmlAttribute));
    *attr_count = 0;
    xml_skip_whitespace(p);
    while (**p && **p != '>' && **p != '/')
    {
        const char *start = *p;
        while (**p && !isspace((unsigned char)**p) && **p != '=')
            (*p)++;
        int len = (int)(*p - start);
        char *key = (char *)malloc(len + 1);
        strncpy(key, start, len);
        key[len] = '\0';
        xml_skip_whitespace(p);
        if (**p == '=')
            (*p)++;
        xml_skip_whitespace(p);
        char quote = **p;
        char *value = NULL;
        if (quote == '"' || quote == '\'')
        {
            (*p)++;
            const char *val_start = *p;
            while (**p && **p != quote)
                (*p)++;
            int vlen = (int)(*p - val_start);
            value = (char *)malloc(vlen + 1);
            strncpy(value, val_start, vlen);
            value[vlen] = '\0';
            if (**p == quote)
                (*p)++;
        }
        if (*attr_count >= capacity)
        {
            capacity *= 2;
            attrs = (XmlAttribute *)realloc(attrs, capacity * sizeof(XmlAttribute));
        }
        attrs[*attr_count].key = key;
        attrs[*attr_count].value = value;
        (*attr_count)++;
        xml_skip_whitespace(p);
    }
    return attrs;
}

static char *xml_parse_text(const char **p)
{
    const char *start = *p;
    while (**p && **p != '<')
        (*p)++;
    int len = (int)(*p - start);
    char *text = (char *)malloc(len + 1);
    strncpy(text, start, len);
    text[len] = '\0';
    return text;
}

static XmlNode *parse_xml_element(const char **p);

static XmlNode *parse_xml_element(const char **p)
{
    xml_skip_whitespace(p);
    if (**p != '<')
        return NULL;
    (*p)++; // saute '<'
    if (**p == '/')
        return NULL;
    char *tag = xml_parse_tag(p);
    XmlNode *node = (XmlNode *)malloc(sizeof(XmlNode));
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
    xml_skip_whitespace(p);
    size_t children_capacity = 4;
    node->children = (XmlNode **)malloc(children_capacity * sizeof(XmlNode *));
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
                    node->children = (XmlNode **)realloc(node->children, children_capacity * sizeof(XmlNode *));
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
                    char *old = node->text;
                    node->text = (char *)malloc(strlen(old) + strlen(text) + 1);
                    strcpy(node->text, old);
                    strcat(node->text, text);
                    free(old);
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
    if (node->child_count == 0 && (node->text == NULL || strlen(node->text) == 0))
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
        for (int j = 0; j < indent; j++)
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
    free(node->text);
    for (size_t i = 0; i < node->child_count; i++)
    {
        free_xml(node->children[i]);
    }
    free(node->children);
    free(node);
}

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
    char *buffer = (char *)malloc(filesize + 1);
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
// PARTIE 4 : LISTE CHAÎNÉE GÉNÉRIQUE DES ÉLÉMENTS
//////////////////////////////
typedef struct ElementRecord
{
    char *key;     // Pour JSON : la clé ou pseudo-clé ; pour XML : le tag
    char *content; // Contenu textuel ou "complex"
    struct ElementRecord *next;
} ElementRecord;

static ElementRecord *create_element_record(const char *key, const char *content)
{
    ElementRecord *elem = (ElementRecord *)malloc(sizeof(ElementRecord));
    elem->key = key ? strdup(key) : NULL;
    elem->content = content ? strdup(content) : strdup("complex");
    elem->next = NULL;
    return elem;
}

static void append_element(ElementRecord **head, ElementRecord *newElem)
{
    if (*head == NULL)
    {
        *head = newElem;
    }
    else
    {
        ElementRecord *temp = *head;
        while (temp->next)
            temp = temp->next;
        temp->next = newElem;
    }
}

static void print_element_list(const ElementRecord *head)
{
    while (head)
    {
        printf("Key: %s\n", head->key ? head->key : "NULL");
        printf("Content: %s\n", head->content ? head->content : "complex");
        printf("--------------------------\n");
        head = head->next;
    }
}

static void free_element_list(ElementRecord *head)
{
    while (head)
    {
        ElementRecord *temp = head;
        head = head->next;
        free(temp->key);
        free(temp->content);
        free(temp);
    }
}

//////////////////////////////
// PARTIE 5 : EXTRACTION RÉCURSIVE DES ÉLÉMENTS
//////////////////////////////

// Extraction récursive pour JSON
static void extract_all_json_elements(const char *parentKey,
                                      const JsonValue *val,
                                      ElementRecord **list)
{
    if (!val)
        return;

    char buffer[256];
    switch (val->type)
    {
    case JSON_STRING:
        snprintf(buffer, sizeof(buffer), "\"%s\"", val->as.stringValue);
        break;
    case JSON_NUMBER:
        snprintf(buffer, sizeof(buffer), "%f", val->as.numberValue);
        break;
    case JSON_BOOL:
        snprintf(buffer, sizeof(buffer), "%s", val->as.boolValue ? "true" : "false");
        break;
    case JSON_NULL:
        snprintf(buffer, sizeof(buffer), "null");
        break;
    default:
        snprintf(buffer, sizeof(buffer), "complex");
        break;
    }

    ElementRecord *elem = create_element_record(parentKey, buffer);
    append_element(list, elem);

    if (val->type == JSON_OBJECT)
    {
        for (size_t i = 0; i < val->as.object.count; i++)
        {
            const char *childKey = val->as.object.entries[i].key;
            const JsonValue *childVal = val->as.object.entries[i].value;
            extract_all_json_elements(childKey, childVal, list);
        }
    }
    else if (val->type == JSON_ARRAY)
    {
        for (size_t i = 0; i < val->as.array.count; i++)
        {
            char arrayKey[64];
            snprintf(arrayKey, sizeof(arrayKey), "[%zu]", i);
            extract_all_json_elements(arrayKey, val->as.array.items[i], list);
        }
    }
}

// Extraction récursive pour XML
static void extract_all_xml_elements(const XmlNode *node, ElementRecord **list)
{
    if (!node)
        return;

    char buffer[256];
    if (node->text && strlen(node->text) > 0)
    {
        snprintf(buffer, sizeof(buffer), "%s", node->text);
    }
    else if (node->child_count > 0)
    {
        snprintf(buffer, sizeof(buffer), "complex");
    }
    else
    {
        snprintf(buffer, sizeof(buffer), "");
    }

    ElementRecord *elem = create_element_record(node->tag, buffer);
    append_element(list, elem);

    for (size_t i = 0; i < node->child_count; i++)
    {
        extract_all_xml_elements(node->children[i], list);
    }
}

//////////////////////////////
// PARTIE 6 : MAIN HYBRIDE
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

    ElementRecord *elementList = NULL;

    if (*p == '<')
    {
        // Traitement XML
        // Sauter le prologue <?xml ... ?> si présent
        if (strncmp(p, "<?xml", 5) == 0)
        {
            p += 5;
            while (*p && !(p[0] == '?' && p[1] == '>'))
                p++;
            if (*p)
                p += 2;
            p = skip_whitespace(p);
        }
        XmlNode *xml_root = parse_xml_element(&p);
        printf("\n--- Structure XML construite ---\n");
        print_xml(xml_root, 0);

        extract_all_xml_elements(xml_root, &elementList);
        free_xml(xml_root);
    }
    else if (*p == '{' || *p == '[')
    {
        // Traitement JSON
        int token_count = 0;
        Token *tokens = tokenize(file_content, &token_count);
        int index = 0;
        JsonValue *json_root = parse_json(tokens, &index, token_count);
        printf("\n--- Structure JSON construite ---\n");
        print_json(json_root, 0);

        extract_all_json_elements("root", json_root, &elementList);
        free_tokens(tokens, token_count);
        free_json(json_root);
    }
    else
    {
        printf("Format de fichier non reconnu.\n");
    }

    free(file_content);

    printf("\n--- Liste chainee des elements principaux ---\n");
    print_element_list(elementList);
    free_element_list(elementList);

    printf("Appuyez sur une touche pour continuer...\n");
    system("pause");
    return 0;
}
