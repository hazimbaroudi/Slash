#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "redirections.h"


/* -------------------------- static declaration -------------------------- */

static int std_fds_cpy[3] = { 0 };
static int fdpipe[][2] = { 0 };

static const char *str_redirections[REDIRECTION_NUMBER] = {
    "<",
    ">", ">|", ">>",
    "2>", "2>|", "2>>"
};

struct redirection
{
    const char *name;
    int flags;
    int fd;
};

static void set_redirection_data(int r_number, struct redirection *rd);
static int is_redirection(char *command);
static int do_redirections(char *str_rd, char *file);
static int init_redirections(void);
static int is_valid_syntax_in_pipe_command(char *args[], int begin, int end);

/* -------------------------------------------------------------------------- */


/* ------------------------ functions implementation ------------------------ */

int process_pipe(int pos, int nb_cmds)
{
    if (pos < nb_cmds - 1) {
        if (dup2(fdpipe[pos][1], STDOUT_FILENO) == -1)
            return 1;

        close(fdpipe[pos][1]);
    }
    if (pos > 0) {
        if (dup2(fdpipe[pos - 1][0], STDIN_FILENO) == -1)
            return 1;

        close(fdpipe[pos - 1][0]);
    }
    if (pos == nb_cmds - 1 && restore_stdio(STDOUT_FILENO))
            return 1;

    return 0;
}


int process_redirection(char *args[])
{
    for (int i = 0; args[i] != NULL; i++) {
        if (!is_redirection(args[i]))
            continue;

        if (do_redirections(args[i], args[i + 1]))
            return 1;

        free_at(args, i);
    }
    return 0;
}


int create_pipe(int nb_cmds)
{
    for (int i = 0; i < nb_cmds - 1; i++) {
        if (pipe(fdpipe[i]) == -1)
            return 1;
    }
    return init_redirections();
}


int restore_stdio(int fd)
{
    if (!std_fds_cpy[fd])
        return 0;

    if (dup2(std_fds_cpy[fd], fd) == -1)
        return 1;

    close(std_fds_cpy[fd]);
    std_fds_cpy[fd] = 0;
    return 0;
}


int restore_all(void)
{
    for (int i = 0; i < 3; i++) {
        if (restore_stdio(i))
            return 1;
    }
    return 0;
}


int is_valid_syntax_in_pipe(char **cmds[])
{
    int i = 0;
    if (!is_valid_syntax_in_pipe_command(cmds[i], 1, 4))
        goto error;

    for (i = 1; cmds[i + 1] != NULL; i++) {
        if (!is_valid_syntax_in_pipe_command(cmds[i], 0, 4))
            goto error;
    }
    if (!is_valid_syntax_in_pipe_command(cmds[i], 0, 1))
        goto error;

    return 1;
error:
    dprintf(STDERR_FILENO, "-slash: syntax error\n");
    return 0;
}


int parse_pipe(char *args[], char **cmds[])
{
    int cmd_number = 0;
    int cmd_index = 0;
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|")) {
            cmds[cmd_number] = &args[cmd_index];
            continue;
        }
        free_at(args, i);
        cmd_index = i + 1;
        cmd_number++;
    }
    return cmd_number + 1;
}


int is_valid_pipe(char *args[])
{
    int i = 0;
    if (!strcmp(args[i], "|"))
        goto error;

    for (i = 0; args[i + 1] != NULL ; i++) {
        if (!strcmp(args[i], "|") && !strcmp(args[i + 1], "|"))
            goto error;
    }
    if (!strcmp(args[i],"|"))
        goto error;

    return 1;
error:
    dprintf(STDERR_FILENO, "-slash: syntax error\n");
    return 0;
}

/* -------------------------------------------------------------------------- */


/* --------------------- static functions implementation -------------------- */

static int is_redirection(char *command)
{
    for (int i = 0; i < REDIRECTION_NUMBER; i++) {
        if (!strcmp(command, str_redirections[i]))
            return 1;
    }
    return 0;
}


static void set_redirection_data(int r_number, struct redirection *rd)
{
    int fd = (r_number + 2) / 3;
    int type = (r_number + 2) % 3;
    int flags = 0;

    if (r_number == 0) {
        flags = O_RDONLY;
    } else {
        flags = O_WRONLY | O_CREAT;
        flags |= type == 0 ? O_EXCL : type == 1 ? O_TRUNC : O_APPEND;
    }
    rd->name = str_redirections[r_number];
    rd->flags = flags;
    rd->fd = fd;
}


static int do_redirections(char *str_rd, char *file)
{
    int fd = 0;
    struct redirection rd = { 0 };
    for (int i = 0; i < REDIRECTION_NUMBER; i++) {
        set_redirection_data(i, &rd);
        if (strcmp(str_rd, rd.name))
            continue;

        if ((fd = open(file, rd.flags, 0666)) < 0)
            return 1;

        if (!std_fds_cpy[rd.fd]) {
            int std_cpy = dup(rd.fd);
            if (std_cpy == -1)
                goto error;

            std_fds_cpy[rd.fd] = std_cpy;
        }
        if (dup2(fd, rd.fd) == -1)
            goto error;

        close(fd);
        return 0;
    }
    return 0;
error:
    close(fd);
    return 1;
}


static int is_valid_syntax_in_pipe_command(char *args[], int begin, int end)
{
    for (int i = 0; args[i] != NULL; i++) {
        for (int rd = begin; rd < end; rd++) {
            if (!strcmp(args[i], str_redirections[rd]))
                return 0;
        }
    }
    return 1;
}


static int init_redirections(void)
{
    for (int i = 0; i < 3; i++) {
        if ((std_fds_cpy[i] = dup(i)) == -1)
            return 1;
    }
    return 0;
}

/* -------------------------------------------------------------------------- */
