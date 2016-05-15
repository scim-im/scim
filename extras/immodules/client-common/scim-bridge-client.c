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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>

#include "scim-bridge-client.h"
#include "scim-bridge-client-imcontext.h"
#include "scim-bridge-client-protected.h"
#include "scim-bridge-message-constant.h"
#include "scim-bridge-messenger.h"
#include "scim-bridge-output.h"
#include "scim-bridge-path.h"
#include "scim-bridge-string.h"

/* Private data type */
typedef struct _IMContextListElement
{
    struct _IMContextListElement *prev;
    struct _IMContextListElement *next;

    ScimBridgeClientIMContext *imcontext;
} IMContextListElement;

typedef struct _IMContextList
{
    IMContextListElement *first;
    IMContextListElement *last;

    ScimBridgeClientIMContext *found_imcontext;

    size_t size;
} IMContextList;

typedef enum _scim_bridge_response_status
{
    RESPONSE_PENDING,
    RESPONSE_SUCCEEDED,
    RESPONSE_FAILED,
    RESPONSE_DONE,
} scim_bridge_response_status;

typedef struct _ScimBridgeResponse
{
    scim_bridge_response_status status;

    const char *header;

    boolean consumed;

    scim_bridge_imcontext_id_t imcontext_id;

} ScimBridgeResponse;

/* Private variables */
static ScimBridgeResponse pending_response;

static ScimBridgeMessenger *messenger = NULL;

static IMContextList imcontext_list;

static boolean initialized = FALSE;

/* Helper Functions */
static boolean check_scim_binary ()
{
    scim_bridge_pdebugln (1, "Checking SCIM binary...");
    
    FILE *pout = popen ("scim -h", "r");
    if (pout != NULL) {
        pclose (pout);
        return TRUE;
    } else {
        scim_bridge_perrorln("Error (%d): %s", errno, strerror (errno));
        return FALSE;
    }
}

static retval_t launch_agent ()
{
    scim_bridge_pdebugln (1, "Invoking the agent...");
    
    FILE *pout = popen (scim_bridge_path_get_agent (), "r");
    if (pout != NULL) {
        pclose (pout);
        return RETVAL_SUCCEEDED;
    } else {
        scim_bridge_perrorln ("Failed to invoking the agent: %s", strerror (errno));
        return RETVAL_FAILED;
    }
}


/* Message Handlers */
static retval_t received_message_unknown (const ScimBridgeMessage *message)
{
    const char *header = scim_bridge_message_get_header (message);
    scim_bridge_perror ("Unknown message: %s", header);
    
    int i;
    for (i = 0; i < scim_bridge_message_get_argument_count (message); ++i) {
        scim_bridge_perror (" %s", scim_bridge_message_get_argument (message, i));
    }
    scim_bridge_perrorln ("");

    // Just ignore it.
    return RETVAL_SUCCEEDED;
}


static retval_t received_message_imengine_status_changed (const ScimBridgeMessage *message)
{
    const char *header = scim_bridge_message_get_header (message);
    const char *ic_id_str = scim_bridge_message_get_argument (message, 0);
    const char *enabled_str = scim_bridge_message_get_argument (message, 1);

    scim_bridge_imcontext_id_t ic_id;
    boolean enabled;
    if (scim_bridge_string_to_int (&ic_id, ic_id_str) || scim_bridge_string_to_boolean (&enabled, enabled_str)) {
        scim_bridge_perrorln ("Invalid arguments for the message: %s (%s, %s)", header, ic_id_str, enabled_str);
    } else {
        ScimBridgeClientIMContext *imcontext = scim_bridge_client_find_imcontext (ic_id);
        if (imcontext != NULL) {
            scim_bridge_client_imcontext_imengine_status_changed (imcontext, enabled);
        } else {
            scim_bridge_perrorln ("No such imcontext: id = %d", ic_id);
        }
    }

    return RETVAL_SUCCEEDED;
}


static retval_t received_message_preedit_mode_changed (const ScimBridgeMessage *message)
{
    const char *header = scim_bridge_message_get_header (message);

    if (pending_response.status == RESPONSE_PENDING && strcmp (pending_response.header, header) == 0) {
        pending_response.status = RESPONSE_SUCCEEDED;
    } else {
        scim_bridge_perrorln ("The message is received in a wrong context: %s", header);
    }

    return RETVAL_SUCCEEDED;
}


static retval_t received_message_imcontext_registered (const ScimBridgeMessage *message)
{
    const char *header = scim_bridge_message_get_header (message);

    if (pending_response.status == RESPONSE_PENDING && strcmp (pending_response.header, header) == 0) {

        const char *ic_id_str = scim_bridge_message_get_argument (message, 0);
        int ic_id;
        if (scim_bridge_string_to_int (&ic_id, ic_id_str)) {
            scim_bridge_perrorln ("Invalid arguments for the message: %s (%s)", header, ic_id_str);
            pending_response.status = RESPONSE_FAILED;
        } else {
            if (ic_id < 0) {
                pending_response.status = RESPONSE_FAILED;
            } else {
                pending_response.imcontext_id = ic_id;
                pending_response.status = RESPONSE_SUCCEEDED;
            }
        }
    } else {
        scim_bridge_perrorln ("The message is received in a wrong context: %s", header);
    }

    return RETVAL_SUCCEEDED;
}


static retval_t received_message_imcontext_deregistered (const ScimBridgeMessage *message)
{
    const char *header = scim_bridge_message_get_header (message);

    if (pending_response.status == RESPONSE_PENDING && strcmp (pending_response.header, header) == 0) {
        pending_response.status = RESPONSE_SUCCEEDED;
    } else {
        scim_bridge_perrorln ("The message is received in a wrong context: %s", header);
    }

    return RETVAL_SUCCEEDED;
}


static retval_t received_message_imcontext_reseted (const ScimBridgeMessage *message)
{
    const char *header = scim_bridge_message_get_header (message);

    if (pending_response.status == RESPONSE_PENDING && strcmp (pending_response.header, header) == 0) {
        pending_response.status = RESPONSE_SUCCEEDED;
    } else {
        scim_bridge_perrorln ("The message is received in a wrong context: %s", header);
    }
    
    return RETVAL_SUCCEEDED;
}


static retval_t received_message_imcontext_enabled (const ScimBridgeMessage *message)
{
    const char *header = scim_bridge_message_get_header (message);

    if (pending_response.status == RESPONSE_PENDING && strcmp (pending_response.header, header) == 0) {
        pending_response.status = RESPONSE_SUCCEEDED;
    } else {
        scim_bridge_perrorln ("The message is received in a wrong context: %s", header);
    }

    return RETVAL_SUCCEEDED;
}


static retval_t received_message_imcontext_disabled (const ScimBridgeMessage *message)
{
    const char *header = scim_bridge_message_get_header (message);

    if (pending_response.status == RESPONSE_PENDING && strcmp (pending_response.header, header) == 0) {
        pending_response.status = RESPONSE_SUCCEEDED;
    } else {
        scim_bridge_perrorln ("The message is received in a wrong context: %s", header);
    }

    return RETVAL_SUCCEEDED;
}


static retval_t received_message_key_event_handled (const ScimBridgeMessage *message)
{
    const char *header = scim_bridge_message_get_header (message);

    if (pending_response.status == RESPONSE_PENDING && strcmp (pending_response.header, header) == 0) {

        const char *consumed_str = scim_bridge_message_get_argument (message, 0);
        boolean consumed;
        if (scim_bridge_string_to_boolean (&consumed, consumed_str)) {
            scim_bridge_perrorln ("Invalid arguments for the message: %s (%s)", header, consumed_str);
            pending_response.status = RESPONSE_FAILED;
        } else {
            pending_response.consumed = consumed;
            pending_response.status = RESPONSE_SUCCEEDED;
        }
    } else {
        scim_bridge_perrorln ("The message is received in a wrong context: %s", header);
    }

    return RETVAL_SUCCEEDED;
}


static retval_t received_message_focus_changed (const ScimBridgeMessage *message)
{
    const char *header = scim_bridge_message_get_header (message);

    if (pending_response.status == RESPONSE_PENDING && strcmp (pending_response.header, header) == 0) {
        pending_response.status = RESPONSE_SUCCEEDED;
    } else {
        scim_bridge_perrorln ("The message is received in a wrong context: %s", header);
    }

    return RETVAL_SUCCEEDED;
}



static retval_t received_message_cursor_location_changed (const ScimBridgeMessage *message)
{
    return RETVAL_SUCCEEDED;
}


static retval_t received_message_set_commit_string (const ScimBridgeMessage *message)
{
    const char *header = scim_bridge_message_get_header (message);
    const char *ic_id_str = scim_bridge_message_get_argument (message, 0);
    const char *string = scim_bridge_message_get_argument (message, 1);

    scim_bridge_imcontext_id_t ic_id;
    if (scim_bridge_string_to_int (&ic_id, ic_id_str)) {
        scim_bridge_perrorln ("Invalid arguments for the message: %s (%s, %s)", header, ic_id_str, string);
    } else {
        ScimBridgeClientIMContext *imcontext = scim_bridge_client_find_imcontext (ic_id);
        if (imcontext != NULL) {
            scim_bridge_client_imcontext_set_commit_string (imcontext, string ? string:"");
        } else {
            scim_bridge_perrorln ("No such imcontext: id = %d", ic_id);
        }
    }

    return RETVAL_SUCCEEDED;
}


