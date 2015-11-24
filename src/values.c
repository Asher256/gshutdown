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

#define DISABLE_GDM

#include <stdio.h>
#include <string.h>
#include <glib.h>
#include "values.h"
#include "gui.h"
#include "detectwm.h"
#include "gdm.h"
#include "debug.h"

/* The group name in the configuration file ~/.gshutdown */
#define GROUP_NAME "general"

/* Global variables */
struct t_values values;

/* Prototypes */
static GKeyFile *load_configuration();
static gboolean save_configuration(GKeyFile *key_file);

/*
 * Detect automatically the Desktop and the 
 * Display manager.
 */
void values_action_method_autodetect()
{
    const gchar *gdm_session;
    const gchar *kdm_session;
    gchar *window_manager = NULL;
    static gboolean first_call = TRUE;

    if(first_call == FALSE)
        return;
    else
        first_call = FALSE;

    /* Default values */
    values.detected_display_manager = DISPLAY_MANAGER_NONE;
    values.detected_desktop = DESKTOP_NONE;

    /* == DETECTION WITH THE WINDOW MANAGER == */
    window_manager = detect_window_manager();
    if(window_manager != NULL) {
        /* GNOME */
        if(g_str_equal(window_manager, "Metacity") == TRUE) {
            values.detected_desktop = DESKTOP_GNOME;
            debug_printf("Metacity = GNOME !\n");
        }
        /* KDE */
        else if(g_str_equal(window_manager, "KWin") == TRUE) {
            values.detected_desktop = DESKTOP_KDE;
            debug_printf("KWin = KDE !\n");
        }
        /* XFCE */
        else if(g_str_equal(window_manager, "Xfwm4") == TRUE) {
            values.detected_desktop = DESKTOP_XFCE;
            debug_printf("Xfwm4 = XFCE !\n");
        }
        #ifdef DEBUG
        else
            debug_printf("The Window Manager is not recognized : %s\n", window_manager);
        #endif

        g_free(window_manager);
    }

    /* == DETECT GDM == */
    #ifndef DISABLE_GDM
    if(gdm_supports_logout_action(GDM_LOGOUT_ACTION_SHUTDOWN)) {
        /*TODO it's the best solution to detect GDM ? */
        values.detected_display_manager = DISPLAY_MANAGER_GDM;
    }
    #endif

    /* == DETECTION WITH ENVIRONMENT VARIABLE == */

    if(values.detected_desktop == DESKTOP_NONE) {
        const gchar *desktop_session;

        /* GNOME */
        desktop_session = g_getenv("GNOME_DESKTOP_SESSION_ID");
        if(desktop_session != NULL && *desktop_session != 0)
            values.detected_desktop = DESKTOP_GNOME;
    }

    /* GDM */
    if(values.detected_display_manager == DISPLAY_MANAGER_NONE) {
        gdm_session = g_getenv("GDMSESSION");
        if(gdm_session != NULL && *gdm_session != 0)
            values.detected_display_manager = DISPLAY_MANAGER_GDM;
    }

    /* KDM */
    if(values.detected_display_manager == DISPLAY_MANAGER_NONE) {
        kdm_session = g_getenv("XDM_MANAGED");
        if(kdm_session != NULL && *kdm_session != 0)
            values.detected_display_manager = DISPLAY_MANAGER_KDM;
    }
}


/*
 * Write all variables in the configuration.
 *
 * Return TRUE on error.
 */ 
gboolean values_save()
{
    gboolean result = FALSE;

    /* Load the configuration */
    GKeyFile *key_file = load_configuration();

    /* Change values */
    if(key_file) {
        #define SET_INT(key, value) g_key_file_set_integer(key_file, GROUP_NAME, key, value);
        #define SET_BOOL(key, value) g_key_file_set_boolean(key_file, GROUP_NAME, key, value);
        #define SET_STRING(key, value) g_key_file_set_string(key_file, GROUP_NAME, key, value);
        #define SET_INT_LIST(key, value, length) g_key_file_set_integer_list(key_file, GROUP_NAME, key, value, length);

        SET_BOOL("show_notifications", values.show_notifications);
        SET_BOOL("iconify", values.iconify);
        SET_INT("show_systray_icon", values.systray_icon);

        SET_BOOL("command_before_action_enabled", values.command_before_action_enabled);
        SET_STRING("command_before_action", values.command_before_action);

        SET_INT("action_method", values.action_method)
        SET_STRING("action_shutdown.command", action_get_command(values.action_shutdown));
        SET_STRING("action_restart.command", action_get_command(values.action_restart));
        SET_STRING("action_logout.command", action_get_command(values.action_logout));

        SET_INT("selected_display_manager", values.selected_display_manager);
        SET_INT("selected_desktop", values.selected_desktop);

        SET_INT("time_when_notify_nb", *values.time_when_notify.nb);
        SET_INT_LIST("time_when_notify_list", values.time_when_notify.time, *values.time_when_notify.nb);

        #undef SET_INT
        #undef SET_BOOL
        #undef SET_STRING
        #undef SET_INT_LIST
    }

    /* Save the configuration */
    result = save_configuration(key_file);

    if(key_file)
        g_key_file_free(key_file);

    debug_printf("VALUES SAVED\n");

    return result;
}

