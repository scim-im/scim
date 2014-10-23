/**
 * @file scim_panel_agent.h
 * @brief Defines scim::PanelAgent and their related types.
 *
 * scim::PanelAgent is a class used to write Panel daemons.
 * It acts like a Socket Server and handles all socket clients
 * issues.
 */

/*
 * Smart Common Input Method
 *
 * Copyright (c) 2004-2005 James Su <suzhe@tsinghua.org.cn>
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
 * $Id: scim_panel_agent.h,v 1.2 2005/06/11 14:50:31 suzhe Exp $
 */

#ifndef __SCIM_PANEL_AGENT_H
#define __SCIM_PANEL_AGENT_H

#include <scim_panel_common.h>

namespace scim {

/**
 * @addtogroup Panel
 * The accessory classes to help develop Panel daemons and FrontEnds
 * which need to communicate with Panel daemons.
 * @{
 */

typedef Slot0<void>
        PanelAgentSlotVoid;

typedef Slot1<void, int>
        PanelAgentSlotInt;

typedef Slot1<void, const String &>
        PanelAgentSlotString;

typedef Slot1<void, const PanelFactoryInfo &>
        PanelAgentSlotFactoryInfo;

typedef Slot1<void, const std::vector <PanelFactoryInfo> &>
        PanelAgentSlotFactoryInfoVector;

typedef Slot1<void, const LookupTable &>
        PanelAgentSlotLookupTable;

typedef Slot1<void, const Property &>
        PanelAgentSlotProperty;

typedef Slot1<void, const PropertyList &>
        PanelAgentSlotPropertyList;

typedef Slot2<void, int, int>
        PanelAgentSlotIntInt;

typedef Slot2<void, int, const Property &>
        PanelAgentSlotIntProperty;

typedef Slot2<void, int, const PropertyList &>
        PanelAgentSlotIntPropertyList;

typedef Slot2<void, int, const HelperInfo &>
        PanelAgentSlotIntHelperInfo;

typedef Slot2<void, const String &, const AttributeList &>
        PanelAgentSlotAttributeString;

/**
 * @brief The class to implement all socket protocol in Panel.
 *
 * This class acts like a stand alone SocketServer.
 * It has its own dedicated main loop, and will be blocked when run () is called.
 * So run () must be called within a separated thread, in order to not block
 * the main loop of the Panel program itself.
 *
 * Before calling run (), the panel must hook the callback functions to the
 * corresponding signals.
 *
 * Note that, there are two special signals: lock(void) and unlock(void). These
 * two signals are used to provide a thread lock to PanelAgent, so that PanelAgent
 * can run correctly within a multi-threading Panel program.
 */
class PanelAgent
{
    class PanelAgentImpl;
    PanelAgentImpl *m_impl;

    PanelAgent (const HelperAgent &);
    const PanelAgent & operator = (const HelperAgent &);

public:
    PanelAgent ();
    ~PanelAgent ();

    /**
     * @brief Initialize this PanelAgent.
     *
     * @param config The name of the config module to be used by Helpers.
     * @param display The name of display, on which the Panel should run.
     * @param resident If this is true then this PanelAgent will keep running
     *                 even if there is no more client connected.
     *
     * @return true if the PanelAgent is initialized correctly and ready to run.
     */
    bool initialize (const String &config, const String &display, bool resident = false);

    /**
     * @brief Check if this PanelAgent is initialized correctly and ready to run.
     *
     * @return true if this PanelAgent is ready to run.
     */
    bool valid (void) const;

    /**
     * @brief Run this PanelAgent.
     *
     * This method has its own dedicated main loop,
     * so it should be run in a separated thread.
     *
     * @return false if the Panel SocketServer encountered an error when running.
     *               Otherwise, it won't return until the server exits.
     */
    bool run (void);

    /**
     * @brief Stop this PanelAgent.
     */
    void stop (void);

public:

    /**
     * @brief Get the list of all helpers.
     *
     * Panel program should provide a menu which contains
     * all stand alone but not auto start Helpers, so that users can activate
     * the Helpers by clicking in the menu.
     *
     * All auto start Helpers should be started by Panel after running PanelAgent
     * by calling PanelAgent::start_helper().
     *
     * @param helpers A list contains information of all Helpers.
     */
    int get_helper_list (std::vector <HelperInfo> & helpers) const;

public:
    /**
     * @brief Let the focused IMEngineInstance object move the preedit caret.
     *
     * @param position The new preedit caret position.
     * @return true if the command was sent correctly.
     */
    bool move_preedit_caret             (uint32         position);

    /**
     * @brief Request help information from the focused IMEngineInstance object.
     * @return true if the command was sent correctly.
     */
    bool request_help                   (void);

    /**
     * @brief Request factory menu from the focused FrontEnd.
     * @return true if the command was sent correctly.
     */
    bool request_factory_menu           (void);

