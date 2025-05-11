/* output.h */
#ifndef OUTPUT_H
#define OUTPUT_H

/**
 * Alloue une matrice V×V de float.
 * @param V nombre de lignes/colonnes
 * @return pointeur sur float*[V], à libérer avec freeMatrix()
 */
float **allocMatrix(int V);

/**
 * Libère une matrice allouée par allocMatrix().
 * @param m   matrice float**
 */
void freeMatrix(float **m);

/**
 * Affiche une matrice V×V avec un titre.
 * @param m     matrice float**
 * @param V     dimension
 * @param title chaîne à afficher avant la matrice
 */
void printMatrix(float **m, int V, const char *title);

/**
 * Écrit une matrice V×V dans un fichier CSV.
 * @param filename nom du fichier à créer/écraser
 * @param m        matrice float**
 * @param V        dimension
 * @return 0 si OK, -1 sinon
 */
int writeCsv(const char *filename, float **m, int V);

/**
 * Affiche un vecteur de taille V avec un titre.
 * @param v     vecteur float*
 * @param V     taille
 * @param title chaîne à afficher avant le vecteur
 */
void printVector(const float *v, int V, const char *title);

/**
 * Écrit un vecteur de taille V dans un fichier CSV (index,valeur).
 * @param filename nom du fichier
 * @param v        vecteur float*
 * @param V        taille
 * @return 0 si OK, -1 sinon
 */
int writeCsv1d(const char *filename, const float *v, int V);

/**
 * Affiche un tableau de composantes connexes (1..compCount).
 * @param components tableau int[V] de numéro de composante
 * @param V          nombre de nœuds
 */
void printComponents(const int *components, int V);

/**
 * Affiche un tableau de points d’articulation (0/1).
 * @param artPoints tableau int[V], 1 si point d’articulation
 * @param V         nombre de nœuds
 */
void printArtPoints(const int *artPoints, int V);

#endif /* OUTPUT_H */