/*
 * Load all variables from the configuration file.
 *
 * Return TRUE on error.
 */
gboolean values_load()
{
    gchar *string;
    gint integer;

    /* Load the configuration */
    GKeyFile *key_file = load_configuration();

    /* Load values */
    #define GET_BOOL(key, variable) \
        if(key_file != NULL && g_key_file_has_key(key_file, GROUP_NAME, key, NULL) == TRUE) \
            variable = g_key_file_get_boolean(key_file, GROUP_NAME, key, NULL);
    #define GET_INT(key, variable) \
        if(key_file != NULL && g_key_file_has_key(key_file, GROUP_NAME, key, NULL) == TRUE) \
            variable = g_key_file_get_integer(key_file, GROUP_NAME, key, NULL);
    #define GET_STRING(key, variable) \
        if(key_file != NULL && g_key_file_has_key(key_file, GROUP_NAME, key, NULL) == TRUE) \
            variable = g_key_file_get_string(key_file, GROUP_NAME, key, NULL);
    #define GET_INT_LIST(key, variable, length) \
        if(key_file != NULL && g_key_file_has_key(key_file, GROUP_NAME, key, NULL) == TRUE) \
            variable = g_key_file_get_integer_list(key_file, GROUP_NAME, key, length , NULL);
    #define NOTNULL(str) ((str != NULL)?str:"")

    /* Show notifications */
    values.show_notifications = TRUE;
    GET_BOOL("show_notifications", values.show_notifications);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_show_notifications), 
                                 values.show_notifications);

    /* Iconify */
    values.iconify = FALSE;
    GET_BOOL("iconify", values.iconify);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_iconify), 
                                 values.iconify);

    /* Show/Hide systray icon */
    values.systray_icon = SYSTRAY_ICON_ALWAYS;
    GET_INT("show_systray_icon", values.systray_icon);

    if(values.systray_icon == SYSTRAY_ICON_ALWAYS)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_systray_always), TRUE);
    else if(values.systray_icon == SYSTRAY_ICON_IF_ACTIVE)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_systray_if_active), TRUE);
    else 
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_systray_never), TRUE);

    /* It's an Action custom command ? */
    integer = ACTION_METHOD_AUTODETECT;
    GET_INT("action_method", integer);

    if(integer == ACTION_METHOD_MANUAL) {
        values.action_method = ACTION_METHOD_MANUAL;
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_action_method_manual), TRUE);
    }
    else if(integer == ACTION_METHOD_CUSTOM_COMMAND) {
        values.action_method = ACTION_METHOD_CUSTOM_COMMAND;
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_action_method_custom_command), TRUE);
    }
    else {
        values.action_method = ACTION_METHOD_AUTODETECT;
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_action_method_autodetect), TRUE);
    }

    /* Command before action */
    values.command_before_action_enabled = FALSE;
    GET_BOOL("command_before_action_enabled", values.command_before_action_enabled);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_command_before_action),
                                 values.command_before_action_enabled);

    string = NULL;
    GET_STRING("command_before_action", string);
    values_set_command_before_action(NOTNULL(string));
    gtk_entry_set_text(GTK_ENTRY(entry_command_before_action), NOTNULL(string));
    if(string != NULL) 
        g_free(string);

    /* Action command */
    string = NULL;
    GET_STRING("action_shutdown.command", string);
    action_set_command(values.action_shutdown, NOTNULL(string));
    if(string != NULL) 
        g_free(string);

    string = NULL;
    GET_STRING("action_restart.command", string);
    action_set_command(values.action_restart, NOTNULL(string));
    if(string != NULL)
        g_free(string);

    string = NULL;
    GET_STRING("action_logout.command", string);
    action_set_command(values.action_logout, NOTNULL(string));
    if(string != NULL)
        g_free(string);

    /* Manual desktop */
    integer = DESKTOP_AUTO;
    GET_INT("selected_desktop", integer);
    {
        gint index = 0; /* Default = auto */
        values.selected_desktop = DESKTOP_AUTO; /* = Auto */

        if(integer == DESKTOP_GNOME) {
            index = 1;
            values.selected_desktop = DESKTOP_GNOME;
        }
        else if(integer == DESKTOP_KDE) {
            index = 2;
            values.selected_desktop = DESKTOP_KDE;
        }
        else if(integer == DESKTOP_XFCE) {
            index = 3;
            values.selected_desktop = DESKTOP_XFCE;
        }

        gtk_combo_box_set_active(GTK_COMBO_BOX(combo_method_manual_desktop), index);
    }

    /* Display manager */
    integer = DISPLAY_MANAGER_AUTO;
    GET_INT("selected_display_manager", integer);
    {
        gint index = 0; /* Default = auto */
        values.selected_display_manager = DISPLAY_MANAGER_AUTO; /* = Auto */

        if(integer == DISPLAY_MANAGER_GDM) {
            index = 1;
            values.selected_display_manager = DISPLAY_MANAGER_GDM;
        }
        else if(integer == DISPLAY_MANAGER_KDM) {
            index = 2;
            values.selected_display_manager = DISPLAY_MANAGER_KDM;
        }

        gtk_combo_box_set_active(GTK_COMBO_BOX(combo_method_manual_display_manager), index);
    }

    integer = -1;
    GET_INT("time_when_notify_nb", integer);
    if(integer >= 0) {
        *values.time_when_notify.nb = integer;
        
        if(values.time_when_notify.time != NULL)
            g_free(values.time_when_notify.time);

        values.time_when_notify.time = g_malloc(integer * sizeof(gsize));

        GET_INT_LIST("time_when_notify_list", values.time_when_notify.time, \
                                              values.time_when_notify.nb);

    }

    #undef GET_BOOL
    #undef GET_INT
    #undef GET_STRING
    #undef GET_INT_LIST
    #undef NOTNULL

    if(key_file != NULL)
        g_key_file_free(key_file);

    debug_printf("VALUES LOADED\n");

    return FALSE;
}

