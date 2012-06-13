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

#include <stdint.h>

/**
 * @file
 * @author Ryo Dairiki <ryo-dairiki@users.sourceforge.net>
 * @brief This header must be included by all the source codes which use IMContextes.
 */
#ifndef SCIMBRIDGEIMCONTEXT_H_
#define SCIMBRIDGEIMCONTEXT_H_

enum _scim_bridge_preedit_mode_t
{
    PREEDIT_FLOATING,
    PREEDIT_HANGING,
    PREEDIT_EMBEDDED,
    PREEDIT_ANY,
};

/**
 * The type for the way to show the preedit.
 */
typedef enum _scim_bridge_preedit_mode_t scim_bridge_preedit_mode_t;

/**
 * This is the type for imcontext id.
 * Notice, every imcontext has the unique id.
 */
typedef int scim_bridge_imcontext_id_t;
#endif                                            /*SCIMBRIDGEIMCONTEXT_H_*/
