/*
 * Copyright (C) 2003 Bastien Nocera <hadess@hadess.net>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#ifndef BACON_MESSAGE_CONNECTION_H
#define BACON_MESSAGE_CONNECTION_H

#ifndef S_IFSOCK
# define S_IFSOCK __S_IFSOCK
# define S_ISSOCK(mode) __S_ISTYPE((mode), __S_IFSOCK)
#endif

#include <glib.h>

G_BEGIN_DECLS

typedef void (*BaconMessageReceivedFunc) (const char *message,
                      gpointer user_data);

typedef struct BaconMessageConnection BaconMessageConnection;

BaconMessageConnection *bacon_message_connection_new    (const char *prefix);
void bacon_message_connection_free            (BaconMessageConnection *conn);
void bacon_message_connection_set_callback        (BaconMessageConnection *conn,
                             BaconMessageReceivedFunc func,
                             gpointer user_data);
void bacon_message_connection_send            (BaconMessageConnection *conn,
                             const char *message);
gboolean bacon_message_connection_get_is_server        (BaconMessageConnection *conn);

G_END_DECLS

#endif /* BACON_MESSAGE_CONNECTION_H */

/* vim:cindent:et:sw=4:ts=4:sts=4:tw=78:fenc=utf-8
 */
