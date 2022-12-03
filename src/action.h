/*
 * This file is part of gshutdown.
 *
 * Copyright (c) GShutdown Team <https://github.com/Asher256/gshutdown/issues>
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

#ifndef __GSHUTDOWN_ACTION__
#define __GSHUTDOWN_ACTION__

#include <glib.h>

typedef enum {
    ACTION_TYPE_NONE = 0,
    ACTION_TYPE_SHUTDOWN = 1,
    ACTION_TYPE_RESTART = 2,
    ACTION_TYPE_LOGOUT = 3,
    ACTION_TYPE_COMMAND = 4
} ActionType;

/* Structure for one action */
typedef struct {
    ActionType type;
    gchar *command; /* command launched if type == ACTION_TYPE_TYPE */
} Action;

typedef enum {
    ACTION_METHOD_AUTODETECT = 0,
    ACTION_METHOD_CUSTOM_COMMAND = 1,
    ACTION_METHOD_MANUAL = 2 /* choose manually if it's KDE, GNOME, 
                                XFCE, GDM, KDM... */
} ActionMethod;

/* Constructor/Destructor */
Action *action_new();
void action_destroy(Action *action);

/* Launch the default action */
gboolean action_run(Action *action);

/* Read/Write the default action */
ActionType action_get_type(Action *action);
void action_set_type(Action *action, ActionType type);

/* Read/write the custom command */
void action_set_command(Action *action, const gchar *command);
const gchar *action_get_command(Action *action);

#endif

/* vim:cindent:et:sw=4:ts=4:sts=4:tw=78:fenc=utf-8
*/
