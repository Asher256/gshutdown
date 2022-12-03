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

#include "main.h"
#include "gui.h"
#include "callback.h"
#include "misc.h"
#include "timer.h"
#include "pconfirm.h"
#include <libnotify/notify.h>

/* About {{{1 */

void on_about_dialog_response(GtkWidget *widget, gint id, gpointer data) 
{
    if(id != GTK_RESPONSE_NONE) 
        gtk_widget_hide(widget);
}

/* }}}1 */

/* Config Action (custom command) {{{1 */

/* Action Variables  */ 
static Action *config_action_current_action = NULL;

/* Called when the config action dialog is closed, to save all informations */
gboolean on_config_action_dialog_delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    const gchar *command;

    gtk_widget_hide(GTK_WIDGET(config_action_dialog));

    /* Apply the action in "values" */
    command = gtk_entry_get_text(GTK_ENTRY(entry_action_custom_command));
    if(config_action_current_action != NULL)
        action_set_command(config_action_current_action, command);

    /* save */
    values_save();

    return gtk_widget_hide_on_delete(widget);
}

void on_btn_config_action_close(GtkWidget *widget, gpointer data)
{
    on_config_action_dialog_delete_event(GTK_WIDGET(config_action_dialog), NULL, NULL);
}

void on_btn_config_action(GtkWidget *widget, gpointer data)
{
    /* It must not be visible */
    if(GTK_WIDGET_VISIBLE(GTK_WIDGET(config_action_dialog)))
        return;

    /* Get the actual action information */
    if(widget == btn_config_shutdown)
        config_action_current_action = values.action_shutdown;

    else if(widget == btn_config_restart)
        config_action_current_action = values.action_restart;

    else if(widget == btn_config_logout) 
        config_action_current_action = values.action_logout;

    else
        return;

    gtk_entry_set_text(GTK_ENTRY(entry_action_custom_command), 
                       action_get_command(config_action_current_action));

    gtk_widget_grab_focus(entry_action_custom_command);

    /* Show the dialog */
    gtk_widget_show(GTK_WIDGET(config_action_dialog));
}

/* }}}1 */

/* Config Action (method) {{{1 */

void on_combo_method_manual_display_manager(GtkWidget *widget, gpointer data)
{
    switch(gtk_combo_box_get_active(GTK_COMBO_BOX(widget))) {
        case 1:  /* GDM */
            values.selected_display_manager = DISPLAY_MANAGER_GDM;
            break;

        case 2:  /* KDM */
            values.selected_display_manager = DISPLAY_MANAGER_KDM;
            break;

        default: /* Auto */
            values.selected_display_manager = DISPLAY_MANAGER_AUTO;
    }
}

void on_combo_method_manual_desktop(GtkWidget *widget, gpointer data)
{
    switch(gtk_combo_box_get_active(GTK_COMBO_BOX(widget))) {
        case 1:  /* GNOME */
            values.selected_desktop = DESKTOP_GNOME;
            break;

        case 2:  /* KDE */
            values.selected_desktop = DESKTOP_KDE;
            break;

        case 3:  /* XFCE */
            values.selected_desktop = DESKTOP_XFCE;
            break;

        default: /* Auto */
            values.selected_desktop = DESKTOP_AUTO;
    }
}

void on_btn_method_manual_close(GtkWidget *widget, gpointer data)
{
    gtk_widget_hide(method_manual_dialog);
}

/* Method in general config dialog {{{2 */

void on_btn_action_method_manual(GtkWidget *widget, gpointer data)
{
    gtk_widget_show(method_manual_dialog);
}

void on_radio_action_method(GtkWidget *widget, gpointer data)
{
    /* Diable all */
    gtk_widget_set_sensitive(btn_check_system, FALSE);
    gtk_widget_set_sensitive(btn_action_method_manual, FALSE);
    gtk_widget_set_sensitive(btn_config_shutdown, FALSE);
    gtk_widget_set_sensitive(btn_config_restart, FALSE);
    gtk_widget_set_sensitive(btn_config_logout, FALSE);

    /* Enable the required widgets */
    if(widget == radio_action_method_autodetect) {
        values.action_method = ACTION_METHOD_AUTODETECT;
        gtk_widget_set_sensitive(btn_check_system, TRUE);
    }
    else if(widget == radio_action_method_manual) {
        values.action_method = ACTION_METHOD_MANUAL;
        gtk_widget_set_sensitive(btn_action_method_manual, TRUE);
    }
    else {
        values.action_method = ACTION_METHOD_CUSTOM_COMMAND;
        gtk_widget_set_sensitive(btn_config_shutdown, TRUE);
        gtk_widget_set_sensitive(btn_config_restart, TRUE);
        gtk_widget_set_sensitive(btn_config_logout, TRUE);
    }
}

