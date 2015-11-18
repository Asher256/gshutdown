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

#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include "action.h"
#include "misc.h"
#include "gui.h"
#include "values.h"
#include "gdm.h"

/* Run the action now ! */
static gboolean action_run_command(Action *action);

/*
 * Constructor.
 */
Action *action_new()
{
    Action *ptr = g_malloc(sizeof(Action));
    ptr->type = ACTION_TYPE_NONE;
    ptr->command = NULL;
    return ptr;
}

/* 
 * Destructor.
 */
void action_destroy(Action *action)
{
    if(action->command != NULL)
        g_free(action->command);
    else
        g_free(action);
}

/*
 * Read the action type (see ActionType
 * in action.h)
 */
ActionType action_get_type(Action *action)
{
    return action->type;
}

/*
 * Modity the action type (see ActionType
 * in action.h)
 */
void action_set_type(Action *action, ActionType type)
{
    action->type = type;
}

/*
 * Change the custom command.
 *
 * NB: "command" string is duplicated with g_strdup.
 */
void action_set_command(Action *action, const gchar *command)
{
    if(action->command != NULL)
        g_free(action->command);

    action->command = g_strdup(command);
}

/*
 * Return the custom command.
 *
 * Warning: the returned pointer is internal, DO
 *          NOT FREE IT!
 */
const gchar *action_get_command(Action *action)
{
    return (action->command) ? action->command : "";
}

/*
 * Kill a session by using the PID in the environment
 * variable SESSION_MANAGER.
 */
void kill_session()
{
    const gchar *session_manager = g_getenv("SESSION_MANAGER");
    if(session_manager) {
        gchar **splited = g_strsplit(session_manager, "/", 0);

        if(splited != NULL) {
            guint length = g_strv_length(splited);
            if(length > 0) {
                pid_t pid = atoi(splited[length - 1]);
                if(pid != 0)
                    kill(pid, SIGQUIT);
            }
            g_strfreev(splited);
        }
    }
}

/*
 * Run the actual action.
 *
 * Return TRUE on error.
 */
gboolean action_run(Action *action)
{
    Desktop current_desktop = values.detected_desktop;
    DisplayManager current_display_manager =  values.detected_display_manager;

    /* Command before action */
    if(values.command_before_action_enabled == TRUE)
        run_command(values.command_before_action);

    /* Method = command */
    if(values.action_method == ACTION_METHOD_CUSTOM_COMMAND || 
       action_get_type(action) == ACTION_TYPE_COMMAND) {
        return action_run_command(action);
    }

    /* Method = action selected manually */
    if(values.action_method == ACTION_METHOD_MANUAL) {
        if(values.selected_desktop != DESKTOP_AUTO)
            current_desktop = values.selected_desktop;

        if(values.selected_display_manager != DISPLAY_MANAGER_AUTO)
            current_display_manager = values.selected_display_manager;
    }

    /* Not a root command ? Quit here, it's not useful to talk with the
     * session manager now. */
    if(action_get_type(action) == ACTION_TYPE_SHUTDOWN ||
        action_get_type(action) == ACTION_TYPE_RESTART) {
        
        /* WARNING: After this line, its important to use
         * current_display_manager and current_desktop */
        
        if(current_desktop == DESKTOP_NONE || 
           current_display_manager == DISPLAY_MANAGER_NONE)
            return TRUE;
        
        /* GDM */
        if(current_display_manager == DISPLAY_MANAGER_GDM) {
            char action_str[256];
            char command[30];

            if(action_get_type(action) == ACTION_TYPE_SHUTDOWN)
                snprintf(command, 29, "SET_LOGOUT_ACTION HALT");
            else if(action_get_type(action) == ACTION_TYPE_RESTART)
                snprintf(command, 29, "SET_LOGOUT_ACTION REBOOT");
            else
                command[0] == 0;
            
            if(*command) {
                snprintf(action_str, 255, "gdmflexiserver -a --command=\"%s\"", command);
                run_command(action_str);
            }
            
        /* TODO code remplacé en attendant une solution !
            GdmLogoutAction logout_action = GDM_LOGOUT_ACTION_NONE;
            
            if(action_get_type(action) == ACTION_TYPE_SHUTDOWN)
                logout_action = GDM_LOGOUT_ACTION_SHUTDOWN;
            else if(action_get_type(action) == ACTION_TYPE_RESTART)
                logout_action = GDM_LOGOUT_ACTION_REBOOT;
            
            gdm_set_logout_action(logout_action);
        */
        } 
        
        /* KDM */
        else if(current_display_manager == DISPLAY_MANAGER_KDM) {
            if(action_get_type(action) == ACTION_TYPE_SHUTDOWN)
                run_command("kdmctl shutdown halt forcenow");
            else if(action_get_type(action) == ACTION_TYPE_RESTART)
                run_command("kdmctl shutdown reboot forcenow");
        }
    }
    
    /* Desktop */
    
    /* GNOME */
    if(current_desktop == DESKTOP_GNOME) 
        /*TODO remplacer gnome-session-save par dbus */
        kill_session();
    
    /* KDE */
    else if(current_desktop == DESKTOP_KDE)
        return run_command("kdeinit_shutdown");
    /* XFCE */
    else if(current_desktop == DESKTOP_XFCE)
        kill_session();
    
    return TRUE; /* Error */
}

/*
 * Run the custom command (action->command).
 *
 * Return TRUE on error.
 */
static gboolean action_run_command(Action *action)
{
    if(action->command == NULL) 
        return TRUE;

    return run_command(action->command);
}

/* vim:cindent:et:sw=4:ts=4:sts=4:tw=78:fenc=utf-8
 */
