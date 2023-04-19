#ifndef SL_BUILTIN_COMMANDS_H
#define SL_BUILTIN_COMMANDS_H

#define BUILTIN_COMMANDS_COUNT  5

#include "commands_manager.h"


/**
 * Vérifie si 'command' est une commande interne.
 * Revoie 1 si c'est le cas 0 sinon
 */
int is_builtin_commands(char *command);


/**
 * Exécute la commande interne se trouvant dans 'args[0]'.
 * 'args[0]' est toujours une commande interne "voir is_builtin_commands".
 * La valeur de retour est celle de la commande interne exécutée
 */
int execute_builtin_commands(char *args[]);

#endif /* SL_BUILTIN_COMMANDS_H */