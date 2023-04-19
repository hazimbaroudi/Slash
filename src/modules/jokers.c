#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include "jokers.h"

/* ---------------------------- static declaration -------------------------- */

/**
 * Expansion de *
 */
static void star_globbing(char *args[]);


/**
 * Expansion de **
 */
static void star_star_globbing(char *args[]);


/**
 * Expand toutes les étoiles pour les éléments de 'args',
 * et stock le résultat dans 'args'.
 * Renvoie le nombre d'éléments de 'args'.
 */
static int process_star(char *args[]);


/**
 * Expand une étoile de 'path' et stock le résultat dans 'buf'.
 * Renvoie le nombres d'éléments de 'buf' s'il n'a pas d'erreur.
 */
static int process_one(char path[], char *buf[]);

/**
 * Expand la double étoile en faisant l'expansion de * dans tous les
 * sous répertoires valides et stock le résultat dans 'buf'.
 */
static int process_star_star(char dir_to_open[], char glob_star[], char *buf[]);


/**
 * Découpe la chaine 'path' en deux selon le délimiter '*'.
 * Place les résultats dans 'dir' et 'end'.
 */
static void split_on_star(char path[], char dir[], char end[]);


/**
 * Déplace tout les éléments d'un tableau d'arguments
 * vers la gauche d'une place à partir d'un certain index, en
 * supprimant l'élément d'indice i
 */
static void shift_left(char *tab[], int i);


/**
 * Insère tous les éléments de 'src' dans 'dst' à l'indice 'i',
 * en décalant tout le reste de 'dst' vers la droite si besoin.
 */
static int insert_at(char *dst[], char *src[], int i);


/**
 * Récupère toutes les entrées d'un répertoire qui ont pour suffix 'suffix'.
 * Dans ce cas on concatène 'str_dir', (l'entrée) et 'end' et stock cette
 * nouvelle chaine dans 'buf'.
 * Renvoie le nombre d'entrées qui ont le bon suffix.
 */
static int get_entries_matches(char str_dir[], char suffix[], char end[],
                                             char *buf[], int match_dir);

/* -------------------------------------------------------------------------- */


/* ------------------------ functions implementation ------------------------ */

void glob(char *args[])
{
    star_star_globbing(args);
    star_globbing(args);
}

/* -------------------------------------------------------------------------- */


/* -------------------- static functions implementation --------------------- */

static void star_globbing(char *args[])
{
    char *buf[MAX_ARGS_NUMBER] = { 0 };
    int res = 0;

    for (int i = 0; args[i] != NULL;) {
        if (*args[i] == '-') {
            i++;
            continue;
        }
        buf[0] = strdup(args[i]);
        res = process_star(buf);
        if (res == 0) {
            i++;
            continue;
        }
        insert_at(args, buf, i);
        clear_tab(buf);
        i += res;
    }
}


static void star_star_globbing(char *args[])
{
    char dir_to_open[MAX_ARGS_NUMBER] = { 0 };
    char glob_star[MAX_ARGS_NUMBER] = { 0 };
    char *buf[MAX_ARGS_NUMBER] = { 0 };

    for (int i = 1; args[i] != NULL;) {
        if (strncmp(args[i], "**", 2)) {
            i++;
            continue;
        }
        *dir_to_open = '.';
        strcpy(glob_star, !strcmp(args[i], "**/") ? "*/" :
                           !strcmp(args[i], "**") ? "*" : args[i] + 3);
        int res = process_star_star(dir_to_open, glob_star, buf);
        if (res < 0) {
            dprintf(STDERR_FILENO, "process_star_star\n");
            goto error;
        } else if (res == 0) {
            i++;
            continue;
        }
        insert_at(args, buf, i);
        clear_tab(buf);
        i += res;
    }
error:
    free_tab(buf);
    return;
}


static int process_star_star(char dir_to_open[], char glob_star[], char *buf[])
{
    int match_count = 0;
    char path[MAX_ARGS_STRLEN] = { 0 };
    char *star_g[MAX_ARGS_NUMBER] = { 0 };

    struct stat st = { 0 };
    struct dirent *entry = NULL;
    DIR *dir = NULL;

    if (*dir_to_open != '.')
        strcat(strcat(path, dir_to_open), "/");

    strcat(path, glob_star);
    star_g[0] = strdup(path);

    if (!(dir = opendir(dir_to_open))) {
        perror("opendir");
        match_count = -1;
        goto error;
    }
    star_globbing(star_g);
    if (strcmp(star_g[0], path)) {
        match_count += tab_len(star_g);
        insert_at(buf, star_g, tab_len(buf));
    } else {
        free_at(star_g, 0);
    }
    clear_tab(star_g);
    while ((entry = readdir(dir))) {
        if (*entry->d_name == '.')
            continue;

        char new_path[MAX_ARGS_STRLEN] = { 0 };
        if (*dir_to_open != '.')
            strcat(strcat(new_path, dir_to_open), "/");

        strcat(new_path, entry->d_name);
        if (lstat(new_path, &st) == -1) {
            perror("lstat");
            match_count = -1;
            goto error;
        }
        if (S_ISLNK(st.st_mode) || !S_ISDIR(st.st_mode))
            continue;

        int res = process_star_star(new_path, glob_star, buf);
        if (res < 0) {
            dprintf(STDERR_FILENO, "process_star_star\n");
            match_count = -1;
            goto error;
        }
        match_count += res;
    }
error:
    free_tab(star_g);
    if (dir)
        closedir(dir);
    if (match_count == -1)
        free_tab(buf);
    return match_count;
}


