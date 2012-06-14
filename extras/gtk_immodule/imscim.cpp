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
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <gtk/gtkimmodule.h>
#include <gtk/gtk.h>
#include "gtkimcontextscim.h"
#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static const GtkIMContextInfo scim_info = { 
  "scim-orig",			/* ID */
  "SCIM Input Method (orig)",		/* Human readable name */
  "scim",			/* Translation domain */
  SCIM_LOCALEDIR,		/* Dir for bindtextdomain (not strictly needed for "gtk+") */
  ""			/* Languages for which this module is the default */
};

static const GtkIMContextInfo *info_list[] = {
  &scim_info
};

void
im_module_init (GTypeModule *type_module)
{
  gtk_im_context_scim_register_type (type_module);
}

void 
im_module_exit (void)
{
  gtk_im_context_scim_shutdown ();
}

void 
im_module_list (const GtkIMContextInfo ***contexts,
		int                      *n_contexts)
{
  *contexts = info_list;
  *n_contexts = G_N_ELEMENTS (info_list);
}

GtkIMContext *
im_module_create (const gchar *context_id)
{
  if (strcmp (context_id, "scim-orig") == 0)
    return gtk_im_context_scim_new ();
  else
    return NULL;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */


