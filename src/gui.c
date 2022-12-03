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

#define DISABLE_TRAY_ICON

#include "main.h"
#include "misc.h"
#include "gui.h"
#include "callback.h"
#ifndef DISABLE_TRAY_ICON
#include "eggtrayicon.h"
#endif

#ifdef LIBNOTIFY_ENABLED
#include <libnotify/notify.h>
#endif

/* Variables {{{1 */

/* Default values (in the arguments) */
ActionType default_action_type = ACTION_TYPE_SHUTDOWN;

/* the xml data */
GladeXML *glade_xml = NULL;

/* Main Window */
GtkWidget *vbox_select_time = NULL;
GtkSpinButton *spin_hour = NULL;
GtkSpinButton *spin_minute = NULL;
GtkCalendar *calendar = NULL;
GtkComboBox *combo_action = NULL;
GtkButton *btn_actual_time = NULL;
GtkComboBox *combo_extras = NULL;
GtkButton *btn_start = NULL;

/* Another widgets */
GtkWidget *about_dialog = NULL;
GtkWidget *combo_method_manual_desktop = NULL;
GtkWidget *combo_method_manual_display_manager = NULL;
GtkWidget *method_manual_dialog = NULL;
GtkWidget *radio_action_method_autodetect = NULL;
GtkWidget *radio_action_method_manual = NULL;
GtkWidget *radio_action_method_custom_command = NULL;
GtkWidget *btn_action_method_manual = NULL;
GtkWidget *check_command_before_action = NULL;
GtkWidget *entry_command_before_action = NULL;
GtkWidget *btn_check_system = NULL;
GtkWidget *check_iconify = NULL;
GtkWidget *label_action_description = NULL;
GtkWidget *btn_config_shutdown = NULL;
GtkWidget *btn_config_restart = NULL;
GtkWidget *btn_config_logout = NULL;
GtkWidget *entry_action_custom_command = NULL;
GtkDialog *config_action_dialog = NULL;
GtkWidget *dialog = NULL;
GtkWidget *main_window = NULL;
GtkComboBox *combo_time_type = NULL;
GtkWidget *mnu_tray_cancel = NULL;
GtkWidget *tray_eventbox = NULL;
GtkTooltips *tray_tooltips = NULL;
GtkButton *btn_test_notifications = NULL;
GtkCheckButton *check_show_notifications = NULL;
GtkWidget *radio_systray_always = NULL;
GtkWidget *radio_systray_if_active = NULL;
GtkWidget *radio_systray_never = NULL;
#ifndef DISABLE_TRAY_ICON
EggTrayIcon *tray_icon = NULL;
#endif
GtkWidget *config_dialog = NULL;
GtkWidget *tray_image = NULL;
GtkWidget *notification_date_dialog = NULL;
GtkButton *btn_notification_date = NULL;
GtkComboBox *notification_date_combo = NULL;
GtkButton *notification_date_add = NULL;
GtkButton *notification_date_remove = NULL;
GtkSpinButton *spin_notification_hour = NULL,
              *spin_notification_minute = NULL,
              *spin_notification_second = NULL; 

/* }}}1 */

/*
 * Initialisation of the graphical user interface.
 *
 * It returne TRUE on error.
 */
gboolean gui_init(int argc, char **argv)
{
    time_t t = time(0);
    struct tm *tm_time;

    gtk_init(&argc, &argv);

    #ifdef LIBNOTIFY_ENABLED
    notify_init(PACKAGE);
    #endif

    glade_xml = gui_load_glade("gshutdown.glade");
    if(glade_xml == NULL)
        return TRUE; /* Error */

    /* Call timer_day at 00:00 */
    tm_time = localtime(&t);
    g_timeout_add((24 - tm_time->tm_hour) * 3600000 - tm_time->tm_min * 60000 - tm_time->tm_sec * 1000, 
                  timer_day, NULL); /* TODO ve */

    return gui_connect_all();
}

/*
 * TODO description
 */
gboolean timer_day()
{
    gui_mark_today();
    g_timeout_add(86400000, gui_mark_today, NULL);
    return FALSE;
}

/*
 * Mark the actual day in the interface.
 */
gboolean gui_mark_today()
{
    time_t t = time(0);
    struct tm *tm_time = localtime(&t);

    gtk_calendar_clear_marks(calendar);

    if(values.time_date.year == tm_time->tm_year + 1900 && values.time_date.month == tm_time->tm_mon)
        gtk_calendar_mark_day(calendar, tm_time->tm_mday);

    return TRUE;
}

