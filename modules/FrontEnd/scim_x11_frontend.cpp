/** @file scim_x11_frontend.cpp
 * implementation of class X11FrontEnd.
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
 * $Id: scim_x11_frontend.cpp,v 1.179.2.6 2007/06/16 06:23:38 suzhe Exp $
 *
 */

#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_FRONTEND
#define Uses_SCIM_ICONV
#define Uses_SCIM_SOCKET
#define Uses_SCIM_TRANSACTION
#define Uses_SCIM_HOTKEY
#define Uses_SCIM_PANEL_CLIENT
#define Uses_C_LOCALE
#define Uses_C_STDIO
#define Uses_C_STDLIB

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include "IMdkit/IMdkit.h"
#include "IMdkit/Xi18n.h"

#include "scim_private.h"
#include "scim.h"

#include "scim_x11_ic.h"
#include "scim_x11_frontend.h"
#include "scim_x11_utils.h"

#define scim_module_init x11_LTX_scim_module_init
#define scim_module_exit x11_LTX_scim_module_exit
#define scim_frontend_module_init x11_LTX_scim_frontend_module_init
#define scim_frontend_module_run x11_LTX_scim_frontend_module_run

#define SCIM_CONFIG_FRONTEND_X11_BROKEN_WCHAR    "/FrontEnd/X11/BrokenWchar"
#define SCIM_CONFIG_FRONTEND_X11_DYNAMIC         "/FrontEnd/X11/Dynamic"
#define SCIM_CONFIG_FRONTEND_X11_SERVER_NAME     "/FrontEnd/X11/ServerName"
#define SCIM_CONFIG_FRONTEND_X11_ONTHESPOT       "/FrontEnd/X11/OnTheSpot"

#define SCIM_KEYBOARD_ICON_FILE            (SCIM_ICONDIR "/keyboard.png")

using namespace scim;

//Local static data
static Pointer <X11FrontEnd> _scim_frontend (0);

//Module Interface
extern "C" {
    void scim_module_init (void)
    {
        SCIM_DEBUG_FRONTEND(1) << "Initializing X11 FrontEnd module...\n";
    }

    void scim_module_exit (void)
    {
        SCIM_DEBUG_FRONTEND(1) << "Exiting X11 FrontEnd module...\n";
        _scim_frontend.reset ();
    }

    void scim_frontend_module_init (const BackEndPointer &backend,
                                    const ConfigPointer &config,
                                    int argc,
                                     char **argv)
    {
        if (config.null () || backend.null ()) 
            throw FrontEndError (String ("X11 FrontEnd couldn't run without Config and BackEnd.\n")); 

        if (_scim_frontend.null ()) {
            SCIM_DEBUG_FRONTEND(1) << "Initializing X11 FrontEnd module (more)...\n";
            _scim_frontend = new X11FrontEnd (backend, config);
            _scim_frontend->init (argc, argv);
        }
    }

    void scim_frontend_module_run (void)
    {
        if (!_scim_frontend.null ()) {
            SCIM_DEBUG_FRONTEND(1) << "Starting X11 FrontEnd module...\n";
            _scim_frontend->run ();
        }
    }
}

X11FrontEnd::X11FrontEnd (const BackEndPointer &backend,
                          const ConfigPointer &config,
                          const String& server_name)
    : FrontEndBase (backend),
      m_xims (0),
      m_display (0),
      m_xims_window (0),
      m_server_name (server_name),
      m_focus_ic (0),
      m_xims_dynamic (true),
      m_wchar_ucs4_equal (scim_if_wchar_ucs4_equal ()),
      m_broken_wchar (false),
      m_shared_input_method (false),
      m_keyboard_layout (SCIM_KEYBOARD_Default),
      m_valid_key_mask (SCIM_KEY_AllMasks),
      m_should_exit (false),
      m_iconv (String ()),
      m_config (config),
      m_old_x_error_handler (0)
{
    if (!_scim_frontend.null () && _scim_frontend != this) {
        throw FrontEndError (
            String ("X11 -- only one frontend can be created!"));
    }

    if (!m_server_name.length ())
        m_server_name = String ("SCIM");

    // Attach Panel Client signal.
    m_panel_client.signal_connect_reload_config                 (slot (this, &X11FrontEnd::panel_slot_reload_config));
    m_panel_client.signal_connect_exit                          (slot (this, &X11FrontEnd::panel_slot_exit));
    m_panel_client.signal_connect_update_lookup_table_page_size (slot (this, &X11FrontEnd::panel_slot_update_lookup_table_page_size));
    m_panel_client.signal_connect_lookup_table_page_up          (slot (this, &X11FrontEnd::panel_slot_lookup_table_page_up));
    m_panel_client.signal_connect_lookup_table_page_down        (slot (this, &X11FrontEnd::panel_slot_lookup_table_page_down));
    m_panel_client.signal_connect_trigger_property              (slot (this, &X11FrontEnd::panel_slot_trigger_property));
    m_panel_client.signal_connect_process_helper_event          (slot (this, &X11FrontEnd::panel_slot_process_helper_event));
    m_panel_client.signal_connect_move_preedit_caret            (slot (this, &X11FrontEnd::panel_slot_move_preedit_caret));
    m_panel_client.signal_connect_select_candidate              (slot (this, &X11FrontEnd::panel_slot_select_candidate));
    m_panel_client.signal_connect_process_key_event             (slot (this, &X11FrontEnd::panel_slot_process_key_event));
    m_panel_client.signal_connect_commit_string                 (slot (this, &X11FrontEnd::panel_slot_commit_string));
    m_panel_client.signal_connect_forward_key_event             (slot (this, &X11FrontEnd::panel_slot_forward_key_event));
    m_panel_client.signal_connect_request_help                  (slot (this, &X11FrontEnd::panel_slot_request_help));
    m_panel_client.signal_connect_request_factory_menu          (slot (this, &X11FrontEnd::panel_slot_request_factory_menu));
    m_panel_client.signal_connect_change_factory                (slot (this, &X11FrontEnd::panel_slot_change_factory));
}

X11FrontEnd::~X11FrontEnd ()
{
    if (m_xims) {
        if (validate_ic (m_focus_ic)) {
            m_panel_client.prepare (m_focus_ic->icid);
            focus_out (m_focus_ic->siid);
            m_panel_client.turn_off (m_focus_ic->icid);
            m_panel_client.send ();
            ims_sync_ic (m_focus_ic);
        }

        XSync(m_display, False);
        IMCloseIM (m_xims);
    }

    if (m_display && m_xims_window) {
        XDestroyWindow (m_display, m_xims_window);
        XCloseDisplay (m_display);
    }

    m_panel_client.close_connection ();
}

void
X11FrontEnd::show_preedit_string (int siid)
{
    SCIM_DEBUG_FRONTEND(2) << " Show preedit string, siid=" << siid << "\n";

    if (is_inputing_ic (siid)) {
        if (ims_is_preedit_callback_mode (m_focus_ic))
            ims_preedit_callback_start (m_focus_ic);
        else
            m_panel_client.show_preedit_string (m_focus_ic->icid);
    }
}

void
X11FrontEnd::show_aux_string (int siid)
{
    SCIM_DEBUG_FRONTEND(2) << " Show aux string, siid=" << siid << "\n";

    if (is_inputing_ic (siid))
        m_panel_client.show_aux_string (m_focus_ic->icid);
}

void
X11FrontEnd::show_lookup_table (int siid)
{
    SCIM_DEBUG_FRONTEND(2) << " Show lookup table, siid=" << siid << "\n";

    if (is_inputing_ic (siid))
        m_panel_client.show_lookup_table (m_focus_ic->icid);
}

void
X11FrontEnd::hide_preedit_string (int siid)
{
    SCIM_DEBUG_FRONTEND(2) << " Hide preedit string, siid=" << siid << "\n";

    if (is_focused_ic (siid)) {
        if (ims_is_preedit_callback_mode (m_focus_ic))
            ims_preedit_callback_done (m_focus_ic);
        else
            m_panel_client.hide_preedit_string (m_focus_ic->icid);
    }
}

void
X11FrontEnd::hide_aux_string (int siid)
{
    SCIM_DEBUG_FRONTEND(2) << " Hide aux string, siid=" << siid << "\n";

    if (is_focused_ic (siid))
        m_panel_client.hide_aux_string (m_focus_ic->icid);
}

void
X11FrontEnd::hide_lookup_table (int siid)
{
    SCIM_DEBUG_FRONTEND(2) << " Hide lookup table, siid=" << siid << "\n";

    if (is_focused_ic (siid))
        m_panel_client.hide_lookup_table (m_focus_ic->icid);
}

