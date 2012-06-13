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
#include <assert.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "scim-bridge-debug.h"
#include "scim-bridge-output.h"

/* Static variables */
static bool initialized = false;

static const char SCIM_BRIDGE_AGENT_PROCESS_NAME[] = "scim-bridge";


/* Helper functions */
void static_initialize ()
{
    setlogmask (LOG_UPTO (LOG_DEBUG));
    closelog ();
    openlog (SCIM_BRIDGE_AGENT_PROCESS_NAME, LOG_NDELAY, LOG_DAEMON);

    initialized = true;
}


static void vxfprintf (FILE *stream, const char *format, va_list ap)
{
    const size_t format_len = strlen (format);
    char *new_format = static_cast <char*> (alloca (sizeof (char) * (format_len + 1)));
    strcpy (new_format, format);
    new_format[format_len] = '\0';
    vfprintf (stream, new_format, ap);
}


static void vxfprintfln (FILE *stream, const char *format, va_list ap)
{
    const size_t format_len = strlen (format);
    char *new_format = static_cast <char*> (alloca (sizeof (char) * (format_len + 2)));
    strcpy (new_format, format);
    new_format[format_len] = '\n';
    new_format[format_len + 1] = '\0';
    vfprintf (stream, new_format, ap);
}


static void vxsyslog (int priority, const char *format, va_list ap)
{
    if (!initialized) static_initialize ();
    
    char *expanded_format;
    vasprintf (&expanded_format, format, ap);
    syslog (priority, "%s", expanded_format);
    free (expanded_format);
}


/* Implementations */
void scim_bridge_pdebugln (scim_bridge_debug_level_t level, const char* format,...)
{
    if ((10 - level) <= scim_bridge_debug_get_level ()) {
        va_list ap;
        va_start (ap, format);
        vxfprintfln (stdout, format, ap);
        va_end (ap);
    }
}


void scim_bridge_pdebug (scim_bridge_debug_level_t level, const char* format,...)
{
    if ((10 - level) <= scim_bridge_debug_get_level ()) {
        va_list ap;
        va_start (ap, format);
        vxfprintf (stdout, format, ap);
        va_end (ap);
    }
}


void scim_bridge_perrorln (const char* format,...)
{
    va_list ap;
    va_start (ap, format);
    vxfprintfln (stderr, format, ap);
    va_end (ap);
    va_start (ap, format);
    vxsyslog (LOG_ERR, format, ap);
    va_end (ap);
}


void scim_bridge_perror (const char* format,...)
{
    va_list ap;
    va_start (ap, format);
    vxfprintf (stderr, format, ap);
    va_end (ap);
    va_start (ap, format);
    vxsyslog (LOG_ERR, format, ap);
    va_end (ap);
}


void scim_bridge_println (const char* format,...)
{
    va_list ap;
    va_start (ap, format);
    vxfprintfln (stdout, format, ap);
    va_end (ap);
    va_start (ap, format);
    vxsyslog (LOG_INFO, format, ap);
    va_end (ap);
}


void scim_bridge_print (const char* format,...)
{
    va_list ap;
    va_start (ap, format);
    vxfprintf (stdout, format, ap);
    va_end (ap);
    va_start (ap, format);
    vxsyslog (LOG_INFO, format, ap);
    va_end (ap);
}