/*
 * Show a visual notification.
 */
void gui_notification(gchar *message, NotificationDelay type_delay, NotificationIcon show_icon)
{
#ifdef LIBNOTIFY_ENABLED
    NotifyNotification *notify;
    gint delay;
    gchar *path;
    GdkPixbuf *pixbuf;
    GtkWidget *widget = NULL;

    if(values.show_notifications == FALSE)
        return;

    notify = notify_notification_new("GShutdown", message, NULL);

    if(show_icon == NOTIFICATION_ICON_TIME) {
        path = data_path("clock.png");

        if(path != NULL) {
            pixbuf = gdk_pixbuf_new_from_file_at_scale(path, 48, 48, TRUE, NULL);
            notify_notification_set_icon_from_pixbuf(notify, pixbuf);
        }
    }

    if(type_delay == NOTIFICATION_TYPE_IMPORTANT)
        delay = 5500;
    else /* NOTIFICATION_TYPE_FAST_INFO */
        delay = 4000;

    notify_notification_set_timeout(notify, delay);

    notify_notification_add_action(notify, "default", "Open main window",
                                   (NotifyActionCallback)on_notify_click, 
                                   NULL, NULL);

    notify_notification_show(notify, NULL);
#endif
}

/*
 * Set the actual time in the spins.
 */
void gui_set_actual_time(gboolean modify_time, gboolean modify_date)
{
    time_t t = time(0);
    struct tm *tm_time = localtime(&t);

    /* Hour, Minute */
    if(modify_time == TRUE) {
        gtk_spin_button_set_value(spin_hour, tm_time->tm_hour);
        gtk_spin_button_set_value(spin_minute, tm_time->tm_min);
    }

    if(modify_date == TRUE) {
        /* Year, Month */
        gtk_calendar_select_month(calendar, tm_time->tm_mon,
                tm_time->tm_year + 1900);

        /* Day */
        gtk_calendar_select_day(calendar, tm_time->tm_mday);
    }

    gtk_calendar_mark_day (calendar, tm_time->tm_mday);
}

/*
 * Clean the list of notification date
 */

void gui_remove_notification_date()
{
   gsize i;
   for(i=0; i < *values.time_when_notify.nb; i++)
      gtk_combo_box_remove_text(GTK_COMBO_BOX(notification_date_combo), 0);
}

/*
 * Update notification date from current configuration
 */
void gui_set_notification_date()
{
    gchar text[100];
    gsize cur_lec;
    guint seconds = 0;
    guint minutes = 0;
    guint hours = 0;

    gui_remove_notification_date();

    for(cur_lec = 0; cur_lec < *values.time_when_notify.nb; cur_lec++) { 
        seconds = *(values.time_when_notify.time + cur_lec);
        hours = seconds / 3600;

        sprintf(text, " ");       

        if(hours > 0) {
            sprintf(text, N_("%s%u hour", "%s%u hours", hours),
                    " ", hours);
            seconds -= 3600 * hours;
        }

        minutes = seconds / 60;
        if(minutes > 0) {
            sprintf(text, N_("%s%u minute", "%s%u minutes", minutes),
                    " ", minutes);
            seconds -= 60 * minutes;
        }

        if(seconds > 0) {
            sprintf(text, N_("%s%u second", "%s%u seconds", seconds),
                    " ", seconds);
        }

        gtk_combo_box_append_text(GTK_COMBO_BOX(notification_date_combo), text);
    }
}

/*
 * Connect all widgets inside 'glade_xml', with it respective functions.
 * It build and show some widgets (like the tray icon).
 *
 * It return TRUE on error.
 */