void
X11FrontEnd::update_preedit_caret (int siid, int caret)
{
    SCIM_DEBUG_FRONTEND(2) << " Update preedit caret, siid=" << siid << " caret=" << caret << "\n";

    if (is_inputing_ic (siid)) {
        if (ims_is_preedit_callback_mode (m_focus_ic))
            ims_preedit_callback_caret (m_focus_ic, caret);
        else
            m_panel_client.update_preedit_caret (m_focus_ic->icid, caret);
    }
}

void
X11FrontEnd::update_preedit_string (int siid, const WideString & str, const AttributeList & attrs)
{
    SCIM_DEBUG_FRONTEND(2) << " Update preedit string, siid=" << siid << "\n";

    if (is_inputing_ic (siid)) {
        if (ims_is_preedit_callback_mode (m_focus_ic))
            ims_preedit_callback_draw (m_focus_ic, str, attrs);
        else
            m_panel_client.update_preedit_string (m_focus_ic->icid, str, attrs); 
    }
}

void
X11FrontEnd::update_aux_string (int siid, const WideString & str, const AttributeList & attrs)
{
    SCIM_DEBUG_FRONTEND(2) << " Update aux string, siid=" << siid << "\n";

    if (is_inputing_ic (siid))
        m_panel_client.update_aux_string (m_focus_ic->icid, str, attrs); 
}

void
X11FrontEnd::update_lookup_table (int siid, const LookupTable & table)
{
    SCIM_DEBUG_FRONTEND(2) << " Update lookup table, siid=" << siid << "\n";

    if (is_inputing_ic (siid))
        m_panel_client.update_lookup_table (m_focus_ic->icid, table);
}

void
X11FrontEnd::commit_string (int siid, const WideString & str)
{
    SCIM_DEBUG_FRONTEND(2) << " Commit string, siid=" << siid << "\n";

    if (is_focused_ic (siid))
        ims_commit_string (m_focus_ic, str);
}

void
X11FrontEnd::forward_key_event (int siid, const KeyEvent & key)
{
    SCIM_DEBUG_FRONTEND(2) << " Forward keyevent, siid=" << siid << "\n";

    if (is_focused_ic (siid))
        ims_forward_key_event (m_focus_ic, key);
}

void
X11FrontEnd::register_properties (int siid, const PropertyList &properties)
{
    SCIM_DEBUG_FRONTEND(2) << " Register properties, siid=" << siid << "\n";

    if (is_inputing_ic (siid))
        m_panel_client.register_properties (m_focus_ic->icid, properties);
}

void
X11FrontEnd::update_property (int siid, const Property &property)
{
    SCIM_DEBUG_FRONTEND(2) << " Update property, siid=" << siid << "\n";

    if (is_inputing_ic (siid))
        m_panel_client.update_property (m_focus_ic->icid, property);
}

void
X11FrontEnd::beep (int siid)
{
    SCIM_DEBUG_FRONTEND(2) << " Beep, siid=" << siid << "\n";

    if (is_inputing_ic (siid))
        XBell (m_display, 0);
}

void
X11FrontEnd::start_helper (int siid, const String &helper_uuid)
{
    SCIM_DEBUG_FRONTEND(2) << " Start helper, siid=" << siid << " Helper=" << helper_uuid << "\n";

    X11IC *ic = m_ic_manager.find_ic_by_siid (siid);

    if (validate_ic (ic))
        m_panel_client.start_helper (ic->icid, helper_uuid);
}

void
X11FrontEnd::stop_helper (int siid, const String &helper_uuid)
{
    SCIM_DEBUG_FRONTEND(2) << " Stop helper, siid=" << siid << " Helper=" << helper_uuid << "\n";

    X11IC *ic = m_ic_manager.find_ic_by_siid (siid);

    if (validate_ic (ic))
        m_panel_client.stop_helper (ic->icid, helper_uuid);
}

void
X11FrontEnd::send_helper_event (int siid, const String &helper_uuid, const Transaction &trans)
{
    SCIM_DEBUG_FRONTEND(2) << " Send helper event, siid=" << siid << " Helper=" << helper_uuid << "\n";

    X11IC *ic = m_ic_manager.find_ic_by_siid (siid);

    if (validate_ic (ic))
        m_panel_client.send_helper_event (ic->icid, helper_uuid, trans);
}

bool
X11FrontEnd::get_surrounding_text (int siid, WideString &text, int &cursor, int maxlen_before, int maxlen_after)
{
    SCIM_DEBUG_FRONTEND(2) << " Get surrounding text, siid=" << siid << "\n";

    text.clear ();
    cursor = 0;

    if (is_inputing_ic (siid)) {
//      return ims_string_conversion_callback_retrieval (m_focus_ic, text, cursor, maxlen_before, maxlen_after);
        return false;
    }

    return false;
}

bool
X11FrontEnd::delete_surrounding_text (int siid, int offset, int len)
{
    SCIM_DEBUG_FRONTEND(2) << " Delete surrounding text, siid=" << siid << " offset = " << offset << " len = " << len << "\n";

    if (is_inputing_ic (siid)) {
//      return ims_string_conversion_callback_substitution (m_focus_ic, offset, len);
        return false;
    }

    return false;
}

void
X11FrontEnd::init (int argc, char **argv)
{
    String str;

    SCIM_DEBUG_FRONTEND (1) << "X11 -- Loading configuration.\n";

    //Read settings.
    reload_config_callback (m_config);

    m_server_name =
        m_config->read (String (SCIM_CONFIG_FRONTEND_X11_SERVER_NAME),
                        m_server_name);

    m_xims_dynamic =
        m_config->read (String (SCIM_CONFIG_FRONTEND_X11_DYNAMIC),
                        true);

    m_config->signal_connect_reload (slot (this, &X11FrontEnd::reload_config_callback));

    m_display_name = init_ims ();

    SCIM_DEBUG_FRONTEND (1) << "X11 -- Connecting to panel daemon.\n";

    if (m_panel_client.open_connection (m_config->get_name (), m_display_name) < 0)
        throw FrontEndError (String ("X11 -- failed to connect to the panel daemon!"));

    // Only use ComposeKeyFactory when it's enabled.
    if (validate_factory (SCIM_COMPOSE_KEY_FACTORY_UUID, "UTF-8"))
        m_fallback_factory = new ComposeKeyFactory ();
    else
        m_fallback_factory = new DummyIMEngineFactory ();

    m_fallback_instance = m_fallback_factory->create_instance (String ("UTF-8"), 0);
    m_fallback_instance->signal_connect_commit_string (slot (this, &X11FrontEnd::fallback_commit_string_cb));
}

void
X11FrontEnd::run ()
{
    if (!m_display || !m_xims_window || !m_xims || m_panel_client.get_connection_number () < 0) {
        SCIM_DEBUG_FRONTEND(1) << "X11 -- Cannot run without initialization!\n";
        return;
    }

    XEvent event;

    fd_set read_fds, active_fds;

    int panel_fd = m_panel_client.get_connection_number ();
    int xserver_fd = ConnectionNumber (m_display);
    int max_fd = (panel_fd > xserver_fd) ? panel_fd : xserver_fd;

    FD_ZERO (&active_fds);
    FD_SET (panel_fd, &active_fds);
    FD_SET (xserver_fd, &active_fds);

    m_should_exit = false;

    // Select between the X Server and the Panel GUI.
    while (!m_should_exit) {
        int ret;

        read_fds = active_fds;

        // Process the events which are already send to me from the X Server
        // before calling select.
        while (XPending (m_display)) {
            XNextEvent (m_display, &event);
            XFilterEvent (&event, None);
        }

        if ((ret = select (max_fd + 1, &read_fds, NULL, NULL, NULL)) < 0) {
            SCIM_DEBUG_FRONTEND(1) << "X11 -- Error when watching events!\n";
            return;
        }

        if (m_should_exit) break;

        if (FD_ISSET (panel_fd, &read_fds)) {
            if (!m_panel_client.filter_event ()) {
                SCIM_DEBUG_FRONTEND(1) << "X11 -- Lost connection with panel daemon, re-establish it!\n";

                m_panel_client.close_connection ();

                max_fd = xserver_fd;
                FD_ZERO (&active_fds);
                FD_SET (xserver_fd, &active_fds);

                if (m_panel_client.open_connection (m_config->get_name (), m_display_name) >= 0) {
                    panel_fd = m_panel_client.get_connection_number ();
                    FD_SET (panel_fd, &active_fds);
                    max_fd = (panel_fd > xserver_fd) ? panel_fd : xserver_fd;
                } else {
                    panel_fd = -1;
                    SCIM_DEBUG_FRONTEND(1) << "X11 -- Lost connection with panel daemon, can't re-establish it!\n";
                }
            }
        }
        // X Events will be processed at beginning of the loop.
    }
}

