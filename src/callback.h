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

#ifndef __GSHUTDOWN_CALLBACK__
#define __GSHUTDOWN_CALLBACK__

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <libnotify/notify.h>
#include "callback_main_window.h"


void on_mnu_tray_cancel(GtkWidget *, gpointer);

/* misc */
void on_about_response(GtkWidget *widget, gint id, gpointer data);
void on_check_show_notifications(GtkWidget *widget, gpointer data);
void on_check_iconify(GtkWidget *widget, gpointer data);
void on_radio_systray_always(GtkWidget *widget, gpointer data);
void on_radio_systray_if_active(GtkWidget *widget, gpointer data);
void on_radio_systray_never(GtkWidget *widget, gpointer data);
void on_btn_config_action(GtkWidget *widget, gpointer data);
gboolean on_config_dialog_delete_event(GtkWidget *widget, GdkEvent *event, gpointer data);
void on_btn_cfg_close(GtkWidget *widget, gpointer data);
gboolean on_config_action_dialog_delete_event(GtkWidget *widget, GdkEvent *event, gpointer data);
void on_btn_config_action_close(GtkWidget *widget, gpointer data);
void on_check_command_before_action(GtkWidget *widget, gpointer data);
void on_radio_action_method(GtkWidget *widget, gpointer data);
void on_btn_action_method_manual(GtkWidget *widget, gpointer data);

void on_combo_method_manual_desktop(GtkWidget *widget, gpointer data);
void on_combo_method_manual_display_manager(GtkWidget *widget, gpointer data);
void on_btn_method_manual_close(GtkWidget *widget, gpointer data);
void on_entry_command_complete(GtkWidget *widget, gpointer data);
void on_notify_click(NotifyNotification *n, const char *action);
void on_btn_check_system_close(GtkWidget *widget, gpointer data);
void on_btn_check_system(GtkWidget *widget, gpointer data);
void on_tray_icon(GtkWidget *button, GdkEventButton *event, GtkWidget *window);
void on_shutdown_now(GtkWidget *widget, gpointer data);
void on_reboot_now(GtkWidget *widget, gpointer data);
void on_logout_now(GtkWidget *widget, gpointer data);
void on_btn_test_notifications(GtkWidget *widget, gpointer data);
void on_btn_notification_date(GtkWidget *widget, gpointer data);
void on_btn_notification_date_close(GtkWidget *widget, gpointer data);
void on_btn_notification_date_add(GtkWidget *widget, gpointer data);
void on_btn_notification_date_remove(GtkWidget *widget, gpointer data);

void on_progress_cancel(GtkWidget *, gpointer);
void on_progress_execute(GtkWidget *, gpointer);
void on_progress_destroy(GtkWidget *, gpointer);

#endif

/* vim:cindent:et:sw=4:ts=4:sts=4:tw=78:fenc=utf-8
 */
