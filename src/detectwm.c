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

#define DISABLE_DETECT_WM

#include <glib.h>

#ifndef DISABLE_DETECT_WM

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#define MAX_PROPERTY_VALUE_LEN 4096

static gchar *wm_get_name(Display *disp);
/* static int wm_info(Display *disp); */
static gchar *get_output_str(gchar *str, gboolean is_utf8);
static void init_charset(void);
static gchar *get_property(Display *disp, Window win, Atom xa_prop_type, gchar *prop_name,
                           unsigned long *size);
   
static struct {
    int verbose;
    int force_utf8;
    int show_class;
    int show_pid;
    int show_geometry;
    int match_by_id;
    int match_by_cls;
    int full_window_title_match;
    int wa_desktop_titles_invalid_utf8;
    char *param_window;
    char *param;
} options;

static gboolean envir_utf8;

void p_verbose(const char *format, ...)
{
    va_list ap;

    if(!options.verbose) 
        return;

    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
}

/*
 * Return the name of the Window Manager.
 * Return NULL if it's not detected.
 *
 * NB: It must be freed by g_free()
 */
gchar *detect_window_manager()
{
    gchar *name;
    Display *disp;

    init_charset();

    if(!(disp = XOpenDisplay(NULL)))
        return NULL;

    name = wm_get_name(disp);

    XCloseDisplay(disp);

    return name;
}

#if 0
int main(int argc, char **argv) {
    int ret = EXIT_SUCCESS;

    Display *disp;

    /* necessary to make g_get_charset() and g_locale_*() work */
    setlocale(LC_ALL, "");    

    init_charset();

    if(!(disp = XOpenDisplay(NULL))) {
        fputs("Cannot open display.\n", stderr);
        return EXIT_FAILURE;
    }

    ret = wm_info(disp);

    XCloseDisplay(disp);
    return ret;
}
#endif

/*
 * Return the name of the window manager.
 * NULL on error.
 */
static gchar *wm_get_name(Display *disp)
{
    Window *sup_window = NULL;
    gchar *wm_name = NULL;
    gboolean name_is_utf8 = TRUE;
    gchar *name_out;
    
    if(!(sup_window = (Window *)get_property(disp, DefaultRootWindow(disp), XA_WINDOW, "_NET_SUPPORTING_WM_CHECK", NULL))) 
        if(!(sup_window = (Window *)get_property(disp, DefaultRootWindow(disp), XA_CARDINAL, "_WIN_SUPPORTING_WM_CHECK", NULL)))
            return NULL;

    /* WM_NAME */
    if(!(wm_name = get_property(disp, *sup_window, XInternAtom(disp, "UTF8_STRING", False), "_NET_WM_NAME", NULL)))
        return NULL;

    name_out = get_output_str(wm_name, name_is_utf8);
  
    g_free(sup_window);
    g_free(wm_name);
    
    return name_out;
}

static void init_charset(void)
{
    const gchar *charset; /* unused */
    gchar *lang = getenv("LANG") ? g_ascii_strup(getenv("LANG"), -1) : NULL; 
    gchar *lc_ctype = getenv("LC_CTYPE") ? g_ascii_strup(getenv("LC_CTYPE"), -1) : NULL;
    
    /* this glib function doesn't work on my system ... */
    envir_utf8 = g_get_charset(&charset);

    /* ... therefore we will examine the environment variables */
    if(lc_ctype &&(strstr(lc_ctype, "UTF8") || strstr(lc_ctype, "UTF-8")))
        envir_utf8 = TRUE;
    else if(lang &&(strstr(lang, "UTF8") || strstr(lang, "UTF-8")))
        envir_utf8 = TRUE;

    g_free(lang);
    g_free(lc_ctype);
    
    if(options.force_utf8)
        envir_utf8 = TRUE;

    p_verbose("envir_utf8: %d\n", envir_utf8);
}

