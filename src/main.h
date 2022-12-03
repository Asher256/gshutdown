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

#ifndef __GSHUTDOWN_MAIN__
#define __GSHUTDOWN_MAIN__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef ENABLE_NLS
    #include <libintl.h>
    #ifdef _
        #undef _
    #endif
    #define _(string) dgettext(GETTEXT_PACKAGE, string)
    #define N_(string1, string2, n) dngettext(GETTEXT_PACKAGE, string1, string2, n)
#else
    #define _(string) (string)
    #define N_(string1, string2, n) internal_ngettext(string1, string2, n)
#endif

void called_at_exit();

#endif

/* vim:cindent:et:sw=4:ts=4:sts=4:tw=78:fenc=utf-8
 */
