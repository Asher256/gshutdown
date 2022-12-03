/*
 * This file is part of gshutdown.
 *
 * Copyright (c) GShutdown Team <gshutdown@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with This program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <unistd.h> /* X_OK */
#include <stdlib.h>
#include <glib.h> 
#include <glib/gstdio.h>
#include "config.h"
#include "misc.h"
#include "main.h"
#include "binreloc.h"

/*
 * Run a shell command, with arguments.
 * Return TRUE on error.
 */
#include <stdio.h>
#include <stdlib.h>    /* exit() */
#include <unistd.h>    /* fork() */
#include <sys/types.h> /* Used by fork() and stat() */
gboolean run_command(const gchar *command)
{
    gboolean result = FALSE;
    gint argc;
    gchar **argv = NULL; /* All arguments of command */
    gchar *path = NULL;  /* The full path of command */

    if(g_shell_parse_argv(command, &argc, &argv, NULL) == FALSE)
        return TRUE;

    path = g_find_program_in_path(argv[0]);
    if(path != NULL && argc >= 1) {
        pid_t pid;

        /* Save */
        gchar *save = argv[0];
        argv[0] = path; /* The full path */

        /* Run the command */
        pid = fork();
        if(pid == 0) { /* Child */
            execv(argv[0], argv);
            exit(1);
        }
        else if(pid < 0) { /* Error */
            perror("fork");
            result = TRUE;
        }

        /* Restore */
        argv[0] = save;
    }
    else
        result = TRUE; /* Error */

    g_free(path);
    g_strfreev(argv);

    return result;
}

/*
 * Which returns the pathnames of the files which would be executed in 
 * the current environment. It does this by searching the PATH for 
 * executable files matching the names of the arguments.
 *
 * The returned string are allocated. It must be freed by g_free(). 
 * If the command are not found, NULL are returned.
 *
 */

/*
 * Quit the program.
 *
 * Macros can be used in status :
 *    QUIT_FAILURE
 *    QUIT_SUCCESS
 */
void quit(int status)
{
    called_at_exit();
    exit(status);
}

/* 
 * Return TRUE if a command exists in $PATH.
 */
gboolean command_exists(const gchar *full_command)
{
    const gchar *env_path;
    gchar **env_path_split, **save_env_path_split;
    gchar result = FALSE; /* Default = Error */
    gint argc;
    gchar **argv = NULL;  /* All arguments of command */
    gchar *command;

    /* Fist tests */
    if(full_command == NULL || g_str_equal(full_command, "") == TRUE)
        return FALSE;

    if(g_shell_parse_argv(full_command, &argc, &argv, NULL) == FALSE)
        return FALSE;

    if(argc < 1) {
        g_strfreev(argv);
        return FALSE;
    }

    /* Get the command name */
    command = argv[0];
    argv[0] = NULL;
    g_strfreev(argv);

    /* Absolute or relative path */
    if(g_str_has_prefix(command, G_DIR_SEPARATOR_S) == TRUE ||
       g_str_has_prefix(command, ".." G_DIR_SEPARATOR_S) == TRUE ||
       g_str_has_prefix(command, "." G_DIR_SEPARATOR_S) == TRUE)
    {
        if(g_access(command, X_OK) == 0)
            result = TRUE;

        g_free(command);
        return result;
    }

    /* Read $PATH */
    if(!(env_path = g_getenv("PATH"))) {
        g_free(command);
        return FALSE;
    }

    /* Split $PATH variable */
    env_path_split = g_strsplit(env_path, G_SEARCHPATH_SEPARATOR_S, 0);
    save_env_path_split = env_path_split;
    if(env_path_split == (gchar **)0) {
        g_free(command);
        return FALSE;
    }

    /* Scan all directories */
    while(*env_path_split != 0) {
        gchar *new_path = g_strdup_printf("%s%s%s", 
                                          *env_path_split, 
                                          G_DIR_SEPARATOR_S, 
                                          command);
        
        if(g_access(new_path, X_OK) == 0) {
            g_free(new_path);
            result = TRUE;
            break;
        }

        g_free(new_path);

        env_path_split++;
    }

    g_strfreev(save_env_path_split);
    g_free(command);

    return result;
}

/*
 * Return a path of a data file.
 *
 * For example, if filename contain file.glade,
 * this function return "/usr/share/gshutdown/file.glade"
 * or "./file.glade" if it's in this directory.
 *
 * If the data file are not found, it return NULL.
 *
 * NOTE: You must free the returned string with
 *       g_free().
 */
gchar *data_path(gchar *filename)
{
#ifdef PACKAGE_DATA_DIR
    gchar *path;
#endif

#ifdef ENABLE_BINRELOC
    {
        static gboolean binreloc_inited = FALSE;
        gchar *dir;

        if(binreloc_inited == FALSE) {
            gbr_init(NULL);
            binreloc_inited = TRUE;
        }

        dir = gbr_find_data_dir(NULL);
        if(dir) {
            path = g_strdup_printf("%s/gshutdown/%s",
                                          dir, filename);
            #ifdef DEBUG
            printf("Binreloc detected data dir: %s\n", dir);
            #endif
            free(dir);
            if(g_file_test(path, G_FILE_TEST_EXISTS) != FALSE) {
                return path;
            }
            else
                g_free(path);
        }
    }
#endif

    /* This directory */
    if(g_file_test(filename, G_FILE_TEST_EXISTS) != 0) 
        return g_strdup(filename);

    /* $PREFIX/share/gshutdown */
#ifdef PACKAGE_DATA_DIR
    path = g_strdup_printf("%s%cgshutdown%c%s", PACKAGE_DATA_DIR, G_DIR_SEPARATOR,
                                                G_DIR_SEPARATOR, filename);
    if(g_file_test(path, G_FILE_TEST_EXISTS) != FALSE)
        return path;
    else
        g_free(path);
#endif

    return NULL;
}

/* vim:cindent:et:sw=4:ts=4:sts=4:tw=78:fenc=utf-8
 */
