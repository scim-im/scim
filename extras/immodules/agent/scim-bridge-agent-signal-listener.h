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
 * @brief This is the header file for ScimBridgeAgentSignalListener.
 */

#ifndef SCIMBRIDGEAGENTSIGNALLISTENER_H_
#define SCIMBRIDGEAGENTSIGNALLISTENER_H_

#include "scim-bridge.h"
#include "scim-bridge-agent-socket-client.h"

class ScimBridgeAgentProtected;

/**
 * The class of signal listeners, which listen to the signals and quit the agent properly.
 */
class ScimBridgeAgentSignalListener: public ScimBridgeAgentSocketClient
{

    public:

        /**
         * Allocate an signal listener. You cannot allocate more than one signal listener.
         *
         * @param agent The agent.
         * @return A new signal listener, or NULL if it failed to initialize it.
         */
        static ScimBridgeAgentSignalListener *alloc (ScimBridgeAgentProtected *agent);


        /**
         * Destructor.
         */
        virtual ~ScimBridgeAgentSignalListener () {}


    protected:

        /**
         * Constructor.
         */
        ScimBridgeAgentSignalListener () {}

};
#endif                                            /*SCIMBRIDGEAGENTSIGNALLISTENER_H_*/
