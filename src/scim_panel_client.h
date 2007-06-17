/**
 * @file scim_panel_client.h
 * @brief Defines scim::PanelClient and it's related types.
 *
 * scim::PanelClient is a class used to connect with a Panel daemon.
 * It acts like a Socket Client and handles all socket communication 
 * issues.
 */

/* 
 * Smart Common Input Method
 * 
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
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
 * $Id: scim_panel_client.h,v 1.4 2005/06/26 16:35:33 suzhe Exp $
 */

#ifndef __SCIM_PANEL_CLIENT_H
#define __SCIM_PANEL_CLIENT_H

#include <scim_panel_common.h>

namespace scim {

/**
 * @addtogroup Panel
 * @{
 */

typedef Slot1<void, int>
        PanelClientSlotVoid;

typedef Slot2<void, int, int>
        PanelClientSlotInt;

typedef Slot2<void, int, const String &>
        PanelClientSlotString;

typedef Slot2<void, int, const WideString &>
        PanelClientSlotWideString;

typedef Slot4<void, int, const String &, const String &, const Transaction &>
        PanelClientSlotStringStringTransaction;

typedef Slot2<void, int, const KeyEvent &>
        PanelClientSlotKeyEvent;

/**
 * @brief PanelClient is used by FrontEnd to communicate with Panel daemon.
 *
 * All socket communication between FrontEnd and Panel is handled by this class.
 * FrontEnd may just register some slots to the corresponding signals to handle
 * the events sent from Panel.
 */
class PanelClient
{
    class PanelClientImpl;
    PanelClientImpl *m_impl;

    PanelClient (const PanelClient &);
    const PanelClient & operator = (const PanelClient &);
    
public:
    PanelClient ();
    ~PanelClient ();

    /**
     * @brief Open socket connection to the Panel.
     *
     * FrontEnd and Panel communicate with each other via the Socket created by Panel.
     *
     * FrontEnd can select/poll on the connection id returned by this method to see
     * if there are any data available to be read. If any data are available,
     * PanelClient::filter_event() should be called to process the data.
     *
     * If PanelClient::filter_event() returns false, then it means that the connection
     * is broken and should be re-established by calling PanelClient::close_connection()
     * and PanelClient::open_connection() again.
     *
     * This method would try to launch the panel daemon and make connection again,
     * if the connection could not be established successfully.
     * So this method should always success, unless the panel could not be started on
     * the certain display.
     *
     * @param config  The config module name which should be used by launching the panel daemon.
     * @param display The display name which the panel daemon should run on.
     * @return The id of the socket connection, -1 means connection is failed.
     */
    int  open_connection        (const String &config, const String &display);

    /**
     * @brief Close the connection to Panel.
     */
    void close_connection       ();

    /**
     * @brief Return the connection id, which was returned by PanelClient::open_connection().
     */
    int  get_connection_number  () const;

    /**
     * @brief Return whether this PanelClient has been connected to a Panel.
     */
    bool is_connected           () const;

    /**
     * @brief Check if there are any events available to be processed.
     *
     * If it returns true then FrontEnd should call
     * PanelClient::filter_event() to process them.
     *
     * @return true if there are any events available.
     */
    bool has_pending_event      () const;

    /**
     * @brief Filter the events sent from Panel daemon.
     *
     * Corresponding signal will be emitted in this method.
     *
     * @return false if the connection is broken, otherwise return true.
     */
    bool filter_event           ();

    /**
     * @brief Prepare the send transation for an IC.
     *
     * This method should be called before any events would be sent to Panel.
     *
     * @param icid The id of the IC which has events to be sent to Panel.
     * @return true if the preparation is ok.
     */
    bool prepare                (int icid);

    /**
     * @brief Send the transaction to Panel.
     * @return true if sent successfully.
     */
    bool send                   ();

public:
    /**
     * @name Action methods to send events to Panel.
     * @{
     */
    void turn_on                (int icid);
    void turn_off               (int icid);
    void update_screen          (int icid, int screen);
    void show_help              (int icid, const String &help);
    void show_factory_menu      (int icid, const std::vector <PanelFactoryInfo> &menu);
    void focus_in               (int icid, const String &uuid);
    void focus_out              (int icid);
    void update_factory_info    (int icid, const PanelFactoryInfo &info);
    void update_spot_location   (int icid, int x, int y);
    void show_preedit_string    (int icid);
    void show_aux_string        (int icid);
    void show_lookup_table      (int icid);
    void hide_preedit_string    (int icid);
    void hide_aux_string        (int icid);
    void hide_lookup_table      (int icid);
    void update_preedit_string  (int icid, const WideString &str, const AttributeList &attrs);
    void update_preedit_caret   (int icid, int caret);
    void update_aux_string      (int icid, const WideString &str, const AttributeList &attrs);
    void update_lookup_table    (int icid, const LookupTable &table);
    void register_properties    (int icid, const PropertyList &properties);
    void update_property        (int icid, const Property &property);
    void start_helper           (int icid, const String &helper_uuid);
    void stop_helper            (int icid, const String &helper_uuid);
    void send_helper_event      (int icid, const String &helper_uuid, const Transaction &trans);
    void register_input_context (int icid, const String &uuid);
    void remove_input_context   (int icid);
    /** @} */

public:
    /**
     * @name Signal connection functions.
     *
     * These functions are used by FrontEnds to connect their corresponding slots to
     * this PanelClient's signals.
     *
     * The first parameter of each slot method is always "int context", which is the
     * id of the input method context.
     *
     * @{
     */

