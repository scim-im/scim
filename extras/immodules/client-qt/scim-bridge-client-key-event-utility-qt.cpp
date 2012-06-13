/*
 * SCIM Bridge
 *
 * Copyright (c) 2006 Ryo Dairiki <ryo-dairiki@users.sourceforge.net>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation and 
 * appearing in the file LICENSE.LGPL included in the package of this file.
 * You can also redistribute it and/or modify it under the terms of 
 * the GNU General Public License as published by the Free Software Foundation and 
 * appearing in the file LICENSE.GPL included in the package of this file.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <sys/time.h>

#include <cstdlib>
#include <map>

#ifdef QT4
#include <QApplication>
#include <QChar>
#include <QEvent>
#include <QKeyEvent>
#else
#include <qapplication.h>
#include <qkeycode.h>
#endif

#include "scim-bridge-output.h"
#include "scim-bridge-client-key-event-utility-qt.h"

using std::map;

/* Static variables */
static bool initialized = false;

static map<int, scim_bridge_key_code_t> qt_to_bridge_key_map;
static map<scim_bridge_key_code_t, int> bridge_to_qt_key_map;


/* Helper functions */
static void register_key (int qt_key_code, scim_bridge_key_code_t bridge_key_code)
{
    qt_to_bridge_key_map[qt_key_code] = bridge_key_code;
    qt_to_bridge_key_map[bridge_key_code] = qt_key_code;
}


