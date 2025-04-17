#include <stdio.h>
#include <string.h>
#include "cli.h"

int cli_init(void)
{
    return 0;
}

void cli_print_help(void)
{
    printf("Usage: projetalgo [--help] [--input <f>] [--run-scenario <n>] [--export <fmt>]\n");
}

int cli_process_args(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    printf("[cli_process_args stub]\n");
    return 0;
}

void cli_loop(void)
{
    printf("[cli_loop stub] exiting immediately\n");
}
