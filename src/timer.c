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

#include <glib.h>
#include "timer.h"
#include "values.h"
#include "gui.h"
#include "pconfirm.h"
#include "main.h"

const guint SECS_IN_YEAR = 31536000;
const guint SECS_IN_MONTH = 2628000;
const guint SECS_IN_DAY = 86400;
const guint SECS_IN_HOUR = 3600;
const guint SECS_IN_MIN = 60;

/*
 * Mumber of days in month.
 *
 * month : 0 to 11
 * year  : YYYY
 */
guint nb_days_in_month(guint month, guint year)
{
    gint nb_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
   
    /* Leap year */
    if((!(year % 4) && year % 100) || !(year % 400))
        nb_days[1] = 29;

    return nb_days[month];
}

/*
 * Return time left in seconds.
 */
guint time_left()
{
    time_t t = time(0);
    struct tm *tm_time = localtime(&t);

    gint timeleft = 0;
    gint nbdays = 0;
    guint month, year;

    /* At Date Time */
    if(values.time_choice == TIME_CHOICE_TIME_DATE) {
        year = tm_time->tm_year + 1900;

        while(year - values.time_date.year) {
           if(values.time_date.month > 1) {
              if((!((year + 1) % 4) && (year + 1) % 100) || !((year + 1) % 400))
                 nbdays = 366;
              else
                 nbdays = 365;
           }
           else {
              if((!(year % 4) && year % 100) || !(year % 400))
                 nbdays = 366;
              else
                 nbdays = 365;
           }

           year++;
        }

        month = tm_time->tm_mon;
        while(values.time_date.month > month) {
           nbdays += nb_days_in_month(month, year);
           month++;
        }

        while(values.time_date.month < month) {
           nbdays -= nb_days_in_month(month, year);
           month--;
        }

        nbdays += values.time_date.day - tm_time->tm_mday;

        timeleft += nbdays * SECS_IN_DAY;
        timeleft += (values.time_date.hour - tm_time->tm_hour) * SECS_IN_HOUR;
        timeleft += (values.time_date.min - tm_time->tm_min) * SECS_IN_MIN;
    }

    /* Time from now */
    if(values.time_choice == TIME_CHOICE_TIME_FROM_NOW) {
        timeleft += values.time_from_now.hour * SECS_IN_HOUR;
        timeleft += values.time_from_now.min * SECS_IN_MIN;
        timeleft += values.time_from_now.sec;
    }

    timeleft -= tm_time->tm_sec;

    if(timeleft < 0)
        timeleft = 0;

    return timeleft;
}

/*
 * Function called every seconds.
 *
 * Returne TRUE on error.
 */
gboolean timer_notify()
{
    guint timeleft;
    time_t t = time(0);
    struct tm *tm_time = localtime(&t);
    static guint min = -1;
    gchar message[100];
    gint i;

    if(min == -1)
        min = tm_time->tm_min;

    if(values.time_choice == TIME_CHOICE_TIME_FROM_NOW) {
        /* Minute has changed */
        if(min != tm_time->tm_min) {
            min = tm_time->tm_min;

            if(values.time_from_now.min)
                values.time_from_now.min--;
            else {
                values.time_from_now.min = 59;
                values.time_from_now.hour--;
            }
        }
    }

    timeleft = time_left();

    if(action_get_type(values.current_action) == ACTION_TYPE_SHUTDOWN)
        sprintf(message, _("The computer will be shut down in :"));
    else if(action_get_type(values.current_action) == ACTION_TYPE_RESTART)
        sprintf(message, _("The computer will be restarted in :"));
    else if(action_get_type(values.current_action) == ACTION_TYPE_LOGOUT)
        sprintf(message, _("Time before logout :"));
    else
        sprintf(message, _("Time before action :"));

    if(timeleft >= SECS_IN_MIN) {
        int tvalue;
        
        if(timeleft < SECS_IN_HOUR) {
            /* minutes */
            tvalue = timeleft / SECS_IN_MIN;
            sprintf(message, N_("%s %u minute", "%s %u minutes", tvalue),
                    message, tvalue);
            
            /* seconds */
            if(timeleft < 5 * SECS_IN_MIN) {
                tvalue = timeleft % SECS_IN_MIN;
                sprintf(message, N_("%s and %u second", "%s and %u seconds", tvalue),
                        message, tvalue);
            }
        } else if(timeleft < SECS_IN_DAY) {
            tvalue = timeleft / SECS_IN_HOUR;
            sprintf(message, N_("%s %u hour", "%s %u hours", tvalue),
                    message, tvalue);
            
            tvalue = (timeleft % SECS_IN_HOUR) / SECS_IN_MIN;
            sprintf(message, N_("%s and %u minute", "%s and %u minutes", tvalue),
                    message, tvalue);
        } else { /* plus d'un jour */
            tvalue = timeleft / SECS_IN_DAY;
            sprintf(message, N_("%s %u day", "%s %u days", tvalue),
                    message, tvalue);
            
            tvalue = (timeleft % SECS_IN_DAY) / SECS_IN_HOUR;
            sprintf(message, N_("%s and %u hour", "%s and %u hours", tvalue),
                    message, tvalue);
        }
        
        gtk_tooltips_set_tip(tray_tooltips, tray_eventbox, message, NULL);
        
        for(i = 0; i < *values.time_when_notify.nb; i++) {
            if(timeleft == values.time_when_notify.time[i]) {
                gui_notification(message, NOTIFICATION_TYPE_IMPORTANT, 
                                 NOTIFICATION_ICON_TIME);
            }
        }
    }
    else {
        sprintf(message, N_("%s %u second", "%s %u seconds", timeleft), 
                message, timeleft);
        
        gtk_tooltips_set_tip(tray_tooltips, tray_eventbox, message, NULL);
        
        if(timeleft <= 30 && progress_confirm_open == FALSE)
            progress_confirm();
    }
    
    return TRUE;
}

/* vim:cindent:et:sw=4:ts=4:sts=4:tw=78:fenc=utf-8
 */
