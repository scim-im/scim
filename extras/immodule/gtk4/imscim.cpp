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
 * "gtk-im-module" extension point.  A dynamically loaded GIO module is looked
 * up by fixed symbol name -- g_io_module_load()/_unload()/_query() -- and is
 * skipped outright when they are missing.  The filename-derived spelling
 * (g_io_<name>_load) applies only to modules built for static linking, so it
 * must not be used here.
 */

#include <gtk/gtk.h>
#include <gtk/gtkimmodule.h>
#include "scimgtkimcontext.h"

extern "C" {

void
g_io_module_load (GIOModule *io_module)
{
    static gboolean initialized = FALSE;

    if (initialized)
        return;

    GType type = gtk_im_context_scim_register_type (G_TYPE_MODULE (io_module));

    // Priority has to stay below GTK's own contexts. With no GTK_IM_MODULE and
    // no gtk-im-module setting, GTK walks this extension point in priority
    // order and takes the first entry whose name matches the display backend --
    // and an unrecognised name like "scim" matches every backend. Registering at
    // 100 tied us with GTK's "wayland" context and sorted us ahead of it, so
    // every GTK4 application silently used SCIM instead of text-input-v3, which
    // on GNOME cut IBus (and therefore our own ibus.so engine) out of the loop
    // entirely. At 10 the Wayland context wins where it applies, we remain the
    // default where GTK offers no backend context of its own, and an explicit
    // GTK_IM_MODULE=scim still selects us by name regardless.
    g_io_extension_point_implement (GTK_IM_MODULE_EXTENSION_POINT_NAME,
                                    type,
                                    "scim",
                                    10);

    g_type_module_use (G_TYPE_MODULE (io_module));

    initialized = TRUE;
}

void
g_io_module_unload (GIOModule *io_module)
{
    (void) io_module;
    gtk_im_context_scim_shutdown ();
}

char **
g_io_module_query (void)
{
    char *eps[] = {
        (char *) GTK_IM_MODULE_EXTENSION_POINT_NAME,
        NULL
    };
    return g_strdupv (eps);
}

} /* extern "C" */
