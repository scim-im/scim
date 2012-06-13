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
 * @brief This is the header for the protected interfaces of the clients.
 */

#ifndef SCIMBRIDGECLIENTPROTECTED_H_
#define SCIMBRIDGECLIENTPROTECTED_H_

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * The connection is established with the agent.
     */
    void scim_bridge_client_messenger_opened ();

    /**
     * The connection with the agent is closed.
     */
    void scim_bridge_client_messenger_closed ();

#ifdef __cplusplus
}
#endif
#endif                                            /*SCIMBRIDGECLIENTPROTECTED_H_*/