/* }}}2 */

/* }}}1 */

/* Config Action (command before action) {{{1 */

void on_check_command_before_action(GtkWidget *widget, gpointer data)
{
    gboolean enabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    gtk_widget_set_sensitive(entry_command_before_action, enabled);
    gtk_widget_grab_focus(entry_command_before_action);

    values.command_before_action_enabled = enabled;
}

/* }}}1 */

/* General configuration {{{1 */

/* 
 * NB: It's update values.
 *
 * Show/Hide notifications
 *
 * */
void on_check_show_notifications(GtkWidget *widget, gpointer data)
{
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) == TRUE) {
        gtk_widget_set_sensitive(GTK_WIDGET(btn_test_notifications), TRUE);
        values.show_notifications = TRUE;
    }
    else {
        gtk_widget_set_sensitive(GTK_WIDGET(btn_test_notifications), FALSE);
        values.show_notifications = FALSE;
    }
}

void on_btn_test_notifications(GtkWidget *widget, gpointer data)
{
    gui_notification(_("Notification test"), NOTIFICATION_TYPE_IMPORTANT, NOTIFICATION_ICON_TIME);
}

void on_btn_notification_date(GtkWidget *widget, gpointer data)
{
    gui_set_notification_date();
    gtk_widget_show(GTK_WIDGET(notification_date_combo));
    gtk_widget_show(GTK_WIDGET(notification_date_dialog));
}

void on_btn_notification_date_close(GtkWidget *widget, gpointer data)
{
    gtk_widget_hide(GTK_WIDGET(data));
}

void on_btn_notification_date_add(GtkWidget *widget, gpointer data)
{
    gchar text[100];
    guint seconds = 0;
    gint minutes = 0;
    gint hours = 0;
    guint time_sec;
    guint i;

    seconds = gtk_spin_button_get_value(spin_notification_second);
    minutes = gtk_spin_button_get_value(spin_notification_minute);
    hours = gtk_spin_button_get_value(spin_notification_hour);

    time_sec = (3600 * hours) + (60 * minutes) + seconds;

    for(i = 0; i < *values.time_when_notify.nb; i++)
       if(values.time_when_notify.time[i] == time_sec)
          return;

    sprintf(text, " ");

    if(hours > 0) {
        sprintf(text, N_("%s%u hour", "%s%u hours", hours),
                " ", hours);
    }

    if(minutes > 0) {
        sprintf(text, 
                N_("%s%u minute", "%s%u minutes", minutes),
                " ", minutes);
    }

    if(seconds > 0) {
        sprintf(text, 
                N_("%s%u second", "%s%u seconds", seconds),
                " ", seconds);
    }

    gtk_combo_box_insert_text(GTK_COMBO_BOX(notification_date_combo), values_add_time_when_notify(time_sec), text);
}

void on_btn_notification_date_remove(GtkWidget *widget, gpointer data)
{
    gint n_active;

    n_active = gtk_combo_box_get_active(GTK_COMBO_BOX(notification_date_combo));

    if(n_active != -1) {
       gtk_combo_box_remove_text(GTK_COMBO_BOX(notification_date_combo), n_active);
       value_del_time_when_notify(n_active);
    }
}

/*
 * config dialog.
 */
gboolean on_config_dialog_delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry_command_before_action));
    values_set_command_before_action(text);
    values_save();
    return gtk_widget_hide_on_delete(widget);
}

void on_btn_cfg_close(GtkWidget *widget, gpointer data)
{
    on_config_dialog_delete_event(config_dialog, NULL, NULL);
}

/* Complete a command */
void on_entry_command_complete(GtkWidget *widget, gpointer data)
{
    gchar *stock_id = NULL;

    if(command_exists(gtk_entry_get_text(GTK_ENTRY(widget))) == TRUE)
        stock_id = GTK_STOCK_APPLY;
    else
        stock_id = GTK_STOCK_CANCEL;
    
    gtk_image_set_from_stock(GTK_IMAGE(data), stock_id,  
                             GTK_ICON_SIZE_BUTTON); 
}