static retval_t received_message_commit_string (const ScimBridgeMessage *message)
{
    const char *header = scim_bridge_message_get_header (message);
    const char *ic_id_str = scim_bridge_message_get_argument (message, 0);

    scim_bridge_imcontext_id_t ic_id;
    if (scim_bridge_string_to_int (&ic_id, ic_id_str)) {
        scim_bridge_perrorln ("Invalid arguments for the message: %s (%s)", header, ic_id_str);
    } else {
        ScimBridgeClientIMContext *imcontext = scim_bridge_client_find_imcontext (ic_id);
        if (imcontext != NULL) {
            scim_bridge_client_imcontext_commit (imcontext);
        } else {
            scim_bridge_perrorln ("No such imcontext: id = %d", ic_id);
        }
    }

    if (!scim_bridge_client_is_messenger_opened ()) {
        scim_bridge_perrorln ("The messenger is closed");
        return RETVAL_FAILED;
    }

    ScimBridgeMessage *responsive_message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_STRING_COMMITED, 0);

    scim_bridge_messenger_push_message (messenger, responsive_message);
    scim_bridge_free_message (responsive_message);

    while (scim_bridge_messenger_get_sending_buffer_size (messenger) > 0) {
        if (scim_bridge_messenger_send_message (messenger, NULL)) {
            scim_bridge_perrorln ("Failed to send a message at received_message_commit_string ()");
            return RETVAL_FAILED;
        }
    }
    return RETVAL_SUCCEEDED;
}


static retval_t received_message_set_preedit_shown (const ScimBridgeMessage *message)
{
    const char *header = scim_bridge_message_get_header (message);
    const char *ic_id_str = scim_bridge_message_get_argument (message, 0);
    const char *shown_str = scim_bridge_message_get_argument (message, 1);

    scim_bridge_imcontext_id_t ic_id;
    boolean shown;
    if (scim_bridge_string_to_int (&ic_id, ic_id_str) || scim_bridge_string_to_boolean (&shown, shown_str)) {
        scim_bridge_perrorln ("Invalid arguments for the message: %s (%s, %s)", header, ic_id_str, shown_str);
    } else {
        ScimBridgeClientIMContext *imcontext = scim_bridge_client_find_imcontext (ic_id);
        if (imcontext != NULL) {
            scim_bridge_client_imcontext_set_preedit_shown (imcontext, shown);
        } else {
            scim_bridge_perrorln ("No such imcontext: id = %d", ic_id);
        }
    }

    return RETVAL_SUCCEEDED;
}


static retval_t received_message_set_preedit_string (const ScimBridgeMessage *message)
{
    const char *header = scim_bridge_message_get_header (message);
    const char *ic_id_str = scim_bridge_message_get_argument (message, 0);
    const char *preedit_string = scim_bridge_message_get_argument (message, 1);

    scim_bridge_imcontext_id_t ic_id;
    if (scim_bridge_string_to_int (&ic_id, ic_id_str)) {
        scim_bridge_perrorln ("Invalid arguments for the message: %s (%s, %s)", header, ic_id_str, preedit_string);
    } else {
        ScimBridgeClientIMContext *imcontext = scim_bridge_client_find_imcontext (ic_id);

        if (imcontext != NULL) {
            scim_bridge_client_imcontext_set_preedit_string (imcontext, preedit_string ? preedit_string : "");
        } else {
            scim_bridge_perrorln ("No such imcontext: id = %d", ic_id);
        }
    }

    return RETVAL_SUCCEEDED;
}


static retval_t received_message_set_preedit_attributes (const ScimBridgeMessage *message)
{
    const char *header = scim_bridge_message_get_header (message);
    const char *ic_id_str = scim_bridge_message_get_argument (message, 0);

    scim_bridge_imcontext_id_t ic_id;
    if (scim_bridge_string_to_int (&ic_id, ic_id_str) || scim_bridge_message_get_argument_count (message) % 4 != 1) {
        scim_bridge_perrorln ("Invalid arguments for the message: %s (%s,...)", header, ic_id_str);
    } else {
        const size_t attribute_count = (scim_bridge_message_get_argument_count (message) - 1) / 4;
        ScimBridgeAttribute **attributes = alloca (sizeof (ScimBridgeAttribute*) * attribute_count);

        int i;
        for (i = 0; i < attribute_count; ++i) {
            attributes[i] = scim_bridge_alloc_attribute ();
            ScimBridgeAttribute *attribute = attributes[i];

            const char *attribute_begin_str = scim_bridge_message_get_argument (message, i * 4 + 1);
            const char *attribute_end_str = scim_bridge_message_get_argument (message, i * 4 + 2);
            const char *attribute_type_str = scim_bridge_message_get_argument (message, i * 4 + 3);
            const char *attribute_value_str = scim_bridge_message_get_argument (message, i * 4 + 4);

            unsigned int attribute_begin;
            unsigned int attribute_end;
            if (scim_bridge_string_to_uint (&attribute_begin, attribute_begin_str) || scim_bridge_string_to_uint (&attribute_end, attribute_end_str)) {
                scim_bridge_perrorln ("Invalid range for an attribute: begin = \"%s\", end = \"%s\"", attribute_begin_str, attribute_end_str);
                scim_bridge_attribute_set_begin (attribute, 0);
                scim_bridge_attribute_set_end (attribute, 0);
                scim_bridge_attribute_set_type (attribute, ATTRIBUTE_NONE);
                scim_bridge_attribute_set_value (attribute, SCIM_BRIDGE_ATTRIBUTE_DECORATE_NONE);
                continue;
            }

            scim_bridge_attribute_set_begin (attribute, attribute_begin);
            scim_bridge_attribute_set_end (attribute, attribute_end);

            if (strcmp (attribute_type_str, SCIM_BRIDGE_MESSAGE_NONE) == 0) {
                scim_bridge_attribute_set_type (attribute, ATTRIBUTE_NONE);
            } else if (strcmp (attribute_type_str, SCIM_BRIDGE_MESSAGE_DECORATE) == 0) {
                scim_bridge_attribute_set_type (attribute, ATTRIBUTE_DECORATE);

                if (strcmp (attribute_value_str, SCIM_BRIDGE_MESSAGE_HIGHLIGHT) == 0) {
                    scim_bridge_attribute_set_value (attribute, SCIM_BRIDGE_ATTRIBUTE_DECORATE_HIGHLIGHT);
                } else if (strcmp (attribute_value_str, SCIM_BRIDGE_MESSAGE_UNDERLINE) == 0) {
                    scim_bridge_attribute_set_value (attribute, SCIM_BRIDGE_ATTRIBUTE_DECORATE_UNDERLINE);
                } else if (strcmp (attribute_value_str, SCIM_BRIDGE_MESSAGE_REVERSE) == 0) {
                    scim_bridge_attribute_set_value (attribute, SCIM_BRIDGE_ATTRIBUTE_DECORATE_REVERSE);
                } else {
                    scim_bridge_perrorln ("Unknown decoration for the attribute: %s", attribute_value_str);
                    scim_bridge_attribute_set_type (attribute, ATTRIBUTE_NONE);
                    scim_bridge_attribute_set_value (attribute, SCIM_BRIDGE_ATTRIBUTE_DECORATE_NONE);
                }

            } else if (strcmp (attribute_type_str, SCIM_BRIDGE_MESSAGE_FOREGROUND) == 0
                || strcmp (attribute_type_str, SCIM_BRIDGE_MESSAGE_BACKGROUND) == 0) {

                if (strcmp (attribute_type_str, SCIM_BRIDGE_MESSAGE_FOREGROUND) == 0) {
                    scim_bridge_attribute_set_type (attribute, ATTRIBUTE_FOREGROUND);
                } else {
                    scim_bridge_attribute_set_type (attribute, ATTRIBUTE_BACKGROUND);
                }

                unsigned int color_val = 0x1000000;
                if (strncmp (attribute_value_str, SCIM_BRIDGE_MESSAGE_COLOR, strlen (SCIM_BRIDGE_MESSAGE_COLOR)) == 0) {
                    const char *color_str = attribute_value_str + strlen (SCIM_BRIDGE_MESSAGE_COLOR);
                    const size_t color_str_length = strlen (color_str);

                    if (color_str_length == 6) {
                        color_val = 0;
                        int j;
                        for (j = 0; j < 6; ++j) {
                            color_val <<= 4;
                            switch (color_str[j]) {
                                case '0':
                                    color_val += 0;
                                    break;
                                case '1':
                                    color_val += 1;
                                    break;
                                case '2':
                                    color_val += 2;
                                    break;
                                case '3':
                                    color_val += 3;
                                    break;
                                case '4':
                                    color_val += 4;
                                    break;
                                case '5':
                                    color_val += 5;
                                    break;
                                case '6':
                                    color_val += 6;
                                    break;
                                case '7':
                                    color_val += 7;
                                    break;
                                case '8':
                                    color_val += 8;
                                    break;
                                case '9':
                                    color_val += 9;
                                    break;
                                case 'a':
                                case 'A':
                                    color_val += 10;
                                    break;
                                case 'b':
                                case 'B':
                                    color_val += 11;
                                    break;
                                case 'c':
                                case 'C':
                                    color_val += 12;
                                    break;
                                case 'd':
                                case 'D':
                                    color_val += 13;
                                    break;
                                case 'e':
                                case 'E':
                                    color_val += 14;
                                    break;
                                case 'f':
                                case 'F':
                                    color_val += 15;
                                    break;
                                default:
                                    color_val = 0x1000000;
                                    break;
                            }
                        }
                    }
                }

                if (color_val > 0xFFFFFF) {
                    scim_bridge_perrorln ("An invalid string for a color: %s", attribute_value_str);
                    scim_bridge_attribute_set_type (attribute, ATTRIBUTE_NONE);
                    scim_bridge_attribute_set_value (attribute, SCIM_BRIDGE_ATTRIBUTE_DECORATE_NONE);
                } else {
                    const unsigned int red = (color_val & 0xFF0000) >> 16;
                    const unsigned int green = (color_val & 0x00FF00) >> 8;
                    const unsigned int blue = (color_val & 0x0000FF) >> 0;

                    scim_bridge_attribute_set_color (attribute, red, green, blue);
                }
            }
        }

        ScimBridgeClientIMContext *imcontext = scim_bridge_client_find_imcontext (ic_id);
        if (imcontext != NULL) {
            scim_bridge_client_imcontext_set_preedit_attributes (imcontext, attributes, attribute_count);
        } else {
            scim_bridge_perrorln ("No such imcontext: id = %d", ic_id);
        }

        int j;
        for (j = 0; j < attribute_count; ++j) {
            scim_bridge_free_attribute (attributes[j]);
        }
    }

    return RETVAL_SUCCEEDED;
}