static void static_initialize ()
{
    if (initialized) return;

    register_key ('/', SCIM_BRIDGE_KEY_CODE_KP_Divide);
    register_key ('*', SCIM_BRIDGE_KEY_CODE_KP_Multiply);
    register_key ('-', SCIM_BRIDGE_KEY_CODE_KP_Subtract);
    register_key ('+', SCIM_BRIDGE_KEY_CODE_KP_Add);
    register_key (Qt::Key_Return, SCIM_BRIDGE_KEY_CODE_KP_Enter);
    register_key (Qt::Key_Escape, SCIM_BRIDGE_KEY_CODE_Escape);
    register_key (Qt::Key_Tab, SCIM_BRIDGE_KEY_CODE_Tab);
    register_key (Qt::Key_Backtab, SCIM_BRIDGE_KEY_CODE_ISO_Left_Tab);
    register_key (Qt::Key_Backspace, SCIM_BRIDGE_KEY_CODE_BackSpace);
    register_key (Qt::Key_Return, SCIM_BRIDGE_KEY_CODE_Return);
    register_key (Qt::Key_Enter, SCIM_BRIDGE_KEY_CODE_KP_Enter);
    register_key (Qt::Key_Insert, SCIM_BRIDGE_KEY_CODE_Insert);
    register_key (Qt::Key_Delete, SCIM_BRIDGE_KEY_CODE_Delete);
    register_key (Qt::Key_Pause, SCIM_BRIDGE_KEY_CODE_Pause);
#ifdef sun
    register_key (Qt::Key_Print, SCIM_BRIDGE_KEY_CODE_F22);
#else
    register_key (Qt::Key_Print, SCIM_BRIDGE_KEY_CODE_Print);
#endif
    register_key (Qt::Key_SysReq, SCIM_BRIDGE_KEY_CODE_Sys_Req);
    register_key (Qt::Key_Home, SCIM_BRIDGE_KEY_CODE_Home);
    register_key (Qt::Key_End, SCIM_BRIDGE_KEY_CODE_End);
    register_key (Qt::Key_Left, SCIM_BRIDGE_KEY_CODE_Left);
    register_key (Qt::Key_Up, SCIM_BRIDGE_KEY_CODE_Up);
    register_key (Qt::Key_Right, SCIM_BRIDGE_KEY_CODE_Right);
    register_key (Qt::Key_Down, SCIM_BRIDGE_KEY_CODE_Down);
#ifdef QT4
    register_key (Qt::Key_PageUp, SCIM_BRIDGE_KEY_CODE_Prior);
    register_key (Qt::Key_PageUp, SCIM_BRIDGE_KEY_CODE_Next);
#else
    register_key (Qt::Key_Prior, SCIM_BRIDGE_KEY_CODE_Prior);
    register_key (Qt::Key_Next, SCIM_BRIDGE_KEY_CODE_Next);
#endif
    register_key (Qt::Key_CapsLock, SCIM_BRIDGE_KEY_CODE_Caps_Lock);
    register_key (Qt::Key_NumLock, SCIM_BRIDGE_KEY_CODE_Num_Lock);
    register_key (Qt::Key_ScrollLock, SCIM_BRIDGE_KEY_CODE_Scroll_Lock);
    register_key (Qt::Key_F1, SCIM_BRIDGE_KEY_CODE_F1);
    register_key (Qt::Key_F2, SCIM_BRIDGE_KEY_CODE_F2);
    register_key (Qt::Key_F3, SCIM_BRIDGE_KEY_CODE_F3);
    register_key (Qt::Key_F4, SCIM_BRIDGE_KEY_CODE_F4);
    register_key (Qt::Key_F5, SCIM_BRIDGE_KEY_CODE_F5);
    register_key (Qt::Key_F6, SCIM_BRIDGE_KEY_CODE_F6);
    register_key (Qt::Key_F7, SCIM_BRIDGE_KEY_CODE_F7);
    register_key (Qt::Key_F8, SCIM_BRIDGE_KEY_CODE_F8);
    register_key (Qt::Key_F9, SCIM_BRIDGE_KEY_CODE_F9);
    register_key (Qt::Key_F10, SCIM_BRIDGE_KEY_CODE_F10);
    register_key (Qt::Key_F11, SCIM_BRIDGE_KEY_CODE_F11);
    register_key (Qt::Key_F12, SCIM_BRIDGE_KEY_CODE_F12);
    register_key (Qt::Key_F13, SCIM_BRIDGE_KEY_CODE_F13);
    register_key (Qt::Key_F14, SCIM_BRIDGE_KEY_CODE_F14);
    register_key (Qt::Key_F15, SCIM_BRIDGE_KEY_CODE_F15);
    register_key (Qt::Key_F16, SCIM_BRIDGE_KEY_CODE_F16);
    register_key (Qt::Key_F17, SCIM_BRIDGE_KEY_CODE_F17);
    register_key (Qt::Key_F18, SCIM_BRIDGE_KEY_CODE_F18);
    register_key (Qt::Key_F19, SCIM_BRIDGE_KEY_CODE_F19);
    register_key (Qt::Key_F20, SCIM_BRIDGE_KEY_CODE_F20);
    register_key (Qt::Key_F21, SCIM_BRIDGE_KEY_CODE_F21);
    register_key (Qt::Key_F22, SCIM_BRIDGE_KEY_CODE_F22);
    register_key (Qt::Key_F23, SCIM_BRIDGE_KEY_CODE_F23);
    register_key (Qt::Key_F24, SCIM_BRIDGE_KEY_CODE_F24);
    register_key (Qt::Key_F25, SCIM_BRIDGE_KEY_CODE_F25);
    register_key (Qt::Key_F26, SCIM_BRIDGE_KEY_CODE_F26);
    register_key (Qt::Key_F27, SCIM_BRIDGE_KEY_CODE_F27);
    register_key (Qt::Key_F28, SCIM_BRIDGE_KEY_CODE_F28);
    register_key (Qt::Key_F29, SCIM_BRIDGE_KEY_CODE_F29);
    register_key (Qt::Key_F30, SCIM_BRIDGE_KEY_CODE_F30);
    register_key (Qt::Key_F31, SCIM_BRIDGE_KEY_CODE_F31);
    register_key (Qt::Key_F32, SCIM_BRIDGE_KEY_CODE_F32);
    register_key (Qt::Key_F33, SCIM_BRIDGE_KEY_CODE_F33);
    register_key (Qt::Key_F34, SCIM_BRIDGE_KEY_CODE_F34);
    register_key (Qt::Key_F35, SCIM_BRIDGE_KEY_CODE_F35);
    register_key (Qt::Key_Super_L, SCIM_BRIDGE_KEY_CODE_Super_L);
    register_key (Qt::Key_Super_R, SCIM_BRIDGE_KEY_CODE_Super_R);
    register_key (Qt::Key_Menu, SCIM_BRIDGE_KEY_CODE_Menu);
    register_key (Qt::Key_Hyper_L, SCIM_BRIDGE_KEY_CODE_Hyper_L);
    register_key (Qt::Key_Hyper_R, SCIM_BRIDGE_KEY_CODE_Hyper_R);
    register_key (Qt::Key_Help, SCIM_BRIDGE_KEY_CODE_Help);
    register_key (Qt::Key_Multi_key, SCIM_BRIDGE_KEY_CODE_Multi_key);
    register_key (Qt::Key_Codeinput, SCIM_BRIDGE_KEY_CODE_Codeinput);
    register_key (Qt::Key_SingleCandidate, SCIM_BRIDGE_KEY_CODE_SingleCandidate);
    register_key (Qt::Key_MultipleCandidate, SCIM_BRIDGE_KEY_CODE_MultipleCandidate);
    register_key (Qt::Key_PreviousCandidate	, SCIM_BRIDGE_KEY_CODE_PreviousCandidate);
    register_key (Qt::Key_Mode_switch, SCIM_BRIDGE_KEY_CODE_Mode_switch);
    register_key (Qt::Key_Kanji, SCIM_BRIDGE_KEY_CODE_Kanji);
    register_key (Qt::Key_Muhenkan, SCIM_BRIDGE_KEY_CODE_Muhenkan);
    register_key (Qt::Key_Henkan, SCIM_BRIDGE_KEY_CODE_Henkan);
    register_key (Qt::Key_Romaji, SCIM_BRIDGE_KEY_CODE_Romaji);
    register_key (Qt::Key_Hiragana, SCIM_BRIDGE_KEY_CODE_Hiragana);
    register_key (Qt::Key_Katakana, SCIM_BRIDGE_KEY_CODE_Katakana);
    register_key (Qt::Key_Hiragana_Katakana, SCIM_BRIDGE_KEY_CODE_Hiragana_Katakana);
    register_key (Qt::Key_Zenkaku, SCIM_BRIDGE_KEY_CODE_Zenkaku);
    register_key (Qt::Key_Hankaku, SCIM_BRIDGE_KEY_CODE_Hankaku);
    register_key (Qt::Key_Zenkaku_Hankaku, SCIM_BRIDGE_KEY_CODE_Zenkaku_Hankaku);
    register_key (Qt::Key_Touroku, SCIM_BRIDGE_KEY_CODE_Touroku);
    register_key (Qt::Key_Massyo, SCIM_BRIDGE_KEY_CODE_Massyo);
    register_key (Qt::Key_Kana_Lock, SCIM_BRIDGE_KEY_CODE_Kana_Lock);
    register_key (Qt::Key_Kana_Shift, SCIM_BRIDGE_KEY_CODE_Kana_Shift);
    register_key (Qt::Key_Eisu_Shift, SCIM_BRIDGE_KEY_CODE_Eisu_Shift);
    register_key (Qt::Key_Eisu_toggle, SCIM_BRIDGE_KEY_CODE_Eisu_toggle);
    register_key (Qt::Key_Hangul, SCIM_BRIDGE_KEY_CODE_Hangul);
    register_key (Qt::Key_Hangul_Start, SCIM_BRIDGE_KEY_CODE_Hangul_Start);
    register_key (Qt::Key_Hangul_End, SCIM_BRIDGE_KEY_CODE_Hangul_End);
    register_key (Qt::Key_Hangul_Hanja, SCIM_BRIDGE_KEY_CODE_Hangul_Hanja);
    register_key (Qt::Key_Hangul_Jamo, SCIM_BRIDGE_KEY_CODE_Hangul_Jamo);
    register_key (Qt::Key_Hangul_Romaja, SCIM_BRIDGE_KEY_CODE_Hangul_Romaja);
    register_key (Qt::Key_Hangul_Jeonja, SCIM_BRIDGE_KEY_CODE_Hangul_Jeonja);
    register_key (Qt::Key_Hangul_Banja, SCIM_BRIDGE_KEY_CODE_Hangul_Banja);
    register_key (Qt::Key_Hangul_PreHanja, SCIM_BRIDGE_KEY_CODE_Hangul_PreHanja);
    register_key (Qt::Key_Hangul_Special, SCIM_BRIDGE_KEY_CODE_Hangul_Special);

    initialized = true;
}


