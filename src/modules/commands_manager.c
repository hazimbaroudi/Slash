#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "commands_manager.h"

int last_command_exit_value = 0;

/* --------------------------- static declaration --------------------------- */

/**
 * Execute une commande et renvoie ca valeur de retour.
 */
static int execute_command(char *args[]);

/* -------------------------------------------------------------------------- */


/* ------------------------ functions implementation ------------------------ */

int execute_external_command(char *args[])
{
    int wstatus;
    switch (fork()) {
    case -1:
        perror("external command forking");
        return 1;

    case 0:
        sl_signals_handling(DEFAULT);
        execvp(*args, args);
        exit(COMMAND_NOT_FOUND);

    default:
        wait(&wstatus);
        if (WIFSIGNALED(wstatus))
            return COMMAND_SIGNALED;

        return WEXITSTATUS(wstatus);
    }
}


int process_command(char *args[])
{
    char **cmds[MAX_ARGS_NUMBER / 2] = { 0 };
    int exit_value = 0;
    int cmd_count = 0;

    glob(args);
    if(!is_valid_pipe(args))
        goto error;

    cmd_count = parse_pipe(args, cmds);
    /* no pipe proccess */
    if(cmd_count == 1) {
        if (process_redirection(cmds[0]))
            goto error;

        exit_value = execute_command(cmds[0]);
        if (restore_all())
            goto error;

        return exit_value;
    }
    /* pipelines proccess */
    if(!is_valid_syntax_in_pipe(cmds) || create_pipe(cmd_count))
        goto error;

    for (int i = 0; i < cmd_count; i++) {
        process_pipe(i, cmd_count);
        process_redirection(cmds[i]);

        exit_value = execute_command(cmds[i]);
        restore_stdio(STDERR_FILENO);
    }
    if(restore_stdio(STDOUT_FILENO) || restore_stdio(STDIN_FILENO))
        goto error;

    return exit_value;
error:
    restore_all();
    return ECHEC_REDIRECTION;
}

void sl_signals_handling(sl_handler_type h_type)
{
    struct sigaction sa = { 0 };
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = (int)h_type == 0 ? SIG_IGN : SIG_DFL;

    if (sigaction(SIGTERM, &sa, NULL) == -1)
        goto error;
    if (sigaction(SIGINT, &sa, NULL) == -1)
        goto error;

    return;
error:
    perror("sigaction");
    exit(EXIT_FAILURE);
}

/* -------------------------------------------------------------------------- */


/* --------------------- static functions implementation -------------------- */

static int execute_command(char **args)
{
    if (is_builtin_commands(*args))
        return execute_builtin_commands(args);

    return execute_external_command(args);
}

/* -------------------------------------------------------------------------- */
