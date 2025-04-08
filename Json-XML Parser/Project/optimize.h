#ifndef OPTIMIZE_H
#define OPTIMIZE_H

#include "graph.h"

/* ===============================
   FLOYD-WARSHALL
   =============================== */
double **createDistanceMatrix(const Graph *graph);
void floydWarshall(int n, double **matrix);
void printDistanceMatrix(int n, double **matrix);
void freeDistanceMatrix(int n, double **matrix);

/* ===============================
   BELLMAN-FORD ADAPTE
   =============================== */
void bellman_ford_time_aware(Graph *graph, int src);

/* ===============================
   PROBLEME DU VOYAGEUR DE COMMERCE (TSP)
   =============================== */
void tsp_genetic_solution(Graph *graph);

/* ===============================
   PLANIFICATION MULTI-JOURS
   =============================== */

/* Structures pour la planification multi-jours */
typedef struct
{
    int id;
    int origin;
    int destination;
    int volume;
    int deadline; // Deadline en minutes (ex. 18:00 -> 1080)
    int day;      // Jour de la semaine (0 à 6)
    int livre;    // 0 = non livré, 1 = livré
} Delivery;

typedef struct
{
    char type[50];
    int capacity;
    int dispo_debut; // Heure de début (en minutes)
    int dispo_fin;   // Heure de fin (en minutes)
    float cost_per_km;
    int position; // Position initiale (index du nœud)
} Vehicle;

void multi_day_scheduling(Delivery *deliveries, int nbDeliveries,
                          Vehicle *vehicules, int nbVehicules,
                          int num_days, Graph *graph);

/* ===============================
   PLANIFICATION JOURNALIÈRE (Gloutonne)
   =============================== */
void planification_gloutonne(Graph *graph, Delivery *deliveries, int nbDeliveries, Vehicle *vehicule);

/* ===============================
   CALCUL DU TEMPS DE TRAJET AVEC CONGESTION
   =============================== */
/*
   Calcule le temps de trajet entre le nœud d'origine et le nœud destination en tenant compte
   de la congestion variable selon l'heure de départ.
   Les plages horaires utilisées ici sont :
     - Morning : de 00:00 à 12:00 (0 à 720 minutes)
     - Afternoon : de 12:00 à 18:00 (720 à 1080 minutes)
     - Night : de 18:00 à 24:00 (1080 à 1440 minutes)
   Les facteurs de congestion sont lus depuis le nœud d'origine.
*/
float calculerTempsTrajet(const Graph *graph, int origin, int destination, int currentTime);

#endif /* OPTIMIZE_H */
