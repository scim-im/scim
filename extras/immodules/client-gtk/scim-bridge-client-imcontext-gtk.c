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

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <gtk/gtk.h>

#if GTK_CHECK_VERSION(3, 0, 0)
#include <gdk/gdk.h>
#else
#include <gdk/gdkkeysyms.h>
#endif

#if GTK_CHECK_VERSION(3, 0, 0)
#else
#include <gtk/gtkimcontext.h>
#endif

#include "scim-bridge-attribute.h"
#include "scim-bridge-client.h"
#include "scim-bridge-client-imcontext-gtk.h"
#include "scim-bridge-client-key-event-utility-gtk.h"
#include "scim-bridge-imcontext.h"
#include "scim-bridge-messenger.h"
#include "scim-bridge-output.h"
#include "scim-bridge-string.h"

#define SEND_EVENT_MASK 0x02

/* Typedef */
struct _ScimBridgeClientIMContext
{
    GtkIMContext parent;
    GtkIMContext *slave;
    boolean slave_preedit;

    scim_bridge_imcontext_id_t id;

    char *preedit_string;
    size_t preedit_string_capacity;
    
    PangoAttrList *preedit_attributes;

    unsigned int preedit_cursor_position;
    boolean preedit_cursor_flicking;
    
    boolean preedit_shown;
    boolean preedit_started;

    char *commit_string;
    size_t commit_string_capacity;
    
    boolean enabled;

    GdkWindow *client_window;

    int cursor_x;
    int cursor_y;
    int window_x;
    int window_y;
};


/* Private variables */
static GdkColor preedit_normal_background;
static GdkColor preedit_normal_foreground;
static GdkColor preedit_active_background;
static GdkColor preedit_active_foreground;

static GType class_type = 0;
static GObjectClass *root_klass = NULL;

static ScimBridgeClientIMContext *focused_imcontext = NULL;
static GtkWidget *focused_widget = NULL;

static guint key_snooper_id = 0;
static boolean key_snooper_used = FALSE;

/* Class functions */
static void scim_bridge_client_imcontext_class_initialize (ScimBridgeClientIMContextClass *klass, gpointer *klass_data);

static void scim_bridge_client_imcontext_initialize (ScimBridgeClientIMContext *context, ScimBridgeClientIMContextClass *klass);
static void scim_bridge_client_imcontext_finalize (GObject *object);

static gboolean scim_bridge_client_imcontext_filter_key_event (GtkIMContext *context, GdkEventKey *event);
static void scim_bridge_client_imcontext_reset (GtkIMContext *context);
static void scim_bridge_client_imcontext_get_preedit_string (GtkIMContext *context, gchar **str, PangoAttrList **attrs, gint *cursor_pos);
static void scim_bridge_client_imcontext_set_preedit_enabled (GtkIMContext *context, gboolean enabled);

static void scim_bridge_client_imcontext_set_client_window (GtkIMContext *context, GdkWindow *window);
static void scim_bridge_client_imcontext_focus_in (GtkIMContext *context);
static void scim_bridge_client_imcontext_focus_out (GtkIMContext *context);
static void scim_bridge_client_imcontext_set_cursor_location (GtkIMContext *context, GdkRectangle *area);

/* slave callbacks */
static void gtk_im_slave_commit_cb (GtkIMContext *context, const char *str, ScimBridgeClientIMContext *imcontext);
static void gtk_im_slave_preedit_changed_cb (GtkIMContext *context, ScimBridgeClientIMContext *imcontext);
static void gtk_im_slave_preedit_start_cb (GtkIMContext *context, ScimBridgeClientIMContext *imcontext);
static void gtk_im_slave_preedit_end_cb (GtkIMContext *context, ScimBridgeClientIMContext *imcontext);

static retval_t filter_key_event (ScimBridgeClientIMContext *imcontext, GdkEventKey *event, boolean *consumed)
{
    scim_bridge_pdebugln (5, "filter_key_event ()");
    
    if (focused_imcontext != imcontext) scim_bridge_client_imcontext_focus_in (GTK_IM_CONTEXT (imcontext));
    focused_widget = gtk_get_event_widget ((GdkEvent*) event);
    
    if (scim_bridge_client_is_messenger_opened ()) {
        ScimBridgeKeyEvent *bridge_key_event = scim_bridge_alloc_key_event ();
        scim_bridge_key_event_gdk_to_bridge (bridge_key_event, imcontext->client_window, event);

        *consumed = FALSE;
        const retval_t retval_error = scim_bridge_client_handle_key_event (imcontext, bridge_key_event, consumed);
        scim_bridge_free_key_event (bridge_key_event);

        if (retval_error) {
            scim_bridge_perrorln ("An IOException at filter_key_event ()");
        } else {
            return RETVAL_SUCCEEDED;
        }
    }

    return RETVAL_FAILED;
}


