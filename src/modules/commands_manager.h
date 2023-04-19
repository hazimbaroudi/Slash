#ifndef COMMANDS_MANAGER_H
#define COMMANDS_MANAGER_H

/* error command return values */
#define COMMAND_NOT_FOUND   127
#define COMMAND_SIGNALED    255
#define ECHEC_REDIRECTION   1

#include "sl_builtin_commands.h"
#include "redirections.h"
#include "jokers.h"

/* Valeur de retour de la dernière commande exécutée */
extern int last_command_exit_value;

/**
 * Gestion de l'exécution d'une commande.
 *  - Jokers
 *  - Redirections
 *  - Pipe
 */
int process_command(char *args[]);


typedef enum { IGNORE, DEFAULT } sl_handler_type;

/**
 * Redéfini le comportement des signaux SIGTERM et SIGINT.
 */
void sl_signals_handling(sl_handler_type h_type);

#endif /* COMMANDS_MANAGER_H */