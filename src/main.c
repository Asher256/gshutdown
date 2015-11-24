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

#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <glib.h> /* g_atexit() */
#include "main.h"
#include "gui.h"
#include "misc.h"
#include "values.h"
#include "bacon.h"
#include "debug.h"
#include "args.h"

/* Variables */
static BaconMessageConnection *bacon_connection = NULL;
static gboolean main_loop = FALSE; /* TRUE if gtk_main() is called */
static gboolean values_loaded = FALSE; /* TRUE if all values loaded from cfg */

/* Prototypes */
static void bacon_init();
static void bacon_on_message_received(const char *message, gpointer data);
static void signals_init();
static void signals_handle_sigchld(int signal);
static void signals_handle_sigint(int signal);
static void data_and_gui_init(int argc, char **argv);
static void gettext_init();
static void init(int argc, char **argv);

/*
 * Le remplacement de gettext, quand ENABLE_NLS n'est pas à TRUE.
 */
#ifndef ENABLE_NLS
char *internal_ngettext(const char *msgid, const char *msgid_plural,
                        unsigned long int n) {
    return (n <= 1) ? msgid : msgid_plural;
}
#endif

/*
 * Main program
 */
int main(int argc, char **argv)
{
    /* Initialisation */
    init(argc, argv);

    /* Main */
    main_loop = TRUE;
    gtk_main();

    return QUIT_SUCCESS;
}

/*
 * General init.
 */
static void init(int argc, char **argv)
{
    gettext_init();
    bacon_init();
    signals_init();
    data_and_gui_init(argc, argv);
}

/*
 * Gettext initialisation.
 */
static void gettext_init()
{
    #ifdef ENABLE_NLS
    bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
    #endif
}

/*
 * Bacon.
 */
static void bacon_init()
{
    bacon_connection = bacon_message_connection_new("gshutdown");

    if(bacon_connection != NULL) {
        if(bacon_message_connection_get_is_server(bacon_connection) == FALSE) {
            debug_printf("Still opened\n");

            bacon_message_connection_send(bacon_connection, "show");
            bacon_message_connection_free(bacon_connection);
            exit(0);
        }
        else {
              debug_printf("First opening\n"); 

            bacon_message_connection_set_callback(bacon_connection, bacon_on_message_received, NULL);
        }
    }
    else
        g_warning("Cannot create the 'gshutdown' connection.");
}

static void bacon_on_message_received(const char *message, gpointer data)
{
   if(g_str_equal(message, "show") == TRUE && main_window != NULL)
      gtk_widget_show(main_window);
}

/* 
 * Signals.
 */
static void signals_init()
{
    signal(SIGINT, signals_handle_sigint);
    signal(SIGCHLD, signals_handle_sigchld);
    g_atexit(called_at_exit);
}

static void signals_handle_sigint(int signal)
{
    debug_printf("SIGINT handled.\n");
    called_at_exit(); /* Free memory allocated, before quit */
}

static void signals_handle_sigchld(int signal)
{
    debug_printf("SIGCHLD handled.\n");
    wait(NULL);
}

/*
 * Init variables : default values and GUI.
 *
 * It must be called after gettext_init(), 
 * bacon_init() and signals_init().
 */
static void data_and_gui_init(int argc, char **argv)
{
    gboolean result;

    /* Default values of the variables */
    values_init();

    /* Gui */
    if(gui_init(argc, argv) == TRUE)
        quit(QUIT_FAILURE);

    /* Load configuration file */
    result = values_load(); /* It's important to call 
                               values_load() after gui_init() */

    if(result == TRUE) { /* Error ? */
        GtkWidget *dialog;
        dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
                                        GTK_DIALOG_MODAL,
                                        GTK_MESSAGE_ERROR,
                                        GTK_BUTTONS_OK,
                                        _("The configuration cannot be loaded from %s."), 
                                        "~/.gshutdown");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }

    /* Handle the arguments. It's important to call
     * this after values_load() */
    result = handle_arguments(argc, argv);
    if(result == TRUE)
        quit(QUIT_FAILURE);

    /* Show if needed detection (GDM, KDM, etc.) error warning */
    detect_error_dialog();

    /* It's ok */
    values_loaded = TRUE;
}

/*
 * This function is called at quit.
 */
void called_at_exit()
{
    static gboolean first_call = TRUE;

    if(first_call) {
        /* Change the "first call" flag */
        first_call = FALSE;

        /* Save the configuration */
        if(values_loaded == TRUE && values_save() == TRUE) {
            /* The condition "values_loaded == TRUE" is important 
             * if gshutdown cfg cannot be loaded. 
             *
             * For example, if the user send a SIGINt before 
             * loading of configuration */
            GtkWidget *dialog;
            
            dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
                                            GTK_DIALOG_MODAL,
                                            GTK_MESSAGE_ERROR,
                                            GTK_BUTTONS_OK,
                                            _("The configuration cannot be saved in %s."),
                                            "~/.gshutdown");

            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
        }

        debug_printf("atexit() function called!\n");

        if(main_loop == TRUE)
            gtk_main_quit();

        bacon_message_connection_free(bacon_connection);

        /* free all alocated variables */
        values_destroy();
    }
}

/* vim:cindent:et:sw=4:ts=4:sts=4:tw=78:fenc=utf-8
 */
