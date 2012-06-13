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
 * @brief This is the header for clutter imcontext of scim-bridge.
 */


#ifndef SCIMBRIDGECLIENTIMCONTEXTCLUTTER_H_
#define SCIMBRIDGECLIENTIMCONTEXTCLUTTER_H_

#include <clutter/clutter.h>
#include <clutter-imcontext/clutter-imcontext.h>
#include <clutter-imcontext/clutter-immodule.h>

#include "scim-bridge.h"
#include "scim-bridge-client-imcontext.h"

#define CLUTTER_TYPE_SCIM_CLIENT_IMCONTEXT (scim_bridge_client_imcontext_get_type ())
#define SCIM_BRIDGE_CLIENT_IMCONTEXT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), CLUTTER_TYPE_SCIM_CLIENT_IMCONTEXT, ScimBridgeClientIMContext))
#define SCIM_BRIDGE_CLIENT_IMCONTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), CLUTTER_TYPE_SCIM_CLIENT_IMCONTEXT, ScimBridgeClientIMContextClass))
#define IS_SCIM_BRIDGE_CLIENT_IMCONTEXT(class) (G_TYPE_CHECK_INSTANCE_CAST ((obj), CLUTTER_TYPE_SCIM_CLIENT_IMCONTEXT))
#define IS_SCIM_BRIDGE_CLIENT_IMCONTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), CLUTTER_TYPE_SCIM_CLIENT_IMCONTEXT))
#define SCIM_BRIDGE_CLIENT_IMCONTEXT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), CLUTTER_TYPE_SCIM_CLIENT_IMCONTEXT, ScimBridgeClientIMContextClass))

struct _ScimBridgeClientIMContextClass
{
    ClutterIMContextClass parent_class;
};

/**
 * IMContext class for clutter client.
 */
typedef struct _ScimBridgeClientIMContextClass ScimBridgeClientIMContextClass;

/**
 * Initialize IMContext class itself.
 */
void scim_bridge_client_imcontext_static_initialize ();

/**
 * Finalize IMContext class itself.
 */
void scim_bridge_client_imcontext_static_finalize ();

/**
 * The connection with the agent is opened.
 */
void scim_bridge_client_imcontext_connection_opened ();

/**
 * The connection with the agent is closed.
 */
void scim_bridge_client_imcontext_connection_closed ();

/**
 * Allocate an IMContext.
 *
 * @return new IMContext.
 */
ClutterIMContext *scim_bridge_client_imcontext_new ();

/**
 * Get the type value of IMContexts.
 *
 * @return The type value of IMContexts.
 */
GType scim_bridge_client_imcontext_get_type ();

/**
 * Register the type value for IMContexts.
 *
 * @param type_module The type module.
 */
void scim_bridge_client_imcontext_register_type (GTypeModule *type_module);

#endif                                            /*SCIMBRIDGECLIENTIMCONTEXTCLUTTER_H_*/