static retval_t set_cursor_location (ScimBridgeClientIMContext *imcontext, int window_x, int window_y, int cursor_x, int cursor_y)
{
    scim_bridge_pdebugln (5, "set_cursor_location ()");

    if (imcontext->window_x == window_x && imcontext->window_y == window_y && imcontext->cursor_x == cursor_x && imcontext->cursor_y == cursor_y) {
        return RETVAL_SUCCEEDED;
    } else {
        imcontext->cursor_x = cursor_x;
        imcontext->cursor_y = cursor_y;
        imcontext->window_x = window_x;
        imcontext->window_y = window_y;

        scim_bridge_pdebugln (3, "The cursor location is changed: x = %d + %d\ty = %d + %d", imcontext->window_x, imcontext->cursor_x, imcontext->window_y, imcontext->cursor_y);

        if (scim_bridge_client_is_messenger_opened ()) {
            if (scim_bridge_client_set_cursor_location (imcontext, imcontext->window_x + imcontext->cursor_x, imcontext->window_y + imcontext->cursor_y)) {
                scim_bridge_perrorln ("An IOException occurred at set_cursor_location ()");
                return RETVAL_FAILED;
            } else {
                return RETVAL_SUCCEEDED;
            }
        }
    }

    return RETVAL_FAILED;
}


static gboolean key_snooper (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    scim_bridge_pdebugln (7, "key_snooper ()");

    if (focused_imcontext && scim_bridge_client_is_messenger_opened () &&
        (event->type == GDK_KEY_PRESS || event->type == GDK_KEY_RELEASE) &&
        !(event->send_event & SEND_EVENT_MASK)) {
        if (focused_imcontext->client_window) {
            int new_window_x;
            int new_window_y;
            gdk_window_get_origin (focused_imcontext->client_window, &new_window_x, &new_window_y);

            if (focused_imcontext->window_x != new_window_x || focused_imcontext->window_y != new_window_y) {

                scim_bridge_pdebugln (1,
                    "The cursor location is changed: x = %d + %d\ty = %d + %d",
                    new_window_x, focused_imcontext->cursor_x, new_window_y, focused_imcontext->cursor_y);

                if (set_cursor_location (focused_imcontext, new_window_x, new_window_y, focused_imcontext->cursor_x, focused_imcontext->cursor_y)) {
                    scim_bridge_perrorln ("An IOException at key_snooper ()");
                    return FALSE;
                }
            }
        }

        boolean consumed = FALSE;
        if (filter_key_event (focused_imcontext, event, &consumed)) {
            scim_bridge_perrorln ("An IOException at key_snooper ()");
            return FALSE;
        } else {
            if (consumed) {
                g_signal_emit_by_name (focused_imcontext, "preedit-changed");
                return TRUE;
            }
        }
    }

    return FALSE;
}


static boolean is_key_snooper_enabled ()
{
    static boolean first_time = TRUE;
    static boolean key_snooper_enabled = FALSE;

    if (first_time) {
        char *env_key_snooper_enabled = getenv ("SCIM_BRIDGE_KEY_SNOOPER_ENABLED");
        if (env_key_snooper_enabled != NULL) scim_bridge_string_to_boolean (&key_snooper_enabled, env_key_snooper_enabled);
        first_time = FALSE;
    }

    return key_snooper_enabled;
}

static boolean is_precise_cursor_enabled ()
{
    static boolean first_time = TRUE;
    static boolean precise_cursor_enabled = FALSE;

    if (first_time) {
        char *env_precise_cursor_enabled = getenv ("SCIM_BRIDGE_PRECISE_CURSOR_ENABLED");
        if (env_precise_cursor_enabled != NULL) scim_bridge_string_to_boolean (&precise_cursor_enabled, env_precise_cursor_enabled);
        first_time = FALSE;
    }

    return precise_cursor_enabled;
}

/* Bindings */
void scim_bridge_client_imcontext_set_id (ScimBridgeClientIMContext *imcontext, scim_bridge_imcontext_id_t new_id)
{
    imcontext->id = new_id;
}


scim_bridge_imcontext_id_t scim_bridge_client_imcontext_get_id (const ScimBridgeClientIMContext *imcontext)
{
    return imcontext->id;
}


void scim_bridge_client_imcontext_set_preedit_string (ScimBridgeClientIMContext *imcontext, const char *preedit_string)
{
    if (imcontext->preedit_string != NULL && preedit_string != NULL && !strcmp (imcontext->preedit_string, preedit_string))
        return;

    size_t preedit_string_length;
    if (preedit_string != NULL) {
        preedit_string_length = strlen (preedit_string);
    } else {
        preedit_string_length = 0;
    }
    if (imcontext->preedit_string_capacity <= preedit_string_length) {
        imcontext->preedit_string_capacity = preedit_string_length;
        free (imcontext->preedit_string);
        imcontext->preedit_string = malloc (sizeof (char) * (imcontext->preedit_string_capacity + 1));
    }
    if (preedit_string_length > 0) {
        strcpy (imcontext->preedit_string, preedit_string);
    } else {
        imcontext->preedit_string[0] = '\0';
    }
}