    /**
     * @brief Signal: reload configuration.
     *
     * slot prototype: void reload_config (int context);
     *
     * The context parameter is useless here.
     */
    Connection signal_connect_reload_config                 (PanelClientSlotVoid                    *slot);

    /**
     * @brief Signal: exit the FrontEnd
     *
     * slot prototype: void exit (int context);
     *
     * The context parameter is useless here.
     */
    Connection signal_connect_exit                          (PanelClientSlotVoid                    *slot);

    /**
     * @brief Signal: update lookup table page size
     *
     * slot prototype: void update_lookup_table_page_size (int context, int page_size);
     */ 
    Connection signal_connect_update_lookup_table_page_size (PanelClientSlotInt                     *slot);

    /**
     * @brief Signal: lookup table page up
     *
     * slot prototype: void lookup_table_page_up (int context);
     */
    Connection signal_connect_lookup_table_page_up          (PanelClientSlotVoid                    *slot);

    /**
     * @brief Signal: lookup table page down
     *
     * slot prototype: void lookup_table_page_down (int context);
     */
    Connection signal_connect_lookup_table_page_down        (PanelClientSlotVoid                    *slot);

    /**
     * @brief Signal: trigger property
     *
     * slot prototype: void trigger_property (int context, const String &property);
     */
    Connection signal_connect_trigger_property              (PanelClientSlotString                  *slot);

    /**
     * @brief Signal: process helper event
     *
     * slot prototype: void process_helper_event (int context, const String &target_uuid, const String &helper_uuid, const Transaction &trans);
     *
     * - target_uuid is UUID of the IMEngineInstance object which should handle the events.
     * - helper_uuid is UUID of the Helper which sent the events.
     * - trans contains the events.
     */
    Connection signal_connect_process_helper_event          (PanelClientSlotStringStringTransaction *slot);

    /**
     * @brief Signal: move preedit caret
     *
     * slot prototype: void move_preedit_caret (int context, int caret_pos);
     */ 
    Connection signal_connect_move_preedit_caret            (PanelClientSlotInt                     *slot);

    /**
     * @brief Signal: select candidate
     *
     * slot prototype: void select_candidate (int context, int cand_index);
     */
    Connection signal_connect_select_candidate              (PanelClientSlotInt                     *slot);

    /**
     * @brief Signal: process key event
     *
     * slot prototype: void process_key_event (int context, const KeyEvent &key);
     */
    Connection signal_connect_process_key_event             (PanelClientSlotKeyEvent                *slot);

    /**
     * @brief Signal: commit string
     *
     * slot prototype: void commit_string (int context, const WideString &wstr);
     */
    Connection signal_connect_commit_string                 (PanelClientSlotWideString              *slot);
    
    /**
     * @brief Signal: forward key event
     *
     * slot prototype: void forward_key_event (int context, const KeyEvent &key);
     */
    Connection signal_connect_forward_key_event             (PanelClientSlotKeyEvent                *slot);

    /**
     * @brief Signal: request help
     *
     * slot prototype: void request_help (int context);
     */
    Connection signal_connect_request_help                  (PanelClientSlotVoid                    *slot);

    /**
     * @brief Signal: request factory menu
     *
     * slot prototype: void request_factory_menu (int context);
     */
    Connection signal_connect_request_factory_menu          (PanelClientSlotVoid                    *slot);

    /**
     * @brief Signal: change factory
     *
     * slot prototype: void change_factory (int context, const String &uuid);
     */
    Connection signal_connect_change_factory                (PanelClientSlotString                  *slot);
    /** @} */
};

/**  @} */

} // namespace scim

#endif //__SCIM_PANEL_CLIENT_H

/*
vi:ts=4:nowrap:ai:expandtab
*/

