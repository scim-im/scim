/* Smart Common Input Method
 *
 * Copyright (c) 2002-2005 James Su <suzhe@tsinghua.org.cn>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

/*
 * GTK4 loads input-method modules as GIO modules that implement the
 * "gtk-im-module" extension point.  GLib derives the entry-point names from the
 * module filename: it drops an optional "lib" prefix and the extension and maps
 * '-' to '_', so both "im-scim.so" and "libim-scim.so" resolve to
 * g_io_im_scim_load()/g_io_im_scim_unload()/g_io_im_scim_query().
 */

#include <gtk/gtk.h>
#include <gtk/gtkimmodule.h>
#include "scimgtkimcontext.h"

extern "C" {

void
g_io_im_scim_load (GIOModule *io_module)
{
    static gboolean initialized = FALSE;

    if (initialized)
        return;

    GType type = gtk_im_context_scim_register_type (G_TYPE_MODULE (io_module));

    g_io_extension_point_implement (GTK_IM_MODULE_EXTENSION_POINT_NAME,
                                    type,
                                    "scim",
                                    100);

    g_type_module_use (G_TYPE_MODULE (io_module));

    initialized = TRUE;
}

void
g_io_im_scim_unload (GIOModule *io_module)
{
    (void) io_module;
    gtk_im_context_scim_shutdown ();
}

char **
g_io_im_scim_query (void)
{
    char *eps[] = {
        (char *) GTK_IM_MODULE_EXTENSION_POINT_NAME,
        NULL
    };
    return g_strdupv (eps);
}

} /* extern "C" */
