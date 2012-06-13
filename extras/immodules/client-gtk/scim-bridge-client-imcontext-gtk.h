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
 * @brief This is the header for gtk imcontext of scim-bridge.
 */


#ifndef SCIMBRIDGECLIENTIMCONTEXTGTK_H_
#define SCIMBRIDGECLIENTIMCONTEXTGTK_H_

#include <gtk/gtk.h>
#if GTK_CHECK_VERSION(3, 0, 0)
#else
#include <gtk/gtkimcontext.h>
#endif

#include <gdk/gdk.h>

#include "scim-bridge.h"
#include "scim-bridge-client-imcontext.h"

#define GTK_TYPE_SCIM_CLIENT_IMCONTEXT	(scim_bridge_client_imcontext_get_type ())
#if GTK_CHECK_VERSION(3, 0, 0)
#define SCIM_BRIDGE_CLIENT_IMCONTEXT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_SCIM_CLIENT_IMCONTEXT, ScimBridgeClientIMContext))
#else
#define SCIM_BRIDGE_CLIENT_IMCONTEXT(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_SCIM_CLIENT_IMCONTEXT, ScimBridgeClientIMContext))
#endif
#define SCIM_BRIDGE_CLIENT_IMCONTEXT_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_SCIM_CLIENT_IMCONTEXT, ScimBridgeClientIMContextClass))
#if GTK_CHECK_VERSION(3, 0, 0)
#define IS_SCIM_BRIDGE_CLIENT_IMCONTEXT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_SCIM_CLIENT_IMCONTEXT))
#else
#define IS_SCIM_BRIDGE_CLIENT_IMCONTEXT(class) (GTK_CHECK_TYPE ((obj), GTK_TYPE_SCIM_CLIENT_IMCONTEXT))
#endif
#define IS_SCIM_BRIDGE_CLIENT_IMCONTEXT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_SCIM_CLIENT_IMCONTEXT))
#if GTK_CHECK_VERSION(3, 0, 0)
#define SCIM_BRIDGE_CLIENT_IMCONTEXT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_SCIM_CLIENT_IMCONTEXT, ScimBridgeClientIMContextClass))
#else
#define SCIM_BRIDGE_CLIENT_IMCONTEXT_GET_CLASS(obj) (GTK_CHECK_GET_CLASS ((obj), GTK_TYPE_SCIM_CLIENT_IMCONTEXT, ScimBridgeClientIMContextClass))
#endif

struct _ScimBridgeClientIMContextClass
{
    GtkIMContextClass parent_class;
};

/**
 * IMContext class for gtk client.
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
GtkIMContext *scim_bridge_client_imcontext_new ();

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

#endif                                            /*SCIMBRIDGECLIENTIMCONTEXTGTK_H_*/
