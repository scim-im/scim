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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "scim-bridge-debug.h"
#include "scim-bridge-output.h"

/* Implementations */
void scim_bridge_println (const char *string,...)
{
    va_list ap;
    const size_t str_length = strlen (string);
    char *format = alloca (sizeof (char) * (str_length + 2));
    strcpy (format, string);
    format[str_length] = '\n';
    format[str_length + 1] = '\0';
    va_start (ap, string);
    vfprintf (stdout, format, ap);
    va_end (ap);
}


void scim_bridge_perrorln (const char *string,...)
{
    va_list ap;
    const size_t str_length = strlen (string);
    char *format = alloca (sizeof (char) * (str_length + 2));
    strcpy (format, string);
    format[str_length] = '\n';
    format[str_length + 1] = '\0';
    va_start (ap, string);
    vfprintf (stderr, format, ap);
    va_end (ap);
}


void scim_bridge_pdebugln (scim_bridge_debug_level_t level, const char *string,...)
{
    if ((10 - level) <= scim_bridge_debug_get_level ()) {
        va_list ap;
        va_start (ap, string);
        const size_t str_length = strlen (string);
        char *format = alloca (sizeof (char) * (str_length + 2));
        strcpy (format, string);
        format[str_length] = '\n';
        format[str_length + 1] = '\0';
        vfprintf (stdout, format, ap);
        va_end (ap);
    }
}


void scim_bridge_print (const char *string,...)
{
    va_list ap;
    va_start (ap, string);
    vfprintf (stdout, string, ap);
    va_end (ap);
}


void scim_bridge_perror (const char *string,...)
{
    va_list ap;
    va_start (ap, string);
    vfprintf (stderr, string, ap);
    va_end (ap);
}


void scim_bridge_pdebug (scim_bridge_debug_level_t level, const char *string,...)
{
    if ((10 - level) <= scim_bridge_debug_get_level ()) {
        va_list ap;
        va_start (ap, string);
        vfprintf (stdout, string, ap);
        va_end (ap);
    }
}
