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
 * $Id: scim_event.cpp,v 1.29 2005/07/13 08:54:55 suzhe Exp $
 */

#define Uses_SCIM_EVENT
#define Uses_SCIM_GLOBAL_CONFIG
#define Uses_SCIM_CONFIG_PATH
#define Uses_C_STDIO
#define Uses_C_STRING
#include "scim_private.h"
#include "scim.h"
#include "scim_stl_map.h"

namespace scim {

struct __Uint16Pair
{
    uint16 first;
    uint16 second;
};

struct __KeyCodeMap
{
    size_t        size;
    __Uint16Pair *map;
};

struct __KeyName
{
    uint16  value;
    const char   *name;
};

class __Uint16PairLessByFirst
{
public:
    bool operator ()(const __Uint16Pair &lhs, const __Uint16Pair &rhs) const {
        return lhs.first < rhs.first;
    }

    bool operator ()(const __Uint16Pair &lhs, const uint16 &rhs) const {
        return lhs.first < rhs;
    }

    bool operator ()(const uint16 &lhs, const __Uint16Pair &rhs) const {
        return lhs < rhs.first;
    }
};

class __KeyNameLessByCode
{
public:
    bool operator ()(const __KeyName &lhs, const __KeyName &rhs) const {
        return lhs.value < rhs.value;
    }

    bool operator ()(const __KeyName &lhs, const uint16 &rhs) const {
        return lhs.value < rhs;
    }

    bool operator ()(const uint16 &lhs, const __KeyName &rhs) const {
        return lhs < rhs.value;
    }
};

class __KeyNameLessByName
{
public:
    bool operator ()(const __KeyName &lhs, const __KeyName &rhs) const {
        return strcmp (lhs.name, rhs.name) < 0;
    }

    bool operator ()(const __KeyName &lhs, const char *rhs) const {
        return strcmp (lhs.name, rhs) < 0;
    }

