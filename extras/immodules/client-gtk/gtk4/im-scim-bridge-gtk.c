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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <gtk/gtkimmodule.h>

#include "scim-bridge.h"
#include "scim-bridge-client.h"
#include "../scim-bridge-client-gtk.h"
#include "scim-bridge-client-imcontext-gtk.h"

/* Implementations */
void g_io_im_scim_load (GIOModule *io_module)
{
    static boolean initialized = FALSE;

    if (initialized) {
        return;
    }

    scim_bridge_client_imcontext_register_type (io_module);
    GIOExtension *ext = g_io_extension_point_implement (
        GTK_IM_MODULE_EXTENSION_POINT_NAME,
        GTK_TYPE_SCIM_CLIENT_IMCONTEXT,
        "scim",
        100);
    scim_bridge_client_gtk_initialize ();

    atexit (scim_bridge_client_gtk_finalize);

    g_type_module_use(G_TYPE_MODULE(io_module));

    initialized = TRUE;
}


void g_io_im_scim_unload (GIOModule *io_module)
{
    g_type_module_unuse(G_TYPE_MODULE(io_module));
    scim_bridge_client_gtk_finalize ();
}
