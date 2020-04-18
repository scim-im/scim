/**
 * @file scim_ibus_types.h
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
 * $Id: scim_ibus_types.h,v 1.56 2005/06/26 16:35:12 suzhe Exp $
 */

#if !defined (__SCIM_IBUS_TYPES_H)
#define __SCIM_IBUS_TYPES_H

class IBusInputContext;
typedef scim::Pointer <IBusInputContext> IBusInputContextPointer;

class IBusInputContextObserver
{
public:
    virtual void input_context_destroy(IBusInputContext *ic) = 0;
    virtual void input_context_capability_updated(IBusInputContext *ic) = 0;
    virtual void input_context_focus_in(IBusInputContext *ic) = 0;
    virtual void input_context_focus_out(IBusInputContext *ic) = 0;
    virtual void input_context_reset(IBusInputContext *ic) = 0;
    virtual void input_context_cursor_location_updated(IBusInputContext *ic) = 0;
    virtual bool input_context_process_key_event(IBusInputContext *ic,
                                                 uint32_t keyval,
                                                 uint32_t keycode,
                                                 uint32_t state) = 0;
};

/**
 * Following type definitions were copyed from ibustypes.h
 */

/**
 * IBusModifierType:
 * @IBUS_SHIFT_MASK: Shift  is activated.
 * @IBUS_LOCK_MASK: Cap Lock is locked.
 * @IBUS_CONTROL_MASK: Control key is activated.
 * @IBUS_MOD1_MASK: Modifier 1 (Usually Alt_L (0x40),  Alt_R (0x6c),  Meta_L (0xcd)) activated.
 * @IBUS_MOD2_MASK: Modifier 2 (Usually Num_Lock (0x4d)) activated.
 * @IBUS_MOD3_MASK: Modifier 3 activated.
 * @IBUS_MOD4_MASK: Modifier 4 (Usually Super_L (0xce),  Hyper_L (0xcf)) activated.
 * @IBUS_MOD5_MASK: Modifier 5 (ISO_Level3_Shift (0x5c),  Mode_switch (0xcb)) activated.
 * @IBUS_BUTTON1_MASK: Mouse button 1 (left) is activated.
 * @IBUS_BUTTON2_MASK: Mouse button 2 (middle) is activated.
 * @IBUS_BUTTON3_MASK: Mouse button 3 (right) is activated.
 * @IBUS_BUTTON4_MASK: Mouse button 4 (scroll up) is activated.
 * @IBUS_BUTTON5_MASK: Mouse button 5 (scroll down) is activated.
 * @IBUS_HANDLED_MASK: Handled mask indicates the event has been handled by ibus.
 * @IBUS_FORWARD_MASK: Forward mask indicates the event has been forward from ibus.
 * @IBUS_IGNORED_MASK: It is an alias of IBUS_FORWARD_MASK.
 * @IBUS_SUPER_MASK: Super (Usually Win) key is activated.
 * @IBUS_HYPER_MASK: Hyper key is activated.
 * @IBUS_META_MASK: Meta key is activated.
 * @IBUS_RELEASE_MASK: Key is released.
 * @IBUS_MODIFIER_MASK: Modifier mask for the all the masks above.
 *
 * Handles key modifier such as control, shift and alt and release event.
 * Note that nits 15 - 25 are currently unused, while bit 29 is used internally.
 */
typedef enum
{
    IBUS_SHIFT_MASK    = 1 << 0,
    IBUS_LOCK_MASK     = 1 << 1,
    IBUS_CONTROL_MASK  = 1 << 2,
    IBUS_MOD1_MASK     = 1 << 3,
    IBUS_MOD2_MASK     = 1 << 4,
    IBUS_MOD3_MASK     = 1 << 5,
    IBUS_MOD4_MASK     = 1 << 6,
    IBUS_MOD5_MASK     = 1 << 7,
    IBUS_BUTTON1_MASK  = 1 << 8,
    IBUS_BUTTON2_MASK  = 1 << 9,
    IBUS_BUTTON3_MASK  = 1 << 10,
    IBUS_BUTTON4_MASK  = 1 << 11,
    IBUS_BUTTON5_MASK  = 1 << 12,

    /* The next few modifiers are used by XKB, so we skip to the end.
     * Bits 15 - 23 are currently unused. Bit 29 is used internally.
     */

    /* ibus mask */
    IBUS_HANDLED_MASK  = 1 << 24,
    IBUS_FORWARD_MASK  = 1 << 25,
    IBUS_IGNORED_MASK  = IBUS_FORWARD_MASK,

    IBUS_SUPER_MASK    = 1 << 26,
    IBUS_HYPER_MASK    = 1 << 27,
    IBUS_META_MASK     = 1 << 28,

    IBUS_RELEASE_MASK  = 1 << 30,

    IBUS_MODIFIER_MASK = 0x5f001fff
} IBusModifierType;

