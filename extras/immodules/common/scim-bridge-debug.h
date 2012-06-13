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
 * @brief This header describes about debug related features.
 */
#ifndef SCIMBRIDGEDEBUG_H_
#define SCIMBRIDGEDEBUG_H_

#include "scim-bridge.h"

/**
 * This is the type of debug level.
 */
typedef int scim_bridge_debug_level_t;

/**
 * The maximum value of the debug level.
 */
static const scim_bridge_debug_level_t SCIM_BRIDGE_MAX_DEBUG_LEVEL = 9;

/**
 * The minimum value of the debug level.
 */
static const scim_bridge_debug_level_t SCIM_BRIDGE_MIN_DEBUG_LEVEL = 0;

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Get the debug level.
     *
     * @return The debug level.
     */
    scim_bridge_debug_level_t scim_bridge_debug_get_level ();

#ifdef __cplusplus
}
#endif
#endif                                            /*SCIMBRIDGEDEBUG_H_*/