gboolean gui_connect_all()
{
    GtkWidget *widget = NULL;
    GtkWidget *image  = NULL;

    /* Main window */
    main_window = glade_xml_get_widget(glade_xml, "main_window");
    combo_time_type = GTK_COMBO_BOX(glade_xml_get_widget(glade_xml, "combo_time_type"));
                      gtk_combo_box_append_text(combo_time_type, _("At date and time"));
                      gtk_combo_box_append_text(combo_time_type, _("After a delay"));
                      gtk_combo_box_append_text(combo_time_type, _("Now"));

    btn_start = GTK_BUTTON(glade_xml_get_widget(glade_xml, "btn_start"));
    vbox_select_time = glade_xml_get_widget(glade_xml, "vbox_select_time");
    btn_actual_time = GTK_BUTTON(glade_xml_get_widget(glade_xml, "btn_actual_time"));
    spin_hour = GTK_SPIN_BUTTON(glade_xml_get_widget(glade_xml, "spin_hour"));
    spin_minute = GTK_SPIN_BUTTON(glade_xml_get_widget(glade_xml, "spin_minute"));
    calendar = GTK_CALENDAR(glade_xml_get_widget(glade_xml, "calendar"));

    combo_action = GTK_COMBO_BOX(glade_xml_get_widget(glade_xml, "combo_action"));
                   gtk_combo_box_append_text(combo_action, _("Turn off the computer"));
                   gtk_combo_box_append_text(combo_action, _("Restart computer"));
                   gtk_combo_box_append_text(combo_action, _("End current session"));
                   gtk_combo_box_set_active(combo_action, 0);
                   #ifdef DEBUG
                   /*  In devel... */
                   gtk_combo_box_append_text(combo_action, _("Extras"));
                   #endif

    combo_extras = GTK_COMBO_BOX(glade_xml_get_widget(glade_xml, "combo_extras"));
    gtk_combo_box_append_text(combo_extras, _("Custom command..."));

    /* Dialog */
    about_dialog = glade_xml_get_widget(glade_xml, "about_dialog");
    gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(about_dialog), "GShutdown " VERSION);

    /* Configuration dialog */
    config_dialog = glade_xml_get_widget(glade_xml, "config_dialog");
    g_signal_connect(G_OBJECT(config_dialog), "delete-event", 
                     G_CALLBACK(on_config_dialog_delete_event), NULL);

    widget = glade_xml_get_widget(glade_xml, "btn_cfg_close");
    g_signal_connect(G_OBJECT(widget), "clicked", 
                     G_CALLBACK(on_btn_cfg_close), NULL);
    
    check_iconify = glade_xml_get_widget(glade_xml, "check_iconify");
    g_signal_connect(G_OBJECT(check_iconify), "toggled", 
                     G_CALLBACK(on_check_iconify), NULL);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_iconify), FALSE);

    radio_systray_always = glade_xml_get_widget(glade_xml, "radio_systray_always");
    radio_systray_if_active = glade_xml_get_widget(glade_xml, "radio_systray_if_active");
    radio_systray_never = glade_xml_get_widget(glade_xml, "radio_systray_never");
    g_signal_connect(G_OBJECT(radio_systray_always), "toggled", 
                     G_CALLBACK(on_radio_systray_always), NULL);
    g_signal_connect(G_OBJECT(radio_systray_if_active), "toggled", 
                     G_CALLBACK(on_radio_systray_if_active), NULL);
    g_signal_connect(G_OBJECT(radio_systray_never), "toggled", 
                     G_CALLBACK(on_radio_systray_never), NULL);

    btn_test_notifications = GTK_BUTTON(glade_xml_get_widget(glade_xml, "btn_test_notifications"));
    g_signal_connect(G_OBJECT(btn_test_notifications), "clicked", 
                     G_CALLBACK(on_btn_test_notifications), NULL);
    btn_notification_date = GTK_BUTTON(glade_xml_get_widget(glade_xml, "btn_notification_date"));
    g_signal_connect(G_OBJECT(btn_notification_date), "clicked", 
                     G_CALLBACK(on_btn_notification_date), NULL);

    check_show_notifications = GTK_CHECK_BUTTON(glade_xml_get_widget(glade_xml, "check_show_notifications"));
    g_signal_connect(G_OBJECT(check_show_notifications), "toggled", 
                     G_CALLBACK(on_check_show_notifications), NULL);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_show_notifications), TRUE);

    /* DIALOG: Notification date */
    notification_date_dialog = glade_xml_get_widget(glade_xml, "notification_date_dialog");
    g_signal_connect(G_OBJECT(notification_date_dialog), "delete-event",
                     G_CALLBACK(gtk_widget_hide_on_delete), NULL);
    widget = glade_xml_get_widget(glade_xml, "btn_notification_close");
    g_signal_connect(G_OBJECT(widget), "clicked", 
                     G_CALLBACK(on_btn_notification_date_close), notification_date_dialog);
    notification_date_combo = GTK_COMBO_BOX(glade_xml_get_widget(glade_xml, "notification_date_combo"));    
    notification_date_add = GTK_BUTTON(glade_xml_get_widget(glade_xml, "notification_date_add"));
    g_signal_connect(G_OBJECT(notification_date_add), "clicked", 
                     G_CALLBACK(on_btn_notification_date_add), NULL);
    notification_date_remove = GTK_BUTTON(glade_xml_get_widget(glade_xml, "notification_date_remove"));
    g_signal_connect(G_OBJECT(notification_date_remove), "clicked", 
                     G_CALLBACK(on_btn_notification_date_remove), NULL);
    spin_notification_hour = GTK_SPIN_BUTTON(glade_xml_get_widget(glade_xml, "spin_notification_hour"));
    spin_notification_minute = GTK_SPIN_BUTTON(glade_xml_get_widget(glade_xml, "spin_notification_minute"));
    spin_notification_second = GTK_SPIN_BUTTON(glade_xml_get_widget(glade_xml, "spin_notification_second"));

    /* Check system dialog */
    dialog = glade_xml_get_widget(glade_xml, "check_system_dialog");
    g_signal_connect(G_OBJECT(dialog), "delete-event",
                     G_CALLBACK(gtk_widget_hide_on_delete), NULL);

    btn_check_system = glade_xml_get_widget(glade_xml, "btn_check_system");
    g_signal_connect(G_OBJECT(btn_check_system), "clicked",
                     G_CALLBACK(on_btn_check_system), dialog);

    widget = glade_xml_get_widget(glade_xml, "btn_check_system_close");
    g_signal_connect(G_OBJECT(widget), "clicked",
                     G_CALLBACK(on_btn_check_system_close), dialog);

    values_action_method_autodetect(); /* Detect Disp. Manager & Desktop */
    {
        GtkWidget *desktop;
        GtkWidget *display;
        gchar *desktop_text = _("Not detected!");
        gchar *display_text = desktop_text;

        desktop = glade_xml_get_widget(glade_xml, "label_check_system_desktop");
        display = glade_xml_get_widget(glade_xml, "label_check_system_display_manager");

        /* Display Manager */
        if(values.detected_display_manager == DISPLAY_MANAGER_GDM)
            display_text = "GDM";
        else if(values.detected_display_manager == DISPLAY_MANAGER_KDM)
            display_text = "KDM";

        /* Desktop */
        if(values.detected_desktop == DESKTOP_GNOME)
            desktop_text = "GNOME";
        else if(values.detected_desktop == DESKTOP_KDE)
            desktop_text = "KDE";
        else if(values.detected_desktop == DESKTOP_XFCE)
            desktop_text = "XFCE";

        gtk_label_set_text(GTK_LABEL(desktop), desktop_text);
        gtk_label_set_text(GTK_LABEL(display), display_text);
    }

    /* Config Action dialog */
    config_action_dialog = GTK_DIALOG(glade_xml_get_widget(glade_xml, "config_action_dialog"));
    g_signal_connect(G_OBJECT(config_action_dialog), "delete-event", 
                     G_CALLBACK(on_config_action_dialog_delete_event), NULL);

    btn_config_shutdown = glade_xml_get_widget(glade_xml, "btn_config_shutdown");
    g_signal_connect(G_OBJECT(btn_config_shutdown), "clicked",
                     G_CALLBACK(on_btn_config_action), NULL);

    widget = glade_xml_get_widget(glade_xml, "btn_config_action_close");
    g_signal_connect(G_OBJECT(widget), "clicked",
                     G_CALLBACK(on_btn_config_action_close), NULL);

    btn_config_restart = glade_xml_get_widget(glade_xml, "btn_config_restart");
    g_signal_connect(G_OBJECT(btn_config_restart), "clicked",
                     G_CALLBACK(on_btn_config_action), NULL);

    btn_config_logout = glade_xml_get_widget(glade_xml, "btn_config_logout");
    g_signal_connect(G_OBJECT(btn_config_logout), "clicked",
                     G_CALLBACK(on_btn_config_action), NULL);
    
    image = glade_xml_get_widget(glade_xml, "image_command_before_action");
    check_command_before_action = glade_xml_get_widget(glade_xml, "check_command_before_action");
    g_signal_connect(G_OBJECT(check_command_before_action), "toggled", 
                     G_CALLBACK(on_check_command_before_action), NULL);
    entry_command_before_action = glade_xml_get_widget(glade_xml, "entry_command_before_action");
    g_signal_connect(G_OBJECT(entry_command_before_action), "changed", 
                     G_CALLBACK(on_entry_command_complete), 
                     image);
    on_check_command_before_action(check_command_before_action, NULL);
    on_entry_command_complete(entry_command_before_action, image);

    image = glade_xml_get_widget(glade_xml, "image_action_custom_command");
    entry_action_custom_command = glade_xml_get_widget(glade_xml, "entry_action_custom_command");
    g_signal_connect(G_OBJECT(entry_action_custom_command), "changed", 
                     G_CALLBACK(on_entry_command_complete), image);
    on_entry_command_complete(entry_action_custom_command, image);

    label_action_description = glade_xml_get_widget(glade_xml, "label_action_description");

    btn_action_method_manual = glade_xml_get_widget(glade_xml, "btn_action_method_manual");
    g_signal_connect(G_OBJECT(btn_action_method_manual), "clicked",
                     G_CALLBACK(on_btn_action_method_manual), NULL);

    radio_action_method_autodetect = glade_xml_get_widget(glade_xml, "radio_action_method_autodetect");
    radio_action_method_custom_command = glade_xml_get_widget(glade_xml, "radio_action_method_custom_command");
    radio_action_method_manual = glade_xml_get_widget(glade_xml, "radio_action_method_manual");
    g_signal_connect(G_OBJECT(radio_action_method_autodetect), "toggled", 
                     G_CALLBACK(on_radio_action_method), NULL);
    g_signal_connect(G_OBJECT(radio_action_method_manual), "toggled", 
                     G_CALLBACK(on_radio_action_method), NULL);
    g_signal_connect(G_OBJECT(radio_action_method_custom_command), "toggled", 
                     G_CALLBACK(on_radio_action_method), NULL);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_action_method_autodetect), 0);
    on_radio_action_method(radio_action_method_autodetect, NULL);

    /* DIALOG: Select the action method manually */
    method_manual_dialog = glade_xml_get_widget(glade_xml, "method_manual_dialog");
    g_signal_connect(G_OBJECT(method_manual_dialog), "delete-event",
                     G_CALLBACK(gtk_widget_hide_on_delete), NULL);

    widget = glade_xml_get_widget(glade_xml, "btn_method_manual_close");
    g_signal_connect(G_OBJECT(widget), "clicked", 
                     G_CALLBACK(on_btn_method_manual_close), NULL);

    widget = glade_xml_get_widget(glade_xml, "combo_method_manual_display_manager");
    g_signal_connect(G_OBJECT(widget), "changed", 
                     G_CALLBACK(on_combo_method_manual_display_manager), NULL);
    gtk_combo_box_append_text(GTK_COMBO_BOX(widget), _("Autodetect"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(widget), "GDM");
    gtk_combo_box_append_text(GTK_COMBO_BOX(widget), "KDM");
    gtk_combo_box_set_active(GTK_COMBO_BOX(widget), 0);
    on_combo_method_manual_display_manager(widget, NULL);

    widget = glade_xml_get_widget(glade_xml, "combo_method_manual_desktop");
    g_signal_connect(G_OBJECT(widget), "changed", 
                     G_CALLBACK(on_combo_method_manual_desktop), NULL);
    gtk_combo_box_append_text(GTK_COMBO_BOX(widget), _("Autodetect"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(widget), "GNOME");
    gtk_combo_box_append_text(GTK_COMBO_BOX(widget), "KDE");
    gtk_combo_box_append_text(GTK_COMBO_BOX(widget), "XFCE");
    gtk_combo_box_set_active(GTK_COMBO_BOX(widget), 0);
    on_combo_method_manual_desktop(widget, NULL);

    combo_method_manual_desktop = glade_xml_get_widget(glade_xml, "combo_method_manual_desktop");
    g_signal_connect(G_OBJECT(combo_method_manual_desktop), "changed", 
                     G_CALLBACK(on_combo_method_manual_desktop), NULL);
    combo_method_manual_display_manager = glade_xml_get_widget(glade_xml, "combo_method_manual_display_manager");
    g_signal_connect(G_OBJECT(combo_method_manual_display_manager), "changed", 
                     G_CALLBACK(on_combo_method_manual_display_manager), NULL);

    /* hide label config notification */
    #ifndef LIBNOTIFY_ENABLED
        widget = glade_xml_get_widget(glade_xml, "frame_config_notifications");
        gtk_widget_hide(widget);
    #endif

    /* Modifications */
    gtk_combo_box_set_active(combo_time_type, 0); /* Select the item 0 */
    gui_set_actual_time(TRUE, TRUE); /* Set the actual time/date */
    gtk_widget_grab_focus(GTK_WIDGET(calendar));

    /* Show all */
    gtk_widget_show(main_window);

    if(gui_systray_icon(main_window))
        return TRUE;

    return FALSE;
}

/*
 * Hide/Show the systray icon.
 */
void gui_systray_icon_hide()
{
#ifndef DISABLE_TRAY_ICON
    gtk_widget_hide_all(GTK_WIDGET(tray_icon));
#else
    return FALSE;
#endif
}

void gui_systray_icon_show()
{
#ifndef DISABLE_TRAY_ICON
    gtk_widget_show_all(GTK_WIDGET(tray_icon));
#else
    return FALSE;
#endif
}

gboolean gui_systray_icon_is_showed()
{
#ifndef DISABLE_TRAY_ICON
   return GTK_WIDGET_VISIBLE(GTK_WIDGET(tray_icon));
#else
    return FALSE;
#endif
}

/*
 * Create an icon in the system tray.
 *
 * It return TRUE on error.
 */
gboolean gui_systray_icon()
{
#ifndef DISABLE_TRAY_ICON
    gchar *path = NULL;

    /* Tray initialization */
    tray_icon = egg_tray_icon_new(PACKAGE);

    tray_eventbox = gtk_event_box_new();
    g_signal_connect(tray_eventbox, "button_press_event", 
                     G_CALLBACK(on_tray_icon), main_window);

    gtk_container_add(GTK_CONTAINER(tray_icon), tray_eventbox);
    g_object_ref(G_OBJECT(tray_icon));

    /* Image loading */
    path = data_path("tray_icon.png");
    if(path == NULL) {
        GtkWidget *dialog;

        dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
                                        GTK_DIALOG_MODAL,
                                        GTK_MESSAGE_ERROR,
                                        GTK_BUTTONS_OK,
                                        _("%s doesn't exist.\n"), 
                                        "tray_icon.png");

        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return TRUE;
    }

    tray_image = gtk_image_new_from_file(path);
    gtk_container_add(GTK_CONTAINER(tray_eventbox), tray_image);

    tray_tooltips = gtk_tooltips_new();
    gtk_tooltips_set_tip(tray_tooltips, tray_eventbox, _("No action scheduled") , NULL);

    g_free(path);

    return FALSE;
#else
    return FALSE;
#endif
}

