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
 * @brief This header describes about the information of the display.
 */
#ifndef SCIMBRIDGEDISPLAY_H_
#define SCIMBRIDGEDISPLAY_H_

#include "scim-bridge.h"

/**
 * The data type for display informations.
 */
typedef struct _ScimBridgeDisplay ScimBridgeDisplay;

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Allocate a new display.
     *
     * @return The new display.
     */
    ScimBridgeDisplay *scim_bridge_alloc_display ();

    /**
     * Free a display.
     *
     * @param display The display to free.
     */
    void scim_bridge_free_display (ScimBridgeDisplay *display);

    /**
     * Copy a display into another.
     *
     * @param dst The destination to copy in.
     * @param src The source display.
     */
    void scim_bridge_copy_display (ScimBridgeDisplay *dst, const ScimBridgeDisplay *src);

    /**
     * See the two display are the same or not.
     *
     * @param dst A display.
     * @param src Another display.
     * @return true if the two displays are the same.
     */
    boolean scim_bridge_display_equals (const ScimBridgeDisplay *dst, const ScimBridgeDisplay *src);

    /**
     * Fetch the current display infomation.
     *
     * @param display The destination to write the data in.
     */
    retval_t scim_bridge_display_fetch_current (ScimBridgeDisplay *display);

    /**
     * Get the name of a display.\n
     * A display name is usually the same as the environmental variable "DISPLAY".\n
     * When you are not in X, the value is undefined for now...
     *
     * @param display The display.
     * @return The display name.
     */
    const char *scim_bridge_display_get_name (const ScimBridgeDisplay *display);

    /**
     * Set the name of a display.
     *
     * @param display The display.
     * @param name The display display_name.
     */
    void scim_bridge_display_set_name (ScimBridgeDisplay *display, const char *display_name);

    /**
     * Get the display number of a display.\n
     * A display number is usually grabbed from the environmental variable "DISPLAY".\n
     * For example, this function returns "0" if "DISPLAY=localhost:0.1".
     *
     * @param display The display.
     * @return The display number.
     */
    int scim_bridge_display_get_display_number (const ScimBridgeDisplay *display);

    /**
     * set the display number of a display.
     *
     * @param display The display.
     * @param display_number The display number.
     */
    void scim_bridge_display_set_display_number (ScimBridgeDisplay *display, int display_number);

    /**
     * Get the screen number of a display.\n
     * A screen number is usually grabbed from the environmental variable "DISPLAY".\n
     * For example, this function returns "1" if "DISPLAY=localhost:0.1".
     *
     * @param display The display.
     * @return The screen number.
     */
    int scim_bridge_display_get_screen_number (const ScimBridgeDisplay *display);

    /**
     * Set the screen number of a display.
     *
     * @param display The display.
     * @param The screen number of a display.
     */
    void scim_bridge_display_set_screen_number (ScimBridgeDisplay *display, int screen_number);

#ifdef __cplusplus
}
#endif
#endif                                            /*SCIMBRIDGEDISPLAY_H_*/
