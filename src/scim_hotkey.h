/**
 * @file scim_hotkey.h
 * @brief Defines the scim::HotkeyMatcher and scim::IMEngineHotkeyMatcher classes.
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
 * $Id: scim_hotkey.h,v 1.5 2005/10/06 18:02:06 liuspider Exp $
 */

#ifndef __SCIM_HOTKEY_H
#define __SCIM_HOTKEY_H

namespace scim {
/**
 * @addtogroup Accessories
 * @{
 */

/**
 * @brief This class is used to match a KeyEvent among a set of hotkeys.
 *
 * This class keeps the key event history so that it can match any kind of
 * key events, including key release events, correctly.
 *
 * If there are large amount of hotkeys to be matched, this class can provide
 * very good performance.
 */
class HotkeyMatcher
{
    class HotkeyMatcherImpl;

    HotkeyMatcherImpl *m_impl;

    HotkeyMatcher (const HotkeyMatcher &);
    const HotkeyMatcher & operator = (const HotkeyMatcher &);

public:
    /**
     * @brief Constructor
     */
    HotkeyMatcher ();

    /**
     * @brief Destructor.
     */
    ~HotkeyMatcher ();

    /**
     * @brief Add a Hotkey into this HotkeyMatcher.
     *
     * If a same Hotkey was already added, then it'll be replaced by this new one.
     *
     * @param key A Hotkey to be added.
     * @param id  An id to be binded to this Hotkey.
     */
    void add_hotkey        (const KeyEvent     &key,  int id);

    /**
     * @brief Add a set of Hotkeys into this HotkeyMatcher.
     *
     * If a same Hotkey in the list was already added, then it'll be replaced by the new one.
     *
     * @param keys A set of Hotkeys to be added.
     * @param id  An id to be binded to these Hotkeys.
     */
    void add_hotkeys       (const KeyEventList &keys, int id);

    /**
     * @brief Find all Hotkeys binded to a specific id.
     *
     * @param id The id to be found.
     * @param keys A KeyEventList object to hold all KeyEvents binded to the id.
     * @return The number of Hotkeys found.
     */
    size_t find_hotkeys     (int id, KeyEventList &keys) const;

    /**
     * @brief Get all Hotkeys added into this HotkeyMatcher.
     *
     * @param keys A KeyEventList object to hold all KeyEvents.
     * @param ids  A int list to hold all corresponding IDs.
     *
     * @return The number of available Hotkeys.
     */
    size_t get_all_hotkeys  (KeyEventList &keys, std::vector <int> &ids) const;

    /**
     * @brief Reset the HotkeyMatcher.
     *
     * The KeyEvent queue will be cleared, all state will be reset.
     * The Hotkeys which were already added will not be touched.
     */
    void reset              (void);

    /**
     * @brief Clear all Hotkeys.
     */
    void clear              (void);

    /**
     * @brief Push a KeyEvent into the queue.
     *
     * This KeyEvent will be matched against the available Hotkeys immediately.
     *
     * @param key The key to be pushed into.
     */
    void push_key_event     (const KeyEvent &key);

    /**
     * @brief Check if the last KeyEvent pushed by push_key_event () matched with any Hotkey.
     *
     * @return true If the KeyEvent matched with a Hotkey.
     */
    bool is_matched         (void) const;

    /**
     * @brief Get the match result.
     *
     * @return The corresponding id of the matched Hotkey. If no Hotkey was matched, return -1.
     */
    int  get_match_result   (void) const;

};

/**
 * @brief This class hold all Hotkeys for each IMEngines.
 */
class IMEngineHotkeyMatcher
{
    class IMEngineHotkeyMatcherImpl;

    IMEngineHotkeyMatcherImpl *m_impl;

    IMEngineHotkeyMatcher (const IMEngineHotkeyMatcher &);
    const IMEngineHotkeyMatcher & operator = (const IMEngineHotkeyMatcher &);

public:
    IMEngineHotkeyMatcher       ();
    ~IMEngineHotkeyMatcher      ();

    /**
     * @brief Load all Hotkeys for IMEngines from Config.
     * 
     * @param config The Config object in which the Hotkeys are stored.
     */
    void   load_hotkeys        (const ConfigPointer &config);

    /**
     * @brief Save all Hotkeys for IMEngines to Config.
     *
     * @param config Store all Hotkeys to this Config object.
     */
    void   save_hotkeys        (const ConfigPointer &config) const;

    /**
     * @brief Add a Hotkey for an IMEngine into this IMEngineHotkeyMatcher.
     *
     * @param key  The Hotkey.
     * @param uuid The UUID of the corresponding IMEngine.
     */
    void   add_hotkey          (const KeyEvent &key, const String &uuid);

    /**
     * @brief Add a set of Hotkeys for an IMEngine into this IMEngineHotkeyMatcher.
     *
     * @param keys  The Hotkeys.
     * @param uuid The UUID of the corresponding IMEngine.
     */
    void   add_hotkeys         (const KeyEventList &keys, const String &uuid);

    /**
     * @brief Find all Hotkeys binded to a specific IMEngine UUID.
     *
     * @param uuid The IMEngine uuid to be found.
     * @param keys A KeyEventList object to hold all KeyEvents binded to the uuid.
     * @return The number of Hotkeys found.
     */
    size_t find_hotkeys        (const String &uuid, KeyEventList &keys) const;

    /**
     * @brief Get all hotkeys in this IMEngineHotkeyMatcher.
     *
     * @param keys   A list of all Hotkeys.
     * @param uuids  A list of all corresponding IMEngine UUIDs.
     * @return the number of hotkeys found.
     */
    size_t get_all_hotkeys     (KeyEventList &keys, std::vector <String> &uuids) const;

