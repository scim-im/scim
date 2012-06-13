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
 * @brief This header describes about the gtk client for scim-bridge.
 */

#ifndef SCIMBRIDGECLIENTGTK_H_
#define SCIMBRIDGECLIENTGTK_H_

#include "scim-bridge.h"
#include "scim-bridge-key-event.h"
#include "scim-bridge-imcontext.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Initialize gtk client for SCIMBridge.
     */
    void scim_bridge_client_gtk_initialize ();

    /**
     * Finalize gtk client for SCIMBridge.
     */
    void scim_bridge_client_gtk_finalize ();

#ifdef __cplusplus
}
#endif
#endif                                            /*SCIMBRIDGECLIENTGTK_H_*/