String
X11FrontEnd::get_supported_locales (void)
{
    std::vector <String> all_locales;
    std::vector <String> supported_locales;

    scim_split_string_list (all_locales, get_all_locales (), ',');

    String last = String (setlocale (LC_CTYPE, 0));

    for (size_t i = 0; i < all_locales.size (); ++i) {
        if (setlocale (LC_CTYPE, all_locales [i].c_str ()) && XSupportsLocale ())
            supported_locales.push_back (all_locales [i]);
    }

    setlocale (LC_CTYPE, last.c_str ());

    return scim_combine_string_list (supported_locales, ',');
}

int
X11FrontEnd::get_default_instance (const String &language, const String &encoding)
{
    DefaultInstanceMap::iterator it = m_default_instance_map.find (encoding);
    String sfid = get_default_factory (language, encoding);

    if (it == m_default_instance_map.end ()) {
        int siid = new_instance (sfid, encoding);

        m_default_instance_map [encoding] = siid;

        return siid;
    } else if (get_instance_uuid (it->second) != sfid) {
        replace_instance (it->second, sfid);
    }
    return it->second;
}

String
X11FrontEnd::init_ims (void)
{
    XIMStyle ims_styles_overspot [] = {
        XIMPreeditPosition  | XIMStatusNothing,
        XIMPreeditNothing   | XIMStatusNothing,
        XIMPreeditPosition  | XIMStatusCallbacks,
        XIMPreeditNothing   | XIMStatusCallbacks,
        0
    };

    XIMStyle ims_styles_onspot [] = {
        XIMPreeditPosition  | XIMStatusNothing,
        XIMPreeditCallbacks | XIMStatusNothing,
        XIMPreeditNothing   | XIMStatusNothing,
        XIMPreeditPosition  | XIMStatusCallbacks,
        XIMPreeditCallbacks | XIMStatusCallbacks,
        XIMPreeditNothing   | XIMStatusCallbacks,
        0
    };

    XIMEncoding ims_encodings[] = {
        const_cast<char*> ("COMPOUND_TEXT"),
        0
    };

    XIMStyles styles;
    XIMEncodings encodings;

    String locales;

    locales = get_supported_locales ();

    SCIM_DEBUG_FRONTEND(1) << "Initializing XIMS: "
            << m_server_name << " with locale (" << locales.length () << "): " << locales << " ...\n";

    // Initialize X Display and Root Windows.
    if (m_xims != (XIMS) 0) {
        throw FrontEndError (String ("X11 -- XIMS already initialized!"));
       }

    m_display = XOpenDisplay (NULL);

    if (!m_display)
        throw FrontEndError (String ("X11 -- Cannot open Display!"));

    m_xims_window = XCreateSimpleWindow (m_display,
                                         DefaultRootWindow (m_display),
                                         -1, -1, 1, 1, 0, 0, 0);

    if (!m_xims_window)
        throw FrontEndError (String ("X11 -- Cannot create IMS Window!"));

    XSetWindowAttributes attrs;
    unsigned long attrmask;

    attrs.override_redirect = true;
    attrmask = CWOverrideRedirect;

    XChangeWindowAttributes (m_display, m_xims_window, attrmask, &attrs);
    XSelectInput (m_display, m_xims_window, KeyPressMask | KeyReleaseMask);

    m_old_x_error_handler = XSetErrorHandler (x_error_handler);

    // Initialize XIMS.
    if (m_config->read (String (SCIM_CONFIG_FRONTEND_ON_THE_SPOT), true) &&
        m_config->read (String (SCIM_CONFIG_FRONTEND_X11_ONTHESPOT), true)) {
        styles.count_styles = sizeof (ims_styles_onspot)/sizeof (XIMStyle) - 1;
        styles.supported_styles = ims_styles_onspot;
    } else {
        styles.count_styles = sizeof (ims_styles_overspot)/sizeof (XIMStyle) - 1;
        styles.supported_styles = ims_styles_overspot;
    }

    encodings.count_encodings = sizeof (ims_encodings)/sizeof (XIMEncoding) - 1;
    encodings.supported_encodings = ims_encodings;

    m_xims = IMOpenIM(m_display,
            IMModifiers, "Xi18n",
            IMServerWindow, m_xims_window,
            IMServerName, m_server_name.c_str (),
            IMLocale, locales.c_str (),
            IMServerTransport, "X/",
            IMInputStyles, &styles,
            IMEncodingList, &encodings,
            IMProtocolHandler, ims_protocol_handler,
            IMFilterEventMask, KeyPressMask | KeyReleaseMask,
            NULL);

    if (m_xims == (XIMS)NULL)
        throw FrontEndError (String ("X11 -- failed to initialize XIM Server!"));

    if (m_xims_dynamic) {
        XIMTriggerKey xim_on_key_list[10];
        XIMTriggerKey xim_off_key_list[10];
 
        XIMTriggerKeys xim_on_keys;
        XIMTriggerKeys xim_off_keys;
 
        uint32 count_trigger_keys, count_on_keys, count_off_keys;
 
        KeyEventList keys;
 
        m_frontend_hotkey_matcher.find_hotkeys (SCIM_FRONTEND_HOTKEY_TRIGGER, keys);
 
        for (count_trigger_keys=0; count_trigger_keys < 10 && count_trigger_keys < keys.size (); ++count_trigger_keys) {
            XKeyEvent xkey = scim_x11_keyevent_scim_to_x11 (m_display, keys [count_trigger_keys]);
            xim_on_key_list [count_trigger_keys].keysym = keys [count_trigger_keys].code;
            xim_on_key_list [count_trigger_keys].modifier = xkey.state;
            xim_on_key_list [count_trigger_keys].modifier_mask = xkey.state;
        }
 
        m_frontend_hotkey_matcher.find_hotkeys (SCIM_FRONTEND_HOTKEY_ON, keys);

        for (count_on_keys=count_trigger_keys; count_on_keys < 10 && (count_on_keys - count_trigger_keys) < keys.size (); ++count_on_keys) {
            XKeyEvent xkey = scim_x11_keyevent_scim_to_x11 (m_display, keys [count_on_keys - count_trigger_keys]);
            xim_on_key_list [count_on_keys].keysym = keys [count_on_keys - count_trigger_keys].code;
            xim_on_key_list [count_on_keys].modifier = xkey.state;
            xim_on_key_list [count_on_keys].modifier_mask = xkey.state;
        }
 
        m_frontend_hotkey_matcher.find_hotkeys (SCIM_FRONTEND_HOTKEY_OFF, keys);

        for (count_off_keys=0; count_off_keys < 10 && count_off_keys < keys.size (); ++count_off_keys) {
            XKeyEvent xkey = scim_x11_keyevent_scim_to_x11 (m_display, keys [count_off_keys]);
            xim_off_key_list [count_off_keys].keysym = keys [count_off_keys].code;
            xim_off_key_list [count_off_keys].modifier = xkey.state;
            xim_off_key_list [count_off_keys].modifier_mask = xkey.state;
        }
 
        xim_on_keys.count_keys = count_on_keys;
        xim_on_keys.keylist = xim_on_key_list;

        xim_off_keys.count_keys = count_off_keys;
        xim_off_keys.keylist = xim_off_key_list;

        IMSetIMValues(m_xims, IMOnKeysList, &xim_on_keys, IMOffKeysList, &xim_off_keys, NULL);
    }

    return String (DisplayString (m_display));
}

bool
X11FrontEnd::filter_hotkeys (X11IC *ic, const KeyEvent &scimkey)
{
    bool ok = false;

    if (!is_focused_ic (ic)) return false;

    m_frontend_hotkey_matcher.push_key_event (scimkey);
    m_imengine_hotkey_matcher.push_key_event (scimkey);

    FrontEndHotkeyAction hotkey_action = m_frontend_hotkey_matcher.get_match_result ();

    // Match trigger and show factory menu hotkeys.
    if (hotkey_action == SCIM_FRONTEND_HOTKEY_TRIGGER) {
        if (!ic->xims_on)
            ims_turn_on_ic (ic);
        else
            ims_turn_off_ic (ic);
        ok = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_ON) {
        if (!ic->xims_on) ims_turn_on_ic (ic);
        ok = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_OFF) {
        if (ic->xims_on) ims_turn_off_ic (ic);
        ok = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_NEXT_FACTORY) {
        String encoding = scim_get_locale_encoding (ic->locale);
        String language = scim_get_locale_language (ic->locale);
        String sfid = get_next_factory ("", encoding, get_instance_uuid (ic->siid));
        if (validate_factory (sfid, encoding)) {
            ims_turn_off_ic (ic);
            replace_instance (ic->siid, sfid);
            m_panel_client.register_input_context (ic->icid, get_instance_uuid (ic->siid));
            set_ic_capabilities (ic);
            set_default_factory (language, sfid);
            ims_turn_on_ic (ic);
        }
        ok = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_PREVIOUS_FACTORY) {
        String encoding = scim_get_locale_encoding (ic->locale);
        String language = scim_get_locale_language (ic->locale);
        String sfid = get_previous_factory ("", encoding, get_instance_uuid (ic->siid));
        if (validate_factory (sfid, encoding)) {
            ims_turn_off_ic (ic);
            replace_instance (ic->siid, sfid);
            m_panel_client.register_input_context (ic->icid, get_instance_uuid (ic->siid));
            set_ic_capabilities (ic);
            set_default_factory (language, sfid);
            ims_turn_on_ic (ic);
        }
        ok = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_SHOW_FACTORY_MENU) {
        panel_req_show_factory_menu (ic);
        ok = true;
    } else if (m_imengine_hotkey_matcher.is_matched ()) {
        String encoding = scim_get_locale_encoding (ic->locale);
        String language = scim_get_locale_language (ic->locale);
        String sfid = m_imengine_hotkey_matcher.get_match_result ();
        if (validate_factory (sfid, encoding)) {
            ims_turn_off_ic (ic); 
            replace_instance (ic->siid, sfid);
            m_panel_client.register_input_context (ic->icid, get_instance_uuid (ic->siid));
            set_ic_capabilities (ic);
            set_default_factory (language, sfid);
            ims_turn_on_ic (ic);
        }
        ok = true;
    }
    return ok;
}

