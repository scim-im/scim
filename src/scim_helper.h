/**
 * @file scim_helper.h
 * @brief Defines scim::HelperAgent and it's related types.
 *
 * scim::HelperAgent is a class used to write Client Helper modules.
 *
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
 * $Id: scim_helper.h,v 1.16 2005/05/24 12:22:51 suzhe Exp $
 */

#ifndef __SCIM_HELPER_H
#define __SCIM_HELPER_H

namespace scim {

/**
 * @addtogroup Helper
 * The accessory classes to help develop and manage Client Helper objects.
 * @{
 */
class HelperError: public Exception
{
public:
    HelperError (const String& what_arg)
        : Exception (String("scim::Helper: ") + what_arg) { }
};

/**
 * @brief Helper option indicates that it's a stand alone Helper.
 *
 * Stand alone Helper has no corresponding IMEngine Factory,
 * Such Helper can not be started by IMEngine Factory. 
 * So Panel must provide a menu, or something else, which contains
 * all stand alone Helper items, so that user can start them by
 * clicking the items.
 */
const uint32 SCIM_HELPER_STAND_ALONE             = 1;

/**
 * @brief Helper option indicates that it must be started
 * automatically when Panel starts.
 *
 * If Helper objects want to start itself as soon as
 * the Panel starts, set this option.
 */
const uint32 SCIM_HELPER_AUTO_START              = (1<<1);

/**
 * @brief Helper option indicates that it should be restarted
 * when it exits abnormally.
 *
 * This option should not be used with #SCIM_HELPER_STAND_ALONE.
 */
const uint32 SCIM_HELPER_AUTO_RESTART            = (1<<2);

/**
 * @brief Helper option indicates that it needs the screen update
 * information.
 *
 * Helper object with this option will receive the UPDATE_SCREEN event
 * when the screen of focused ic is changed.
 */
const uint32 SCIM_HELPER_NEED_SCREEN_INFO        = (1<<3);

/**
 * @brief Helper option indicates that it needs the spot location
 * information.
 *
 * Helper object with this option will receive the SPOT_LOCATION_INFO event
 * when the spot location of focused ic is changed.
 */
const uint32 SCIM_HELPER_NEED_SPOT_LOCATION_INFO = (1<<4);

/**
 * @brief Structure to hold the information of a Helper object.
 */
struct HelperInfo
{
    String uuid;            /**< The UUID of this Helper object */
    String name;            /**< The Name of this Helper object, UTF-8 encoding. */
    String icon;            /**< The Icon file path of this Helper object. */
    String description;     /**< The short description of this Helper object. */
    uint32 option;          /**< The options of this Helper object. @sa #SCIM_HELPER_STAND_ALONE etc.*/

    HelperInfo (const String &puuid = String (),
                const String &pname = String (),
                const String &picon = String (),
                const String &pdesc = String (),
                uint32 opt = 0)
        : uuid (puuid),
          name (pname),
          icon (picon),
          description (pdesc),
          option (opt) {
    }
};

class HelperAgent;

typedef Slot3<void, const HelperAgent *, int, const String &>
        HelperAgentSlotVoid;

typedef Slot4<void, const HelperAgent *, int, const String &, const String &>
        HelperAgentSlotString;

typedef Slot4<void, const HelperAgent *, int, const String &, int>
        HelperAgentSlotInt;

typedef Slot5<void, const HelperAgent *, int, const String &, int, int>
        HelperAgentSlotIntInt;

typedef Slot4<void, const HelperAgent *, int, const String &, const Transaction &>
        HelperAgentSlotTransaction;

/**
 * @brief The accessory class to write a Helper object.
 *
 * This class implements all Socket Transaction protocol between
 * Helper object and Panel.
 */
class HelperAgent
{
    class HelperAgentImpl;
    HelperAgentImpl *m_impl;

    HelperAgent (const HelperAgent &);
    const HelperAgent & operator = (const HelperAgent &);

public:
    HelperAgent  ();
    ~HelperAgent ();

