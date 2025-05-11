#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scenario.h"
#include "cJSON.h" // Librairie JSON, à inclure dans le projet

// Fonction pour charger un scénario à partir d'un fichier JSON
Scenario* loadScenarioFromJSON(const char* filePath) {
    FILE* file = fopen(filePath, "r");
    if (!file) {
        fprintf(stderr, "Erreur : Impossible d'ouvrir le fichier %s\n", filePath);
        return NULL;
    }

    // Lire le contenu du fichier
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    char* fileContent = (char*)malloc(fileSize + 1);
    fread(fileContent, 1, fileSize, file);
    fileContent[fileSize] = '\0';
    fclose(file);

    // Parser le JSON
    cJSON* root = cJSON_Parse(fileContent);
    free(fileContent);
    if (!root) {
        fprintf(stderr, "Erreur : JSON invalide\n");
        return NULL;
    }

    // Allouer la structure du scénario
    Scenario* scenario = (Scenario*)malloc(sizeof(Scenario));
    strcpy(scenario->date, cJSON_GetObjectItem(root, "date")->valuestring);

    // Charger les conditions
    cJSON* conditions = cJSON_GetObjectItem(root, "conditions");
    strcpy(scenario->conditions.traffic, cJSON_GetObjectItem(conditions, "trafic")->valuestring);
    strcpy(scenario->conditions.weather, cJSON_GetObjectItem(conditions, "météo")->valuestring);
    strcpy(scenario->conditions.incident, cJSON_GetObjectItem(conditions, "incident")->valuestring);

    // Charger les demandes (colis)
    cJSON* requests = cJSON_GetObjectItem(root, "colis");
    scenario->num_requests = cJSON_GetArraySize(requests);
    scenario->requests = (ScenarioRequest*)malloc(sizeof(ScenarioRequest) * scenario->num_requests);

    for (int i = 0; i < scenario->num_requests; i++) {
        cJSON* request = cJSON_GetArrayItem(requests, i);
        scenario->requests[i].id = cJSON_GetObjectItem(request, "id")->valueint;
        scenario->requests[i].origin = cJSON_GetObjectItem(request, "origine")->valueint;
        scenario->requests[i].destination = cJSON_GetObjectItem(request, "destination")->valueint;
        scenario->requests[i].volume = cJSON_GetObjectItem(request, "volume")->valueint;
        strcpy(scenario->requests[i].priority, cJSON_GetObjectItem(request, "priorite")->valuestring);
        strcpy(scenario->requests[i].deadline, cJSON_GetObjectItem(request, "deadline")->valuestring);
    }

    // Charger les véhicules
    cJSON* vehicles = cJSON_GetObjectItem(root, "vehicules");
    scenario->num_vehicles = cJSON_GetArraySize(vehicles);
    scenario->vehicles = (Vehicle*)malloc(sizeof(Vehicle) * scenario->num_vehicles);

    for (int i = 0; i < scenario->num_vehicles; i++) {
        cJSON* vehicle = cJSON_GetArrayItem(vehicles, i);
        scenario->vehicles[i].id = cJSON_GetObjectItem(vehicle, "id")->valueint;
        strcpy(scenario->vehicles[i].type, cJSON_GetObjectItem(vehicle, "type")->valuestring);
        scenario->vehicles[i].capacity = cJSON_GetObjectItem(vehicle, "capacite")->valueint;
        strcpy(scenario->vehicles[i].availability_start, cJSON_GetObjectItem(vehicle, "dispo_debut")->valuestring);
        strcpy(scenario->vehicles[i].availability_end, cJSON_GetObjectItem(vehicle, "dispo_fin")->valuestring);
        scenario->vehicles[i].cost_per_km = cJSON_GetObjectItem(vehicle, "cout_par_km")->valuedouble;
        scenario->vehicles[i].position = cJSON_GetObjectItem(vehicle, "position")->valueint;
    }

    cJSON_Delete(root);
    return scenario;
}

// Fonction pour libérer la mémoire d'un scénario
void freeScenario(Scenario* scenario) {
    if (scenario) {
        free(scenario->requests);
        free(scenario->vehicles);
        free(scenario);
    }
}

// Fonction pour afficher un scénario
void printScenario(const Scenario* scenario) {
    if (!scenario) return;

    printf("Date : %s\n", scenario->date);
    printf("Conditions : Trafic: %s, Météo: %s, Incident: %s\n",
           scenario->conditions.traffic,
           scenario->conditions.weather,
           scenario->conditions.incident);

    printf("Demandes (Colis) :\n");
    for (int i = 0; i < scenario->num_requests; i++) {
        printf("  ID: %d, Origine: %d, Destination: %d, Volume: %d, Priorité: %s, Deadline: %s\n",
               scenario->requests[i].id,
               scenario->requests[i].origin,
               scenario->requests[i].destination,
               scenario->requests[i].volume,
               scenario->requests[i].priority,
               scenario->requests[i].deadline);
    }

    printf("Véhicules :\n");
    for (int i = 0; i < scenario->num_vehicles; i++) {
        printf("  ID: %d, Type: %s, Capacité: %d, Disponibilité: %s - %s, Coût/km: %.2f, Position: %d\n",
               scenario->vehicles[i].id,
               scenario->vehicles[i].type,
               scenario->vehicles[i].capacity,
               scenario->vehicles[i].availability_start,
               scenario->vehicles[i].availability_end,
               scenario->vehicles[i].cost_per_km,
               scenario->vehicles[i].position);
    }
}