/**
 * IBusCapabilite:
 * @IBUS_CAP_PREEDIT_TEXT: UI is capable to show pre-edit text.
 * @IBUS_CAP_AUXILIARY_TEXT: UI is capable to show auxiliary text.
 * @IBUS_CAP_LOOKUP_TABLE: UI is capable to show the lookup table.
 * @IBUS_CAP_FOCUS: UI is capable to get focus.
 * @IBUS_CAP_PROPERTY: UI is capable to have property.
 * @IBUS_CAP_SURROUNDING_TEXT: Client can provide surround text,
 *  or IME can handle surround text.
 *
 * Capability flags of UI.
 */
typedef enum {
    IBUS_CAP_PREEDIT_TEXT       = 1 << 0,
    IBUS_CAP_AUXILIARY_TEXT     = 1 << 1,
    IBUS_CAP_LOOKUP_TABLE       = 1 << 2,
    IBUS_CAP_FOCUS              = 1 << 3,
    IBUS_CAP_PROPERTY           = 1 << 4,
    IBUS_CAP_SURROUNDING_TEXT   = 1 << 5,
} IBusCapabilite;

/**
 * IBusPreeditFocusMode:
 * @IBUS_ENGINE_PREEDIT_CLEAR: pre-edit text is cleared.
 * @IBUS_ENGINE_PREEDIT_COMMIT: pre-edit text is committed.
 *
 * Pre-edit commit mode when the focus is lost.
 */
typedef enum {
    IBUS_ENGINE_PREEDIT_CLEAR   = 0,
    IBUS_ENGINE_PREEDIT_COMMIT  = 1,
} IBusPreeditFocusMode;

/**
 * IBusOrientation:
 * @IBUS_ORIENTATION_HORIZONTAL: Horizontal orientation.
 * @IBUS_ORIENTATION_VERTICAL: Vertival orientation.
 * @IBUS_ORIENTATION_SYSTEM: Use ibus global orientation setup.
 *
 * Orientation of UI.
 */
typedef enum {
    IBUS_ORIENTATION_HORIZONTAL = 0,
    IBUS_ORIENTATION_VERTICAL   = 1,
    IBUS_ORIENTATION_SYSTEM     = 2,
} IBusOrientation;

/**
 * IBusInputPurpose:
 * @IBUS_INPUT_PURPOSE_FREE_FORM: Allow any character
 * @IBUS_INPUT_PURPOSE_ALPHA: Allow only alphabetic characters
 * @IBUS_INPUT_PURPOSE_DIGITS: Allow only digits
 * @IBUS_INPUT_PURPOSE_NUMBER: Edited field expects numbers
 * @IBUS_INPUT_PURPOSE_PHONE: Edited field expects phone number
 * @IBUS_INPUT_PURPOSE_URL: Edited field expects URL
 * @IBUS_INPUT_PURPOSE_EMAIL: Edited field expects email address
 * @IBUS_INPUT_PURPOSE_NAME: Edited field expects the name of a person
 * @IBUS_INPUT_PURPOSE_PASSWORD: Like @IBUS_INPUT_PURPOSE_FREE_FORM,
 * but characters are hidden
 * @IBUS_INPUT_PURPOSE_PIN: Like @IBUS_INPUT_PURPOSE_DIGITS, but
 * characters are hidden
 *
 * Describes primary purpose of the input context.  This information
 * is particularly useful to implement intelligent behavior in
 * engines, such as automatic input-mode switch and text prediction.
 *
 * This enumeration may be extended in the future; engines should
 * interpret unknown values as 'free form'.
 */