void scim_bridge_client_imcontext_set_preedit_shown (ScimBridgeClientIMContext *imcontext, boolean preedit_shown)
{
    imcontext->preedit_shown = preedit_shown;
    if (!preedit_shown) {
        free (imcontext->preedit_string);
        imcontext->preedit_string = malloc (sizeof (char));
        imcontext->preedit_string[0] = '\0';
        imcontext->preedit_string_capacity = 0;
        imcontext->preedit_cursor_position = 0;
        if (imcontext->preedit_attributes != NULL) {
            pango_attr_list_unref (imcontext->preedit_attributes);
            imcontext->preedit_attributes = NULL;
        }
    }
}

void scim_bridge_client_imcontext_set_preedit_cursor_position (ScimBridgeClientIMContext *imcontext, int cursor_position)
{
    imcontext->preedit_cursor_position = cursor_position;
}


void scim_bridge_client_imcontext_set_preedit_attributes (ScimBridgeClientIMContext *imcontext, ScimBridgeAttribute** const preedit_attributes, int attribute_count)
{   
    if (imcontext->preedit_attributes != NULL)
        pango_attr_list_unref (imcontext->preedit_attributes);
    
    imcontext->preedit_attributes = pango_attr_list_new ();
    
    int preedit_string_length = 0;
    int preedit_wstring_length = 0;
    if (imcontext->preedit_string != NULL) {
        preedit_string_length = strlen (imcontext->preedit_string);
        preedit_wstring_length = g_utf8_strlen (imcontext->preedit_string, -1);
    }
    
    boolean *has_attribute = alloca (sizeof (boolean) * preedit_string_length);
    int i;
    for (i = 0; i < preedit_string_length; ++i) {
        has_attribute[i] = FALSE;
    }

    for (i = 0; i < attribute_count; ++i) {
        const ScimBridgeAttribute *attr = preedit_attributes[i];
        const int begin_pos = scim_bridge_attribute_get_begin (attr);
        const int end_pos = scim_bridge_attribute_get_end (attr);

        if (begin_pos <= end_pos && 0 <= begin_pos && end_pos <= preedit_wstring_length) {
            const int start_index = g_utf8_offset_to_pointer (imcontext->preedit_string, begin_pos) - imcontext->preedit_string;
            const int end_index = g_utf8_offset_to_pointer (imcontext->preedit_string, end_pos) - imcontext->preedit_string;

            const scim_bridge_attribute_type_t attr_type = scim_bridge_attribute_get_type (attr);
            const scim_bridge_attribute_value_t attr_value = scim_bridge_attribute_get_value (attr);

            boolean valid_attribute = FALSE;
            if (attr_type == ATTRIBUTE_DECORATE) {
                if (attr_value == SCIM_BRIDGE_ATTRIBUTE_DECORATE_UNDERLINE) {
                    valid_attribute = TRUE;

                    PangoAttribute *pango_attr = pango_attr_underline_new (PANGO_UNDERLINE_SINGLE);
                    pango_attr->start_index = start_index;
                    pango_attr->end_index = end_index;
                    pango_attr_list_insert (imcontext->preedit_attributes, pango_attr);
                } else if (attr_value == SCIM_BRIDGE_ATTRIBUTE_DECORATE_REVERSE) {
                    valid_attribute = TRUE;

                    PangoAttribute *pango_attr0 = pango_attr_foreground_new (preedit_normal_background.red, preedit_normal_background.green, preedit_normal_background.blue);
                    pango_attr0->start_index = start_index;
                    pango_attr0->end_index = end_index;
                    pango_attr_list_insert (imcontext->preedit_attributes, pango_attr0);

                    PangoAttribute *pango_attr1 = pango_attr_background_new (preedit_normal_foreground.red, preedit_normal_foreground.green, preedit_normal_foreground.blue);
                    pango_attr1->start_index = start_index;
                    pango_attr1->end_index = end_index;
                    pango_attr_list_insert (imcontext->preedit_attributes, pango_attr1);
                } else if (attr_value == SCIM_BRIDGE_ATTRIBUTE_DECORATE_HIGHLIGHT) {
                    valid_attribute = TRUE;

                    PangoAttribute *pango_attr0 = pango_attr_foreground_new (preedit_active_foreground.red, preedit_active_foreground.green, preedit_active_foreground.blue);
                    pango_attr0->start_index = start_index;
                    pango_attr0->end_index = end_index;
                    pango_attr_list_insert (imcontext->preedit_attributes, pango_attr0);

                    PangoAttribute *pango_attr1 = pango_attr_background_new (preedit_active_background.red, preedit_active_background.green, preedit_active_background.blue);
                    pango_attr1->start_index = start_index;
                    pango_attr1->end_index = end_index;
                    pango_attr_list_insert (imcontext->preedit_attributes, pango_attr1);
                } else {
                    scim_bridge_perrorln ("Unknown preedit decoration!");
                }
            } else if (attr_type == ATTRIBUTE_FOREGROUND) {
                valid_attribute = TRUE;

                const unsigned int red = scim_bridge_attribute_get_red (attr) * 256;
                const unsigned int green = scim_bridge_attribute_get_green (attr) * 256;
                const unsigned int blue = scim_bridge_attribute_get_blue (attr) * 256;

                PangoAttribute *pango_attr = pango_attr_foreground_new (red, green, blue);
                pango_attr->start_index = start_index;
                pango_attr->end_index = end_index;
                pango_attr_list_insert (imcontext->preedit_attributes, pango_attr);
            } else if (attr_type == ATTRIBUTE_BACKGROUND) {
                valid_attribute = TRUE;

                const unsigned int red = scim_bridge_attribute_get_red (attr) * 256;
                const unsigned int green = scim_bridge_attribute_get_green (attr) * 256;
                const unsigned int blue = scim_bridge_attribute_get_blue (attr) * 256;

                PangoAttribute *pango_attr = pango_attr_background_new (red, green, blue);
                pango_attr->start_index = start_index;
                pango_attr->end_index = end_index;
                pango_attr_list_insert (imcontext->preedit_attributes, pango_attr);
            }

            if (valid_attribute) {
                int j;
                for (j = start_index; j < end_index; ++j) {
                    has_attribute[j] = TRUE;
                }
            }
        }

    }

    // Add underlines for the all characters without attributes.
    for (i = 0; i < preedit_string_length; ++i) {
        if (has_attribute[i] == FALSE) {
            PangoAttribute *pango_attr = pango_attr_underline_new (PANGO_UNDERLINE_SINGLE);
            pango_attr->start_index = i;
            for (; i < preedit_string_length && has_attribute[i] == FALSE; ++i);
            pango_attr->end_index = i;
            pango_attr_list_insert (imcontext->preedit_attributes, pango_attr);
        }
    }
}


