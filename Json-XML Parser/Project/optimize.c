/* optimize.c
   Module d'optimisation regroupant :
     - Floyd-Warshall (pour le calcul des plus courts chemins)
     - Bellman-Ford adapté aux contraintes temporelles (simplifié)
     - Algorithme génétique pour le problème du voyageur de commerce (TSP)
     - Planification journalière gloutonne (tournées dans la journée)
     - Planification multi-jours
     - Calcul du temps de trajet prenant en compte une congestion variable
*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "optimize.h"
#include "graph.h"

/* ===============================
   FLOYD-WARSHALL
   =============================== */
double **createDistanceMatrix(const Graph *graph)
{
    int n = graph->V;
    double **matrix = malloc(n * sizeof(double *));
    if (!matrix)
    {
        fprintf(stderr, "Erreur d'allocation pour la matrice de distances.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < n; i++)
    {
        matrix[i] = malloc(n * sizeof(double));
        if (!matrix[i])
        {
            fprintf(stderr, "Erreur d'allocation pour la ligne %d de la matrice.\n", i);
            exit(EXIT_FAILURE);
        }
        for (int j = 0; j < n; j++)
        {
            matrix[i][j] = (i == j) ? 0.0 : 1e9; // INF = 1e9
        }
        // Parcourir la liste d'adjacence pour récupérer le coût (baseTime)
        AdjListNode *edge = graph->array[i].head;
        while (edge)
        {
            int j = edge->dest;
            double cost = edge->attr.baseTime; // Utilisation de baseTime
            if (cost < matrix[i][j])
                matrix[i][j] = cost;
            edge = edge->next;
        }
    }
    return matrix;
}

void floydWarshall(int n, double **matrix)
{
    for (int k = 0; k < n; k++)
    {
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                double newDist = matrix[i][k] + matrix[k][j];
                if (newDist < matrix[i][j])
                    matrix[i][j] = newDist;
            }
        }
    }
}

void printDistanceMatrix(int n, double **matrix)
{
    printf("\n--- Matrice des plus courts chemins (Floyd-Warshall) ---\n");
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if (matrix[i][j] >= 1e9)
                printf("%7s ", "INF");
            else
                printf("%7.2f ", matrix[i][j]);
        }
        printf("\n");
    }
}

void freeDistanceMatrix(int n, double **matrix)
{
    for (int i = 0; i < n; i++)
        free(matrix[i]);
    free(matrix);
}

/* ===============================
   BELLMAN–FORD ADAPTE (simplifié)
   =============================== */
