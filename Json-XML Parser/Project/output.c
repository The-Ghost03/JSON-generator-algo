/* output.c */
#include "output.h"
#include <stdio.h>
#include <stdlib.h>

float **allocMatrix(int V)
{
    float **m = malloc(V * sizeof(*m));
    if (!m)
        return NULL;
    for (int i = 0; i < V; i++)
    {
        m[i] = malloc(V * sizeof(*m[i]));
        if (!m[i])
        {
            for (int k = 0; k < i; k++)
                free(m[k]);
            free(m);
            return NULL;
        }
    }
    return m;
}

void freeMatrix(float **m)
{
    if (!m)
        return;
    /* On suppose que l'appelant connaît la dimension et libère les lignes */
    /* Si vous voulez libérer sans dimension, conservez V globalement */
    /* Exemple d'utilisation : freeMatrix(fwDist); where fwDist was allocMatrix(V). */
    /* Ici on arrête après free(m) pour éviter UB si on ne connaît pas V. */
    free(m);
}

void printMatrix(float **m, int V, const char *title)
{
    printf("\n%s\n", title);
    /* En-tête de colonnes */
    printf("     ");
    for (int j = 0; j < V; j++)
        printf("%8d", j);
    printf("\n");
    for (int i = 0; i < V; i++)
    {
        printf("%4d ", i);
        for (int j = 0; j < V; j++)
        {
            float x = m[i][j];
            if (x == (1.0f / 0.0f))
                printf("       INF");
            else if (x != x)
                printf("       NaN");
            else
                printf("%8.2f", x);
        }
        printf("\n");
    }
}

int writeCsv(const char *filename, float **m, int V)
{
    FILE *f = fopen(filename, "w");
    if (!f)
        return -1;
    /* Optionnel : en-tête */
    for (int j = 0; j < V; j++)
    {
        fprintf(f, j ? ",%d" : "%d", j);
    }
    fprintf(f, "\n");
    for (int i = 0; i < V; i++)
    {
        for (int j = 0; j < V; j++)
        {
            float x = m[i][j];
            if (x == (1.0f / 0.0f))
                fprintf(f, j ? ",INF" : "INF");
            else if (x != x)
                fprintf(f, j ? ",NaN" : "NaN");
            else
                fprintf(f, j ? ",%.6f" : "%.6f", x);
        }
        fprintf(f, "\n");
    }
    fclose(f);
    return 0;
}

void printVector(const float *v, int V, const char *title)
{
    printf("\n%s\n", title);
    for (int i = 0; i < V; i++)
    {
        float x = v[i];
        if (x == (1.0f / 0.0f))
            printf("%4d: INF\n", i);
        else if (x != x)
            printf("%4d: NaN\n", i);
        else
            printf("%4d: %.2f\n", i, x);
    }
}

int writeCsv1d(const char *filename, const float *v, int V)
{
    FILE *f = fopen(filename, "w");
    if (!f)
        return -1;
    fprintf(f, "node,value\n");
    for (int i = 0; i < V; i++)
    {
        float x = v[i];
        if (x == (1.0f / 0.0f))
            fprintf(f, "%d,INF\n", i);
        else if (x != x)
            fprintf(f, "%d,NaN\n", i);
        else
            fprintf(f, "%d,%.6f\n", i, x);
    }
    fclose(f);
    return 0;
}

void printComponents(const int *components, int V)
{
    printf("\nComposantes connexes par nœud:\n");
    for (int i = 0; i < V; i++)
    {
        printf("  nœud %2d -> composante %d\n", i, components[i]);
    }
}

void printArtPoints(const int *artPoints, int V)
{
    printf("\nPoints d'articulation:\n");
    int count = 0;
    for (int i = 0; i < V; i++)
    {
        if (artPoints[i])
        {
            printf("  nœud %2d\n", i);
            count++;
        }
    }
    if (!count)
    {
        printf("  Aucun point d'articulation trouvé.\n");
    }
}