    /**
     * @brief Open socket connection to the Panel.
     *
     * Helper objects and Panel communicate with each other via the Socket
     * created by Panel, just same as the Socket between FrontEnds and Panel.
     *
     * Helper object can select/poll on the connection id returned by this function
     * to see if there are any data available to be read. If any data are available,
     * Helper object should call HelperAgent::filter_event() to process the data.
     *
     * This method should be called after the necessary signal-slots are connected.
     * If this Helper is started by an IMEngine Instance, then signal attach_input_context
     * will be emitted during this call.
     *
     * Signal update_screen will be emitted during this call as well to set the startup
     * screen of this Helper. The ic and ic_uuid parameters are invalid here.
     *
     * @param info The information of this Helper object.
     * @param display The display which this Helper object should run on.
     *
     * @return The connection socket id. -1 means failed to create
     *         the connection.
     */
    int  open_connection        (const HelperInfo   &info,
                                 const String       &display);

    /**
     * @brief Close the socket connection to Panel.
     */
    void close_connection       ();

    /**
     * @brief Get the connection id previously returned by open_connection().
     */
    int  get_connection_number  () const;

    /**
     * @brief Return whether this HelperAgent has been connected to a Panel.
     */
    bool is_connected           () const;

    /**
     * @brief Check if there are any events available to be processed.
     *
     * If it returns true then Helper object should call
     * HelperAgent::filter_event() to process them.
     *
     * @return true if there are any events available.
     */
    bool has_pending_event      () const;

    /**
     * @brief Process the pending events.
     *
     * This function will emit the corresponding signals according
     * to the events.
     *
     * @return false if the connection is broken, otherwise return true.
     */
    bool filter_event           ();

    /**
     * @brief Request SCIM to reload all configuration.
     *
     * This function should only by used by Setup Helper to request
     * scim's reloading the configuration.
     */
    void reload_config          () const;

    /**
     * @brief Register some properties into Panel.
     *
     * This function send the request to Panel to register a list
     * of Properties.
     *
     * @param properties The list of Properties to be registered into Panel.
     *
     * @sa scim::Property.
     */
    void register_properties    (const PropertyList &properties) const;

    /**
     * @brief Update a registered property.
     *
     * @param property The property to be updated.
     */
    void update_property        (const Property     &property) const;

    /**
     * @brief Send a set of events to an IMEngineInstance.
     *
     * All events should be put into a Transaction.
     * And the events can only be received by one IMEngineInstance object.
     *
     * @param ic The handle of the Input Context to receive the events.
     * @param ic_uuid The UUID of the Input Context.
     * @param trans The Transaction object holds the events.
     */
    void send_imengine_event    (int                 ic,
                                 const String       &ic_uuid,
                                 const Transaction  &trans) const;

    /**
     * @brief Send a KeyEvent to an IMEngineInstance.
     *
     * @param ic The handle of the IMEngineInstance to receive the event.
     *        -1 means the currently focused IMEngineInstance.
     * @param ic_uuid The UUID of the IMEngineInstance. Empty means don't match.
     * @param key The KeyEvent to be sent.
     */
    void send_key_event         (int                 ic,
                                 const String       &ic_uuid,
                                 const KeyEvent     &key) const;

    /**
     * @brief Forward a KeyEvent to client application directly.
     *
     * @param ic The handle of the client Input Context to receive the event.
     *        -1 means the currently focused Input Context.
     * @param ic_uuid The UUID of the IMEngine used by the Input Context.
     *        Empty means don't match.
     * @param key The KeyEvent to be forwarded.
     */
    void forward_key_event      (int                 ic,
                                 const String       &ic_uuid,
                                 const KeyEvent     &key) const;