/* Implementations */
QKeyEvent *scim_bridge_key_event_bridge_to_qt (const ScimBridgeKeyEvent *bridge_key_event)
{
    if (!initialized) static_initialize ();
    
    const QEvent::Type type = scim_bridge_key_event_is_pressed (bridge_key_event) ? QEvent::KeyPress : QEvent::KeyRelease;
    
    const scim_bridge_key_code_t bridge_key_code = scim_bridge_key_event_get_code (bridge_key_event);

    unsigned int ascii_code = '\0';
    unsigned int qt_key_code;
    if (bridge_key_code < 0x1000) {
        if (bridge_key_code >= SCIM_BRIDGE_KEY_CODE_a && bridge_key_code <= SCIM_BRIDGE_KEY_CODE_z) {
            ascii_code = bridge_key_code;
#ifdef QT4
            qt_key_code = QChar (ascii_code).toUpper ().toAscii ();
#else
            qt_key_code = QChar (ascii_code).upper ();
#endif
        } else {
            ascii_code = bridge_key_code;
            qt_key_code = bridge_key_code;
        }
    } else if (bridge_key_code < 0x3000) {
#ifdef Q_WS_WIN
        qt_key_code = bridge_key_code;
    } else {
        qt_key_code = Key_unknown;
    }
#else
        qt_key_code = bridge_key_code | Qt::UNICODE_ACCEL;
    } else {
        map<scim_bridge_key_code_t, int>::iterator iter = bridge_to_qt_key_map.find (bridge_key_code);
        if (iter != bridge_to_qt_key_map.end ()) {
            qt_key_code = iter->second;
        } else {
            qt_key_code = Qt::Key_unknown;
        }
    }
