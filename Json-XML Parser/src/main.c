#include <stdio.h>
#include <stdlib.h>
#include "cli.h"
#include "utils/logger.h" // <— ajouté
#include "utils/error.h"  // optionnel si vous utilisez error_* dans main

int main(int argc, char *argv[])
{
    // Initialisation du logger et du système d'erreurs
    logger_init();

    // Traitement des arguments ou lancement du menu interactif
    if (argc > 1)
    {
        if (cli_process_args(argc, argv) != 0)
        {
            fprintf(stderr, "Erreur dans le traitement des arguments.\n");
            return EXIT_FAILURE;
        }
    }
    else
    {
        cli_loop();
    }

    // Nettoyage avant sortie
    logger_shutdown();
    return EXIT_SUCCESS;
}
