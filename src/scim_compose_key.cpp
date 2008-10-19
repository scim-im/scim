/** @file scim_compose_key.cpp
 *  @brief Implementation of class ComposeKeyFactory and ComposeKeyInstance.
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
 * $Id: scim_compose_key.cpp,v 1.7 2005/08/16 07:26:54 suzhe Exp $
 *
 */

#define Uses_SCIM_COMPOSE_KEY
#define Uses_C_CTYPE

#include "scim_private.h"
#include "scim.h"

#define SCIM_KEYBOARD_ICON_FILE            (SCIM_ICONDIR "/keyboard.png")

namespace scim {

#define SCIM_MAX_COMPOSE_LEN 5

struct ComposeSequence
{
    uint32 keys [SCIM_MAX_COMPOSE_LEN];
    ucs4_t unicode;
};

class ComposeSequenceLessByKeys
{
public:
    bool operator () (const ComposeSequence &lhs, const ComposeSequence &rhs) const {
        for (size_t i = 0; i < SCIM_MAX_COMPOSE_LEN; ++i) {
            if (lhs.keys [i] < rhs.keys [i]) return true;
            else if (lhs.keys [i] > rhs.keys [i]) return false;
        }
        return false;
    }

    bool operator () (const ComposeSequence &lhs, const uint32 *rhs) const {
        for (size_t i = 0; i < SCIM_MAX_COMPOSE_LEN; ++i) {
            if (lhs.keys [i] < rhs [i]) return true;
            else if (lhs.keys [i] > rhs [i]) return false;
        }
        return false;
    }

    bool operator () (const uint32 *lhs, const ComposeSequence &rhs) const {
        for (size_t i = 0; i < SCIM_MAX_COMPOSE_LEN; ++i) {
            if (lhs [i] < rhs.keys [i]) return true;
            else if (lhs [i] > rhs.keys [i]) return false;
        }
        return false;
    }

