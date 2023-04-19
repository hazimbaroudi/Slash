#ifndef SL_UTILS_H
#define SL_UTILS_H

#define MAX_ARGS_STRLEN     4096
#define MAX_ARGS_NUMBER     4096


/**
 * Parsing d'une string
 * Découpe 'input' en token stocker dans 'buffer' selon des délimiteurs.
 * Renvoie le nombre de tokens crées
 */
int str_parse(char *input, char *buffer[], char delimiters[]);


/**
 * Compare si 'suf' est un suffix de 'str'.
 */
int strsuf(char *str, char *suf);


/**
 * Divise en deux la chaine 'input' à partir du délimiteur 'd',
 * Et place les deux parties dans les buffers 'l' et 'r'.
 */
void split_on_char(char *input, char l[], char r[], char d);


/**
 * Compare si les deux tableaux sont égaux.
 * Renvoie 1 si c'est le cas, 0 sinon.
 */
int strtab_cmp(char *tab1[], char *tab2[]);


/**
 * Renvoie la longueur d'un tableau.
 */
size_t tab_len(char *tab[]);


/**
 * Mets tous les éléments de 'tab' à NULL.
 */
void clear_tab(char *tab[]);


/**
 * Mets tous les éléments de 'tab' à NULL,
 * après avoir libérée la mémoire avec `free`.
 */
void free_tab(char *tab[]);


/**
 * Libère la mémoire de l'élément d'indice 'i',
 * et le mets à NULL.
 */
void free_at(char *tab[], int i);


/**
 * Enlève tous les '/' en trop dans un chemin absolu.
 */
void clean_path(char *path);

#endif /* SL_UTILS_H */
