#ifndef CLI_H
#define CLI_H

int cli_init(void);
void cli_print_help(void);
int cli_process_args(int argc, char *argv[]);
void cli_loop(void);

#endif // CLI_H