    /**
     * @brief Reset the IMEngineHotkeyMatcher.
     *
     * The KeyEvent queue will be cleared, all state will be reset.
     * The Hotkeys which were already added will not be touched.
     */
    void   reset                (void);

    /**
     * @brief Clear all Hotkeys and reset the IMEngineHotkeyMatcher.
     */
    void   clear                (void);

    /**
     * @brief Push a KeyEvent into the queue.
     *
     * This KeyEvent will be matched against the available Hotkeys immediately.
     *
     * @param key The key to be pushed into.
     */
    void   push_key_event       (const KeyEvent &key);

    /**
     * @brief Check if the last KeyEvent pushed by push_key_event () matched with any Hotkey.
     *
     * @return true If the KeyEvent matched with a Hotkey.
     */
    bool   is_matched           (void) const;

    /**
     * @brief Get the match result.
     *
     * @return The corresponding UUID of the matched Hotkey. If no Hotkey was matched, return null string.
     */
    String get_match_result   (void) const;
};

/**
 * @brief FrontEnd actions which could be binded with Hotkeys.
 */
enum FrontEndHotkeyAction
{
    SCIM_FRONTEND_HOTKEY_NOOP               = 0,    /**< No action */
    SCIM_FRONTEND_HOTKEY_TRIGGER            = 1,    /**< Turn on/off the input method. */
    SCIM_FRONTEND_HOTKEY_ON                 = 2,    /**< Turn on the input method. */
    SCIM_FRONTEND_HOTKEY_OFF                = 3,    /**< Turn off the input method. */
    SCIM_FRONTEND_HOTKEY_NEXT_FACTORY       = 4,    /**< Switch current Input Context to use the next available IMEngine Factory. */
    SCIM_FRONTEND_HOTKEY_PREVIOUS_FACTORY   = 5,    /**< Switch current Input Context to use the previous available IMEngine Factory. */
    SCIM_FRONTEND_HOTKEY_SHOW_FACTORY_MENU  = 6     /**< Show a menu of all available IMEngine Factories. */
};

/**
 * @brief This class hold all FrontEnd specific Hotkeys, such as trigger keys, on/off keys, etc. 
 */
class FrontEndHotkeyMatcher
{
    class FrontEndHotkeyMatcherImpl;

    FrontEndHotkeyMatcherImpl *m_impl;

    FrontEndHotkeyMatcher (const FrontEndHotkeyMatcher &);
    const FrontEndHotkeyMatcher & operator = (const FrontEndHotkeyMatcher &);

public:
    FrontEndHotkeyMatcher       ();
    ~FrontEndHotkeyMatcher      ();

    /**
     * @brief Load all FrontEnd specific Hotkeys from Config.
     * 
     * @param config The Config object in which the Hotkeys are stored.
     */
    void   load_hotkeys        (const ConfigPointer &config);

    /**
     * @brief Save all FrontEnd specific Hotkeys to Config.
     *
     * @param config Store all Hotkeys to this Config object.
     */
    void   save_hotkeys        (const ConfigPointer &config) const;

    /**
     * @brief Add a Hotkey for an FrontEnd into this FrontEndHotkeyMatcher.
     *
     * @param key  The Hotkey.
     * @param action The action to do when the hotkey is matched.
     */
    void   add_hotkey          (const KeyEvent &key, FrontEndHotkeyAction action);

    /**
     * @brief Add a set of Hotkeys for an FrontEnd into this FrontEndHotkeyMatcher.
     *
     * @param keys  The Hotkeys.
     * @param action The action to do when the hotkey is matched.
     */
    void   add_hotkeys         (const KeyEventList &keys, FrontEndHotkeyAction action);

    /**
     * @brief Find all Hotkeys binded to a specific action.
     *
     * @param action The action to be found.
     * @param keys A KeyEventList object to hold all KeyEvents binded to the action.
     * @return The number of Hotkeys found.
     */
    size_t find_hotkeys        (FrontEndHotkeyAction action, KeyEventList &keys) const;

    /**
     * @brief Get all hotkeys in this FrontEndHotkeyMatcher.
     *
     * @param keys   A list of all Hotkeys.
     * @param actions  A list of all corresponding actions
     */
    size_t get_all_hotkeys     (KeyEventList &keys, std::vector <FrontEndHotkeyAction> &actions) const;

    /**
     * @brief Reset the FrontEndHotkeyMatcher.
     *
     * The KeyEvent queue will be cleared, all state will be reset.
     * The Hotkeys which were already added will not be touched.
     */
    void   reset                (void);

    /**
     * @brief Clear all Hotkeys and reset the FrontEndHotkeyMatcher.
     */
    void   clear                (void);

    /**
     * @brief Push a KeyEvent into the queue.
     *
     * This KeyEvent will be matched against the available Hotkeys immediately.
     *
     * @param key The key to be pushed into.
     */
    void   push_key_event       (const KeyEvent &key);

    /**
     * @brief Check if the last KeyEvent pushed by push_key_event () matched with any Hotkey.
     *
     * @return true If the KeyEvent matched with a Hotkey.
     */
    bool   is_matched           (void) const;

    /**
     * @brief Get the match result.
     *
     * @return The corresponding action of the matched Hotkey.
     */
    FrontEndHotkeyAction get_match_result   (void) const;
};

/** @} */

} // namespace scim

#endif //__SCIM_HOTKEY_H

/*
vi:ts=4:nowrap:ai:expandtab
*/

