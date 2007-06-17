/* scimtrayicon.h
 * converted from eggtrayicon.h in libegg.
 *
 * Copyright (C) 2002 Anders Carlsson <andersca@gnu.org>
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

#ifndef __SCIM_TRAY_ICON_H__
#define __SCIM_TRAY_ICON_H__

#include <gtk/gtkplug.h>
#include <gdk/gdkx.h>

G_BEGIN_DECLS

#define SCIM_TYPE_TRAY_ICON            (scim_tray_icon_get_type ())
#define SCIM_TRAY_ICON(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SCIM_TYPE_TRAY_ICON, ScimTrayIcon))
#define SCIM_TRAY_ICON_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SCIM_TYPE_TRAY_ICON, ScimTrayIconClass))
#define SCIM_IS_TRAY_ICON(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SCIM_TYPE_TRAY_ICON))
#define SCIM_IS_TRAY_ICON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SCIM_TYPE_TRAY_ICON))
#define SCIM_TRAY_ICON_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SCIM_TYPE_TRAY_ICON, ScimTrayIconClass))
    
typedef struct _ScimTrayIcon       ScimTrayIcon;
typedef struct _ScimTrayIconClass  ScimTrayIconClass;

struct _ScimTrayIcon
{
    GtkPlug parent_instance;

    guint stamp;
  
    Atom selection_atom;
    Atom manager_atom;
    Atom system_tray_opcode_atom;
    Atom orientation_atom;
    Window manager_window;

    GtkOrientation orientation;
};

struct _ScimTrayIconClass
{
    GtkPlugClass parent_class;
};

GType         scim_tray_icon_get_type       (void);

ScimTrayIcon *scim_tray_icon_new_for_screen (GdkScreen    *screen,
                                             const gchar  *name);

ScimTrayIcon *scim_tray_icon_new            (const gchar  *name);

guint         scim_tray_icon_send_message   (ScimTrayIcon *icon,
                                             gint          timeout,
                                             const char   *message,
                                             gint          len);

void          scim_tray_icon_cancel_message (ScimTrayIcon *icon,
                                             guint         id);

GtkOrientation scim_tray_icon_get_orientation (ScimTrayIcon *icon);

G_END_DECLS

#endif /* __SCIM_TRAY_ICON_H__ */

/*
vi:ts=4:nowrap:ai:expandtab
*/
