#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

///////////////////////
// Définition des tokens
///////////////////////
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
    char *value; // pour STRING, NUMBER, BOOLEAN ou NULL (si nécessaire)
} Token;

#define INITIAL_TOKEN_CAPACITY 100

// Fonction pour créer un token
Token create_token(TokenType type, const char *value)
{
    Token t;
    t.type = type;
    if (value != NULL)
        t.value = strdup(value);
    else
        t.value = NULL;
    return t;
}

// Libération de la mémoire des tokens
void free_tokens(Token *tokens, int count)
{
    for (int i = 0; i < count; i++)
        free(tokens[i].value);
    free(tokens);
}

// Fonction utilitaire pour sauter les espaces blancs
const char *skip_whitespace(const char *s)
{
    while (*s && isspace((unsigned char)*s))
        s++;
    return s;
}

// Fonction qui convertit un sous-texte en token numérique
Token parse_number(const char **s)
{
    const char *start = *s;
    while (**s && (isdigit((unsigned char)**s) || **s == '.' || **s == '-' ||
                   **s == '+' || **s == 'e' || **s == 'E'))
    {
        (*s)++;
    }
    int len = *s - start;
    char *numStr = malloc(len + 1);
    strncpy(numStr, start, len);
    numStr[len] = '\0';
    Token t = create_token(TOKEN_NUMBER, numStr);
    free(numStr);
    return t;
}

// Fonction qui convertit un texte en token chaîne
Token parse_string(const char **s)
{
    (*s)++; // saut du guillemet d'ouverture
    const char *start = *s;
    while (**s && **s != '"')
    {
        if (**s == '\\')
            (*s)++; // saut du caractère d'échappement
        (*s)++;
    }
    int len = *s - start;
    char *strVal = malloc(len + 1);
    strncpy(strVal, start, len);
    strVal[len] = '\0';
    Token t = create_token(TOKEN_STRING, strVal);
    free(strVal);
    if (**s == '"')
        (*s)++; // saut du guillemet de fermeture
    return t;
}

// Fonction principale de tokenisation
Token *tokenize(const char *input, int *token_count)
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

// Fonction pour lire le contenu d'un fichier dans une chaîne
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

// Fonction pour écrire les tokens dans un fichier de sortie (optionnel)
void write_tokens_to_file(const char *filename, Token *tokens, int token_count)
{
    FILE *fp = fopen(filename, "w");
    if (!fp)
    {
        perror("Erreur ouverture fichier de sortie");
        return;
    }
    for (int i = 0; i < token_count; i++)
    {
        switch (tokens[i].type)
        {
        case TOKEN_BEGIN_OBJECT:
            fputs("{", fp);
            break;
        case TOKEN_END_OBJECT:
            fputs("}", fp);
            break;
        case TOKEN_BEGIN_ARRAY:
            fputs("[", fp);
            break;
        case TOKEN_END_ARRAY:
            fputs("]", fp);
            break;
        case TOKEN_COLON:
            fputs(":", fp);
            break;
        case TOKEN_COMMA:
            fputs(",", fp);
            break;
        case TOKEN_STRING:
            fprintf(fp, "\"%s\"", tokens[i].value);
            break;
        case TOKEN_NUMBER:
        case TOKEN_BOOLEAN:
            fputs(tokens[i].value, fp);
            break;
        case TOKEN_NULL:
            fputs("null", fp);
            break;
        case TOKEN_EOF:
            break;
        case TOKEN_UNKNOWN:
            break;
        }
    }
    fclose(fp);
}

///////////////////////
// Structures et fonctions pour la liste chaînée des nœuds
///////////////////////
typedef struct NodeRecord
{
    int id;
    char *nom;
    char *type_noeud;
    float latitude;
    float longitude;
    int capacite;
    struct NodeRecord *next;
} NodeRecord;

NodeRecord *create_node_record()
{
    NodeRecord *node = (NodeRecord *)malloc(sizeof(NodeRecord));
    node->id = 0;
    node->nom = NULL;
    node->type_noeud = NULL;
    node->latitude = 0.0;
    node->longitude = 0.0;
    node->capacite = 0;
    node->next = NULL;
    return node;
}