static retval_t received_message_set_preedit_cursor_position (const ScimBridgeMessage *message)
{
    const char *header = scim_bridge_message_get_header (message);
    const char *ic_id_str = scim_bridge_message_get_argument (message, 0);
    const char *cursor_position_str = scim_bridge_message_get_argument (message, 1);

    scim_bridge_imcontext_id_t ic_id;
    unsigned int cursor_position;
    if (scim_bridge_string_to_int (&ic_id, ic_id_str) || scim_bridge_string_to_uint (&cursor_position, cursor_position_str)) {
        scim_bridge_perrorln ("Invalid arguments for the message: %s (%s, %s)", header, ic_id_str, cursor_position_str);
    } else {
        ScimBridgeClientIMContext *imcontext = scim_bridge_client_find_imcontext (ic_id);

        if (imcontext != NULL) {
            scim_bridge_client_imcontext_set_preedit_cursor_position (imcontext, cursor_position);
        } else {
            scim_bridge_perrorln ("No such imcontext: id = %d", ic_id);
        }
    }

    return RETVAL_SUCCEEDED;
}


static retval_t received_message_update_preedit (const ScimBridgeMessage *message)
{
    const char *header = scim_bridge_message_get_header (message);
    const char *ic_id_str = scim_bridge_message_get_argument (message, 0);

    int ic_id;
    if (scim_bridge_string_to_int (&ic_id, ic_id_str)) {
        scim_bridge_perrorln ("Invalid arguments for the message: %s (%s)", header, ic_id_str);
    } else {
        ScimBridgeClientIMContext *imcontext = scim_bridge_client_find_imcontext (ic_id);

        if (imcontext != NULL) {
            scim_bridge_client_imcontext_update_preedit (imcontext);
        } else {
            scim_bridge_perrorln ("No such imcontext: id = %d", ic_id);
        }
    }

    if (!scim_bridge_client_is_messenger_opened ()) {
        scim_bridge_perrorln ("The messenger is closed");
        return RETVAL_FAILED;
    }

    ScimBridgeMessage *responsive_message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_PREEDIT_UPDATED, 0);

    scim_bridge_messenger_push_message (messenger, responsive_message);
    scim_bridge_free_message (responsive_message);

    while (scim_bridge_messenger_get_sending_buffer_size (messenger) > 0) {
        if (scim_bridge_messenger_send_message (messenger, NULL)) {
            scim_bridge_perrorln ("Failed to send a message at received_message_update_preedit ()");
            return RETVAL_FAILED;
        }
    }
    return RETVAL_SUCCEEDED;
}


static retval_t received_message_beep (const ScimBridgeMessage *message)
{
    const char *header = scim_bridge_message_get_header (message);
    const char *ic_id_str = scim_bridge_message_get_argument (message, 0);

    int ic_id;
    if (scim_bridge_string_to_int (&ic_id, ic_id_str)) {
        scim_bridge_perrorln ("Invalid arguments for the message: %s (%s)", header, ic_id_str);
    } else {
        ScimBridgeClientIMContext *imcontext = scim_bridge_client_find_imcontext (ic_id);

        if (imcontext != NULL) {
            scim_bridge_client_imcontext_beep (imcontext);
        } else {
            scim_bridge_perrorln ("No such imcontext: id = %d", ic_id);
        }
    }

    return RETVAL_SUCCEEDED;
}


static retval_t received_message_forward_key_event (const ScimBridgeMessage *message)
{
    const char *header = scim_bridge_message_get_header (message);
    const char *ic_id_str = scim_bridge_message_get_argument (message, 0);
    const char *key_code_str = scim_bridge_message_get_argument (message, 1);
    const char *key_pressed_str = scim_bridge_message_get_argument (message, 2);

    const int modifier_count = scim_bridge_message_get_argument_count (message) - 3;

    boolean shift_down = FALSE;
    boolean control_down = FALSE;
    boolean alt_down = FALSE;
    boolean meta_down = FALSE;
    boolean super_down = FALSE;
    boolean hyper_down = FALSE;
    boolean caps_lock_down = FALSE;
    boolean num_lock_down = FALSE;
    boolean unknown_down = FALSE;

    boolean kana_ro = FALSE;

    int i;
    for (i = 0; i < modifier_count; ++i) {
        const char *modifier_str = scim_bridge_message_get_argument (message, i + 3);

        if (strcmp (modifier_str, SCIM_BRIDGE_MESSAGE_SHIFT) == 0) {
            shift_down = TRUE;
        } else if (strcmp (modifier_str, SCIM_BRIDGE_MESSAGE_CONTROL) == 0) {
            control_down = TRUE;
        } else if (strcmp (modifier_str, SCIM_BRIDGE_MESSAGE_ALT) == 0) {
            alt_down = TRUE;
        } else if (strcmp (modifier_str, SCIM_BRIDGE_MESSAGE_META) == 0) {
            meta_down = TRUE;
        } else if (strcmp (modifier_str, SCIM_BRIDGE_MESSAGE_SUPER) == 0) {
            super_down = TRUE;
        } else if (strcmp (modifier_str, SCIM_BRIDGE_MESSAGE_HYPER) == 0) {
            hyper_down = TRUE;
        } else if (strcmp (modifier_str, SCIM_BRIDGE_MESSAGE_CAPS_LOCK) == 0) {
            caps_lock_down = TRUE;
        } else if (strcmp (modifier_str, SCIM_BRIDGE_MESSAGE_NUM_LOCK) == 0) {
            num_lock_down = TRUE;
        } else if (strcmp (modifier_str, SCIM_BRIDGE_MESSAGE_KANA_RO) == 0) {
            kana_ro = TRUE;
        } else {
            scim_bridge_perrorln ("Unknown modifier: %s", modifier_str);
            unknown_down = TRUE;
        }
    }

    scim_bridge_imcontext_id_t ic_id;
    unsigned int key_code;
    boolean key_pressed;
    if (scim_bridge_string_to_int (&ic_id, ic_id_str) || scim_bridge_string_to_uint (&key_code, key_code_str)
        || scim_bridge_string_to_boolean (&key_pressed, key_pressed_str)) {

        scim_bridge_perror ("Invalid arguments for the message: %s (%s, %s, %s", header, ic_id_str, key_code_str, key_pressed_str);
    
        boolean first_modifier = TRUE;
        
        if (shift_down) {
            if (first_modifier) {
                scim_bridge_perror (", ");
                first_modifier = FALSE;
            } else {
                scim_bridge_perror (" + ");
            }
            scim_bridge_perror ("%s", SCIM_BRIDGE_MESSAGE_SHIFT);
        }
        if (control_down) {
            if (first_modifier) {
                scim_bridge_perror (", ");
                first_modifier = FALSE;
            } else {
                scim_bridge_perror (" + ");
            }
            scim_bridge_perror ("%s", SCIM_BRIDGE_MESSAGE_CONTROL);
        }
        if (alt_down) {
            if (first_modifier) {
                scim_bridge_perror (", ");
                first_modifier = FALSE;
            } else {
                scim_bridge_perror (" + ");
            }
            scim_bridge_perror ("%s", SCIM_BRIDGE_MESSAGE_ALT);
        }
        if (meta_down) {
            if (first_modifier) {
                scim_bridge_perror (", ");
                first_modifier = FALSE;
            } else {
                scim_bridge_perror (" + ");
            }
            scim_bridge_perror ("%s", SCIM_BRIDGE_MESSAGE_META);
        }
        if (super_down) {
            if (first_modifier) {
                scim_bridge_perror (", ");
                first_modifier = FALSE;
            } else {
                scim_bridge_perror (" + ");
            }
            scim_bridge_perror ("%s", SCIM_BRIDGE_MESSAGE_SUPER);
        }
        if (hyper_down) {
            if (first_modifier) {
                scim_bridge_perror (", ");
                first_modifier = FALSE;
            } else {
                scim_bridge_perror (" + ");
            }
            scim_bridge_perror ("%s", SCIM_BRIDGE_MESSAGE_HYPER);
        }
        if (caps_lock_down) {
            if (first_modifier) {
                scim_bridge_perror (", ");
                first_modifier = FALSE;
            } else {
                scim_bridge_perror (" + ");
            }
            scim_bridge_perror ("%s", SCIM_BRIDGE_MESSAGE_CAPS_LOCK);
        }
        if (num_lock_down) {
            if (first_modifier) {
                scim_bridge_perror (", ");
                first_modifier = FALSE;
            } else {
                scim_bridge_perror (" + ");
            }
            scim_bridge_perror ("%s", SCIM_BRIDGE_MESSAGE_NUM_LOCK);
        }
        if (kana_ro) {
            if (first_modifier) {
                scim_bridge_perror (", ");
                first_modifier = FALSE;
            } else {
                scim_bridge_perror (" + ");
            }
            scim_bridge_perror ("%s", SCIM_BRIDGE_MESSAGE_KANA_RO);
        }
        if (unknown_down) {
            if (first_modifier) {
                scim_bridge_perror (", ");
                first_modifier = FALSE;
            } else {
                scim_bridge_perror (" + ");
            }
            scim_bridge_perror ("%s", SCIM_BRIDGE_MESSAGE_UNKNOWN);
        }
        scim_bridge_perrorln (")");
        
    } else {
        ScimBridgeClientIMContext *imcontext = scim_bridge_client_find_imcontext (ic_id);

        if (imcontext != NULL) {
            ScimBridgeKeyEvent *key_event = scim_bridge_alloc_key_event ();
            scim_bridge_key_event_set_code (key_event, key_code);
            scim_bridge_key_event_set_pressed (key_event, key_pressed);
            scim_bridge_key_event_set_shift_down (key_event, shift_down);
            scim_bridge_key_event_set_control_down (key_event, control_down);
            scim_bridge_key_event_set_alt_down (key_event, alt_down);
            scim_bridge_key_event_set_meta_down (key_event, meta_down);
            scim_bridge_key_event_set_super_down (key_event, super_down);
            scim_bridge_key_event_set_hyper_down (key_event, hyper_down);
            scim_bridge_key_event_set_caps_lock_down (key_event, caps_lock_down);
            scim_bridge_key_event_set_num_lock_down (key_event, num_lock_down);
            
            scim_bridge_key_event_set_quirk_enabled (key_event, SCIM_BRIDGE_KEY_QUIRK_KANA_RO, kana_ro);

            scim_bridge_client_imcontext_forward_key_event (imcontext, key_event);
            scim_bridge_free_key_event (key_event);
        } else {
            scim_bridge_perrorln ("No such imcontext: id = %d", ic_id);
        }
    }

    return RETVAL_SUCCEEDED;
}