typedef enum
{
  IBUS_INPUT_PURPOSE_FREE_FORM,
  IBUS_INPUT_PURPOSE_ALPHA,
  IBUS_INPUT_PURPOSE_DIGITS,
  IBUS_INPUT_PURPOSE_NUMBER,
  IBUS_INPUT_PURPOSE_PHONE,
  IBUS_INPUT_PURPOSE_URL,
  IBUS_INPUT_PURPOSE_EMAIL,
  IBUS_INPUT_PURPOSE_NAME,
  IBUS_INPUT_PURPOSE_PASSWORD,
  IBUS_INPUT_PURPOSE_PIN
} IBusInputPurpose;

/**
 * IBusInputHints:
 * @IBUS_INPUT_HINT_NONE: No special behaviour suggested
 * @IBUS_INPUT_HINT_SPELLCHECK: Suggest checking for typos
 * @IBUS_INPUT_HINT_NO_SPELLCHECK: Suggest not checking for typos
 * @IBUS_INPUT_HINT_WORD_COMPLETION: Suggest word completion
 * @IBUS_INPUT_HINT_LOWERCASE: Suggest to convert all text to lowercase
 * @IBUS_INPUT_HINT_UPPERCASE_CHARS: Suggest to capitalize all text
 * @IBUS_INPUT_HINT_UPPERCASE_WORDS: Suggest to capitalize the first
 *     character of each word
 * @IBUS_INPUT_HINT_UPPERCASE_SENTENCES: Suggest to capitalize the
 *     first word of each sentence
 * @IBUS_INPUT_HINT_INHIBIT_OSK: Suggest to not show an onscreen keyboard
 *     (e.g for a calculator that already has all the keys).
 * @IBUS_INPUT_HINT_VERTICAL_WRITING: The text is vertical.
 *
 * Describes hints that might be taken into account by engines.  Note
 * that engines may already tailor their behaviour according to the
 * #IBusInputPurpose of the entry.
 */
typedef enum
{
  IBUS_INPUT_HINT_NONE                = 0,
  IBUS_INPUT_HINT_SPELLCHECK          = 1 << 0,
  IBUS_INPUT_HINT_NO_SPELLCHECK       = 1 << 1,
  IBUS_INPUT_HINT_WORD_COMPLETION     = 1 << 2,
  IBUS_INPUT_HINT_LOWERCASE           = 1 << 3,
  IBUS_INPUT_HINT_UPPERCASE_CHARS     = 1 << 4,
  IBUS_INPUT_HINT_UPPERCASE_WORDS     = 1 << 5,
  IBUS_INPUT_HINT_UPPERCASE_SENTENCES = 1 << 6,
  IBUS_INPUT_HINT_INHIBIT_OSK         = 1 << 7,
  IBUS_INPUT_HINT_VERTICAL_WRITING    = 1 << 8
} IBusInputHints;

typedef enum {
    IBUS_ATTR_TYPE_UNDERLINE    = 1,
    IBUS_ATTR_TYPE_FOREGROUND   = 2,
    IBUS_ATTR_TYPE_BACKGROUND   = 3,
} IBusAttrType;

typedef enum {
    IBUS_ATTR_UNDERLINE_NONE    = 0,
    IBUS_ATTR_UNDERLINE_SINGLE  = 1,
    IBUS_ATTR_UNDERLINE_DOUBLE  = 2,
    IBUS_ATTR_UNDERLINE_LOW     = 3,
    IBUS_ATTR_UNDERLINE_ERROR   = 4,
} IBusAttrUnderline;

struct IBusAttribute {
    uint32_t type;
    uint32_t value;
    uint32_t start_index;
    uint32_t end_index;
};

typedef enum {
    PROP_TYPE_NORMAL = 0,
    PROP_TYPE_TOGGLE = 1,
    PROP_TYPE_RADIO = 2,
    PROP_TYPE_MENU = 3,
    PROP_TYPE_SEPARATOR = 4,
} IBusPropType;

typedef enum {
    PROP_STATE_UNCHECKED = 0,
    PROP_STATE_CHECKED = 1,
    PROP_STATE_INCONSISTENT = 2,
} IBusPropState;

#endif // __SCIM_IBUS_TYPES_H

/*
vi:ts=4:nowrap:ai:expandtab
*/