// Fonction qui parcourt les tokens pour extraire la liste des nœuds sous la clé "nodes"
NodeRecord *parse_nodes_list(Token *tokens, int token_count)
{
    int i = 0;
    // Rechercher le token "nodes"
    while (i < token_count)
    {
        if (tokens[i].type == TOKEN_STRING && strcmp(tokens[i].value, "nodes") == 0)
        {
            i++;
            if (i < token_count && tokens[i].type == TOKEN_COLON)
                i++;
            if (i < token_count && tokens[i].type == TOKEN_BEGIN_ARRAY)
            {
                i++; // Passe '['
                break;
            }
        }
        else
        {
            i++;
        }
    }

    NodeRecord *head = NULL;
    NodeRecord *tail = NULL;

    // Parcourir le tableau d'objets jusqu'au TOKEN_END_ARRAY
    while (i < token_count && tokens[i].type != TOKEN_END_ARRAY)
    {
        if (tokens[i].type == TOKEN_BEGIN_OBJECT)
        {
            i++; // Passe '{'
            NodeRecord *newNode = create_node_record();
            while (i < token_count && tokens[i].type != TOKEN_END_OBJECT)
            {
                if (tokens[i].type == TOKEN_STRING)
                {
                    char *field = tokens[i].value;
                    i++; // Passe la clé
                    if (i < token_count && tokens[i].type == TOKEN_COLON)
                        i++; // Passe ':'
                    if (strcmp(field, "id") == 0)
                    {
                        if (i < token_count && tokens[i].type == TOKEN_NUMBER)
                            newNode->id = atoi(tokens[i].value);
                        i++;
                    }
                    else if (strcmp(field, "nom") == 0)
                    {
                        if (i < token_count && tokens[i].type == TOKEN_STRING)
                            newNode->nom = strdup(tokens[i].value);
                        i++;
                    }
                    else if (strcmp(field, "type_noeud") == 0)
                    {
                        if (i < token_count && tokens[i].type == TOKEN_STRING)
                            newNode->type_noeud = strdup(tokens[i].value);
                        i++;
                    }
                    else if (strcmp(field, "latitude") == 0)
                    {
                        if (i < token_count && tokens[i].type == TOKEN_NUMBER)
                            newNode->latitude = atof(tokens[i].value);
                        i++;
                    }
                    else if (strcmp(field, "longitude") == 0)
                    {
                        if (i < token_count && tokens[i].type == TOKEN_NUMBER)
                            newNode->longitude = atof(tokens[i].value);
                        i++;
                    }
                    else if (strcmp(field, "capacite") == 0)
                    {
                        if (i < token_count && tokens[i].type == TOKEN_NUMBER)
                            newNode->capacite = atoi(tokens[i].value);
                        i++;
                    }
                    else
                    {
                        i++; // Ignorer les champs inconnus
                    }
                }
                else
                {
                    i++; // Passer virgules ou autres tokens
                }
            }
            if (i < token_count && tokens[i].type == TOKEN_END_OBJECT)
                i++; // Passe '}'
            if (head == NULL)
            {
                head = newNode;
                tail = newNode;
            }
            else
            {
                tail->next = newNode;
                tail = newNode;
            }
            if (i < token_count && tokens[i].type == TOKEN_COMMA)
                i++; // Passe ','
        }
        else
        {
            i++;
        }
    }
    if (i < token_count && tokens[i].type == TOKEN_END_ARRAY)
        i++; // Passe ']'
    return head;
}

void print_node_list(NodeRecord *head)
{
    while (head != NULL)
    {
        printf("ID: %d\n", head->id);
        printf("Nom: %s\n", head->nom ? head->nom : "NULL");
        printf("Type: %s\n", head->type_noeud ? head->type_noeud : "NULL");
        printf("Latitude: %.6f\n", head->latitude);
        printf("Longitude: %.6f\n", head->longitude);
        printf("Capacite: %d\n", head->capacite);
        printf("--------------------------\n");
        head = head->next;
    }
}

void free_node_list(NodeRecord *head)
{
    NodeRecord *temp;
    while (head != NULL)
    {
        temp = head;
        head = head->next;
        free(temp->nom);
        free(temp->type_noeud);
        free(temp);
    }
}

///////////////////////
// Fonction main
///////////////////////
int main()
{
    char input_filename[256];
    char output_filename[256];

    // Demande du nom du fichier d'entrée
    printf("Entrez le nom du fichier d'entree (ex: test.json) : ");
    scanf("%255s", input_filename);

    // Lecture du fichier JSON
    char *json_input = read_file(input_filename);
    if (!json_input)
    {
        fprintf(stderr, "Erreur lors de la lecture du fichier %s\n", input_filename);
        system("pause");
        return 1;
    }

    // Tokenisation du contenu JSON
    int token_count = 0;
    Token *tokens = tokenize(json_input, &token_count);
    free(json_input); // Plus besoin du contenu original

    // Extraction des nœuds de la clé "nodes" pour remplir la liste chaînée
    NodeRecord *nodeList = parse_nodes_list(tokens, token_count);

    // Affichage de la liste des nœuds
    printf("\n--- Liste des noeuds ---\n");
    print_node_list(nodeList);

    // Demande du nom du fichier de sortie et écriture des tokens (optionnel)
    printf("Entrez le nom du fichier de sortie (ex: output.json) : ");
    scanf("%255s", output_filename);
    write_tokens_to_file(output_filename, tokens, token_count);
    printf("Le contenu a ete lu depuis %s et reecrit dans %s\n", input_filename, output_filename);

    printf("Appuyez sur une touche pour continuer...\n");
    system("pause");

    // Libération de la mémoire
    free_tokens(tokens, token_count);
    free_node_list(nodeList);

    return 0;
}
