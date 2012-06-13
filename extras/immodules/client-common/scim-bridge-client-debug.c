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

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "scim-bridge-debug.h"
#include "scim-bridge-string.h"

/* Static variables */
static scim_bridge_debug_level_t debug_level = -1;


/* Private function */
static void static_initialize ()
{
    int debug_level_int;
    char *debug_level_str = getenv ("SCIM_BRIDGE_DEBUG_LEVEL");
    if (debug_level_str == NULL || scim_bridge_string_to_int (&debug_level_int, debug_level_str)) {
        debug_level = 0;
    } else {
        debug_level = debug_level_int;
        if (debug_level > 10) debug_level = 10;
    }
}


/* Implementations */
scim_bridge_debug_level_t scim_bridge_debug_get_level ()
{
    if (debug_level < 0) static_initialize ();
    return debug_level;
}