/*
 * Load glade file.
 *
 * NULL are returned if the filename doesn't
 * exist.
 */
GladeXML *gui_load_glade(gchar *filename)
{
    gchar *path = NULL;
    GladeXML *glade_xml = NULL;

    /* Load glade file */
    path = data_path(filename);

    if(path == NULL) {
        GtkWidget *dialog;

        dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
                                       GTK_DIALOG_MODAL,
                                       GTK_MESSAGE_ERROR,
                                       GTK_BUTTONS_OK,
                                       _("%s doesn't exist.\n"), 
                                       filename);

        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return NULL;
    }

    glade_xml = glade_xml_new(path, NULL, NULL);
    g_free(path);

    glade_xml_signal_autoconnect(glade_xml);

    return glade_xml;
}

/*
 * Create the tray icon menu.
 */
GtkWidget *create_mnu_tray_action_now(void)
{
    GtkWidget *image, *item;
    GtkWidget *menu = gtk_menu_new();

    /* Macros */
    #define ITEM(name, stock, callback) \
        item = gtk_image_menu_item_new_with_mnemonic(name); \
        gtk_widget_show(item); \
        gtk_container_add(GTK_CONTAINER(menu), item); \
        g_signal_connect(G_OBJECT(item), "activate", \
                         G_CALLBACK(callback), NULL); \
        image = gtk_image_new_from_stock(stock, GTK_ICON_SIZE_MENU); \
        gtk_widget_show(image); \
        gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), image); \

    /* Items */
    ITEM(_("Shutdown"), "gtk-stop", on_shutdown_now);
    ITEM(_("Reboot"), "gtk-refresh", on_reboot_now);
    ITEM(_("Logout"), "gtk-disconnect", on_logout_now);

    #undef ITEM

    return menu;
}

/*
 * Generate TrayMenu.
 *
 * NOTE: Must be called one time. Because mnu_tray_cancel are global.
 */
GtkMenu *traymenu()
{
    GtkWidget *item, *image;
    GtkWidget *menu = gtk_menu_new();
    GtkAccelGroup *accel_group = gtk_accel_group_new();

    /* Macros */
    #define ITEM(name, stock, callback) \
        if(stock == NULL && name == NULL) { \
            item = gtk_separator_menu_item_new(); \
            gtk_widget_show(item); \
            gtk_container_add(GTK_CONTAINER(menu), item); \
            gtk_widget_set_sensitive(item, FALSE); \
        } else { \
            if(stock == NULL && name != NULL) \
                item = gtk_menu_item_new_with_mnemonic(name); \
            else { \
                if(name == NULL) \
                    item = gtk_image_menu_item_new_from_stock(stock, accel_group); \
                else \
                    item =  gtk_image_menu_item_new_with_mnemonic(name); \
            } \
            gtk_widget_show(item); \
            gtk_container_add(GTK_CONTAINER(menu), item); \
            if(callback != NULL) \
                g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(callback), NULL); \
            if(stock != NULL) { \
                image = gtk_image_new_from_stock(stock, GTK_ICON_SIZE_MENU); \
                gtk_widget_show(image); \
                gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), image); \
            } \
        }
