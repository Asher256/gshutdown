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

/* window */
void on_btn_start_clicked(GtkButton *button, gpointer data);
void on_calendar_day_selected(GtkCalendar *calendar, gpointer data);
void on_combo_action_changed(GtkWidget *widget, gpointer data);
void on_btn_actual_time_clicked(GtkButton *, gpointer);
void on_spin_hour_value_changed(GtkSpinButton *, GtkScrollType *, gpointer);
void on_spin_minute_value_changed(GtkSpinButton *, GtkScrollType *, gpointer);
void on_combo_time_type_changed(GtkComboBox *, gpointer );
gboolean on_window_close_delete_event(GtkWidget *, gpointer);

/* menu */
void on_mnu_quit_activate(GtkWidget *, gpointer);
void on_mnu_about_activate(GtkWidget *widget, gpointer data);
void on_mnu_config_activate(GtkComboBox *combo, gpointer data);

/* vim:cindent:et:sw=4:ts=4:sts=4:tw=78:fenc=utf-8
*/