void scim_bridge_client_imcontext_update_preedit (ScimBridgeClientIMContext *imcontext)
{
    if (imcontext->preedit_shown && !imcontext->preedit_started) {
        g_signal_emit_by_name ((ScimBridgeClientIMContext*) imcontext, "preedit-start");
        imcontext->preedit_started = TRUE;
    }
    
    if (is_precise_cursor_enabled ()) {
        const size_t old_cursor_position = imcontext->preedit_cursor_position;
        imcontext->preedit_cursor_position = 0;
        if (imcontext->preedit_string != NULL)
            imcontext->preedit_cursor_position = g_utf8_strlen (imcontext->preedit_string, -1);
        
        imcontext->preedit_cursor_flicking = TRUE;
        g_signal_emit_by_name ((ScimBridgeClientIMContext*) imcontext, "preedit-changed");
        
        imcontext->preedit_cursor_position = old_cursor_position;
        imcontext->preedit_cursor_flicking = FALSE;
    }
    
    g_signal_emit_by_name ((ScimBridgeClientIMContext*) imcontext, "preedit-changed");
    
    if (!imcontext->preedit_shown && imcontext->preedit_started) {
        g_signal_emit_by_name ((ScimBridgeClientIMContext*) imcontext, "preedit-end");
        imcontext->preedit_started = FALSE;
    }
}


void scim_bridge_client_imcontext_set_commit_string (ScimBridgeClientIMContext *imcontext, const char *commit_string)
{
    size_t commit_string_length;
    if (commit_string != NULL) {
        commit_string_length = strlen (commit_string);
    } else {
        commit_string_length = 0;
    }
    
    if (imcontext->commit_string_capacity <= commit_string_length) {
        imcontext->commit_string_capacity = commit_string_length;
        free (imcontext->commit_string);
        imcontext->commit_string = malloc (sizeof (char) * (imcontext->commit_string_capacity + 1));
    }
    if (commit_string_length > 0) {
        strcpy (imcontext->commit_string, commit_string);
    } else {
        imcontext->commit_string[0] = '\0';
    }
}


void scim_bridge_client_imcontext_commit (ScimBridgeClientIMContext *imcontext)
{
    g_signal_emit_by_name ((ScimBridgeClientIMContext*) imcontext, "commit", imcontext->commit_string);
}


void scim_bridge_client_imcontext_beep (ScimBridgeClientIMContext *imcontext)
{
    gdk_beep ();
}


