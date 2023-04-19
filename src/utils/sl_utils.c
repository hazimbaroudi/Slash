#include <stdlib.h>
#include <string.h>

#include "sl_utils.h"


int str_parse(char *input, char *buffer[], char delimiter[])
{
    int i = 0;
    char *tok = strtok(input, delimiter);
    while (tok != NULL) {
        buffer[i++] = tok;
        tok = strtok(NULL, delimiter); 
    }
    return i;
}


int strsuf(char *str, char *suf)
{
    char *str_suf = str + strlen(str) - strlen(suf);
    return strcmp(str_suf, suf);
}


void split_on_char(char *input, char l[], char r[], char d)
{
    size_t len = strlen(input);
    for (size_t i = 0; i < len; i++) {
        if (input[i] != d)
            continue;

        memmove(l, input, i);
        memmove(r, input + i + 1, len - i + 1);
        return;
    }
    memmove(l, input, len);
    memset(r, 0, strlen(r));
}


int strtab_cmp(char *tab1[], char *tab2[])
{
    size_t len1 = tab_len(tab1);
    size_t len2 = tab_len(tab2);

    if (len1 != len2)
        return 0;

    for (size_t i = 0; i < len1; i++) {
        if (strcmp(tab1[i], tab2[i]))
            return 0;
    }
    return 1;
}


size_t tab_len(char *tab[])
{
    for (size_t i = 0; i < MAX_ARGS_NUMBER; i++) {
        if (!tab[i])
            return i;
    }
    return MAX_ARGS_NUMBER;
}


void free_tab(char *tab[])
{
    if (!tab)
        return;

    for (int i = 0; i < MAX_ARGS_NUMBER; i++) {
        if (tab[i])
            free_at(tab, i);
    }
}


void clear_tab(char *tab[])
{
    if (!tab)
        return;

    for (int i = 0; i < MAX_ARGS_NUMBER; i++)
        tab[i] = NULL;
}


void free_at(char *tab[], int i)
{
    free(tab[i]);
    tab[i] = NULL;
}


void clean_path(char *path)
{
    char *buf[MAX_ARGS_STRLEN] = { 0 };
    char path_cpy[MAX_ARGS_STRLEN] = { 0 };
    int end_with_sl = path[strlen(path) - 1] == '/';

    memmove(path_cpy, path, strlen(path));
    memset(path, 0, strlen(path));
    int len = str_parse(path_cpy, buf, "/");
    for (int i = 0; i< len; i++)
        strcat(strcat(path, "/"), buf[i]);

    if (end_with_sl)
        strcat(path, "/");
}