#endif

#ifdef QT4
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;

    if (scim_bridge_key_event_is_alt_down (bridge_key_event)) modifiers |= Qt::AltModifier;
    if (scim_bridge_key_event_is_shift_down (bridge_key_event)) modifiers |= Qt::ShiftModifier;
    if (scim_bridge_key_event_is_control_down (bridge_key_event)) modifiers |= Qt::ControlModifier;
    if (scim_bridge_key_event_is_meta_down (bridge_key_event)) modifiers |= Qt::MetaModifier;

    return new QKeyEvent (type, qt_key_code, modifiers);
#else
    unsigned int modifiers = 0;

    if (scim_bridge_key_event_is_alt_down (bridge_key_event)) modifiers |= Qt::AltButton;
    if (scim_bridge_key_event_is_shift_down (bridge_key_event)) modifiers |= Qt::ShiftButton;
    if (scim_bridge_key_event_is_control_down (bridge_key_event)) modifiers |= Qt::ControlButton;
    if (scim_bridge_key_event_is_meta_down (bridge_key_event)) modifiers |= Qt::MetaButton;

    return new QKeyEvent (type, qt_key_code, ascii_code, modifiers);
#endif
}


ScimBridgeKeyEvent *scim_bridge_key_event_qt_to_bridge (const QKeyEvent *key_event)
{
    if (!initialized) static_initialize ();
    
    ScimBridgeKeyEvent *bridge_key_event = scim_bridge_alloc_key_event ();

#ifdef QT4
    const Qt::KeyboardModifiers modifiers = key_event->modifiers ();

    if (modifiers & Qt::ShiftModifier) {
        scim_bridge_key_event_set_shift_down (bridge_key_event, TRUE);
    }
    if (modifiers & Qt::ControlModifier) {
        scim_bridge_key_event_set_control_down (bridge_key_event, TRUE);
    }
    if (modifiers & Qt::AltModifier) {
        scim_bridge_key_event_set_alt_down (bridge_key_event, TRUE);
    }
    if (modifiers & Qt::MetaModifier) {
        scim_bridge_key_event_set_meta_down (bridge_key_event, TRUE);
    }
#else
    const int modifiers = key_event->state ();

    if (modifiers & Qt::ShiftButton) {
        scim_bridge_key_event_set_shift_down (bridge_key_event, TRUE);
    }
    if (modifiers & Qt::ControlButton) {
        scim_bridge_key_event_set_control_down (bridge_key_event, TRUE);
    }
    if (modifiers & Qt::AltButton) {
        scim_bridge_key_event_set_alt_down (bridge_key_event, TRUE);
    }
    if (modifiers & Qt::MetaButton) {
        scim_bridge_key_event_set_meta_down (bridge_key_event, TRUE);
    }
#endif

    const int qt_key_code = key_event->key ();
    int bridge_key_code;
    
    #ifdef SCIM_QT_IMMODULE_USE_KDE

    KKeyNative nk = KKey (qt_key_code);
    bridge_key_code = nk.sym ();

    #else
    
    if ((qt_key_code & Qt::UNICODE_ACCEL) || (qt_key_code & 0xffff) < 0x1000) {
        const QString qt_key_raw_str = QString (QChar (qt_key_code));
        const QString qt_key_str = key_event->text ();
        
        if ((qt_key_str == qt_key_raw_str) ^ scim_bridge_key_event_is_shift_down (bridge_key_event)) {
            scim_bridge_pdebugln (5, "CapsLock: on");
        	scim_bridge_key_event_set_caps_lock_down (bridge_key_event, true);
        } else {
            scim_bridge_pdebugln (5, "CapsLock: off");
            scim_bridge_key_event_set_caps_lock_down (bridge_key_event, false);
        }
        
        if (!scim_bridge_key_event_is_caps_lock_down (bridge_key_event) ^ scim_bridge_key_event_is_shift_down (bridge_key_event)) {
#ifdef QT4
	        bridge_key_code = QChar (qt_key_code).toLower ().unicode ();
#else
	        bridge_key_code = QChar (qt_key_code).lower ().unicode ();
#endif
	    } else {
#ifdef QT4
	        bridge_key_code = QChar (qt_key_code).toUpper ().unicode ();
#else
	        bridge_key_code = QChar (qt_key_code).upper ().unicode ();
#endif
	    }
    } else {
        map<int, scim_bridge_key_code_t>::iterator iter = qt_to_bridge_key_map.find (qt_key_code);
        if (iter != qt_to_bridge_key_map.end ()) {
            bridge_key_code = iter->second;
        } else {
            bridge_key_code = SCIM_BRIDGE_KEY_CODE_NullKey;
        }
    }

    #endif

    scim_bridge_key_event_set_code (bridge_key_event, bridge_key_code);
    scim_bridge_key_event_set_pressed (bridge_key_event, key_event->type () != QEvent::KeyRelease);

    return bridge_key_event;
}


