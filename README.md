SLASH
=====

Slash *(small laudable shell)* est un **interpréteur de commandes** interactif reprenant quelques fonctionnalités des shells comme `bash` ou encore `zsh`.


## Sommaire

- [Description](#description)
- [Installation et exécution](#installation-et-exécution)
  - [Prérequis](#prérequis)
  - [Compilation](#compilation)
  - [Lancement](#lancement)
- [Utilisations](#utilisations)
- [Fonctionnalités](#fonctionnalités)
  - [Commandes externes](#commandes-externes)
  - [Commandes internes](#commandes-internes)
  - [Jokers](#jokers)
  - [Redirections](#redirections)
  - [Signaux](#signaux)


## Description

Le shell `slash` donne la possibilité d'exécuter quelques **commandes internes**
ainsi que toutes les **commandes externes**, il permet également *l'expansion des chemins* contenant certains **jokers**.
`slash` gére les **redirections** des *flots standard* ainsi que les **combinaisons par tube**.

Pour plus de détails voir [Fonctionnalités](#fonctionnalités).



## Installation et exécution

Guide d'installation pour l'utilisation de `slash`.

### Prérequis

Premièrement, [`gcc`](https://gcc.gnu.org), [`make`](https://www.gnu.org/software/make/)
et la bibliothèque [`readline`](https://tiswww.case.edu/php/chet/readline/rltop.html) sont **requis** pour le fonctionnement du shell.

Ensuite cloner ce dépôt si ce n'est pas encore fait.
Ou mettez-le à jour avec `git pull`.

```bash
git clone git@gaufre.informatique.univ-paris-diderot.fr:timeus/projet-sy5-slash.git
```

Puis placez-vous dans le répertoire cloné.

```bash
cd projet-sy5-slash
```

### Compilation

La **compilation** et **le nettoyage des fichiers provisoires** se font via les commandes:

- `make` pour compiler l'entièreté du projet.
- `make clean` pour effacer les fichiers provisoires.

Voir [Makefile](Makefile) pour plus de détails.

### Lancement

Enfin pour lancer `slash` exécuté la commande `./slash`.

## Utilisations

Pour utiliser `slash`, voici un exemple;

```bash
[0].../jects/projet-sy5-slash$ cd
[0]/home/ryan$ pwd -L
/home/ryan
[0]/home/ryan$ cd repertoire-inexistant
-slash: cd: no such file or directory: repertoire-inexistant
[1]/home/ryan$ exit
proccess slash finished with exit code 1
```

## Fonctionnalités

`slash` implémente les fonctionnalités les suivantes:

### Commandes externes

`slash` peut exécuter toutes les commandes externes, avec ou sans
arguments, en tenant compte de la variable d'environnement `PATH`.


### Commandes internes

`slash` possède les commandes trois internes suivantes:

##### `exit [val]`

Termine le processus `slash` avec comme valeur de retour `val` (ou par défaut la valeur de retour de la dernière commande
exécutée).

Lorsqu'il atteint la fin de son entrée standard (ie si l'utilisateur
saisit `ctrl-D` en mode interactif), `slash` se comporte comme si la
commande interne `exit` (sans paramètre) avait été saisie.

##### `pwd [-L | -P]`

Affiche la (plus précisément, une) référence absolue du répertoire de travail 
courant :

- avec l'option `-P`, sa référence absolue *physique*, c'est-à-dire ne
  faisant intervenir aucun lien symbolique;

- avec l'option `-L` (option par défaut), sa référence absolue *logique*,
  déduite des paramètres des précédents changements de répertoire
  courant, et contenant éventuellement des liens symboliques.


##### `cd [-L | -P] [ref | -]`

Change de répertoire de travail courant en le répertoire `ref` (s'il
s'agit d'une référence valide), le précédent répertoire de travail si le
paramètre est `-`, ou `$HOME` en l'absence de paramètre.

Avec l'option `-P`, `ref` (et en particulier ses composantes `..`) est
interprétée au regard de la structure physique de l'arborescence.

Avec l'option `-L` (option par défaut), `ref` (et en particulier ses
composantes `..`) est interprétée de manière logique (`a/../b` est
interprétée comme `b`) si cela a du sens, et de manière physique sinon.

##### `history [-c] `

Affiche l'historique de commandes

Avec l'option `-c` supprime l'historique de commandes

##### `type [cmd]`

Affiche le type de `cmd` , si il s'agit d'une commande interne ou d'une commande externe

### Jokers

`slash` réalise l'expansion des jokers suivants :

- `*` : tout **préfixe** (ne commençant pas par `'.'`) d'un **nom de
  base** (valide, donc en particulier non vide);

- `**/` : tout préfixe de chemin **physique** de la forme `*/*/.../` (en 
  particulier, chaque composante est non vide, il s'agit donc de références relatives au répertoire courant);

### Redirections

`slash` gère les redirections suivantes :

- `cmd < fic` : redirection de l'entrée standard de la commande `cmd` sur
  le fichier (ordinaire, tube nommé...) `fic`

- `cmd > fic` : redirection de la sortie standard de la commande `cmd`
  sur le fichier (ordinaire, tube nommé...) `fic` **sans écrasement**
  (ie, échoue si `fic` existe déjà)

- `cmd >| fic` : redirection de la sortie standard de la commande `cmd`
  sur le fichier (ordinaire, tube nommé...) `fic` **avec écrasement**
  éventuel

- `cmd >> fic` : redirection de la sortie standard de la commande `cmd`
  sur le fichier (ordinaire, tube nommé...) `fic` **en concaténation**

- `cmd 2> fic` : redirection de la sortie erreur standard de la commande
  `cmd` sur le fichier (ordinaire, tube nommé...) `fic` **sans
  écrasement** (ie, échoue si `fic` existe déjà)

- `cmd 2>| fic` : redirection de la sortie erreur standard de la commande
  `cmd` sur le fichier (ordinaire, tube nommé...) `fic` **avec
  écrasement** éventuel

- `cmd 2>> fic` : redirection de la sortie erreur standard de la commande
  `cmd` sur le fichier (ordinaire, tube nommé...) `fic` **en
  concaténation**

- `cmd1 | cmd2` : redirection de la sortie standard de `cmd1` et de
  l'entrée standard de `cmd2` sur un même tube anonyme

Les espaces de part et d'autre des symboles de redirection sont requis.

Par ailleurs, dans un *pipeline* `cmd1 | cmd2 | ... | cmdn`, les
redirections additionnelles sont autorisées :

- redirection de l'entrée standard de `cmd1` 
- redirection de la sortie standard de `cmdn`
- redirection des sorties erreurs des `n` commandes

### Signaux

`slash` ignore les signaux `SIGINT` et `SIGTERM`, contrairement aux
processus exécutant des commandes externes.
