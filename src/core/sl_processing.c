#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "sl_processing.h"

/* -------------------------- static declaration -------------------------- */

/* Buffer pour le prompt de slash */
static char sl_prompt[MAX_PROMPT_LEN * 2];


/**
 * Stock la ligne de commande renvoyer par la fonction 'readline'.
 * Variable allouée avec malloc
 */
static char *user_input = NULL;

/**
 * Tableau qui stock les arguments de la ligne de commande.
 * Attention chaque éléments du tableau ont été alloués avec `malloc`.
 */
static char *user_args[MAX_ARGS_NUMBER] = { 0 };


/* Initialisation de slash */
static void init_slash(void);


/**
 * Nettoyage des variables alloué avec malloc
 */
static void clean_slash(void);


/**
 * Retourne le prompt formater à afficher de type :
 * "[0]/home/titi/pas/trop/long$ "
 * "[1]...ong/mais/la/ca/depasse$ "
 * Ainsi qu'un code couleur
 */
static void refresh_prompt(void);


/* Initialisation de slash */
static void init_slash(void);


/**
 * Lis la ligne de commande via 'readline' et la retourne 
 */
static char *read_input(void);


/**
 * Parsing de la ligne de commande selon le délimiteur 'SL_DELIMITER'.
 * Stockage des tokens dans le buffer 'args'
 */
static void parse_input(char *input, char *args[]);


/**
 * Exécution de la ligne commande (si elle existe/valide)
 * Retourne la valeur de retour de la commande si elle existe
 * et COMMAND_NOT_FOUND (i.e 127) sinon
 */
static int execute_command(char *args[]);

/* -------------------------------------------------------------------------- */


/* ------------------------ functions implementation ------------------------ */

void process_slash(void)
{
    init_slash();
    while (1) {
        user_input = read_input();
        if (!user_input)
            break;
        if (!strcmp(user_input, ""))
            continue;

        parse_input(user_input, user_args);
        last_command_exit_value = execute_command(user_args);
        clean_slash();
    }
    /**
     * EOF reçu, slash termine avec valeur de retour
     * de la dernière commande exécutée
     */
    dprintf(STDOUT_FILENO,"\nprocess slash finished with exit code %d\n",
                                                last_command_exit_value);
    exit(last_command_exit_value);
}

/* -------------------------------------------------------------------------- */


/* --------------------- static functions implementation -------------------- */


static void init_slash(void)
{
    rl_outstream = stderr;
    last_command_exit_value = 0;
    atexit(&clean_slash);

    sl_signals_handling(IGNORE);
}


static void clean_slash(void)
{
    for (int i = 0; i < MAX_ARGS_NUMBER; i++) {
        if (user_args[i]) {
            free(user_args[i]);
            user_args[i] = NULL;
        }
    }
    if (user_input)
        free(user_input);
}


static void refresh_prompt(void)
{
    int e_val = last_command_exit_value;
    char *cwd = getenv("PWD");
    size_t path_len = strlen(cwd);

    memset(sl_prompt, 0, MAX_PROMPT_LEN * 2);
    if (e_val == 255)
        sprintf(sl_prompt, "%s[SIG]", e_val ? RED : GREEN);
    else
        sprintf(sl_prompt, "%s[%d]", e_val ? RED : GREEN, e_val);

    size_t pt_len = strlen(sl_prompt);
    size_t rl_pt_len = pt_len - COLOR_CODE_LEN;

    if (path_len + rl_pt_len + 2 <= MAX_PROMPT_LEN)
        sprintf(sl_prompt + pt_len, BLUE "%s", cwd);
    else
        sprintf(sl_prompt + pt_len, BLUE "...%s",
            cwd + (path_len - MAX_PROMPT_LEN + rl_pt_len + 5));

    sprintf(sl_prompt + strlen(sl_prompt), RESET "$ ");
}


static char *read_input(void)
{
    char *input_line = NULL;

    refresh_prompt();
    input_line = readline(sl_prompt);
    if (!input_line)
        return NULL;

    if (strlen(input_line) > MAX_ARGS_STRLEN) {
        dprintf(STDERR_FILENO, "-slash: too many arguments\n");
        free(input_line);
        return NULL;
    }
    if (*input_line)
        add_history(input_line);

    return input_line;
}


static void parse_input(char *input, char *args[])
{
    str_parse(input, args, SL_DELIMITER);
    for (int i = 0; args[i] != NULL; i++)
        args[i] = strdup(args[i]);
}


static int execute_command(char *args[])
{
    int return_val = process_command(args);
    if (return_val != COMMAND_NOT_FOUND)
        return return_val;

    /* Aucune commande n'a été reconnue */
    dprintf(STDERR_FILENO, "-slash: %s: command not found\n", args[0]);
    return COMMAND_NOT_FOUND;
}

/* -------------------------------------------------------------------------- */