int
X11FrontEnd::ims_open_handler (XIMS ims, IMOpenStruct *call_data)
{
    SCIM_DEBUG_FRONTEND(2) << " IMS Open handler: LANG=" << call_data->lang.name
                 << " Connect ID=" << call_data->connect_id << "\n";

    m_ic_manager.new_connection (call_data);
    return 1;
}

int
X11FrontEnd::ims_close_handler (XIMS ims, IMCloseStruct *call_data)
{
    SCIM_DEBUG_FRONTEND(2) << " IMS Close handler: Connect ID="
            << call_data->connect_id << "\n";

    m_ic_manager.delete_connection (call_data);
    return 1;
}

int
X11FrontEnd::ims_create_ic_handler (XIMS ims, IMChangeICStruct *call_data)
{
    String locale = m_ic_manager.get_connection_locale (call_data->connect_id);
    String language = scim_get_locale_language (locale);
    String encoding = scim_get_locale_encoding (locale);

    SCIM_DEBUG_FRONTEND(2) << " IMS Create handler: Encoding=" << encoding << "\n";

    if (language.empty () || encoding.empty ())
        return 0;

    int siid = -1;

    if (m_shared_input_method) {
        siid = get_default_instance (language, encoding);
    } else {
        String sfid = get_default_factory (language, encoding);
        siid = new_instance (sfid, encoding);
    }

    if (siid >= 0) {
        uint32 attrs = m_ic_manager.create_ic (call_data, siid);

        X11IC *ic = m_ic_manager.find_ic (call_data->icid);

        SCIM_DEBUG_FRONTEND(2) << " IMS Create handler OK: SIID="
            << siid << " ICID = " << ic->icid << " Connect ID=" << call_data->connect_id  << "\n";

        m_panel_client.prepare (ic->icid);

        m_panel_client.register_input_context (ic->icid, get_instance_uuid (siid));

        if (attrs & SCIM_X11_IC_INPUT_STYLE)
            set_ic_capabilities (ic);

        m_panel_client.send ();

        if (m_shared_input_method) {
            ic->xims_on = m_config->read (String (SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), ic->xims_on);
            ic->shared_siid = true;
        }

        return 1;
    } else {
        SCIM_DEBUG_FRONTEND(2) << " IMS Create handler Failed: "
            << " Connect ID=" << call_data->connect_id  << "\n";
    }
    return 0;
}

int
X11FrontEnd::ims_set_ic_values_handler (XIMS ims, IMChangeICStruct *call_data)
{
    uint32 changes;

    X11IC *ic = m_ic_manager.find_ic (call_data->icid);

    if (!validate_ic (ic)) {
        SCIM_DEBUG_FRONTEND(1) << "Cannot find IC for icid " << call_data->icid << "\n";
        return 0;
    }

    changes = m_ic_manager.set_ic_values (call_data);

    if (changes & SCIM_X11_IC_ENCODING) {
        SCIM_DEBUG_FRONTEND(1) << "Cannot change IC encoding on the fly!\n";
        return 0;
    }

    SCIM_DEBUG_FRONTEND(2) << " IMS Set IC values handler, ICID="
                    << call_data->icid << " Connect ID="
                    << call_data->connect_id << " Changes="
                    << changes << "\n";

    m_panel_client.prepare (ic->icid);

    //It's focus IC
    if (is_focused_ic (ic)) {
        if (changes & SCIM_X11_IC_PRE_SPOT_LOCATION)
            panel_req_update_spot_location (ic);
    }

    // Update the client capabilities, if the input style was changed.
    if (changes & SCIM_X11_IC_INPUT_STYLE)
        set_ic_capabilities (ic);

    m_panel_client.send ();

    return 1;
}

int
X11FrontEnd::ims_get_ic_values_handler (XIMS ims, IMChangeICStruct *call_data)
{
    SCIM_DEBUG_FRONTEND(2) << " IMS Get IC values handler, ICID="
                    << call_data->icid << " Connect ID="
                    << call_data->connect_id << "\n";

    m_ic_manager.get_ic_values (call_data);
    return 1;
}

int
X11FrontEnd::ims_destroy_ic_handler (XIMS ims, IMDestroyICStruct *call_data)
{
    X11IC *ic = m_ic_manager.find_ic (call_data->icid);

    SCIM_DEBUG_FRONTEND(2) << " IMS Destroy IC handler, ICID="
                    << call_data->icid << " Connect ID=" 
                    << call_data->connect_id << "\n";

    if (!validate_ic (ic)) {
        SCIM_DEBUG_FRONTEND(1) << "Cannot find IC for icid " << call_data->icid << "\n";
        return 0;
    }

    m_panel_client.prepare (ic->icid);

    if (is_focused_ic (ic)) {
        focus_out (ic->siid);
        m_panel_client.turn_off (ic->icid);
        m_panel_client.focus_out (ic->icid);
    }

    X11IC *old_focus = m_focus_ic;

    m_focus_ic = ic;

    if (!ic->shared_siid) delete_instance (ic->siid);

    m_panel_client.remove_input_context (ic->icid);
    m_panel_client.send ();

    if (is_focused_ic (ic))
        m_focus_ic = 0;
    else
        m_focus_ic = old_focus;

    m_ic_manager.destroy_ic (call_data);
    return 1;
}

int
X11FrontEnd::ims_set_ic_focus_handler (XIMS ims, IMChangeFocusStruct *call_data)
{
    SCIM_DEBUG_FRONTEND(2) << " IMS Set IC focus handler, ID="
                    << call_data->icid << " Connect ID="
                    << call_data->connect_id << "\n";
    
    X11IC *ic =m_ic_manager.find_ic (call_data->icid);

    if (!validate_ic (ic)) {
        SCIM_DEBUG_FRONTEND(1) << "Cannot find IC for icid " << call_data->icid << "\n";
        return 0;
    }

    if (validate_ic (m_focus_ic) && m_focus_ic->icid != ic->icid) {
        m_panel_client.prepare (m_focus_ic->icid);
        stop_ic (m_focus_ic);
        m_panel_client.focus_out (m_focus_ic->icid);
        m_panel_client.send ();
    }

    String encoding = scim_get_locale_encoding (ic->locale);
    String language = scim_get_locale_language (ic->locale);
    bool need_reg = false;
    bool need_cap = false;
    bool need_reset = false;

    m_focus_ic = ic;

    m_panel_client.prepare (ic->icid);

    if (m_shared_input_method) {
        SCIM_DEBUG_FRONTEND(3) << "Shared input method.\n";

        if (!ic->shared_siid) {
            delete_instance (ic->siid);
            ic->shared_siid = true;
        }

        ic->siid = get_default_instance (language, encoding);
        ic->onspot_preedit_started = false;
        ic->onspot_preedit_length = 0;
        ic->onspot_caret = 0;

        ic->xims_on = m_config->read (String (SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), ic->xims_on);

        need_reg = true;
        need_cap = true;
        need_reset = true;
    } else if (ic->shared_siid) {
        String sfid = get_default_factory (language, encoding);
        ic->siid = new_instance (sfid, encoding);
        ic->onspot_preedit_started = false;
        ic->onspot_preedit_length = 0;
        ic->onspot_caret = 0;
        ic->shared_siid = false;
        need_reg = true;
        need_cap = true;
    }

    panel_req_focus_in (ic);

    if (need_reset) reset (ic->siid);
    if (need_cap) set_ic_capabilities (ic);
    if (need_reg) m_panel_client.register_input_context (ic->icid, get_instance_uuid (ic->siid));

    if (ic->xims_on) {
        start_ic (ic);
    } else {
    	panel_req_update_factory_info (ic);
        m_panel_client.turn_off (ic->icid);
    }

    m_panel_client.send ();

    return 1;
}

