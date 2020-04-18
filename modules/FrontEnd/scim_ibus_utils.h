/** @file scim_ibus_utils.h
 * definition of IBUSUTILS related classes.
 */

/* 
 * Smart Common Input Method
 * 
 * Copyright (c) 2002-2005 James Su <suzhe@tsinghua.org.cn>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Publutils
 * Lutilsense as published by the Free Software Foundation; either
 * version 2 of the Lutilsense, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTUTILSULAR PURPOSE.  See the
 * GNU Lesser General Publutils Lutilsense for more details.
 *
 * You should have received a copy of the GNU Lesser General Publutils
 * Lutilsense along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 *
 * $Id: scim_ibus_utils.h,v 1.10 2005/06/26 16:35:12 suzhe Exp $
 */

#if !defined (__SCIM_IBUS_UTILS_H)
#define __SCIM_IBUS_UTILS_H

#define LOG_LEVEL_TRACE             1
#define LOG_LEVEL_DEBUG             2
#define LOG_LEVEL_INFO              3
#define LOG_LEVEL_WARN              4
#define LOG_LEVEL_ERROR             5
#define LOG_LEVEL_FATAL             6
#define log_level_enabled(l)        ((l) >= LOG_LEVEL)
#define log_print(level, fmt, ...)  ({                                                              \
    int l = (level);                                                                                \
    if (log_level_enabled(l)) {                                                                     \
        static const char *names[] = { "TRC", "DBG", "INF", "WRN", "ERR", "FAL" };                  \
        fprintf(stderr, "%s " __FILE__ ":%-4d " fmt "\n", names[(l)], __LINE__, ##__VA_ARGS__);     \
    }                                                                                               \
})
#define log_trace(fmt, ...)          log_print(LOG_LEVEL_TRACE, fmt, ##__VA_ARGS__)
#define log_debug(fmt, ...)          log_print(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define log_info(fmt, ...)           log_print(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...)           log_print(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define log_error(fmt, ...)          log_print(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define log_fatal(fmt, ...)          log_print(LOG_LEVEL_FATAL, fmt, ##__VA_ARGS__)
#define log_func()                   log_trace("%s", __FUNCTION__)
#define log_func_not_impl(...)       ({ log_error("%s not implement yet", __FUNCTION__); return __VA_ARGS__; })
#define log_func_incomplete(...)     ({ log_error("%s implementation incomplete", __FUNCTION__); return __VA_ARGS__; })
#define log_func_ignored(...)        ({ log_trace("%s ignored", __FUNCTION__); return __VA_ARGS__; })
#ifndef LOG_LEVEL
#define LOG_LEVEL                   LOG_LEVEL_INFO
#endif

#ifndef SD_BUS_METHOD_WITH_NAMES
#define SD_BUS_METHOD_WITH_NAMES(_member, _signature, _in_names, _result, _out_names, _handler, _flags) \
    SD_BUS_METHOD(_member, _signature, _result, _handler, _flags)
#endif

#ifndef SD_BUS_SIGNAL_WITH_NAMES
#define SD_BUS_SIGNAL_WITH_NAMES(_member, _signature, _names, _flags) \
    SD_BUS_SIGNAL(_member, _signature, _flags)
#endif

#ifndef SD_BUS_PARAM
#define SD_BUS_PARAM(name) {}
#endif

static inline uint16_t
scim_ibus_keystate_to_scim_keymask (uint32_t state)
{
    uint16_t mask = 0;

    if (state & IBUS_SHIFT_MASK) {
        mask |= scim::SCIM_KEY_ShiftMask;
    }

    if (state & IBUS_LOCK_MASK) {
        mask |= scim::SCIM_KEY_CapsLockMask;
    }

    if (state & IBUS_CONTROL_MASK) {
        mask |= scim::SCIM_KEY_ControlMask;
    }

    if (state & IBUS_MOD1_MASK) {
        mask |= scim::SCIM_KEY_AltMask;
    }

    if (state & IBUS_META_MASK) {
        mask |= scim::SCIM_KEY_MetaMask;
    }

    if (state & IBUS_SUPER_MASK) {
        mask |= scim::SCIM_KEY_SuperMask;
    }

    if (state & IBUS_HYPER_MASK) {
        mask |= scim::SCIM_KEY_HyperMask;
    }

    if (state & IBUS_MOD2_MASK) {
        mask |= scim::SCIM_KEY_NumLockMask;
    }

    if (state & IBUS_RELEASE_MASK) {
        mask |= SCIM_KEY_ReleaseMask;
    }

    return mask;
}

