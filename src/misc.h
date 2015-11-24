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
#ifndef __GSHUTDOWN_MISC__
#define __GSHUTDOWN_MISC__

#include <glib.h>      /* gchar */

/* Return a data file path. e.g: gshutdown.glade */
gchar *data_path(gchar *filename);

/* run a command with fork/exec */
gboolean run_command(const gchar *command);

/* Test if a command exists */
gboolean command_exists(const gchar *command);

/* Which returns the pathnames of the files which would 
 * be executed in the current environment */
gchar *which(gchar *command);

/* Quit the program */
#define QUIT_FAILURE 1
#define QUIT_SUCCESS 0
void quit(int);

#endif

/* vim:cindent:et:sw=4:ts=4:sts=4:tw=78:fenc=utf-8
*/
