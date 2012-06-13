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
 * @brief This is the header file for ScimBridgeAgentInterruptionListener.
 */

#ifndef SCIMBRIDGEAGENTINTERRUPTIONLISTENER_H_
#define SCIMBRIDGEAGENTINTERRUPTIONLISTENER_H_

#include "scim-bridge.h"
#include "scim-bridge-agent-socket-client.h"

class ScimBridgeAgentProtected;

/**
 * The class of interruption listeners, which listen to the interruptions.
 */
class ScimBridgeAgentInterruptionListener: public ScimBridgeAgentSocketClient
{

    public:

        /**
         * Allocate an interruption listener.
         *
         * @return A new interruption listener, or NULL if it failed to initialize it.
         */
        static ScimBridgeAgentInterruptionListener *alloc ();

        /**
         * Destructor.
         */
        virtual ~ScimBridgeAgentInterruptionListener () {}

        /**
         * See if interruptions have occurred.
         *
         * @return true if interruptions have occurred.
         */
        virtual bool is_interrupted () const = 0;

        /**
         * Make an interruption.
         */
        virtual void interrupt () = 0;

        /**
         * Clear the interruption.
         */
        virtual void clear_interruption () = 0;

    protected:

        /**
         * Constructor.
         */
        ScimBridgeAgentInterruptionListener () {}

};
#endif                                            /*SCIMBRIDGEAGENTINTERRUPTIONLISTENER_H_*/
