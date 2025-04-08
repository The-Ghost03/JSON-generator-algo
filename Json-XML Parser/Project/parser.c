/* parser.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"

/* --- MODULE DE PARSING JSON --- */

/* Ici on place l'implémentation des fonctions de parsing JSON.
   Par exemple, tokenize, parse_json, parse_object, parse_array, print_json, free_json, etc.
   Pour la clarté de cet exemple, nous n'incluons que les fonctions déjà présentées précédemment. */

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

#define INITIAL_TOKEN_CAPACITY 100

static Token create_token(TokenType type, const char *value)
{
    Token t;
    t.type = type;
    t.value = value ? strdup(value) : NULL;
    return t;
}

static const char *skip_whitespace(const char *s)
{
    while (*s && isspace((unsigned char)*s))
        s++;
    return s;
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
        (*s)++;
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

/* Ici, les fonctions parse_json, parse_array, parse_object, print_json et free_json
   seront les mêmes que celles présentées précédemment. Pour la concision, elles ne sont pas
   affichées en intégralité ici. Vous pouvez y reporter vos implémentations existantes. */

/* Fonction principale du parseur JSON à appeler */
JsonValue *parse_json_file(const char *fileContent)
{
    int token_count = 0;
    Token *tokens = tokenize(fileContent, &token_count);
    int index = 0;
    JsonValue *root = /* Appel à votre fonction parse_json */ NULL;
    /* Placez ici l'appel à parse_json(tokens, &index, token_count) */
    /* N'oubliez pas de libérer les tokens par la suite */
    return root;
}

/* Pour le parsing XML, adaptez de manière similaire. */
Graph *buildGraphFromJson(const JsonValue *root)
{
    /* Implémentez la conversion de la structure JSON en Graph (comme dans vos précédentes fonctions) */
    /* Cette fonction est décrite dans graph.c dans notre version précédente */
    return NULL;
}

Graph *buildGraphFromXml(const void *xmlRoot)
{
    /* Implémentez ou adaptez selon vos besoins pour le XML */
    return NULL;
}

void print_json(const JsonValue *value, int indent)
{
    /* Implémentation similaire à celle proposée précédemment */
}

void free_json(JsonValue *value)
{
    /* Implémentation similaire à celle proposée précédemment */
}
