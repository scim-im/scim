/*
 * SCIM Bridge
 *
 * Copyright (c) 2006 Ryo Dairiki <ryo-dairiki@users.sourceforge.net>
 * Copyright (C) 2009, Intel Corporation.
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

#include <X11/Xlib.h>
#include <clutter/clutter.h>
#include <clutter/x11/clutter-x11.h>
#include <clutter-imcontext/clutter-imcontextsimple.h>

#include "scim-bridge-attribute.h"
#include "scim-bridge-client.h"
#include "scim-bridge-client-imcontext-clutter.h"
#include "scim-bridge-client-key-event-utility-clutter.h"
#include "scim-bridge-imcontext.h"
#include "scim-bridge-messenger.h"
#include "scim-bridge-output.h"
#include "scim-bridge-string.h"

/* Typedef */
struct _ScimBridgeClientIMContext
{
    ClutterIMContext parent;

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

    ClutterStage *client_stage;

    int cursor_x;
    int cursor_y;
    int window_x;
    int window_y;
};

static GType class_type = 0;
static GObjectClass *root_klass = NULL;

static ScimBridgeClientIMContext *focused_imcontext = NULL;
static ClutterActor *focused_actor = NULL;

static ClutterIMContext *fallback_imcontext = NULL;
static gulong fallback_commit_handler;
static gulong fallback_preedit_changed_handler;


/* Class functions */
static void scim_bridge_client_imcontext_class_initialize (ScimBridgeClientIMContextClass *klass, gpointer *klass_data);

static void scim_bridge_client_imcontext_initialize (ScimBridgeClientIMContext *context, ScimBridgeClientIMContextClass *klass);
static void scim_bridge_client_imcontext_finalize (GObject *object);

static gboolean scim_bridge_client_imcontext_filter_key_event (ClutterIMContext *context, ClutterKeyEvent *event);
static void scim_bridge_client_imcontext_reset (ClutterIMContext *context);
static void scim_bridge_client_imcontext_show (ClutterIMContext *context);
static void scim_bridge_client_imcontext_hide (ClutterIMContext *context);
static void scim_bridge_client_imcontext_get_preedit_string (ClutterIMContext *context, gchar **str, PangoAttrList **attrs, gint *cursor_pos);
static void scim_bridge_client_imcontext_set_preedit_enabled (ClutterIMContext *context, gboolean enabled);

static void scim_bridge_client_imcontext_focus_in (ClutterIMContext *context);
static void scim_bridge_client_imcontext_focus_out (ClutterIMContext *context);
static void scim_bridge_client_imcontext_set_cursor_location (ClutterIMContext *context, ClutterIMRectangle *area);


/* Helper functions */
static void fallback_commit (ClutterIMContext *context, const char *str, gpointer data)
{
    scim_bridge_pdebugln (4, "fallback_commit ()");
    if (focused_imcontext != NULL && !focused_imcontext->enabled && str != NULL) {
        g_signal_emit_by_name (focused_imcontext, "commit", str);
    }
}

static void fallback_preedit_changed (ClutterIMContext *context, gpointer data)
{
    scim_bridge_pdebugln (4, "fallback_preedit_changed ()");
    if (focused_imcontext != NULL && !focused_imcontext->enabled && context != NULL) {
        gchar* preedit_string = NULL;
        gint preedit_cursor_position = 0;
        PangoAttrList *preedit_attributes = NULL;
        clutter_im_context_get_preedit_string (context, &preedit_string, &preedit_attributes, &preedit_cursor_position);

        if (preedit_string != NULL) {
            free (focused_imcontext->preedit_string);
            focused_imcontext->preedit_string = preedit_string;
            focused_imcontext->preedit_string_capacity = strlen (preedit_string);
            focused_imcontext->preedit_shown = TRUE;
        } else {
            focused_imcontext->preedit_string[0] = '\0';
            focused_imcontext->preedit_shown = FALSE;
        }

        focused_imcontext->preedit_cursor_position = preedit_cursor_position;

        if (focused_imcontext->preedit_attributes != NULL)
            pango_attr_list_unref (focused_imcontext->preedit_attributes);
        focused_imcontext->preedit_attributes = preedit_attributes;

        g_signal_emit_by_name (focused_imcontext, "preedit-changed");
    }
}