    /**
     * @brief Commit a WideString to client application directly.
     *
     * @param ic The handle of the client Input Context to receive the WideString.
     *        -1 means the currently focused Input Context.
     * @param ic_uuid The UUID of the IMEngine used by the Input Context.
     *        Empty means don't match.
     * @param wstr The WideString to be committed.
     */
    void commit_string          (int                 ic,
                                 const String       &ic_uuid,
                                 const WideString   &wstr) const;

public:
    /**
     * @brief Connect a slot to Helper exit signal.
     *
     * This signal is used to let the Helper exit.
     *
     * The prototype of the slot is:
     *
     * void exit (const HelperAgent *agent, int ic, const String &ic_uuid);
     *
     * Parameters:
     * - agent    The pointer to the HelperAgent object which emits this signal.
     * - ic       An opaque handle of the currently focused input context.
     * - ic_uuid  The UUID of the IMEngineInstance associated with the focused input context.
     */
    Connection signal_connect_exit                   (HelperAgentSlotVoid        *slot);

    /**
     * @brief Connect a slot to Helper attach input context signal.
     *
     * This signal is used to attach an input context to this helper.
     *
     * When an input context requst to start this helper, then this
     * signal will be emitted as soon as the helper is started.
     *
     * When an input context want to start an already started helper,
     * this signal will also be emitted.
     *
     * Helper can send some events back to the IMEngineInstance in this
     * signal-slot, to inform that it has been started sccessfully.
     *
     * The prototype of the slot is:
     *
     * void attach_input_context (const HelperAgent *agent, int ic, const String &ic_uuid);
     */
    Connection signal_connect_attach_input_context   (HelperAgentSlotVoid        *slot);

    /**
     * @brief Connect a slot to Helper detach input context signal.
     *
     * This signal is used to detach an input context from this helper.
     *
     * When an input context requst to stop this helper, then this
     * signal will be emitted.
     *
     * Helper shouldn't send any event back to the IMEngineInstance, because
     * the IMEngineInstance attached to the ic should have been destroyed.
     *
     * The prototype of the slot is:
     *
     * void detach_input_context (const HelperAgent *agent, int ic, const String &ic_uuid);
     */
    Connection signal_connect_detach_input_context   (HelperAgentSlotVoid        *slot);

    /**
     * @brief Connect a slot to Helper reload config signal.
     *
     * This signal is used to let the Helper reload configuration.
     *
     * The prototype of the slot is:
     *
     * void reload_config (const HelperAgent *agent, int ic, const String &ic_uuid);
     */
    Connection signal_connect_reload_config          (HelperAgentSlotVoid        *slot);

    /**
     * @brief Connect a slot to Helper update screen signal.
     *
     * This signal is used to let the Helper move its GUI to another screen.
     * It can only be emitted when SCIM_HELPER_NEED_SCREEN_INFO is set in HelperInfo.option.
     * 
     * The prototype of the slot is:
     *
     * void update_screen (const HelperAgent *agent, int ic, const String &ic_uuid, int screen_number);
     */
    Connection signal_connect_update_screen          (HelperAgentSlotInt         *slot);

    /**
     * @brief Connect a slot to Helper update spot location signal.
     *
     * This signal is used to let the Helper move its GUI according to the current spot location.
     * It can only be emitted when SCIM_HELPER_NEED_SPOT_LOCATION_INFO is set in HelperInfo.option.
     * 
     * The prototype of the slot is:
     * void update_spot_location (const HelperAgent *agent, int ic, const String &ic_uuid, int x, int y);
     */
    Connection signal_connect_update_spot_location   (HelperAgentSlotIntInt      *slot);

    /**
     * @brief Connect a slot to Helper trigger property signal.
     *
     * This signal is used to trigger a property registered by this Helper.
     * A property will be triggered when user clicks on it.
     *
     * The prototype of the slot is:
     * void trigger_property (const HelperAgent *agent, int ic, const String &ic_uuid, const String &property);
     */
    Connection signal_connect_trigger_property       (HelperAgentSlotString      *slot);

    /**
     * @brief Connect a slot to Helper process imengine event signal.
     *
     * This signal is used to deliver the events sent from IMEngine to Helper.
     *
     * The prototype of the slot is:
     * void process_imengine_event (const HelperAgent *agent, int ic, const String &ic_uuid, const Transaction &transaction);
     */
    Connection signal_connect_process_imengine_event (HelperAgentSlotTransaction *slot);
};

/**  @} */

} // namespace scim

#endif //__SCIM_HELPER_H

/*
vi:ts=4:nowrap:ai:expandtab
*/

