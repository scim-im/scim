/**
 * @file scim_ibus_frontend.h
 * @brief definition of IBusFrontEnd related classes.
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
 * $Id: scim_ibus_frontend.h,v 1.26 2020/04/29 17:01:56 derekdai Exp $
 */

#if !defined (__SCIM_IBUS_FRONTEND_H)
#define __SCIM_IBUS_FRONTEND_H

#include "scim_stl_map.h"

using namespace scim;

class IBusCtx;

class IBusFrontEnd : public FrontEndBase
{
    friend class IBusCtx;

    typedef std::map <int, IBusCtx*>                                        IBusIDCtxMap;
    typedef std::set <IBusCtx*>                                             IBusCtxSet;
    typedef std::map <std::string, IBusCtxSet>                              IBusNameCtxMap;

    ConfigPointer           m_config;

    FrontEndHotkeyMatcher   m_frontend_hotkey_matcher;
    IMEngineHotkeyMatcher   m_imengine_hotkey_matcher;

    // siid
    int                     m_current_instance;

    IBusCtx                *m_current_ibus_ctx;

    PanelClient             m_panel_client;

    int                     m_ctx_counter;

    IBusIDCtxMap            m_id_ctx_map;
    IBusIDCtxMap            m_siid_ctx_map;
    IBusNameCtxMap          m_name_ctx_map;

    sd_event               *m_loop; 
    sd_bus                 *m_bus; 
    sd_bus_slot            *m_portal_slot;
    sd_bus_slot            *m_match_slot;
    sd_event_source        *m_panel_source;

    static const sd_bus_vtable  m_portal_vtbl[];
    static const sd_bus_vtable  m_inputcontext_vtbl[];
    static const sd_bus_vtable  m_service_vtbl[];

public:
    IBusFrontEnd (const BackEndPointer &backend,
                    const ConfigPointer  &config);

    virtual ~IBusFrontEnd ();

protected:
    /**
     * vitual functions to notify client something happened,
     * see scim_frontend.cpp for explaination
     */
    virtual void show_preedit_string     (int siid);
    virtual void show_aux_string         (int siid);
    virtual void show_lookup_table       (int siid);

    virtual void hide_preedit_string     (int siid);
    virtual void hide_aux_string         (int siid);
    virtual void hide_lookup_table       (int siid);

    virtual void update_preedit_caret    (int siid, int caret);
    virtual void update_preedit_string   (int siid, const WideString & str, const AttributeList & attrs);
    virtual void update_aux_string       (int siid, const WideString & str, const AttributeList & attrs);
    virtual void commit_string           (int siid, const WideString & str);
    virtual void forward_key_event       (int siid, const KeyEvent & key);
    virtual void update_lookup_table     (int siid, const LookupTable & table);

    virtual void register_properties     (int siid, const PropertyList & properties);
    virtual void update_property         (int siid, const Property & property);

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
    int generate_ctx_id ();
    bool is_current_ctx                   (const IBusCtx *ctx) const { return ctx == m_current_ibus_ctx; }

    bool check_client_connection          (const Socket &client) const;

    void reload_config_callback (const ConfigPointer &config);

    // IBus callbacks
    int portal_create_ctx                 (sd_bus_message *m);
    int srv_destroy                       (IBusCtx *ctx, sd_bus_message *m);
    int ctx_focus_in                      (IBusCtx *ctx, sd_bus_message *m);
    int ctx_focus_out                     (IBusCtx *ctx, sd_bus_message *m);
    int ctx_reset                         (IBusCtx *ctx, sd_bus_message *m);
    int ctx_process_key_event             (IBusCtx *ctx, sd_bus_message *m);
    int ctx_set_cursor_location           (IBusCtx *ctx, sd_bus_message *m);
    int ctx_set_cursor_location_relative  (IBusCtx *ctx, sd_bus_message *m);
    int ctx_process_hand_writing_event    (IBusCtx *ctx, sd_bus_message *m);
    int ctx_cancel_hand_writing           (IBusCtx *ctx, sd_bus_message *m);
    int ctx_property_activate             (IBusCtx *ctx, sd_bus_message *m);
    int ctx_set_engine                    (IBusCtx *ctx, sd_bus_message *m);
    int ctx_get_engine                    (IBusCtx *ctx, sd_bus_message *m);
    int ctx_set_surrounding_text          (IBusCtx *ctx, sd_bus_message *m);
    int ctx_set_capabilities              (IBusCtx *ctx, sd_bus_message *m);
    int ctx_get_client_commit_preedit     (IBusCtx *ctx, sd_bus_message *value);
    int ctx_set_client_commit_preedit     (IBusCtx *ctx, sd_bus_message *value);
    int ctx_get_content_type              (IBusCtx *ctx, sd_bus_message *value);
    int ctx_set_content_type              (IBusCtx *ctx, sd_bus_message *value);

    int client_destroy                    (sd_bus_message *m);

    IBusCtx *find_ctx_by_siid             (int siid) const;
    IBusCtx *find_ctx                     (int id) const;
    IBusCtx *find_ctx                     (const char *path) const;

    inline bool validate_ctx              (IBusCtx *ctx) const;

    void start_ctx                        (IBusCtx *ctx);
    void stop_ctx                         (IBusCtx *ctx);
    void signal_ctx                       (int siid,
                                           const char *signal,
                                           ...) const;

    void destroy_ctx                      (IBusCtx *ctx);

    int panel_connect                     ();
    void panel_disconnect                 ();
    int panel_handle_io                   (sd_event_source *s,
                                           int fd,
                                           uint32_t revents);

    void panel_slot_reload_config (int client_id);
    void panel_slot_exit          (int client_id);
    void panel_slot_update_lookup_table_page_size (int client_id, int page_size);
    void panel_slot_lookup_table_page_up (int client_id);
    void panel_slot_lookup_table_page_down (int client_id);
    void panel_slot_trigger_property (int client_id, const String &property);
    void panel_slot_process_helper_event (int client_id, const String &target_uuid, const String &helper_uuid, const Transaction &trans);
    void panel_slot_move_preedit_caret (int client_id, int caret_pos);
    void panel_slot_select_candidate (int client_id, int cand_index);
    void panel_slot_process_key_event (int client_id, const KeyEvent &key);
    void panel_slot_commit_string (int client_id, const WideString &wstr);
    void panel_slot_forward_key_event (int client_id, const KeyEvent &key);
    void panel_slot_request_help (int client_id);
    void panel_slot_request_factory_menu (int client_id);
    void panel_slot_change_factory (int client_id, const String &uuid);

    void panel_req_update_screen          (const IBusCtx *ctx);
    void panel_req_show_help              (const IBusCtx *ctx);
    void panel_req_show_factory_menu      (const IBusCtx *ctx);
    void panel_req_focus_in               (const IBusCtx *ctx);
    void panel_req_focus_out              (const IBusCtx *ctx);
    void panel_req_update_factory_info    (const IBusCtx *ctx);
    void panel_req_update_spot_location   (const IBusCtx *ctx);

    bool filter_hotkeys                   (IBusCtx *ctx, const KeyEvent &key);
};

#endif

/*
vi:ts=4:nowrap:ai:expandtab
*/
