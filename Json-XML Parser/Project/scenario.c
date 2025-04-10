#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scenario.h"
#include "parser.h" // Réutilisation du parseur JSON

// Convertit une chaîne "HH:MM" en minutes depuis minuit
static int timeStringToMinutes(const char *timeStr)
{
    int hours, minutes;
    if (sscanf(timeStr, "%d:%d", &hours, &minutes) == 2)
        return hours * 60 + minutes;
    return 0;
}

// Concatène les éléments d'un tableau JSON de chaînes en une seule chaîne séparée par ", "
static void concatConditions(const JsonValue *conditionsArr, char *dest, size_t destSize)
{
    dest[0] = '\0';
    if (conditionsArr->type != JSON_ARRAY)
        return;
    for (size_t i = 0; i < conditionsArr->as.array.count; i++)
    {
        JsonValue *item = conditionsArr->as.array.items[i];
        if (item && item->type == JSON_STRING)
        {
            strncat(dest, item->as.stringValue, destSize - strlen(dest) - 1);
            if (i < conditionsArr->as.array.count - 1)
            {
                strncat(dest, ", ", destSize - strlen(dest) - 1);
            }
        }
    }
}

// Charge le premier scénario depuis le fichier JSON
Scenario *loadScenarioFromJSON(const char *filename)
{
    char *fileContent = read_file(filename);
    if (!fileContent)
    {
        printf("Erreur de lecture du fichier scenario.\n");
        return NULL;
    }

    JsonValue *jsonRoot = parse_json_file(fileContent);
    free(fileContent);
    if (!jsonRoot || jsonRoot->type != JSON_OBJECT)
    {
        printf("Erreur de parsing du fichier JSON scenario.\n");
        return NULL;
    }

    // Rechercher la clé "scenarios" dans le JSON racine
    JsonValue *scenariosArr = NULL;
    for (size_t i = 0; i < jsonRoot->as.object.count; i++)
    {
        if (strcmp(jsonRoot->as.object.entries[i].key, "scenarios") == 0)
        {
            scenariosArr = jsonRoot->as.object.entries[i].value;
            break;
        }
    }

    if (!scenariosArr || scenariosArr->type != JSON_ARRAY || scenariosArr->as.array.count == 0)
    {
        printf("Aucun scénario trouvé dans le fichier JSON.\n");
        free_json(jsonRoot);
        return NULL;
    }

    // Pour cette version, on prend le premier scénario du tableau
    JsonValue *scObj = scenariosArr->as.array.items[0];
    if (!scObj || scObj->type != JSON_OBJECT)
    {
        printf("Format du scénario invalide.\n");
        free_json(jsonRoot);
        return NULL;
    }

    Scenario *sc = malloc(sizeof(Scenario));
    if (!sc)
    {
        printf("Erreur d'allocation pour le scénario.\n");
        free_json(jsonRoot);
        return NULL;
    }
    memset(sc, 0, sizeof(Scenario));
    sc->num_days = 1; // Valeur par défaut

    // Parcours des champs du scénario
    for (size_t i = 0; i < scObj->as.object.count; i++)
    {
        char *key = scObj->as.object.entries[i].key;
        JsonValue *value = scObj->as.object.entries[i].value;

        if (strcmp(key, "name") == 0 && value->type == JSON_STRING)
        {
            strncpy(sc->name, value->as.stringValue, sizeof(sc->name) - 1);
        }
        else if (strcmp(key, "date") == 0 && value->type == JSON_STRING)
        {
            strncpy(sc->date, value->as.stringValue, sizeof(sc->date) - 1);
        }
        else if (strcmp(key, "conditions") == 0 && value->type == JSON_ARRAY)
        {
            concatConditions(value, sc->conditions, sizeof(sc->conditions));
        }
        else if (strcmp(key, "demandes") == 0 && value->type == JSON_ARRAY)
        {
            sc->nbDemandes = (int)value->as.array.count;
            sc->demandes = malloc(sc->nbDemandes * sizeof(Delivery));
            if (!sc->demandes)
            {
                printf("Erreur d'allocation pour les demandes.\n");
                freeScenario(sc);
                free_json(jsonRoot);
                return NULL;
            }
            for (size_t j = 0; j < value->as.array.count; j++)
            {
                JsonValue *delivObj = value->as.array.items[j];
                Delivery d = {0};
                d.id = (int)j + 1; // Id généré automatiquement
                d.day = 0;         // Par défaut
                d.livre = 0;       // Non livré
                if (delivObj && delivObj->type == JSON_OBJECT)
                {
                    for (size_t k = 0; k < delivObj->as.object.count; k++)
                    {
                        char *dKey = delivObj->as.object.entries[k].key;
                        JsonValue *dVal = delivObj->as.object.entries[k].value;
                        if (strcmp(dKey, "origine") == 0 && dVal->type == JSON_NUMBER)
                        {
                            d.origin = (int)dVal->as.numberValue - 1; // Conversion en indice (commence à 0)
                        }
                        else if (strcmp(dKey, "destination") == 0 && dVal->type == JSON_NUMBER)
                        {
                            d.destination = (int)dVal->as.numberValue - 1;
                        }
                        else if (strcmp(dKey, "volume") == 0 && dVal->type == JSON_NUMBER)
                        {
                            d.volume = (int)dVal->as.numberValue;
                        }
                        else if (strcmp(dKey, "deadline") == 0 && dVal->type == JSON_STRING)
                        {
                            d.deadline = timeStringToMinutes(dVal->as.stringValue);
                        }
                        // La priorité n'est pas stockée dans notre structure Delivery.
                    }
                }
                sc->demandes[j] = d;
            }
        }
        else if (strcmp(key, "vehicules") == 0 && value->type == JSON_ARRAY)
        {
            sc->nbVehicules = (int)value->as.array.count;
            sc->vehicules = malloc(sc->nbVehicules * sizeof(Vehicle));
            if (!sc->vehicules)
            {
                printf("Erreur d'allocation pour les véhicules.\n");
                freeScenario(sc);
                free_json(jsonRoot);
                return NULL;
            }
            for (size_t j = 0; j < value->as.array.count; j++)
            {
                JsonValue *vehObj = value->as.array.items[j];
                Vehicle v = {0};
                strncpy(v.type, "undefined", sizeof(v.type));
                v.capacity = 0;
                v.dispo_debut = 0;
                v.dispo_fin = 1440; // Par défaut, fin de journée
                v.cost_per_km = 0.0f;
                v.position = 0;
                if (vehObj && vehObj->type == JSON_OBJECT)
                {
                    for (size_t k = 0; k < vehObj->as.object.count; k++)
                    {
                        char *vKey = vehObj->as.object.entries[k].key;
                        JsonValue *vVal = vehObj->as.object.entries[k].value;
                        if (strcmp(vKey, "type") == 0 && vVal->type == JSON_STRING)
                        {
                            strncpy(v.type, vVal->as.stringValue, sizeof(v.type) - 1);
                        }
                        else if (strcmp(vKey, "capacite") == 0 && vVal->type == JSON_NUMBER)
                        {
                            v.capacity = (int)vVal->as.numberValue;
                        }
                        else if (strcmp(vKey, "disponibilite") == 0 && vVal->type == JSON_STRING)
                        {
                            // Format attendu "HH:MM-HH:MM"
                            char *dash = strchr(vVal->as.stringValue, '-');
                            if (dash)
                            {
                                char debut[6], fin[6];
                                int len = dash - vVal->as.stringValue;
                                strncpy(debut, vVal->as.stringValue, len);
                                debut[len] = '\0';
                                strncpy(fin, dash + 1, 5);
                                fin[5] = '\0';
                                v.dispo_debut = timeStringToMinutes(debut);
                                v.dispo_fin = timeStringToMinutes(fin);
                            }
                        }
                        else if (strcmp(vKey, "cout_km") == 0 && vVal->type == JSON_NUMBER)
                        {
                            v.cost_per_km = (float)vVal->as.numberValue;
                        }
                    }
                }
                sc->vehicules[j] = v;
            }
        }
    }
    free_json(jsonRoot);
    return sc;
}

// Libère la mémoire allouée pour un scénario
void freeScenario(Scenario *sc)
{
    if (sc)
    {
        if (sc->demandes)
            free(sc->demandes);
        if (sc->vehicules)
            free(sc->vehicules);
        free(sc);
    }
}

// Applique les algorithmes du scénario sur le graphe fourni
void applyScenario(Scenario *sc, Graph *graph)
{
    if (!sc || !graph)
    {
        printf("Scenario ou graphe nul.\n");
        return;
    }
    printf("Exécution du scénario \"%s\" du %s\n", sc->name, sc->date);
    printf("Conditions: %s\n", sc->conditions);
    printf("Nombre de demandes : %d, Nombre de véhicules : %d, Jours : %d\n",
           sc->nbDemandes, sc->nbVehicules, sc->num_days);

    // Exemple : appel de la planification gloutonne avec le premier véhicule
    if (sc->nbDemandes > 0 && sc->nbVehicules > 0)
    {
        Vehicle *v = &sc->vehicules[0];
        printf("\n-- Planification gloutonne avec le vehicule: %s --\n", v->type);
        planification_gloutonne(graph, sc->demandes, sc->nbDemandes, v);
    }
    // Vous pouvez ajouter d'autres appels à d'autres algorithmes ici
}