    bool operator () (const uint32 *lhs, const uint32 *rhs) const {
        for (size_t i = 0; i < SCIM_MAX_COMPOSE_LEN; ++i) {
            if (lhs [i] < rhs [i]) return true;
            else if (lhs [i] > rhs [i]) return false;
        }
        return false;
    }

};

// Generated from /usr/X11R6/lib/X11/locale/en_US.UTF-8/Compose
// Get rid off all keys with unicode value.
// Merged with the table in gtk+2.x
static const ComposeSequence __scim_compose_seqs[] = {
#include "scim_compose_key_data.h"
};

#define SCIM_NUM_COMPOSE_SEQS (sizeof (__scim_compose_seqs) / sizeof (__scim_compose_seqs [0]))

static uint16 __scim_compose_ignores [] = {
    SCIM_KEY_ISO_Level3_Shift,
    SCIM_KEY_ISO_Group_Shift,
    SCIM_KEY_Mode_switch,
    SCIM_KEY_Shift_L,
    SCIM_KEY_Shift_R,
    SCIM_KEY_Control_L,
    SCIM_KEY_Control_R,
    SCIM_KEY_Caps_Lock,
    SCIM_KEY_Shift_Lock,
    SCIM_KEY_Meta_L,
    SCIM_KEY_Meta_R,
    SCIM_KEY_Alt_L,
    SCIM_KEY_Alt_R,
    SCIM_KEY_Super_L,
    SCIM_KEY_Super_R,
    SCIM_KEY_Hyper_L,
    SCIM_KEY_Hyper_R
};

#define SCIM_NUM_COMPOSE_IGNORES (sizeof (__scim_compose_ignores) / sizeof (__scim_compose_ignores [0]))

ComposeKeyFactory::ComposeKeyFactory ()
{
    set_locales ("C");
}

ComposeKeyFactory::~ComposeKeyFactory ()
{
}

WideString
ComposeKeyFactory::get_name () const
{
    return utf8_mbstowcs (_("English/European"));
}

WideString
ComposeKeyFactory::get_authors () const
{
    return utf8_mbstowcs ("James Su <suzhe@tsinghua.org.cn>");
}

WideString
ComposeKeyFactory::get_credits () const
{
    return WideString ();
}

WideString
ComposeKeyFactory::get_help () const
{
    return WideString ();
}

String
ComposeKeyFactory::get_uuid () const
{
    return String (SCIM_COMPOSE_KEY_FACTORY_UUID);
}

String
ComposeKeyFactory::get_icon_file () const
{
    return String (SCIM_KEYBOARD_ICON_FILE);
}

bool
ComposeKeyFactory::validate_encoding (const String& encoding) const
{
    return true;
}

bool
ComposeKeyFactory::validate_locale (const String& locale) const
{
    return true;
}

IMEngineInstancePointer
ComposeKeyFactory::create_instance (const String& encoding, int id)
{
    return new ComposeKeyInstance (this, encoding, id);
}

ComposeKeyInstance::ComposeKeyInstance (ComposeKeyFactory *factory,
                                        const String& encoding,
                                        int id)
    : IMEngineInstanceBase (factory, encoding, id)
{
    m_compose_buffer [0] = m_compose_buffer [1] = m_compose_buffer [2] = m_compose_buffer [3] = 0;
    m_compose_buffer [4] = m_compose_buffer [5] = m_compose_buffer [6] = m_compose_buffer [7] = 0;
}

ComposeKeyInstance::~ComposeKeyInstance ()
{
}

bool
ComposeKeyInstance::process_key_event (const KeyEvent& key)
{
    if (key.is_key_release ()) return false;

    // Ignore modifier key presses.
    if (std::binary_search (__scim_compose_ignores, __scim_compose_ignores + SCIM_NUM_COMPOSE_IGNORES, (uint16) key.code))
        return false;

    // Ignore the key if ctrl or alt is down.
    if (key.is_control_down () || key.is_alt_down ())
        return false;

    int n_compose = 0;

    while (m_compose_buffer [n_compose] != 0 && n_compose < SCIM_MAX_COMPOSE_LEN)
        ++ n_compose;

    // The buffer is full, then reset the buffer first.
    if (n_compose == SCIM_MAX_COMPOSE_LEN) {
        reset ();
        n_compose = 0;
    }

    m_compose_buffer [n_compose] = (uint32) key.code;

    const ComposeSequence *it = std::lower_bound (__scim_compose_seqs,
                                                  __scim_compose_seqs + SCIM_NUM_COMPOSE_SEQS,
                                                  m_compose_buffer,
                                                  ComposeSequenceLessByKeys ());

    // Not result found, reset the buffer and return false.
    if (it == __scim_compose_seqs + SCIM_NUM_COMPOSE_SEQS) {
        reset ();
        return false;
    }

    // Check if the compose sequence is match.
    for (n_compose = 0; n_compose < SCIM_MAX_COMPOSE_LEN; ++ n_compose) {
        if (m_compose_buffer [n_compose] == 0)
            break;

        // Not match, reset the buffer and return.
        // If it's the first key press, then return false to forward it.
        // Otherwise return true to ignore it.
        if (m_compose_buffer [n_compose] != it->keys [n_compose]) {
            reset ();
            return n_compose != 0;
        }
    }

    // Match exactly, commit the result.
    if (n_compose == SCIM_MAX_COMPOSE_LEN || it->keys [n_compose] == 0) {
        WideString wstr;
        wstr.push_back (it->unicode);
        commit_string (wstr);
        reset ();
    }

    return true;
}

void
ComposeKeyInstance::move_preedit_caret (unsigned int /*pos*/)
{
}

void
ComposeKeyInstance::select_candidate (unsigned int /*item*/)
{
}

void
ComposeKeyInstance::update_lookup_table_page_size (unsigned int /*page_size*/)
{
}

void
ComposeKeyInstance::lookup_table_page_up ()
{
}

void
ComposeKeyInstance::lookup_table_page_down ()
{
}

void
ComposeKeyInstance::reset ()
{
    m_compose_buffer [0] = m_compose_buffer [1] = m_compose_buffer [2] = m_compose_buffer [3] = 0;
    m_compose_buffer [4] = m_compose_buffer [5] = m_compose_buffer [6] = m_compose_buffer [7] = 0;
}

void
ComposeKeyInstance::focus_in ()
{
    register_properties (PropertyList ());
    reset ();
}

void
ComposeKeyInstance::focus_out ()
{
}

void
ComposeKeyInstance::trigger_property (const String & /*property*/)
{
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