boolean scim_bridge_client_imcontext_get_surrounding_text (ScimBridgeClientIMContext *imcontext, int before_max, int after_max, char **string, int *cursor_position)
{
    char *str;
    int cur_pos_in_utf8;

    if (gtk_im_context_get_surrounding (GTK_IM_CONTEXT (imcontext), &str, &cur_pos_in_utf8)) {
        const size_t fetch_wstr_length = g_utf8_strlen (str, -1);
        const size_t after_wstr_length = g_utf8_strlen (str + cur_pos_in_utf8, -1);
        const size_t before_wstr_length = fetch_wstr_length - after_wstr_length;

        size_t before_copy_wstr_length;
        size_t after_copy_wstr_length;
        if (after_wstr_length > after_max) {
            after_copy_wstr_length = after_max;
        } else {
            after_copy_wstr_length = after_wstr_length;
        }
        if (before_wstr_length > before_max) {
            before_copy_wstr_length = before_max;
        } else {
            before_copy_wstr_length = before_wstr_length;
        }

        const size_t begin_wstr_index = before_wstr_length - before_copy_wstr_length;
        const size_t end_wstr_index = fetch_wstr_length - (after_wstr_length - after_copy_wstr_length);

        char* begin_str_ptr = g_utf8_offset_to_pointer (str, begin_wstr_index);
        char* end_str_ptr = g_utf8_offset_to_pointer (str, end_wstr_index);
        size_t str_length = end_str_ptr - begin_str_ptr;

        *string = malloc (sizeof (char) * (str_length + 1));
        strncpy (*string, begin_str_ptr, str_length);
        (*string)[str_length] = '\0';
        *cursor_position = before_copy_wstr_length;

        g_free (str);
        return TRUE;
    } else {
        *string = NULL;

        return FALSE;
    }
}


boolean scim_bridge_client_imcontext_delete_surrounding_text (ScimBridgeClientIMContext *imcontext, int offset, int length)
{
    boolean retval = gtk_im_context_delete_surrounding (GTK_IM_CONTEXT (imcontext), offset, length);
    return retval;
}


boolean scim_bridge_client_imcontext_replace_surrounding_text (ScimBridgeClientIMContext *imcontext, int cursor_position, const char *string)
{
    gtk_im_context_set_surrounding (GTK_IM_CONTEXT (imcontext), string, -1, cursor_position);
    return TRUE;
}

void scim_bridge_client_imcontext_forward_key_event (ScimBridgeClientIMContext *imcontext, const ScimBridgeKeyEvent *key_event)
{ 
    if (imcontext && imcontext == focused_imcontext) {
        GdkEventKey gdk_event;
        scim_bridge_key_event_bridge_to_gdk (&gdk_event, imcontext->client_window, key_event);
        gdk_event.send_event |= SEND_EVENT_MASK;

        if (!gtk_im_context_filter_keypress (GTK_IM_CONTEXT (imcontext->slave), &gdk_event)) {
            // To avoid timing issue, we need emit the signal directly, rather than put the event into the queue.
            if (focused_widget) {
                gboolean result;
                g_signal_emit_by_name(focused_widget,
                    scim_bridge_key_event_is_pressed (key_event) ? "key-press-event" : "key-release-event",
                    &gdk_event,
                    &result
                );
            } else {
                gdk_event_put ((GdkEvent *) &gdk_event);
            }
        }
    }
}


void scim_bridge_client_imcontext_imengine_status_changed (ScimBridgeClientIMContext *imcontext, boolean enabled)
{
    if (imcontext->preedit_shown) {
        if (imcontext->enabled) {
            scim_bridge_client_imcontext_set_preedit_shown (imcontext, FALSE);
            scim_bridge_client_imcontext_update_preedit (imcontext);
        }
    }
    imcontext->enabled = enabled;
}


/* Class Implementations */
void scim_bridge_client_imcontext_static_initialize ()
{
    gdk_color_parse ("gray92", &preedit_normal_background);
    gdk_color_parse ("black", &preedit_normal_foreground);
    gdk_color_parse ("light blue", &preedit_active_background);
    gdk_color_parse ("black", &preedit_active_foreground);

    focused_imcontext = NULL;
}


void scim_bridge_client_imcontext_static_finalize ()
{
    if (key_snooper_used) {
        gtk_key_snooper_remove (key_snooper_id);
        key_snooper_id = 0;
        key_snooper_used = FALSE;
    }

    focused_imcontext = NULL;
}


void scim_bridge_client_imcontext_connection_opened ()
{
}


void scim_bridge_client_imcontext_connection_closed ()
{
    if (focused_imcontext != NULL) scim_bridge_client_imcontext_set_preedit_shown (focused_imcontext, FALSE);
}


void scim_bridge_client_imcontext_class_initialize (ScimBridgeClientIMContextClass *klass, gpointer *klass_data)
{
    root_klass = (GObjectClass *) g_type_class_peek_parent (klass);

    GtkIMContextClass *gtk_im_klass = GTK_IM_CONTEXT_CLASS (klass);
    gtk_im_klass->set_client_window = scim_bridge_client_imcontext_set_client_window;
    gtk_im_klass->filter_keypress = scim_bridge_client_imcontext_filter_key_event;
    gtk_im_klass->reset = scim_bridge_client_imcontext_reset;
    gtk_im_klass->get_preedit_string = scim_bridge_client_imcontext_get_preedit_string;
    gtk_im_klass->focus_in  = scim_bridge_client_imcontext_focus_in;
    gtk_im_klass->focus_out = scim_bridge_client_imcontext_focus_out;
    gtk_im_klass->set_cursor_location = scim_bridge_client_imcontext_set_cursor_location;
    gtk_im_klass->set_use_preedit = scim_bridge_client_imcontext_set_preedit_enabled;

    GObjectClass *gobject_klass = G_OBJECT_CLASS (klass);
    gobject_klass->finalize = scim_bridge_client_imcontext_finalize;
}


