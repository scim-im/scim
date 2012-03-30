/** @file gtkimcontextscim.h
 *  @brief immodule for GTK2/GTK3.
 */

/* 
 * Smart Common Input Method
 * 
 * Copyright (c) 2002-2005 James Su <suzhe@tsinghua.org.cn>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 *
 * $Id: gtkimcontextscim.h,v 1.12 2005/06/27 15:31:49 suzhe Exp $
 */

#ifndef __GTK_IM_CONTEXT_SCIM_H__
#define __GTK_IM_CONTEXT_SCIM_H__

#include <gtk/gtk.h>

typedef struct _GtkIMContextSCIM       GtkIMContextSCIM;
typedef struct _GtkIMContextSCIMClass  GtkIMContextSCIMClass;
typedef struct _GtkIMContextSCIMImpl   GtkIMContextSCIMImpl;

struct _GtkIMContextSCIM
{
  GtkIMContext object;
  GtkIMContext *slave;
  bool slave_preedit;

  GtkIMContextSCIMImpl *impl;

  int id; /* Input Context id*/
};

struct _GtkIMContextSCIMClass
{
  GtkIMContextClass parent_class;
};

GtkIMContext *gtk_im_context_scim_new (void);

void gtk_im_context_scim_register_type (GTypeModule *type_module);
void gtk_im_context_scim_shutdown (void);

#endif /* __GTK_IM_CONTEXT_SCIM_H__ */