#define SEPARATOR() ITEM(NULL, NULL, NULL)
#define STOCK_ITEM(stock, callback) ITEM(NULL, stock, callback)

    /* Items */
    ITEM(_("Immediate action"), NULL, NULL);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), create_mnu_tray_action_now());

    ITEM(_("Cancel current action"), "gtk-cancel", on_mnu_tray_cancel);
    mnu_tray_cancel = item;
    gtk_widget_set_sensitive(mnu_tray_cancel, values.action_scheduled);

    STOCK_ITEM("gtk-preferences", NULL);
    g_signal_connect(G_OBJECT(item), "activate", 
                     G_CALLBACK(on_mnu_config_activate), dialog);

    SEPARATOR();

    STOCK_ITEM("gtk-quit", on_mnu_quit_activate);

#undef STOCK_ITEM
#undef ITEM
#undef SEPARATOR

    return GTK_MENU(menu);
}

gboolean confirm_dialog(Action *id_action)
{
    GtkWidget *dialog;
    gint response;
    gchar *message;

    if(detect_error_dialog())
        return FALSE;

    /* Message dialog */
    if(id_action == values.action_shutdown)
        message = _("Are you sure you want to shutdown your computer now ?");
    else if(id_action == values.action_restart)
        message = _("Are you sure you want to restart your computer now ?");
    else if(id_action == values.action_logout)
        message = _("Are you sure you want to logout from your session now ?");

    dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
                                    GTK_DIALOG_MODAL,
                                    GTK_MESSAGE_QUESTION,
                                    GTK_BUTTONS_YES_NO,
                                    message);

    response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(GTK_WIDGET(dialog));

    if(response == GTK_RESPONSE_YES)
        return TRUE;

    return FALSE;
}

gboolean detect_error_dialog()
{
    if(values.action_method == ACTION_METHOD_AUTODETECT) {
        GtkWidget *dialog;
        gchar *message = "";

        if(values.current_action != values.action_logout && 
           values.detected_display_manager == DISPLAY_MANAGER_NONE) {
            message = _("GShutdown was unable to detect your display manager. " 
                        "Currently, GShutdown only supports GDM and KDM.\n\n"
                        "Go to the configuration menu and specify the correct method to do that.");
        }
        else if(values.detected_desktop == DESKTOP_NONE)
            message = _("GShutdown was unable to detect your desktop. "
                        "Currently, GShutdown only supports GNOME and KDE.\n\n"
                        "Go to the configuration menu and specify the correct method to do that.");
        else
            return FALSE;

        dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
                                        GTK_DIALOG_MODAL,
                                        GTK_MESSAGE_ERROR,
                                        GTK_BUTTONS_OK,
                                        message);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);

        return TRUE;
    }

    return FALSE;
}

/* vim:cindent:et:sw=4:ts=4:sts=4:tw=78:fenc=utf-8:foldmethod=indent
 */
