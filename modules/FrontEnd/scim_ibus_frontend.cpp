/** @file scim_ibus_frontend.cpp
 * implementation of class IBusFrontEnd.
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
 * $Id: scim_ibus_frontend.cpp,v 1.37 2020/04/29 08:36:42 derekdai Exp $
 *
 */

#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_FRONTEND
#define Uses_SCIM_PANEL_CLIENT
#define Uses_SCIM_HOTKEY

#include <set>
#include <sstream>
#include <cassert>
#include <cstdarg>
#include <sys/time.h>
#include <systemd/sd-event.h>
#include <systemd/sd-bus.h>
#include "scim_private.h"
#include "scim.h"
#include "scim_ibus_ctx.h"
#include "scim_ibus_frontend.h"
#include "scim_ibus_types.h"
#include "scim_ibus_utils.h"

#define scim_module_init                          ibus_LTX_scim_module_init
#define scim_module_exit                          ibus_LTX_scim_module_exit
#define scim_frontend_module_init                 ibus_LTX_scim_frontend_module_init
#define scim_frontend_module_run                  ibus_LTX_scim_frontend_module_run

#define SCIM_KEYBOARD_ICON_FILE                   (SCIM_ICONDIR "/keyboard.png")

#define IBUS_PORTAL_SERVICE                       "org.freedesktop.portal.IBus"
#define IBUS_PORTAL_OBJECT_PATH                   "/org/freedesktop/IBus"
#define IBUS_PORTAL_INTERFACE                     "org.freedesktop.IBus.Portal"
#define IBUS_INPUTCONTEXT_OBJECT_PATH             "/org/freedesktop/IBus/InputContext_"
#define IBUS_INPUTCONTEXT_INTERFACE               "org.freedesktop.IBus.InputContext"
#define IBUS_SERVICE_INTERFACE                    "org.freedesktop.IBus.Service"
#define IBUS_INPUTCONTEXT_OBJECT_PATH_BUF_SIZE    (STRLEN (IBUS_INPUTCONTEXT_OBJECT_PATH) + 10)

#define STRLEN(s)                                 (sizeof (s) - 1)

#define autounref(t)                              __attribute__((cleanup(t ## _unrefp))) t *

using namespace scim;

template<int (IBusFrontEnd::*mf)(sd_bus_message *m)>
static int portal_message_adapter (sd_bus_message *m,
                                   void *userdata,
                                   sd_bus_error *ret_error);

template<int (IBusFrontEnd::*mf)(IBusCtx *ctx, sd_bus_message *m)>
static int
ctx_message_adapter(sd_bus_message *m,
                    void *userdata,
                    sd_bus_error *ret_error);

template<int (IBusFrontEnd::*mf) (IBusCtx *ctx, sd_bus_message *value)>
static int
ctx_prop_adapter (sd_bus *bus,
 	              const char *path,
 	              const char *interface,
 	              const char *property,
 	              sd_bus_message *value,
 	              void *userdata,
 	              sd_bus_error *ret_error);

template<typename T, int (T::*mf)(sd_event_source *s, int fd, uint32_t revents)>
int
sd_event_io_adapter(sd_event_source *s,
                    int fd,
                    uint32_t
                    revents,
                    void *userdata)
{
    return (static_cast<T *>(userdata)->*mf)(s, fd, revents);
}

template<int (IBusFrontEnd::*mf)(sd_bus_message *m)>
static int
message_adapter(sd_bus_message *m,
                void *userdata,
                sd_bus_error *ret_error);

static inline const char * ctx_gen_object_path (int id,
                                                char *buf,
                                                size_t buf_size);

static Pointer <IBusFrontEnd> _scim_frontend (0);