static retval_t received_message_get_surrounding_text (const ScimBridgeMessage *message)
{
    const char *header = scim_bridge_message_get_header (message);
    const char *ic_id_str = scim_bridge_message_get_argument (message, 0);
    const char *before_max_str = scim_bridge_message_get_argument (message, 1);
    const char *after_max_str = scim_bridge_message_get_argument (message, 2);

    int ic_id;
    unsigned int before_max;
    unsigned int after_max;

    char *string;
    int cursor_position;

    boolean succeeded = FALSE;
    if (scim_bridge_string_to_int (&ic_id, ic_id_str)
        || scim_bridge_string_to_uint (&before_max, before_max_str) || scim_bridge_string_to_uint (&after_max, after_max_str)) {
        scim_bridge_perrorln ("Invalid arguments for the message: %s (%s, %s, %s)", header, ic_id_str, before_max_str, after_max_str);
    } else {
        ScimBridgeClientIMContext *imcontext = scim_bridge_client_find_imcontext (ic_id);

        if (imcontext != NULL) {
            succeeded = scim_bridge_client_imcontext_get_surrounding_text (imcontext, before_max, after_max, &string, &cursor_position);
            if (succeeded) {
                scim_bridge_pdebugln (5, "surrounding text = '%s', cursor_position = %d", string, cursor_position);
            } else {
                scim_bridge_pdebugln (5, "surrounding text = N/A");
            }
        } else {
    
            scim_bridge_perrorln ("No such imcontext: id = %d", ic_id);
        }
    }

    if (!scim_bridge_client_is_messenger_opened ()) {
        scim_bridge_perrorln ("The messenger is closed");
        return RETVAL_FAILED;
    }

    ScimBridgeMessage *responsive_message;

    if (succeeded) {
        responsive_message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_SURROUNDING_TEXT_GOTTEN, 3);
        scim_bridge_message_set_argument (responsive_message, 0, SCIM_BRIDGE_MESSAGE_TRUE);

        char *cursor_position_str = alloca (sizeof (char) * (cursor_position / 10 + 2));
        scim_bridge_string_from_uint (&cursor_position_str, cursor_position);
        scim_bridge_message_set_argument (responsive_message, 1, cursor_position_str);

        scim_bridge_message_set_argument (responsive_message, 2, string);
    } else {
        responsive_message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_SURROUNDING_TEXT_GOTTEN, 2);
        scim_bridge_message_set_argument (responsive_message, 0, SCIM_BRIDGE_MESSAGE_FALSE);
    }

    scim_bridge_messenger_push_message (messenger, responsive_message);
    scim_bridge_free_message (responsive_message);

    while (scim_bridge_messenger_get_sending_buffer_size (messenger) > 0) {
        if (scim_bridge_messenger_send_message (messenger, NULL)) {
            scim_bridge_perrorln ("Failed to send a message at received_message_get_surrounding_text ()");
            return RETVAL_FAILED;
        }
    }
    return RETVAL_SUCCEEDED;
}


static retval_t received_message_delete_surrounding_text (const ScimBridgeMessage *message)
{
    const char *header = scim_bridge_message_get_header (message);
    const char *ic_id_str = scim_bridge_message_get_argument (message, 0);
    const char *offset_str = scim_bridge_message_get_argument (message, 1);
    const char *length_str = scim_bridge_message_get_argument (message, 2);

    int ic_id;
    int offset;
    unsigned int length;

    boolean succeeded = FALSE;
    if (scim_bridge_string_to_int (&ic_id, ic_id_str) || scim_bridge_string_to_int (&offset, offset_str) || scim_bridge_string_to_uint (&length, length_str)) {
        scim_bridge_perrorln ("Invalid arguments for the message: %s (%s, %s, %s)", header, ic_id_str, offset_str, length_str);
    } else {
        ScimBridgeClientIMContext *imcontext = scim_bridge_client_find_imcontext (ic_id);

        if (imcontext != NULL) {
            succeeded = scim_bridge_client_imcontext_delete_surrounding_text (imcontext, offset, length);
        } else {
            scim_bridge_perrorln ("No such imcontext: id = %d", ic_id);
        }
    }

    if (!scim_bridge_client_is_messenger_opened ()) {
        scim_bridge_perrorln ("The messenger is closed");
        return RETVAL_FAILED;
    }

    ScimBridgeMessage *responsive_message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_SURROUNDING_TEXT_DELETED, 1);

    if (succeeded) {
        scim_bridge_message_set_argument (responsive_message, 0, SCIM_BRIDGE_MESSAGE_TRUE);
    } else {
        scim_bridge_message_set_argument (responsive_message, 0, SCIM_BRIDGE_MESSAGE_FALSE);
    }

    scim_bridge_messenger_push_message (messenger, responsive_message);
    scim_bridge_free_message (responsive_message);

    while (scim_bridge_messenger_get_sending_buffer_size (messenger) > 0) {
        if (scim_bridge_messenger_send_message (messenger, NULL)) {
            scim_bridge_perrorln ("Failed to send a message at received_message_delete_surrounding_text ()");
            return RETVAL_FAILED;
        }
    }
    return RETVAL_SUCCEEDED;
}


static retval_t received_message_replace_surrounding_text (const ScimBridgeMessage *message)
{
    const char *header = scim_bridge_message_get_header (message);
    const char *ic_id_str = scim_bridge_message_get_argument (message, 0);
    const char *cursor_position_str = scim_bridge_message_get_argument (message, 1);
    const char *string = scim_bridge_message_get_argument (message, 2);

    int ic_id;
    int cursor_position;

    boolean succeeded = FALSE;
    if (scim_bridge_string_to_int (&ic_id, ic_id_str) || scim_bridge_string_to_int (&cursor_position, cursor_position_str)) {
        scim_bridge_perrorln ("Invalid arguments for the message: %s (%s, %s, %s)", header, ic_id_str, cursor_position, string);
    } else {
        ScimBridgeClientIMContext *imcontext = scim_bridge_client_find_imcontext (ic_id);

        if (imcontext != NULL) {
            succeeded = scim_bridge_client_imcontext_replace_surrounding_text (imcontext, cursor_position, string);
        } else {
            scim_bridge_perrorln ("No such imcontext: id = %d", ic_id);
        }
    }

    if (!scim_bridge_client_is_messenger_opened ()) {
        scim_bridge_perrorln ("The messenger is closed");
        return RETVAL_FAILED;
    }

    ScimBridgeMessage *responsive_message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_SURROUNDING_TEXT_REPLACED, 1);

    if (succeeded) {
        scim_bridge_message_set_argument (responsive_message, 0, SCIM_BRIDGE_MESSAGE_TRUE);
    } else {
        scim_bridge_message_set_argument (responsive_message, 0, SCIM_BRIDGE_MESSAGE_FALSE);
    }

    scim_bridge_messenger_push_message (messenger, responsive_message);
    scim_bridge_free_message (responsive_message);

    while (scim_bridge_messenger_get_sending_buffer_size (messenger) > 0) {
        if (scim_bridge_messenger_send_message (messenger, NULL)) {
            scim_bridge_perrorln ("Failed to send a message at received_message_replace_surrounding_text ()");
            return RETVAL_FAILED;
        }
    }
    return RETVAL_SUCCEEDED;
}


/* Public functions */
retval_t scim_bridge_client_initialize ()
{
    scim_bridge_pdebugln (5, "scim_bridge_client_initialize ()");
    if (initialized) return RETVAL_SUCCEEDED;

    initialized = TRUE;

    messenger = NULL;

    imcontext_list.first = NULL;
    imcontext_list.last = NULL;
    imcontext_list.found_imcontext = NULL;
    imcontext_list.size = 0;

    return RETVAL_SUCCEEDED;
}


retval_t scim_bridge_client_finalize ()
{
    scim_bridge_pdebugln (5, "scim_bridge_client_finalize ()");
    if (!initialized) return RETVAL_SUCCEEDED;

    if (messenger != NULL) scim_bridge_client_close_messenger ();
    messenger = NULL;

    IMContextListElement *i = imcontext_list.first;
    while (i != NULL) {
        IMContextListElement *j = i;
        i = i->next;
        free (j);
    }
    imcontext_list.first = NULL;
    imcontext_list.last = NULL;
    imcontext_list.found_imcontext = NULL;
    imcontext_list.size = 0;

    initialized = FALSE;
    return RETVAL_SUCCEEDED;
}


boolean scim_bridge_client_is_initialized ()
{
    return initialized;
}


