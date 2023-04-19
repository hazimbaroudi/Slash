#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <dirent.h>
#include <readline/history.h>

#include "sl_builtin_commands.h"


/* ---------------------------- static declaration -------------------------- */

/**
 * - Avec l'option -P, sa référence absolue physique,
 *   c'est-à-dire ne faisant intervenir aucun lien symbolique;
 * - Avec l'option -L (option par défaut), sa référence absolue logique,
 *   déduite des paramètres des précédents changements de répertoire
 *   courant, et contenant éventuellement des liens symboliques.
 */
static int sl_pwd(char *args[]);


/**
 * Change de répertoire de travail courant en le répertoire ref,
 * s'il s'agit d'une référence valide:
 * - Avec le paramètre -, le précédent répertoire de travail.
 * - En l'absence de paramètre $HOME.
 *
 * - Avec l'option -P, ref (et en particulier ses composantes ..) est
 *   interprétée au regard de la structure physique de l'arborescence.
 * - Avec l'option -L (option par défaut), ref (et en particulier ses
 *   composantes ..) est interprétée de manière logique (a/../b est
 *   interprétée comme b) si cela a du sens, et de manière physique sinon.
 *
 * La valeur de retour est 0 en cas de succès, 1 en cas d'échec.
 */
static int sl_cd(char *args[]);


/**
 * - Termine le processus slash avec comme valeur de retour val
 *   (ou par défaut la valeur de retour de la dernière commande exécutée).
 * - Lorsqu'il atteint la fin de son entrée standard (ie si l'utilisateur
 *   saisit ctrl-D en mode interactif), slash se comporte comme si la
 *   commande interne exit (sans paramètre) avait été saisie.
 */
static int sl_exit(char *args[]);


/**
 * Met la variable d'environnement OLDPWD à PWD et PWD à 'pwd'
 * return int 0 en cas de succès, 1 en cas d'échec.
 */
static int set_pwd(char pwd[]);


/**
 * Place dans 'new_pwd' le chemin vers 'dir'
 * selon la valeur de 'phy_intp'.
 * (i.e selon si l'interprétation doit être physique ou logique)
 */
static int set_paths(char *dir, char new_pwd[], int phy_intp);


/**
 * affiche l'historique de commandes
 * avec l'option -c : supprime l'historique
 */
static int sl_history(char *args[]);


/**
 * affiche le type de commande
 * si c'est une commande interne ou externe
 */
static int sl_type(char *args[]);


/**
 * Place la forme canonique (i.e nettoyée) de 'path' dans 'new_path'
 * Exemple : toto/./titi/.././tata -> toto/tata
 */
static void canonicalization(char path[], char new_path[]);

/* Tableau contenant l'ensemble des noms des commandes internes */
static const char *str_builtin_cmd[] = {
    "cd", "pwd", "exit", "history", "type"
};

/* Tableau contenant l'ensemble des fonctions des commandes internes */
static int (*builtin_cmd[])(char *args[]) = {
    sl_cd, sl_pwd, sl_exit, sl_history, sl_type
};

/* -------------------------------------------------------------------------- */


/* ------------------------ functions implementation ------------------------ */

int is_builtin_commands(char *command)
{
    for (int i = 0; i < BUILTIN_COMMANDS_COUNT; i++) {
        if (!strcmp(command, str_builtin_cmd[i]))
            return 1;
    }
    return 0;
}

int execute_builtin_commands(char *args[])
{
    for (int i = 0; i < BUILTIN_COMMANDS_COUNT; i++) {
        if (!strcmp(args[0], str_builtin_cmd[i]))
            return builtin_cmd[i](args);
    }
    /* N'arrive jamais */
    return COMMAND_NOT_FOUND;
}

/* -------------------------------------------------------------------------- */


/* -------------------- static functions implementation --------------------- */

static int sl_pwd(char *args[])
{
    char *cwd = NULL, buf[PATH_MAX] = {0};

    if (!(args[1] && strcmp(args[1], "-L")))
        cwd = getenv("PWD");
    else if (!strcmp(args[1], "-P"))
        cwd = getcwd(buf, PATH_MAX);
    else
        goto err_invalid_args;

    if (!cwd) {
        perror("-slash: pwd: ");
        return 1;
    }
    dprintf(STDOUT_FILENO, "%s\n", cwd);
    return 0;

err_invalid_args:
    dprintf(STDERR_FILENO, "-slash: pwd: %s: invalid option\n"
						"pwd: usage: pwd [-L | -P]\n", args[1]);
    return 1;
}

static int sl_cd(char *args[])
{
    char new_pwd[PATH_MAX] = { 0 };
    char *dir = NULL;
    int phy_intp = 0;

    if (args[1]) {
        dir = args[2];
        if (!strcmp(args[1], "-P")) {
            phy_intp = 1;
        } else if (strcmp(args[1], "-L")) {
            if (strcmp(args[1], "-") && *args[1] == '-')
                goto err_invalid_args;
            if (args[2])
                goto err_nb_args;
            dir = args[1];
        }
        if (args[3] != NULL)
            goto err_nb_args;
    }
    if (set_paths(dir, new_pwd, phy_intp))
        goto err;
    if (!chdir(new_pwd))
        return set_pwd(new_pwd);

    dprintf(STDERR_FILENO, "-slash: cd: %s: %s\n", dir, strerror(errno));
    return 1;

err_nb_args:
    dprintf(STDERR_FILENO, "-slash: cd: too many arguments\n");
    return 1;
err_invalid_args:
    dprintf(STDERR_FILENO, "-slash: cd: %s: invalid option\n"
						   "cd: usage: cd [-L | -P] [dir]\n", args[1]);
    return 1;
err:
    perror("-slash: cd: ");
    return 1;
}