static retval_t filter_key_event (ScimBridgeClientIMContext *imcontext, ClutterKeyEvent *event, boolean *consumed)
{
    scim_bridge_pdebugln (5, "filter_key_event ()");

    if (focused_imcontext != imcontext) scim_bridge_client_imcontext_focus_in (CLUTTER_IM_CONTEXT (imcontext));

/* if the source is null, then it's the event we forward out, and we do not handle it again */
    if (clutter_event_get_source ((ClutterEvent*) event) == NULL)
	return RETVAL_SUCCEEDED;

    focused_actor = clutter_event_get_source ((ClutterEvent*) event);

    if (scim_bridge_client_is_messenger_opened ()) {
        ScimBridgeKeyEvent *bridge_key_event = scim_bridge_alloc_key_event ();
        scim_bridge_key_event_clutter_to_bridge (bridge_key_event, imcontext->client_stage, event);

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
}


boolean scim_bridge_client_imcontext_get_surrounding_text (ScimBridgeClientIMContext *imcontext, int before_max, int after_max, char **string, int *cursor_position)
{
    char *str;
    int cur_pos_in_utf8;

    if (clutter_im_context_get_surrounding (CLUTTER_IM_CONTEXT (imcontext), &str, &cur_pos_in_utf8)) {
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
    boolean retval = clutter_im_context_delete_surrounding (CLUTTER_IM_CONTEXT (imcontext), offset, length);
    return retval;
}


boolean scim_bridge_client_imcontext_replace_surrounding_text (ScimBridgeClientIMContext *imcontext, int cursor_position, const char *string)
{
    clutter_im_context_set_surrounding (CLUTTER_IM_CONTEXT (imcontext), string, -1, cursor_position);
    return TRUE;
}


void scim_bridge_client_imcontext_forward_key_event (ScimBridgeClientIMContext *imcontext, const ScimBridgeKeyEvent *key_event)
{
    ClutterKeyEvent clutter_event;

    scim_bridge_key_event_bridge_to_clutter (&clutter_event, imcontext->client_stage, key_event);

    if (imcontext == focused_imcontext && focused_actor != NULL) {
        const char *signal_name = NULL;
        if (scim_bridge_key_event_is_pressed (key_event)) {
            signal_name = "key-press-event";
        } else {
            signal_name = "key-release-event";
        }

        gboolean consumed = FALSE;
        g_signal_emit_by_name (focused_actor, signal_name, &clutter_event, &consumed);
    } else {
        //gdk_event_put ((GdkEvent*) &clutter_event);
    }
}


void scim_bridge_client_imcontext_imengine_status_changed (ScimBridgeClientIMContext *imcontext, boolean enabled)
{
    if (imcontext->preedit_shown) {
        if (imcontext->enabled) {
            scim_bridge_client_imcontext_set_preedit_shown (imcontext, FALSE);
            scim_bridge_client_imcontext_update_preedit (imcontext);
        } else {
            clutter_im_context_reset (CLUTTER_IM_CONTEXT (fallback_imcontext));
        }
    }
    imcontext->enabled = enabled;
}


/* Class Implementations */
void scim_bridge_client_imcontext_static_initialize ()
{
    focused_imcontext = NULL;

    fallback_imcontext = clutter_im_context_simple_new ();
    fallback_commit_handler = g_signal_connect (G_OBJECT (fallback_imcontext), "commit", G_CALLBACK (fallback_commit), NULL);
    fallback_preedit_changed_handler = g_signal_connect (G_OBJECT (fallback_imcontext), "preedit_changed", G_CALLBACK (fallback_preedit_changed), NULL);
}


void scim_bridge_client_imcontext_static_finalize ()
{
    g_signal_handlers_disconnect_by_func (fallback_imcontext, &fallback_commit_handler, NULL);
    g_object_unref (fallback_imcontext);
    fallback_imcontext = NULL;
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

    ClutterIMContextClass *clutter_im_klass = CLUTTER_IM_CONTEXT_CLASS (klass);
    clutter_im_klass->filter_keypress = scim_bridge_client_imcontext_filter_key_event;
    clutter_im_klass->reset = scim_bridge_client_imcontext_reset;
    clutter_im_klass->show = scim_bridge_client_imcontext_show;
    clutter_im_klass->hide = scim_bridge_client_imcontext_hide;
    clutter_im_klass->get_preedit_string = scim_bridge_client_imcontext_get_preedit_string;
    clutter_im_klass->focus_in  = scim_bridge_client_imcontext_focus_in;
    clutter_im_klass->focus_out = scim_bridge_client_imcontext_focus_out;
    clutter_im_klass->set_cursor_location = scim_bridge_client_imcontext_set_cursor_location;
    clutter_im_klass->set_use_preedit = scim_bridge_client_imcontext_set_preedit_enabled;

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
        (GInstanceInitFunc) scim_bridge_client_imcontext_initialize,
    };

    if (!class_type) class_type = g_type_module_register_type (type_module, CLUTTER_TYPE_IM_CONTEXT, "ScimBridgeClientIMContext", &klass_info, 0);
}


ClutterIMContext *scim_bridge_client_imcontext_new ()
{
    scim_bridge_pdebugln (4, "scim_bridge_client_imcontext_new ()");

    ScimBridgeClientIMContext *ic = SCIM_BRIDGE_CLIENT_IMCONTEXT (g_object_new (CLUTTER_TYPE_SCIM_CLIENT_IMCONTEXT, NULL));
    return CLUTTER_IM_CONTEXT (ic);
}


void scim_bridge_client_imcontext_initialize (ScimBridgeClientIMContext *imcontext, ScimBridgeClientIMContextClass *klass)
{
    scim_bridge_pdebugln (5, "scim_bridge_client_imcontext_initialize  ()");

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

    imcontext->client_stage = NULL;

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

    if (imcontext == focused_imcontext) scim_bridge_client_imcontext_focus_out (CLUTTER_IM_CONTEXT (imcontext));

    if (!scim_bridge_client_is_messenger_opened ()) {
        scim_bridge_perrorln ("The messenger is now down");
    } else if (scim_bridge_client_deregister_imcontext (imcontext)) {
        scim_bridge_perrorln ("Failed to deregister an IMContext");
    } else {
        scim_bridge_pdebugln (3, "IMContext deregistered: id = %d", imcontext->id);
    }

    if (imcontext->client_stage) g_object_unref (imcontext->client_stage);

    free (imcontext->preedit_string);
    free (imcontext->commit_string);

    if (imcontext->preedit_attributes != NULL)
        pango_attr_list_unref (imcontext->preedit_attributes);

    imcontext->preedit_attributes = NULL;

    root_klass->finalize (object);
}


/* Class functions */
gboolean scim_bridge_client_imcontext_filter_key_event (ClutterIMContext *context, ClutterKeyEvent *event)
{
    scim_bridge_pdebugln (8, "scim_bridge_client_imcontext_filter_key_event ()");

    ScimBridgeClientIMContext *imcontext = SCIM_BRIDGE_CLIENT_IMCONTEXT (context);

    if (scim_bridge_client_is_messenger_opened () && imcontext != NULL ) {
        if (context->actor != NULL) {
	    ClutterActor *stage = clutter_actor_get_stage (context->actor);
	    Window current_window, root, parent, *childs;
	    unsigned int nchild;
	    XWindowAttributes winattr;
	    Display *xdpy;
		gfloat new_window_x_float;
		gfloat new_window_y_float;
		int new_window_x;
		int new_window_y;

		clutter_actor_get_transformed_position (
			context->actor, &new_window_x_float, &new_window_y_float);
		new_window_x = (int)new_window_x_float;
		new_window_y = (int)new_window_y_float;

	    xdpy = clutter_x11_get_default_display ();
	    current_window = clutter_x11_get_stage_window(CLUTTER_STAGE(stage));

	    while(1) {
                XGetWindowAttributes (xdpy, current_window, &winattr);
                new_window_x += winattr.x;
                new_window_y += winattr.y;

                XQueryTree(xdpy, current_window, &root, &parent, &childs, &nchild);
                current_window = parent;
                if (root == parent)
                    break;
            }

            if (imcontext->window_x != new_window_x || imcontext->window_y != new_window_y) {
                imcontext->window_x = new_window_x;
                imcontext->window_y = new_window_y;

                scim_bridge_pdebugln (1,
                    "The cursor location is changed: x = %d + %d\ty = %d + %d",
                    imcontext->window_x, imcontext->cursor_x, imcontext->window_y, imcontext->cursor_y);

                if (set_cursor_location (imcontext, new_window_x, new_window_y, imcontext->cursor_x, imcontext->cursor_y)) {
                    scim_bridge_perrorln ("An IOException occurred at scim_bridge_client_imcontext_filter_key_event ()");
                    return clutter_im_context_filter_keypress (fallback_imcontext, event);
                }
            }
        }

        boolean consumed = FALSE;
        if (filter_key_event (imcontext, event, &consumed)) {
            scim_bridge_perrorln ("An IOException occurred at scim_bridge_client_imcontext_filter_key_event ()");
        } else if (consumed) {
            return TRUE;
        }
    }

    if (imcontext == NULL || !imcontext->enabled) {
        return clutter_im_context_filter_keypress (fallback_imcontext, event);
    }

    return FALSE;
}


void scim_bridge_client_imcontext_reset (ClutterIMContext *context)
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


void scim_bridge_client_imcontext_show(ClutterIMContext *context)
{
    scim_bridge_pdebugln (8, "scim_bridge_client_imcontext_show ()");

    ScimBridgeClientIMContext *imcontext = SCIM_BRIDGE_CLIENT_IMCONTEXT (context);

    if (scim_bridge_client_is_messenger_opened () && imcontext != NULL && !imcontext->enabled) {
		scim_bridge_client_imcontext_focus_in (context);
        if (scim_bridge_client_set_imcontext_enabled (imcontext, TRUE)) {
            scim_bridge_perrorln ("An IOException occurred at scim_bridge_client_imcontext_show ()");
        }
    }
}


void scim_bridge_client_imcontext_hide(ClutterIMContext *context)
{
    scim_bridge_pdebugln (8, "scim_bridge_client_imcontext_hide ()");

    ScimBridgeClientIMContext *imcontext = SCIM_BRIDGE_CLIENT_IMCONTEXT (context);

    if (scim_bridge_client_is_messenger_opened () && imcontext != NULL && imcontext->enabled) {
        if (scim_bridge_client_set_imcontext_enabled (imcontext, FALSE)) {
            scim_bridge_perrorln ("An IOException occurred at scim_bridge_client_imcontext_hide ()");
        }
    }

    scim_bridge_client_imcontext_focus_out (context);
}


void scim_bridge_client_imcontext_get_preedit_string (ClutterIMContext *context, gchar **str, PangoAttrList **pango_attrs, gint *cursor_pos)
{
    scim_bridge_pdebugln (4, "scim_bridge_client_imcontext_get_preedit_string ()");

    ScimBridgeClientIMContext *imcontext = SCIM_BRIDGE_CLIENT_IMCONTEXT (context);

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


void scim_bridge_client_imcontext_focus_in (ClutterIMContext *context)
{
    scim_bridge_pdebugln (8, "scim_bridge_client_imcontext_focus_in ()");

    ScimBridgeClientIMContext *imcontext = SCIM_BRIDGE_CLIENT_IMCONTEXT (context);

    if (focused_imcontext != NULL && focused_imcontext != imcontext) scim_bridge_client_imcontext_focus_out (CLUTTER_IM_CONTEXT (focused_imcontext));
    focused_imcontext = imcontext;

    if (!scim_bridge_client_is_messenger_opened () && scim_bridge_client_is_reconnection_enabled ()) {
        scim_bridge_pdebugln (7, "Trying to open the connection again...");
        scim_bridge_client_open_messenger ();
    }

    if (scim_bridge_client_is_messenger_opened () && imcontext != NULL) {
        if (scim_bridge_client_change_focus (imcontext, TRUE)) {
            scim_bridge_perrorln ("An IOException occurred at scim_bridge_client_imcontext_focus_in ()");
        }
    }
}


void scim_bridge_client_imcontext_focus_out (ClutterIMContext *context)
{
    scim_bridge_pdebugln (8, "scim_bridge_client_imcontext_focus_out ()");

    ScimBridgeClientIMContext *imcontext = SCIM_BRIDGE_CLIENT_IMCONTEXT (context);
    focused_actor = NULL;

    focused_imcontext = imcontext;
    if (imcontext->preedit_shown) {
        if (imcontext->enabled) {
            scim_bridge_client_imcontext_set_preedit_shown (imcontext, FALSE);
            scim_bridge_client_imcontext_update_preedit (imcontext);
        } else {
            clutter_im_context_reset (CLUTTER_IM_CONTEXT (fallback_imcontext));
        }
    }
    if (scim_bridge_client_is_messenger_opened () && imcontext != NULL) {
        if (scim_bridge_client_change_focus (imcontext, FALSE)) {
            scim_bridge_perrorln ("An IOException occurred at scim_bridge_client_imcontext_focus_out ()");
        }
    }

    focused_imcontext = NULL;
}


void scim_bridge_client_imcontext_set_client_stage (ClutterIMContext *context, ClutterStage *new_stage)
{
    scim_bridge_pdebugln (7, "scim_bridge_client_imcontext_set_client_stage ()");

    ScimBridgeClientIMContext *imcontext = SCIM_BRIDGE_CLIENT_IMCONTEXT (context);

    if (imcontext != NULL) {
        if (imcontext->client_stage != NULL) g_object_unref (imcontext->client_stage);
        imcontext->client_stage = new_stage;
        if (new_stage != NULL) {
            g_object_ref (new_stage);
        }
    }
}


void scim_bridge_client_imcontext_set_cursor_location (ClutterIMContext *context, ClutterIMRectangle *area)
{
    scim_bridge_pdebugln (4, "scim_bridge_client_imcontext_set_cursor_location ()");

    ScimBridgeClientIMContext *imcontext = SCIM_BRIDGE_CLIENT_IMCONTEXT (context);

    if (imcontext->preedit_cursor_flicking)
        return;

    if (imcontext != NULL && context->actor != NULL) {
        const int new_cursor_x = area->x + area->width;
        const int new_cursor_y = area->y + area->height + 8;

        if (set_cursor_location (imcontext, imcontext->window_x, imcontext->window_y, new_cursor_x, new_cursor_y)) {
            scim_bridge_perrorln ("An IOException occurred at scim_bridge_client_imcontext_set_cursor_location ()");
        }
    }
}


void scim_bridge_client_imcontext_set_preedit_enabled (ClutterIMContext *context, gboolean enabled)
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