retval_t scim_bridge_client_open_messenger ()
{
    scim_bridge_pdebugln (8, "scim_bridge_client_open_messenger ()");

    if (!initialized) {
        scim_bridge_perrorln ("The client has not been initialized");
        return RETVAL_FAILED;
    }

    if (messenger != NULL) {
        scim_bridge_perrorln ("The messenger has already been opend");
        return RETVAL_SUCCEEDED;
    }
    
    if (check_scim_binary () != TRUE) {
        scim_bridge_perrorln ("There is no SCIM binary");
        return RETVAL_FAILED;
    }

    int i;
    for (i = 0; i < 10; ++i) {
        int socket_fd = socket (PF_UNIX, SOCK_STREAM, 0);
        if (socket_fd < 0) {
            scim_bridge_perrorln ("Failed to create the message socket: %s", strerror (errno));
            return RETVAL_FAILED;
        }

        struct sockaddr_un socket_addr;
        memset (&socket_addr, 0, sizeof (struct sockaddr_un));
        socket_addr.sun_family = AF_UNIX;
        strcpy (socket_addr.sun_path, scim_bridge_path_get_socket ());

        if (connect (socket_fd, (struct sockaddr*)&socket_addr, SUN_LEN(&socket_addr))) {
            if (i == 5 && launch_agent ()) {
                scim_bridge_perrorln ("Cannot launch the agent");
                return RETVAL_FAILED;
            } else {
                scim_bridge_pdebugln (8, "Failed to connect the message socket: %s", strerror (errno));
                close (socket_fd);
                usleep (5000);
            }
        } else {
            messenger = scim_bridge_alloc_messenger (socket_fd);
            pending_response.consumed = TRUE;
            pending_response.header = NULL;
            pending_response.imcontext_id = -1;
            pending_response.status = RESPONSE_DONE;

            IMContextListElement *first = imcontext_list.first;
            IMContextListElement *last = imcontext_list.last;
            size_t size = imcontext_list.size = imcontext_list.size;

            imcontext_list.first = NULL;
            imcontext_list.last = NULL;
            imcontext_list.size = 0;
            imcontext_list.found_imcontext = NULL;

            while (first != NULL) {
                if (scim_bridge_client_register_imcontext (first->imcontext)) {
                    scim_bridge_perrorln ("Cannot register the IMContexts...");
                    
                    first->prev = imcontext_list.last;
                    if (imcontext_list.last != NULL) {
                        imcontext_list.last->next = first;
                        imcontext_list.last = last;
                    } else {
                        imcontext_list.first = first;
                        imcontext_list.last = last;
                    }
                    imcontext_list.size += size;

                    IMContextListElement *i;
                    for (i = imcontext_list.first; i != NULL; i = i->next) {
                        scim_bridge_client_imcontext_set_id (i->imcontext, -1);
                    }

                    return RETVAL_FAILED;
                } else {
                    IMContextListElement *i = first;
                    first = first->next;
                    free (i);
                    --size;
                }
            }
            
            scim_bridge_client_messenger_opened ();
            return RETVAL_SUCCEEDED;
        }
    }

    scim_bridge_perrorln ("Failed to establish the connection: %s", strerror (errno));
    return RETVAL_FAILED;
}


retval_t scim_bridge_client_close_messenger ()
{
    scim_bridge_pdebugln (8, "scim_bridge_client_close_messenger ()");

    if (messenger == NULL) {
        scim_bridge_perrorln ("The messenger is closed");
        return RETVAL_SUCCEEDED;
    }

    scim_bridge_free_messenger (messenger);
    messenger = NULL;

    pending_response.consumed = FALSE;
    pending_response.imcontext_id = -1;
    pending_response.status = RESPONSE_DONE;

    IMContextListElement *i;
    for (i = imcontext_list.first; i != NULL; i = i->next) {
        scim_bridge_client_imcontext_set_id (i->imcontext, -1);
    }

    scim_bridge_client_messenger_closed ();

    return RETVAL_SUCCEEDED;
}


boolean scim_bridge_client_is_messenger_opened ()
{
    scim_bridge_pdebugln (3, "scim_bridge_client_is_messenger_opened ()");

    return messenger != NULL;
}


int scim_bridge_client_get_messenger_fd ()
{
    if (messenger == NULL) {
        return -1;
    } else {
        return  scim_bridge_messenger_get_socket_fd (messenger);
    }
}


boolean scim_bridge_client_is_reconnection_enabled ()
{
    static boolean first_time = TRUE;
    static boolean reconnection_enabled = TRUE;
    
    if (first_time) {
        char *env_reconnection_enabled = getenv ("SCIM_BRIDGE_RECONNECTION_ENABLED");
        if (env_reconnection_enabled != NULL) scim_bridge_string_to_boolean (&reconnection_enabled, env_reconnection_enabled);
        first_time = FALSE;
    }

    return reconnection_enabled;
}

ScimBridgeClientIMContext *scim_bridge_client_find_imcontext (scim_bridge_imcontext_id_t id)
{
    if (id < 0) return NULL;

    if (imcontext_list.found_imcontext != NULL && scim_bridge_client_imcontext_get_id (imcontext_list.found_imcontext) == id) {
        return imcontext_list.found_imcontext;
    }

    IMContextListElement *i;
    for (i = imcontext_list.first; i != NULL; i = i->next) {
        const scim_bridge_imcontext_id_t current_id = scim_bridge_client_imcontext_get_id (i->imcontext);
        if (current_id > id) {
            return NULL;
        } else if (current_id == id) {
            imcontext_list.found_imcontext = i->imcontext;
            return imcontext_list.found_imcontext;
        }
    }

    return NULL;
}


retval_t scim_bridge_client_read_and_dispatch ()
{
    scim_bridge_pdebugln (2, "scim_bridge_client_read_and_dispatch");

    if (!initialized) {
        scim_bridge_perrorln ("The client library is not initialized at scim_bridge_client_read_and_dispatch ()");
        return RETVAL_FAILED;
    }

    if (!scim_bridge_client_is_messenger_opened ()) {
        scim_bridge_perrorln ("The messenger is closed");
        return RETVAL_FAILED;
    }

    ScimBridgeMessage *message;
    while (scim_bridge_messenger_poll_message (messenger, &message)) {
        if (scim_bridge_messenger_receive_message (messenger, NULL)) {
            scim_bridge_perrorln ("Failed to receive messages at scim_bridge_client_read_and_dispatch ()");
            scim_bridge_client_close_messenger ();
            return RETVAL_FAILED;
        }
    }

    while (message != NULL) {
        const char *message_header = scim_bridge_message_get_header (message);

        scim_bridge_pdebug (5, "A message has been received: %s", message_header);
        int i;
        for (i = 0; i < scim_bridge_message_get_argument_count (message); ++i) {
            scim_bridge_pdebug (5, " %s", scim_bridge_message_get_argument (message, i));
        }
        scim_bridge_pdebugln (5, "");

        retval_t retval = RETVAL_SUCCEEDED;
        if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_IMENGINE_STATUS_CHANGED) == 0) {
            retval = received_message_imengine_status_changed (message);
        } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_PREEDIT_MODE_CHANGED) == 0) {
            retval = received_message_preedit_mode_changed (message);
        } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_SET_COMMIT_STRING) == 0) {
            retval = received_message_set_commit_string (message);
        } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_COMMIT_STRING) == 0) {
            retval = received_message_commit_string (message);
        } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_SET_PREEDIT_SHOWN) == 0) {
            retval = received_message_set_preedit_shown (message);
        } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_SET_PREEDIT_STRING) == 0) {
            retval = received_message_set_preedit_string (message);
        } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_SET_PREEDIT_CURSOR_POSITION) == 0) {
            retval = received_message_set_preedit_cursor_position (message);
        } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_SET_PREEDIT_ATTRIBUTES) == 0) {
            retval = received_message_set_preedit_attributes (message);
        } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_UPDATE_PREEDIT) == 0) {
            retval = received_message_update_preedit (message);
        } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_KEY_EVENT_HANDLED) == 0) {
            retval = received_message_key_event_handled (message);
        } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_IMCONTEXT_REGISTERED) == 0) {
            retval = received_message_imcontext_registered (message);
        } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_IMCONTEXT_DEREGISTERED) == 0) {
            retval = received_message_imcontext_deregistered (message);
        } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_IMCONTEXT_RESETED) == 0) {
            retval = received_message_imcontext_reseted (message);
        } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_ENABLED) == 0) {
            retval = received_message_imcontext_enabled (message);
        } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_DISABLED) == 0) {
            retval = received_message_imcontext_disabled (message);
        } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_FORWARD_KEY_EVENT) == 0) {
            retval = received_message_forward_key_event (message);
        } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_BEEP) == 0) {
            retval = received_message_beep (message);
        } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_CURSOR_LOCATION_CHANGED) == 0) {
            retval = received_message_cursor_location_changed (message);
        } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_FOCUS_CHANGED) == 0) {
            retval = received_message_focus_changed (message);
        } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_GET_SURROUNDING_TEXT) == 0) {
            retval = received_message_get_surrounding_text (message);
        } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_DELETE_SURROUNDING_TEXT) == 0) {
            retval = received_message_delete_surrounding_text (message);
        } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_REPLACE_SURROUNDING_TEXT) == 0) {
            retval = received_message_replace_surrounding_text (message);
        } else {
            retval = received_message_unknown (message);
        }

        scim_bridge_free_message (message);
        if (retval) {
            scim_bridge_client_close_messenger ();
            return RETVAL_FAILED;
        } else if (scim_bridge_messenger_poll_message (messenger, &message)) {
            scim_bridge_pdebugln (2, "read and dispatch, done");
            break;
        }
    }

    return RETVAL_SUCCEEDED;
}


