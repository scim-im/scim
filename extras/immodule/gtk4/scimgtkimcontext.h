/** @file scimgtkimcontext.h
 *  @brief native SCIM immodule for GTK4.
 *
 *  This is the GTK4 counterpart of ../gtkimcontextscim.h.  GTK4's input-method
 *  and event APIs differ enough from GTK2/GTK3 that the implementation lives in
 *  its own translation unit rather than being #if-branched into the shared
 *  GTK2/3 source.
 */

/*
 * Smart Common Input Method
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 */

#pragma once

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

/* Returns the registered GType so the GIO module can wire it to the
 * gtk-im-module extension point. */
GType gtk_im_context_scim_register_type (GTypeModule *type_module);
void  gtk_im_context_scim_shutdown (void);
