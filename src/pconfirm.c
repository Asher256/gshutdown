/*
 * This file is part of gshutdown.
 *
 * Copyright (c) GShutdown Team <https://github.com/Asher256/gshutdown/issues>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 *(at your option) any later version.
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
#include "callback.h"
#include "pconfirm.h"
#include "values.h"
#include "gtk/gtk.h"

static GtkWidget *cancel_dialog = NULL;
gboolean progress_confirm_open = FALSE;

gboolean update_progress_bar(gpointer data)
{
    if(gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(data)) < 0.99) {
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(data),
                                      gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(data)) + 0.01);
        return TRUE;
    }
    else {
        on_progress_execute(NULL, cancel_dialog);
        return FALSE;
    }
}

void create_cancel_dialog(void)
{
    GtkWidget *vbox1;
    GtkWidget *hbox4;
    GtkWidget *image;
    GtkWidget *label;
    GtkWidget *vbox3;
    GtkWidget *progressbar;
    GtkWidget *hbuttonbox1;
    GtkWidget *cancel_button;
    GtkWidget *execute_button;
    gchar *message;
    guint *func_ref;

    cancel_dialog = gtk_window_new(GTK_WINDOW_POPUP);
    gtk_container_set_border_width(GTK_CONTAINER(cancel_dialog), 20);
    gtk_window_set_title(GTK_WINDOW(cancel_dialog), _("Warning !"));
    gtk_window_set_position(GTK_WINDOW(cancel_dialog), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(cancel_dialog), FALSE);
    gtk_window_set_type_hint(GTK_WINDOW(cancel_dialog), GDK_WINDOW_TYPE_HINT_DESKTOP);

    vbox1 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox1);
    gtk_container_add(GTK_CONTAINER(cancel_dialog), vbox1);

    hbox4 = gtk_hbox_new(FALSE, 15);
    gtk_widget_show(hbox4);
    gtk_box_pack_start(GTK_BOX(vbox1), hbox4, FALSE, FALSE, 0);

    image = gtk_image_new_from_stock("gtk-dialog-warning", GTK_ICON_SIZE_DIALOG);
    gtk_widget_show(image);
    gtk_box_pack_start(GTK_BOX(hbox4), image, TRUE, TRUE, 0);

    if(values.current_action == values.action_shutdown)
        message = _("Warning ! Your computer is going to shutdown now !");
    else if(values.current_action == values.action_restart)
        message = _("Warning ! Your computer is going to restart now !");
    else if(values.current_action == values.action_logout)
        message = _("Warning ! Your going to logout now !");

    label = gtk_label_new(message);
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(hbox4), label, FALSE, FALSE, 0);

    vbox3 = gtk_vbox_new(FALSE, 15);
    gtk_widget_show(vbox3);
    gtk_box_pack_start(GTK_BOX(vbox1), vbox3, TRUE, TRUE, 0);

    progressbar = gtk_progress_bar_new();
    gtk_widget_show(progressbar);
    gtk_box_pack_start(GTK_BOX(vbox3), progressbar, FALSE, FALSE, 0);

    hbuttonbox1 = gtk_hbutton_box_new();
    gtk_widget_show(hbuttonbox1);
    gtk_box_pack_start(GTK_BOX(vbox3), hbuttonbox1, TRUE, TRUE, 0);

    cancel_button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
    gtk_widget_show(cancel_button);
    gtk_container_add(GTK_CONTAINER(hbuttonbox1), cancel_button);
    g_signal_connect(G_OBJECT(cancel_button), "clicked",
            G_CALLBACK(on_progress_cancel),
            cancel_dialog);

    execute_button = gtk_button_new_from_stock(GTK_STOCK_EXECUTE);
    gtk_widget_show(execute_button);
    gtk_container_add(GTK_CONTAINER(hbuttonbox1), execute_button);
    g_signal_connect(G_OBJECT(execute_button), "clicked",
            G_CALLBACK(on_progress_execute),
            cancel_dialog);

    func_ref = g_malloc(sizeof(guint));
    *func_ref = g_timeout_add(300, update_progress_bar, progressbar);
    g_signal_connect(G_OBJECT(cancel_dialog), "destroy",
                     G_CALLBACK(on_progress_destroy), func_ref);

    gtk_widget_show(cancel_dialog);
}

void progress_confirm(void)
{
    create_cancel_dialog();
    progress_confirm_open = TRUE;  
}

void progress_destroy(void)
{
    gtk_widget_destroy(cancel_dialog);
}

/* vim:cindent:et:sw=4:ts=4:sts=4:tw=78:fenc=utf-8
 */