    /**
     * @brief Change the factory used by the focused IMEngineInstance object.
     *
     * @param uuid The uuid of the new factory.
     * @return true if the command was sent correctly.
     */
    bool change_factory                 (const String  &uuid);

    /**
     * @brief Let the focused IMEngineInstance object
     *        select a candidate in current lookup table.
     *
     * @param item The index of the selected candidate.
     * @return true if the command was sent correctly.
     */
    bool select_candidate               (uint32         item);

    /**
     * @brief Let the focused IMEngineInstance object
     *        flip the LookupTable to previous page.
     * @return true if the command was sent correctly.
     */
    bool lookup_table_page_up           (void);

    /**
     * @brief Let the focused IMEngineInstance object
     *        flip the LookupTable to next page.
     * @return true if the command was sent correctly.
     */
    bool lookup_table_page_down         (void);

    /**
     * @brief Let the focused IMEngineInstance object
     *        update the page size of the LookupTable.
     *
     * @param size The new page size.
     * @return true if the command was sent correctly.
     */
    bool update_lookup_table_page_size  (uint32         size);

    /**
     * @brief Trigger a property of the focused IMEngineInstance object.
     *
     * @param property The property key to be triggered.
     * @return true if the command was sent correctly.
     */
    bool trigger_property               (const String  &property);

    /**
     * @brief Trigger a property of a Helper object.
     *
     * @param client The client id of the Helper object.
     * @param property The property key to be triggered.
     * @return true if the command was sent correctly.
     */
    bool trigger_helper_property        (int            client,
                                         const String  &property);

    /**
     * @brief Start a stand alone helper.
     *
     * @param uuid The uuid of the Helper object to be started.
     * @return true if the command was sent correctly.
     */
    bool start_helper                   (const String  &uuid);

    /**
     * @brief Let all FrontEnds and Helpers reload configuration.
     * @return true if the command was sent correctly.
     */
    bool reload_config                  (void);

    /**
     * @brief Let all FrontEnds, Helpers and this Panel exit.
     * @return true if the command was sent correctly.
     */
    bool exit                           (void);

public:
    /**
     * @brief Signal: Reload configuration.
     *
     * When a Helper object send a RELOAD_CONFIG event to this Panel,
     * this signal will be emitted. Panel should reload all configuration here.
     */
    Connection signal_connect_reload_config              (PanelAgentSlotVoid                *slot);

    /**
     * @brief Signal: Turn on.
     *
     * slot prototype: void turn_on (void);
     */
    Connection signal_connect_turn_on                    (PanelAgentSlotVoid                *slot);

    /**
     * @brief Signal: Turn off.
     *
     * slot prototype: void turn_off (void);
     */
    Connection signal_connect_turn_off                   (PanelAgentSlotVoid                *slot);

    /**
     * @brief Signal: Update screen.
     *
     * slot prototype: void update_screen (int screen);
     */
    Connection signal_connect_update_screen              (PanelAgentSlotInt                 *slot);

    /**
     * @brief Signal: Update spot location.
     *
     * slot prototype: void update_spot_location (int x, int y);
     */
    Connection signal_connect_update_spot_location       (PanelAgentSlotIntInt              *slot);

    /**
     * @brief Signal: Update factory information.
     *
     * slot prototype: void update_factory_info (const PanelFactoryInfo &info);
     */
    Connection signal_connect_update_factory_info        (PanelAgentSlotFactoryInfo         *slot);

    /**
     * @brief Signal: Show help.
     *
     * slot prototype: void show_help (const String &help);
     */
    Connection signal_connect_show_help                  (PanelAgentSlotString              *slot);

    /**
     * @brief Signal: Show factory menu.
     *
     * slot prototype: void show_factory_menu (const std::vector <PanelFactoryInfo> &menu);
     */
    Connection signal_connect_show_factory_menu          (PanelAgentSlotFactoryInfoVector   *slot);

    /**
     * @brief Signal: Show preedit string.
     *
     * slot prototype: void show_preedit_string (void):
     */
    Connection signal_connect_show_preedit_string        (PanelAgentSlotVoid                *slot);

    /**
     * @brief Signal: Show aux string.
     *
     * slot prototype: void show_aux_string (void):
     */
    Connection signal_connect_show_aux_string            (PanelAgentSlotVoid                *slot);

    /**
     * @brief Signal: Show lookup table.
     *
     * slot prototype: void show_lookup_table (void):
     */
    Connection signal_connect_show_lookup_table          (PanelAgentSlotVoid                *slot);

    /**
     * @brief Signal: Hide preedit string.
     *
     * slot prototype: void hide_preedit_string (void);
     */
    Connection signal_connect_hide_preedit_string        (PanelAgentSlotVoid                *slot);

    /**
     * @brief Signal: Hide aux string.
     *
     * slot prototype: void hide_aux_string (void);
     */
    Connection signal_connect_hide_aux_string            (PanelAgentSlotVoid                *slot);