int
X11FrontEnd::ims_unset_ic_focus_handler (XIMS ims, IMChangeFocusStruct *call_data)
{
    SCIM_DEBUG_FRONTEND(2) << " IMS Unset IC focus handler, ID="
                    << call_data->icid << " Connect ID="
                    << call_data->connect_id << "\n";

    X11IC *ic =m_ic_manager.find_ic (call_data->icid);

    if (!validate_ic (ic)) {
        SCIM_DEBUG_FRONTEND(1) << "Cannot find IC for icid " << call_data->icid << "\n";
        return 0;
    }

    if (is_focused_ic (ic)) {
        m_panel_client.prepare (ic->icid);
        stop_ic (ic);
        m_panel_client.focus_out (ic->icid);
        m_panel_client.send ();
        m_focus_ic = 0;
    }

    return 1;
}

int
X11FrontEnd::ims_reset_ic_handler (XIMS ims, IMResetICStruct *call_data)
{
    SCIM_DEBUG_FRONTEND(2) << " IMS Reset IC handler, ID="
                    << call_data->icid << " Connect ID="
                    << call_data->connect_id << "\n";
    
    X11IC *ic =m_ic_manager.find_ic (call_data->icid);

    if (!validate_ic (ic)) {
        SCIM_DEBUG_FRONTEND(1) << "Cannot find IC for icid " << call_data->icid << "\n";
        return 0;
    }

    m_panel_client.prepare (ic->icid);
    reset (ic->siid);
    m_panel_client.send ();

    return 1;
}

int
X11FrontEnd::ims_trigger_notify_handler (XIMS ims, IMTriggerNotifyStruct *call_data)
{
    SCIM_DEBUG_FRONTEND(2) << " IMS Trigger notify handler, Flag="
                    << call_data->flag << " KeyIndex="
                    << call_data->key_index << " EventMask="
                    << call_data->event_mask << "\n";

    X11IC *ic =m_ic_manager.find_ic (call_data->icid);

    if (!validate_ic (ic)) {
        SCIM_DEBUG_FRONTEND(1) << "Cannot find IC for icid " << call_data->icid << "\n";
        return 0;
    }

    int ret = 0;

    m_panel_client.prepare (ic->icid);

    if (!call_data->flag) {
        ims_turn_on_ic (ic);
        ret = 1; 
    } else {
        ims_turn_off_ic (ic);
        ret = 1;
    }

    m_panel_client.send ();

    return ret;
}

int
X11FrontEnd::ims_forward_event_handler (XIMS ims, IMForwardEventStruct *call_data)
{
    SCIM_DEBUG_FRONTEND(2) << " IMS Forward event handler, ICID="
                    << call_data->icid << " Connect ID="
                    << call_data->connect_id << " SerialNo="
                    << call_data->serial_number << "EventType="
                    << call_data->event.type << "\n";
    
    if (call_data->event.type != KeyPress && call_data->event.type != KeyRelease)
        return 1;

    X11IC *ic = m_ic_manager.find_ic (call_data->icid);

    if (!validate_ic (ic)) {
        SCIM_DEBUG_FRONTEND(1) << "Cannot find IC for icid " << call_data->icid << "\n";
        return 0;
    }

    // If the ic is not focused, then return.
    if (!is_focused_ic (ic)) {
        SCIM_DEBUG_FRONTEND(1) << "IC " << call_data->icid << " is not focused, focus it first.\n";
        return 1;
    }

    XKeyEvent *event = (XKeyEvent*) &(call_data->event);
    KeyEvent scimkey = scim_x11_keyevent_x11_to_scim (m_display, *event);

    scimkey.mask  &= m_valid_key_mask;

    // Set keyboard layout information.
    scimkey.layout = m_keyboard_layout;

    SCIM_DEBUG_FRONTEND(3)  << "  KeyEvent:\n"
                            << "   Type=" << event->type << " Display=" << event->display << " Serial=" << event->serial << " Send=" << event->send_event << "\n"
                            << "      X=" << event->x << " Y=" << event->y << " XRoot=" << event->x_root << " YRoot=" << event->y_root << "\n"
                            << "   Time=" << event->time << " SameScreen=" << event->same_screen << " SubWin=" << event->subwindow << " Win=" << event->window << "\n"
                            << "   Root=" << event->root << " KeyCode=" << event->keycode << " State=" << event->state << "\n"
                            << "  scimKeyEvent=(" << scimkey.code << "," << scimkey.mask << ")\n";

    m_panel_client.prepare (ic->icid);

    if (!filter_hotkeys (ic, scimkey)) {
        if (!ic->xims_on || !process_key_event (ic->siid, scimkey)) {
            if (!m_fallback_instance->process_key_event (scimkey))
                IMForwardEvent (ims, (XPointer) call_data);
        }
    }

    m_panel_client.send ();

    return 1;
}

int
X11FrontEnd::ims_sync_reply_handler (XIMS ims, IMSyncXlibStruct *call_data)
{
    SCIM_DEBUG_FRONTEND(2) << " IMS Sync reply handler.\n";
    return 1;
}

int
X11FrontEnd::ims_preedit_start_reply_handler (XIMS ims, IMPreeditCBStruct *call_data)
{
    SCIM_DEBUG_FRONTEND(2) << " IMS Preedit start reply handler.\n";
    return 1;
}

int
X11FrontEnd::ims_preedit_caret_reply_handler (XIMS ims, IMPreeditCBStruct *call_data)
{
    SCIM_DEBUG_FRONTEND(2) << " IMS Preedit caret reply handler.\n";
    return 1;
}

void
X11FrontEnd::ims_commit_string (const X11IC *ic, const WideString& str)
{
    IMCommitStruct cms;
    XTextProperty tp;

    SCIM_DEBUG_FRONTEND(2) << " IMS Committing string.\n";

    if (ims_wcstocts (tp, ic, str)) {
        memset (&cms, 0, sizeof (cms));
        cms.major_code = XIM_COMMIT;
        cms.icid = ic->icid;
        cms.connect_id = ic->connect_id;
        cms.flag = XimLookupChars;
        cms.commit_string = (char *) tp.value;
        IMCommitString (m_xims, (XPointer) & cms);
        XFree (tp.value);
    }
}

void
X11FrontEnd::ims_forward_key_event (const X11IC *ic, const KeyEvent &key)
{
    IMForwardEventStruct fe;
    XEvent xkp;

    XKeyEvent *event = (XKeyEvent*) (&xkp);

    //create event
    xkp.xkey = scim_x11_keyevent_scim_to_x11 (m_display, key);

    memset (&fe, 0, sizeof (fe));
    fe.major_code = XIM_FORWARD_EVENT;
    fe.icid = ic->icid;
    fe.connect_id = ic->connect_id;
    fe.sync_bit = 0;
    fe.serial_number = 0L;

    if (ic->focus_win)
        xkp.xkey.window = ic->focus_win;
    else if (ic->client_win)
        xkp.xkey.window = ic->client_win;

    memcpy (&(fe.event), &xkp, sizeof (fe.event));
    IMForwardEvent (m_xims, (XPointer) & fe);
}

bool
X11FrontEnd::ims_wcstocts (XTextProperty &tp,const X11IC *ic, const WideString& src)
{
    if (!validate_ic (ic)) return false;

    String last = String (setlocale (LC_CTYPE, 0));

    if (!setlocale (LC_CTYPE, ic->locale.c_str ())) {
        SCIM_DEBUG_FRONTEND(3) << "  wcstocts -- unspported locale " << ic->locale.c_str () << "\n";

        setlocale (LC_CTYPE, last.c_str ());
        return false;
    }

    int ret;

    if (m_wchar_ucs4_equal && !m_broken_wchar) {
        wchar_t *wclist [1];

        SCIM_DEBUG_FRONTEND(3) << "  Convert WideString to COMPOUND_TEXT -- Using XwcTextListToTextProperty.\n";

        wclist [0] = new wchar_t [src.length () + 1];
        memcpy (wclist [0], src.data (), sizeof (wchar_t) * src.length ());
        wclist [0][src.length ()] = 0;
        ret = XwcTextListToTextProperty (m_display, wclist, 1, XCompoundTextStyle, &tp);
        delete [] wclist [0];
    } else {
        char *clist [1];
        String mbs;

        SCIM_DEBUG_FRONTEND(3) << "  Convert WideString to COMPOUND_TEXT -- Using XmbTextListToTextProperty.\n";

        if (!m_iconv.set_encoding (ic->encoding)) {
            SCIM_DEBUG_FRONTEND(3) << "  Convert WideString to COMPOUND_TEXT -- Cannot initialize iconv for encoding "
                      << ic->encoding << "\n";

            setlocale (LC_CTYPE, last.c_str ());
            return false;
        }

        m_iconv.convert (mbs, src);
        clist [0] = (char *) mbs.c_str ();
        ret = XmbTextListToTextProperty (m_display, clist, 1, XCompoundTextStyle, &tp);
    }

    setlocale (LC_CTYPE, last.c_str ());
    return ret >= 0;
}

