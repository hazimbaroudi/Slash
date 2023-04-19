#ifndef SL_PROCESSING_H
#define SL_PROCESSING_H

#define SL_DELIMITER        " \t"

#define MAX_PROMPT_LEN      30
#define COLOR_CODE_LEN      7

#define GREEN               "\001\033[32m\002"
#define RED                 "\001\033[91m\002"
#define BLUE                "\001\033[36m\002"
#define RESET               "\001\033[00m\002"

#include "../modules/commands_manager.h"

/**
 * Initialisation et boucle de slash,
 * et effectue sur la ligne de commande les opérations:
 * - Lecture
 * - Parsing
 * - Exécution
 */
void process_slash(void);

#endif /* SL_PROCESSING_H */