/* Called from GUI through IMContext */
retval_t scim_bridge_client_register_imcontext (ScimBridgeClientIMContext *imcontext)
{
    scim_bridge_pdebugln (5, "scim_bridge_client_register_imcontext");

    if (!initialized) {
        scim_bridge_perrorln ("ScimBridge is not initialized at scim_bridge_client_register_imcontext ()");
        return RETVAL_FAILED;
    }

    if (!scim_bridge_client_is_messenger_opened ()) {
        scim_bridge_perrorln ("The messenger is closed");
        return RETVAL_FAILED;
    }

    if (pending_response.status != RESPONSE_DONE) {
        scim_bridge_perrorln ("Another command is pending...");
        return RETVAL_FAILED;
    }

    if (scim_bridge_client_imcontext_get_id (imcontext) != -1) {
        scim_bridge_perrorln ("The imcontext has already been registered");
        return RETVAL_FAILED;
    }

    scim_bridge_pdebugln (5, "Sending 'register_imcontext' message");
    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_REGISTER_IMCONTEXT, 0);

    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    while (scim_bridge_messenger_get_sending_buffer_size (messenger) > 0) {
        if (scim_bridge_messenger_send_message (messenger, NULL)) {
            scim_bridge_perrorln ("Failed to send a message at scim_bridge_client_register_imcontext ()");
            scim_bridge_client_close_messenger ();
            return RETVAL_FAILED;
        }
    }

    pending_response.header = SCIM_BRIDGE_MESSAGE_IMCONTEXT_REGISTERED;
    pending_response.imcontext_id = -1;
    pending_response.status = RESPONSE_PENDING;

    while (pending_response.status == RESPONSE_PENDING) {
        if (scim_bridge_client_read_and_dispatch ()) {
            scim_bridge_perrorln ("An IOException at scim_bridge_client_register_imcontext ()");
            pending_response.header = NULL;
            pending_response.status = RESPONSE_DONE;
            return RETVAL_FAILED;
        }
    }

    if (pending_response.status == RESPONSE_FAILED) {
        scim_bridge_perrorln ("Failed to allocate an imcontext at scim_bridge_client_register_imcontext ()");
        pending_response.header = NULL;
        pending_response.status = RESPONSE_DONE;
        return RETVAL_FAILED;
    } else {
        scim_bridge_pdebugln (6, "registered: id = %d", pending_response.imcontext_id);
        scim_bridge_client_imcontext_set_id (imcontext, pending_response.imcontext_id);

        if (imcontext_list.size == 0 || scim_bridge_client_imcontext_get_id (imcontext_list.last->imcontext) < pending_response.imcontext_id) {
            IMContextListElement *new_element = malloc (sizeof (IMContextListElement));
            new_element->imcontext = imcontext;
            new_element->prev = imcontext_list.last;
            new_element->next = NULL;
            if (imcontext_list.last != NULL) {
                imcontext_list.last->next = new_element;
            } else {
                imcontext_list.first = new_element;
            }
            imcontext_list.last = new_element;
            if (imcontext_list.first == NULL) imcontext_list.first = new_element;
            ++imcontext_list.size;
        } else {
            const scim_bridge_imcontext_id_t id = scim_bridge_client_imcontext_get_id (imcontext);
            IMContextListElement *i;
            for (i = imcontext_list.first; i != NULL; i = i->next) {
                const scim_bridge_imcontext_id_t current_id = scim_bridge_client_imcontext_get_id (i->imcontext);
                if (current_id > id) {
                    IMContextListElement *new_element = malloc (sizeof (IMContextListElement));
                    new_element->imcontext = imcontext;
                    new_element->prev = i->prev;
                    new_element->next = i;
                    if (i->prev != NULL) {
                        i->prev->next = new_element;
                    } else {
                        imcontext_list.first = new_element;
                    }
                    i->prev = new_element;
                    ++imcontext_list.size;
                    break;
                }
            }
        }

        pending_response.header = NULL;
        pending_response.status = RESPONSE_DONE;
        return RETVAL_SUCCEEDED;
    }
}


retval_t scim_bridge_client_deregister_imcontext (ScimBridgeClientIMContext *imcontext)
{
    const scim_bridge_imcontext_id_t id = scim_bridge_client_imcontext_get_id (imcontext);
    scim_bridge_pdebugln (5, "scim_bridge_client_deregister_imcontext: ic = %d", id);

    if (!initialized) {
        scim_bridge_perrorln ("ScimBridge is not initialized at scim_bridge_client_deregister_imcontext ()");
        return RETVAL_FAILED;
    }

    if (!scim_bridge_client_is_messenger_opened ()) {
        scim_bridge_perrorln ("The messenger is closed");
        return RETVAL_FAILED;
    }

    if (pending_response.status != RESPONSE_DONE) {
        scim_bridge_perrorln ("Another command is pending...");
        return RETVAL_FAILED;
    }

    if (imcontext == imcontext_list.found_imcontext) imcontext_list.found_imcontext = NULL;

    IMContextListElement *i;
    for (i = imcontext_list.first; i != NULL; i = i->next) {
        if (scim_bridge_client_imcontext_get_id (i->imcontext) == id) {
            IMContextListElement *prev = i->prev;
            IMContextListElement *next = i->next;
            if (prev != NULL) {
                prev->next = next;
            } else {
                imcontext_list.first = next;
            }
            if (next != NULL) {
                next->prev = prev;
            } else {
                imcontext_list.last = prev;
            }
            free (i);
            --imcontext_list.size;
            scim_bridge_client_imcontext_set_id (imcontext, -1);
            break;
        } else if (scim_bridge_client_imcontext_get_id (i->imcontext) > id || i->next == NULL) {
            scim_bridge_perrorln ("The imcontext has not been registered yet");
            return RETVAL_FAILED;
        }
    }

    scim_bridge_pdebugln (5, "Sending 'deregister_imcontext' message: ic_id = %d", id);
    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_DEREGISTER_IMCONTEXT, 1);

    char *ic_id_str;
    scim_bridge_string_from_uint (&ic_id_str, id);
    scim_bridge_message_set_argument (message, 0, ic_id_str);
    free (ic_id_str);

    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    while (scim_bridge_messenger_get_sending_buffer_size (messenger) > 0) {
        if (scim_bridge_messenger_send_message (messenger, NULL)) {
            scim_bridge_perrorln ("Failed to send a message at scim_bridge_client_deregister_imcontext ()");
            scim_bridge_client_close_messenger ();
            return RETVAL_FAILED;
        }
    }

    pending_response.header = SCIM_BRIDGE_MESSAGE_IMCONTEXT_DEREGISTERED;
    pending_response.status = RESPONSE_PENDING;

    while (pending_response.status == RESPONSE_PENDING) {
        if (scim_bridge_client_read_and_dispatch ()) {
            scim_bridge_perrorln ("An IOException at scim_bridge_client_deregister_imcontext ()");
            pending_response.header = NULL;
            pending_response.status = RESPONSE_DONE;

            return RETVAL_FAILED;
        }
    }

    if (pending_response.status == RESPONSE_FAILED) {
        scim_bridge_perrorln ("Failed to free imcontext at scim_bridge_client_deregister_imcontext ()");
        pending_response.header = NULL;
        pending_response.status = RESPONSE_DONE;
        return RETVAL_FAILED;
    } else {
        scim_bridge_pdebugln (6, "deregistered: id = %d", id);

        pending_response.header = NULL;
        pending_response.status = RESPONSE_DONE;
        return RETVAL_SUCCEEDED;
    }
}


retval_t scim_bridge_client_reset_imcontext (const ScimBridgeClientIMContext *imcontext)
{
    const scim_bridge_imcontext_id_t id = scim_bridge_client_imcontext_get_id (imcontext);
    scim_bridge_pdebugln (5, "scim_bridge_client_reset_imcontext: ic = %d", id);

    if (!initialized) {
        scim_bridge_perrorln ("ScimBridge is not initialized at scim_bridge_client_reset_imcontext ()");
        return RETVAL_FAILED;
    }

    if (pending_response.status != RESPONSE_DONE) {
        scim_bridge_perrorln ("Another command is pending...");
        return RETVAL_FAILED;
    }

    if (!scim_bridge_client_is_messenger_opened ()) {
        scim_bridge_perrorln ("The messenger is closed");
        return RETVAL_FAILED;
    }

    scim_bridge_pdebugln (5, "Sending 'reset_imcontext' message: ic_id = %d", id);
    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_RESET_IMCONTEXT, 1);

    char *ic_id_str;
    scim_bridge_string_from_uint (&ic_id_str, id);
    scim_bridge_message_set_argument (message, 0, ic_id_str);
    free (ic_id_str);
    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    while (scim_bridge_messenger_get_sending_buffer_size (messenger) > 0) {
        if (scim_bridge_messenger_send_message (messenger, NULL)) {
            scim_bridge_perrorln ("Failed to send a message at scim_bridge_client_reset_imcontext ()");
            scim_bridge_client_close_messenger ();
            return RETVAL_FAILED;
        }
    }

    pending_response.header = SCIM_BRIDGE_MESSAGE_IMCONTEXT_RESETED;
    pending_response.status = RESPONSE_PENDING;

    while (pending_response.status == RESPONSE_PENDING) {
        if (scim_bridge_client_read_and_dispatch ()) {
            scim_bridge_perrorln ("An IOException at scim_bridge_client_reset_imcontext ()");
            pending_response.header = NULL;
            pending_response.status = RESPONSE_DONE;
            return RETVAL_FAILED;
        }
    }

    if (pending_response.status == RESPONSE_SUCCEEDED) {
        scim_bridge_pdebugln (6, "reseted: id = %d", id);
        pending_response.header = NULL;
        pending_response.status = RESPONSE_DONE;
        return RETVAL_SUCCEEDED;
    } else {
        scim_bridge_perrorln ("An unknown error occurred at scim_bridge_client_reset_imcontext ()");
        pending_response.header = NULL;
        pending_response.status = RESPONSE_DONE;
        return RETVAL_FAILED;
    }
}


