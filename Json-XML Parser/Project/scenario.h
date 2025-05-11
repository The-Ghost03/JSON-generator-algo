#ifndef SCENARIO_H
#define SCENARIO_H

#include <stdbool.h>

// Définition de la structure pour une demande (colis) dans un scénario
typedef struct ScenarioRequest {
    int id;
    int origin;          // Noeud source
    int destination;     // Noeud de destination
    int volume;          // Volume de la demande
    char priority[10];   // Priorité (standard, express, fragile)
    char deadline[6];    // Heure limite (HH:MM)
} ScenarioRequest;

// Définition de la structure pour un véhicule
typedef struct Vehicle {
    int id;
    char type[20];       // Type de véhicule (camion, utilitaire, etc.)
    int capacity;        // Capacité en volume
    char availability_start[6]; // Heure de début de disponibilité
    char availability_end[6];   // Heure de fin de disponibilité
    float cost_per_km;   // Coût par kilomètre
    int position;        // Position initiale du véhicule (noeud)
} Vehicle;

// Définition de la structure pour les conditions générales
typedef struct Conditions {
    char traffic[15];    // État du trafic (normal, congestionné, etc.)
    char weather[15];    // État de la météo (dégagée, pluie, etc.)
    char incident[50];   // Description des incidents (aucun, travaux, etc.)
} Conditions;

// Définition de la structure pour un scénario
typedef struct Scenario {
    char date[11];       // Date du scénario (YYYY-MM-DD)
    Conditions conditions; // Conditions générales
    int num_requests;    // Nombre de demandes
    ScenarioRequest* requests; // Liste des demandes
    int num_vehicles;    // Nombre de véhicules
    Vehicle* vehicles;   // Liste des véhicules
} Scenario;

// Fonctions pour gérer les scénarios
Scenario* loadScenarioFromJSON(const char* filePath);
void freeScenario(Scenario* scenario);
void printScenario(const Scenario* scenario);

#endif // SCENARIO_H