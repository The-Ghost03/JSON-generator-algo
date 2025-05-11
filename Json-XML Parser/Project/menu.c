/* menu.c */
#include "menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *MENU_TEXT =
    "\n=== Menu principal ===\n"
    " 1. Charger graphe JSON\n"
    " 2. Charger graphe XML\n"
    " 3. Floyd–Warshall\n"
    " 4. Bellman–Ford\n"
    " 5. Détection de cycle\n"
    " 6. Composantes connexes\n"
    " 7. Points d'articulation\n"
    " 8. Quitter\n"
    "Votre choix> ";

void initMenu(void)
{
    /* Pas de ressources à initialiser pour l'instant */
}

void freeMenu(void)
{
    /* Pas de ressources à libérer pour l'instant */
}

MenuOption displayMenu(void)
{
    char line[32];
    int choice = 0;
    while (1)
    {
        fputs(MENU_TEXT, stdout);
        if (!fgets(line, sizeof(line), stdin))
        {
            /* Réinitialiser l'état d'erreur et réessayer */
            clearerr(stdin);
            continue;
        }
        choice = atoi(line);
        if (choice >= LOAD_JSON && choice <= EXIT)
        {
            return (MenuOption)choice;
        }
        printf("Choix invalide. Veuillez entrer un nombre entre %d et %d.\n",
               LOAD_JSON, EXIT);
    }
}

int askJsonPath(char **path_out)
{
    char buf[1024];
    printf("Entrez le chemin du fichier JSON: ");
    if (!fgets(buf, sizeof(buf), stdin))
        return -1;
    /* Supprimer le saut de ligne */
    buf[strcspn(buf, "\r\n")] = '\0';
    size_t len = strlen(buf);
    *path_out = malloc(len + 1);
    if (!*path_out)
        return -1;
    strcpy(*path_out, buf);
    return 0;
}

int askXmlPath(char **path_out)
{
    char buf[1024];
    printf("Entrez le chemin du fichier XML: ");
    if (!fgets(buf, sizeof(buf), stdin))
        return -1;
    buf[strcspn(buf, "\r\n")] = '\0';
    size_t len = strlen(buf);
    *path_out = malloc(len + 1);
    if (!*path_out)
        return -1;
    strcpy(*path_out, buf);
    return 0;
}

int askSourceNode(int V)
{
    char line[32];
    int src;
    while (1)
    {
        printf("Entrez le numéro du nœud source (0–%d): ", V - 1);
        if (!fgets(line, sizeof(line), stdin))
        {
            clearerr(stdin);
            continue;
        }
        src = atoi(line);
        if (src >= 0 && src < V)
        {
            return src;
        }
        printf("Noeud invalide. Veuillez entrer un entier entre 0 et %d.\n", V - 1);
    }
}
