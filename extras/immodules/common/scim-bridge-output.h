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

/**
 * @file
 * @author Ryo Dairiki <ryo-dairiki@users.sourceforge.net>
 * @brief This header is used for print out error messages.
 */
#ifndef SCIMBRIDGEOUTPUT_H_
#define SCIMBRIDGEOUTPUT_H_

#include "scim-bridge.h"
#include "scim-bridge-debug.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Print out a message and give a line feed ('\n').
     * The messages will be shown in the stdout.
     *
     * @param format The message in printf style.
     */
    void scim_bridge_println (const char *format,...);

    /**
     * Print out an error message and give a line feed ('\n').
     * The error messages will be shown in the stderr.
     *
     * @param format The message in printf style.
     */
    void scim_bridge_perrorln (const char *format,...);

    /**
     * Print out an debug message and give a line feed ('\n').
     * The error messages will be shown in the stdout.
     * Note, the message will be ignored if the given debug level is lower than that of the preference.
     * @ ee scim_bridge_debug_get_level ()
     *
     * @param level The debug level.
     * @param format The message in printf style.
     */
    void scim_bridge_pdebugln (scim_bridge_debug_level_t level, const char *format,...);

    /**
     * Print out a message without giving a line feed ('\n').
     * The messages will be shown in the stdout.
     *
     * @param format The message in printf style.
     */
    void scim_bridge_print (const char *format,...);

    /**
     * Print out an error message without giving a line feed ('\n').
     * The error messages will be shown in the stderr.
     *
     * @param format The message in printf style.
     */
    void scim_bridge_perror (const char *format,...);

    /**
     * Print out an debug message without giving a line feed ('\n').
     * The error messages will be shown in the stdout.
     * Note, the message will be ignored if the given debug level is lower than that of the preference.
     * @ ee scim_bridge_debug_get_level ()
     *
     * @param level The debug level.
     * @param format The message in printf style.
     */
    void scim_bridge_pdebug (scim_bridge_debug_level_t level, const char *format,...);

#ifdef __cplusplus
}
#endif
#endif                                            /*SCIMBRIDGEOUTPUT_H_*/