static inline uint16_t
scim_keymask_to_ibus_keystate (uint32_t mask)
{
    uint16_t state = 0;

    if (mask & SCIM_KEY_ShiftMask) {
        mask |= IBUS_SHIFT_MASK;
    }

    if (mask & SCIM_KEY_CapsLockMask) {
        mask |= IBUS_LOCK_MASK;
    }

    if (mask & SCIM_KEY_ControlMask) {
        mask |= IBUS_CONTROL_MASK;
    }

    if (mask & SCIM_KEY_AltMask) {
        mask |= IBUS_MOD1_MASK;
    }

    if (mask & SCIM_KEY_MetaMask) {
        mask |= IBUS_MOD2_MASK;
    }

    if (mask & SCIM_KEY_SuperMask) {
        mask |= IBUS_SUPER_MASK;
    }

    if (mask & SCIM_KEY_HyperMask) {
        mask |= IBUS_HYPER_MASK;
    }

    if (mask & SCIM_KEY_NumLockMask) {
        mask |= IBUS_MOD5_MASK;
    }

    if (mask & SCIM_KEY_ReleaseMask) {
        mask |= IBUS_RELEASE_MASK;
    }

    return state;
}

static inline scim::KeyEvent
scim_ibus_keyevent_to_scim_keyevent (KeyboardLayout layout,
                                     uint32_t keyval,
                                     uint32_t keycode,
                                     uint32_t state)
{
     return KeyEvent (keyval,
                      scim_ibus_keystate_to_scim_keymask (state),
                      layout);

//    if (scimkey.code == SCIM_KEY_backslash) {
//        int keysym_size = 0;
//        KeySym *keysyms = XGetKeyboardMapping (display, xkey.keycode, 1, &keysym_size);
//        if (keysyms != NULL) {
//            if (keysyms[0] == XK_backslash &&
//		(keysym_size > 1 && keysyms[1] == XK_underscore))
//                scimkey.mask |= SCIM_KEY_QuirkKanaRoMask;
//            XFree (keysyms);
//        }
//    }
}

static bool scim_attr_to_ibus_attr (const Attribute &src, IBusAttribute &dest)
{
    log_func ();

    switch (src.get_type()) {
        case SCIM_ATTR_DECORATE:
            dest.type = IBUS_ATTR_TYPE_UNDERLINE;
            switch (src.get_value ()) {
                case SCIM_ATTR_DECORATE_UNDERLINE:
                    dest.value = IBUS_ATTR_UNDERLINE_SINGLE;
                    break;
                case SCIM_ATTR_DECORATE_HIGHLIGHT:
//                    dest.value = IBUS_ATTR_UNDERLINE_DOUBLE;
                    dest.type = IBUS_ATTR_TYPE_BACKGROUND;
                    dest.value = 0xc8c8f0;
                    break;
                case SCIM_ATTR_DECORATE_REVERSE:
                    dest.value = IBUS_ATTR_UNDERLINE_ERROR;
                    break;
                default:
                    return false;
            }
            break;
        case SCIM_ATTR_FOREGROUND:
            dest.type = IBUS_ATTR_TYPE_FOREGROUND;
            dest.value = src.get_value ();
            break;
        case SCIM_ATTR_BACKGROUND:
            dest.type = IBUS_ATTR_TYPE_BACKGROUND;
            dest.value = src.get_value ();
            break;
        default:
            return false;
    }

    dest.start_index = src.get_start ();
    dest.end_index = src.get_end ();

    return true;
}

