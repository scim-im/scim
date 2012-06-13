/*
 * SCIM Bridge
 *
 * Copyright (c) 2006 Ryo Dairiki <ryo-dairiki@users.sourceforge.net>
 * Copyright (C) 2009, Intel Corporation.
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
 * @author Raymond liu <raymond.liu@intel.com>
 * @brief This header describes about the clutter client for scim-bridge.
 */

#ifndef SCIMBRIDGECLIENTCLUTTER_H_
#define SCIMBRIDGECLIENTCLUTTER_H_

#include "scim-bridge.h"
#include "scim-bridge-key-event.h"
#include "scim-bridge-imcontext.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Initialize clutter client for SCIMBridge.
     */
    void scim_bridge_client_clutter_initialize ();

    /**
     * Finalize clutter client for SCIMBridge.
     */
    void scim_bridge_client_clutter_finalize ();

#ifdef __cplusplus
}
#endif
#endif                                            /*SCIMBRIDGECLIENTCLUTTER_H_*/