static int process_star(char *args[])
{
    char *buf[MAX_ARGS_NUMBER] = { 0 };
    char *args_cpy[MAX_ARGS_NUMBER] = { 0 };
    int res = 0;

    for (int i = 0; args[i] != NULL; i++)
        args_cpy[i] = strdup(args[i]);

    for (int i = 0; args[i] != NULL;) {
        if (*args[i] == '-') {
            i++;
            continue;
        }
        res = process_one(args[i], buf);
        if (res == -2) {
            goto error;
        } else if (res == -1) {
            shift_left(args, i);
            continue;
        } else if (res == 0) {
            i++;
            continue;
        }
        insert_at(args, buf, i);
        clear_tab(buf);
        i += res;
    }
    if (strtab_cmp(args_cpy, args)) {
        free_tab(args_cpy);
        return tab_len(args);
    }
    free_tab(args_cpy);
    return process_star(args);
error:
    free_tab(args);
    free_tab(args_cpy);
    exit(EXIT_FAILURE);
}


static int process_one(char path[], char *buf[])
{
    char path_cpy[MAX_ARGS_STRLEN] = { 0 };
    char str_dir[MAX_ARGS_STRLEN] = { 0 };
    char suffix[MAX_ARGS_STRLEN] = { 0 };
    char end[MAX_ARGS_STRLEN] = { 0 };
    int match_dir = 0;

    if (!strstr(path, "*"))
        return 0;

    memmove(path_cpy, path, strlen(path));
    split_on_star(path_cpy, str_dir, end);

    memset(path_cpy, 0, strlen(path_cpy));
    memmove(path_cpy, end, strlen(end));
    memset(end, 0, strlen(end));

    char *end_ptr = strstr(path_cpy, "/");
    if (end_ptr) {
        match_dir = 1;
        strcpy(end, end_ptr);
        strncpy(suffix, path_cpy, strlen(path_cpy) - strlen(end_ptr));
        clean_path(end);
    } else {
        strcpy(suffix, path_cpy);
    }
    if ((strlen(path) == strlen(str_dir) && strlen(suffix) != 0) ||
        (*str_dir != '.' && str_dir[strlen(str_dir) - 1] != '/'))
        return 0;

    return get_entries_matches(str_dir, suffix, end, buf, match_dir);
}


static char *build_string(char str1[], char str2[], char str3[])
{
    size_t len = strlen(str1) + strlen(str2) + strlen(str3);
    char *str = calloc(len + 1, sizeof(char));
    if (!str)
        return NULL;

    if (strcmp(str1, "."))
        strcat(str, str1);

    return strcat(strcat(str, str2), str3);
}


static int get_entries_matches(char str_dir[], char suffix[], char end[],
                                             char *buf[], int match_dir)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    struct stat st = { 0 };

    char entry_buf[MAX_ARGS_STRLEN] = { 0 };
    size_t i = 0;

    if (!(dir = opendir(str_dir)))
        return -1;

    while ((entry = readdir(dir))) {
        if (*entry->d_name == '.' || strsuf(entry->d_name, suffix))
            continue;

        memset(entry_buf, 0, MAX_ARGS_STRLEN);
        strcpy(entry_buf, str_dir);
        if (*str_dir == '.')
            strcat(entry_buf, "/");

        strcat(entry_buf, entry->d_name);
        if (stat(entry_buf, &st) < 0)
            goto error;

        if (!S_ISDIR(st.st_mode) && match_dir)
            continue;

        buf[i] = build_string(str_dir, entry->d_name, end);
        if (!buf[i])
            goto error;

        if (!strstr(buf[i], "*") && access(buf[i], F_OK)) {
            shift_left(buf, i);
            continue;
        }
        i++;
    }
    closedir(dir);
    return i == 0 ? -1 : i;
error:
    closedir(dir);
    return -2;
}


static void shift_left(char *tab[], int i)
{
    free_at(tab, i);
    int nb_elts = tab_len(tab + i + 1);
    for (int j = 0; j < nb_elts; j++)
        tab[i + j] = tab[i + j + 1];

    tab[i + nb_elts] = NULL;
}


static int insert_at(char *dst[], char *src[], int i)
{
    size_t src_len = tab_len(src);
    size_t dst_len = tab_len(dst);

    if (src_len + dst_len > MAX_ARGS_NUMBER)
        return -1;

    if (src_len == 0) {
        shift_left(dst, i);
        return 0;
    }
    free_at(dst, i);
    if (dst[i + 1]) {
        dst_len = tab_len(dst + i + 1);
        for (int j = dst_len; j > 0; j--)
            dst[src_len + i + j - 1] = dst[i + j];
    }
    for (int j = 0; j < src_len; j++)
        dst[i + j] = src[j];

    return 0;
}


static void split_on_star(char path[], char dir[], char end[])
{
    char l[MAX_ARGS_STRLEN] = { 0 };
    char r[MAX_ARGS_STRLEN] = { 0 };

    split_on_char(path, dir, end, '*');
    if (*end != 0) {
        split_on_char(end, l, r, '*');
        char d[MAX_ARGS_STRLEN] = {0};
        strcat(strcat(strcat(d, dir), "*"), l);
        if (!access(d, F_OK)) {
            strcpy(dir, d);
            strcpy(end, r);
            return;
        }
    }
    if (!strcmp(dir, ""))
        *dir = '.';
}

/* -------------------------------------------------------------------------- */