bool
X11FrontEnd::ims_is_preedit_callback_mode (const X11IC *ic)
{
    return validate_ic (ic) && (ic->input_style & XIMPreeditCallbacks);
}

void
X11FrontEnd::ims_preedit_callback_start (X11IC *ic)
{
    if (!validate_ic (ic) || ic->onspot_preedit_started) return;

    ic->onspot_preedit_started = true;

    SCIM_DEBUG_FRONTEND(2) << " Onspot preedit start, ICID="
            << ic->icid << " Connect ID=" << ic->connect_id << "\n";

    IMPreeditCBStruct pcb;

    pcb.major_code        = XIM_PREEDIT_START;
    pcb.minor_code        = 0;
    pcb.connect_id        = ic->connect_id;
    pcb.icid              = ic->icid;
    pcb.todo.return_value = 0;
    IMCallCallback (m_xims, (XPointer) & pcb);
}

void
X11FrontEnd::ims_preedit_callback_done (X11IC *ic)
{
    if (!validate_ic (ic) || !ic->onspot_preedit_started) return;

    SCIM_DEBUG_FRONTEND(2) << " Onspot preedit done, ICID="
            << ic->icid << " Connect ID=" << ic->connect_id << "\n";

    // First clear the preedit string.
    ims_preedit_callback_draw (ic, WideString ());

    ic->onspot_preedit_started = false;

    IMPreeditCBStruct pcb;

    pcb.major_code        = XIM_PREEDIT_DONE;
    pcb.minor_code        = 0;
    pcb.connect_id        = ic->connect_id;
    pcb.icid              = ic->icid;
    pcb.todo.return_value = 0;
    IMCallCallback (m_xims, (XPointer) & pcb);
}

void
X11FrontEnd::ims_preedit_callback_draw (X11IC *ic, const WideString& str, const AttributeList & attrs)
{
    if (!validate_ic (ic)) return;

    if (!ic->onspot_preedit_started)
        ims_preedit_callback_start (ic);

    SCIM_DEBUG_FRONTEND(2) << " Onspot preedit draw, ICID="
            << ic->icid << " Connect ID=" << ic->connect_id << "\n";

    IMPreeditCBStruct pcb;
    XIMText text;
    XIMFeedback *feedback;
    XIMFeedback attr;
    XTextProperty tp;

    unsigned int i, j, len;

    len = str.length ();
    if (!len && !ic->onspot_preedit_length)
        return;

    feedback = new XIMFeedback [str.length () + 1];

    for (i = 0; i < len; ++i)
        feedback [i] = 0;

    for (i = 0; i < attrs.size (); ++i) {
        attr = 0;
        if (attrs [i].get_type () == SCIM_ATTR_DECORATE) {
            if (attrs [i].get_value () == SCIM_ATTR_DECORATE_REVERSE)
                attr = XIMReverse;
            else if (attrs [i].get_value () == SCIM_ATTR_DECORATE_HIGHLIGHT)
                attr = XIMHighlight;
        }
        for (j = attrs [i].get_start (); j < attrs [i].get_end () && j < len; ++j)
            feedback [j] |= attr;
    }

    for (i = 0; i < len; ++i)
        if (!feedback [i])
            feedback [i] = XIMUnderline;

    feedback [len] = 0;

    pcb.major_code = XIM_PREEDIT_DRAW;
    pcb.connect_id = ic->connect_id;
    pcb.icid = ic->icid;

    pcb.todo.draw.caret = len;
    pcb.todo.draw.chg_first = 0;
    pcb.todo.draw.chg_length = ic->onspot_preedit_length;
    pcb.todo.draw.text = &text;

    text.feedback = feedback;

    if (len > 0 && ims_wcstocts (tp, ic, str)) {
        text.encoding_is_wchar = false;
        text.length = strlen ((char*)tp.value);
        text.string.multi_byte = (char*)tp.value;
        IMCallCallback (m_xims, (XPointer) & pcb);
        XFree (tp.value);
    } else {
        text.encoding_is_wchar = false;
        text.length = 0;
        text.string.multi_byte = const_cast<char*> ("");
        IMCallCallback (m_xims, (XPointer) & pcb);
        len = 0;
    }

    ic->onspot_preedit_length = len;

    delete [] feedback;
}

void
X11FrontEnd::ims_preedit_callback_caret (X11IC *ic, int caret)
{
    if (!validate_ic (ic) || !ic->onspot_preedit_started || caret > ic->onspot_preedit_length || caret < 0)
        return;

    SCIM_DEBUG_FRONTEND(2) << " Onspot preedit caret, ICID="
            << ic->icid << " Connect ID=" << ic->connect_id << "\n";

    //save the caret position for future usage when updating preedit string.
    ic->onspot_caret = caret;

    //client usually does not support this callback
    IMPreeditCBStruct pcb;

    pcb.major_code = XIM_PREEDIT_CARET;
    pcb.connect_id = ic->connect_id;
    pcb.icid       = ic->icid;
    pcb.todo.caret.direction = XIMAbsolutePosition;
    pcb.todo.caret.position = caret;
    pcb.todo.caret.style = XIMIsPrimary;
    IMCallCallback (m_xims, (XPointer) & pcb);
}

bool
X11FrontEnd::ims_string_conversion_callback_retrieval (X11IC *ic, WideString &text, int &cursor, int maxlen_before, int maxlen_after)
{
#if 0
    if (!validate_ic (ic) || (maxlen_before == 0 && maxlen_after == 0))
        return false;

    SCIM_DEBUG_FRONTEND(2) << " String conversion callback retrieval, ICID="
            << ic->icid << " Connect ID=" << ic->connect_id << "\n";

    //client usually does not support this callback
    IMStrConvCBStruct sccb;

    sccb.major_code        = XIM_STR_CONVERSION;
    sccb.connect_id        = ic->connect_id;
    sccb.icid              = ic->icid;
    sccb.strconv.text      = 0;

    sccb.strconv.operation = XIMStringConversionRetrieval;
    sccb.strconv.position  = 0;

    sccb.strconv.direction = XIMBackwardChar;
    sccb.strconv.factor    = (short)((maxlen_before > 0) ? maxlen_before : 1);

    IMCallCallback (m_xims, (XPointer) & sccb);

    std::cerr << "Surrounding text: ";

    if (sccb.strconv.text && sccb.strconv.text->string.mbs) {
        std::cerr << sccb.strconv.text->string.mbs << " ";
    }

    sccb.strconv.direction = XIMForwardChar;
    sccb.strconv.factor    = (maxlen_after > 0) ? maxlen_after : 1;

    IMCallCallback (m_xims, (XPointer) & sccb);

    if (sccb.strconv.text && sccb.strconv.text->string.mbs) {
        std::cerr << sccb.strconv.text->string.mbs;
    }

    std::cerr << "\n";
#endif
    return false;
}

bool
X11FrontEnd::ims_string_conversion_callback_substitution (X11IC *ic, int offset, int len)
{
#if 0
    if (!validate_ic (ic) || len <= 0)
        return false;

    SCIM_DEBUG_FRONTEND(2) << " String conversion callback substitution, ICID="
            << ic->icid << " Connect ID=" << ic->connect_id << "\n";

    //client usually does not support this callback
    IMStrConvCBStruct sccb;

    sccb.major_code        = XIM_STR_CONVERSION;
    sccb.connect_id        = ic->connect_id;
    sccb.icid              = ic->icid;
    sccb.strconv.text      = 0;
    sccb.strconv.operation = XIMStringConversionSubstitution;
    sccb.strconv.position  = (XIMStringConversionPosition) offset;
    sccb.strconv.direction = XIMForwardChar;
    sccb.strconv.factor    = (short) len;

    IMCallCallback (m_xims, (XPointer) & sccb);

    return sccb.strconv.text != NULL;
#endif
    return false;
}

void
X11FrontEnd::ims_turn_on_ic (X11IC *ic)
{
    if (validate_ic (ic) && !ic->xims_on) {
        SCIM_DEBUG_FRONTEND(2) << "ims_turn_on_ic.\n";

        ic->xims_on = true;

        //Record the IC on/off status
        if (m_shared_input_method)
            m_config->write (String (SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), true);

        if (is_focused_ic (ic)) {
            panel_req_focus_in (ic);
            start_ic (ic);
        }
    }
}

