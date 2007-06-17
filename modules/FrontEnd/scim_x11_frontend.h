/**
 * @file scim_x11_frontend.h
 * @brief definition of X11FrontEnd related classes.
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
 * $Id: scim_x11_frontend.h,v 1.56 2005/06/26 16:35:12 suzhe Exp $
 */

#if !defined (__SCIM_X11_FRONTEND_H)
#define __SCIM_X11_FRONTEND_H

#include "scim_stl_map.h"

using namespace scim;

class X11FrontEnd : public FrontEndBase
{
// first = UUID.
// second= siid.
#if SCIM_USE_STL_EXT_HASH_MAP
    typedef __gnu_cxx::hash_map <String, int, scim_hash_string> DefaultInstanceMap;
#elif SCIM_USE_STL_HASH_MAP
    typedef std::hash_map <String, int, scim_hash_string>       DefaultInstanceMap;
#else
    typedef std::map <String, int>                              DefaultInstanceMap;
#endif

    X11ICManager            m_ic_manager;
    XIMS                    m_xims;
    Display                *m_display;
    Window                  m_xims_window;
    String                  m_server_name;
    String                  m_display_name;

    PanelClient             m_panel_client;

    X11IC                  *m_focus_ic;

    FrontEndHotkeyMatcher   m_frontend_hotkey_matcher;
    IMEngineHotkeyMatcher   m_imengine_hotkey_matcher;

    bool                    m_xims_dynamic;
    bool                    m_wchar_ucs4_equal;
    bool                    m_broken_wchar;
    bool                    m_shared_input_method;

    KeyboardLayout          m_keyboard_layout;

    int                     m_valid_key_mask;

    bool                    m_should_exit;

    IConvert                m_iconv;
    ConfigPointer           m_config;

    IMEngineFactoryPointer  m_fallback_factory;
    IMEngineInstancePointer m_fallback_instance;

    DefaultInstanceMap      m_default_instance_map;

    int (*m_old_x_error_handler) (Display *, XErrorEvent *);

public:
    X11FrontEnd (const BackEndPointer &backend,
                 const ConfigPointer &config,
                 const String &server_name = String ("SCIM"));

    virtual ~X11FrontEnd ();

protected:
    virtual void show_preedit_string     (int siid);
    virtual void show_aux_string         (int siid);
    virtual void show_lookup_table       (int siid);

    virtual void hide_preedit_string     (int siid);
    virtual void hide_aux_string         (int siid);
    virtual void hide_lookup_table       (int siid);

    virtual void update_preedit_caret    (int siid, int caret);
    virtual void update_preedit_string   (int siid, const WideString & str, const AttributeList & attrs = AttributeList ());
    virtual void update_aux_string       (int siid, const WideString & str, const AttributeList & attrs = AttributeList ());
    virtual void commit_string           (int siid, const WideString & str);
    virtual void forward_key_event       (int siid, const KeyEvent & key);
    virtual void update_lookup_table     (int siid, const LookupTable & table);

    virtual void register_properties     (int siid, const PropertyList & properties);
    virtual void update_property         (int siid, const Property     & property);
    virtual void beep                    (int siid);
    virtual void start_helper            (int siid, const String &helper_uuid);
    virtual void stop_helper             (int siid, const String &helper_uuid);
    virtual void send_helper_event       (int siid, const String &helper_uuid, const Transaction &trans);

    virtual bool get_surrounding_text    (int siid, WideString &text, int &cursor, int maxlen_before, int maxlen_after);
    virtual bool delete_surrounding_text (int siid, int offset, int len);

public:
    virtual void init (int argc, char **argv);
    virtual void run ();

private:
    String get_supported_locales (void);

    int get_default_instance (const String &language, const String &encoding);

    // Return the display name
    String init_ims (void);

    XKeyEvent keyevent_scim_to_x11 (const KeyEvent& key);
    KeyEvent keyevent_x11_to_scim (const XKeyEvent& key);

    bool filter_hotkeys                 (X11IC *ic, const KeyEvent &key);

