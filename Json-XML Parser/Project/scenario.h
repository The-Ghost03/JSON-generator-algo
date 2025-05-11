/* scenario.h */
#ifndef SCENARIO_H
#define SCENARIO_H

/**
 * Configuration d’un scénario de réseau :
 * - filePath : chemin vers le fichier JSON ou XML à parser
 */
typedef struct
{
    char *filePath;
} ScenarioConfig;

/**
 * Initialise le module scénario (aucune ressource pour l’instant).
 */
void initScenario(void);

/**
 * Propose à l’utilisateur de choisir un scénario prédéfini
 * (petit, moyen, grand) ou un fichier personnalisé.
 * Alloue cfg->filePath avec strdup() ou malloc().
 *
 * @param cfg  pointeur vers ScenarioConfig à remplir
 * @return 0 si OK, -1 en cas d’erreur ou interruption
 */
int chooseScenario(ScenarioConfig *cfg);

/**
 * Libère les ressources allouées dans ScenarioConfig.
 */
void freeScenarioConfig(ScenarioConfig *cfg);

#endif /* SCENARIO_H */