    /**
     * @brief Signal: Hide lookup table.
     *
     * slot prototype: void hide_lookup_table (void);
     */
    Connection signal_connect_hide_lookup_table          (PanelAgentSlotVoid                *slot);

    /**
     * @brief Signal: Update preedit string.
     *
     * slot prototype: void update_preedit_string (const String &str, const AttributeList &attrs);
     * - str   -- The UTF-8 encoded string to be displayed in preedit area.
     * - attrs -- The attributes of the string.
     */
    Connection signal_connect_update_preedit_string      (PanelAgentSlotAttributeString     *slot);

    /**
     * @brief Signal: Update preedit caret.
     *
     * slot prototype: void update_preedit_caret (int caret);
     */
    Connection signal_connect_update_preedit_caret       (PanelAgentSlotInt                 *slot);

    /**
     * @brief Signal: Update aux string.
     *
     * slot prototype: void update_aux_string (const String &str, const AttributeList &attrs);
     * - str   -- The UTF-8 encoded string to be displayed in aux area.
     * - attrs -- The attributes of the string.
     */
    Connection signal_connect_update_aux_string          (PanelAgentSlotAttributeString     *slot);

    /**
     * @brief Signal: Update lookup table.
     *
     * slot prototype: void update_lookup_table (const LookupTable &table);
     * - table -- The new LookupTable object.
     */
    Connection signal_connect_update_lookup_table        (PanelAgentSlotLookupTable         *slot);

    /**
     * @brief Signal: Register properties.
     *
     * Register properties of the focused instance.
     *
     * slot prototype: void register_properties (const PropertyList &props);
     * - props -- The properties to be registered.
     */
    Connection signal_connect_register_properties        (PanelAgentSlotPropertyList        *slot);

    /**
     * @brief Signal: Update property.
     *
     * Update a property of the focused instance.
     *
     * slot prototype: void update_property (const Property &prop);
     * - prop -- The property to be updated.
     */
    Connection signal_connect_update_property            (PanelAgentSlotProperty            *slot);

    /**
     * @brief Signal: Register properties of a helper.
     *
     * slot prototype: void register_helper_properties (int id, const PropertyList &props);
     * - id    -- The client id of the helper object.
     * - props -- The properties to be registered.
     */
    Connection signal_connect_register_helper_properties (PanelAgentSlotIntPropertyList     *slot);

    /**
     * @brief Signal: Update helper property.
     *
     * slot prototype: void update_helper_property (int id, const Property &prop);
     * - id   -- The client id of the helper object.
     * - prop -- The property to be updated.
     */
    Connection signal_connect_update_helper_property     (PanelAgentSlotIntProperty         *slot);

    /**
     * @brief Signal: Register a helper object.
     *
     * A newly started helper object will send this event to Panel.
     *
     * slot prototype: register_helper (int id, const HelperInfo &helper);
     * - id     -- The client id of the helper object.
     * - helper -- The information of the helper object.
     */
    Connection signal_connect_register_helper            (PanelAgentSlotIntHelperInfo       *slot);

    /**
     * @brief Signal: Remove a helper object.
     *
     * If a running helper close its connection to Panel, then this signal will be triggered to
     * tell Panel to remove all data associated to this helper.
     *
     * slot prototype: void remove_helper (int id);
     * - id -- The client id of the helper object to be removed.
     */
    Connection signal_connect_remove_helper              (PanelAgentSlotInt                 *slot);

    /**
     * @brief Signal: A transaction is started.
     *
     * This signal infers that the Panel should disable update before this transaction finishes.
     *
     * slot prototype: void signal_connect_transaction_start (void);
     */
    Connection signal_connect_transaction_start    (PanelAgentSlotVoid                *slot);

    /**
     * @brief Signal: A transaction is finished.
     *
     * This signal will get emitted when a transaction is finished. This implys to re-enable
     * panel GUI update
     *
     * slot prototype: void signal_connect_transaction_end (void);
     */
    Connection signal_connect_transaction_end      (PanelAgentSlotVoid                *slot);

    /**
     * @brief Signal: Lock the exclusive lock for this PanelAgent.
     *
     * The panel program should provide a thread lock and hook a slot into this signal to lock it.
     * PanelAgent will use this lock to ensure the data integrity.
     *
     * slot prototype: void lock (void);
     */
    Connection signal_connect_lock                       (PanelAgentSlotVoid                *slot);

    /**
     * @brief Signal: Unlock the exclusive lock for this PanelAgent.
     *
     * slot prototype: void unlock (void);
     */
    Connection signal_connect_unlock                     (PanelAgentSlotVoid                *slot);
};

/**  @} */

} // namespace scim

#endif //__SCIM_PANEL_AGENT_H

/*
vi:ts=4:nowrap:ai:expandtab
*/