/*
 * Free all values.
 */
void values_destroy()
{
    action_destroy(values.action_shutdown);
    action_destroy(values.action_restart);
    action_destroy(values.action_logout);

    if(values.command_before_action != NULL)
        g_free(values.command_before_action);

    if(values.time_when_notify.nb != NULL)
        g_free(values.time_when_notify.nb);

    if(values.time_when_notify.time != NULL)
        g_free(values.time_when_notify.time);

    memset(&values, 0, sizeof(struct t_values));
}

/* 
 * Init all values.
 */
void values_init()
{
    memset(&values, 0, sizeof(struct t_values));

    values.show_notifications = TRUE;
    values.systray_icon = SYSTRAY_ICON_ALWAYS;

    /* action allocation */
    values.action_shutdown = action_new();
    action_set_type(values.action_shutdown, ACTION_TYPE_SHUTDOWN);

    values.action_restart = action_new();
    action_set_type(values.action_restart, ACTION_TYPE_RESTART);

    values.action_logout = action_new();
    action_set_type(values.action_logout, ACTION_TYPE_LOGOUT);

    /* action variables */
    values.current_action = values.action_shutdown; /* default action */

    values.action_scheduled = FALSE;
    values.action_method = ACTION_METHOD_AUTODETECT;

    /* command */
    values_set_command_before_action("");
    values.command_before_action_enabled = FALSE;

    /* Time when notify */
    values.time_when_notify.nb = g_malloc(sizeof(gsize));
    *values.time_when_notify.nb = 6;

    values.time_when_notify.time = g_malloc(6 * sizeof(gint));
    values.time_when_notify.time[0] = 60;
    values.time_when_notify.time[1] = 120;
    values.time_when_notify.time[2] = 300;
    values.time_when_notify.time[3] = 900;
    values.time_when_notify.time[4] = 1800;
    values.time_when_notify.time[5] = 3600;
}

/*
 * Return the path of the configuration file. "" is returned
 * if it's not found.
 *
 * Warning: The returned string must be freed with g_free().
 */
