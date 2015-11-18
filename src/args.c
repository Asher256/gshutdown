/*
 * This file is part of gshutdown.
 *
 * Copyright (c) 2006-2007, GShutdown Team <gshutdown@gmail.com>
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

#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "debug.h"
#include "timer.h"
#include "misc.h"
#include "gui.h"
#include "args.h"
#include "callback.h"

static gboolean date_string_to_values(gchar *, gboolean);

/*
 * Handle all arguments.
 *
 * Return TRUE on error.
 */
gboolean handle_arguments(int argc, char **argv)
{
    gchar *action = NULL;
    gchar *launch_at = NULL;
    gchar *launch_from_now = NULL;
    gboolean launch_now = FALSE;
    gboolean minimize_window = FALSE; /* Minimiser la fenêtre par défaut TODO mettre dans un fichier à part avec le traitement des arguments */
    gboolean ctx_ok; /* arguments parsing is ok ? */
    GOptionContext *ctx;
    GOptionEntry options[] = {
        {"launch-after-delay", 'h', 0, G_OPTION_ARG_STRING, &launch_from_now, _("Launch the action after a certain delay : HH:MM"), NULL},
        {"launch-at", 'a', 0, G_OPTION_ARG_STRING, &launch_at, _("Launch the action at : HH:MM YYYY/MM/DD"), NULL},
        {"launch-now", 'n', 0, G_OPTION_ARG_NONE, &launch_now, _("Launch the action now."), NULL},
        {"action", 'c', 0, G_OPTION_ARG_STRING, &action, _("Available actions are : \"shutdown\", \"reboot\" or \"logout\""), NULL},
        {"minimize", 'm', 0, G_OPTION_ARG_NONE, &minimize_window, _("Minimize the window at startup."), NULL},
        {NULL}
    };
   
    /* Initialisation of g_option */
    ctx = g_option_context_new(NULL);
    g_option_context_add_main_entries(ctx, options, NULL);
    ctx_ok = g_option_context_parse(ctx, &argc, &argv, NULL);
    g_option_context_free(ctx);

    /* First errors */
    if(ctx_ok == FALSE) {
        fprintf(stderr, _("Error when parsing the arguments...\n"));
        quit(QUIT_FAILURE);
    }

    {
        gint launch_counter = 0;

        if(launch_from_now != NULL) 
            launch_counter++;

        if(launch_at != NULL)
            launch_counter++;

        if(launch_now != FALSE) 
            launch_counter++;

        if(launch_counter > 2) {
            fprintf(stderr, _("You cannot call --launch-after-delay, --launch-at or --launch-now at the same time.\n"));
            quit(QUIT_FAILURE);
        }
    }

    /* Minimize the window */
    if(minimize_window == TRUE) {
        if(gui_systray_icon_is_showed()) 
            gtk_widget_hide(main_window);
        else 
            gtk_window_iconify(GTK_WINDOW(main_window));
    }

    /* action */
    if(action != NULL) {
        if(g_str_equal(action, "shutdown"))
            gtk_combo_box_set_active(combo_action, 0);

        else if(g_str_equal(action, "reboot")) 
            gtk_combo_box_set_active(combo_action, 1);

        else if(g_str_equal(action, "logout")) 
            gtk_combo_box_set_active(combo_action, 2);

        else {
            fprintf(stderr, _("The authorized values in --action option are : shutdown, reboot or logout\n"));
            return TRUE;
        }
    }

    /* Launch at/now/from now */
    if(launch_now == TRUE) {
        gtk_combo_box_set_active(combo_time_type, 2);
        action_run(values.current_action);
    }
    else if(launch_at != NULL || launch_from_now != NULL) {
        gint combo_value = 1;
        gchar *value = launch_from_now;
        gchar *option = "--launch-after-delay";
        gboolean time_only = TRUE; /* Only the time is parsed */

        if(launch_at != NULL) { 
            combo_value = 0;
            value = launch_at;
            option = "--launch-at";
            time_only = FALSE;
        }
        
        gtk_combo_box_set_active(combo_time_type, combo_value);
        if(date_string_to_values(value, time_only) == TRUE) {
            fprintf(stderr, _("Error on parsing the content of %s : \"%s\".\n"), option, value);
            quit(QUIT_FAILURE);
        }

        on_btn_start_clicked(btn_start, NULL);
    }

    /* Free */
    g_free(launch_at);
    g_free(launch_from_now);
    g_free(action);
    
    return FALSE;
}

