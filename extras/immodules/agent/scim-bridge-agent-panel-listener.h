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
 * @brief This is the header file for ScimBridgeAgentPanelListener.
 */

#ifndef SCIMBRIDGEAGENTPANELLISTENER_H_
#define SCIMBRIDGEAGENTPANELLISTENER_H_

#include <scim.h>

#include "scim-bridge-agent-panel-listener-protected.h"
#include "scim-bridge-agent-socket-client.h"

#include "scim-bridge-display.h"
#include "scim-bridge-imcontext.h"

class ScimBridgeAgentProtected;

/**
 * The class of panel listeners, which communicate with panel agents.
 */
class ScimBridgeAgentPanelListener: public ScimBridgeAgentSocketClient, public ScimBridgeAgentPanelListenerProtected
{

    public:

        /**
         * Allocate a panel listener.
         *
         * @param config_name The name of the current configuration.
         * @param display The current display.
         * @param agent The agent.
         */
        static ScimBridgeAgentPanelListener *alloc (const scim::String &config_name, const ScimBridgeDisplay *display, ScimBridgeAgentProtected *agent);

        /**
         * Destructor.
         */
        virtual ~ScimBridgeAgentPanelListener () {}

    protected:

        /**
         * Constructor.
         */
        ScimBridgeAgentPanelListener () {}

};
#endif                                            /*SCIMBRIDGEAGENTPANELLISTENER_H_*/