    bool operator ()(const char *lhs, const __KeyName &rhs) const {
        return strcmp (lhs, rhs.name) < 0;
    }
};

// Keyboard Layout data
#include "scim_keyboard_layout_data.h"

// KeyEvent data
#include "scim_keyevent_data.h"

char
KeyEvent::get_ascii_code () const
{
    if (code >= SCIM_KEY_space && code <= SCIM_KEY_asciitilde)
        return (char) code;

    if (code >= SCIM_KEY_KP_0 && code <= SCIM_KEY_KP_9)
        return (char) (code - SCIM_KEY_KP_0 + SCIM_KEY_0);

    if (code == SCIM_KEY_Return)
        return 0x0d;
    if (code == SCIM_KEY_Linefeed)
        return 0x0a;
    if (code == SCIM_KEY_Tab)
        return 0x09;
    if (code == SCIM_KEY_BackSpace)
        return 0x08;
    if (code == SCIM_KEY_Escape)
        return 0x1b;

    return 0;
}

ucs4_t
KeyEvent::get_unicode_code () const
{
    /* First check for Latin-1 characters (1:1 mapping) */
    if ((code >= 0x0020 && code <= 0x007e) ||
        (code >= 0x00a0 && code <= 0x00ff))
        return code;

    /* Also check for directly encoded 24-bit UCS characters:
     */
    if ((code & 0xff000000) == 0x01000000)
        return code & 0x00ffffff;

    /* Translation of KP_Decimal depends on locale.
    if (code == SCIM_KP_Decimal)
        return get_decimal_char ();
    */

    /* Invalid keyevent code. */
    if (code > 0xFFFF)
        return 0;

    /* binary search in table */
    __Uint16Pair * it = std::lower_bound (__scim_key_to_unicode_tab,
                                          __scim_key_to_unicode_tab + SCIM_NUM_KEY_UNICODES,
                                          (uint16) code,
                                          __Uint16PairLessByFirst ());

    if (it != __scim_key_to_unicode_tab + SCIM_NUM_KEY_UNICODES && it->first == (uint16) code)
        return it->second;

    /* No matching Unicode value found */
    return 0;
}

String
KeyEvent::get_key_string () const
{
    size_t i;
    String maskstr;
    String codestr;
    uint16 mask_skip = 0;

    for (i=0; i < SCIM_NUM_KEY_MASKS; ++i) {
        if ((__scim_key_mask_names [i].value & mask) && !(__scim_key_mask_names [i].value & mask_skip)) {
            if (maskstr.length ())
                maskstr += (String ("+") + String (__scim_key_mask_names [i].name));
            else
                maskstr += String (__scim_key_mask_names [i].name);
        }
        mask_skip |= __scim_key_mask_names [i].value;
    }

    if (code == 0xFFFFFF) {
        codestr = String ("VoidSymbol");
    } else if (code <= 0xFFFF){
        __KeyName *it = std::lower_bound (__scim_keys_by_code,
                                          __scim_keys_by_code + SCIM_NUM_KEY_NAMES,
                                          (uint16) code,
                                          __KeyNameLessByCode ());

        if (it != __scim_keys_by_code + SCIM_NUM_KEY_NAMES && it->value == code)
            codestr = String (it->name);
    }

    if (!codestr.length () && code) {
        char buf [20];
        snprintf (buf, 20, ((code <= 0xFFFF) ? "0x%04x" : "0x%06x"), code);
        codestr = String (buf);
    }

    if (maskstr.length () && codestr.length ())
        return maskstr + String ("+") + codestr;
    if (maskstr.length())
        return maskstr;
    if (codestr.length ())
        return codestr;

    return String ();
}

static uint16
inline __remap_keycode (uint16 from, const __KeyCodeMap &map)
{
    if (map.size == 0) return from;

    __Uint16Pair *it = std::lower_bound (map.map, map.map + map.size, from, __Uint16PairLessByFirst ());

    if (it != map.map + map.size && it->first == from)
        return it->second;

    return from;
}

KeyEvent
KeyEvent::map_to_layout (KeyboardLayout new_layout) const
{
    if (layout == SCIM_KEYBOARD_Unknown || new_layout == SCIM_KEYBOARD_Unknown || layout == new_layout ||
        layout >= SCIM_KEYBOARD_NUM_LAYOUTS || new_layout >= SCIM_KEYBOARD_NUM_LAYOUTS || new_layout < 0 ||
        code > 0xFFFF)
        return *this;

    KeyEvent evt (code, mask, new_layout);

    uint16 new_code = (uint16) code;

    switch (mask & (SCIM_KEY_CapsLockMask | SCIM_KEY_ShiftMask)) {
        case 0:
            new_code = __remap_keycode (new_code, __normal_map [layout]);
            new_code = __remap_keycode (new_code, __normal_invert_map [new_layout]);
            break;
        case SCIM_KEY_CapsLockMask:
            new_code = __remap_keycode (new_code, __caps_map [layout]);
            new_code = __remap_keycode (new_code, __caps_invert_map [new_layout]);
            break;
        case SCIM_KEY_ShiftMask:
            new_code = __remap_keycode (new_code, __shift_map [layout]);
            new_code = __remap_keycode (new_code, __shift_invert_map [new_layout]);
            break;
        case (SCIM_KEY_ShiftMask | SCIM_KEY_CapsLockMask):
            new_code = __remap_keycode (new_code, __caps_shift_map [layout]);
            new_code = __remap_keycode (new_code, __caps_shift_invert_map [new_layout]);
            break;
    }

    evt.code = (uint32) new_code;

    return evt;
}

bool
scim_key_to_string (String &str, const KeyEvent & key)
{
    str = key.get_key_string ();
    return str.length () != 0;
}

bool
scim_string_to_key (KeyEvent &key, const String & str)
{
    std::vector <String> list;
    bool skip;
    size_t i;

    key.code = 0;
    key.mask = 0;

    scim_split_string_list (list, str, '+');

    for (std::vector <String>::iterator it=list.begin (); it!=list.end (); ++it) {
        skip = false;
        for (i = 0; i < SCIM_NUM_KEY_MASKS; ++i) {
            if (*it == String (__scim_key_mask_names [i].name)) {
                key.mask |= __scim_key_mask_names [i].value;
                skip = true;
                break;
            }
        }

        if (skip) continue;

        __KeyName *p = std::lower_bound (__scim_keys_by_name,
                                         __scim_keys_by_name + SCIM_NUM_KEY_NAMES,
                                         it->c_str (),
                                         __KeyNameLessByName ());

        if (p != __scim_keys_by_name + SCIM_NUM_KEY_NAMES && strcmp (p->name, it->c_str ()) == 0) {
            key.code = p->value;
        } else if (it->length () >= 6 && (*it)[0] == '0' && ((*it)[1] == 'x' || (*it)[1] == 'X')){
            key.code = strtol (it->c_str () + 2, NULL, 16);
        } else if (strcmp (p->name, "VoidSymbol") == 0) {
            key.code = SCIM_KEY_VoidSymbol; 
        }
    }

    return key.code != 0;
}

bool
scim_key_list_to_string (String &str, const std::vector<KeyEvent> & keylist)
{
    std::vector<String> strlist;

    for (std::vector<KeyEvent>::const_iterator it = keylist.begin (); it != keylist.end (); ++it) {
        if (scim_key_to_string (str, *it))
            strlist.push_back (str);
    }

    str = scim_combine_string_list (strlist, ',');

    return str.length () != 0;
}

bool
scim_string_to_key_list (std::vector<KeyEvent> &keylist, const String &str)
{
    std::vector <String> strlist;
    scim_split_string_list (strlist, str, ',');

    keylist.clear ();

    for (std::vector <String>::iterator it = strlist.begin (); it != strlist.end (); ++it) {
        KeyEvent key;
        if (scim_string_to_key (key, *it))
            keylist.push_back (key);
    }
    return keylist.size () > 0;
}


String
scim_keyboard_layout_to_string (KeyboardLayout layout)
{
    if (layout >= 0 && layout < SCIM_KEYBOARD_NUM_LAYOUTS)
        return String (__scim_keyboard_layout_ids_by_code [layout].name);

    return String (__scim_keyboard_layout_ids_by_code [0].name);
}

KeyboardLayout
scim_string_to_keyboard_layout (const String &str)
{
    if (str == __scim_keyboard_layout_ids_by_code [0].name) return SCIM_KEYBOARD_Unknown;
    if (str == __scim_keyboard_layout_ids_by_code [1].name || str == String ("US") || str == String ("Default")) return SCIM_KEYBOARD_Default;

    __KeyName *it =
            std::lower_bound (__scim_keyboard_layout_ids_by_name + 2,
                              __scim_keyboard_layout_ids_by_name + SCIM_KEYBOARD_NUM_LAYOUTS,
                              str.c_str (),
                              __KeyNameLessByName());

    if (it != __scim_keyboard_layout_ids_by_name + SCIM_KEYBOARD_NUM_LAYOUTS && strcmp (it->name, str.c_str ()) == 0)
        return static_cast <KeyboardLayout> (it->value);

    return SCIM_KEYBOARD_Unknown;
}

String
scim_keyboard_layout_get_display_name (KeyboardLayout layout)
{
    if (layout >= 0 && layout < SCIM_KEYBOARD_NUM_LAYOUTS)
        return String (_(__scim_keyboard_layout_names [layout]));

    return String (_(__scim_keyboard_layout_names [0]));
}

KeyboardLayout
scim_get_default_keyboard_layout ()
{
    String layout_name (__scim_keyboard_layout_ids_by_code [0].name);
    layout_name = scim_global_config_read (SCIM_GLOBAL_CONFIG_DEFAULT_KEYBOARD_LAYOUT, layout_name);

    return scim_string_to_keyboard_layout (layout_name);
}

void
scim_set_default_keyboard_layout (KeyboardLayout layout)
{
    String layout_name = scim_keyboard_layout_to_string (layout);
    scim_global_config_write (SCIM_GLOBAL_CONFIG_DEFAULT_KEYBOARD_LAYOUT, layout_name);
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