const sd_bus_vtable IBusFrontEnd::m_portal_vtbl [] = {
    SD_BUS_VTABLE_START (0),
    SD_BUS_METHOD_WITH_NAMES (
            "CreateInputContext",
            "s", SD_BUS_PARAM (client_name),
            "o", SD_BUS_PARAM (object_path),
            (&portal_message_adapter<&IBusFrontEnd::portal_create_ctx>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END,
};

const sd_bus_vtable IBusFrontEnd::m_inputcontext_vtbl [] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD_WITH_NAMES(
            "ProcessKeyEvent",
            "uuu", SD_BUS_PARAM(keyval) SD_BUS_PARAM(keycode) SD_BUS_PARAM(state),
            "b", SD_BUS_PARAM(handled),
            (&ctx_message_adapter<&IBusFrontEnd::ctx_process_key_event>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "SetCursorLocation",
            "iiii", SD_BUS_PARAM(x) SD_BUS_PARAM(y) SD_BUS_PARAM(w) SD_BUS_PARAM(h),
            "", SD_BUS_PARAM(),
            (&ctx_message_adapter<&IBusFrontEnd::ctx_set_cursor_location>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "SetCursorLocationRelative",
            "iiii", SD_BUS_PARAM(x) SD_BUS_PARAM(y) SD_BUS_PARAM(w) SD_BUS_PARAM(h),
            "", SD_BUS_PARAM(),
            (&ctx_message_adapter<&IBusFrontEnd::ctx_set_cursor_location_relative>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "ProcessHandWritingEvent",
            "ad", SD_BUS_PARAM(coordinates),
            "", SD_BUS_PARAM(),
            (&ctx_message_adapter<&IBusFrontEnd::ctx_process_hand_writing_event>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "CancelHandWriting",
            "u", SD_BUS_PARAM(n_strokes),
            "", SD_BUS_PARAM(),
            (&ctx_message_adapter<&IBusFrontEnd::ctx_cancel_hand_writing>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD(
            "FocusIn", "", "",
            (&ctx_message_adapter<&IBusFrontEnd::ctx_focus_in>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD(
            "FocusOut", "", "",
            (&ctx_message_adapter<&IBusFrontEnd::ctx_focus_out>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD(
            "Reset", "", "",
            (&ctx_message_adapter<&IBusFrontEnd::ctx_reset>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "SetCapabilities",
            "u", SD_BUS_PARAM(caps),
            "", SD_BUS_PARAM(),
            (&ctx_message_adapter<&IBusFrontEnd::ctx_set_capabilities>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "PropertyActivate",
            "su", SD_BUS_PARAM(name) SD_BUS_PARAM(state),
            "", SD_BUS_PARAM(),
            (&ctx_message_adapter<&IBusFrontEnd::ctx_property_activate>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "SetEngine",
            "s", SD_BUS_PARAM(name),
            "", SD_BUS_PARAM(),
            (&ctx_message_adapter<&IBusFrontEnd::ctx_set_engine>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "GetEngine",
            "", SD_BUS_PARAM(),
            "v", SD_BUS_PARAM(desc),
            (&ctx_message_adapter<&IBusFrontEnd::ctx_get_engine>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD_WITH_NAMES(
            "SetSurroundingText",
            "vuu", SD_BUS_PARAM(text) SD_BUS_PARAM(cursor_pos) SD_BUS_PARAM(anchor_pos),
            "v", SD_BUS_PARAM(desc),
            (&ctx_message_adapter<&IBusFrontEnd::ctx_set_surrounding_text>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_SIGNAL_WITH_NAMES(
            "CommitText",
            "v", SD_BUS_PARAM(text),
            0),
    SD_BUS_SIGNAL_WITH_NAMES(
            "ForwardKeyEvent",
            "uuu", SD_BUS_PARAM(keyval) SD_BUS_PARAM(keycode) SD_BUS_PARAM(state),
            0),
    SD_BUS_SIGNAL_WITH_NAMES(
            "UpdatePreeditText",
            "vub", SD_BUS_PARAM(text) SD_BUS_PARAM(cursor_pos) SD_BUS_PARAM(visible),
            0),
    SD_BUS_SIGNAL_WITH_NAMES(
            "UpdatePreeditTextWithMode",
            "vubu", SD_BUS_PARAM(text) SD_BUS_PARAM(cursor_pos) SD_BUS_PARAM(visible) SD_BUS_PARAM(mode),
            0),
    SD_BUS_SIGNAL("ShowPreeditText", "", 0),
    SD_BUS_SIGNAL("HidePreeditText", "", 0),
    SD_BUS_SIGNAL_WITH_NAMES(
            "UpdateAuxiliaryTextWithMode",
            "vb", SD_BUS_PARAM(text) SD_BUS_PARAM(visible),
            0),
    SD_BUS_SIGNAL("ShowAuxiliaryText", "", 0),
    SD_BUS_SIGNAL("HideAuxiliaryText", "", 0),
    SD_BUS_SIGNAL_WITH_NAMES(
            "UpdateLookupTable",
            "vb", SD_BUS_PARAM(table) SD_BUS_PARAM(visible),
            0),
    SD_BUS_SIGNAL("ShowLookupTable", "", 0),
    SD_BUS_SIGNAL("HideLookupTable", "", 0),
    SD_BUS_SIGNAL("PageUpLookupTable", "", 0),
    SD_BUS_SIGNAL("PageDownLookupTable", "", 0),
    SD_BUS_SIGNAL("CursorUpLookupTable", "", 0),
    SD_BUS_SIGNAL("CursorDownLookupTable", "", 0),
    SD_BUS_SIGNAL("RegisterProperties", "v", 0),
    SD_BUS_SIGNAL_WITH_NAMES(
            "UpdateProperty",
            "v", SD_BUS_PARAM(prop),
            0),
    SD_BUS_WRITABLE_PROPERTY(
            "ClientCommitPreedit",
            "(b)",
            (&ctx_prop_adapter<&IBusFrontEnd::ctx_get_client_commit_preedit>),
            (&ctx_prop_adapter<&IBusFrontEnd::ctx_set_client_commit_preedit>),
            0,
            SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_WRITABLE_PROPERTY(
            "ContentType",
            "(uu)",
            (&ctx_prop_adapter<&IBusFrontEnd::ctx_get_content_type>),
            (&ctx_prop_adapter<&IBusFrontEnd::ctx_set_content_type>),
            0,
            SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_VTABLE_END,
};

const sd_bus_vtable IBusFrontEnd::m_service_vtbl[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD(
            "Destroy",
            "",
            "",
            (&ctx_message_adapter<&IBusFrontEnd::srv_destroy>),
            SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END,
};

IBusFrontEnd::IBusFrontEnd (const BackEndPointer &backend,
                            const ConfigPointer  &config)
    : FrontEndBase (backend),
      m_config (config),
      m_current_instance (-1),
      m_current_ibus_ctx (NULL),
      m_ctx_counter (0),
      m_loop (NULL),
      m_bus (NULL),
      m_portal_slot (NULL),
      m_match_slot (NULL),
      m_panel_source (NULL)
{
    log_func ();

    SCIM_DEBUG_FRONTEND (2) << " Constructing IBusFrontEnd object...\n";

    // Attach Panel Client signal.
    m_panel_client.signal_connect_reload_config                 (slot (this, &IBusFrontEnd::panel_slot_reload_config));
    m_panel_client.signal_connect_exit                          (slot (this, &IBusFrontEnd::panel_slot_exit));
    m_panel_client.signal_connect_update_lookup_table_page_size (slot (this, &IBusFrontEnd::panel_slot_update_lookup_table_page_size));
    m_panel_client.signal_connect_lookup_table_page_up          (slot (this, &IBusFrontEnd::panel_slot_lookup_table_page_up));
    m_panel_client.signal_connect_lookup_table_page_down        (slot (this, &IBusFrontEnd::panel_slot_lookup_table_page_down));
    m_panel_client.signal_connect_trigger_property              (slot (this, &IBusFrontEnd::panel_slot_trigger_property));
    m_panel_client.signal_connect_process_helper_event          (slot (this, &IBusFrontEnd::panel_slot_process_helper_event));
    m_panel_client.signal_connect_move_preedit_caret            (slot (this, &IBusFrontEnd::panel_slot_move_preedit_caret));
    m_panel_client.signal_connect_select_candidate              (slot (this, &IBusFrontEnd::panel_slot_select_candidate));
    m_panel_client.signal_connect_process_key_event             (slot (this, &IBusFrontEnd::panel_slot_process_key_event));
    m_panel_client.signal_connect_commit_string                 (slot (this, &IBusFrontEnd::panel_slot_commit_string));
    m_panel_client.signal_connect_forward_key_event             (slot (this, &IBusFrontEnd::panel_slot_forward_key_event));
    m_panel_client.signal_connect_request_help                  (slot (this, &IBusFrontEnd::panel_slot_request_help));
    m_panel_client.signal_connect_request_factory_menu          (slot (this, &IBusFrontEnd::panel_slot_request_factory_menu));
    m_panel_client.signal_connect_change_factory                (slot (this, &IBusFrontEnd::panel_slot_change_factory));
}

IBusFrontEnd::~IBusFrontEnd ()
{
    log_func ();

    SCIM_DEBUG_FRONTEND (2) << " Destructing IBusFrontEnd object...\n";

    IBusIDCtxMap::iterator it = m_id_ctx_map.begin ();
    for (; it != m_id_ctx_map.end (); ++ it) {
        delete it->second;
    }

    panel_disconnect ();
    
    if (m_panel_source) {
        sd_event_source_unref (m_panel_source);
    }

    if (m_match_slot) {
        sd_bus_slot_unref (m_match_slot);
    }

    if (m_portal_slot) {
        sd_bus_slot_unref (m_portal_slot);
    }

    if (m_bus) {
        sd_bus_release_name (m_bus, IBUS_PORTAL_SERVICE);
        sd_bus_detach_event (m_bus);
        sd_bus_unref (m_bus);
    }

    if (m_loop) {
        sd_event_unref (m_loop);
    }
}

void
IBusFrontEnd::show_preedit_string (int siid)
{
    log_func ();

    signal_ctx (siid, "ShowPreeditText");
}

void
IBusFrontEnd::show_aux_string (int siid)
{
    log_func ();

    signal_ctx (siid, "ShowAuxiliaryText");
}

void
IBusFrontEnd::show_lookup_table (int siid)
{
    log_func ();

    IBusCtx *ctx = find_ctx_by_siid (siid);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        m_panel_client.show_lookup_table (ctx->id ());
        m_panel_client.send ();
    }
}

void
IBusFrontEnd::hide_preedit_string (int siid)
{
    log_func ();

    signal_ctx (siid, "HidePreeditText");
}

void
IBusFrontEnd::hide_aux_string (int siid)
{
    log_func ();

    signal_ctx (siid, "HideAuxiliaryText");
}

void
IBusFrontEnd::hide_lookup_table (int siid)
{
    log_func ();

    IBusCtx *ctx = find_ctx_by_siid (siid);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        m_panel_client.hide_lookup_table (ctx->id ());
        m_panel_client.send ();
    }
}

void
IBusFrontEnd::update_preedit_caret (int siid, int caret)
{
    log_func ();

    log_debug ("caret move from %d to %d",
            m_current_ibus_ctx->preedit_caret (),
            caret);

    m_current_ibus_ctx->preedit_caret (caret);

    signal_ctx (siid,
                "UpdatePreeditTextWithMode",
                &m_current_ibus_ctx->preedit_text (),
                &m_current_ibus_ctx->preedit_attrs (),
                m_current_ibus_ctx->preedit_caret (),
                true,
                IBUS_ENGINE_PREEDIT_CLEAR);
}

void
IBusFrontEnd::update_preedit_string (int siid,
                                     const WideString & str,
                                     const AttributeList & attrs)
{
    log_func ();

    m_current_ibus_ctx->preedit_text (str);
    m_current_ibus_ctx->preedit_attrs (attrs);

    char buf[256];
    log_debug ("preedit text: '%s' => '%s', attrs: %s",
            utf8_wcstombs (m_current_ibus_ctx->preedit_text ()).c_str (),
            utf8_wcstombs (str).c_str (),
            scim_attr_list_to_str (attrs, buf, sizeof (buf)));

    signal_ctx (siid,
                "UpdatePreeditTextWithMode",
                &str,
                &attrs,
                m_current_ibus_ctx->preedit_caret (),
                true,
                IBUS_ENGINE_PREEDIT_CLEAR);
}

void
IBusFrontEnd::update_aux_string (int siid,
                                 const WideString & str,
                                 const AttributeList & attrs)
{
    log_func ();

    signal_ctx (siid, "UpdateAuxiliaryText", &str, &attrs);
}

void
IBusFrontEnd::commit_string (int siid, const WideString & str)
{
    log_func ();

    signal_ctx (siid, "CommitText", &str);

    m_current_ibus_ctx->preedit_text (WideString ());
    m_current_ibus_ctx->preedit_attrs (AttributeList ());
    m_current_ibus_ctx->preedit_caret (0);
}

void
IBusFrontEnd::forward_key_event (int siid, const KeyEvent & key)
{
    log_func ();

    signal_ctx (siid, "ForwardKeyEvent", &key);
}

void
IBusFrontEnd::update_lookup_table (int siid, const LookupTable & table)
{
    log_func ();

    IBusCtx *ctx = find_ctx_by_siid (siid);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        m_panel_client.update_lookup_table (ctx->id (), table);
        m_panel_client.send ();
    }
}

void
IBusFrontEnd::register_properties (int siid, const PropertyList &properties)
{
    log_func ();
    
    IBusCtx *ctx = find_ctx_by_siid (siid);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        m_panel_client.register_properties (ctx->id (), properties);
        m_panel_client.send ();
    }
}

void
IBusFrontEnd::update_property (int siid, const Property &property)
{
    log_func ();

    IBusCtx *ctx = find_ctx_by_siid (siid);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        m_panel_client.update_property (ctx->id (), property);
        m_panel_client.send ();
    }
}

void
IBusFrontEnd::beep (int siid)
{
    log_func_ignored ();
}

void
IBusFrontEnd::start_helper (int siid, const String &helper_uuid)
{
    log_func ();

    IBusCtx *ctx = find_ctx_by_siid (siid);
    if (m_current_instance == siid && validate_ctx (ctx)) {
        SCIM_DEBUG_FRONTEND (2) << "start_helper (" << helper_uuid << ")\n";
        m_panel_client.prepare (ctx->id ());
        m_panel_client.start_helper (ctx->id (), helper_uuid);
        m_panel_client.send();
    }
}

void
IBusFrontEnd::stop_helper (int siid, const String &helper_uuid)
{
    log_func ();

    SCIM_DEBUG_FRONTEND (2) << "stop_helper (" << helper_uuid << ")\n";

    IBusCtx *ctx = find_ctx_by_siid (siid);
    if (m_current_instance == siid && validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        m_panel_client.stop_helper (ctx->id (), helper_uuid);
        m_panel_client.send();
    }
}

void
IBusFrontEnd::send_helper_event (int siid, const String &helper_uuid, const Transaction &trans)
{
    log_func ();

    IBusCtx *ctx = find_ctx_by_siid (siid);
    if (m_current_instance == siid && validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        m_panel_client.send_helper_event (ctx->id (), helper_uuid, trans);
        m_panel_client.send();
    }
}

bool
IBusFrontEnd::get_surrounding_text (int siid, WideString &text, int &cursor, int maxlen_before, int maxlen_after)
{
    log_func_not_impl (false);

//    text.clear ();
//    cursor = 0;
//
//    if (m_current_instance == siid && m_current_socket_client >= 0 && (maxlen_before != 0 || maxlen_after != 0)) {
//        if (maxlen_before < 0) maxlen_before = -1;
//        if (maxlen_after < 0) maxlen_after = -1;
//
//        m_temp_trans.clear ();
//        m_temp_trans.put_command (SCIM_TRANS_CMD_REPLY);
//        m_temp_trans.put_command (SCIM_TRANS_CMD_GET_SURROUNDING_TEXT);
//        m_temp_trans.put_data ((uint32) maxlen_before);
//        m_temp_trans.put_data ((uint32) maxlen_after);
//
//        Socket socket_client (m_current_socket_client);
//
//        if (m_temp_trans.write_to_socket (socket_client) &&
//            m_temp_trans.read_from_socket (socket_client, m_socket_timeout)) {
//
//            int cmd;
//            uint32 key;
//            uint32 cur;
//
//            if (m_temp_trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REQUEST &&
//                m_temp_trans.get_data (key) && key == m_current_socket_client_key &&
//                m_temp_trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_GET_SURROUNDING_TEXT &&
//                m_temp_trans.get_data (text) && m_temp_trans.get_data (cur)) {
//                cursor = (int) cur;
//                return true;
//            }
//        }
//    }
//    return false;
}

bool
IBusFrontEnd::delete_surrounding_text (int siid, int offset, int len)
{
    log_func_not_impl (false);

//    if (m_current_instance == siid && m_current_socket_client >= 0 && len > 0) {
//        m_temp_trans.clear ();
//        m_temp_trans.put_command (SCIM_TRANS_CMD_REPLY);
//        m_temp_trans.put_command (SCIM_TRANS_CMD_DELETE_SURROUNDING_TEXT);
//        m_temp_trans.put_data ((uint32) offset);
//        m_temp_trans.put_data ((uint32) len);
//
//        Socket socket_client (m_current_socket_client);
//
//        if (m_temp_trans.write_to_socket (socket_client) &&
//            m_temp_trans.read_from_socket (socket_client, m_socket_timeout)) {
//
//            int cmd;
//            uint32 key;
//
//            if (m_temp_trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REQUEST &&
//                m_temp_trans.get_data (key) && key == m_current_socket_client_key &&
//                m_temp_trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_DELETE_SURROUNDING_TEXT &&
//                m_temp_trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK)
//                return true;
//        }
//    }
//    return false;
}

void
IBusFrontEnd::init (int argc, char **argv)
{
    log_func();

    int max_clients = -1;

    reload_config_callback (m_config);

    if (!m_config.null ()) {
        m_config->signal_connect_reload (slot (this, &IBusFrontEnd::reload_config_callback));
    }

    if (sd_event_new (&m_loop) < 0) {
        throw FrontEndError ("IBusFrontEnd -- Cannot create event loop.");
    }

    if (sd_bus_open_user (&m_bus) < 0) {
        throw FrontEndError ("IBusFrontEnd -- Cannot connect to session bus.");
    }

    if (sd_bus_attach_event (m_bus, m_loop, SD_EVENT_PRIORITY_NORMAL) < 0) {
        throw FrontEndError ("IBusFrontEnd -- Cannot attach bus to loop.");
    }

    if (sd_bus_request_name (m_bus,
                             IBUS_PORTAL_SERVICE,
                             SD_BUS_NAME_ALLOW_REPLACEMENT |
                                 SD_BUS_NAME_REPLACE_EXISTING) < 0) {
        throw FrontEndError (String ("IBus -- failed to aquire service name!"));
    }

    if (sd_bus_add_object_vtable (m_bus,
                                  &m_portal_slot,
                                  IBUS_PORTAL_OBJECT_PATH,
                                  IBUS_PORTAL_INTERFACE,
                                  m_portal_vtbl,
                                  this) < 0) {
        throw FrontEndError (String ("IBus -- failed to add portal object!"));
    }

    if (sd_bus_add_match (m_bus,
                          &m_match_slot,
                          "type='signal',"
                             "sender='org.freedesktop.DBus',"
                             "path='/org/freedesktop/DBus',"
                             "interface='org.freedesktop.DBus',"
                             "member='NameOwnerChanged',"
                             "arg2=''",
                          &message_adapter<&IBusFrontEnd::client_destroy>,
                          NULL) < 0) {
        throw FrontEndError (String ("IBus -- failed to add match for NameOwnerChanged signal!"));
    }

    if (panel_connect() < 0) {
        throw FrontEndError (String ("IBus -- failed to connect to the panel daemon!"));
    }
}

void
IBusFrontEnd::run ()
{
    log_func();

    if (m_loop) {
        sd_event_loop (m_loop);
    }
}

int
IBusFrontEnd::generate_ctx_id ()
{
    return ++ m_ctx_counter;
}

void
IBusFrontEnd::reload_config_callback (const ConfigPointer &config)
{
    log_func();

    SCIM_DEBUG_FRONTEND (1) << "Reload configuration.\n";

    m_frontend_hotkey_matcher.load_hotkeys (config);
    m_imengine_hotkey_matcher.load_hotkeys (config);
}

/**
 * IBusFrontEnd::ibus_new_instance
 */
int
IBusFrontEnd::portal_create_ctx(sd_bus_message *m)
{
    log_func();

    String locale = scim_get_current_locale ();
    String encoding = scim_get_locale_encoding (locale);
    String language = scim_get_current_language ();
    String sfid = get_default_factory (language, encoding);

    log_debug ("locale=%s", locale.c_str ());

    int siid = new_instance (sfid, encoding);

    int id = generate_ctx_id ();
    char path[IBUS_INPUTCONTEXT_OBJECT_PATH_BUF_SIZE];
    ctx_gen_object_path(id, path, sizeof(path));

    log_debug ("instance name=%s, authors=%s, encoding=%s, uuid=%s",
            utf8_wcstombs (get_instance_name (siid)).c_str(),
            utf8_wcstombs (get_instance_authors (siid)).c_str(),
            get_instance_encoding (siid).c_str (),
            get_instance_uuid (siid).c_str ());

    int r;

    IBusCtx *ctx = new IBusCtx (sd_bus_message_get_sender (m),
                                locale,
                                id,
                                siid);
    if ((r = ctx->init (m_bus, path))) {
        return r;
    }

    m_panel_client.prepare (id);
    m_panel_client.register_input_context (id, get_instance_uuid (siid));
    m_panel_client.send ();

    m_id_ctx_map [id] = ctx;
    m_siid_ctx_map [siid] = ctx;
    m_name_ctx_map [sd_bus_message_get_sender (m)].insert (ctx);

    if ((r = sd_bus_reply_method_return (m, "o", path)) < 0) {
        delete ctx;
    }

    return r;
}

int
IBusFrontEnd::srv_destroy(IBusCtx *ctx, sd_bus_message *m)
{
    log_func ();

    destroy_ctx (ctx);

    m_name_ctx_map[sd_bus_message_get_sender (m)].erase (ctx);

    return 0;
}

int
IBusFrontEnd::ctx_process_key_event(IBusCtx *ctx, sd_bus_message *m)
{
    log_func ();

    uint32_t keyval = 0;
    uint32_t keycode = 0;
    uint32_t state = 0;

    if (sd_bus_message_read(m, "uuu", &keyval, &keycode, &state) < 0) {
        return -1;
    }

    KeyEvent event = scim_ibus_keyevent_to_scim_keyevent (ctx->keyboard_layout (),
                                                          keyval,
                                                          keycode,
                                                          state);

    int siid = ctx->siid ();

    m_current_instance = siid;

    m_panel_client.prepare (ctx->id ());
    bool consumed = filter_hotkeys (ctx, event)
                    ? true
                    : ctx->is_on() && process_key_event (siid, event)
                        ? true
                        : false;
    m_panel_client.send ();

    m_current_instance = -1;

    String eventstr;
    log_debug ("%s %s",
               (scim_key_to_string (eventstr, event), eventstr).c_str (),
               consumed ? "consumed" : "ignored");

    return sd_bus_reply_method_return (m, "b", consumed);
}

int
IBusFrontEnd::ctx_set_cursor_location(IBusCtx *ctx, sd_bus_message *m)
{
    log_func ();

    int r;
    if ((r = ctx->cursor_location_from_message (m)) < 0) {
        return r;
    }

    m_panel_client.prepare (ctx->id ());
    panel_req_update_spot_location (ctx);
    m_panel_client.send ();

    return 0;
}

int
IBusFrontEnd::ctx_set_cursor_location_relative(IBusCtx *ctx, sd_bus_message *m)
{
    log_func ();

    int r;
    if ((r = ctx->cursor_location_relative_from_message (m)) < 0) {
        return r;
    }

    m_panel_client.prepare (ctx->id ());
    panel_req_update_spot_location (ctx);
    m_panel_client.send ();

    return 0;
}


int
IBusFrontEnd::ctx_process_hand_writing_event(IBusCtx *ctx, sd_bus_message *m)
{
    log_func_ignored (0);
}

int
IBusFrontEnd::ctx_cancel_hand_writing(IBusCtx *ctx, sd_bus_message *m)
{
    log_func_ignored (0);
}

int
IBusFrontEnd::ctx_property_activate(IBusCtx *ctx, sd_bus_message *m)
{
    log_func_ignored (0);
}

int
IBusFrontEnd::ctx_set_engine(IBusCtx *ctx, sd_bus_message *m)
{
    log_func_ignored (0);
}

int
IBusFrontEnd::ctx_get_engine(IBusCtx *ctx, sd_bus_message *m)
{
    log_func_ignored (0);
}

int
IBusFrontEnd::ctx_set_surrounding_text(IBusCtx *ctx, sd_bus_message *m)
{
    log_func_ignored (0);
}

int
IBusFrontEnd::ctx_focus_in(IBusCtx *ctx, sd_bus_message *m)
{
    log_func ();

    uint32 siid = ctx->siid();

    m_current_ibus_ctx = ctx;
    m_current_instance = siid;

    focus_in (siid); 

    m_panel_client.prepare (ctx->id ());
    panel_req_focus_in (ctx);
    if (ctx->is_on ()) {
        start_ctx (ctx);
    }
    m_panel_client.send ();

    m_current_instance = -1;

    return 0;
}

int
IBusFrontEnd::ctx_focus_out(IBusCtx *ctx, sd_bus_message *m)
{
    log_func ();

    uint32 siid = ctx->siid ();

    m_current_instance = siid;

    m_panel_client.prepare (ctx->id ());
    m_panel_client.focus_out (ctx-> id());
    m_panel_client.send ();

    focus_out (siid); 

    m_current_ibus_ctx = NULL;
    m_current_instance = -1;

    return 0;
}

int
IBusFrontEnd::ctx_reset(IBusCtx *ctx, sd_bus_message *m)
{
    log_func ();

    uint32 siid = ctx->siid ();

    m_current_instance = siid;

    reset (siid); 

    ctx->preedit_reset ();

    m_current_instance = -1;

    return 0;
}

/**
 * IBusFrontEnd::ibus_update_client_capabilities
 */
int
IBusFrontEnd::ctx_set_capabilities(IBusCtx *ctx, sd_bus_message *m)
{
    log_func ();

    SCIM_DEBUG_FRONTEND (3) << "  SI (" << ctx->siid () << ").\n";

    int r;
    if ((r = ctx->caps_from_message (m)) < 0) {
        return r;
    }

    int siid = ctx->siid ();

    m_current_instance = siid;
    update_client_capabilities (siid, ctx->scim_caps ()); 
    m_current_instance = -1;

    log_debug("IBus caps=%s => SCIM caps=%s",
              ibus_caps_to_str (ctx->caps()).c_str(),
              scim_caps_to_str (ctx->scim_caps()).c_str());

    return r;
}

int
IBusFrontEnd::ctx_get_client_commit_preedit (IBusCtx *ctx, sd_bus_message *value)
{
    log_func ();

    return ctx->client_commit_preedit_to_message (value);
}

int
IBusFrontEnd::ctx_set_client_commit_preedit (IBusCtx *ctx, sd_bus_message *value)
{
    log_func ();

    return ctx->client_commit_preedit_from_message (value);
}

int
IBusFrontEnd::ctx_get_content_type (IBusCtx *ctx, sd_bus_message *value)
{
    log_func ();

    return ctx->content_type_to_message (value);
}

int
IBusFrontEnd::ctx_set_content_type (IBusCtx *ctx, sd_bus_message *value)
{
    log_func ();

    return ctx->content_type_from_message (value);
}

int
IBusFrontEnd::client_destroy (sd_bus_message *m)
{
    log_func();

    const char *name;
    
    int r;
    if ((r = sd_bus_message_read (m, "s", &name)) < 0) {
        return r;
    }

    IBusNameCtxMap::iterator ctx_set = m_name_ctx_map.find (name);
    if (ctx_set == m_name_ctx_map.end ()) {
        return 0;
    }

    IBusCtxSet::iterator ctx_it = ctx_set->second.begin ();
    for (; ctx_it != ctx_set->second.end (); ctx_it ++) {
        destroy_ctx (*ctx_it);
    }

    m_name_ctx_map.erase (ctx_set);

    return 0;
}

IBusCtx *
IBusFrontEnd::find_ctx_by_siid (int siid) const
{
    IBusIDCtxMap::const_iterator it = m_siid_ctx_map.find (siid);
    if (it == m_siid_ctx_map.end ()) {
        return NULL;
    }

    return it->second;
}

IBusCtx *
IBusFrontEnd::find_ctx (int id) const
{
    IBusIDCtxMap::const_iterator it = m_id_ctx_map.find (id);
    if (it == m_id_ctx_map.end ()) {
        return NULL;
    }

    return it->second;
}

IBusCtx *
IBusFrontEnd::find_ctx (const char *path) const
{
    if (strncmp (IBUS_INPUTCONTEXT_OBJECT_PATH,
                 path,
                 STRLEN(IBUS_INPUTCONTEXT_OBJECT_PATH))) {
        return NULL;
    }

    const char *id_str = strrchr (path + STRLEN(IBUS_INPUTCONTEXT_OBJECT_PATH),
                                  '_');
    if (!id_str) {
        return NULL;
    }

    int id = (int) strtol (id_str + 1, NULL, 10);
    if (!id) {
        // id start from 1
        return NULL;
    }

    return _scim_frontend->find_ctx (id);
}

inline bool
IBusFrontEnd::validate_ctx (IBusCtx *ctx) const {
    return ctx != NULL &&
           m_id_ctx_map.find (ctx->id ()) != m_id_ctx_map.end ();
}

void
IBusFrontEnd::start_ctx (IBusCtx *ctx)
{
    log_func ();

    if (validate_ctx (ctx)) {

        ctx->on ();

        // TODO figure out how to update panel on screens
//        panel_req_update_screen (ctx);
        panel_req_update_spot_location (ctx);
        panel_req_update_factory_info (ctx);

        m_panel_client.turn_on (ctx->id ());
        m_panel_client.hide_lookup_table (ctx->id ());
        signal_ctx (ctx->siid (), "HidePreeditText");
        signal_ctx (ctx->siid (), "HideAuxiliaryText");

        if (ctx->shared_siid ()) reset (ctx->siid ());
        focus_in (ctx->siid ());
    }
}

void
IBusFrontEnd::stop_ctx (IBusCtx *ctx)
{
    log_func ();

    if (validate_ctx (ctx)) {
        focus_out (ctx->siid ());
        if (ctx->shared_siid ()) reset (ctx->siid ());

        signal_ctx (ctx->siid (), "HidePreeditText");
        signal_ctx (ctx->siid (), "HideAuxiliaryText");
        m_panel_client.hide_lookup_table (ctx->id ());
        m_panel_client.turn_off (ctx->id ());

        panel_req_update_factory_info (ctx);

        ctx->off ();
    }
}

void
IBusFrontEnd::destroy_ctx (IBusCtx *ctx)
{
    log_func ();

    if (m_id_ctx_map.find (ctx->id ()) != m_id_ctx_map.end ())  {
        m_id_ctx_map.erase (ctx->id ());
    }

    if (m_current_instance == ctx->siid ()) {
        m_current_instance = -1;
    }

    if (ctx == m_current_ibus_ctx) {
        m_current_ibus_ctx = NULL;
    }

    m_panel_client.prepare (ctx->id ());
    m_panel_client.remove_input_context (ctx->id ());
    m_panel_client.send ();

    m_id_ctx_map.erase (ctx->id ());
    m_siid_ctx_map.erase (ctx->siid ());

    delete ctx;
}

static int
serialize_signal (sd_bus_message *m, va_list &args)
{
    return 0;
}

static int
serialize_commit_text_signal (sd_bus_message *m, va_list &args)
{
    const WideString &wstr = *va_arg (args, const WideString *);
    String str = utf8_wcstombs (wstr);

    log_func ();

    log_debug ("commit text: '%s'", str.c_str());

    return sd_bus_message_append (m,
                                  "v", "(sa{sv}sv)",
                                  "IBusText",
                                  0,
                                  str.c_str(),
                                  "(sa{sv}av)",
                                      "IBusAttrList",
                                      0,
                                      0);
}

static int
serialize_forward_key_event_signal (sd_bus_message *m, va_list &args)
{
    KeyEvent &event = *va_arg (args, KeyEvent *);

    log_func ();

    // TODO find out how to get keycode from KeyEvent
    return sd_bus_message_append (m, "uuu",
                                  event.code,   // keyval
                                  0,            // keycode
                                  scim_keymask_to_ibus_keystate (event.mask) |
                                      IBUS_FORWARD_MASK);
}

static inline int
sd_bus_message_close_containers (sd_bus_message *m, int levels)
{
    int r;
    for (; levels > 0; levels --) {
        if ((r = sd_bus_message_close_container (m)) < 0) {
            return r;
        }
    }

    return 0;
}

static int
serialize_attribute (sd_bus_message *m, const Attribute &scim_attr)
{
    IBusAttribute attr;
    if (!scim_attr_to_ibus_attr (scim_attr, attr)) {
        // useless or unmappable attribute, skip
        return 0;
    }

    return sd_bus_message_append (m, "v",
                                  "(sa{sv}uuuu)",
                                      "IBusAttribute",
                                      0,
                                      attr.type,
                                      attr.value,
                                      attr.start_index,
                                      attr.end_index);
}

static int
serialize_attribute_list (sd_bus_message *m,
                          const AttributeList &attrs)
{
    int r;
    if ((r = sd_bus_message_open_container (m, 'v', "(sa{sv}av)")) < 0) {
        return r;
    }

    if ((r = sd_bus_message_open_container (m, 'r', "sa{sv}av")) < 0) {
        return r;
    }

    if ((r = sd_bus_message_append (m, "sa{sv}", "IBusAttrList", 0)) < 0) {
        return r;
    }

    if ((r = sd_bus_message_open_container (m, 'a', "v")) < 0) {
        return r;
    }

    AttributeList::const_iterator it = attrs.begin ();
    for (; it != attrs.end(); it ++) {
        if ((r = serialize_attribute (m, *it)) < 0) {
            return r;
        }
    }

    return sd_bus_message_close_containers (m, 3);
}

static int
serialize_text (sd_bus_message *m,
                const WideString &wstr,
                const AttributeList &attrs)
{
    int r;
    if ((r = sd_bus_message_open_container (m, 'v', "(sa{sv}sv)")) < 0) {
        return r;
    }

    if ((r = sd_bus_message_open_container (m, 'r', "sa{sv}sv")) < 0) {
        return r;
    }

    if ((r = sd_bus_message_append (m,
                                    "sa{sv}s",
                                    "IBusText",
                                    0,
                                    utf8_wcstombs (wstr).c_str ())) < 0) {
        return r;
    }

    if ((r = serialize_attribute_list(m, attrs)) < 0) {
        return r;
    }

    return sd_bus_message_close_containers (m, 2);
}

static int
serialize_text_with_mode_signal (sd_bus_message *m, va_list &args)
{
    const WideString &str = *va_arg (args, const WideString *);
    const AttributeList &attrs = *va_arg (args, const AttributeList *);
    uint32_t caret_pos = va_arg (args, uint32_t);
    bool visible = (bool) !!va_arg (args, int);
    uint32_t mode = va_arg (args, uint32_t);

    log_func ();

    int r;
    if ((r = serialize_text (m, str, attrs)) < 0) {
        return r;
    }

    return sd_bus_message_append (m, "ubu", caret_pos, visible, mode);
}

void
IBusFrontEnd::signal_ctx (int siid, const char *signal, ...) const
{
    log_func ();

    IBusCtx *ctx = find_ctx_by_siid (siid);
    if (!validate_ctx (ctx)) {
        return;
    }

    char path [IBUS_INPUTCONTEXT_OBJECT_PATH_BUF_SIZE];
    ctx_gen_object_path (ctx->id (), path, sizeof (path));

    int (*serializer) (sd_bus_message *m, va_list &args) = NULL;

    // compare strings by interned address
    if ("ForwardKeyEvent" == signal) {
        serializer = &serialize_forward_key_event_signal;
    } else if ("UpdatePreeditTextWithMode" == signal) {
        serializer = &serialize_text_with_mode_signal;
    } else if ("CommitText" == signal) {
        serializer = &serialize_commit_text_signal;
    } else if ("ShowPreeditText" == signal ||
               "HidePreeditText" == signal ||
               "ShowAuxiliaryText" == signal ||
               "HideAuxiliaryText" == signal) {
        serializer = serialize_signal;
    } else if ("UpdateLookupTable" == signal ||
               "ShowLookupTable" == signal ||
               "HideLookupTable" == signal ||
               "PageUpLookupTable" == signal ||
               "PageDownLookupTable" == signal ||
               "CursorUpLookupTable" == signal ||
               "CursorDownLookupTable" == signal ||
               "RegisterProperties" == signal ||
               "UpdateProperty" == signal) {
        log_info ("ignore signal %s emitting", signal);
        return;
    } else {
        log_error ("unknown signal %s", signal);
        return;
    }

    int r;
    autounref(sd_bus_message) m = NULL;
    if ((r = sd_bus_message_new_signal (m_bus,
                                        &m,
                                        path,
                                        IBUS_INPUTCONTEXT_INTERFACE,
                                        signal)) < 0) {
        log_warn ("unable to create signal: %s", strerror (-r));
    }

    va_list args;
    va_start (args, signal);
    if ((r = serializer (m, args)) < 0) {
        log_warn ("error occured while appending signal arguments: %s",
                  strerror (-r));
        return;
    }

    if ((r = sd_bus_message_set_destination (m, ctx->owner ().c_str ())) < 0) {
        log_warn ("unable to set signal destination: %s", strerror (-r));
    }

    log_trace ("emit %s.%s => %s", path, signal, ctx->owner ().c_str());

    if ((r = sd_bus_send (m_bus, m, NULL)) < 0) {
        log_warn ("unabled to emit %s.%s: %s",
                  sd_bus_message_get_interface (m),
                  sd_bus_message_get_member (m),
                  strerror (-r));
    }
}

int
IBusFrontEnd::panel_connect ()
{
    log_func();

    if (m_panel_client.is_connected()) {
        return 0;
    }

    int r;
    if ((r = m_panel_client.open_connection (m_config->get_name (), getenv ("DISPLAY"))) < 0) {
        return r;
    }

    int fd = m_panel_client.get_connection_number();
    if ((r = sd_event_add_io (m_loop,
                              &m_panel_source,
                              fd,
                              EPOLLIN,
                              &sd_event_io_adapter<IBusFrontEnd, &IBusFrontEnd::panel_handle_io>,
                              this)) < 0) {
        return r;
    }

    return r;
}

void
IBusFrontEnd::panel_disconnect ()
{
    log_func();

    if (m_panel_source) {
        sd_event_source_unref(m_panel_source);
        m_panel_source = NULL; 
    }

    if (m_panel_client.is_connected()) {
        m_panel_client.close_connection();
    }
}

int
IBusFrontEnd::panel_handle_io (sd_event_source *s, int fd, uint32_t revents)
{
    log_func();

    if (!m_panel_client.filter_event ()) {
        panel_disconnect();
        panel_connect();
    }

    return 0;
}

void
IBusFrontEnd::panel_slot_reload_config (int context)
{
    log_func ();

    m_config->reload ();
}

void
IBusFrontEnd::panel_slot_exit (int context)
{
    log_func ();

    sd_event_exit (m_loop, 0);
}

void
IBusFrontEnd::panel_slot_update_lookup_table_page_size (int context, int page_size)
{
    log_func ();

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        update_lookup_table_page_size (ctx->siid (), page_size);
        m_panel_client.send ();
    }
}
void
IBusFrontEnd::panel_slot_lookup_table_page_up (int context)
{
    log_func ();

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        lookup_table_page_up (ctx->siid ());
        m_panel_client.send ();
    }
}
void
IBusFrontEnd::panel_slot_lookup_table_page_down (int context)
{
    log_func ();

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        lookup_table_page_down (ctx->siid ());
        m_panel_client.send ();
    }
}
void
IBusFrontEnd::panel_slot_trigger_property (int context, const String &property)
{
    log_func ();

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        trigger_property (ctx->siid (), property);
        m_panel_client.send ();
    }
}
void
IBusFrontEnd::panel_slot_process_helper_event (int context, const String &target_uuid, const String &helper_uuid, const Transaction &trans)
{
    log_func ();

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx) && get_instance_uuid (ctx->siid ()) == target_uuid) {
        m_panel_client.prepare (ctx->id ());
        process_helper_event (ctx->siid (), helper_uuid, trans);
        m_panel_client.send ();
    }
}
void
IBusFrontEnd::panel_slot_move_preedit_caret (int context, int caret_pos)
{
    log_func ();

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        move_preedit_caret (ctx->siid (), caret_pos);
        m_panel_client.send ();
    }
}
void
IBusFrontEnd::panel_slot_select_candidate (int context, int cand_index)
{
    log_func ();

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        select_candidate (ctx->siid (), cand_index);
        m_panel_client.send ();
    }
    log_func_not_impl ();
}
void
IBusFrontEnd::panel_slot_process_key_event (int context, const KeyEvent &key)
{
    log_func ();

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());

        if (!filter_hotkeys (ctx, key)) {
            if (!ctx->is_on () || !process_key_event (ctx->siid (), key)) {
//                if (!m_fallback_instance->process_key_event (key))
                    signal_ctx (ctx->siid (), "ForwardKeyEvent", &key);
            }
        }

        m_panel_client.send ();
    }
}
void
IBusFrontEnd::panel_slot_commit_string (int context, const WideString &wstr)
{
    log_func ();

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        commit_string (ctx->id (), wstr);
//        ims_commit_string (ctx, wstr);
    }
}
void
IBusFrontEnd::panel_slot_forward_key_event (int context, const KeyEvent &key)
{
    log_func ();

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        forward_key_event (ctx->id (), key);
//        ims_forward_key_event (ic, key);
    }
}
void
IBusFrontEnd::panel_slot_request_help (int context)
{
    log_func ();

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        panel_req_show_help (ctx);
        m_panel_client.send ();
    }
}
void
IBusFrontEnd::panel_slot_request_factory_menu (int context)
{
    log_func ();

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        panel_req_show_factory_menu (ctx);
        m_panel_client.send ();
    }
}
void
IBusFrontEnd::panel_slot_change_factory (int context, const String &uuid)
{
    log_func ();

    SCIM_DEBUG_FRONTEND (1) << "panel_slot_change_factory " << uuid << "\n";

    IBusCtx *ctx = find_ctx (context);
    if (validate_ctx (ctx)) {
        m_panel_client.prepare (ctx->id ());
        if (uuid.length () == 0 && ctx->is_on ()) {
            SCIM_DEBUG_FRONTEND (2) << "panel_slot_change_factory : turn off.\n";
            stop_ctx (ctx);
        }else if(uuid.length () == 0 && (ctx->is_on ()  == false)){
    		panel_req_update_factory_info (ctx);
        	m_panel_client.turn_off (ctx->id ());        	
        }else if (uuid.length ()) {
            String encoding = scim_get_locale_encoding (ctx->locale ());
            String language = scim_get_locale_language (ctx->locale ());
            if (validate_factory (uuid, encoding)) {
                stop_ctx (ctx);
                replace_instance (ctx->siid (), uuid);
                m_panel_client.register_input_context (ctx->id (), get_instance_uuid (ctx->siid ()));
                set_default_factory (language, uuid);
                start_ctx (ctx);
            }
        }
        m_panel_client.send ();
    }
}

void
IBusFrontEnd::panel_req_update_screen (const IBusCtx *ctx)
{
    log_func_not_impl ();

//    Window target = ic->focus_win ? ic->focus_win : ic->client_win;
//    XWindowAttributes xwa;
//    if (target && 
//        XGetWindowAttributes (m_display, target, &xwa) &&
//        validate_ctx (ic)) {
//        int num = ScreenCount (m_display);
//        int idx;
//        for (idx = 0; idx < num; ++ idx) {
//            if (ScreenOfDisplay (m_display, idx) == xwa.screen) {
//                m_panel_client.update_screen (ic->icid, idx);
                m_panel_client.update_screen (ctx->id (), 0);
//                return;
//            }
//        }
//    }
}

void
IBusFrontEnd::panel_req_show_help (const IBusCtx *ctx)
{
    log_func ();

    String help;
    String tmp;

    help =  String (_("Smart Common Input Method platform ")) +
            String (SCIM_VERSION) +
            String (_("\n(C) 2002-2005 James Su <suzhe@tsinghua.org.cn>\n\n"));

    if (ctx->is_on ()) {
        help += utf8_wcstombs (get_instance_name (ctx->siid ()));
        help += String (_(":\n\n"));

        help += utf8_wcstombs (get_instance_authors (ctx->siid ()));
        help += String (_("\n\n"));

        help += utf8_wcstombs (get_instance_help (ctx->siid ()));
        help += String (_("\n\n"));

        help += utf8_wcstombs (get_instance_credits (ctx->siid ()));
    }

    m_panel_client.show_help (ctx->id (), help);
}

void
IBusFrontEnd::panel_req_show_factory_menu (const IBusCtx *ctx)
{
    log_func ();

    std::vector<String> uuids;
    if (get_factory_list_for_encoding (uuids, ctx->encoding ())) {
        std::vector <PanelFactoryInfo> menu;
        for (size_t i = 0; i < uuids.size (); ++ i) {
            menu.push_back (PanelFactoryInfo (
                                    uuids [i],
                                    utf8_wcstombs (get_factory_name (uuids [i])),
                                    get_factory_language (uuids [i]),
                                    get_factory_icon_file (uuids [i])));
        }
        m_panel_client.show_factory_menu (ctx->id (), menu);
    }
}

void
IBusFrontEnd::panel_req_focus_in (const IBusCtx *ctx)
{
    log_func ();

    m_panel_client.focus_in (ctx->id (), get_instance_uuid (ctx->siid ()));
}

void
IBusFrontEnd::panel_req_update_factory_info (const IBusCtx *ctx)
{
    log_func ();

    if (!is_current_ctx (ctx)) {
        return;
    }

    PanelFactoryInfo info;
    if (ctx->is_on ()) {
        String uuid = get_instance_uuid (ctx->siid ());
        info = PanelFactoryInfo (uuid,
                                 utf8_wcstombs (get_factory_name (uuid)),
                                 get_factory_language (uuid),
                                 get_factory_icon_file (uuid));
    } else {
        info = PanelFactoryInfo (String (""),
                                 String (_("English/Keyboard")),
                                 String ("C"),
                                 String (SCIM_KEYBOARD_ICON_FILE));
    }
    m_panel_client.update_factory_info (ctx->id (), info);
}

void
IBusFrontEnd::panel_req_update_spot_location (const IBusCtx *ctx)
{
    log_func ();

    m_panel_client.update_spot_location (ctx->id (),
                                         ctx->cursor_location ().x,
                                         ctx->cursor_location ().y +
                                             ctx->cursor_location ().h);
}

bool
IBusFrontEnd::filter_hotkeys (IBusCtx *ctx, const KeyEvent &scimkey)
{
    bool ok = false;

    if (!is_current_ctx (ctx)) return false;

    m_frontend_hotkey_matcher.push_key_event (scimkey);
    m_imengine_hotkey_matcher.push_key_event (scimkey);

    FrontEndHotkeyAction hotkey_action = m_frontend_hotkey_matcher.get_match_result ();

    // Match trigger and show factory menu hotkeys.
    if (hotkey_action == SCIM_FRONTEND_HOTKEY_TRIGGER) {
        if (!ctx->is_on ())
            start_ctx (ctx);
        else
            stop_ctx (ctx);
        ok = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_ON) {
        if (!ctx->is_on ()) {
            start_ctx (ctx);
        }
        ok = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_OFF) {
        if (ctx->is_on ()) {
            stop_ctx (ctx);
        }
        ok = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_NEXT_FACTORY) {
        String encoding = scim_get_locale_encoding (ctx->locale ());
        String language = scim_get_locale_language (ctx->locale ());
        String sfid = get_next_factory ("", encoding, get_instance_uuid (ctx->siid ()));
        if (validate_factory (sfid, encoding)) {
            stop_ctx (ctx);
            replace_instance (ctx->siid (), sfid);
            m_panel_client.register_input_context (ctx->id (), get_instance_uuid (ctx->siid ()));
            set_default_factory (language, sfid);
            start_ctx (ctx);

            log_debug ("instance name=%s, authors=%s, encoding=%s, uuid=%s",
                    utf8_wcstombs (get_instance_name (ctx->siid ())).c_str(),
                    utf8_wcstombs (get_instance_authors (ctx->siid ())).c_str(),
                    get_instance_encoding (ctx->siid ()).c_str (),
                    get_instance_uuid (ctx->siid ()).c_str ());
        }
        ok = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_PREVIOUS_FACTORY) {
        String encoding = scim_get_locale_encoding (ctx->locale ());
        String language = scim_get_locale_language (ctx->locale ());
        String sfid = get_previous_factory ("", encoding, get_instance_uuid (ctx->siid ()));
        if (validate_factory (sfid, encoding)) {
            stop_ctx (ctx);
            replace_instance (ctx->siid (), sfid);
            m_panel_client.register_input_context (ctx->id (), get_instance_uuid (ctx->siid ()));
            set_default_factory (language, sfid);
            start_ctx (ctx);

            log_debug ("instance name=%s, authors=%s, encoding=%s, uuid=%s",
                    utf8_wcstombs (get_instance_name (ctx->siid ())).c_str(),
                    utf8_wcstombs (get_instance_authors (ctx->siid ())).c_str(),
                    get_instance_encoding (ctx->siid ()).c_str (),
                    get_instance_uuid (ctx->siid ()).c_str ());
        }
        ok = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_SHOW_FACTORY_MENU) {
        panel_req_show_factory_menu (ctx);
        ok = true;
    } else if (m_imengine_hotkey_matcher.is_matched ()) {
        String encoding = scim_get_locale_encoding (ctx->locale ());
        String language = scim_get_locale_language (ctx->locale ());
        String sfid = m_imengine_hotkey_matcher.get_match_result ();
        if (validate_factory (sfid, encoding)) {
            stop_ctx (ctx);
            replace_instance (ctx->siid (), sfid);
            m_panel_client.register_input_context (ctx->id (), get_instance_uuid (ctx->siid ()));
            set_default_factory (language, sfid);
            start_ctx (ctx);

            log_debug ("instance name=%s, authors=%s, encoding=%s, uuid=%s",
                    utf8_wcstombs (get_instance_name (ctx->siid ())).c_str(),
                    utf8_wcstombs (get_instance_authors (ctx->siid ())).c_str(),
                    get_instance_encoding (ctx->siid ()).c_str (),
                    get_instance_uuid (ctx->siid ()).c_str ());
        }
        ok = true;
    }

    return ok;
}

template<int (IBusFrontEnd::*mf)(sd_bus_message *m)>
static int
portal_message_adapter (sd_bus_message *m,
                        void *userdata,
                        sd_bus_error *ret_error)
{
    log_trace ("%s => %s.%s",
               sd_bus_message_get_sender (m),
               sd_bus_message_get_path (m),
               sd_bus_message_get_member (m));

    return (_scim_frontend->*mf)(m);
}

template<int (IBusFrontEnd::*mf)(IBusCtx *ctx, sd_bus_message *m)>
static int
ctx_message_adapter (sd_bus_message *m,
                     void *userdata,
                     sd_bus_error *ret_error)
{
    log_trace ("%s => %s.%s",
               sd_bus_message_get_sender (m),
               sd_bus_message_get_path (m),
               sd_bus_message_get_member (m));

    return (_scim_frontend->*mf)((IBusCtx *) userdata, m);
}

template<int (IBusFrontEnd::*mf) (IBusCtx *userdata, sd_bus_message *value)>
static int
ctx_prop_adapter (sd_bus *bus,
 	              const char *path,
 	              const char *interface,
 	              const char *property,
 	              sd_bus_message *value,
 	              void *userdata,
 	              sd_bus_error *ret_error)
{
    log_trace ("%s => %s.%s",
               sd_bus_message_get_sender (sd_bus_get_current_message (bus)),
               path,
               property);

    return (_scim_frontend->*mf)((IBusCtx *) userdata, value);
}

template<int (IBusFrontEnd::*mf)(sd_bus_message *m)>
static int
message_adapter(sd_bus_message *m,
                void *userdata,
                sd_bus_error *ret_error)
{
    return (_scim_frontend->*mf)(m);
}

static inline const char *
ctx_gen_object_path (int id, char *buf, size_t buf_size)
{
    assert (buf_size >= STRLEN (IBUS_INPUTCONTEXT_OBJECT_PATH) + 10);

    snprintf(buf, buf_size, IBUS_INPUTCONTEXT_OBJECT_PATH "%d", id);

    return buf;
}

//Module Interface
extern "C" {
    void scim_module_init (void)
    {
        SCIM_DEBUG_FRONTEND(1) << "Initializing Socket FrontEnd module...\n";
    }

    void scim_module_exit (void)
    {
        SCIM_DEBUG_FRONTEND(1) << "Exiting Socket FrontEnd module...\n";
        _scim_frontend.reset ();
    }

    void scim_frontend_module_init (const BackEndPointer &backend,
                                    const ConfigPointer &config,
                                    int argc,
                                     char **argv)
    {
        if (_scim_frontend.null ()) {
            SCIM_DEBUG_FRONTEND(1) << "Initializing Socket FrontEnd module (more)...\n";
            _scim_frontend = new IBusFrontEnd (backend, config);
            _scim_frontend->init (argc, argv);
        }
    }

    void scim_frontend_module_run (void)
    {
        if (!_scim_frontend.null ()) {
            SCIM_DEBUG_FRONTEND(1) << "Starting Socket FrontEnd module...\n";
            _scim_frontend->run ();
        }
    }
}

/*
vi:ts=4:nowrap:expandtab
*/