void
X11FrontEnd::ims_turn_off_ic (X11IC *ic)
{
    if (validate_ic (ic) && ic->xims_on) {
        SCIM_DEBUG_FRONTEND(2) << "ims_turn_off_ic.\n";

        ic->xims_on = false;

        //Record the IC on/off status
        if (m_shared_input_method)
            m_config->write (String (SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), false);

        if (is_focused_ic (ic))
            stop_ic (ic);
    }
}

void
X11FrontEnd::ims_sync_ic (X11IC *ic)
{
    if (validate_ic (ic)) {
        IMSyncXlibStruct data;

        data.major_code = XIM_SYNC;
        data.minor_code = 0;
        data.connect_id = ic->connect_id;
        data.icid       = ic->icid;

        IMSyncXlib(m_xims, (XPointer) &data);
    }
}

void
X11FrontEnd::set_ic_capabilities (const X11IC *ic)
{
    if (validate_ic (ic)) {
        unsigned int cap = SCIM_CLIENT_CAP_ALL_CAPABILITIES - SCIM_CLIENT_CAP_SURROUNDING_TEXT;

        if (!ims_is_preedit_callback_mode (ic))
            cap -= SCIM_CLIENT_CAP_ONTHESPOT_PREEDIT;

        update_client_capabilities (ic->siid, cap);
    }
}

void
X11FrontEnd::start_ic (X11IC *ic)
{
    if (validate_ic (ic)) {
        if (m_xims_dynamic) {
            IMPreeditStateStruct ips;
            ips.major_code = 0;
            ips.minor_code = 0;
            ips.icid = ic->icid;
            ips.connect_id = ic->connect_id;
            IMPreeditStart (m_xims, (XPointer) & ips);
        }

        panel_req_update_screen (ic);
        panel_req_update_spot_location (ic);
        panel_req_update_factory_info (ic);

        m_panel_client.turn_on (ic->icid);
        m_panel_client.hide_preedit_string (ic->icid);
        m_panel_client.hide_aux_string (ic->icid);
        m_panel_client.hide_lookup_table (ic->icid);

        if (ic->shared_siid) reset (ic->siid);

        focus_in (ic->siid);
    }
}

void
X11FrontEnd::stop_ic (X11IC *ic)
{
    if (validate_ic (ic)) {
        focus_out (ic->siid);
        if (ic->shared_siid) reset (ic->siid);

        if (ims_is_preedit_callback_mode (ic))
            ims_preedit_callback_done (ic);

        panel_req_update_factory_info (ic);
        m_panel_client.turn_off (ic->icid);

        if (m_xims_dynamic) {
            IMPreeditStateStruct ips;
            ips.major_code = 0;
            ips.minor_code = 0;
            ips.icid = ic->icid;
            ips.connect_id = ic->connect_id;
            IMPreeditEnd (m_xims, (XPointer) & ips);
        }
    }
}

int
X11FrontEnd::ims_protocol_handler (XIMS ims, IMProtocol *call_data)
{
    if (!_scim_frontend || !call_data || ims != _scim_frontend->m_xims)
        return 0;

    switch (call_data->major_code) {
        case XIM_OPEN:
            return _scim_frontend->ims_open_handler (ims, (IMOpenStruct *) call_data);
        case XIM_CLOSE:
            return _scim_frontend->ims_close_handler (ims, (IMCloseStruct *) call_data);
        case XIM_CREATE_IC:
            return _scim_frontend->ims_create_ic_handler (ims, (IMChangeICStruct *) call_data);
        case XIM_DESTROY_IC:
            return _scim_frontend->ims_destroy_ic_handler (ims, (IMDestroyICStruct *) call_data);
        case XIM_SET_IC_VALUES:
            return _scim_frontend->ims_set_ic_values_handler (ims, (IMChangeICStruct *) call_data);
        case XIM_GET_IC_VALUES:
            return _scim_frontend->ims_get_ic_values_handler (ims, (IMChangeICStruct *) call_data);
        case XIM_FORWARD_EVENT:
            return _scim_frontend->ims_forward_event_handler (ims, (IMForwardEventStruct *) call_data);
        case XIM_SET_IC_FOCUS:
            return _scim_frontend->ims_set_ic_focus_handler (ims, (IMChangeFocusStruct *) call_data);
        case XIM_UNSET_IC_FOCUS:
            return _scim_frontend->ims_unset_ic_focus_handler (ims, (IMChangeFocusStruct *) call_data);
        case XIM_RESET_IC:
            return _scim_frontend->ims_reset_ic_handler (ims, (IMResetICStruct *) call_data);
        case XIM_TRIGGER_NOTIFY:
            return _scim_frontend->ims_trigger_notify_handler (ims, (IMTriggerNotifyStruct *) call_data);
        case XIM_PREEDIT_START_REPLY:
            return _scim_frontend->ims_preedit_start_reply_handler (ims, (IMPreeditCBStruct *) call_data);
        case XIM_PREEDIT_CARET_REPLY:
            return _scim_frontend->ims_preedit_caret_reply_handler (ims, (IMPreeditCBStruct *) call_data);
        case XIM_SYNC_REPLY:
            return _scim_frontend->ims_sync_reply_handler (ims, (IMSyncXlibStruct *) call_data);
        default:
            SCIM_DEBUG_FRONTEND(1) << "Unknown major code " << call_data->major_code << "\n";
            break;
    }
    return 1;
}

int
X11FrontEnd::x_error_handler (Display *display, XErrorEvent *error)
{
#if ENABLE_DEBUG
    char buf [256];

    XGetErrorText (display, error->error_code, buf, 256);

    SCIM_DEBUG_FRONTEND (1)
            << "X Error occurred:\n"
            << "  Display     = " << display << "\n"
            << "  Type        = " << error->type << "\n"
            << "  Resourceid  = " << error->resourceid << "\n"
            << "  Serial      = " << error->serial << "\n"
            << "  ErrorCode   = " << (uint32) error->error_code << "\n"
            << "  RequestCode = " << (uint32) error->request_code << "\n"
            << "  MinorCode   = " << (uint32) error->minor_code << "\n"
            << "  Error Text  = " << buf << "\n";
#endif

    // trap all possible error for broken focus ic.
    if ((error->error_code == BadWindow ||
         error->error_code == BadMatch) &&
        (error->request_code == X_GetWindowAttributes ||
         error->request_code == X_GetProperty ||
         error->request_code == X_SendEvent ||
         error->request_code == X_TranslateCoords)) {
        SCIM_DEBUG_FRONTEND(1) << "Discard This Error\n";
    } else if (_scim_frontend && _scim_frontend->m_old_x_error_handler) {
        _scim_frontend->m_old_x_error_handler (display, error);
    }

    return 0;
}


//===================== Panel Slot callbacks =======================
void
X11FrontEnd::panel_slot_reload_config (int context)
{
    m_config->reload ();
}

void
X11FrontEnd::panel_slot_exit (int context)
{
    m_should_exit = true;
}

