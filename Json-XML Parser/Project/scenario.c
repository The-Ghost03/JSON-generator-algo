/* scenario.c */
#include "scenario.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *SCENARIO_MENU =
    "\n=== Choix du scénario ===\n"
    " 1. Petit réseau (data/small.json)\n"
    " 2. Réseau moyen (data/medium.json)\n"
    " 3. Grand réseau (data/large.json)\n"
    " 4. Fichier personnalisé\n"
    "Votre choix> ";

/* Lit une ligne au clavier et la retourne (sans '\n') via malloc() */
static int askFilePath(char **path_out)
{
    char buf[1024];
    if (!fgets(buf, sizeof(buf), stdin))
        return -1;
    buf[strcspn(buf, "\r\n")] = '\0';
    size_t len = strlen(buf);
    if (len == 0)
    {
        fprintf(stderr, "Chemin vide.\n");
        return -1;
    }
    *path_out = malloc(len + 1);
    if (!*path_out)
        return -1;
    strcpy(*path_out, buf);
    return 0;
}

void initScenario(void)
{
    /* Aucun état à initialiser pour le moment */
}

int chooseScenario(ScenarioConfig *cfg)
{
    char line[8];
    int choice = 0;

    while (1)
    {
        fputs(SCENARIO_MENU, stdout);
        if (!fgets(line, sizeof(line), stdin))
        {
            clearerr(stdin);
            continue;
        }
        choice = atoi(line);
        switch (choice)
        {
        case 1:
            cfg->filePath = strdup("data/small.json");
            return cfg->filePath ? 0 : -1;
        case 2:
            cfg->filePath = strdup("data/medium.json");
            return cfg->filePath ? 0 : -1;
        case 3:
            cfg->filePath = strdup("data/large.json");
            return cfg->filePath ? 0 : -1;
        case 4:
            printf("Entrez le chemin du fichier JSON ou XML: ");
            if (askFilePath(&cfg->filePath) == 0)
                return 0;
            /* sinon retenter */
            break;
        default:
            printf("Choix invalide (%d). Veuillez réessayer.\n", choice);
        }
    }
}

void freeScenarioConfig(ScenarioConfig *cfg)
{
    if (cfg->filePath)
    {
        free(cfg->filePath);
        cfg->filePath = NULL;
    }
}