retval_t scim_bridge_client_set_imcontext_enabled (const ScimBridgeClientIMContext * imcontext, boolean enabled)
{
    const scim_bridge_imcontext_id_t id = scim_bridge_client_imcontext_get_id (imcontext);
    ScimBridgeMessage *message;

    scim_bridge_pdebugln (5, "scim_bridge_client_set_imcontext_enabled: ic = %d", id);

    if (!initialized) {
        scim_bridge_perrorln ("ScimBridge is not initialized at scim_bridge_client_set_imcontext_enabled ()");
        return RETVAL_FAILED;
    }

    if (pending_response.status != RESPONSE_DONE) {
        scim_bridge_perrorln ("Another command is pending...");
        return RETVAL_FAILED;
    }

    if (!scim_bridge_client_is_messenger_opened ()) {
        scim_bridge_perrorln ("The messenger is closed");
        return RETVAL_FAILED;
    }

    scim_bridge_pdebugln (5, "Sending 'enable_imcontext' message: ic_id = %d", id);

    if (enabled) {
        message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_ENABLE_IMCONTEXT, 1);
    } else {
        message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_DISABLE_IMCONTEXT, 1);
    }

    char *ic_id_str;
    scim_bridge_string_from_uint (&ic_id_str, id);
    scim_bridge_message_set_argument (message, 0, ic_id_str);
    free (ic_id_str);
    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    while (scim_bridge_messenger_get_sending_buffer_size (messenger) > 0) {
        if (scim_bridge_messenger_send_message (messenger, NULL)) {
            scim_bridge_perrorln ("Failed to send a message at scim_bridge_client_set_imcontext_enabled ()");
            scim_bridge_client_close_messenger ();
            return RETVAL_FAILED;
        }
    }

    if (enabled) {
        pending_response.header = SCIM_BRIDGE_MESSAGE_ENABLED;
    } else {
        pending_response.header = SCIM_BRIDGE_MESSAGE_DISABLED;
    }

    pending_response.status = RESPONSE_PENDING;

    while (pending_response.status == RESPONSE_PENDING) {
        if (scim_bridge_client_read_and_dispatch ()) {
            scim_bridge_perrorln ("An IOException at scim_bridge_client_set_imcontext_enabled ()");
            pending_response.header = NULL;
            pending_response.status = RESPONSE_DONE;
            return RETVAL_FAILED;
        }
    }

    if (pending_response.status == RESPONSE_SUCCEEDED) {
        scim_bridge_pdebugln (6, "set_imcontext_enabled returned: id = %d", id);
        pending_response.header = NULL;
        pending_response.status = RESPONSE_DONE;
        return RETVAL_SUCCEEDED;
    } else {
        scim_bridge_perrorln ("An unknown error occurred at scim_bridge_client_set_imcontext_enabled ()");
        pending_response.header = NULL;
        pending_response.status = RESPONSE_DONE;
        return RETVAL_FAILED;
    }
}


retval_t scim_bridge_client_handle_key_event (const ScimBridgeClientIMContext *imcontext, const ScimBridgeKeyEvent *key_event, boolean *consumed)
{
    const scim_bridge_imcontext_id_t id = scim_bridge_client_imcontext_get_id (imcontext);
    scim_bridge_pdebugln (5, "scim_bridge_client_handle_key_event: ic = %d", id);

    if (!initialized) {
        scim_bridge_perrorln ("ScimBridge is not initialized at scim_bridge_client_reset_imcontext ()");
        return RETVAL_FAILED;
    }

    if (!scim_bridge_client_is_messenger_opened ()) {
        scim_bridge_perrorln ("The messenger is closed");
        return RETVAL_FAILED;
    }

    if (pending_response.status != RESPONSE_DONE) {
        scim_bridge_perrorln ("Another command is pending...");
        return RETVAL_FAILED;
    }

    const scim_bridge_key_code_t key_code = scim_bridge_key_event_get_code (key_event);

    const boolean key_pressed = scim_bridge_key_event_is_pressed (key_event);
    scim_bridge_pdebug (5, "scim_bridge_client_key_event_occurred: ic = %d, key_code = %u, pressed = %s", id, key_code, key_pressed ? "true":"false");

    int modifier_count = 0;

    if (scim_bridge_key_event_is_shift_down (key_event)) {
        if (modifier_count == 0) {
            scim_bridge_pdebug (5, ", modifier = ");
        } else {
            scim_bridge_pdebug (5, " + ");
        }
        ++modifier_count;
        scim_bridge_pdebug (5, "%s", SCIM_BRIDGE_MESSAGE_SHIFT);
    }
    if (scim_bridge_key_event_is_control_down (key_event)) {
        if (modifier_count == 0) {
            scim_bridge_pdebug (5, ", modifier = ");
        } else {
            scim_bridge_pdebug (5, " + ");
        }
        ++modifier_count;
        scim_bridge_pdebug (5, "%s", SCIM_BRIDGE_MESSAGE_CONTROL);
    }
    if (scim_bridge_key_event_is_alt_down (key_event)) {
        if (modifier_count == 0) {
            scim_bridge_pdebug (5, ", modifier = ");
        } else {
            scim_bridge_pdebug (5, " + ");
        }
        ++modifier_count;
        scim_bridge_pdebug (5, "%s", SCIM_BRIDGE_MESSAGE_ALT);
    }
    if (scim_bridge_key_event_is_meta_down (key_event)) {
        if (modifier_count == 0) {
            scim_bridge_pdebug (5, ", modifier = ");
        } else {
            scim_bridge_pdebug (5, " + ");
        }
        ++modifier_count;
        scim_bridge_pdebug (5, "%s", SCIM_BRIDGE_MESSAGE_META);
    }
    if (scim_bridge_key_event_is_super_down (key_event)) {
        if (modifier_count == 0) {
            scim_bridge_pdebug (5, ", modifier = ");
        } else {
            scim_bridge_pdebug (5, " + ");
        }
        ++modifier_count;
        scim_bridge_pdebug (5, "%s", SCIM_BRIDGE_MESSAGE_SUPER);
    }
    if (scim_bridge_key_event_is_hyper_down (key_event)) {
        if (modifier_count == 0) {
            scim_bridge_pdebug (5, ", modifier = ");
        } else {
            scim_bridge_pdebug (5, " + ");
        }
        ++modifier_count;
        scim_bridge_pdebug (5, "%s", SCIM_BRIDGE_MESSAGE_HYPER);
    }
    if (scim_bridge_key_event_is_caps_lock_down (key_event)) {
        if (modifier_count == 0) {
            scim_bridge_pdebug (5, ", modifier = ");
        } else {
            scim_bridge_pdebug (5, " + ");
        }
        ++modifier_count;
        scim_bridge_pdebug (5, "%s", SCIM_BRIDGE_MESSAGE_CAPS_LOCK);
    }
    if (scim_bridge_key_event_is_num_lock_down (key_event)) {
        if (modifier_count == 0) {
            scim_bridge_pdebug (5, ", modifier = ");
        } else {
            scim_bridge_pdebug (5, " + ");
        }
        ++modifier_count;
        scim_bridge_pdebug (5, "%s", SCIM_BRIDGE_MESSAGE_NUM_LOCK);
    }
    if (scim_bridge_key_event_is_quirk_enabled (key_event, SCIM_BRIDGE_KEY_QUIRK_KANA_RO)) {
        if (modifier_count == 0) {
            scim_bridge_pdebug (5, ", modifier = ");
        } else {
            scim_bridge_pdebug (5, " + ");
        }
        ++modifier_count;
        scim_bridge_pdebug (5, "%s", SCIM_BRIDGE_MESSAGE_KANA_RO);
    }
    scim_bridge_pdebugln (5, "");
    
    scim_bridge_pdebugln (5, "Sending 'handle_key_event' message: ic_id = %d", id);

    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_HANDLE_KEY_EVENT, modifier_count + 3);

    char *imcontext_id_str;
    scim_bridge_string_from_int (&imcontext_id_str, id);
    scim_bridge_message_set_argument (message, 0,imcontext_id_str);

    char *key_code_str;
    scim_bridge_string_from_uint (&key_code_str, scim_bridge_key_event_get_code (key_event));
    scim_bridge_message_set_argument (message, 1, key_code_str);

    char *key_pressed_str;
    scim_bridge_string_from_boolean (&key_pressed_str, scim_bridge_key_event_is_pressed (key_event));
    scim_bridge_message_set_argument (message, 2, key_pressed_str);

    free (imcontext_id_str);
    free (key_code_str);
    free (key_pressed_str);

    size_t arg_index = 3;

    if (scim_bridge_key_event_is_shift_down (key_event)) {
        scim_bridge_message_set_argument (message, arg_index, SCIM_BRIDGE_MESSAGE_SHIFT);
        ++arg_index;
    }
    if (scim_bridge_key_event_is_control_down (key_event)) {
        scim_bridge_message_set_argument (message, arg_index, SCIM_BRIDGE_MESSAGE_CONTROL);
        ++arg_index;
    }
    if (scim_bridge_key_event_is_alt_down (key_event)) {
        scim_bridge_message_set_argument (message, arg_index, SCIM_BRIDGE_MESSAGE_ALT);
        ++arg_index;
    }
    if (scim_bridge_key_event_is_meta_down (key_event)) {
        scim_bridge_message_set_argument (message, arg_index, SCIM_BRIDGE_MESSAGE_META);
        ++arg_index;
    }
    if (scim_bridge_key_event_is_super_down (key_event)) {
        scim_bridge_message_set_argument (message, arg_index, SCIM_BRIDGE_MESSAGE_SUPER);
        ++arg_index;
    }
    if (scim_bridge_key_event_is_hyper_down (key_event)) {
        scim_bridge_message_set_argument (message, arg_index, SCIM_BRIDGE_MESSAGE_HYPER);
        ++arg_index;
    }
    if (scim_bridge_key_event_is_caps_lock_down (key_event)) {
        scim_bridge_message_set_argument (message, arg_index, SCIM_BRIDGE_MESSAGE_CAPS_LOCK);
        ++arg_index;
    }
    if (scim_bridge_key_event_is_num_lock_down (key_event)) {
        scim_bridge_message_set_argument (message, arg_index, SCIM_BRIDGE_MESSAGE_NUM_LOCK);
        ++arg_index;
    }
    if (scim_bridge_key_event_is_quirk_enabled (key_event, SCIM_BRIDGE_KEY_QUIRK_KANA_RO)) {
        scim_bridge_message_set_argument (message, arg_index, SCIM_BRIDGE_MESSAGE_KANA_RO);
        ++arg_index;
    }

    pending_response.header = SCIM_BRIDGE_MESSAGE_KEY_EVENT_HANDLED;
    pending_response.consumed = FALSE;
    pending_response.status = RESPONSE_PENDING;

    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    while (scim_bridge_messenger_get_sending_buffer_size (messenger) > 0) {
        if (scim_bridge_messenger_send_message (messenger, NULL)) {
            scim_bridge_perrorln ("Failed to send a message at scim_bridge_client_handle_key_event ()");
            scim_bridge_client_close_messenger ();
            return RETVAL_FAILED;
        }
    }

    while (pending_response.status == RESPONSE_PENDING) {
        if (scim_bridge_client_read_and_dispatch ()) {
            scim_bridge_perrorln ("An IOException at scim_bridge_client_handle_key_event ()");
            pending_response.header = NULL;
            pending_response.status = RESPONSE_DONE;
            return RETVAL_FAILED;
        }
    }

    if (pending_response.status == RESPONSE_SUCCEEDED) {
        scim_bridge_pdebugln (3, "The key event was %s", pending_response.consumed ? "consumed":"ignored");
        *consumed = pending_response.consumed;
        pending_response.header = NULL;
        pending_response.status = RESPONSE_DONE;
        return RETVAL_SUCCEEDED;
    } else {
        scim_bridge_perrorln ("An unknown error occurred at scim_bridge_client_handle_key_event ()");
        pending_response.header = NULL;
        pending_response.status = RESPONSE_DONE;
        return RETVAL_FAILED;
    }
}