    int ims_open_handler                (XIMS ims, IMOpenStruct *call_data);
    int ims_close_handler               (XIMS ims, IMCloseStruct *call_data);
    int ims_create_ic_handler           (XIMS ims, IMChangeICStruct *call_data);
    int ims_set_ic_values_handler       (XIMS ims, IMChangeICStruct *call_data);
    int ims_get_ic_values_handler       (XIMS ims, IMChangeICStruct *call_data);
    int ims_destroy_ic_handler          (XIMS ims, IMDestroyICStruct *call_data);
    int ims_set_ic_focus_handler        (XIMS ims, IMChangeFocusStruct *call_data);
    int ims_unset_ic_focus_handler      (XIMS ims, IMChangeFocusStruct *call_data);
    int ims_reset_ic_handler            (XIMS ims, IMResetICStruct *call_data);
    int ims_trigger_notify_handler      (XIMS ims, IMTriggerNotifyStruct *call_data);
    int ims_forward_event_handler       (XIMS ims, IMForwardEventStruct *call_data);
    int ims_sync_reply_handler          (XIMS ims, IMSyncXlibStruct *call_data);
    int ims_preedit_start_reply_handler (XIMS ims, IMPreeditCBStruct *call_data);
    int ims_preedit_caret_reply_handler (XIMS ims, IMPreeditCBStruct *call_data);

    void ims_commit_string (const X11IC *ic, const WideString& str);
    void ims_forward_key_event (const X11IC *ic, const KeyEvent& key);
    bool ims_wcstocts (XTextProperty &tp, const X11IC *ic, const WideString& src);

    bool ims_is_preedit_callback_mode (const X11IC *ic);
    void ims_preedit_callback_start (X11IC *ic);
    void ims_preedit_callback_done (X11IC *ic);
    void ims_preedit_callback_draw (X11IC *ic, const WideString& str, const AttributeList & attrs = AttributeList ());
    void ims_preedit_callback_caret (X11IC *ic, int caret);

    bool ims_string_conversion_callback_retrieval (X11IC *ic, WideString &text, int &cursor, int maxlen_before, int maxlen_after); 
    bool ims_string_conversion_callback_substitution (X11IC *ic, int offset, int len); 

    void ims_turn_on_ic (X11IC *ic);
    void ims_turn_off_ic (X11IC *ic);

    void ims_sync_ic (X11IC *ic);

    void set_ic_capabilities (const X11IC *ic);

    void start_ic (X11IC *ic);
    void stop_ic (X11IC *ic);

    bool is_focused_ic    (int siid) { return validate_ic (m_focus_ic) && m_focus_ic->siid == siid; }
    bool is_inputing_ic   (int siid) { return is_focused_ic (siid) && m_focus_ic->xims_on; }
    bool is_forwarding_ic (int siid) { return is_focused_ic (siid) && !m_focus_ic->xims_on; }

    bool is_focused_ic    (const X11IC *ic) { return validate_ic (m_focus_ic) && validate_ic (ic) && m_focus_ic->icid == ic->icid; }
    bool is_inputing_ic   (const X11IC *ic) { return is_focused_ic (ic) && ic->xims_on; }
    bool is_forwarding_ic (const X11IC *ic) { return is_focused_ic (ic) && !ic->xims_on; }

    bool is_input_mode () {
        return validate_ic (m_focus_ic) && m_focus_ic->xims_on;
    }

    bool is_forward_mode () {
        return !is_input_mode ();
    }

    void panel_slot_reload_config (int context);
    void panel_slot_exit          (int context);
    void panel_slot_update_lookup_table_page_size (int context, int page_size);
    void panel_slot_lookup_table_page_up (int context);
    void panel_slot_lookup_table_page_down (int context);
    void panel_slot_trigger_property (int context, const String &property);
    void panel_slot_process_helper_event (int context, const String &target_uuid, const String &helper_uuid, const Transaction &trans);
    void panel_slot_move_preedit_caret (int context, int caret_pos);
    void panel_slot_select_candidate (int context, int cand_index);
    void panel_slot_process_key_event (int context, const KeyEvent &key);
    void panel_slot_commit_string (int context, const WideString &wstr);
    void panel_slot_forward_key_event (int context, const KeyEvent &key);
    void panel_slot_request_help (int context);
    void panel_slot_request_factory_menu (int context);
    void panel_slot_change_factory (int context, const String &uuid);

    void panel_req_update_screen (const X11IC *ic);
    void panel_req_show_help (const X11IC *ic);
    void panel_req_show_factory_menu (const X11IC *ic);
    void panel_req_focus_in (const X11IC *ic);
    void panel_req_update_factory_info (const X11IC *ic);
    void panel_req_update_spot_location (const X11IC *ic);

    void reload_config_callback (const ConfigPointer &config);

    void fallback_commit_string_cb (IMEngineInstanceBase * si, const WideString & str);

private:
    static int ims_protocol_handler (XIMS ims, IMProtocol *call_data);

    static int  x_error_handler (Display *display, XErrorEvent *error);

    static bool validate_ic (const X11IC * ic) { return ic && ic->icid > 0 && ic->siid >= 0; }
};

#endif

/*
vi:ts=4:nowrap:ai:expandtab
*/
