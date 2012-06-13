/*
 * SCIM Bridge
 *
 * Copyright (c) 2006 Ryo Dairiki <ryo-dairiki@users.sourceforge.net>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation and 
 * appearing in the file LICENSE.LGPL included in the package of this file.
 * You can also redistribute it and/or modify it under the terms of 
 * the GNU General Public License as published by the Free Software Foundation and 
 * appearing in the file LICENSE.GPL included in the package of this file.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <stdlib.h>
#include <string.h>

#include "scim-bridge-display.h"
#include "scim-bridge-output.h"

/* Type definitions */
struct _ScimBridgeDisplay
{
    char *name;
    int display_number;
    int screen_number;
};

/* Implementations */
ScimBridgeDisplay *scim_bridge_alloc_display ()
{
    ScimBridgeDisplay *display = malloc (sizeof (ScimBridgeDisplay));
    display->name = malloc (sizeof (char));
    display->name[0] = '\0';
    display->display_number = -1;
    display->screen_number = -1;

    return display;
}


void scim_bridge_free_display (ScimBridgeDisplay *display)
{
    if (display == NULL) return;

    free (display->name);
    free (display);
}


void scim_bridge_copy_display (ScimBridgeDisplay *dst, const ScimBridgeDisplay *src)
{
    if (dst == NULL || src == NULL) {
        scim_bridge_perrorln ("The pointer given as a display is NULL");
        abort ();
    }

    const size_t str_length = strlen (src->name);
    dst->name = realloc (dst->name, sizeof (char) * (str_length + 1));
    strcpy (dst->name, src->name);
    
    dst->display_number = src->display_number;
    dst->screen_number = src->screen_number;
}


boolean scim_bridge_display_equals (const ScimBridgeDisplay *dst, const ScimBridgeDisplay *src)
{
    return dst->display_number == src->display_number && dst->screen_number == src->screen_number;
}


retval_t scim_bridge_display_fetch_current (ScimBridgeDisplay *display)
{
    if (display == NULL) {
        scim_bridge_perrorln ("The pointer given as a display is NULL");
        return RETVAL_FAILED;
    }

    int display_number = 0;
    int screen_number = 0;

    char *display_name = getenv ("DISPLAY");
    if (display_name == NULL) {
        return RETVAL_FAILED;
    } else {
        const char *c;
        for (c = display_name; TRUE; ++c) {
            if (*c == ':') {
                break;
            } else if (*c == '\0') {
                return RETVAL_FAILED;
            }
        }
    
        boolean reading_display_number = TRUE;
        static char *digits = "0123456789";
        for (c += sizeof (char); *c != '\0'; ++c) {
            switch (*c) {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    if (reading_display_number) {
                        display_number = display_number * 10 + (index (digits, *c) - digits);
                    } else {
                        screen_number = screen_number * 10 + (index (digits, *c) - digits);
                    }
                    break;
                case '.':
                    if (reading_display_number) {
                        reading_display_number = FALSE;
                    } else {
                        return RETVAL_FAILED;
                    }
                    break;
                default:
                    return RETVAL_FAILED;
            }
        }

        const size_t display_name_str_length = strlen (display_name);

        free (display->name);
        display->name = malloc (sizeof (char) * (display_name_str_length + 1));
        strcpy (display->name, display_name);

        display->display_number = display_number;
        display->screen_number = screen_number;
        return RETVAL_SUCCEEDED;
    }
}


const char *scim_bridge_display_get_name (const ScimBridgeDisplay *display)
{
    if (display == NULL) {
        scim_bridge_perrorln ("The pointer given as a display is NULL");
        return NULL;
    }

    return display->name;
}


void scim_bridge_display_set_name (ScimBridgeDisplay *display, const char *display_name)
{
    if (display == NULL) {
        scim_bridge_perrorln ("The pointer given as a display is NULL");
        abort ();
    }

    if (display_name == NULL) {
        scim_bridge_perrorln ("The pointer given as a string is NULL");
        abort ();
    }

    free (display->name);
    display->name = malloc (sizeof (char) * (strlen (display_name) + 1));
    strcpy (display->name, display_name);
}


int scim_bridge_display_get_display_number (const ScimBridgeDisplay *display)
{
    if (display == NULL) {
        scim_bridge_perrorln ("The pointer given as a display is NULL");
        abort ();
    }

    return display->display_number;
}


void scim_bridge_display_set_display_number (ScimBridgeDisplay *display, int display_number)
{
    if (display == NULL) {
        scim_bridge_perrorln ("The pointer given as a display is NULL");
        abort ();
    }

    display->display_number = display_number;
}


int scim_bridge_display_get_screen_number (const ScimBridgeDisplay *display)
{
    if (display == NULL) {
        scim_bridge_perrorln ("The pointer given as a display is NULL");
        abort ();
    }

    return display->screen_number;
}


void scim_bridge_display_set_screen_number (ScimBridgeDisplay *display, int screen_number)
{
    if (display == NULL) {
        scim_bridge_perrorln ("The pointer given as a display is NULL");
        abort ();
    }

    display->screen_number = screen_number;
}