gchar *get_configuration_path()
{
    const gchar *home = g_get_home_dir();

    if(home == NULL)
        return g_strdup("");

    return g_build_path(G_DIR_SEPARATOR_S, home, ".gshutdown", NULL);
}

/*
 * Load the configuration file.
 *
 * Return NULL on error.
 *
 * Warning: You must be free the returned pointer
 *          with g_key_file_free().
 */
static GKeyFile *load_configuration()
{
    gchar *path;
    GKeyFile *config;
    gboolean result;

    /* Load */
    path = get_configuration_path();
    if((*path) == 0) {
        g_free(path);
        return NULL;
    }

    config = g_key_file_new();
    result = g_key_file_load_from_file(config, path, 
                                       G_KEY_FILE_NONE, NULL);

    /* Error */
    if(result == FALSE) {
        g_key_file_free(config);
        config = NULL;
    }

    /* Result */
    g_free(path);

    return config;
}

/*
 * Save the content of "key_file" in ~/.gshutdown.
 */ 
static gboolean save_configuration(GKeyFile *key_file)
{
    gchar *path;
    FILE *handle;
    gboolean result = FALSE;

    /* Save the values */
    path = get_configuration_path();
    handle = fopen(path, "w");

    if(handle) {
        gchar **keys_list = NULL;
        gsize keys_list_size = 0;

        fprintf(handle, "[%s]\n", GROUP_NAME);

        if(key_file != NULL)
            keys_list = g_key_file_get_keys(key_file, GROUP_NAME, 
                                            &keys_list_size, NULL);

        /* Save all keys */
        if(keys_list != NULL) {
            gsize i;

            /* Write the variables */
            for(i = 0; i < keys_list_size; i++) {
                gchar *value = g_key_file_get_string(key_file, GROUP_NAME,
                                                     keys_list[i], NULL);
                if(value == NULL)
                    continue;

                fprintf(handle, "%s=%s\n", keys_list[i], value);

                g_free(value);
            }

            g_strfreev(keys_list);
        }

        fclose(handle);
    }
    else
        result = TRUE; /* Error fopen */

    /* Free */
    g_free(path);

    return result;
}

/*
 * Return or modify the command before action.
 */
const gchar *values_get_command_before_action()
{
    return (values.command_before_action != NULL) ? values.command_before_action : "";
}

void values_set_command_before_action(const gchar *command)
{
    if(values.command_before_action != NULL)
        g_free(values.command_before_action);

    values.command_before_action = g_strdup((command != NULL) ?  command : "");
}

/*
 * Add a notification date (sorted)
 */
gsize values_add_time_when_notify(gsize time)
{
   gsize cur = 0;
   gsize i;
   gsize *buffer;

   buffer = g_malloc(*values.time_when_notify.nb * sizeof(gint));
   for(i = 0; i < *values.time_when_notify.nb; i++)
      buffer[i] = values.time_when_notify.time[i];

   g_free(values.time_when_notify.time);
   values.time_when_notify.time = g_malloc((*values.time_when_notify.nb + 1) * sizeof(gint));

   if(*(buffer + *values.time_when_notify.nb - 1) < time)
      cur = *values.time_when_notify.nb;
   else
      while(*(buffer + cur) < time)
        cur++;

   *values.time_when_notify.nb += 1;

   for(i = 0; i < cur; i++)
      values.time_when_notify.time[i] = buffer[i];

   *(values.time_when_notify.time + cur) = time;

   for(i = cur + 1; i < *values.time_when_notify.nb; i++)
      values.time_when_notify.time[i] = buffer[i - 1];

   return cur;
}

/*
 * Delete a notification date
 */
void value_del_time_when_notify(gint cur)
{
   gsize i;
   gsize *buffer;

   buffer = g_malloc(*values.time_when_notify.nb * sizeof(gint));

   for(i = 0; i < *values.time_when_notify.nb; i++)
      buffer[i] = values.time_when_notify.time[i];

   g_free(values.time_when_notify.time);
   values.time_when_notify.time = g_malloc((*values.time_when_notify.nb - 1) * sizeof(gint));

   *values.time_when_notify.nb -= 1;

   for(i = 0; i < cur; i++)
      values.time_when_notify.time[i] = buffer[i];

   for(i = cur; i < *values.time_when_notify.nb; i++)
      values.time_when_notify.time[i] = buffer[i + 1];
}

/* vim:cindent:et:sw=4:ts=4:sts=4:tw=78:fenc=utf-8
 */
