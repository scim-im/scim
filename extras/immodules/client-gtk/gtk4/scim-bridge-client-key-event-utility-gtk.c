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

#include <sys/time.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#ifdef GDK_WINDOWING_X11 
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <gdk/x11/gdkx.h>
#endif

#include "scim-bridge-client-key-event-utility-gtk.h"
#include "scim-bridge-key-event.h"

/* Implementations */
void scim_bridge_key_event_bridge_to_gdk (GdkEvent *gdk_key_event,
                                          GtkWidget *client_widget,
                                          const ScimBridgeKeyEvent *key_event)
{
//    gdk_key_event->state = 0;
//    if (scim_bridge_key_event_is_shift_down (key_event)) gdk_key_event->state |= GDK_SHIFT_MASK;
//    if (scim_bridge_key_event_is_caps_lock_down (key_event)) gdk_key_event->state |= GDK_LOCK_MASK;
//    if (scim_bridge_key_event_is_control_down (key_event)) gdk_key_event->state |= GDK_CONTROL_MASK;
//    if (scim_bridge_key_event_is_alt_down (key_event)) gdk_key_event->state |= GDK_MOD1_MASK;
//    if (scim_bridge_key_event_is_num_lock_down (key_event)) gdk_key_event->state |= GDK_MOD2_MASK;
//
//    if (scim_bridge_key_event_is_pressed (key_event)) {
//        gdk_key_event->type = GDK_KEY_PRESS;
//    } else {
//        gdk_key_event->type = GDK_KEY_RELEASE;
//        gdk_key_event->state |= GDK_RELEASE_MASK;
//    }
//
//    gdk_key_event->widget = client_widget;
//
//    struct timeval current_time;
//    gettimeofday (&current_time, NULL);
//
//    gdk_key_event->time = current_time.tv_sec * 1000 + current_time.tv_usec / 1000;
//    gdk_key_event->keyval = scim_bridge_key_event_get_code (key_event);
//    gdk_key_event->length = 0;
//    gdk_key_event->string = 0;
//
//    GdkKeymap *key_map = get_gdk_keymap (gdk_key_event->widget);
//
//    GdkKeymapKey *keys;
//    gint n_keys;
//
//    if (gdk_keymap_get_entries_for_keyval (key_map, gdk_key_event->keyval, &keys, &n_keys)) {
//        gdk_key_event->hardware_keycode = keys[0].keycode;
//        gdk_key_event->group = keys [0].group;
//    } else {
//        gdk_key_event->hardware_keycode = 0;
//        gdk_key_event->group = 0;
//    }
}


void scim_bridge_key_event_gdk_to_bridge (ScimBridgeKeyEvent *bridge_key_event,
                                          GtkWidget *widget,
                                          const GdkEvent *key_event)
{
    // Use Key Symbole provided by gtk.
    guint keyval = gdk_key_event_get_keyval((GdkEvent *) key_event);
    GdkModifierType state = gdk_event_get_modifier_state((GdkEvent *) key_event);
    scim_bridge_key_event_set_code (bridge_key_event, (scim_bridge_key_code_t) keyval);

    scim_bridge_key_event_clear_modifiers (bridge_key_event);
    if (state & GDK_SHIFT_MASK || keyval == GDK_KEY_Shift_L || keyval == GDK_KEY_Shift_R)
        scim_bridge_key_event_set_shift_down (bridge_key_event, TRUE);
    if (state & GDK_LOCK_MASK || keyval == GDK_KEY_Caps_Lock)
        scim_bridge_key_event_set_caps_lock_down (bridge_key_event, TRUE);
    if (state & GDK_CONTROL_MASK || keyval == GDK_KEY_Control_L || keyval == GDK_KEY_Control_R)
        scim_bridge_key_event_set_control_down (bridge_key_event, TRUE);
    if (state & GDK_ALT_MASK || keyval == GDK_KEY_Alt_L || keyval == GDK_KEY_Alt_R)
        scim_bridge_key_event_set_alt_down (bridge_key_event, TRUE);
    if (gdk_device_get_num_lock_state(gdk_event_get_device((GdkEvent *) key_event)))
        scim_bridge_key_event_set_num_lock_down (bridge_key_event, TRUE);


    if (gdk_event_get_event_type((GdkEvent *) key_event) != GDK_KEY_RELEASE) {
        scim_bridge_key_event_set_pressed (bridge_key_event, TRUE);
    } else {
        scim_bridge_key_event_set_pressed (bridge_key_event, FALSE);
    }
    
#ifdef GDK_WINDOWING_X11
    GdkX11Display *display = NULL;

    if (widget != NULL) {
        display = GDK_X11_DISPLAY (gtk_widget_get_display(widget));
    } else {
        display = GDK_X11_DISPLAY (gdk_display_get_default ());
    }
    
    if (scim_bridge_key_event_get_code (bridge_key_event) == SCIM_BRIDGE_KEY_CODE_backslash) {
        boolean kana_ro = FALSE;
        int keysym_size = 0;
        KeySym *keysyms = XGetKeyboardMapping (
            gdk_x11_display_get_xdisplay(display),
            gdk_key_event_get_keycode((GdkEvent *) key_event),
            1,
            &keysym_size);
        if (keysyms != NULL) {
            kana_ro = (keysyms[0] == XK_backslash && keysyms[1] == XK_underscore);
            XFree (keysyms);
        }
        scim_bridge_key_event_set_quirk_enabled (bridge_key_event, SCIM_BRIDGE_KEY_QUIRK_KANA_RO, kana_ro);
    }
#endif
}