GType scim_bridge_client_imcontext_get_type ()
{
    return class_type;
}


void scim_bridge_client_imcontext_register_type (GTypeModule *type_module)
{
    scim_bridge_pdebugln (2, "scim_bridge_client_imcontext_register_type ()");

    static const GTypeInfo klass_info = {
        sizeof (ScimBridgeClientIMContextClass),
        /* no base class initializer */
        NULL,
        /* no base class finalizer */
        NULL,
        /* class initializer */
        (GClassInitFunc) scim_bridge_client_imcontext_class_initialize,
        /* no class finalizer */
        NULL,
        /* no class data */
        NULL,
        sizeof (ScimBridgeClientIMContext),
        0,
        /* object initizlier */
#if GTK_CHECK_VERSION(3, 0, 0)
        (GInstanceInitFunc) scim_bridge_client_imcontext_initialize,
#else
        (GtkObjectInitFunc) scim_bridge_client_imcontext_initialize,
#endif
        0
    };

    class_type = g_type_module_register_type (type_module, GTK_TYPE_IM_CONTEXT, "ScimBridgeClientIMContext", &klass_info, 0);
}


GtkIMContext *scim_bridge_client_imcontext_new ()
{
    scim_bridge_pdebugln (4, "scim_bridge_client_imcontext_new ()");

    ScimBridgeClientIMContext *ic = SCIM_BRIDGE_CLIENT_IMCONTEXT (g_object_new (GTK_TYPE_SCIM_CLIENT_IMCONTEXT, NULL));
    return GTK_IM_CONTEXT (ic);
}


void scim_bridge_client_imcontext_initialize (ScimBridgeClientIMContext *imcontext, ScimBridgeClientIMContextClass *klass)
{
    scim_bridge_pdebugln (5, "scim_bridge_client_imcontext_initialize  ()");

    /* slave exists for using gtk+'s table based input method */
    imcontext->slave_preedit = FALSE;
    imcontext->slave = gtk_im_context_simple_new ();
    g_signal_connect(G_OBJECT(imcontext->slave),
                     "commit",
                     G_CALLBACK(gtk_im_slave_commit_cb),
                     imcontext);
    
    g_signal_connect(G_OBJECT(imcontext->slave),
                     "preedit-changed",
                     G_CALLBACK(gtk_im_slave_preedit_changed_cb),
                     imcontext);
    
    g_signal_connect(G_OBJECT(imcontext->slave),
                     "preedit-start",
                     G_CALLBACK(gtk_im_slave_preedit_start_cb),
                     imcontext);
    
    g_signal_connect(G_OBJECT(imcontext->slave),
                     "preedit-end",
                     G_CALLBACK(gtk_im_slave_preedit_end_cb),
                     imcontext);

    imcontext->preedit_shown = FALSE;
    imcontext->preedit_started = FALSE;
    
    imcontext->preedit_cursor_position = 0;
    imcontext->preedit_cursor_flicking = FALSE;

    imcontext->preedit_string = malloc (sizeof (char));
    imcontext->preedit_string[0] = '\0';
    imcontext->preedit_string_capacity = 0;

    imcontext->preedit_attributes = NULL;

    imcontext->commit_string = malloc (sizeof (char));
    imcontext->commit_string[0] = '\0';
    imcontext->commit_string_capacity = 0;

    imcontext->enabled = FALSE;

    imcontext->client_window = NULL;

    imcontext->id = -1;

    if (!scim_bridge_client_is_messenger_opened ()) {
        scim_bridge_perrorln ("The messenger is now down");
    } else if (scim_bridge_client_register_imcontext (imcontext)) {
        scim_bridge_perrorln ("Failed to register the IMContext");
    } else {
        scim_bridge_pdebugln (1, "IMContext registered: id = %d", imcontext->id);
    }
}