#ifdef Q_WS_X11
XEvent *scim_bridge_key_event_bridge_to_x11 (const ScimBridgeKeyEvent *bridge_key_event, Display *display, WId window_id)
{
    XEvent *x_event = static_cast<XEvent*> (malloc (sizeof (XEvent)));

    XKeyEvent *x_key_event = &x_event->xkey;
    
    x_key_event->type = scim_bridge_key_event_is_pressed (bridge_key_event) ? XKeyPress : XKeyRelease;
    x_key_event->display = display;
    x_key_event->window = window_id;
    x_key_event->subwindow = window_id;
    x_key_event->serial = 0;
    x_key_event->send_event = FALSE;
    x_key_event->same_screen = FALSE;

    struct timeval current_time;
    gettimeofday (&current_time, NULL);
    x_key_event->time = (current_time.tv_sec * 1000) + (current_time.tv_usec / 1000);
    
    if (display != NULL) {
        x_key_event->root = DefaultRootWindow (display);
        x_key_event->keycode = XKeysymToKeycode (display, (KeySym) scim_bridge_key_event_get_code (bridge_key_event));
    } else {
        x_key_event->root = None;
        x_key_event->keycode = SCIM_BRIDGE_KEY_CODE_NullKey;
    }

    x_key_event->state = 0;
    if (scim_bridge_key_event_is_shift_down (bridge_key_event)) x_key_event->state |= ShiftMask;
    if (scim_bridge_key_event_is_control_down (bridge_key_event)) x_key_event->state |= ControlMask;
    if (scim_bridge_key_event_is_caps_lock_down (bridge_key_event)) x_key_event->state |= LockMask;
    if (scim_bridge_key_event_is_alt_down (bridge_key_event)) x_key_event->state |= Mod1Mask;
    if (scim_bridge_key_event_is_meta_down (bridge_key_event)) x_key_event->state |= Mod4Mask;
    
    return x_event;
}