/*
 * Called when user clicked on the notify
 */

void on_notify_click(NotifyNotification *n, const char *action)
{
    gtk_widget_show(main_window);
}

/*
 * Check system dialog.
 */
void on_btn_check_system(GtkWidget *widget, gpointer data)
{
    gtk_widget_show(GTK_WIDGET(data));
}

void on_btn_check_system_close(GtkWidget *widget, gpointer data)
{
    gtk_widget_hide(GTK_WIDGET(data));
}

/*
 * Radio button to show/hide the systray icon.
 */
void on_radio_systray_always(GtkWidget *widget, gpointer data)
{
    values.systray_icon = SYSTRAY_ICON_ALWAYS;
    gtk_widget_set_sensitive(check_iconify, TRUE);
    gui_systray_icon_show();
}

void on_radio_systray_if_active(GtkWidget *widget, gpointer data)
{
    values.systray_icon = SYSTRAY_ICON_IF_ACTIVE;
    gtk_widget_set_sensitive(check_iconify, TRUE);

    if(values.action_scheduled)
        gui_systray_icon_show();
    else
        gui_systray_icon_hide();
}

void on_radio_systray_never(GtkWidget *widget, gpointer data)
{
    values.systray_icon = SYSTRAY_ICON_NEVER;
    gtk_widget_set_sensitive(check_iconify, FALSE);
    gui_systray_icon_hide();
}

void on_check_iconify(GtkWidget *widget, gpointer data)
{
    values.iconify = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

/* }}}1 */

/* Menu {{{1 */

/*
 * "Cancel" in the contextual menu.
 */
void on_mnu_tray_cancel(GtkWidget *widget, gpointer data)
{
    if(values.action_scheduled)
        on_btn_start_clicked(btn_start, NULL);
}

/* }}}1 */

/* Systray {{{1 */

void on_shutdown_now(GtkWidget *widget, gpointer data)
{
    if(confirm_dialog(values.action_shutdown) == TRUE) {
        gui_notification(_("Computer is going to shutdown now !"), 
                         NOTIFICATION_TYPE_IMPORTANT, 
                         NOTIFICATION_ICON_TIME);
        action_run(values.action_shutdown);
    }
}

void on_reboot_now(GtkWidget *widget, gpointer data)
{
    if(confirm_dialog(values.action_restart) == TRUE) {
        gui_notification(_("Computer is going to reboot now !"), 
                         NOTIFICATION_TYPE_IMPORTANT, 
                         NOTIFICATION_ICON_TIME);
        action_run(values.action_restart);
    }
}

void on_logout_now(GtkWidget *widget, gpointer data)
{
    if(confirm_dialog(values.action_logout) == TRUE) {
        gui_notification(_("Current session will be terminated !"), 
                         NOTIFICATION_TYPE_IMPORTANT, 
                         NOTIFICATION_ICON_TIME);
        action_run(values.action_logout);
    }
}

void on_tray_icon(GtkWidget *button, GdkEventButton *event, GtkWidget *window)
{
    switch(event->button) {
        case 1:
            if(!GTK_WIDGET_VISIBLE(window))
                gtk_window_present(GTK_WINDOW(window));
            else
                gtk_widget_hide(window);
            break;

        case 2:
            gtk_menu_popup(GTK_MENU(create_mnu_tray_action_now()), 
                           NULL, NULL, NULL, NULL, 
                           event->button, event->time);
            break;

        case 3:
            gtk_menu_popup(traymenu(), NULL, NULL, NULL, NULL, 
                           event->button, event->time);
            break;
    }
}

/* }}}1 */

/* Progress confirm {{{1 */

void on_progress_cancel(GtkWidget *widget, gpointer data)
{
    gtk_widget_destroy(GTK_WIDGET(data));
    on_btn_start_clicked(btn_start, NULL);
}

void on_progress_execute(GtkWidget *widget, gpointer data)
{
    gtk_widget_destroy(GTK_WIDGET(data));   
    action_run(values.current_action);
    on_btn_start_clicked(btn_start, NULL);
}

void on_progress_destroy(GtkWidget *widget, gpointer data)
{
    guint *func_ref;
    func_ref = data;

    g_source_remove(*func_ref);
    g_free(func_ref);

    progress_confirm_open = FALSE;
}

/* }}}1 */

/* vim:cindent:et:sw=4:ts=4:sts=4:tw=78:fenc=utf-8:foldmethod=marker
 */