void scim_bridge_client_imcontext_finalize (GObject *object)
{
    scim_bridge_pdebugln (5, "scim_bridge_client_imcontext_finalize ()");

    ScimBridgeClientIMContext *imcontext = SCIM_BRIDGE_CLIENT_IMCONTEXT (object);

    if (imcontext == focused_imcontext) scim_bridge_client_imcontext_focus_out (GTK_IM_CONTEXT (imcontext));

    if (!scim_bridge_client_is_messenger_opened ()) {
        scim_bridge_perrorln ("The messenger is now down");
    } else if (scim_bridge_client_deregister_imcontext (imcontext)) {
        scim_bridge_perrorln ("Failed to deregister an IMContext");
    } else {
        scim_bridge_pdebugln (3, "IMContext deregistered: id = %d", imcontext->id);
    }

    if (imcontext->client_window) g_object_unref (imcontext->client_window);

    free (imcontext->preedit_string);
    free (imcontext->commit_string);

    if (imcontext->preedit_attributes != NULL)
        pango_attr_list_unref (imcontext->preedit_attributes);
    
    imcontext->preedit_attributes = NULL;
    
    g_signal_handlers_disconnect_by_func(imcontext->slave,
                                         (void *)gtk_im_slave_commit_cb,
                                         (void *)imcontext);
    g_signal_handlers_disconnect_by_func(imcontext->slave,
                                         (void *)gtk_im_slave_preedit_changed_cb,
                                         (void *)imcontext);
    g_signal_handlers_disconnect_by_func(imcontext->slave,
                                         (void *)gtk_im_slave_preedit_start_cb,
                                         (void *)imcontext);
    g_signal_handlers_disconnect_by_func(imcontext->slave,
                                         (void *)gtk_im_slave_preedit_end_cb,
                                         (void *)imcontext);
    g_object_unref(imcontext->slave);

    root_klass->finalize (object);
}


/* Class functions */
gboolean scim_bridge_client_imcontext_filter_key_event (GtkIMContext *context, GdkEventKey *event)
{
    scim_bridge_pdebugln (8, "scim_bridge_client_imcontext_filter_key_event ()");

    ScimBridgeClientIMContext *imcontext = SCIM_BRIDGE_CLIENT_IMCONTEXT (context);

    boolean ret = FALSE;
    if (imcontext) {
        if (!key_snooper_used) ret = key_snooper(0, event, 0);

        if (imcontext->slave) {
            if (!ret) {
                ret = gtk_im_context_filter_keypress (imcontext->slave, event);
            } else if (imcontext->slave_preedit) {
                imcontext->slave_preedit = FALSE;
                gtk_im_context_reset (imcontext->slave);
            }
        }

    }

    return ret;
}


void scim_bridge_client_imcontext_reset (GtkIMContext *context)
{
    scim_bridge_pdebugln (8, "scim_bridge_client_imcontext_reset ()");

    ScimBridgeClientIMContext *imcontext = SCIM_BRIDGE_CLIENT_IMCONTEXT (context);

    if (imcontext != focused_imcontext) return;

    if (scim_bridge_client_is_messenger_opened () && imcontext != NULL) {
        if (scim_bridge_client_reset_imcontext (imcontext)) {
            scim_bridge_perrorln ("An IOException occurred at scim_bridge_client_imcontext_reset ()");
        }
    }
}


void scim_bridge_client_imcontext_get_preedit_string (GtkIMContext *context, gchar **str, PangoAttrList **pango_attrs, gint *cursor_pos)
{
    scim_bridge_pdebugln (4, "scim_bridge_client_imcontext_get_preedit_string ()");

    ScimBridgeClientIMContext *imcontext = SCIM_BRIDGE_CLIENT_IMCONTEXT (context);

    if (imcontext->slave_preedit) {
        gtk_im_context_get_preedit_string (imcontext->slave, str, pango_attrs, cursor_pos);
        return;
    }
    
    if (scim_bridge_client_is_messenger_opened () && imcontext != NULL && imcontext->preedit_shown) {
        const size_t preedit_string_length = strlen (imcontext->preedit_string);
        const size_t preedit_wstring_length = g_utf8_strlen (imcontext->preedit_string, -1);
        
        if (str) {
            if (preedit_string_length > 0) {
                *str = g_strdup (imcontext->preedit_string);
            } else {
                *str = g_strdup ("");
            }
        }
        
        if (cursor_pos) {
            if (imcontext->preedit_cursor_position > preedit_wstring_length) {
                *cursor_pos = preedit_wstring_length;
            } else {
                *cursor_pos = imcontext->preedit_cursor_position;
            }
        }
        
        if (pango_attrs) {
            *pango_attrs = imcontext->preedit_attributes;
            pango_attr_list_ref (imcontext->preedit_attributes);
        }
    } else {
        if (str) *str = g_strdup ("");
        if (cursor_pos) *cursor_pos = 0;
        if (pango_attrs) *pango_attrs = pango_attr_list_new ();
    }
}


void scim_bridge_client_imcontext_focus_in (GtkIMContext *context)
{
    scim_bridge_pdebugln (8, "scim_bridge_client_imcontext_focus_in ()");

    ScimBridgeClientIMContext *imcontext = SCIM_BRIDGE_CLIENT_IMCONTEXT (context);
    
    if (focused_imcontext != NULL && focused_imcontext != imcontext) scim_bridge_client_imcontext_focus_out (GTK_IM_CONTEXT (focused_imcontext));
    focused_imcontext = imcontext;

    if (!scim_bridge_client_is_messenger_opened () && scim_bridge_client_is_reconnection_enabled ()) {
        scim_bridge_pdebugln (7, "Trying to open the connection again...");
        scim_bridge_client_open_messenger ();
    }

    if (!key_snooper_used && is_key_snooper_enabled ()) {
        key_snooper_id = gtk_key_snooper_install ((GtkKeySnoopFunc) &key_snooper, NULL);
        key_snooper_used = TRUE;
    }
    if (scim_bridge_client_is_messenger_opened () && imcontext != NULL) {
        if (scim_bridge_client_change_focus (imcontext, TRUE)) {
            scim_bridge_perrorln ("An IOException occurred at scim_bridge_client_imcontext_focus_in ()");
        }
    }
}


