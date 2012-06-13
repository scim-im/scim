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
 * @brief This header describes abut path related information.
 */
#ifndef SCIMBRIDGEPATH_H_
#define SCIMBRIDGEPATH_H_

#include "scim-bridge.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Get the socket path for messengers.
     *
     * @return The socket path.
     */
    const char *scim_bridge_path_get_socket ();

    /**
     * Get the path for the lockfile of the agent.
     *
     * @return The path for the lockfile.
     */
    const char *scim_bridge_path_get_lockfile ();

    /**
     * Get the path for the binary of the agent.
     *
     * @return The path for the binary of the agent.
     */
    const char *scim_bridge_path_get_agent ();

#ifdef __cplusplus
}
#endif
#endif                                            /*SCIMBRIDGEPATH_H_*/
