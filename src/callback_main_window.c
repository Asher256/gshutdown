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

#ifndef __GSHUTDOWN_CALLBACK_MAIN_WINDOW__
#define __GSHUTDOWN_CALLBACK_MAIN_WINDOW__

#include "main.h"
#include "misc.h"
#include "timer.h"
#include "callback.h"
#include "values.h"
#include "gui.h"
#include "pconfirm.h"

/* 
 * Button : actual time.
 */
void on_btn_actual_time_clicked(GtkButton *button, gpointer data)
{
    gui_set_actual_time(TRUE, TRUE);
}

/*
 * Start button (launch / stop button).
 */
void on_btn_start_clicked(GtkButton *button, gpointer data)
{
    static guint func_ref;
    GtkWidget *dialog;
    guint timeleft;
    gchar *path;

    time_t t = time(0);
    struct tm *tm_time = localtime(&t);

    /* Launch the action */
    if(values.action_scheduled == FALSE) {
        /* Test if the detection is ok */
        if(detect_error_dialog())
           return;

        /* Save sec */
        values.time_from_now.sec = tm_time->tm_sec;
        timeleft = time_left();

        /* Launch ! */
        if(timeleft > 30) {
           if(values.systray_icon != SYSTRAY_ICON_NEVER) {
               gui_systray_icon_show();
   
               /* Iconify with a notification ? */
               if(values.iconify && values.systray_icon != SYSTRAY_ICON_NEVER) { 
                   gtk_widget_hide(main_window);
                   gui_notification(_("I'm here !\n\nMove over me to see the countdown."), 
                                    NOTIFICATION_TYPE_FAST_INFO, NOTIFICATION_ICON_NONE);
               }
           }

           values.action_scheduled = TRUE;

           /* Change systray image icon */
           path = data_path("tray_icon2.png");
           gtk_image_set_from_file(GTK_IMAGE(tray_image), path);

           /* Time type widgets */
           gtk_widget_set_sensitive(vbox_select_time, 0);
           gtk_widget_set_sensitive(GTK_WIDGET(combo_action), 0);
           gtk_widget_set_sensitive(GTK_WIDGET(combo_time_type), 0);
           gtk_button_set_label(button, _("_Stop"));    

           /* Timer : 1s*/
           func_ref = g_timeout_add(1000, timer_notify, NULL);
        }
        else {
           if(timeleft > 0 || values.time_choice == TIME_CHOICE_NOW) {
              if(confirm_dialog(values.current_action) == TRUE)
                 action_run(values.current_action);

           } else {
              dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
                                     GTK_DIALOG_DESTROY_WITH_PARENT,
                                     GTK_MESSAGE_ERROR,
                                     GTK_BUTTONS_CLOSE,
                                     _("Chosen time already exceeded"));

              gtk_dialog_run(GTK_DIALOG(dialog));
              gtk_widget_destroy(dialog);
           }
        }
    }
    /* Stop the action */
    else {
        if(values.systray_icon != SYSTRAY_ICON_ALWAYS)
            gui_systray_icon_hide();

        if(values.iconify == TRUE && values.systray_icon != SYSTRAY_ICON_NEVER)
            gtk_widget_show(main_window);

        if(progress_confirm_open)
           progress_destroy();

        values.action_scheduled = FALSE;

        /* Change systray image icon */
        path = data_path("tray_icon.png");
        gtk_image_set_from_file(GTK_IMAGE(tray_image), path);


        /* Remove timer */
        g_source_remove(func_ref);

        /* Time Type widgets */
        on_combo_time_type_changed(combo_time_type, NULL);

        /* Other */
        gtk_widget_set_sensitive(vbox_select_time, 1);
        gtk_widget_set_sensitive(GTK_WIDGET(combo_action), 1);
        gtk_widget_set_sensitive(GTK_WIDGET(combo_time_type), 1);
        gtk_button_set_label(button, _("_Start"));    

        /* Change tray tooltip */
        gtk_tooltips_set_tip(tray_tooltips, tray_eventbox, _("No action scheduled"), NULL);
    }
}

/* 
 * It modify the "values".
 */
void on_combo_action_changed(GtkWidget *widget, gpointer data)
{
    gint current = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));

    switch(current) {
        case 0: /* Shutdown */
            gtk_widget_hide(GTK_WIDGET(combo_extras));
            values.current_action = values.action_shutdown;
            break;

        case 1: /* Restart */
            gtk_widget_hide(GTK_WIDGET(combo_extras));
            values.current_action = values.action_restart;
            break;

        case 2: /* Logout */
            gtk_widget_hide(GTK_WIDGET(combo_extras));
            values.current_action = values.action_logout;
            break;

        case 3: /* Extras */
            gtk_widget_show(GTK_WIDGET(combo_extras));
            break;
    }
}

/* 
 * If the type of the time is changed :
 *     - Hide and show some widgets
 *     - Load and save some values
 */