retval_t scim_bridge_client_change_focus (const ScimBridgeClientIMContext *imcontext, boolean focus_in)
{
    const scim_bridge_imcontext_id_t id = scim_bridge_client_imcontext_get_id (imcontext);
    scim_bridge_pdebugln (5, "scim_bridge_client_change_focus: ic = %d, focus_in = %s", id, focus_in ? "true":"false");

    if (!initialized) {
        scim_bridge_perrorln ("ScimBridge is not initialized at scim_bridge_client_change_focus ()");
        return RETVAL_FAILED;
    }

    if (!scim_bridge_client_is_messenger_opened ()) {
        scim_bridge_perrorln ("The messenger is closed");
        return RETVAL_FAILED;
    }

    if (pending_response.status != RESPONSE_DONE) {
        scim_bridge_perrorln ("Another command is pending...");
        return RETVAL_FAILED;
    }

    scim_bridge_pdebugln (5, "Sending 'change_focus' message: ic_id = %d, focus_in = %s", id, focus_in ? "true":"false");

    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_CHANGE_FOCUS, 2);

    char *ic_id_str;
    scim_bridge_string_from_uint (&ic_id_str, id);
    scim_bridge_message_set_argument (message, 0, ic_id_str);

    char *focus_in_str;
    scim_bridge_string_from_boolean (&focus_in_str, focus_in);
    scim_bridge_message_set_argument (message, 1, focus_in_str);

    free (ic_id_str);
    free (focus_in_str);

    pending_response.header = SCIM_BRIDGE_MESSAGE_FOCUS_CHANGED;
    pending_response.consumed = FALSE;
    pending_response.status = RESPONSE_PENDING;
    
    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    while (scim_bridge_messenger_get_sending_buffer_size (messenger) > 0) {
        if (scim_bridge_messenger_send_message (messenger, NULL)) {
            scim_bridge_perrorln ("Failed to send a message at scim_bridge_client_change_focus ()");
            scim_bridge_client_close_messenger ();
            return RETVAL_FAILED;
        }
    }

    while (pending_response.status == RESPONSE_PENDING) {
        if (scim_bridge_client_read_and_dispatch ()) {
            scim_bridge_perrorln ("An IOException at scim_bridge_client_change_focus ()");
            pending_response.header = NULL;
            pending_response.status = RESPONSE_DONE;
            return RETVAL_FAILED;
        }
    }
    
    if (pending_response.status == RESPONSE_SUCCEEDED) {
        scim_bridge_pdebugln (6, "The focus changed: id = %d", id);
        pending_response.header = NULL;
        pending_response.status = RESPONSE_DONE;
        return RETVAL_SUCCEEDED;
    } else {
        scim_bridge_perrorln ("An unknown error occurred at scim_bridge_client_change_focus ()");
        pending_response.header = NULL;
        pending_response.status = RESPONSE_DONE;
        return RETVAL_FAILED;
    }
}


retval_t scim_bridge_client_set_cursor_location (const ScimBridgeClientIMContext *imcontext, int x, int y)
{

    const scim_bridge_imcontext_id_t id = scim_bridge_client_imcontext_get_id (imcontext);
    scim_bridge_pdebugln (5, "scim_bridge_client_set_cursor_location: ic = %d, x = %d, y = %d", id, x, y);

    if (!initialized) {
        scim_bridge_perrorln ("ScimBridge is not initialized at scim_bridge_client_cursor_set_location ()");
        return RETVAL_FAILED;
    }

    if (!scim_bridge_client_is_messenger_opened ()) {
        scim_bridge_perrorln ("The messenger is closed");
        return RETVAL_FAILED;
    }

    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_SET_CURSOR_LOCATION, 3);

    char *ic_id_str;
    scim_bridge_string_from_uint (&ic_id_str, id);
    scim_bridge_message_set_argument (message, 0, ic_id_str);

    char *x_str;
    scim_bridge_string_from_int (&x_str, x);
    scim_bridge_message_set_argument (message, 1, x_str);

    char *y_str;
    scim_bridge_string_from_int (&y_str, y);
    scim_bridge_message_set_argument (message, 2, y_str);

    free (ic_id_str);
    free (x_str);
    free (y_str);

    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    while (scim_bridge_messenger_get_sending_buffer_size (messenger) > 0) {
        if (scim_bridge_messenger_send_message (messenger, NULL)) {
            scim_bridge_perrorln ("Failed to send a message at scim_bridge_client_set_cursor_location ()");
            scim_bridge_client_close_messenger ();
            return RETVAL_FAILED;
        }
    }

    scim_bridge_pdebugln (6, "the cursor location changed: id = %d", id);
    return RETVAL_SUCCEEDED;
}


retval_t scim_bridge_client_set_preedit_mode (const ScimBridgeClientIMContext *imcontext, scim_bridge_preedit_mode_t mode)
{
    const scim_bridge_imcontext_id_t id = scim_bridge_client_imcontext_get_id (imcontext);

    const char *mode_str;
    switch (mode) {
        case PREEDIT_FLOATING:
            mode_str = SCIM_BRIDGE_MESSAGE_FLOATING;
            break;
        case PREEDIT_HANGING:
            mode_str = SCIM_BRIDGE_MESSAGE_HANGING;
            break;
        case PREEDIT_EMBEDDED:
            mode_str = SCIM_BRIDGE_MESSAGE_EMBEDDED;
            break;
        case PREEDIT_ANY:
            mode_str = SCIM_BRIDGE_MESSAGE_ANY;
            break;
        default:
            scim_bridge_perrorln ("An unknown value is given as a preedit mode.");
            return RETVAL_FAILED;
    }

    scim_bridge_pdebugln (5, "scim_bridge_client_set_preedit_mode: ic = %d, mode = %s", id, mode_str);

    if (!initialized) {
        scim_bridge_perrorln ("ScimBridge is not initialized at scim_bridge_client_set_preedit_mode ()");
        return RETVAL_FAILED;
    }

    if (!scim_bridge_client_is_messenger_opened ()) {
        scim_bridge_perrorln ("The messenger is closed");
        return RETVAL_FAILED;
    }

    if (pending_response.status != RESPONSE_DONE) {
        scim_bridge_perrorln ("Another command is pending...");
        return RETVAL_FAILED;
    }

    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_SET_PREEDIT_MODE, 2);

    char *ic_id_str;
    scim_bridge_string_from_uint (&ic_id_str, id);
    scim_bridge_message_set_argument (message, 0, ic_id_str);

    scim_bridge_message_set_argument (message, 1, mode_str);

    free (ic_id_str);

    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    while (scim_bridge_messenger_get_sending_buffer_size (messenger) > 0) {
        if (scim_bridge_messenger_send_message (messenger, NULL)) {
            scim_bridge_perrorln ("Failed to send a message at scim_bridge_client_set_preedit_mode ()");
            scim_bridge_client_close_messenger ();
            return RETVAL_FAILED;
        }
    }

    pending_response.header = SCIM_BRIDGE_MESSAGE_PREEDIT_MODE_CHANGED;
    pending_response.status = RESPONSE_PENDING;

    while (pending_response.status == RESPONSE_PENDING) {
        if (scim_bridge_client_read_and_dispatch ()) {
            scim_bridge_perrorln ("An IOException at scim_bridge_client_set_preedit_mode ()");
            pending_response.header = NULL;
            pending_response.status = RESPONSE_DONE;

            return RETVAL_FAILED;
        }
    }

    if (pending_response.status == RESPONSE_FAILED) {
        scim_bridge_perrorln ("Failed to change the preedit mode at scim_bridge_client_set_preedit_mode ()");
        pending_response.header = NULL;
        pending_response.status = RESPONSE_DONE;
        return RETVAL_FAILED;
    } else {
        scim_bridge_pdebugln (6, "The preedit mode changed: id = %d", id);
        pending_response.header = NULL;
        pending_response.status = RESPONSE_DONE;
        return RETVAL_SUCCEEDED;
    }
}