void bellman_ford_time_aware(Graph *graph, int src)
{
    int V = graph->V;
    int INF_INT = INT_MAX / 2;
    int *dist = malloc(V * sizeof(int));
    if (!dist)
    {
        fprintf(stderr, "Erreur d'allocation pour Bellman–Ford.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < V; i++)
        dist[i] = INF_INT;
    dist[src] = 0;

    // Relaxation V-1 fois
    for (int i = 0; i < V - 1; i++)
    {
        for (int u = 0; u < V; u++)
        {
            AdjListNode *edge = graph->array[u].head;
            while (edge)
            {
                int v = edge->dest;
                int weight = (int)edge->attr.baseTime;
                if (dist[u] + weight < dist[v])
                    dist[v] = dist[u] + weight;
                edge = edge->next;
            }
        }
    }

    // Affichage des distances calculées
    printf("\n--- Résultats de Bellman–Ford adapté ---\n");
    for (int i = 0; i < V; i++)
    {
        if (dist[i] < INF_INT)
            printf("Noeud %d : %d\n", i + 1, dist[i]);
        else
            printf("Noeud %d : INF\n", i + 1);
    }
    free(dist);
}

/* ===============================
   PROBLEME DU VOYAGEUR DE COMMERCE (TSP) - Algorithme génétique
   =============================== */
#define POPULATION_SIZE 200
#define GENERATIONS 1000
#define MUTATION_RATE 0.05

typedef struct
{
    int *path;     // Ordre de visite des nœuds
    float fitness; // Distance totale du trajet
} Individual;

static void swap_int(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

static void generateRandomPath(Individual *ind, int nodeCount)
{
    ind->path = malloc(nodeCount * sizeof(int));
    if (!ind->path)
    {
        fprintf(stderr, "Erreur d'allocation pour le chemin de l'individu.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < nodeCount; i++)
        ind->path[i] = i;
    for (int i = 0; i < nodeCount; i++)
    {
        int j = rand() % nodeCount;
        swap_int(&ind->path[i], &ind->path[j]);
    }
}

static float evaluateFitness(Individual *ind, const Graph *graph, double **distMatrix)
{
    float totalDistance = 0.0f;
    int n = graph->V;
    for (int i = 0; i < n - 1; i++)
        totalDistance += distMatrix[ind->path[i]][ind->path[i + 1]];
    totalDistance += distMatrix[ind->path[n - 1]][ind->path[0]]; // Retour au départ
    return totalDistance;
}

static Individual tournamentSelection(Individual *population, int popSize)
{
    int i1 = rand() % popSize;
    int i2 = rand() % popSize;
    return (population[i1].fitness < population[i2].fitness) ? population[i1] : population[i2];
}

static void crossover(const Individual *parent1, const Individual *parent2, Individual *offspring, int nodeCount)
{
    int point = rand() % nodeCount;
    for (int i = 0; i < point; i++)
        offspring->path[i] = parent1->path[i];

    int index = point;
    for (int i = 0; i < nodeCount; i++)
    {
        int gene = parent2->path[i];
        int exists = 0;
        for (int j = 0; j < point; j++)
        {
            if (offspring->path[j] == gene)
            {
                exists = 1;
                break;
            }
        }
        if (!exists)
            offspring->path[index++] = gene;
    }
}

static void mutate(Individual *ind, int nodeCount)
{
    if ((rand() / (float)RAND_MAX) < MUTATION_RATE)
    {
        int i = rand() % nodeCount;
        int j = rand() % nodeCount;
        if (i > j)
        {
            int temp = i;
            i = j;
            j = temp;
        }
        while (i < j)
        {
            swap_int(&ind->path[i], &ind->path[j]);
            i++;
            j--;
        }
    }
}

void tsp_genetic_solution(Graph *graph)
{
    int n = graph->V;
    double **distMatrix = createDistanceMatrix(graph);
    floydWarshall(n, distMatrix);

    Individual *population = malloc(POPULATION_SIZE * sizeof(Individual));
    Individual *newPopulation = malloc(POPULATION_SIZE * sizeof(Individual));
    if (!population || !newPopulation)
    {
        fprintf(stderr, "Erreur d'allocation pour la population TSP.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < POPULATION_SIZE; i++)
    {
        generateRandomPath(&population[i], n);
        population[i].fitness = evaluateFitness(&population[i], graph, distMatrix);
    }

    for (int gen = 0; gen < GENERATIONS; gen++)
    {
        for (int i = 0; i < POPULATION_SIZE; i++)
        {
            Individual parent1 = tournamentSelection(population, POPULATION_SIZE);
            Individual parent2 = tournamentSelection(population, POPULATION_SIZE);
            newPopulation[i].path = malloc(n * sizeof(int));
            if (!newPopulation[i].path)
            {
                fprintf(stderr, "Erreur d'allocation pour un individu de la nouvelle population.\n");
                exit(EXIT_FAILURE);
            }
            crossover(&parent1, &parent2, &newPopulation[i], n);
            mutate(&newPopulation[i], n);
            newPopulation[i].fitness = evaluateFitness(&newPopulation[i], graph, distMatrix);
        }
        for (int i = 0; i < POPULATION_SIZE; i++)
        {
            free(population[i].path);
            population[i].path = newPopulation[i].path;
            population[i].fitness = newPopulation[i].fitness;
        }
    }

    Individual best = population[0];
    for (int i = 1; i < POPULATION_SIZE; i++)
    {
        if (population[i].fitness < best.fitness)
            best = population[i];
    }

    printf("\n--- Meilleur trajet trouvé (TSP) ---\n");
    printf("Distance totale : %.2f\n", best.fitness);
    printf("Chemin : ");
    for (int i = 0; i < n; i++)
    {
        printf("%d ", best.path[i] + 1);
    }
    printf("\n");

    for (int i = 0; i < POPULATION_SIZE; i++)
        free(population[i].path);
    free(population);
    free(newPopulation);
    freeDistanceMatrix(n, distMatrix);
}

/* ===============================
   CALCUL DU TEMPS DE TRAJET AVEC CONGESTION VARIABLE
   =============================== */
/*
   Cette fonction améliore le calcul du temps de trajet en prenant en compte un facteur de congestion
   variable en fonction de l'heure de départ. On utilise ici les paramètres de congestion présents dans
   le nœud d'origine :
       - Morning : 0 à 720 minutes,
       - Afternoon : 720 à 1080 minutes,
       - Night : 1080 à 1440 minutes.
   Le temps de trajet effectif est le baseTime multiplié par le facteur correspondant.
*/
float calculerTempsTrajet(const Graph *graph, int origin, int destination, int currentTime)
{
    // Recherche de l'arête reliant origin et destination
    AdjListNode *edge = graph->array[origin].head;
    while (edge && edge->dest != destination)
        edge = edge->next;
    if (!edge)
        return 1e9; // Pas de connexion

    float baseTime = edge->attr.baseTime;
    float factor = 1.0f;

    // Choix du facteur de congestion selon currentTime (en minutes depuis minuit)
    if (currentTime < 720)
        factor = graph->nodes[origin].congestion_morning;
    else if (currentTime < 1080)
        factor = graph->nodes[origin].congestion_afternoon;
    else
        factor = graph->nodes[origin].congestion_night;

    return baseTime * factor;
}

/* ===============================
   PLANIFICATION JOURNALIÈRE (Gloutonne)
   =============================== */
void planification_gloutonne(Graph *graph, Delivery *deliveries, int nbDeliveries, Vehicle *vehicule)
{
    int tempsActuel = vehicule->dispo_debut;
    int posActuelle = vehicule->position;

    while (vehicule->capacity > 0)
    {
        int meilleurIndex = -1;
        float meilleurTemps = 1e9; // INF

        for (int i = 0; i < nbDeliveries; i++)
        {
            if (deliveries[i].livre == 1 || deliveries[i].volume > vehicule->capacity)
                continue;
            // Utiliser la fonction qui prend en compte la congestion variable
            float tempsTrajet = calculerTempsTrajet(graph, posActuelle, deliveries[i].destination, tempsActuel);
            if (tempsTrajet < 1e9 &&
                (tempsActuel + (int)tempsTrajet <= vehicule->dispo_fin) &&
                (tempsActuel + (int)tempsTrajet <= deliveries[i].deadline))
            {
                if (tempsTrajet < meilleurTemps)
                {
                    meilleurTemps = tempsTrajet;
                    meilleurIndex = i;
                }
            }
        }
        if (meilleurIndex != -1)
        {
            printf("Livraison du colis %d vers %d\n", deliveries[meilleurIndex].id, deliveries[meilleurIndex].destination);
            vehicule->capacity -= deliveries[meilleurIndex].volume;
            tempsActuel += (int)meilleurTemps;
            posActuelle = deliveries[meilleurIndex].destination;
            deliveries[meilleurIndex].livre = 1;
        }
        else
        {
            break;
        }
    }
    printf("Fin de tournée pour le véhicule %s\n", vehicule->type);
}

/* ===============================
   PLANIFICATION MULTI-JOURS
   =============================== */
void multi_day_scheduling(Delivery *deliveries, int nbDeliveries, Vehicle *vehicules, int nbVehicules, int num_days, Graph *graph)
{
    typedef struct
    {
        int cost;
        int used_capacity;
        int last_time;
        int completed;
    } DPState;

    // Allocation du tableau DP avec dimensions [num_days][nbDeliveries+1][nbVehicules]
    DPState ***dp = malloc(num_days * sizeof(DPState **));
    for (int d = 0; d < num_days; d++)
    {
        dp[d] = malloc((nbDeliveries + 1) * sizeof(DPState *));
        for (int i = 0; i <= nbDeliveries; i++)
            dp[d][i] = malloc(nbVehicules * sizeof(DPState));
    }

    // Initialisation
    for (int d = 0; d < num_days; d++)
    {
        for (int i = 0; i <= nbDeliveries; i++)
        {
            for (int v = 0; v < nbVehicules; v++)
            {
                dp[d][i][v].cost = INT_MAX;
                dp[d][i][v].used_capacity = 0;
                dp[d][i][v].last_time = vehicules[v].dispo_debut;
                dp[d][i][v].completed = 0;
            }
        }
    }
    for (int v = 0; v < nbVehicules; v++)
        dp[0][0][v].cost = 0;

    // Remplissage du DP (simplifié)
    for (int d = 0; d < num_days; d++)
    {
        for (int i = 1; i <= nbDeliveries; i++)
        {
            for (int v = 0; v < nbVehicules; v++)
            {
                if (d > 0)
                    dp[d][i][v] = dp[d - 1][i][v];

                Delivery curr = deliveries[i - 1];
                if (curr.day <= d)
                {
                    int prev_cost = (d > 0) ? dp[d - 1][i - 1][v].cost : dp[0][i - 1][v].cost;
                    if (prev_cost != INT_MAX)
                    {
                        AdjListNode *edge = graph->array[curr.origin].head;
                        while (edge && edge->dest != curr.destination)
                            edge = edge->next;
                        if (edge)
                        {
                            int travel_time = (int)edge->attr.baseTime;
                            int new_time = dp[d][i - 1][v].last_time + travel_time;
                            if (new_time <= vehicules[v].dispo_fin &&
                                new_time <= curr.deadline &&
                                dp[d][i - 1][v].used_capacity + curr.volume <= vehicules[v].capacity)
                            {
                                int new_cost = prev_cost + (int)(edge->attr.distance * vehicules[v].cost_per_km);
                                if (new_cost < dp[d][i][v].cost)
                                {
                                    dp[d][i][v].cost = new_cost;
                                    dp[d][i][v].used_capacity = dp[d][i - 1][v].used_capacity + curr.volume;
                                    dp[d][i][v].last_time = new_time;
                                    dp[d][i][v].completed = dp[d][i - 1][v].completed + 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    int min_cost = INT_MAX, best_day = 0, best_vehicle = 0;
    for (int d = 0; d < num_days; d++)
    {
        for (int v = 0; v < nbVehicules; v++)
        {
            if (dp[d][nbDeliveries][v].cost < min_cost)
            {
                min_cost = dp[d][nbDeliveries][v].cost;
                best_day = d;
                best_vehicle = v;
            }
        }
    }
    printf("\n--- Meilleure solution multi-jours ---\n");
    printf("Coût total: %d\n", min_cost);
    printf("Jour: %d\n", best_day);
    printf("Véhicule: %d\n", best_vehicle);
    printf("Livraisons complétées: %d/%d\n", dp[best_day][nbDeliveries][best_vehicle].completed, nbDeliveries);

    for (int d = 0; d < num_days; d++)
    {
        for (int i = 0; i <= nbDeliveries; i++)
            free(dp[d][i]);
        free(dp[d]);
    }
    free(dp);
}

/* Fin de optimize.c */
