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

void progress_confirm(void);
void create_cancel_dialog (void);
void progress_destroy(void);
gboolean update_progress_bar (gpointer data);
extern gboolean progress_confirm_open;

/* vim:cindent:et:sw=4:ts=4:sts=4:tw=78:fenc=utf-8
 */