#if log_level_enabled(LOG_LEVEL_DEBUG)
static inline std::string ibus_caps_to_str(uint32_t caps)
{
    std::ostringstream ss;
    if (caps & IBUS_CAP_PREEDIT_TEXT) {
        ss << "preedit-text|";
    }
    if (caps & IBUS_CAP_AUXILIARY_TEXT) {
        ss << "aux-text|";
    }
    if (caps & IBUS_CAP_LOOKUP_TABLE) {
        ss << "lookup-table|";
    }
    if (caps & IBUS_CAP_FOCUS) {
        ss << "focus|";
    }
    if (caps & IBUS_CAP_PROPERTY) {
        ss << "property|";
    }
    if (caps & IBUS_CAP_SURROUNDING_TEXT) {
        ss << "surrounding-text|";
    }

    std::string s = ss.str();
    if (s.size() > 0) {
        s.resize(s.size() - 1);
    }

    return s;
}

static const char *scim_attr_list_to_str (const AttributeList &attrs, char *buf, size_t buf_size)
{
    buf[0] = '\0';
    char *p = buf;

    AttributeList::const_iterator it = attrs.begin ();
    for (; it != attrs.end (); ++ it) {
        switch (it->get_type ()) {
            case SCIM_ATTR_DECORATE:
                p += snprintf (p, buf_size - (p - buf),
                               "Attr{type=Decorate, decorate=%s, start=%d, end=%d}, ",
                               it->get_value () == SCIM_ATTR_DECORATE_NONE
                                   ? "None"
                                   : it->get_value () == SCIM_ATTR_DECORATE_UNDERLINE
                                       ? "Underline"
                                       : it->get_value () == SCIM_ATTR_DECORATE_HIGHLIGHT
                                           ? "Highlight"
                                           : "Reverse",
                               it->get_start (),
                               it->get_end ());
                break;
            case SCIM_ATTR_FOREGROUND:
                p += snprintf (p, buf_size - (p - buf),
                               "Attr{type=Foreground, color=(%x,%x,%x), start=%d, end=%d}, ",
                               SCIM_RGB_COLOR_RED(it->get_value ()),
                               SCIM_RGB_COLOR_GREEN(it->get_value ()),
                               SCIM_RGB_COLOR_BLUE(it->get_value ()),
                               it->get_start (),
                               it->get_end ());
                break;
            case SCIM_ATTR_BACKGROUND:
                p += snprintf (p, buf_size - (p - buf),
                               "Attr{type=Background, color=(%x,%x,%x), start=%d, end=%d}, ",
                               SCIM_RGB_COLOR_RED(it->get_value ()),
                               SCIM_RGB_COLOR_GREEN(it->get_value ()),
                               SCIM_RGB_COLOR_BLUE(it->get_value ()),
                               it->get_start (),
                               it->get_end ());
                break;
        }
    }

    if (p - buf > 2) {
        *(p - 2) = '\0';
    }

    return buf;
}

static inline std::string
scim_caps_to_str (uint32_t caps)
{
    std::ostringstream ss;

    if (caps & SCIM_CLIENT_CAP_ONTHESPOT_PREEDIT)
    {
        ss << "onthespo-preedit|";
    }
    if (caps & SCIM_CLIENT_CAP_SINGLE_LEVEL_PROPERTY)
    {
        ss << "single-level-prop|";
    }
    if (caps & SCIM_CLIENT_CAP_MULTI_LEVEL_PROPERTY)
    {
        ss << "multi-level-prop|";
    }
    if (caps & SCIM_CLIENT_CAP_TRIGGER_PROPERTY)
    {
        ss << "trigger-prop|";
    }
    if (caps & SCIM_CLIENT_CAP_HELPER_MODULE)
    {
        ss << "helper-module|";
    }
    if (caps & SCIM_CLIENT_CAP_SURROUNDING_TEXT)
    {
        ss << "surrounding-text|";
    }

    std::string s = ss.str();
    if (s.size() > 0) {
        s.resize(s.size() - 1);
    }

    return s;
}

#else
static inline std::string
ibus_caps_to_str(uint32_t caps)
{
    return "";
}

static inline std::string
scim_caps_to_str (uint32_t caps)
{
    return "";
}

static inline const char *
scim_attr_list_to_str (const AttributeList &attrs,
                                          char *buf,
                                          size_t buf_size)
{
    return "";
}
#endif

#endif // _SCIM_IBUS_UTILS_H

/*
vi:ts=4:nowrap:ai:expandtab
*/