#if 0
static int wm_info(Display *disp) {
    Window *sup_window = NULL;
    gchar *wm_name = NULL;
    gchar *wm_class = NULL;
    unsigned long *wm_pid = NULL;
    unsigned long *showing_desktop = NULL;
    gboolean name_is_utf8 = TRUE;
    gchar *name_out;
    gchar *class_out;
    
    if(!(sup_window =(Window *)get_property(disp, DefaultRootWindow(disp),
                                            XA_WINDOW, 
                                            "_NET_SUPPORTING_WM_CHECK", NULL))) {
        if(!(sup_window =(Window *)get_property(disp, DefaultRootWindow(disp),
                                                XA_CARDINAL, 
                                                "_WIN_SUPPORTING_WM_CHECK", NULL))) {
            fputs("Cannot get window manager info properties.\n"
                  "(_NET_SUPPORTING_WM_CHECK or _WIN_SUPPORTING_WM_CHECK)\n", stderr);
            return EXIT_FAILURE;
        }
    }

    /* WM_NAME */
    if(!(wm_name = get_property(disp, *sup_window,
            XInternAtom(disp, "UTF8_STRING", False), "_NET_WM_NAME", NULL))) {
        name_is_utf8 = FALSE;
        if(!(wm_name = get_property(disp, *sup_window,
                XA_STRING, "_NET_WM_NAME", NULL))) {
            p_verbose("Cannot get name of the window manager (_NET_WM_NAME).\n");
        }
    }
    name_out = get_output_str(wm_name, name_is_utf8);
  
    /* WM_CLASS */
    if(!(wm_class = get_property(disp, *sup_window,
            XInternAtom(disp, "UTF8_STRING", False), "WM_CLASS", NULL))) {
        name_is_utf8 = FALSE;
        if(!(wm_class = get_property(disp, *sup_window,
                XA_STRING, "WM_CLASS", NULL))) {
            p_verbose("Cannot get class of the window manager (WM_CLASS).\n");
        }
    }
    class_out = get_output_str(wm_class, name_is_utf8);
  

    /* WM_PID */
    if(!(wm_pid = (unsigned long *)get_property(disp, *sup_window,
                    XA_CARDINAL, "_NET_WM_PID", NULL))) {
        p_verbose("Cannot get pid of the window manager (_NET_WM_PID).\n");
    }
    
    /* _NET_SHOWING_DESKTOP */
    if(!(showing_desktop = (unsigned long *)get_property(disp, DefaultRootWindow(disp),
                    XA_CARDINAL, "_NET_SHOWING_DESKTOP", NULL))) {
        p_verbose("Cannot get the _NET_SHOWING_DESKTOP property.\n");
    }
    
    /* print out the info */
    printf("Name: %s\n", name_out ? name_out : "N/A");
    printf("Class: %s\n", class_out ? class_out : "N/A");
    
    if(wm_pid) 
        printf("PID: %lu\n", *wm_pid);
    else
        printf("PID: N/A\n");
    
    if(showing_desktop) {
        printf("Window manager's \"showing the desktop\" mode: %s\n",
                *showing_desktop == 1 ? "ON" : "OFF");
    }
    else {
        printf("Window manager's \"showing the desktop\" mode: N/A\n");
    }
    
    g_free(name_out);
    g_free(sup_window);
    g_free(wm_name);
    g_free(wm_class);
    g_free(wm_pid);
    g_free(showing_desktop);
    
    return EXIT_SUCCESS;
}
#endif

static gchar *get_property(Display *disp, Window win,
                           Atom xa_prop_type, gchar *prop_name, 
                           unsigned long *size)
{
    Atom xa_prop_name;
    Atom xa_ret_type;
    int ret_format;
    unsigned long ret_nitems;
    unsigned long ret_bytes_after;
    unsigned long tmp_size;
    unsigned char *ret_prop;
    gchar *ret;
    
    xa_prop_name = XInternAtom(disp, prop_name, False);
    
    /* MAX_PROPERTY_VALUE_LEN / 4 explanation (XGetWindowProperty manpage):
     *
     * long_length = Specifies the length in 32-bit multiples of the
     *               data to be retrieved.
     */
    if(XGetWindowProperty(disp, win, xa_prop_name, 0, MAX_PROPERTY_VALUE_LEN / 4, False,
            xa_prop_type, &xa_ret_type, &ret_format,     
            &ret_nitems, &ret_bytes_after, &ret_prop) != Success) {
        p_verbose("Cannot get %s property.\n", prop_name);
        return NULL;
    }
  
    if(xa_ret_type != xa_prop_type) {
        p_verbose("Invalid type of %s property.\n", prop_name);
        XFree(ret_prop);
        return NULL;
    }

    /* null terminate the result to make string handling easier */
    tmp_size =(ret_format / 8) * ret_nitems;
    ret = g_malloc(tmp_size + 1);
    memcpy(ret, ret_prop, tmp_size);
    ret[tmp_size] = '\0';

    if(size)
        *size = tmp_size;
    
    XFree(ret_prop);
    return ret;
}

static gchar *get_output_str(gchar *str, gboolean is_utf8)
{
    gchar *out;
   
    if(str == NULL)
        return NULL;
    
    if(envir_utf8) {
        if(is_utf8)
            out = g_strdup(str);
        
        else {
            if(!(out = g_locale_to_utf8(str, -1, NULL, NULL, NULL))) {
                p_verbose("Cannot convert string from locale charset to UTF-8.\n");
                out = g_strdup(str);
            }
        }
    }
    else {
        if(is_utf8) {
            if(!(out = g_locale_from_utf8(str, -1, NULL, NULL, NULL))) {
                p_verbose("Cannot convert string from UTF-8 to locale charset.\n");
                out = g_strdup(str);
            }
        }
        else 
            out = g_strdup(str);
    }

    return out;
}

#else

gchar *detect_window_manager() {
    return NULL;
}

#endif

/* vim:cindent:et:sw=4:ts=4:sts=4:tw=78:fenc=utf-8
 */
