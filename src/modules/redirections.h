#ifndef REDIRECTIONS_H
#define REDIRECTIONS_H

#define REDIRECTION_NUMBER  7

#include "../utils/sl_utils.h"


/**
 * Gere les redirections pour le pipe (tube de la cmd a la position pos)
 */
int process_pipe(int pos, int nb_cmds);


/**
 * Gere les redirections
 * return 0 si il n'y a pas d'erreurs, 1 sinon
 */
int process_redirection(char *args[]);


/**
 * Creer les pipes en fonction de nb_cmds
 * return 0 si il n'y a pas d'erreurs, 1 sinon
 */
int create_pipe(int nb_cmds);


/**
 * Rétablie une entrée/sortie standard
 */
int restore_stdio(int fd);


/**
 * Rétablie les entrées/sorties standards
 */
int restore_all(void);


/**
 * Vérifie si les commandes de cmds ont une bonne syntaxe
 * par rapport aux redirections additionnelles
 *
 * return 0 si il n'y a pas d'erreurs, 1 sinon
 */
int is_valid_syntax_in_pipe(char **cmds[]);


/**
 * Parse les arguments en fonction du pipe et mets les
 * différentes commandes dans cmds
 *
 * return 0 si il n'y a pas d'erreurs, 1 sinon
 */
int parse_pipe(char *args[], char **cmds[]);


/**
 * return 0 si il n'y a pas d'erreurs de syntaxe, 1 sinon.
 * (pas de pipe au debut, pas 2 pipes de suites, pas de pipe a la fin)
 */
int is_valid_pipe(char *args[]);

#endif /* REDIRECTIONS_H */
