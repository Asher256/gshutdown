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

#ifndef __GSHUTDOWN_GUI__
#define __GSHUTDOWN_GUI__

#include <gtk/gtk.h>
#include <glade/glade.h>
#include "values.h"

/* Variables {{{1 */

extern GladeXML *glade_xml;

/* main window */
extern GtkWidget *vbox_select_time;
extern GtkSpinButton *spin_hour;
extern GtkSpinButton *spin_minute;
extern GtkCalendar *calendar;
extern GtkComboBox *combo_action;
extern GtkButton *btn_actual_time;
extern GtkComboBox *combo_extras;
extern GtkButton *btn_start;

/* another */
extern GtkWidget *about_dialog;
extern GtkWidget *combo_method_manual_desktop;
extern GtkWidget *combo_method_manual_display_manager;
extern GtkWidget *radio_action_method_autodetect;
extern GtkWidget *radio_action_method_manual;
extern GtkWidget *radio_action_method_custom_command;
extern GtkWidget *check_command_before_action;
extern GtkWidget *method_manual_dialog;
extern GtkWidget *btn_action_method_manual;
extern GtkButton *btn_test_notifications;
extern GtkWidget *main_window;
extern GtkComboBox *combo_time_type;
extern GtkWidget *mnu_tray_cancel;
extern GtkTooltips *tray_tooltips;
extern GtkCheckButton *check_show_notifications;
extern GtkWidget *radio_systray_always;
extern GtkWidget *radio_systray_if_active;
extern GtkWidget *radio_systray_never;
extern GtkDialog *config_action_dialog;
extern GtkWidget *tray_eventbox;
extern GtkWidget *entry_action_custom_command;
extern GtkWidget *btn_config_shutdown;
extern GtkWidget *btn_config_restart;
extern GtkWidget *btn_config_logout;
extern GtkWidget *label_action_description;
extern GtkWidget *check_iconify;
extern GtkWidget *config_dialog;
extern GtkWidget *btn_check_system;
extern GtkWidget *tray_image;
extern GtkWidget *entry_command_before_action;
extern GtkWidget *notification_date_dialog;
extern GtkComboBox *notification_date_combo;
extern GtkButton *notification_date_add;
extern GtkButton *notification_date_remove;
extern GtkSpinButton *spin_notification_hour, *spin_notification_minute, *spin_notification_second; 

/* }}}1 */

/* Prototypes {{{1 */

gboolean confirm_dialog(Action *id_action);
gboolean gui_init(int argc, char **argv);
gboolean gui_connect_all();
gboolean gui_systray_icon();
void gui_set_actual_time(gboolean, gboolean);
void gui_set_time(guint id);
GladeXML *gui_load_glade(gchar *file);
GtkMenu* traymenu();
GtkWidget* create_mnu_tray_action_now (void);
void gui_set_time(guint id);
void gui_systray_icon_hide();
void gui_systray_icon_show();
gboolean gui_systray_icon_is_showed();
gboolean timer_day();
gboolean gui_mark_today();
gboolean detect_error_dialog();
gboolean handle_arguments(int argc, char **argv);
void gui_remove_notification_date();
void gui_set_notification_date();

/* }}}1 */

typedef enum {
    NOTIFICATION_TYPE_IMPORTANT = 0,
    NOTIFICATION_TYPE_FAST_INFO = 1 
} NotificationDelay;

typedef enum {
    NOTIFICATION_ICON_NONE = 0,
    NOTIFICATION_ICON_TIME = 1 
} NotificationIcon;

void gui_notification(gchar *message, NotificationDelay type_delay, NotificationIcon show_icon);

#endif

/* vim:cindent:et:sw=4:ts=4:sts=4:tw=78:fenc=utf-8:foldmethod=marker
 */
