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
 * @brief This header describes about the data type of attributes.
 *
 * @mainpage
 *
 * ScimBridge is a wrapper library that allows the apps
 * which use SCIM as a input method to become binary independent with SCIM.\n
 * This will help the maintainers of SCIM, by simplify the complicated dependency around SCIM, libstdc++, and GUI libraries.
 *
 * <li><a href="../../index.html">The Index of the Documents</a></li>
 * <ul>
 *  <li><a href="../../user/index.html">User Manual</a></li>
 *  <li><a href="../../developer/index.html">Developer Manual</a></li>
 * </ul>
 */
#ifndef SCIMBRIDGEAGENT_H_
#define SCIMBRIDGEAGENT_H_

#include "scim-bridge.h"

/**
 * The public interface of the agent.
 */
class ScimBridgeAgent
{

    public:

        /**
         * Allocate an agent.
         *
         * @return A new agent.
         */
        static ScimBridgeAgent *alloc ();

        /**
         * Destructor.
         */
        virtual ~ScimBridgeAgent () {}

        /**
         * Set if it exits automatically when there is no client at all.
         *
         * @param enabled Give it true if you want to disable this feature.
         */
        virtual void set_noexit_enabled (bool enabled) = 0;

        /**
         * Set if the process daemonize itself when launch () is called.
         *
         * @param enabled Give it true if you want to disable this feature.
         */
        virtual void set_standalone_enabled (bool enabled) = 0;

        /**
         * Launch the main loop of the agent.
         * This funtion blocks until it quits.
         */
        virtual retval_t launch () = 0;

    protected:

        /**
         * Constructor.
         */
        ScimBridgeAgent () {}

};
#endif                                            /*SCIMBRIDGEAGENT_H_*/