void
X11FrontEnd::panel_slot_update_lookup_table_page_size (int context, int page_size)
{
    X11IC *ic = m_ic_manager.find_ic (context);
    if (validate_ic (ic)) {
        m_panel_client.prepare (ic->icid);
        update_lookup_table_page_size (ic->siid, page_size);
        m_panel_client.send ();
    }
}
void
X11FrontEnd::panel_slot_lookup_table_page_up (int context)
{
    X11IC *ic = m_ic_manager.find_ic (context);
    if (validate_ic (ic)) {
        m_panel_client.prepare (ic->icid);
        lookup_table_page_up (ic->siid);
        m_panel_client.send ();
    }
}
void
X11FrontEnd::panel_slot_lookup_table_page_down (int context)
{
    X11IC *ic = m_ic_manager.find_ic (context);
    if (validate_ic (ic)) {
        m_panel_client.prepare (ic->icid);
        lookup_table_page_down (ic->siid);
        m_panel_client.send ();
    }
}
void
X11FrontEnd::panel_slot_trigger_property (int context, const String &property)
{
    X11IC *ic = m_ic_manager.find_ic (context);
    if (validate_ic (ic)) {
        m_panel_client.prepare (ic->icid);
        trigger_property (ic->siid, property);
        m_panel_client.send ();
    }
}
void
X11FrontEnd::panel_slot_process_helper_event (int context, const String &target_uuid, const String &helper_uuid, const Transaction &trans)
{
    X11IC *ic = m_ic_manager.find_ic (context);
    if (validate_ic (ic) && get_instance_uuid (ic->siid) == target_uuid) {
        m_panel_client.prepare (ic->icid);
        process_helper_event (ic->siid, helper_uuid, trans);
        m_panel_client.send ();
    }
}
void
X11FrontEnd::panel_slot_move_preedit_caret (int context, int caret_pos)
{
    X11IC *ic = m_ic_manager.find_ic (context);
    if (validate_ic (ic)) {
        m_panel_client.prepare (ic->icid);
        move_preedit_caret (ic->siid, caret_pos);
        m_panel_client.send ();
    }
}
void
X11FrontEnd::panel_slot_select_candidate (int context, int cand_index)
{
    X11IC *ic = m_ic_manager.find_ic (context);
    if (validate_ic (ic)) {
        m_panel_client.prepare (ic->icid);
        select_candidate (ic->siid, cand_index);
        m_panel_client.send ();
    }
}
void
X11FrontEnd::panel_slot_process_key_event (int context, const KeyEvent &key)
{
    X11IC *ic = m_ic_manager.find_ic (context);
    if (validate_ic (ic)) {
        m_panel_client.prepare (ic->icid);

        if (!filter_hotkeys (ic, key)) {
            if (!ic->xims_on || !process_key_event (ic->siid, key)) {
                if (!m_fallback_instance->process_key_event (key))
                    ims_forward_key_event (ic, key);
            }
        }

        m_panel_client.send ();
    }
}
void
X11FrontEnd::panel_slot_commit_string (int context, const WideString &wstr)
{
    X11IC *ic = m_ic_manager.find_ic (context);
    if (validate_ic (ic)) {
        ims_commit_string (ic, wstr);
    }
}
void
X11FrontEnd::panel_slot_forward_key_event (int context, const KeyEvent &key)
{
    X11IC *ic = m_ic_manager.find_ic (context);
    if (validate_ic (ic)) {
        ims_forward_key_event (ic, key);
    }
}
void
X11FrontEnd::panel_slot_request_help (int context)
{
    X11IC *ic = m_ic_manager.find_ic (context);
    if (validate_ic (ic)) {
        m_panel_client.prepare (ic->icid);
        panel_req_show_help (ic);
        m_panel_client.send ();
    }
}
void
X11FrontEnd::panel_slot_request_factory_menu (int context)
{
    X11IC *ic = m_ic_manager.find_ic (context);
    if (validate_ic (ic)) {
        m_panel_client.prepare (ic->icid);
        panel_req_show_factory_menu (ic);
        m_panel_client.send ();
    }
}
void
X11FrontEnd::panel_slot_change_factory (int context, const String &uuid)
{
    SCIM_DEBUG_FRONTEND (1) << "panel_slot_change_factory " << uuid << "\n";

    X11IC *ic = m_ic_manager.find_ic (context);
    if (validate_ic (ic)) {
        m_panel_client.prepare (ic->icid);
        if (uuid.length () == 0 && ic->xims_on) {
            SCIM_DEBUG_FRONTEND (2) << "panel_slot_change_factory : turn off.\n";
            ims_turn_off_ic (ic);
        }else if(uuid.length () == 0 && (ic->xims_on == false)){
    		panel_req_update_factory_info (ic);
        	m_panel_client.turn_off (ic->icid);        	
        }else if (uuid.length ()) {
            String encoding = scim_get_locale_encoding (ic->locale);
            String language = scim_get_locale_language (ic->locale);
            if (validate_factory (uuid, encoding)) {
                ims_turn_off_ic (ic);
                replace_instance (ic->siid, uuid);
                m_panel_client.register_input_context (ic->icid, get_instance_uuid (ic->siid));
                set_ic_capabilities (ic);
                set_default_factory (language, uuid);
                ims_turn_on_ic (ic);
            }
        }
        m_panel_client.send ();
    }
}

//================ Panel Request methods ====================
void
X11FrontEnd::panel_req_update_screen (const X11IC *ic)
{
    Window target = ic->focus_win ? ic->focus_win : ic->client_win;
    XWindowAttributes xwa;
    if (target && 
        XGetWindowAttributes (m_display, target, &xwa) &&
        validate_ic (ic)) {
        int num = ScreenCount (m_display);
        int idx;
        for (idx = 0; idx < num; ++ idx) {
            if (ScreenOfDisplay (m_display, idx) == xwa.screen) {
                m_panel_client.update_screen (ic->icid, idx);
                return;
            }
        }
    }
}

void
X11FrontEnd::panel_req_show_help (const X11IC *ic)
{
    String help;
    String tmp;

    help =  String (_("Smart Common Input Method platform ")) +
            String (SCIM_VERSION) +
            String (_("\n(C) 2002-2005 James Su <suzhe@tsinghua.org.cn>\n\n"));

    if (ic->xims_on) {
        help += utf8_wcstombs (get_instance_name (ic->siid));
        help += String (_(":\n\n"));

        help += utf8_wcstombs (get_instance_authors (ic->siid));
        help += String (_("\n\n"));

        help += utf8_wcstombs (get_instance_help (ic->siid));
        help += String (_("\n\n"));

        help += utf8_wcstombs (get_instance_credits (ic->siid));
    }

    m_panel_client.show_help (ic->icid, help);
}

void
X11FrontEnd::panel_req_show_factory_menu (const X11IC *ic)
{
    std::vector<String> uuids;
    if (get_factory_list_for_encoding (uuids, ic->encoding)) {
        std::vector <PanelFactoryInfo> menu;
        for (size_t i = 0; i < uuids.size (); ++ i) {
            menu.push_back (PanelFactoryInfo (
                                    uuids [i],
                                    utf8_wcstombs (get_factory_name (uuids [i])),
                                    get_factory_language (uuids [i]),
                                    get_factory_icon_file (uuids [i])));
        }
        m_panel_client.show_factory_menu (ic->icid, menu);
    }
}

void
X11FrontEnd::panel_req_focus_in (const X11IC * ic)
{
    m_panel_client.focus_in (ic->icid, get_instance_uuid (ic->siid));
}

void
X11FrontEnd::panel_req_update_factory_info (const X11IC *ic)
{
    if (is_focused_ic (ic)) {
        PanelFactoryInfo info;
        if (ic->xims_on) {
            String uuid = get_instance_uuid (ic->siid);
            info = PanelFactoryInfo (uuid, utf8_wcstombs (get_factory_name (uuid)), get_factory_language (uuid), get_factory_icon_file (uuid));
        } else {
            info = PanelFactoryInfo (String (""), String (_("English/Keyboard")), String ("C"), String (SCIM_KEYBOARD_ICON_FILE));
        }
        m_panel_client.update_factory_info (ic->icid, info);
    }
}

void
X11FrontEnd::panel_req_update_spot_location (const X11IC *ic)
{
    Window target = ic->focus_win ? ic->focus_win : ic->client_win;
    XWindowAttributes xwa;

    if (target && 
        XGetWindowAttributes (m_display, target, &xwa) &&
        validate_ic (ic)) {

        int spot_x, spot_y;
        Window child;

        if (m_focus_ic->pre_attr.spot_location.x >= 0 &&
            m_focus_ic->pre_attr.spot_location.y >= 0) {
            XTranslateCoordinates (m_display, target,
                xwa.root,
                m_focus_ic->pre_attr.spot_location.x + 8,
                m_focus_ic->pre_attr.spot_location.y + 8,
                &spot_x, &spot_y, &child);
        } else {
            XTranslateCoordinates (m_display, target,
                xwa.root,
                0,
                xwa.height,
                &spot_x, &spot_y, &child);
        }
        m_panel_client.update_spot_location (ic->icid, spot_x, spot_y);
    }
}

void
X11FrontEnd::reload_config_callback (const ConfigPointer &config)
{
    SCIM_DEBUG_FRONTEND(1) << "Reload configuration.\n";

    m_frontend_hotkey_matcher.load_hotkeys (config);
    m_imengine_hotkey_matcher.load_hotkeys (config);

    KeyEvent key;

    scim_string_to_key (key,
        config->read (String (SCIM_CONFIG_HOTKEYS_FRONTEND_VALID_KEY_MASK),
                      String ("Shift+Control+Alt+Lock")));

    m_valid_key_mask = (key.mask > 0) ? key.mask : 0xFFFF;
    m_valid_key_mask |= SCIM_KEY_ReleaseMask;
    // Special treatment for two backslash keys on jp106 keyboard.
    m_valid_key_mask |= SCIM_KEY_QuirkKanaRoMask;

    m_broken_wchar =
        config->read (String (SCIM_CONFIG_FRONTEND_X11_BROKEN_WCHAR),
                      m_broken_wchar);

    m_shared_input_method =
        config->read (String (SCIM_CONFIG_FRONTEND_SHARED_INPUT_METHOD),
                      m_shared_input_method);

    // Get keyboard layout setting
    // Flush the global config first, in order to load the new configs from disk.
    scim_global_config_flush ();

    m_keyboard_layout = scim_get_default_keyboard_layout ();
}

void
X11FrontEnd::fallback_commit_string_cb (IMEngineInstanceBase * si, const WideString & str)
{
    if (validate_ic (m_focus_ic))
        ims_commit_string (m_focus_ic, str);
}

/*
vi:ts=4:nowrap:expandtab
*/
