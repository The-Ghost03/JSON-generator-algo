/* menu.h */
#ifndef MENU_H
#define MENU_H

/** Options du menu principal */
typedef enum
{
    LOAD_JSON = 1,
    LOAD_XML = 2,
    RUN_FLOYD = 3,
    RUN_BELLMAN = 4,
    DETECT_CYCLE = 5,
    COMPONENTS = 6,
    ARTICULATION = 7,
    EXIT = 8
} MenuOption;

/** Initialise le module menu (aucune ressource pour l’instant) */
void initMenu(void);

/**
 * Affiche le menu et lit le choix de l'utilisateur.
 * Réessaie tant que le choix n'est pas valide.
 * @return MenuOption sélectionné
 */
MenuOption displayMenu(void);

/** Libère les ressources du module menu (aucune pour l’instant) */
void freeMenu(void);

/**
 * Demande à l'utilisateur le chemin d'un fichier JSON.
 * Alloue une chaîne qu'il faut libérer avec free().
 * @param path_out pointeur vers char* qui recevra l'adresse allouée
 * @return 0 si OK, -1 sinon
 */
int askJsonPath(char **path_out);

/**
 * Demande à l'utilisateur le chemin d'un fichier XML.
 * Alloue une chaîne qu'il faut libérer avec free().
 * @param path_out pointeur vers char* qui recevra l'adresse allouée
 * @return 0 si OK, -1 sinon
 */
int askXmlPath(char **path_out);

/**
 * Demande à l'utilisateur un numéro de nœud source valide [0..V-1].
 * Répète tant que l'entrée n'est pas correcte.
 * @param V nombre de nœuds dans le graphe
 * @return indice du nœud source
 */
int askSourceNode(int V);

#endif /* MENU_H */
