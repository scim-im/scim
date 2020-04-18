/** @file scim_ibus_ctx.cpp
 * implementation of class IBusCtx.
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
 * $Id: scim_ibus_ctx.cpp,v 1.179.2.6 2020/04/29 06:23:38 derekdai Exp $
 *
 */

#define Uses_SCIM_FRONTEND
#define Uses_SCIM_PANEL_CLIENT
#define Uses_SCIM_HOTKEY
#define Uses_STL_UTILITY
#define Uses_C_STDIO
#define Uses_C_STDLIB

#include <set>
#include <sstream>
#include <systemd/sd-bus.h>
#include "scim_private.h"
#include "scim.h"
#include "scim_ibus_ctx.h"
#include "scim_ibus_types.h"
#include "scim_ibus_utils.h"
#include "scim_ibus_frontend.h"

#define IBUS_INPUTCONTEXT_OBJECT_PATH             "/org/freedesktop/IBus/InputContext_"
#define IBUS_INPUTCONTEXT_INTERFACE               "org.freedesktop.IBus.InputContext"
#define IBUS_SERVICE_INTERFACE                    "org.freedesktop.IBus.Service"
#define IBUS_INPUTCONTEXT_OBJECT_PATH_BUF_SIZE    (STRLEN (IBUS_INPUTCONTEXT_OBJECT_PATH) + 10)

IBusCtx::IBusCtx (const String &owner, const String &locale, int id, int siid)
    : m_ibus_id (owner),
      m_id (id),
      m_siid (siid),
      m_caps (0),
      m_client_commit_preedit (true),
      m_on (false),
      m_shared_siid (false),
      m_preedit_caret (0),
      m_keyboard_layout (scim_get_default_keyboard_layout ()),
      m_locale (locale),
      m_preedit_text (),
      m_preedit_attrs (),
      m_inputcontext_slot (NULL),
      m_service_slot (NULL)
{
    log_func ();
}

IBusCtx::~IBusCtx()
{
    log_func ();

    if (m_inputcontext_slot) {
        sd_bus_slot_unref (m_inputcontext_slot);
    }

    if (m_service_slot) {
        sd_bus_slot_unref (m_service_slot);
    }
}

int IBusCtx::init (sd_bus *bus, const char *path)
{
    log_func ();

    int r;
    if ((r = sd_bus_add_object_vtable (bus,
                                       &m_inputcontext_slot,
                                       path,
                                       IBUS_INPUTCONTEXT_INTERFACE,
                                       IBusFrontEnd::m_inputcontext_vtbl,
                                       this)) < 0) {
        return r;
    }

    if ((r = sd_bus_add_object_vtable (bus,
                                       &m_service_slot,
                                       path,
                                       IBUS_SERVICE_INTERFACE,
                                       IBusFrontEnd::m_service_vtbl,
                                       this)) < 0) {
        return r;
    }

    return 0;
}

uint32_t IBusCtx::scim_caps () const
{
    uint32_t caps = SCIM_CLIENT_CAP_TRIGGER_PROPERTY;

    if (m_caps & IBUS_CAP_PREEDIT_TEXT) {
        caps |= SCIM_CLIENT_CAP_ONTHESPOT_PREEDIT;
    }

    if (m_caps & IBUS_CAP_AUXILIARY_TEXT) {
        // no corresponding caps in SCIM
    }

    if (m_caps & IBUS_CAP_LOOKUP_TABLE) {
        caps |= SCIM_CLIENT_CAP_HELPER_MODULE;
    }

    if (m_caps & IBUS_CAP_FOCUS) {
        // no corresponding caps in SCIM
    }

    if (m_caps & IBUS_CAP_PROPERTY) {
        caps |= SCIM_CLIENT_CAP_SINGLE_LEVEL_PROPERTY |
                SCIM_CLIENT_CAP_MULTI_LEVEL_PROPERTY;
    }

    if (m_caps & IBUS_CAP_SURROUNDING_TEXT) {
        caps |= SCIM_CLIENT_CAP_SURROUNDING_TEXT;
    }

    // for testing
    caps = SCIM_CLIENT_CAP_ALL_CAPABILITIES;

    return caps;
}

int IBusCtx::caps_from_message (sd_bus_message *v)
{
    return sd_bus_message_read (v, "u", &m_caps);
}

int IBusCtx::content_type_from_message (sd_bus_message *v)
{
    return m_content_type.from_message (v);
}

int IBusCtx::content_type_to_message (sd_bus_message *v) const
{
    return m_content_type.to_message (v);
}

int IBusCtx::cursor_location_from_message (sd_bus_message *v)
{
    return m_cursor_location.from_message (v);
}

int IBusCtx::cursor_location_relative_from_message (sd_bus_message *v)
{
    IBusRect old = m_cursor_location;

    int r;
    if ((r = m_cursor_location.from_message (v)) < 0 ) {
        return r;
    }

    m_cursor_location += old;

    return 0;
}

int IBusCtx::client_commit_preedit_from_message (sd_bus_message *v)
{
    int r;
    if ((r = sd_bus_message_enter_container (v, 'r', "b")) < 0) {
        return r;
    }

    return sd_bus_message_read (v, "b", &m_client_commit_preedit);
}

int IBusCtx::client_commit_preedit_to_message (sd_bus_message *v) const
{
    int r;
    if ((r = sd_bus_message_open_container (v, 'r', "b")) < 0) {
        return r;
    }

    if ((r = sd_bus_message_append (v, "b", &m_client_commit_preedit)) < 0) {
        return r;
    }

    return sd_bus_message_close_container (v);
}