/* date_string_to_values()
 *
 * Convert string format to values structure. If time_only == TRUE
 * only the time is parsed.
 *
 * Format: HH:MM YYYY/MM/DD
 */
static gboolean date_string_to_values(gchar *string, gboolean time_only)
{
    gchar **list;
    gboolean result = FALSE;
    
    /* special date */
    gchar *tomorrow[] = {"tomorrow", "demain"};
    gint tomorrow_size = 2;

    /* In the time parsing, of this variable is true, the function 
     * can accept a time format like this : "MM" else than "HH:MM". */
    gboolean accept_minute_alone = time_only;

    /* valid() {{{2
     *
     * Generic tester used by valid_time()
     * and valid_date().
     *
     * TODO message d'erreur détaillé pour que
     *      l'utilisateur sache quels sont ses
     *      erreurs.
     *
     * type :
     *  0 = valid time
     *  1 = valid date
     */
    gboolean valid(gchar *string, gchar type)
    {
        gchar *string_p;         /* useful pointer */
        gchar **list;            /* contain the splits of 'string' */
        gchar *separator;        /* separator. e.g ":" for time */
        gint value[3];           /* values of string (splited) */
        size_t maximum_len_size; /* size of maximum_len[] table */
        size_t maximum_len[3] = {4, 2, 2};

        if(type == 0) { /* time */
            separator = ":";
            maximum_len_size = 2;
        }
        else { /* date */
            gint i;
            separator = "/";
            maximum_len_size = 3;

            /* special strings */
            for(i = 0; i < tomorrow_size; i++) {
                if(g_str_equal(tomorrow[i], string) == TRUE)
                    return TRUE;
            }
        }

        /* All characters are conform ? */
        string_p = string;
        while(*string_p != 0) {
            if(!g_ascii_isdigit(*string_p) && *string_p != *separator)
                return FALSE;
            string_p++;
        }

        /* Split and test the numbers */
        list = g_strsplit(string, separator, 0);
        if(list != NULL && g_strv_length(list) > 1) {
            gint i;
            gboolean valid = TRUE;

            for(i = 0; i < maximum_len_size; i++) {
                if(list[i] == NULL || strlen(list[i]) > maximum_len[i]) {
                    valid = FALSE;
                    break;
                }

                value[i] = atoi(list[i]);
            }

            g_strfreev(list);

            /* test values */
            if(valid == TRUE) {
                /* time */
                if(type == 0) {
                    for(i = 0; i < 2; i++) 
                        if(value[i] < 0 || value[i] > 59)
                            valid = FALSE;
                }
                /* date */
                else {
                    gint nb_days;
                    gint year = value[0];
                    gint month = value[1];
                    gint day = value[2];

                    /* month */
                    if(month > 12 || month < 1)
                        valid = FALSE;

                    /* year */
                    else if(year < 1970)
                        valid = FALSE;

                    /* day */
                    else {
                        nb_days = nb_days_in_month(month - 1, year);
                        if(day > nb_days || day < 1) 
                            valid = FALSE;
                    }
                }
            }

            return valid;
        }
        /* An error in time because it's not separated with ":" ?
         * It's ok, if accept_minute_alone is TRUE */
        else if(accept_minute_alone == TRUE) {
            /* Test if the string contain only a number */
            gchar *p_string = string;
            while(*p_string != 0) {
                if(!g_ascii_isdigit(*p_string)) 
                    return FALSE; /* Error ! */
                else
                    p_string++;
            }

            return TRUE; /* Good, it's the minute ! */
        }


        return FALSE;
    }

    /* valid() }}}2 */

    /* Valid time and date {{{2 */

    /* 
     * Function to test if string is a time
     * Format: HH:MM.
     */
    gboolean valid_time(gchar *string)
    { return valid(string, 0); }

    /* 
     * Function to test if 'string' is a date.
     *
     * Format: YYYY/MM/DD
     *
     * NB: it's better to use g_strstrip() with 
     *     string before calling this function.
     */
    gboolean valid_date(gchar *string)
    { return valid(string, 1); }

    /* }}}2 */

    /* Extract time and date {{{2 */

    /* 
     * Attention, you must verify 'element' with valid_time()
     * before calling this function.
     */
    void extract_time_string_to_gui(const gchar *element)
    {
        gchar **sub_element = g_strsplit(element, ":", 2);

        if(sub_element != NULL && g_strv_length(sub_element) > 1) {
            gtk_spin_button_set_value(spin_hour, atoi(sub_element[0]));
            gtk_spin_button_set_value(spin_minute, atoi(sub_element[1]));
            g_strfreev(sub_element);
        }
        else if(accept_minute_alone == TRUE) {
            gtk_spin_button_set_value(spin_hour, 0);
            gtk_spin_button_set_value(spin_minute, atoi(element));
        }

        debug_printf("The time is now %s\n", element);
    }

    /* 
     * Attention, you muse verify 'element' with valid_date()
     * before calling this function.
     */
    void extract_date_string_to_gui(const gchar *element)
    {
        gchar **sub_element;
        gint i;

        /* tomorrow */
        for(i = 0; i < tomorrow_size; i++) {
            if(g_str_equal(element, tomorrow[i]) == TRUE) {
                guint year, month, day;
                guint nb_days;

                gtk_calendar_get_date(calendar, &year, &month, &day);

                /* TODO Incrémenter le jour avec une fonction plus haut niveau ? */
                nb_days = nb_days_in_month(month, year);
                if(day >= nb_days) {
                    month++;
                    if(month > 11) {
                        month = 0;
                        year++;
                    }

                    day = 1;
                }
                else
                    day++;

                gtk_calendar_select_month(calendar, month, year);
                gtk_calendar_select_day(calendar, day);

                return;
            }
        }
       
        /* Handle format YYYY/MM/DD */
        sub_element = g_strsplit(element, "/", 3);

        if(sub_element != NULL) {
            gint year = atoi(sub_element[0]);
            gint month = atoi(sub_element[1]);
            gint day = atoi(sub_element[2]);

            gtk_calendar_select_month(calendar, month - 1, year);
            gtk_calendar_select_day(calendar, day);

            g_strfreev(sub_element);
        }

        debug_printf("The time is now %s\n", element);
    }

    /* }}}2 */

    /* Split all elements (time or date) available in 'string' */
    list = g_strsplit(string, " ", 0);

    if(list != NULL) {
        char **list_element = list;
        gint nb_element_handled = 0;

        while(*list_element != NULL && nb_element_handled < 2) {
            char *element = g_strstrip(*list_element);

            /* ignore empty string */
            if(element[0] == 0) {
                list_element++;
                continue;
            }
            /* it's a date */
            else if(time_only == FALSE && valid_date(element)) {
                extract_date_string_to_gui(element);
                nb_element_handled++;
            }
            /* it's a time */
            else if(valid_time(element)) {
                extract_time_string_to_gui(element);
                nb_element_handled++;
            }
            /* Error */
            else {
                /* Error message TODO */
                result = TRUE;
                break;
            }

            list_element++;
        }

        g_strfreev(list);
    }

    return result; /* Error */
}

/* vim:cindent:et:sw=4:ts=4:sts=4:tw=78:fenc=utf-8
*/
