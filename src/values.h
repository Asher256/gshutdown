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

#ifndef __GSHUTDOWN_VALUES__
#define __GSHUTDOWN_VALUES__

#include <glib.h>
#include "action.h"

typedef enum {
    DISPLAY_MANAGER_NONE = 0,
    DISPLAY_MANAGER_AUTO = 0,
    DISPLAY_MANAGER_GDM = 1,
    DISPLAY_MANAGER_KDM = 2
} DisplayManager;

typedef enum {
    DESKTOP_NONE = 0,
    DESKTOP_AUTO = 0,
    DESKTOP_GNOME = 1,
    DESKTOP_KDE = 2,
    DESKTOP_XFCE = 3
} Desktop;

typedef enum {
    TIME_CHOICE_TIME_DATE = 0,
    TIME_CHOICE_TIME_FROM_NOW = 1,
    TIME_CHOICE_NOW = 2
} TimeChoice;

typedef enum {
    SYSTRAY_ICON_ALWAYS = 0,
    SYSTRAY_ICON_IF_ACTIVE = 1,
    SYSTRAY_ICON_NEVER = 2
} SystrayIcon;

struct t_values
{
    /* values swapped with the interface */
    struct {
        guint day, month, year;
        guint hour, min;
    } time_date;

    struct {
        guint hour, min, sec;
    } time_from_now;

    TimeChoice time_choice;

    DisplayManager detected_display_manager;
    Desktop detected_desktop;

    ActionMethod action_method;
    gboolean action_scheduled;

    /* The configuration values saved in ~/.gshutdown */
    gboolean show_notifications;
    SystrayIcon systray_icon;

    Action *action_shutdown;
    Action *action_restart;
    Action *action_logout;
    Action *current_action; /* = ptr of action_shutdown, action_restart... */

    gboolean iconify; /* Iconify when the action is started */

    gboolean command_before_action_enabled;
    gchar *command_before_action; /* New allocated string, default=g_strdup("") */

    /* selection in manual action method */
    DisplayManager selected_display_manager;
    Desktop selected_desktop;

    struct {
       gsize *nb;
       gint *time;
    } time_when_notify;
};

/* All global variables */
extern struct t_values values;
extern gboolean action_scheduled;

/* Prototypes */
void values_init();
void values_destroy();

gboolean values_save();
gboolean values_load();

gchar *get_configuration_path();

const gchar *values_get_command_before_action();
void values_set_command_before_action(const gchar *command);

void values_action_method_autodetect();

gsize values_add_time_when_notify(gsize time);
void value_del_time_when_notify(gint cur);

#endif

/* vim:cindent:et:sw=4:ts=4:sts=4:tw=78:fenc=utf-8
 */