static int sl_exit(char *args[])
{
    int exit_code = last_command_exit_value;
    char *end_ptr = NULL;

    if (args[1]) {
        exit_code = strtol(args[1], &end_ptr, 10);
        if (strcmp(end_ptr, ""))
            exit_code = 2;
    }

    printf("process slash finished with exit code %d\n", exit_code);
    exit(exit_code);
}


static int set_paths(char *dir, char new_pwd[], int phy_intp)
{
    char path[PATH_MAX] = { 0 };
    char *pwd = getenv("PWD");

    if (!pwd)
        return 1;
    if (!dir || !strcmp(dir, "-")) {
        char *dest = dir == NULL ? getenv("HOME") : getenv("OLDPWD");
        if (!dest)
            return 1;
        memmove(new_pwd, dest, strlen(dest));
        return 0;
    }
    if (phy_intp)
        goto physical_interpretation;
    if (*dir == '/')
        memmove(path, dir, strlen(dir));
    else
        sprintf(path, "%s%s%s", pwd, strcmp(pwd, "/") ? "/" : "", dir);

    canonicalization(path, new_pwd);
    /* Si l'interprétation logique n'a pas de sens -> interprétation physique */
    if (chdir(new_pwd))
        goto physical_interpretation;

    return 0;
physical_interpretation:
    memset(new_pwd, 0, strlen(new_pwd));
    memmove(new_pwd, dir, strlen(dir));
    return realpath(dir, new_pwd) == NULL;
}


void canonicalization(char path[], char new_path[])
{
    static char path_cpy[PATH_MAX];
    char *tokens[PATH_MAX] = { 0 };

    memset(path_cpy, 0, PATH_MAX);
    memmove(path_cpy, path, PATH_MAX);
    int len = 0;
    int nb_token = str_parse(path, tokens, "/");
    for (int i = 0; i < nb_token; i++) {
        if (!strcmp(tokens[i], "."))
            continue;
        if (strcmp(tokens[i], "..") && i + 1 < nb_token) {
            if (!strcmp(tokens[i + 1], "..")) {
                i++;
                continue;
            }
        }
        sprintf(new_path + len, "/%s", tokens[i]);
        len += 1 + strlen(tokens[i]);
    }
    if (len == 0) {
        *new_path = '/';
        return;
    }
    if (!strncmp(new_path, "/..", 3))
        memmove(new_path + (new_path[3] != '/'), new_path + 3, len);
    if (!strcmp(path_cpy, new_path))
        return;

    memmove(path, new_path, PATH_MAX);
    memset(new_path, 0, PATH_MAX);
    canonicalization(path, new_path);
}


static int set_pwd(char pwd[])
{
    char *oldpwd = getenv("OLDPWD");
    if (setenv("OLDPWD", getenv("PWD"), 1))
        return 1;
    if (setenv("PWD", pwd, 1)) {
        setenv("OLDPWD", oldpwd, 1);
        return 1;
    }
    return 0;
}


static int sl_history(char *args[])
{
#ifndef __APPLE_CC__
    if(args[1]) {
        if (!strcmp(args[1], "-c")) {
            clear_history();
            dprintf(STDOUT_FILENO, "-slash: deleted commands history\n");
            return 0;
        }
        dprintf(STDERR_FILENO, "-slash: history: invalid option\n");
        return 1;
    }
    HIST_ENTRY **history = history_list();
    if (!history) {
        dprintf(STDERR_FILENO, "-slash: empty history\n");
        return 1;
    }
    for (int i = 0; history[i]; i++)
        dprintf(STDOUT_FILENO, "%d: %s\n", i + history_base, history[i]->line);
#endif
    return 0;
}


static int sl_type(char *args[])
{
    #define MAX_PATH_STRLEN 1024

    char *path = getenv("PATH");
    char path_cpy[MAX_PATH_STRLEN] = { 0 };
    char *buffer[MAX_PATH_STRLEN] = { 0 };
    struct dirent *entry = NULL;
    DIR *dir = NULL;

    if (!args[1]) {
        dprintf(STDERR_FILENO, "-slash: type: invalid argument\n");
        return 1;
    } else if (args[2]) {
        dprintf(STDERR_FILENO, "-slash: type: too many arguments\n");
        return 1;
    } else if (is_builtin_commands(args[1])) {
        dprintf(STDOUT_FILENO,"%s is a primitive of slash\n", args[1]);
        return 0;
    }
    if (strlen(path) >= MAX_PATH_STRLEN)
        return 1;

    strcpy(path_cpy,path);
    int nb_dir = str_parse(path_cpy, buffer,":");
    for (int i = 0; i < nb_dir; i++) {
        if (!(dir = opendir(buffer[i])))
            goto not_found;

        while ((entry = readdir(dir))) {
            if(*entry->d_name == '.' || strcmp(entry->d_name, args[1])) {
                continue;
            }
            dprintf(STDOUT_FILENO,"%s is %s/%s\n",entry->d_name,buffer[i],entry->d_name );
            closedir(dir);
            return 0;
        }
        closedir(dir);
    }
not_found:
    dprintf(STDERR_FILENO,"slash: type: %s : not found\n", args[1]);
    return 1;
}

/* -------------------------------------------------------------------------- */