void on_combo_time_type_changed(GtkComboBox *combo, gpointer data)
{
    /* Hide/Show widgets */
    gtk_widget_set_sensitive(GTK_WIDGET(calendar), 0);
    gtk_widget_set_sensitive(GTK_WIDGET(spin_hour), 0);
    gtk_widget_set_sensitive(GTK_WIDGET(spin_minute), 0);
    gtk_widget_set_sensitive(GTK_WIDGET(btn_actual_time), 0);

    /* Update "values" */
    switch(gtk_combo_box_get_active(combo)) {
        case 0:
            values.time_choice = TIME_CHOICE_TIME_DATE;

            /* Set values */
            gtk_spin_button_set_value(spin_hour, values.time_date.hour);
            gtk_spin_button_set_value(spin_minute, values.time_date.min);

            /* Enable widgets */
            gtk_widget_set_sensitive(GTK_WIDGET(btn_actual_time), 1);
            gtk_widget_set_sensitive(GTK_WIDGET(spin_hour), 1);
            gtk_widget_set_sensitive(GTK_WIDGET(spin_minute), 1);
            gtk_widget_set_sensitive(GTK_WIDGET(calendar), 1);
            break;

        case 1:
            values.time_choice = TIME_CHOICE_TIME_FROM_NOW;

            /* Set values */
            gtk_spin_button_set_value(spin_hour, values.time_from_now.hour);
            gtk_spin_button_set_value(spin_minute, values.time_from_now.min);

            /* Enable widgets */
            gtk_widget_set_sensitive(GTK_WIDGET(spin_hour), 1);
            gtk_widget_set_sensitive(GTK_WIDGET(spin_minute), 1);
            break;

        case 2:
            values.time_choice = TIME_CHOICE_NOW;
            break;
    }
}

/*
 * If a the day is changed in the calendar.
 */
void on_calendar_day_selected(GtkCalendar *calendar, gpointer data)
{
    gtk_calendar_get_date(calendar, &values.time_date.year, 
                                    &values.time_date.month,
                                    &values.time_date.day);
    gui_mark_today();
}

/*
 * Hour changed.
 */
void on_spin_hour_value_changed(GtkSpinButton *spin, GtkScrollType *arg1, gpointer data)
{
    gint current_choice;
    gint day, month, year;

    current_choice = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_time_type));

    if(gtk_spin_button_get_value(spin_hour) == 24) {
        gtk_spin_button_set_value(spin_hour, 0);

        if(current_choice == TIME_CHOICE_TIME_DATE) {
            day = values.time_date.day;
            if(day < nb_days_in_month(values.time_date.month ,values.time_date.year)) {
                day += 1;
                gtk_calendar_select_day(calendar, day);
            } else {
                month = values.time_date.month + 1;
                year = values.time_date.year;
                if(month == 12) {
                    month = 0;
                    year += 1;
                }
                day = 1;
                gtk_calendar_select_day(calendar, day);
                gtk_calendar_select_month(calendar, month,
                        year);
            }

            on_calendar_day_selected(calendar, NULL);
        }
    }
    else if(gtk_spin_button_get_value(spin_hour) == -1) {
        gtk_spin_button_set_value(spin_hour, 23);
        if(current_choice == TIME_CHOICE_TIME_DATE) {
            day = values.time_date.day;
            if(day == 1) {
                month = values.time_date.month - 1;
                year = values.time_date.year;
                if(month == -1) {
                    month = 11;
                    year -= 1;
                }
                day = nb_days_in_month(month, values.time_date.year);
                gtk_calendar_select_month(calendar, month,
                        year);
                gtk_calendar_select_day(calendar, day);
            } else {
                day -= 1;
                gtk_calendar_select_day(calendar, day);
            }

            on_calendar_day_selected(calendar, NULL);
        }
    }

    switch(current_choice) {
        case 0: /* Time and date */
            values.time_date.hour = gtk_spin_button_get_value(spin_hour);
            break;

        case 1: /* Time from now */
            values.time_from_now.hour = gtk_spin_button_get_value(spin_hour);
            break;
    }
}

/*
 * Minute changed.
 */
void on_spin_minute_value_changed(GtkSpinButton *spin, GtkScrollType *arg1, gpointer data)
{
    gint current_choice;

    if(gtk_spin_button_get_value(spin_minute) == 60) {
        gtk_spin_button_set_value(spin_minute, 0);
        gtk_spin_button_set_value(spin_hour, gtk_spin_button_get_value(spin_hour) + 1);
        on_spin_hour_value_changed(spin_hour, NULL, NULL);
    } else if(gtk_spin_button_get_value(spin_minute) == -1) {
        gtk_spin_button_set_value(spin_minute, 59);
        gtk_spin_button_set_value(spin_hour, gtk_spin_button_get_value(spin_hour) - 1);
        on_spin_hour_value_changed(spin_hour, NULL, NULL);
    }

    current_choice = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_time_type));

    switch(current_choice) {
        case 0: /* Time and date */
            values.time_date.min = gtk_spin_button_get_value(spin_minute);
            break;

        case 1: /* Time from now */
            values.time_from_now.min = gtk_spin_button_get_value(spin_minute);
            break;
    }
}

/*
 * Called before closing Window.
 */
gboolean on_main_window_delete_event(GtkWidget *widget, gpointer data)
{
    if(gui_systray_icon_is_showed())
        gtk_widget_hide(widget);
    else
        on_mnu_quit_activate(widget, data);

    return TRUE;
}

/*
 * Quit the program.
 */
void on_mnu_quit_activate(GtkWidget *widget, gpointer data)
{
    GtkWidget *dialog;
    gint response;
    
    dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
                                    GTK_DIALOG_MODAL,
                                    GTK_MESSAGE_QUESTION,
                                    GTK_BUTTONS_YES_NO,
                                    (gchar *)_("Are you sure you want to quit ?"));

    response = gtk_dialog_run(GTK_DIALOG(dialog));
    if(response == GTK_RESPONSE_YES)
        quit(QUIT_SUCCESS); /* atexit called */

    gtk_widget_destroy(GTK_WIDGET(dialog));
}

/*
 * Show about dialog.
 */
void on_mnu_about_activate(GtkWidget *widget, gpointer data)
{
    gtk_widget_show(GTK_WIDGET(about_dialog));
}

/*
 * Show the preference dialog.
 */
void on_mnu_config_activate(GtkComboBox *combo, gpointer data)
{
    gtk_widget_show(config_dialog);
}


#endif

/* vim:cindent:et:sw=4:ts=4:sts=4:tw=78:fenc=utf-8
*/
