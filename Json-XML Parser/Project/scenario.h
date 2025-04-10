#ifndef SCENARIO_H
#define SCENARIO_H

#include "optimize.h" // Pour les types Delivery et Vehicle

// Structure représentant un scénario de test
typedef struct
{
    char name[100];
    char date[20];
    char conditions[512]; // On concatène les conditions en une seule chaîne
    Delivery *demandes;
    int nbDemandes;
    Vehicle *vehicules; // Champ pour les véhicules
    int nbVehicules;
    int num_days; // Nombre de jours (par défaut 1)
} Scenario;

/* Charge le premier scénario depuis un fichier JSON */
Scenario *loadScenarioFromJSON(const char *filename);

/* Libère la mémoire allouée pour un scénario */
void freeScenario(Scenario *sc);

/* Applique les algorithmes associés au scénario sur le graphe fourni */
void applyScenario(Scenario *sc, Graph *graph);

#endif /* SCENARIO_H */