void scim_bridge_client_imcontext_focus_out (GtkIMContext *context)
{
    scim_bridge_pdebugln (8, "scim_bridge_client_imcontext_focus_out ()");

    ScimBridgeClientIMContext *imcontext = SCIM_BRIDGE_CLIENT_IMCONTEXT (context);
    focused_widget = NULL;

    focused_imcontext = imcontext;
    if (imcontext->preedit_shown) {
        if (imcontext->enabled) {
            scim_bridge_client_imcontext_set_preedit_shown (imcontext, FALSE);
            scim_bridge_client_imcontext_update_preedit (imcontext);
        }
    }
    if (scim_bridge_client_is_messenger_opened () && imcontext != NULL) {
        if (scim_bridge_client_change_focus (imcontext, FALSE)) {
            scim_bridge_perrorln ("An IOException occurred at scim_bridge_client_imcontext_focus_out ()");
        }
    }
    if (key_snooper_used) {
        gtk_key_snooper_remove (key_snooper_id);
        key_snooper_id = 0;
        key_snooper_used = FALSE;
    }

    focused_imcontext = NULL;
}


void scim_bridge_client_imcontext_set_client_window (GtkIMContext *context, GdkWindow *new_window)
{
    scim_bridge_pdebugln (7, "scim_bridge_client_imcontext_set_client_window ()");

    ScimBridgeClientIMContext *imcontext = SCIM_BRIDGE_CLIENT_IMCONTEXT (context);

    if (imcontext != NULL) {
        if (imcontext->client_window != NULL) g_object_unref (imcontext->client_window);
        imcontext->client_window = new_window;
        if (new_window != NULL) {
            g_object_ref (new_window);
            gdk_window_get_origin (imcontext->client_window, &imcontext->window_x, &imcontext->window_y);
        }
    }
}


void scim_bridge_client_imcontext_set_cursor_location (GtkIMContext *context, GdkRectangle *area)
{
    scim_bridge_pdebugln (4, "scim_bridge_client_imcontext_set_cursor_location ()");

    ScimBridgeClientIMContext *imcontext = SCIM_BRIDGE_CLIENT_IMCONTEXT (context);
    if (imcontext->preedit_cursor_flicking)
        return;

    if (imcontext != NULL && imcontext->client_window != NULL) {
        const int new_cursor_x = area->x + area->width;
        const int new_cursor_y = area->y + area->height + 8;

        int new_window_x;
        int new_window_y;
        gdk_window_get_origin (imcontext->client_window, &new_window_x, &new_window_y);

        if (set_cursor_location (imcontext, new_window_x, new_window_y, new_cursor_x, new_cursor_y)) {
            scim_bridge_perrorln ("An IOException occurred at scim_bridge_client_imcontext_set_cursor_location ()");
        }
    }
}


void scim_bridge_client_imcontext_set_preedit_enabled (GtkIMContext *context, gboolean enabled)
{
    scim_bridge_pdebugln (8, "scim_bridge_client_imcontext_set_preedit_enabled ()");

    ScimBridgeClientIMContext *imcontext = SCIM_BRIDGE_CLIENT_IMCONTEXT (context);

    if (imcontext != NULL) {
        if (scim_bridge_client_is_messenger_opened ()) {
            if (scim_bridge_client_set_preedit_mode (imcontext, enabled ? PREEDIT_EMBEDDED:PREEDIT_ANY)) {
                scim_bridge_perrorln ("An IOException occurred at scim_bridge_client_imcontext_set_preedit_enabled ()");
            }
        }
    }
}

static void
gtk_im_slave_commit_cb (
    GtkIMContext *context,
    const char *str,
    ScimBridgeClientIMContext *imcontext
) {
    g_return_if_fail(str);
    g_signal_emit_by_name(imcontext, "commit", str);
}

static void
gtk_im_slave_preedit_changed_cb (
    GtkIMContext *context,
    ScimBridgeClientIMContext *imcontext
) {
    imcontext->slave_preedit = TRUE;
    g_signal_emit_by_name(imcontext, "preedit-changed");
}

static void
gtk_im_slave_preedit_start_cb (
    GtkIMContext *context,
    ScimBridgeClientIMContext *imcontext
) {
    imcontext->slave_preedit = TRUE;
    g_signal_emit_by_name(imcontext, "preedit-start");
}

static void
gtk_im_slave_preedit_end_cb (
    GtkIMContext *context,
    ScimBridgeClientIMContext *imcontext
) {
    imcontext->slave_preedit = FALSE;
    g_signal_emit_by_name(imcontext, "preedit-end");
}