ScimBridgeKeyEvent* scim_bridge_key_event_x11_to_bridge (const XEvent *x_event)
{
    const XKeyEvent *x_key_event = &x_event->xkey;
    
    const int key_strlen = 32;
    char key_str[key_strlen];
    KeySym keysym;
   
    if (XLookupString (const_cast <XKeyEvent*> (&x_event->xkey), key_str, key_strlen, &keysym, 0) <= 0) {
        keysym = XLookupKeysym (const_cast <XKeyEvent*> (&x_event->xkey), 0);
    }
     
    ScimBridgeKeyEvent *bridge_key_event = scim_bridge_alloc_key_event ();
    scim_bridge_key_event_set_code (bridge_key_event, keysym);

    if (x_key_event->type == XKeyRelease) {
        scim_bridge_key_event_set_pressed (bridge_key_event, false);
    } else {
        scim_bridge_key_event_set_pressed (bridge_key_event, true);
    }

    if (x_key_event->state & ShiftMask || (x_key_event->type == XKeyPress && (x_key_event->keycode == XK_Shift_L || x_key_event->keycode == XK_Shift_R))) {
        scim_bridge_key_event_set_shift_down (bridge_key_event, TRUE);
    }
    if (x_key_event->state & ControlMask || (x_key_event->type == XKeyPress && (x_key_event->keycode == XK_Control_L || x_key_event->keycode == XK_Control_R))) {
        scim_bridge_key_event_set_control_down (bridge_key_event, TRUE);
    }
    if (x_key_event->state & LockMask || (x_key_event->type == XKeyPress && (x_key_event->keycode == XK_Caps_Lock))) {
        scim_bridge_key_event_set_caps_lock_down (bridge_key_event, TRUE);
    }
    if (x_key_event->state & Mod1Mask || (x_key_event->type == XKeyPress && (x_key_event->keycode == XK_Alt_L || x_key_event->keycode == XK_Alt_R))) {
        scim_bridge_key_event_set_alt_down (bridge_key_event, TRUE);
    }
    if (x_key_event->state & Mod4Mask || (x_key_event->type == XKeyPress && (x_key_event->keycode == XK_Meta_L || x_key_event->keycode == XK_Meta_R))) {
        scim_bridge_key_event_set_meta_down (bridge_key_event, TRUE);
    }

    if (scim_bridge_key_event_get_code (bridge_key_event) == SCIM_BRIDGE_KEY_CODE_backslash) {
        boolean kana_ro = FALSE;
        int keysym_size = 0;
        KeySym *keysyms = XGetKeyboardMapping (x_key_event->display, x_key_event->keycode, 1, &keysym_size);
        if (keysyms != NULL) {
            kana_ro = (keysyms[0] == XK_backslash && keysyms[1] == XK_underscore);
            XFree (keysyms);
        }
        scim_bridge_key_event_set_quirk_enabled (bridge_key_event, SCIM_BRIDGE_KEY_QUIRK_KANA_RO, kana_ro);
    }
    
    return bridge_key_event;
}

#endif
