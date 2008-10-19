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
 * $Id: scim_hotkey.cpp,v 1.8 2005/06/18 13:19:35 suzhe Exp $
 */

#define Uses_SCIM_HOTKEY
#include "scim_private.h"
#include "scim.h"
#include "scim_stl_map.h"

namespace scim {

struct scim_hash_keyevent {
    size_t operator () (const KeyEvent &key) const {
        return (key.code & 0xFFFF) | (((uint32)key.mask) << 16);
    }
};

#if SCIM_USE_STL_EXT_HASH_MAP
typedef __gnu_cxx::hash_map <KeyEvent, int, scim_hash_keyevent> HotkeyRepository;
#elif SCIM_USE_STL_HASH_MAP
typedef std::hash_map <KeyEvent, int, scim_hash_keyevent>       HotkeyRepository;
#else
typedef std::map <KeyEvent, int>                                HotkeyRepository;
#endif

class HotkeyMatcher::HotkeyMatcherImpl
{
public:
    HotkeyRepository m_hotkeys;
    uint32           m_prev_code;
    bool             m_matched;
    int              m_result;

    HotkeyMatcherImpl ()
        : m_prev_code (0),
          m_matched (false),
          m_result  (-1) {
    }
};

HotkeyMatcher::HotkeyMatcher ()
    : m_impl (new HotkeyMatcherImpl ())
{
}
HotkeyMatcher::~HotkeyMatcher ()
{
    delete m_impl;
}

void
HotkeyMatcher::add_hotkey (const KeyEvent &key, int id)
{
    if (!key.empty ())
        m_impl->m_hotkeys [key] = id;
}

void
HotkeyMatcher::add_hotkeys (const KeyEventList &keys, int id)
{
    for (KeyEventList::const_iterator it = keys.begin (); it != keys.end (); ++it) {
        if (!it->empty ())
            m_impl->m_hotkeys [*it] = id;
    }
}

size_t
HotkeyMatcher::get_all_hotkeys (KeyEventList &keys, std::vector <int> &ids) const
{
    keys.clear ();
    ids.clear ();

    for (HotkeyRepository::iterator it = m_impl->m_hotkeys.begin (); it != m_impl->m_hotkeys.end (); ++it) {
        keys.push_back (it->first);
        ids.push_back (it->second);
    }

    return keys.size ();
}

size_t
HotkeyMatcher::find_hotkeys     (int id, KeyEventList &keys) const
{
    keys.clear ();

    for (HotkeyRepository::iterator it = m_impl->m_hotkeys.begin (); it != m_impl->m_hotkeys.end (); ++it) {
        if (it->second == id) keys.push_back (it->first);
    }

    return keys.size ();
}

void
HotkeyMatcher::push_key_event (const KeyEvent &key)
{
    HotkeyRepository::iterator it = m_impl->m_hotkeys.find (key);

    if (it != m_impl->m_hotkeys.end () && (!(key.mask & SCIM_KEY_ReleaseMask) || m_impl->m_prev_code == key.code)) {
        m_impl->m_matched = true;
        m_impl->m_result = it->second;
    } else {
        m_impl->m_matched = false;
        m_impl->m_result = -1;
    }

    m_impl->m_prev_code = key.code;
}

void
HotkeyMatcher::reset (void)
{
    m_impl->m_prev_code = 0;
    m_impl->m_matched   = false;
    m_impl->m_result    = -1;
}

void
HotkeyMatcher::clear (void)
{
    m_impl->m_hotkeys.clear ();
    reset ();
}

bool
HotkeyMatcher::is_matched (void) const
{
    return m_impl->m_matched;
}

int
HotkeyMatcher::get_match_result (void) const
{
    return m_impl->m_result;
}

//===================== IMEngineHotkeyMatcher =======================
class IMEngineHotkeyMatcher::IMEngineHotkeyMatcherImpl
{
public:    
    HotkeyMatcher        m_matcher;
    std::vector <String> m_uuids;
};

IMEngineHotkeyMatcher::IMEngineHotkeyMatcher ()
    : m_impl (new IMEngineHotkeyMatcherImpl ())
{
}

IMEngineHotkeyMatcher::~IMEngineHotkeyMatcher ()
{
    delete m_impl;
}

void
IMEngineHotkeyMatcher::load_hotkeys (const ConfigPointer &config)
{
    clear ();

    if (config.null () || !config->valid ()) return;

    std::vector <String> uuids;

    scim_split_string_list (uuids, config->read (String (SCIM_CONFIG_HOTKEYS_IMENGINE_LIST), String ("")));

    std::sort (uuids.begin (), uuids.end ());
    uuids.erase (std::unique (uuids.begin (), uuids.end ()), uuids.end ());

    if (uuids.size ()) {
        KeyEventList keys;
        for (std::vector <String>::iterator uit = uuids.begin (); uit != uuids.end (); ++uit) {
            if (scim_string_to_key_list (keys, config->read (String (SCIM_CONFIG_HOTKEYS_IMENGINE "/") + *uit, String ("")))) {
                m_impl->m_matcher.add_hotkeys (keys, (int) m_impl->m_uuids.size ());
                m_impl->m_uuids.push_back (*uit);
            }
        }
    }
}

void
IMEngineHotkeyMatcher::save_hotkeys (const ConfigPointer &config) const
{
    if (config.null () || !config->valid () || !m_impl->m_uuids.size ()) return;

    KeyEventList keys;
    String keystr;
    std::vector <String> uuids;

    for (size_t i = 0; i < m_impl->m_uuids.size (); ++i) {
        if (m_impl->m_matcher.find_hotkeys ((int) i, keys) > 0 && scim_key_list_to_string (keystr, keys)) {
            config->write (String (SCIM_CONFIG_HOTKEYS_IMENGINE "/") + m_impl->m_uuids [i], keystr);
            uuids.push_back (m_impl->m_uuids [i]);
        }
    }

    config->write (String (SCIM_CONFIG_HOTKEYS_IMENGINE_LIST), scim_combine_string_list (uuids));
}

void
IMEngineHotkeyMatcher::add_hotkey (const KeyEvent &key, const String &uuid)
{
    if (key.empty () || !uuid.length ()) return;

    size_t i;

    for (i = 0; i < m_impl->m_uuids.size (); ++i) {
        if (m_impl->m_uuids [i] == uuid)
            break;
    }

    if (i == m_impl->m_uuids.size ())
        m_impl->m_uuids.push_back (uuid);

    m_impl->m_matcher.add_hotkey (key, i);
}

void
IMEngineHotkeyMatcher::add_hotkeys (const KeyEventList &keys, const String &uuid)
{
    if (!keys.size () || !uuid.length ()) return;

    size_t i;

    for (i = 0; i < m_impl->m_uuids.size (); ++i) {
        if (m_impl->m_uuids [i] == uuid)
            break;
    }

    if (i == m_impl->m_uuids.size ())
        m_impl->m_uuids.push_back (uuid);

    m_impl->m_matcher.add_hotkeys (keys, i);
}

size_t
IMEngineHotkeyMatcher::find_hotkeys (const String &uuid, KeyEventList &keys) const
{
    for (size_t i = 0; i < m_impl->m_uuids.size (); ++i) {
        if (m_impl->m_uuids [i] == uuid)
            return m_impl->m_matcher.find_hotkeys ((int) i, keys);
    }

    keys.clear ();
    return 0;
}

size_t
IMEngineHotkeyMatcher::get_all_hotkeys (KeyEventList &keys, std::vector <String> &uuids) const
{
    keys.clear ();
    uuids.clear ();
    std::vector <int> ids;

    if (m_impl->m_matcher.get_all_hotkeys (keys, ids) > 0) {
        for (size_t i = 0; i < ids.size (); ++i)
            uuids.push_back (m_impl->m_uuids [ids [i]]);
    }

    return keys.size ();
}

void
IMEngineHotkeyMatcher::reset (void)
{
    m_impl->m_matcher.reset ();
}

void
IMEngineHotkeyMatcher::clear (void)
{
    m_impl->m_matcher.clear ();
    m_impl->m_uuids.clear ();
}

void
IMEngineHotkeyMatcher::push_key_event (const KeyEvent &key)
{
    m_impl->m_matcher.push_key_event (key);
}

bool
IMEngineHotkeyMatcher::is_matched (void) const
{
    return m_impl->m_matcher.is_matched ();
}

String
IMEngineHotkeyMatcher::get_match_result (void) const
{
    int id = m_impl->m_matcher.get_match_result ();

    if (id >= 0 && id < (int) m_impl->m_uuids.size ())
        return m_impl->m_uuids [id];

    return String ();
}

//============================ FrontEndHotkeyMatcher ==========================
static const char *__scim_frontend_hotkey_config_paths [] =
{
    0,
    SCIM_CONFIG_HOTKEYS_FRONTEND_TRIGGER,
    SCIM_CONFIG_HOTKEYS_FRONTEND_ON,
    SCIM_CONFIG_HOTKEYS_FRONTEND_OFF,
    SCIM_CONFIG_HOTKEYS_FRONTEND_NEXT_FACTORY,
    SCIM_CONFIG_HOTKEYS_FRONTEND_PREVIOUS_FACTORY,
    SCIM_CONFIG_HOTKEYS_FRONTEND_SHOW_FACTORY_MENU,
    0
};

static const char *__scim_frontend_hotkey_defaults [] =
{
    0,
    "Control+space",
    "",
    "",
    "Control+Alt+Down,Control+Shift+Shift_L+KeyRelease,Control+Shift+Shift_R+KeyRelease",
    "Control+Alt+Up,Control+Shift+Control_L+KeyRelease,Control+Shift+Control_R+KeyRelease",
    "Control+Alt+Right",
    0
};

class FrontEndHotkeyMatcher::FrontEndHotkeyMatcherImpl
{
public:    
    HotkeyMatcher        m_matcher;
};

FrontEndHotkeyMatcher::FrontEndHotkeyMatcher ()
    : m_impl (new FrontEndHotkeyMatcherImpl ())
{
}

FrontEndHotkeyMatcher::~FrontEndHotkeyMatcher ()
{
    delete m_impl;
}

void
FrontEndHotkeyMatcher::load_hotkeys (const ConfigPointer &config)
{
    clear ();

    if (config.null () || !config->valid ()) return;

    KeyEventList keys;

    // Load the least important hotkeys first.
    for (int i = SCIM_FRONTEND_HOTKEY_SHOW_FACTORY_MENU; i >= SCIM_FRONTEND_HOTKEY_TRIGGER; --i) {
        if (scim_string_to_key_list (keys, config->read (String (__scim_frontend_hotkey_config_paths [i]), String (__scim_frontend_hotkey_defaults [i]))))
            m_impl->m_matcher.add_hotkeys (keys, i);
    }
}

void
FrontEndHotkeyMatcher::save_hotkeys (const ConfigPointer &config) const
{
    if (config.null () || !config->valid ()) return;

    KeyEventList keys;
    String keystr;

    for (int i = SCIM_FRONTEND_HOTKEY_TRIGGER; i <= SCIM_FRONTEND_HOTKEY_SHOW_FACTORY_MENU; ++i) {
        if (m_impl->m_matcher.find_hotkeys (i, keys) > 0 && scim_key_list_to_string (keystr, keys))
            config->write (String (__scim_frontend_hotkey_config_paths [i]), keystr);
    }
}

void
FrontEndHotkeyMatcher::add_hotkey (const KeyEvent &key, FrontEndHotkeyAction action)
{
    if (key.empty () || action < SCIM_FRONTEND_HOTKEY_TRIGGER || action > SCIM_FRONTEND_HOTKEY_SHOW_FACTORY_MENU) return;

    m_impl->m_matcher.add_hotkey (key, (int) action);
}

void
FrontEndHotkeyMatcher::add_hotkeys (const KeyEventList &keys, FrontEndHotkeyAction action)
{
    if (!keys.size () || action < SCIM_FRONTEND_HOTKEY_TRIGGER || action > SCIM_FRONTEND_HOTKEY_SHOW_FACTORY_MENU) return;

    m_impl->m_matcher.add_hotkeys (keys, (int) action);
}

size_t
FrontEndHotkeyMatcher::find_hotkeys (FrontEndHotkeyAction action, KeyEventList &keys) const
{
    return m_impl->m_matcher.find_hotkeys ((int) action, keys);
}

size_t
FrontEndHotkeyMatcher::get_all_hotkeys (KeyEventList &keys, std::vector <FrontEndHotkeyAction> &actions) const
{
    keys.clear ();
    actions.clear ();

    std::vector <int> ids;

    m_impl->m_matcher.get_all_hotkeys (keys, ids);

    for (size_t i = 0; i < ids.size (); ++i)
        actions.push_back (static_cast<FrontEndHotkeyAction> (ids [i]));

    return keys.size ();
}

void
FrontEndHotkeyMatcher::reset (void)
{
    m_impl->m_matcher.reset ();
}

void
FrontEndHotkeyMatcher::clear (void)
{
    m_impl->m_matcher.clear ();
}

void
FrontEndHotkeyMatcher::push_key_event (const KeyEvent &key)
{
    m_impl->m_matcher.push_key_event (key);
}

bool
FrontEndHotkeyMatcher::is_matched (void) const
{
    return m_impl->m_matcher.is_matched ();
}

FrontEndHotkeyAction
FrontEndHotkeyMatcher::get_match_result (void) const
{
    int id = m_impl->m_matcher.get_match_result ();

    if (id >= SCIM_FRONTEND_HOTKEY_TRIGGER && id <= SCIM_FRONTEND_HOTKEY_SHOW_FACTORY_MENU)
        return static_cast<FrontEndHotkeyAction> (id);

    return SCIM_FRONTEND_HOTKEY_NOOP;
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
