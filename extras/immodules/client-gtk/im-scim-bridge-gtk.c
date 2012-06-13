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
#include "scim-bridge-client-gtk.h"
#include "scim-bridge-client-imcontext-gtk.h"

static const GtkIMContextInfo scim_bridge_info =
{
    /* ID */
    "scim-bridge",
    /* Human readable name */
    "SCIM Bridge Input Method",
    /* Translation domain */
    "",
    /* Dir for bindtextdomain (not strictly needed for "gtk+") */
    "",
    /* Languages for which this module is the default */
    ""
};

static const GtkIMContextInfo *info_list[] =
{
    &scim_bridge_info
};

/* Public functions */
void im_module_init (GTypeModule *type_module);
void im_module_exit (void);
void im_module_list (const GtkIMContextInfo ***contexts, int *context_count);
GtkIMContext *im_module_create (const gchar *context_id);


/* Implementations */
void im_module_init (GTypeModule *type_module)
{
    scim_bridge_client_imcontext_register_type (type_module);
    scim_bridge_client_gtk_initialize ();
    
    static boolean first_time = TRUE;
    if (first_time) atexit (scim_bridge_client_gtk_finalize);
    first_time = FALSE;
}


void im_module_exit ()
{
    scim_bridge_client_gtk_finalize ();
}


void im_module_list (const GtkIMContextInfo ***contexts, int *context_count)
{
    *contexts = info_list;
    *context_count = G_N_ELEMENTS (info_list);
}


GtkIMContext *im_module_create (const gchar *context_id)
{
    if (strcmp (context_id, "scim-bridge") == 0) {
        return scim_bridge_client_imcontext_new ();
    } else {
        return NULL;
    }
}
