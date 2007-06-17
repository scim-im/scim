/**
 * @file scim_event.h
 * @brief Defines the scim::KeyEvent class and related enums, functions.
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
 * $Id: scim_event.h,v 1.26.2.1 2007/04/11 15:24:10 suzhe Exp $
 */

#ifndef __SCIM_EVENT_H
#define __SCIM_EVENT_H

namespace scim {

/**
 * @addtogroup Accessories
 * @{
 */

/**
 * @brief Enum values of all valid key masks.
 *
 * The key masks indicate which modifier keys
 * is pressed down during the keyboard hit.
 *
 * The special SCIM_KEY_ReleaseMask indicates
 * the key release event.
 *
 */
enum KeyMask
{
    SCIM_KEY_NullMask       = 0,            /**< Key press event without modifier key. */
    SCIM_KEY_ShiftMask      = (1<<0),       /**< The Shift key is pressed down */
    SCIM_KEY_CapsLockMask   = (1<<1),       /**< The CapsLock key is pressed down */
    SCIM_KEY_ControlMask    = (1<<2),       /**< The Control key is pressed down */
    SCIM_KEY_AltMask        = (1<<3),       /**< The Alt  key is pressed down */
    SCIM_KEY_MetaMask       = (1<<4),       /**< The Meta key is pressed down */
    SCIM_KEY_SuperMask      = (1<<5),       /**< The Super key is pressed down */
    SCIM_KEY_HyperMask      = (1<<6),       /**< The Hyper key is pressed down */
    SCIM_KEY_NumLockMask    = (1<<7),       /**< The NumLock key is pressed down */

    SCIM_KEY_LockMask       = SCIM_KEY_CapsLockMask, /**< For backward API compatibility, do not use it in new code */
    SCIM_KEY_Mod1Mask       = SCIM_KEY_AltMask,      /**< For backward API compatibility, do not use it in new code */
    SCIM_KEY_Mod2Mask       = SCIM_KEY_MetaMask,     /**< For backward API compatibility, do not use it in new code */
    SCIM_KEY_Mod3Mask       = SCIM_KEY_SuperMask,    /**< For backward API compatibility, do not use it in new code */
    SCIM_KEY_Mod4Mask       = SCIM_KEY_HyperMask,    /**< For backward API compatibility, do not use it in new code */ 
    SCIM_KEY_Mod5Mask       = SCIM_KEY_NumLockMask,  /**< For backward API compatibility, do not use it in new code */ 
    SCIM_KEY_ScrollLockMask = 0,                     /**< For backward API compatibility, do not use it in new code */ 

    SCIM_KEY_QuirkKanaRoMask= (1<<14),      /**< Special mask to distinguish two backslash keys on jp106 keyboard */

    SCIM_KEY_ReleaseMask    = (1<<15),      /**< It's a key release event */
    SCIM_KEY_AllMasks       = 0xC0FF        /**< All valid Masks */
};

/**
 * @brief Enum values of all valid key codes.
 *
 * If keycode & 0xff000000 == 0x01000000 then this key code is directly encoded 24-bit UCS character.
 * The UCS value is keycode & 0x00ffffff.
 */
enum KeyCode
{
    SCIM_KEY_NullKey                            = 0,
    SCIM_KEY_VoidSymbol                         = 0xFFFFFF,

    /* function keys */
    SCIM_KEY_BackSpace                          = 0xFF08,    /* back space, back char */
    SCIM_KEY_Tab                                = 0xFF09,
    SCIM_KEY_Linefeed                           = 0xFF0A,    /* Linefeed, LF */
    SCIM_KEY_Clear                              = 0xFF0B,
    SCIM_KEY_Return                             = 0xFF0D,    /* Return, enter */
    SCIM_KEY_Pause                              = 0xFF13,    /* Pause, hold */
    SCIM_KEY_Scroll_Lock                        = 0xFF14,
    SCIM_KEY_Sys_Req                            = 0xFF15,
    SCIM_KEY_Escape                             = 0xFF1B,
    SCIM_KEY_Delete                             = 0xFFFF,    /* Delete, rubout */

    /* International & multi-key character composition */
    SCIM_KEY_Multi_key                          = 0xFF20,    /* Multi-key character compose */
    SCIM_KEY_Codeinput                          = 0xFF37,
    SCIM_KEY_SingleCandidate                    = 0xFF3C,
    SCIM_KEY_MultipleCandidate                  = 0xFF3D,
    SCIM_KEY_PreviousCandidate                  = 0xFF3E,

    /* Japanese keyboard support */
    SCIM_KEY_Kanji                              = 0xFF21,    /* Kanji, Kanji convert */
    SCIM_KEY_Muhenkan                           = 0xFF22,    /* Cancel Conversion */
    SCIM_KEY_Henkan_Mode                        = 0xFF23,    /* Start/Stop Conversion */
    SCIM_KEY_Henkan                             = 0xFF23,    /* Alias for Henkan_Mode */
    SCIM_KEY_Romaji                             = 0xFF24,    /* to Romaji */
    SCIM_KEY_Hiragana                           = 0xFF25,    /* to Hiragana */
    SCIM_KEY_Katakana                           = 0xFF26,    /* to Katakana */
    SCIM_KEY_Hiragana_Katakana                  = 0xFF27, /* Hiragana/Katakana toggle */
    SCIM_KEY_Zenkaku                            = 0xFF28,    /* to Zenkaku */
    SCIM_KEY_Hankaku                            = 0xFF29,    /* to Hankaku */
    SCIM_KEY_Zenkaku_Hankaku                    = 0xFF2A, /* Zenkaku/Hankaku toggle */
    SCIM_KEY_Touroku                            = 0xFF2B,    /* Add to Dictionary */
    SCIM_KEY_Massyo                             = 0xFF2C,    /* Delete from Dictionary */
    SCIM_KEY_Kana_Lock                          = 0xFF2D,    /* Kana Lock */
    SCIM_KEY_Kana_Shift                         = 0xFF2E,    /* Kana Shift */
    SCIM_KEY_Eisu_Shift                         = 0xFF2F,    /* Alphanumeric Shift */
    SCIM_KEY_Eisu_toggle                        = 0xFF30,    /* Alphanumeric toggle */
    SCIM_KEY_Kanji_Bangou                       = 0xFF37,   /* Codeinput */
    SCIM_KEY_Zen_Koho                           = 0xFF3D,    /* Multiple/All Candidate(s) */
    SCIM_KEY_Mae_Koho                           = 0xFF3E,    /* Previous Candidate */

    /* Cursor control & motion */
    SCIM_KEY_Home                               = 0xFF50,
    SCIM_KEY_Left                               = 0xFF51,    /* Move left, left arrow */
    SCIM_KEY_Up                                 = 0xFF52,    /* Move up, up arrow */
    SCIM_KEY_Right                              = 0xFF53,    /* Move right, right arrow */
    SCIM_KEY_Down                               = 0xFF54,    /* Move down, down arrow */
    SCIM_KEY_Prior                              = 0xFF55,    /* Prior, previous */
    SCIM_KEY_Page_Up                            = 0xFF55,
    SCIM_KEY_Next                               = 0xFF56,    /* Next */
    SCIM_KEY_Page_Down                          = 0xFF56,
    SCIM_KEY_End                                = 0xFF57,    /* EOL */
    SCIM_KEY_Begin                              = 0xFF58,    /* BOL */

    /* Misc Functions */
    SCIM_KEY_Select                             = 0xFF60,    /* Select, mark */
    SCIM_KEY_Print                              = 0xFF61,
    SCIM_KEY_Execute                            = 0xFF62,    /* Execute, run, do */
    SCIM_KEY_Insert                             = 0xFF63,    /* Insert, insert here */
    SCIM_KEY_Undo                               = 0xFF65,    /* Undo, oops */
    SCIM_KEY_Redo                               = 0xFF66,    /* redo, again */
    SCIM_KEY_Menu                               = 0xFF67,
    SCIM_KEY_Find                               = 0xFF68,    /* Find, search */
    SCIM_KEY_Cancel                             = 0xFF69,    /* Cancel, stop, abort, exit */
    SCIM_KEY_Help                               = 0xFF6A,    /* Help */
    SCIM_KEY_Break                              = 0xFF6B,
    SCIM_KEY_Mode_switch                        = 0xFF7E,    /* Character set switch */
    SCIM_KEY_Num_Lock                           = 0xFF7F,

    /* keypad */
    SCIM_KEY_KP_Space                           = 0xFF80,    /* space */
    SCIM_KEY_KP_Tab                             = 0xFF89,
    SCIM_KEY_KP_Enter                           = 0xFF8D,    /* enter */
    SCIM_KEY_KP_F1                              = 0xFF91,    /* PF1, KP_A, ... */
    SCIM_KEY_KP_F2                              = 0xFF92,
    SCIM_KEY_KP_F3                              = 0xFF93,
    SCIM_KEY_KP_F4                              = 0xFF94,
    SCIM_KEY_KP_Home                            = 0xFF95,
    SCIM_KEY_KP_Left                            = 0xFF96,
    SCIM_KEY_KP_Up                              = 0xFF97,
    SCIM_KEY_KP_Right                           = 0xFF98,
    SCIM_KEY_KP_Down                            = 0xFF99,
    SCIM_KEY_KP_Prior                           = 0xFF9A,
    SCIM_KEY_KP_Page_Up                         = 0xFF9A,
    SCIM_KEY_KP_Next                            = 0xFF9B,
    SCIM_KEY_KP_Page_Down                       = 0xFF9B,
    SCIM_KEY_KP_End                             = 0xFF9C,
    SCIM_KEY_KP_Begin                           = 0xFF9D,
    SCIM_KEY_KP_Insert                          = 0xFF9E,
    SCIM_KEY_KP_Delete                          = 0xFF9F,
    SCIM_KEY_KP_Equal                           = 0xFFBD,   /* equals */
    SCIM_KEY_KP_Multiply                        = 0xFFAA,
    SCIM_KEY_KP_Add                             = 0xFFAB,
    SCIM_KEY_KP_Separator                       = 0xFFAC,   /* separator, often comma */
    SCIM_KEY_KP_Subtract                        = 0xFFAD,
    SCIM_KEY_KP_Decimal                         = 0xFFAE,
    SCIM_KEY_KP_Divide                          = 0xFFAF,

    SCIM_KEY_KP_0                               = 0xFFB0,
    SCIM_KEY_KP_1                               = 0xFFB1,
    SCIM_KEY_KP_2                               = 0xFFB2,
    SCIM_KEY_KP_3                               = 0xFFB3,
    SCIM_KEY_KP_4                               = 0xFFB4,
    SCIM_KEY_KP_5                               = 0xFFB5,
    SCIM_KEY_KP_6                               = 0xFFB6,
    SCIM_KEY_KP_7                               = 0xFFB7,
    SCIM_KEY_KP_8                               = 0xFFB8,
    SCIM_KEY_KP_9                               = 0xFFB9,

    /* Auxilliary Functions */
    SCIM_KEY_F1                                 = 0xFFBE,
    SCIM_KEY_F2                                 = 0xFFBF,
    SCIM_KEY_F3                                 = 0xFFC0,
    SCIM_KEY_F4                                 = 0xFFC1,
    SCIM_KEY_F5                                 = 0xFFC2,
    SCIM_KEY_F6                                 = 0xFFC3,
    SCIM_KEY_F7                                 = 0xFFC4,
    SCIM_KEY_F8                                 = 0xFFC5,
    SCIM_KEY_F9                                 = 0xFFC6,
    SCIM_KEY_F10                                = 0xFFC7,
    SCIM_KEY_F11                                = 0xFFC8,
    SCIM_KEY_F12                                = 0xFFC9,
    SCIM_KEY_F13                                = 0xFFCA,
    SCIM_KEY_F14                                = 0xFFCB,
    SCIM_KEY_F15                                = 0xFFCC,
    SCIM_KEY_F16                                = 0xFFCD,
    SCIM_KEY_F17                                = 0xFFCE,
    SCIM_KEY_F18                                = 0xFFCF,
    SCIM_KEY_F19                                = 0xFFD0,
    SCIM_KEY_F20                                = 0xFFD1,
    SCIM_KEY_F21                                = 0xFFD2,
    SCIM_KEY_F22                                = 0xFFD3,
    SCIM_KEY_F23                                = 0xFFD4,
    SCIM_KEY_F24                                = 0xFFD5,
    SCIM_KEY_F25                                = 0xFFD6,
    SCIM_KEY_F26                                = 0xFFD7,
    SCIM_KEY_F27                                = 0xFFD8,
    SCIM_KEY_F28                                = 0xFFD9,
    SCIM_KEY_F29                                = 0xFFDA,
    SCIM_KEY_F30                                = 0xFFDB,
    SCIM_KEY_F31                                = 0xFFDC,
    SCIM_KEY_F32                                = 0xFFDD,
    SCIM_KEY_F33                                = 0xFFDE,
    SCIM_KEY_F34                                = 0xFFDF,
    SCIM_KEY_F35                                = 0xFFE0,

    /* modifier keys */
    SCIM_KEY_Shift_L                            = 0xFFE1,    /* Left shift */
    SCIM_KEY_Shift_R                            = 0xFFE2,    /* Right shift */
    SCIM_KEY_Control_L                          = 0xFFE3,    /* Left control */
    SCIM_KEY_Control_R                          = 0xFFE4,    /* Right control */
    SCIM_KEY_Caps_Lock                          = 0xFFE5,    /* Caps lock */
    SCIM_KEY_Shift_Lock                         = 0xFFE6,    /* Shift lock */

    SCIM_KEY_Meta_L                             = 0xFFE7,    /* Left meta */
    SCIM_KEY_Meta_R                             = 0xFFE8,    /* Right meta */
    SCIM_KEY_Alt_L                              = 0xFFE9,    /* Left alt */
    SCIM_KEY_Alt_R                              = 0xFFEA,    /* Right alt */
    SCIM_KEY_Super_L                            = 0xFFEB,    /* Left super */
    SCIM_KEY_Super_R                            = 0xFFEC,    /* Right super */
    SCIM_KEY_Hyper_L                            = 0xFFED,    /* Left hyper */
    SCIM_KEY_Hyper_R                            = 0xFFEE,    /* Right hyper */

    /*
     * ISO 9995 Function and Modifier Keys
     * Byte 3 = 0xFE
     */

    SCIM_KEY_ISO_Lock                           = 0xFE01,
    SCIM_KEY_ISO_Level2_Latch                   = 0xFE02,
    SCIM_KEY_ISO_Level3_Shift                   = 0xFE03,
    SCIM_KEY_ISO_Level3_Latch                   = 0xFE04,
    SCIM_KEY_ISO_Level3_Lock                    = 0xFE05,
    SCIM_KEY_ISO_Group_Shift                    = 0xFF7E,    /* Alias for mode_switch */
    SCIM_KEY_ISO_Group_Latch                    = 0xFE06,
    SCIM_KEY_ISO_Group_Lock                     = 0xFE07,
    SCIM_KEY_ISO_Next_Group                     = 0xFE08,
    SCIM_KEY_ISO_Next_Group_Lock                = 0xFE09,
    SCIM_KEY_ISO_Prev_Group                     = 0xFE0A,
    SCIM_KEY_ISO_Prev_Group_Lock                = 0xFE0B,
    SCIM_KEY_ISO_First_Group                    = 0xFE0C,
    SCIM_KEY_ISO_First_Group_Lock               = 0xFE0D,
    SCIM_KEY_ISO_Last_Group                     = 0xFE0E,
    SCIM_KEY_ISO_Last_Group_Lock                = 0xFE0F,

    SCIM_KEY_ISO_Left_Tab                       = 0xFE20,
    SCIM_KEY_ISO_Move_Line_Up                   = 0xFE21,
    SCIM_KEY_ISO_Move_Line_Down                 = 0xFE22,
    SCIM_KEY_ISO_Partial_Line_Up                = 0xFE23,
    SCIM_KEY_ISO_Partial_Line_Down              = 0xFE24,
    SCIM_KEY_ISO_Partial_Space_Left             = 0xFE25,
    SCIM_KEY_ISO_Partial_Space_Right            = 0xFE26,
    SCIM_KEY_ISO_Set_Margin_Left                = 0xFE27,
    SCIM_KEY_ISO_Set_Margin_Right               = 0xFE28,
    SCIM_KEY_ISO_Release_Margin_Left            = 0xFE29,
    SCIM_KEY_ISO_Release_Margin_Right           = 0xFE2A,
    SCIM_KEY_ISO_Release_Both_Margins           = 0xFE2B,
    SCIM_KEY_ISO_Fast_Cursor_Left               = 0xFE2C,
    SCIM_KEY_ISO_Fast_Cursor_Right              = 0xFE2D,
    SCIM_KEY_ISO_Fast_Cursor_Up                 = 0xFE2E,
    SCIM_KEY_ISO_Fast_Cursor_Down               = 0xFE2F,
    SCIM_KEY_ISO_Continuous_Underline           = 0xFE30,
    SCIM_KEY_ISO_Discontinuous_Underline        = 0xFE31,
    SCIM_KEY_ISO_Emphasize                      = 0xFE32,
    SCIM_KEY_ISO_Center_Object                  = 0xFE33,
    SCIM_KEY_ISO_Enter                          = 0xFE34,

    SCIM_KEY_dead_grave                         = 0xFE50,
    SCIM_KEY_dead_acute                         = 0xFE51,
    SCIM_KEY_dead_circumflex                    = 0xFE52,
    SCIM_KEY_dead_tilde                         = 0xFE53,
    SCIM_KEY_dead_macron                        = 0xFE54,
    SCIM_KEY_dead_breve                         = 0xFE55,
    SCIM_KEY_dead_abovedot                      = 0xFE56,
    SCIM_KEY_dead_diaeresis                     = 0xFE57,
    SCIM_KEY_dead_abovering                     = 0xFE58,
    SCIM_KEY_dead_doubleacute                   = 0xFE59,
    SCIM_KEY_dead_caron                         = 0xFE5A,
    SCIM_KEY_dead_cedilla                       = 0xFE5B,
    SCIM_KEY_dead_ogonek                        = 0xFE5C,
    SCIM_KEY_dead_iota                          = 0xFE5D,
    SCIM_KEY_dead_voiced_sound                  = 0xFE5E,
    SCIM_KEY_dead_semivoiced_sound              = 0xFE5F,
    SCIM_KEY_dead_belowdot                      = 0xFE60,
    SCIM_KEY_dead_hook                          = 0xFE61,
    SCIM_KEY_dead_horn                          = 0xFE62,

    SCIM_KEY_First_Virtual_Screen               = 0xFED0,
    SCIM_KEY_Prev_Virtual_Screen                = 0xFED1,
    SCIM_KEY_Next_Virtual_Screen                = 0xFED2,
    SCIM_KEY_Last_Virtual_Screen                = 0xFED4,
    SCIM_KEY_Terminate_Server                   = 0xFED5,

    SCIM_KEY_AccessX_Enable                     = 0xFE70,
    SCIM_KEY_AccessX_Feedback_Enable            = 0xFE71,
    SCIM_KEY_RepeatKeys_Enable                  = 0xFE72,
    SCIM_KEY_SlowKeys_Enable                    = 0xFE73,
    SCIM_KEY_BounceKeys_Enable                  = 0xFE74,
    SCIM_KEY_StickyKeys_Enable                  = 0xFE75,
    SCIM_KEY_MouseKeys_Enable                   = 0xFE76,
    SCIM_KEY_MouseKeys_Accel_Enable             = 0xFE77,
    SCIM_KEY_Overlay1_Enable                    = 0xFE78,
    SCIM_KEY_Overlay2_Enable                    = 0xFE79,
    SCIM_KEY_AudibleBell_Enable                 = 0xFE7A,

    SCIM_KEY_Pointer_Left                       = 0xFEE0,
    SCIM_KEY_Pointer_Right                      = 0xFEE1,
    SCIM_KEY_Pointer_Up                         = 0xFEE2,
    SCIM_KEY_Pointer_Down                       = 0xFEE3,
    SCIM_KEY_Pointer_UpLeft                     = 0xFEE4,
    SCIM_KEY_Pointer_UpRight                    = 0xFEE5,
    SCIM_KEY_Pointer_DownLeft                   = 0xFEE6,
    SCIM_KEY_Pointer_DownRight                  = 0xFEE7,
    SCIM_KEY_Pointer_Button_Dflt                = 0xFEE8,
    SCIM_KEY_Pointer_Button1                    = 0xFEE9,
    SCIM_KEY_Pointer_Button2                    = 0xFEEA,
    SCIM_KEY_Pointer_Button3                    = 0xFEEB,
    SCIM_KEY_Pointer_Button4                    = 0xFEEC,
    SCIM_KEY_Pointer_Button5                    = 0xFEED,
    SCIM_KEY_Pointer_DblClick_Dflt              = 0xFEEE,
    SCIM_KEY_Pointer_DblClick1                  = 0xFEEF,
    SCIM_KEY_Pointer_DblClick2                  = 0xFEF0,
    SCIM_KEY_Pointer_DblClick3                  = 0xFEF1,
    SCIM_KEY_Pointer_DblClick4                  = 0xFEF2,
    SCIM_KEY_Pointer_DblClick5                  = 0xFEF3,
    SCIM_KEY_Pointer_Drag_Dflt                  = 0xFEF4,
    SCIM_KEY_Pointer_Drag1                      = 0xFEF5,
    SCIM_KEY_Pointer_Drag2                      = 0xFEF6,
    SCIM_KEY_Pointer_Drag3                      = 0xFEF7,
    SCIM_KEY_Pointer_Drag4                      = 0xFEF8,
    SCIM_KEY_Pointer_Drag5                      = 0xFEFD,

    SCIM_KEY_Pointer_EnableKeys                 = 0xFEF9,
    SCIM_KEY_Pointer_Accelerate                 = 0xFEFA,
    SCIM_KEY_Pointer_DfltBtnNext                = 0xFEFB,
    SCIM_KEY_Pointer_DfltBtnPrev                = 0xFEFC,

    /*
     * 3270 Terminal Keys
     * Byte 3 = 0xFD
     */
    SCIM_KEY_3270_Duplicate                     = 0xFD01,
    SCIM_KEY_3270_FieldMark                     = 0xFD02,
    SCIM_KEY_3270_Right2                        = 0xFD03,
    SCIM_KEY_3270_Left2                         = 0xFD04,
    SCIM_KEY_3270_BackTab                       = 0xFD05,
    SCIM_KEY_3270_EraseEOF                      = 0xFD06,
    SCIM_KEY_3270_EraseInput                    = 0xFD07,
    SCIM_KEY_3270_Reset                         = 0xFD08,
    SCIM_KEY_3270_Quit                          = 0xFD09,
    SCIM_KEY_3270_PA1                           = 0xFD0A,
    SCIM_KEY_3270_PA2                           = 0xFD0B,
    SCIM_KEY_3270_PA3                           = 0xFD0C,
    SCIM_KEY_3270_Test                          = 0xFD0D,
    SCIM_KEY_3270_Attn                          = 0xFD0E,
    SCIM_KEY_3270_CursorBlink                   = 0xFD0F,
    SCIM_KEY_3270_AltCursor                     = 0xFD10,
    SCIM_KEY_3270_KeyClick                      = 0xFD11,
    SCIM_KEY_3270_Jump                          = 0xFD12,
    SCIM_KEY_3270_Ident                         = 0xFD13,
    SCIM_KEY_3270_Rule                          = 0xFD14,
    SCIM_KEY_3270_Copy                          = 0xFD15,
    SCIM_KEY_3270_Play                          = 0xFD16,
    SCIM_KEY_3270_Setup                         = 0xFD17,
    SCIM_KEY_3270_Record                        = 0xFD18,
    SCIM_KEY_3270_ChangeScreen                  = 0xFD19,
    SCIM_KEY_3270_DeleteWord                    = 0xFD1A,
    SCIM_KEY_3270_ExSelect                      = 0xFD1B,
    SCIM_KEY_3270_CursorSelect                  = 0xFD1C,
    SCIM_KEY_3270_PrintScreen                   = 0xFD1D,
    SCIM_KEY_3270_Enter                         = 0xFD1E,

    /* Latin 1 */
    SCIM_KEY_space                              = 0x020,
    SCIM_KEY_exclam                             = 0x021,
    SCIM_KEY_quotedbl                           = 0x022,
    SCIM_KEY_numbersign                         = 0x023,
    SCIM_KEY_dollar                             = 0x024,
    SCIM_KEY_percent                            = 0x025,
    SCIM_KEY_ampersand                          = 0x026,
    SCIM_KEY_apostrophe                         = 0x027,
    SCIM_KEY_quoteright                         = 0x027,    /* deprecated */
    SCIM_KEY_parenleft                          = 0x028,
    SCIM_KEY_parenright                         = 0x029,
    SCIM_KEY_asterisk                           = 0x02a,
    SCIM_KEY_plus                               = 0x02b,
    SCIM_KEY_comma                              = 0x02c,
    SCIM_KEY_minus                              = 0x02d,
    SCIM_KEY_period                             = 0x02e,
    SCIM_KEY_slash                              = 0x02f,
    SCIM_KEY_0                                  = 0x030,
    SCIM_KEY_1                                  = 0x031,
    SCIM_KEY_2                                  = 0x032,
    SCIM_KEY_3                                  = 0x033,
    SCIM_KEY_4                                  = 0x034,
    SCIM_KEY_5                                  = 0x035,
    SCIM_KEY_6                                  = 0x036,
    SCIM_KEY_7                                  = 0x037,
    SCIM_KEY_8                                  = 0x038,
    SCIM_KEY_9                                  = 0x039,
    SCIM_KEY_colon                              = 0x03a,
    SCIM_KEY_semicolon                          = 0x03b,
    SCIM_KEY_less                               = 0x03c,
    SCIM_KEY_equal                              = 0x03d,
    SCIM_KEY_greater                            = 0x03e,
    SCIM_KEY_question                           = 0x03f,
    SCIM_KEY_at                                 = 0x040,
    SCIM_KEY_A                                  = 0x041,
    SCIM_KEY_B                                  = 0x042,
    SCIM_KEY_C                                  = 0x043,
    SCIM_KEY_D                                  = 0x044,
    SCIM_KEY_E                                  = 0x045,
    SCIM_KEY_F                                  = 0x046,
    SCIM_KEY_G                                  = 0x047,
    SCIM_KEY_H                                  = 0x048,
    SCIM_KEY_I                                  = 0x049,
    SCIM_KEY_J                                  = 0x04a,
    SCIM_KEY_K                                  = 0x04b,
    SCIM_KEY_L                                  = 0x04c,
    SCIM_KEY_M                                  = 0x04d,
    SCIM_KEY_N                                  = 0x04e,
    SCIM_KEY_O                                  = 0x04f,
    SCIM_KEY_P                                  = 0x050,
    SCIM_KEY_Q                                  = 0x051,
    SCIM_KEY_R                                  = 0x052,
    SCIM_KEY_S                                  = 0x053,
    SCIM_KEY_T                                  = 0x054,
    SCIM_KEY_U                                  = 0x055,
    SCIM_KEY_V                                  = 0x056,
    SCIM_KEY_W                                  = 0x057,
    SCIM_KEY_X                                  = 0x058,
    SCIM_KEY_Y                                  = 0x059,
    SCIM_KEY_Z                                  = 0x05a,
    SCIM_KEY_bracketleft                        = 0x05b,
    SCIM_KEY_backslash                          = 0x05c,
    SCIM_KEY_bracketright                       = 0x05d,
    SCIM_KEY_asciicircum                        = 0x05e,
    SCIM_KEY_underscore                         = 0x05f,
    SCIM_KEY_grave                              = 0x060,
    SCIM_KEY_a                                  = 0x061,
    SCIM_KEY_b                                  = 0x062,
    SCIM_KEY_c                                  = 0x063,
    SCIM_KEY_d                                  = 0x064,
    SCIM_KEY_e                                  = 0x065,
    SCIM_KEY_f                                  = 0x066,
    SCIM_KEY_g                                  = 0x067,
    SCIM_KEY_h                                  = 0x068,
    SCIM_KEY_i                                  = 0x069,
    SCIM_KEY_j                                  = 0x06a,
    SCIM_KEY_k                                  = 0x06b,
    SCIM_KEY_l                                  = 0x06c,
    SCIM_KEY_m                                  = 0x06d,
    SCIM_KEY_n                                  = 0x06e,
    SCIM_KEY_o                                  = 0x06f,
    SCIM_KEY_p                                  = 0x070,
    SCIM_KEY_q                                  = 0x071,
    SCIM_KEY_r                                  = 0x072,
    SCIM_KEY_s                                  = 0x073,
    SCIM_KEY_t                                  = 0x074,
    SCIM_KEY_u                                  = 0x075,
    SCIM_KEY_v                                  = 0x076,
    SCIM_KEY_w                                  = 0x077,
    SCIM_KEY_x                                  = 0x078,
    SCIM_KEY_y                                  = 0x079,
    SCIM_KEY_z                                  = 0x07a,
    SCIM_KEY_braceleft                          = 0x07b,
    SCIM_KEY_bar                                = 0x07c,
    SCIM_KEY_braceright                         = 0x07d,
    SCIM_KEY_asciitilde                         = 0x07e,

    SCIM_KEY_nobreakspace                       = 0x0a0,
    SCIM_KEY_exclamdown                         = 0x0a1,
    SCIM_KEY_cent                               = 0x0a2,
    SCIM_KEY_sterling                           = 0x0a3,
    SCIM_KEY_currency                           = 0x0a4,
    SCIM_KEY_yen                                = 0x0a5,
    SCIM_KEY_brokenbar                          = 0x0a6,
    SCIM_KEY_section                            = 0x0a7,
    SCIM_KEY_diaeresis                          = 0x0a8,
    SCIM_KEY_copyright                          = 0x0a9,
    SCIM_KEY_ordfeminine                        = 0x0aa,
    SCIM_KEY_guillemotleft                      = 0x0ab,     /* left angle quotation mark */
    SCIM_KEY_notsign                            = 0x0ac,
    SCIM_KEY_hyphen                             = 0x0ad,
    SCIM_KEY_registered                         = 0x0ae,
    SCIM_KEY_macron                             = 0x0af,
    SCIM_KEY_degree                             = 0x0b0,
    SCIM_KEY_plusminus                          = 0x0b1,
    SCIM_KEY_twosuperior                        = 0x0b2,
    SCIM_KEY_threesuperior                      = 0x0b3,
    SCIM_KEY_acute                              = 0x0b4,
    SCIM_KEY_mu                                 = 0x0b5,
    SCIM_KEY_paragraph                          = 0x0b6,
    SCIM_KEY_periodcentered                     = 0x0b7,
    SCIM_KEY_cedilla                            = 0x0b8,
    SCIM_KEY_onesuperior                        = 0x0b9,
    SCIM_KEY_masculine                          = 0x0ba,
    SCIM_KEY_guillemotright                     = 0x0bb,     /* right angle quotation mark */
    SCIM_KEY_onequarter                         = 0x0bc,
    SCIM_KEY_onehalf                            = 0x0bd,
    SCIM_KEY_threequarters                      = 0x0be,
    SCIM_KEY_questiondown                       = 0x0bf,
    SCIM_KEY_Agrave                             = 0x0c0,
    SCIM_KEY_Aacute                             = 0x0c1,
    SCIM_KEY_Acircumflex                        = 0x0c2,
    SCIM_KEY_Atilde                             = 0x0c3,
    SCIM_KEY_Adiaeresis                         = 0x0c4,
    SCIM_KEY_Aring                              = 0x0c5,
    SCIM_KEY_AE                                 = 0x0c6,
    SCIM_KEY_Ccedilla                           = 0x0c7,
    SCIM_KEY_Egrave                             = 0x0c8,
    SCIM_KEY_Eacute                             = 0x0c9,
    SCIM_KEY_Ecircumflex                        = 0x0ca,
    SCIM_KEY_Ediaeresis                         = 0x0cb,
    SCIM_KEY_Igrave                             = 0x0cc,
    SCIM_KEY_Iacute                             = 0x0cd,
    SCIM_KEY_Icircumflex                        = 0x0ce,
    SCIM_KEY_Idiaeresis                         = 0x0cf,
    SCIM_KEY_ETH                                = 0x0d0,
    SCIM_KEY_Eth                                = 0x0d0,     /* deprecated */
    SCIM_KEY_Ntilde                             = 0x0d1,
    SCIM_KEY_Ograve                             = 0x0d2,
    SCIM_KEY_Oacute                             = 0x0d3,
    SCIM_KEY_Ocircumflex                        = 0x0d4,
    SCIM_KEY_Otilde                             = 0x0d5,
    SCIM_KEY_Odiaeresis                         = 0x0d6,
    SCIM_KEY_multiply                           = 0x0d7,
    SCIM_KEY_Ooblique                           = 0x0d8,
    SCIM_KEY_Oslash                             = SCIM_KEY_Ooblique,
    SCIM_KEY_Ugrave                             = 0x0d9,
    SCIM_KEY_Uacute                             = 0x0da,
    SCIM_KEY_Ucircumflex                        = 0x0db,
    SCIM_KEY_Udiaeresis                         = 0x0dc,
    SCIM_KEY_Yacute                             = 0x0dd,
    SCIM_KEY_THORN                              = 0x0de,
    SCIM_KEY_Thorn                              = 0x0de,     /* deprecated */
    SCIM_KEY_ssharp                             = 0x0df,
    SCIM_KEY_agrave                             = 0x0e0,
    SCIM_KEY_aacute                             = 0x0e1,
    SCIM_KEY_acircumflex                        = 0x0e2,
    SCIM_KEY_atilde                             = 0x0e3,
    SCIM_KEY_adiaeresis                         = 0x0e4,
    SCIM_KEY_aring                              = 0x0e5,
    SCIM_KEY_ae                                 = 0x0e6,
    SCIM_KEY_ccedilla                           = 0x0e7,
    SCIM_KEY_egrave                             = 0x0e8,
    SCIM_KEY_eacute                             = 0x0e9,
    SCIM_KEY_ecircumflex                        = 0x0ea,
    SCIM_KEY_ediaeresis                         = 0x0eb,
    SCIM_KEY_igrave                             = 0x0ec,
    SCIM_KEY_iacute                             = 0x0ed,
    SCIM_KEY_icircumflex                        = 0x0ee,
    SCIM_KEY_idiaeresis                         = 0x0ef,
    SCIM_KEY_eth                                = 0x0f0,
    SCIM_KEY_ntilde                             = 0x0f1,
    SCIM_KEY_ograve                             = 0x0f2,
    SCIM_KEY_oacute                             = 0x0f3,
    SCIM_KEY_ocircumflex                        = 0x0f4,
    SCIM_KEY_otilde                             = 0x0f5,
    SCIM_KEY_odiaeresis                         = 0x0f6,
    SCIM_KEY_division                           = 0x0f7,
    SCIM_KEY_oslash                             = 0x0f8,
    SCIM_KEY_ooblique                           = SCIM_KEY_oslash,
    SCIM_KEY_ugrave                             = 0x0f9,
    SCIM_KEY_uacute                             = 0x0fa,
    SCIM_KEY_ucircumflex                        = 0x0fb,
    SCIM_KEY_udiaeresis                         = 0x0fc,
    SCIM_KEY_yacute                             = 0x0fd,
    SCIM_KEY_thorn                              = 0x0fe,
    SCIM_KEY_ydiaeresis                         = 0x0ff,

    /*
     *   Latin 2
     *   Byte 3 = 1
     */
    SCIM_KEY_Aogonek                            = 0x1a1,
    SCIM_KEY_breve                              = 0x1a2,
    SCIM_KEY_Lstroke                            = 0x1a3,
    SCIM_KEY_Lcaron                             = 0x1a5,
    SCIM_KEY_Sacute                             = 0x1a6,
    SCIM_KEY_Scaron                             = 0x1a9,
    SCIM_KEY_Scedilla                           = 0x1aa,
    SCIM_KEY_Tcaron                             = 0x1ab,
    SCIM_KEY_Zacute                             = 0x1ac,
    SCIM_KEY_Zcaron                             = 0x1ae,
    SCIM_KEY_Zabovedot                          = 0x1af,
    SCIM_KEY_aogonek                            = 0x1b1,
    SCIM_KEY_ogonek                             = 0x1b2,
    SCIM_KEY_lstroke                            = 0x1b3,
    SCIM_KEY_lcaron                             = 0x1b5,
    SCIM_KEY_sacute                             = 0x1b6,
    SCIM_KEY_caron                              = 0x1b7,
    SCIM_KEY_scaron                             = 0x1b9,
    SCIM_KEY_scedilla                           = 0x1ba,
    SCIM_KEY_tcaron                             = 0x1bb,
    SCIM_KEY_zacute                             = 0x1bc,
    SCIM_KEY_doubleacute                        = 0x1bd,
    SCIM_KEY_zcaron                             = 0x1be,
    SCIM_KEY_zabovedot                          = 0x1bf,
    SCIM_KEY_Racute                             = 0x1c0,
    SCIM_KEY_Abreve                             = 0x1c3,
    SCIM_KEY_Lacute                             = 0x1c5,
    SCIM_KEY_Cacute                             = 0x1c6,
    SCIM_KEY_Ccaron                             = 0x1c8,
    SCIM_KEY_Eogonek                            = 0x1ca,
    SCIM_KEY_Ecaron                             = 0x1cc,
    SCIM_KEY_Dcaron                             = 0x1cf,
    SCIM_KEY_Dstroke                            = 0x1d0,
    SCIM_KEY_Nacute                             = 0x1d1,
    SCIM_KEY_Ncaron                             = 0x1d2,
    SCIM_KEY_Odoubleacute                       = 0x1d5,
    SCIM_KEY_Rcaron                             = 0x1d8,
    SCIM_KEY_Uring                              = 0x1d9,
    SCIM_KEY_Udoubleacute                       = 0x1db,
    SCIM_KEY_Tcedilla                           = 0x1de,
    SCIM_KEY_racute                             = 0x1e0,
    SCIM_KEY_abreve                             = 0x1e3,
    SCIM_KEY_lacute                             = 0x1e5,
    SCIM_KEY_cacute                             = 0x1e6,
    SCIM_KEY_ccaron                             = 0x1e8,
    SCIM_KEY_eogonek                            = 0x1ea,
    SCIM_KEY_ecaron                             = 0x1ec,
    SCIM_KEY_dcaron                             = 0x1ef,
    SCIM_KEY_dstroke                            = 0x1f0,
    SCIM_KEY_nacute                             = 0x1f1,
    SCIM_KEY_ncaron                             = 0x1f2,
    SCIM_KEY_odoubleacute                       = 0x1f5,
    SCIM_KEY_udoubleacute                       = 0x1fb,
    SCIM_KEY_rcaron                             = 0x1f8,
    SCIM_KEY_uring                              = 0x1f9,
    SCIM_KEY_tcedilla                           = 0x1fe,
    SCIM_KEY_abovedot                           = 0x1ff,

    /*
     *   Latin 3
     *   Byte 3 = 2
     */
    SCIM_KEY_Hstroke                            = 0x2a1,
    SCIM_KEY_Hcircumflex                        = 0x2a6,
    SCIM_KEY_Iabovedot                          = 0x2a9,
    SCIM_KEY_Gbreve                             = 0x2ab,
    SCIM_KEY_Jcircumflex                        = 0x2ac,
    SCIM_KEY_hstroke                            = 0x2b1,
    SCIM_KEY_hcircumflex                        = 0x2b6,
    SCIM_KEY_idotless                           = 0x2b9,
    SCIM_KEY_gbreve                             = 0x2bb,
    SCIM_KEY_jcircumflex                        = 0x2bc,
    SCIM_KEY_Cabovedot                          = 0x2c5,
    SCIM_KEY_Ccircumflex                        = 0x2c6,
    SCIM_KEY_Gabovedot                          = 0x2d5,
    SCIM_KEY_Gcircumflex                        = 0x2d8,
    SCIM_KEY_Ubreve                             = 0x2dd,
    SCIM_KEY_Scircumflex                        = 0x2de,
    SCIM_KEY_cabovedot                          = 0x2e5,
    SCIM_KEY_ccircumflex                        = 0x2e6,
    SCIM_KEY_gabovedot                          = 0x2f5,
    SCIM_KEY_gcircumflex                        = 0x2f8,
    SCIM_KEY_ubreve                             = 0x2fd,
    SCIM_KEY_scircumflex                        = 0x2fe,


    /*
     *   Latin 4
     *   Byte 3 = 3
     */
    SCIM_KEY_kra                                = 0x3a2,
    SCIM_KEY_kappa                              = 0x3a2,   /* deprecated */
    SCIM_KEY_Rcedilla                           = 0x3a3,
    SCIM_KEY_Itilde                             = 0x3a5,
    SCIM_KEY_Lcedilla                           = 0x3a6,
    SCIM_KEY_Emacron                            = 0x3aa,
    SCIM_KEY_Gcedilla                           = 0x3ab,
    SCIM_KEY_Tslash                             = 0x3ac,
    SCIM_KEY_rcedilla                           = 0x3b3,
    SCIM_KEY_itilde                             = 0x3b5,
    SCIM_KEY_lcedilla                           = 0x3b6,
    SCIM_KEY_emacron                            = 0x3ba,
    SCIM_KEY_gcedilla                           = 0x3bb,
    SCIM_KEY_tslash                             = 0x3bc,
    SCIM_KEY_ENG                                = 0x3bd,
    SCIM_KEY_eng                                = 0x3bf,
    SCIM_KEY_Amacron                            = 0x3c0,
    SCIM_KEY_Iogonek                            = 0x3c7,
    SCIM_KEY_Eabovedot                          = 0x3cc,
    SCIM_KEY_Imacron                            = 0x3cf,
    SCIM_KEY_Ncedilla                           = 0x3d1,
    SCIM_KEY_Omacron                            = 0x3d2,
    SCIM_KEY_Kcedilla                           = 0x3d3,
    SCIM_KEY_Uogonek                            = 0x3d9,
    SCIM_KEY_Utilde                             = 0x3dd,
    SCIM_KEY_Umacron                            = 0x3de,
    SCIM_KEY_amacron                            = 0x3e0,
    SCIM_KEY_iogonek                            = 0x3e7,
    SCIM_KEY_eabovedot                          = 0x3ec,
    SCIM_KEY_imacron                            = 0x3ef,
    SCIM_KEY_ncedilla                           = 0x3f1,
    SCIM_KEY_omacron                            = 0x3f2,
    SCIM_KEY_kcedilla                           = 0x3f3,
    SCIM_KEY_uogonek                            = 0x3f9,
    SCIM_KEY_utilde                             = 0x3fd,
    SCIM_KEY_umacron                            = 0x3fe,

/*
 * Latin-8
 * Byte 3 = 18
 */
    SCIM_KEY_Babovedot                          = 0x12a1,
    SCIM_KEY_babovedot                          = 0x12a2,
    SCIM_KEY_Dabovedot                          = 0x12a6,
    SCIM_KEY_Wgrave                             = 0x12a8,
    SCIM_KEY_Wacute                             = 0x12aa,
    SCIM_KEY_dabovedot                          = 0x12ab,
    SCIM_KEY_Ygrave                             = 0x12ac,
    SCIM_KEY_Fabovedot                          = 0x12b0,
    SCIM_KEY_fabovedot                          = 0x12b1,
    SCIM_KEY_Mabovedot                          = 0x12b4,
    SCIM_KEY_mabovedot                          = 0x12b5,
    SCIM_KEY_Pabovedot                          = 0x12b7,
    SCIM_KEY_wgrave                             = 0x12b8,
    SCIM_KEY_pabovedot                          = 0x12b9,
    SCIM_KEY_wacute                             = 0x12ba,
    SCIM_KEY_Sabovedot                          = 0x12bb,
    SCIM_KEY_ygrave                             = 0x12bc,
    SCIM_KEY_Wdiaeresis                         = 0x12bd,
    SCIM_KEY_wdiaeresis                         = 0x12be,
    SCIM_KEY_sabovedot                          = 0x12bf,
    SCIM_KEY_Wcircumflex                        = 0x12d0,
    SCIM_KEY_Tabovedot                          = 0x12d7,
    SCIM_KEY_Ycircumflex                        = 0x12de,
    SCIM_KEY_wcircumflex                        = 0x12f0,
    SCIM_KEY_tabovedot                          = 0x12f7,
    SCIM_KEY_ycircumflex                        = 0x12fe,

    /*
     * Latin-9 (a.k.a. Latin-0)
     * Byte 3 = 19
     */

    SCIM_KEY_OE                                 = 0x13bc,
    SCIM_KEY_oe                                 = 0x13bd,
    SCIM_KEY_Ydiaeresis                         = 0x13be,

    /*
     * Katakana
     * Byte 3 = 4
     */

    SCIM_KEY_overline                           = 0x47e,
    SCIM_KEY_kana_fullstop                      = 0x4a1,
    SCIM_KEY_kana_openingbracket                = 0x4a2,
    SCIM_KEY_kana_closingbracket                = 0x4a3,
    SCIM_KEY_kana_comma                         = 0x4a4,
    SCIM_KEY_kana_conjunctive                   = 0x4a5,
    SCIM_KEY_kana_middledot                     = 0x4a5,  /* deprecated */
    SCIM_KEY_kana_WO                            = 0x4a6,
    SCIM_KEY_kana_a                             = 0x4a7,
    SCIM_KEY_kana_i                             = 0x4a8,
    SCIM_KEY_kana_u                             = 0x4a9,
    SCIM_KEY_kana_e                             = 0x4aa,
    SCIM_KEY_kana_o                             = 0x4ab,
    SCIM_KEY_kana_ya                            = 0x4ac,
    SCIM_KEY_kana_yu                            = 0x4ad,
    SCIM_KEY_kana_yo                            = 0x4ae,
    SCIM_KEY_kana_tsu                           = 0x4af,
    SCIM_KEY_kana_tu                            = 0x4af,  /* deprecated */
    SCIM_KEY_prolongedsound                     = 0x4b0,
    SCIM_KEY_kana_A                             = 0x4b1,
    SCIM_KEY_kana_I                             = 0x4b2,
    SCIM_KEY_kana_U                             = 0x4b3,
    SCIM_KEY_kana_E                             = 0x4b4,
    SCIM_KEY_kana_O                             = 0x4b5,
    SCIM_KEY_kana_KA                            = 0x4b6,
    SCIM_KEY_kana_KI                            = 0x4b7,
    SCIM_KEY_kana_KU                            = 0x4b8,
    SCIM_KEY_kana_KE                            = 0x4b9,
    SCIM_KEY_kana_KO                            = 0x4ba,
    SCIM_KEY_kana_SA                            = 0x4bb,
    SCIM_KEY_kana_SHI                           = 0x4bc,
    SCIM_KEY_kana_SU                            = 0x4bd,
    SCIM_KEY_kana_SE                            = 0x4be,
    SCIM_KEY_kana_SO                            = 0x4bf,
    SCIM_KEY_kana_TA                            = 0x4c0,
    SCIM_KEY_kana_CHI                           = 0x4c1,
    SCIM_KEY_kana_TI                            = 0x4c1,  /* deprecated */
    SCIM_KEY_kana_TSU                           = 0x4c2,
    SCIM_KEY_kana_TU                            = 0x4c2,  /* deprecated */
    SCIM_KEY_kana_TE                            = 0x4c3,
    SCIM_KEY_kana_TO                            = 0x4c4,
    SCIM_KEY_kana_NA                            = 0x4c5,
    SCIM_KEY_kana_NI                            = 0x4c6,
    SCIM_KEY_kana_NU                            = 0x4c7,
    SCIM_KEY_kana_NE                            = 0x4c8,
    SCIM_KEY_kana_NO                            = 0x4c9,
    SCIM_KEY_kana_HA                            = 0x4ca,
    SCIM_KEY_kana_HI                            = 0x4cb,
    SCIM_KEY_kana_FU                            = 0x4cc,
    SCIM_KEY_kana_HU                            = 0x4cc,  /* deprecated */
    SCIM_KEY_kana_HE                            = 0x4cd,
    SCIM_KEY_kana_HO                            = 0x4ce,
    SCIM_KEY_kana_MA                            = 0x4cf,
    SCIM_KEY_kana_MI                            = 0x4d0,
    SCIM_KEY_kana_MU                            = 0x4d1,
    SCIM_KEY_kana_ME                            = 0x4d2,
    SCIM_KEY_kana_MO                            = 0x4d3,
    SCIM_KEY_kana_YA                            = 0x4d4,
    SCIM_KEY_kana_YU                            = 0x4d5,
    SCIM_KEY_kana_YO                            = 0x4d6,
    SCIM_KEY_kana_RA                            = 0x4d7,
    SCIM_KEY_kana_RI                            = 0x4d8,
    SCIM_KEY_kana_RU                            = 0x4d9,
    SCIM_KEY_kana_RE                            = 0x4da,
    SCIM_KEY_kana_RO                            = 0x4db,
    SCIM_KEY_kana_WA                            = 0x4dc,
    SCIM_KEY_kana_N                             = 0x4dd,
    SCIM_KEY_voicedsound                        = 0x4de,
    SCIM_KEY_semivoicedsound                    = 0x4df,
    SCIM_KEY_kana_switch                        = 0xFF7E,  /* Alias for mode_switch */

    /*
     *  Arabic
     *  Byte 3 = 5
     */
    SCIM_KEY_Farsi_0                            = 0x590,
    SCIM_KEY_Farsi_1                            = 0x591,
    SCIM_KEY_Farsi_2                            = 0x592,
    SCIM_KEY_Farsi_3                            = 0x593,
    SCIM_KEY_Farsi_4                            = 0x594,
    SCIM_KEY_Farsi_5                            = 0x595,
    SCIM_KEY_Farsi_6                            = 0x596,
    SCIM_KEY_Farsi_7                            = 0x597,
    SCIM_KEY_Farsi_8                            = 0x598,
    SCIM_KEY_Farsi_9                            = 0x599,
    SCIM_KEY_Arabic_percent                     = 0x5a5,
    SCIM_KEY_Arabic_superscript_alef            = 0x5a6,
    SCIM_KEY_Arabic_tteh                        = 0x5a7,
    SCIM_KEY_Arabic_peh                         = 0x5a8,
    SCIM_KEY_Arabic_tcheh                       = 0x5a9,
    SCIM_KEY_Arabic_ddal                        = 0x5aa,
    SCIM_KEY_Arabic_rreh                        = 0x5ab,
    SCIM_KEY_Arabic_comma                       = 0x5ac,
    SCIM_KEY_Arabic_fullstop                    = 0x5ae,
    SCIM_KEY_Arabic_0                           = 0x5b0,
    SCIM_KEY_Arabic_1                           = 0x5b1,
    SCIM_KEY_Arabic_2                           = 0x5b2,
    SCIM_KEY_Arabic_3                           = 0x5b3,
    SCIM_KEY_Arabic_4                           = 0x5b4,
    SCIM_KEY_Arabic_5                           = 0x5b5,
    SCIM_KEY_Arabic_6                           = 0x5b6,
    SCIM_KEY_Arabic_7                           = 0x5b7,
    SCIM_KEY_Arabic_8                           = 0x5b8,
    SCIM_KEY_Arabic_9                           = 0x5b9,
    SCIM_KEY_Arabic_semicolon                   = 0x5bb,
    SCIM_KEY_Arabic_question_mark               = 0x5bf,
    SCIM_KEY_Arabic_hamza                       = 0x5c1,
    SCIM_KEY_Arabic_maddaonalef                 = 0x5c2,
    SCIM_KEY_Arabic_hamzaonalef                 = 0x5c3,
    SCIM_KEY_Arabic_hamzaonwaw                  = 0x5c4,
    SCIM_KEY_Arabic_hamzaunderalef              = 0x5c5,
    SCIM_KEY_Arabic_hamzaonyeh                  = 0x5c6,
    SCIM_KEY_Arabic_alef                        = 0x5c7,
    SCIM_KEY_Arabic_beh                         = 0x5c8,
    SCIM_KEY_Arabic_tehmarbuta                  = 0x5c9,
    SCIM_KEY_Arabic_teh                         = 0x5ca,
    SCIM_KEY_Arabic_theh                        = 0x5cb,
    SCIM_KEY_Arabic_jeem                        = 0x5cc,
    SCIM_KEY_Arabic_hah                         = 0x5cd,
    SCIM_KEY_Arabic_khah                        = 0x5ce,
    SCIM_KEY_Arabic_dal                         = 0x5cf,
    SCIM_KEY_Arabic_thal                        = 0x5d0,
    SCIM_KEY_Arabic_ra                          = 0x5d1,
    SCIM_KEY_Arabic_zain                        = 0x5d2,
    SCIM_KEY_Arabic_seen                        = 0x5d3,
    SCIM_KEY_Arabic_sheen                       = 0x5d4,
    SCIM_KEY_Arabic_sad                         = 0x5d5,
    SCIM_KEY_Arabic_dad                         = 0x5d6,
    SCIM_KEY_Arabic_tah                         = 0x5d7,
    SCIM_KEY_Arabic_zah                         = 0x5d8,
    SCIM_KEY_Arabic_ain                         = 0x5d9,
    SCIM_KEY_Arabic_ghain                       = 0x5da,
    SCIM_KEY_Arabic_tatweel                     = 0x5e0,
    SCIM_KEY_Arabic_feh                         = 0x5e1,
    SCIM_KEY_Arabic_qaf                         = 0x5e2,
    SCIM_KEY_Arabic_kaf                         = 0x5e3,
    SCIM_KEY_Arabic_lam                         = 0x5e4,
    SCIM_KEY_Arabic_meem                        = 0x5e5,
    SCIM_KEY_Arabic_noon                        = 0x5e6,
    SCIM_KEY_Arabic_ha                          = 0x5e7,
    SCIM_KEY_Arabic_heh                         = 0x5e7,  /* deprecated */
    SCIM_KEY_Arabic_waw                         = 0x5e8,
    SCIM_KEY_Arabic_alefmaksura                 = 0x5e9,
    SCIM_KEY_Arabic_yeh                         = 0x5ea,
    SCIM_KEY_Arabic_fathatan                    = 0x5eb,
    SCIM_KEY_Arabic_dammatan                    = 0x5ec,
    SCIM_KEY_Arabic_kasratan                    = 0x5ed,
    SCIM_KEY_Arabic_fatha                       = 0x5ee,
    SCIM_KEY_Arabic_damma                       = 0x5ef,
    SCIM_KEY_Arabic_kasra                       = 0x5f0,
    SCIM_KEY_Arabic_shadda                      = 0x5f1,
    SCIM_KEY_Arabic_sukun                       = 0x5f2,
    SCIM_KEY_Arabic_madda_above                 = 0x5f3,
    SCIM_KEY_Arabic_hamza_above                 = 0x5f4,
    SCIM_KEY_Arabic_hamza_below                 = 0x5f5,
    SCIM_KEY_Arabic_jeh                         = 0x5f6,
    SCIM_KEY_Arabic_veh                         = 0x5f7,
    SCIM_KEY_Arabic_keheh                       = 0x5f8,
    SCIM_KEY_Arabic_gaf                         = 0x5f9,
    SCIM_KEY_Arabic_noon_ghunna                 = 0x5fa,
    SCIM_KEY_Arabic_heh_doachashmee             = 0x5fb,
    SCIM_KEY_Farsi_yeh                          = 0x5fc,
    SCIM_KEY_Arabic_farsi_yeh                   = SCIM_KEY_Farsi_yeh,
    SCIM_KEY_Arabic_yeh_baree                   = 0x5fd,
    SCIM_KEY_Arabic_heh_goal                    = 0x5fe,
    SCIM_KEY_Arabic_switch                      = 0xFF7E,  /* Alias for mode_switch */

    /*
     * Cyrillic
     * Byte 3 = 6
     */
    SCIM_KEY_Cyrillic_GHE_bar                   = 0x680,
    SCIM_KEY_Cyrillic_ghe_bar                   = 0x690,
    SCIM_KEY_Cyrillic_ZHE_descender             = 0x681,
    SCIM_KEY_Cyrillic_zhe_descender             = 0x691,
    SCIM_KEY_Cyrillic_KA_descender              = 0x682,
    SCIM_KEY_Cyrillic_ka_descender              = 0x692,
    SCIM_KEY_Cyrillic_KA_vertstroke             = 0x683,
    SCIM_KEY_Cyrillic_ka_vertstroke             = 0x693,
    SCIM_KEY_Cyrillic_EN_descender              = 0x684,
    SCIM_KEY_Cyrillic_en_descender              = 0x694,
    SCIM_KEY_Cyrillic_U_straight                = 0x685,
    SCIM_KEY_Cyrillic_u_straight                = 0x695,
    SCIM_KEY_Cyrillic_U_straight_bar            = 0x686,
    SCIM_KEY_Cyrillic_u_straight_bar            = 0x696,
    SCIM_KEY_Cyrillic_HA_descender              = 0x687,
    SCIM_KEY_Cyrillic_ha_descender              = 0x697,
    SCIM_KEY_Cyrillic_CHE_descender             = 0x688,
    SCIM_KEY_Cyrillic_che_descender             = 0x698,
    SCIM_KEY_Cyrillic_CHE_vertstroke            = 0x689,
    SCIM_KEY_Cyrillic_che_vertstroke            = 0x699,
    SCIM_KEY_Cyrillic_SHHA                      = 0x68a,
    SCIM_KEY_Cyrillic_shha                      = 0x69a,

    SCIM_KEY_Cyrillic_SCHWA                     = 0x68c,
    SCIM_KEY_Cyrillic_schwa                     = 0x69c,
    SCIM_KEY_Cyrillic_I_macron                  = 0x68d,
    SCIM_KEY_Cyrillic_i_macron                  = 0x69d,
    SCIM_KEY_Cyrillic_O_bar                     = 0x68e,
    SCIM_KEY_Cyrillic_o_bar                     = 0x69e,
    SCIM_KEY_Cyrillic_U_macron                  = 0x68f,
    SCIM_KEY_Cyrillic_u_macron                  = 0x69f,

    SCIM_KEY_Serbian_dje                        = 0x6a1,
    SCIM_KEY_Macedonia_gje                      = 0x6a2,
    SCIM_KEY_Cyrillic_io                        = 0x6a3,
    SCIM_KEY_Ukrainian_ie                       = 0x6a4,
    SCIM_KEY_Ukranian_je                        = 0x6a4,  /* deprecated */
    SCIM_KEY_Macedonia_dse                      = 0x6a5,
    SCIM_KEY_Ukrainian_i                        = 0x6a6,
    SCIM_KEY_Ukranian_i                         = 0x6a6,  /* deprecated */
    SCIM_KEY_Ukrainian_yi                       = 0x6a7,
    SCIM_KEY_Ukranian_yi                        = 0x6a7,  /* deprecated */
    SCIM_KEY_Cyrillic_je                        = 0x6a8,
    SCIM_KEY_Serbian_je                         = 0x6a8,  /* deprecated */
    SCIM_KEY_Cyrillic_lje                       = 0x6a9,
    SCIM_KEY_Serbian_lje                        = 0x6a9,  /* deprecated */
    SCIM_KEY_Cyrillic_nje                       = 0x6aa,
    SCIM_KEY_Serbian_nje                        = 0x6aa,  /* deprecated */
    SCIM_KEY_Serbian_tshe                       = 0x6ab,
    SCIM_KEY_Macedonia_kje                      = 0x6ac,
    SCIM_KEY_Ukrainian_ghe_with_upturn          = 0x6ad,
    SCIM_KEY_Byelorussian_shortu                = 0x6ae,
    SCIM_KEY_Cyrillic_dzhe                      = 0x6af,
    SCIM_KEY_Serbian_dze                        = 0x6af,  /* deprecated */
    SCIM_KEY_numerosign                         = 0x6b0,
    SCIM_KEY_Serbian_DJE                        = 0x6b1,
    SCIM_KEY_Macedonia_GJE                      = 0x6b2,
    SCIM_KEY_Cyrillic_IO                        = 0x6b3,
    SCIM_KEY_Ukrainian_IE                       = 0x6b4,
    SCIM_KEY_Ukranian_JE                        = 0x6b4,  /* deprecated */
    SCIM_KEY_Macedonia_DSE                      = 0x6b5,
    SCIM_KEY_Ukrainian_I                        = 0x6b6,
    SCIM_KEY_Ukranian_I                         = 0x6b6,  /* deprecated */
    SCIM_KEY_Ukrainian_YI                       = 0x6b7,
    SCIM_KEY_Ukranian_YI                        = 0x6b7,  /* deprecated */
    SCIM_KEY_Cyrillic_JE                        = 0x6b8,
    SCIM_KEY_Serbian_JE                         = 0x6b8,  /* deprecated */
    SCIM_KEY_Cyrillic_LJE                       = 0x6b9,
    SCIM_KEY_Serbian_LJE                        = 0x6b9,  /* deprecated */
    SCIM_KEY_Cyrillic_NJE                       = 0x6ba,
    SCIM_KEY_Serbian_NJE                        = 0x6ba,  /* deprecated */
    SCIM_KEY_Serbian_TSHE                       = 0x6bb,
    SCIM_KEY_Macedonia_KJE                      = 0x6bc,
    SCIM_KEY_Ukrainian_GHE_WITH_UPTURN          = 0x6bd,
    SCIM_KEY_Byelorussian_SHORTU                = 0x6be,
    SCIM_KEY_Cyrillic_DZHE                      = 0x6bf,
    SCIM_KEY_Serbian_DZE                        = 0x6bf,  /* deprecated */
    SCIM_KEY_Cyrillic_yu                        = 0x6c0,
    SCIM_KEY_Cyrillic_a                         = 0x6c1,
    SCIM_KEY_Cyrillic_be                        = 0x6c2,
    SCIM_KEY_Cyrillic_tse                       = 0x6c3,
    SCIM_KEY_Cyrillic_de                        = 0x6c4,
    SCIM_KEY_Cyrillic_ie                        = 0x6c5,
    SCIM_KEY_Cyrillic_ef                        = 0x6c6,
    SCIM_KEY_Cyrillic_ghe                       = 0x6c7,
    SCIM_KEY_Cyrillic_ha                        = 0x6c8,
    SCIM_KEY_Cyrillic_i                         = 0x6c9,
    SCIM_KEY_Cyrillic_shorti                    = 0x6ca,
    SCIM_KEY_Cyrillic_ka                        = 0x6cb,
    SCIM_KEY_Cyrillic_el                        = 0x6cc,
    SCIM_KEY_Cyrillic_em                        = 0x6cd,
    SCIM_KEY_Cyrillic_en                        = 0x6ce,
    SCIM_KEY_Cyrillic_o                         = 0x6cf,
    SCIM_KEY_Cyrillic_pe                        = 0x6d0,
    SCIM_KEY_Cyrillic_ya                        = 0x6d1,
    SCIM_KEY_Cyrillic_er                        = 0x6d2,
    SCIM_KEY_Cyrillic_es                        = 0x6d3,
    SCIM_KEY_Cyrillic_te                        = 0x6d4,
    SCIM_KEY_Cyrillic_u                         = 0x6d5,
    SCIM_KEY_Cyrillic_zhe                       = 0x6d6,
    SCIM_KEY_Cyrillic_ve                        = 0x6d7,
    SCIM_KEY_Cyrillic_softsign                  = 0x6d8,
    SCIM_KEY_Cyrillic_yeru                      = 0x6d9,
    SCIM_KEY_Cyrillic_ze                        = 0x6da,
    SCIM_KEY_Cyrillic_sha                       = 0x6db,
    SCIM_KEY_Cyrillic_e                         = 0x6dc,
    SCIM_KEY_Cyrillic_shcha                     = 0x6dd,
    SCIM_KEY_Cyrillic_che                       = 0x6de,
    SCIM_KEY_Cyrillic_hardsign                  = 0x6df,
    SCIM_KEY_Cyrillic_YU                        = 0x6e0,
    SCIM_KEY_Cyrillic_A                         = 0x6e1,
    SCIM_KEY_Cyrillic_BE                        = 0x6e2,
    SCIM_KEY_Cyrillic_TSE                       = 0x6e3,
    SCIM_KEY_Cyrillic_DE                        = 0x6e4,
    SCIM_KEY_Cyrillic_IE                        = 0x6e5,
    SCIM_KEY_Cyrillic_EF                        = 0x6e6,
    SCIM_KEY_Cyrillic_GHE                       = 0x6e7,
    SCIM_KEY_Cyrillic_HA                        = 0x6e8,
    SCIM_KEY_Cyrillic_I                         = 0x6e9,
    SCIM_KEY_Cyrillic_SHORTI                    = 0x6ea,
    SCIM_KEY_Cyrillic_KA                        = 0x6eb,
    SCIM_KEY_Cyrillic_EL                        = 0x6ec,
    SCIM_KEY_Cyrillic_EM                        = 0x6ed,
    SCIM_KEY_Cyrillic_EN                        = 0x6ee,
    SCIM_KEY_Cyrillic_O                         = 0x6ef,
    SCIM_KEY_Cyrillic_PE                        = 0x6f0,
    SCIM_KEY_Cyrillic_YA                        = 0x6f1,
    SCIM_KEY_Cyrillic_ER                        = 0x6f2,
    SCIM_KEY_Cyrillic_ES                        = 0x6f3,
    SCIM_KEY_Cyrillic_TE                        = 0x6f4,
    SCIM_KEY_Cyrillic_U                         = 0x6f5,
    SCIM_KEY_Cyrillic_ZHE                       = 0x6f6,
    SCIM_KEY_Cyrillic_VE                        = 0x6f7,
    SCIM_KEY_Cyrillic_SOFTSIGN                  = 0x6f8,
    SCIM_KEY_Cyrillic_YERU                      = 0x6f9,
    SCIM_KEY_Cyrillic_ZE                        = 0x6fa,
    SCIM_KEY_Cyrillic_SHA                       = 0x6fb,
    SCIM_KEY_Cyrillic_E                         = 0x6fc,
    SCIM_KEY_Cyrillic_SHCHA                     = 0x6fd,
    SCIM_KEY_Cyrillic_CHE                       = 0x6fe,
    SCIM_KEY_Cyrillic_HARDSIGN                  = 0x6ff,

    /*
     * Greek
     * Byte 3 = 7
     */
    SCIM_KEY_Greek_ALPHAaccent                  = 0x7a1,
    SCIM_KEY_Greek_EPSILONaccent                = 0x7a2,
    SCIM_KEY_Greek_ETAaccent                    = 0x7a3,
    SCIM_KEY_Greek_IOTAaccent                   = 0x7a4,
    SCIM_KEY_Greek_IOTAdieresis                 = 0x7a5,
    SCIM_KEY_Greek_IOTAdiaeresis                = SCIM_KEY_Greek_IOTAdieresis, /* old typo */
    SCIM_KEY_Greek_OMICRONaccent                = 0x7a7,
    SCIM_KEY_Greek_UPSILONaccent                = 0x7a8,
    SCIM_KEY_Greek_UPSILONdieresis              = 0x7a9,
    SCIM_KEY_Greek_OMEGAaccent                  = 0x7ab,
    SCIM_KEY_Greek_accentdieresis               = 0x7ae,
    SCIM_KEY_Greek_horizbar                     = 0x7af,
    SCIM_KEY_Greek_alphaaccent                  = 0x7b1,
    SCIM_KEY_Greek_epsilonaccent                = 0x7b2,
    SCIM_KEY_Greek_etaaccent                    = 0x7b3,
    SCIM_KEY_Greek_iotaaccent                   = 0x7b4,
    SCIM_KEY_Greek_iotadieresis                 = 0x7b5,
    SCIM_KEY_Greek_iotaaccentdieresis           = 0x7b6,
    SCIM_KEY_Greek_omicronaccent                = 0x7b7,
    SCIM_KEY_Greek_upsilonaccent                = 0x7b8,
    SCIM_KEY_Greek_upsilondieresis              = 0x7b9,
    SCIM_KEY_Greek_upsilonaccentdieresis        = 0x7ba,
    SCIM_KEY_Greek_omegaaccent                  = 0x7bb,
    SCIM_KEY_Greek_ALPHA                        = 0x7c1,
    SCIM_KEY_Greek_BETA                         = 0x7c2,
    SCIM_KEY_Greek_GAMMA                        = 0x7c3,
    SCIM_KEY_Greek_DELTA                        = 0x7c4,
    SCIM_KEY_Greek_EPSILON                      = 0x7c5,
    SCIM_KEY_Greek_ZETA                         = 0x7c6,
    SCIM_KEY_Greek_ETA                          = 0x7c7,
    SCIM_KEY_Greek_THETA                        = 0x7c8,
    SCIM_KEY_Greek_IOTA                         = 0x7c9,
    SCIM_KEY_Greek_KAPPA                        = 0x7ca,
    SCIM_KEY_Greek_LAMDA                        = 0x7cb,
    SCIM_KEY_Greek_LAMBDA                       = 0x7cb,
    SCIM_KEY_Greek_MU                           = 0x7cc,
    SCIM_KEY_Greek_NU                           = 0x7cd,
    SCIM_KEY_Greek_XI                           = 0x7ce,
    SCIM_KEY_Greek_OMICRON                      = 0x7cf,
    SCIM_KEY_Greek_PI                           = 0x7d0,
    SCIM_KEY_Greek_RHO                          = 0x7d1,
    SCIM_KEY_Greek_SIGMA                        = 0x7d2,
    SCIM_KEY_Greek_TAU                          = 0x7d4,
    SCIM_KEY_Greek_UPSILON                      = 0x7d5,
    SCIM_KEY_Greek_PHI                          = 0x7d6,
    SCIM_KEY_Greek_CHI                          = 0x7d7,
    SCIM_KEY_Greek_PSI                          = 0x7d8,
    SCIM_KEY_Greek_OMEGA                        = 0x7d9,
    SCIM_KEY_Greek_alpha                        = 0x7e1,
    SCIM_KEY_Greek_beta                         = 0x7e2,
    SCIM_KEY_Greek_gamma                        = 0x7e3,
    SCIM_KEY_Greek_delta                        = 0x7e4,
    SCIM_KEY_Greek_epsilon                      = 0x7e5,
    SCIM_KEY_Greek_zeta                         = 0x7e6,
    SCIM_KEY_Greek_eta                          = 0x7e7,
    SCIM_KEY_Greek_theta                        = 0x7e8,
    SCIM_KEY_Greek_iota                         = 0x7e9,
    SCIM_KEY_Greek_kappa                        = 0x7ea,
    SCIM_KEY_Greek_lamda                        = 0x7eb,
    SCIM_KEY_Greek_lambda                       = 0x7eb,
    SCIM_KEY_Greek_mu                           = 0x7ec,
    SCIM_KEY_Greek_nu                           = 0x7ed,
    SCIM_KEY_Greek_xi                           = 0x7ee,
    SCIM_KEY_Greek_omicron                      = 0x7ef,
    SCIM_KEY_Greek_pi                           = 0x7f0,
    SCIM_KEY_Greek_rho                          = 0x7f1,
    SCIM_KEY_Greek_sigma                        = 0x7f2,
    SCIM_KEY_Greek_finalsmallsigma              = 0x7f3,
    SCIM_KEY_Greek_tau                          = 0x7f4,
    SCIM_KEY_Greek_upsilon                      = 0x7f5,
    SCIM_KEY_Greek_phi                          = 0x7f6,
    SCIM_KEY_Greek_chi                          = 0x7f7,
    SCIM_KEY_Greek_psi                          = 0x7f8,
    SCIM_KEY_Greek_omega                        = 0x7f9,
    SCIM_KEY_Greek_switch                       = 0xFF7E,  /* Alias for mode_switch */

    /*
     * Technical
     * Byte 3 = 8
     */
    SCIM_KEY_leftradical                        = 0x8a1,
    SCIM_KEY_topleftradical                     = 0x8a2,
    SCIM_KEY_horizconnector                     = 0x8a3,
    SCIM_KEY_topintegral                        = 0x8a4,
    SCIM_KEY_botintegral                        = 0x8a5,
    SCIM_KEY_vertconnector                      = 0x8a6,
    SCIM_KEY_topleftsqbracket                   = 0x8a7,
    SCIM_KEY_botleftsqbracket                   = 0x8a8,
    SCIM_KEY_toprightsqbracket                  = 0x8a9,
    SCIM_KEY_botrightsqbracket                  = 0x8aa,
    SCIM_KEY_topleftparens                      = 0x8ab,
    SCIM_KEY_botleftparens                      = 0x8ac,
    SCIM_KEY_toprightparens                     = 0x8ad,
    SCIM_KEY_botrightparens                     = 0x8ae,
    SCIM_KEY_leftmiddlecurlybrace               = 0x8af,
    SCIM_KEY_rightmiddlecurlybrace              = 0x8b0,
    SCIM_KEY_topleftsummation                   = 0x8b1,
    SCIM_KEY_botleftsummation                   = 0x8b2,
    SCIM_KEY_topvertsummationconnector          = 0x8b3,
    SCIM_KEY_botvertsummationconnector          = 0x8b4,
    SCIM_KEY_toprightsummation                  = 0x8b5,
    SCIM_KEY_botrightsummation                  = 0x8b6,
    SCIM_KEY_rightmiddlesummation               = 0x8b7,
    SCIM_KEY_lessthanequal                      = 0x8bc,
    SCIM_KEY_notequal                           = 0x8bd,
    SCIM_KEY_greaterthanequal                   = 0x8be,
    SCIM_KEY_integral                           = 0x8bf,
    SCIM_KEY_therefore                          = 0x8c0,
    SCIM_KEY_variation                          = 0x8c1,
    SCIM_KEY_infinity                           = 0x8c2,
    SCIM_KEY_nabla                              = 0x8c5,
    SCIM_KEY_approximate                        = 0x8c8,
    SCIM_KEY_similarequal                       = 0x8c9,
    SCIM_KEY_ifonlyif                           = 0x8cd,
    SCIM_KEY_implies                            = 0x8ce,
    SCIM_KEY_identical                          = 0x8cf,
    SCIM_KEY_radical                            = 0x8d6,
    SCIM_KEY_includedin                         = 0x8da,
    SCIM_KEY_includes                           = 0x8db,
    SCIM_KEY_intersection                       = 0x8dc,
    SCIM_KEY_union                              = 0x8dd,
    SCIM_KEY_logicaland                         = 0x8de,
    SCIM_KEY_logicalor                          = 0x8df,
    SCIM_KEY_partialderivative                  = 0x8ef,
    SCIM_KEY_function                           = 0x8f6,
    SCIM_KEY_leftarrow                          = 0x8fb,
    SCIM_KEY_uparrow                            = 0x8fc,
    SCIM_KEY_rightarrow                         = 0x8fd,
    SCIM_KEY_downarrow                          = 0x8fe,

    /*
     * Special
     * Byte 3 = 9
     */
    SCIM_KEY_blank                              = 0x9df,
    SCIM_KEY_soliddiamond                       = 0x9e0,
    SCIM_KEY_checkerboard                       = 0x9e1,
    SCIM_KEY_ht                                 = 0x9e2,
    SCIM_KEY_ff                                 = 0x9e3,
    SCIM_KEY_cr                                 = 0x9e4,
    SCIM_KEY_lf                                 = 0x9e5,
    SCIM_KEY_nl                                 = 0x9e8,
    SCIM_KEY_vt                                 = 0x9e9,
    SCIM_KEY_lowrightcorner                     = 0x9ea,
    SCIM_KEY_uprightcorner                      = 0x9eb,
    SCIM_KEY_upleftcorner                       = 0x9ec,
    SCIM_KEY_lowleftcorner                      = 0x9ed,
    SCIM_KEY_crossinglines                      = 0x9ee,
    SCIM_KEY_horizlinescan1                     = 0x9ef,
    SCIM_KEY_horizlinescan3                     = 0x9f0,
    SCIM_KEY_horizlinescan5                     = 0x9f1,
    SCIM_KEY_horizlinescan7                     = 0x9f2,
    SCIM_KEY_horizlinescan9                     = 0x9f3,
    SCIM_KEY_leftt                              = 0x9f4,
    SCIM_KEY_rightt                             = 0x9f5,
    SCIM_KEY_bott                               = 0x9f6,
    SCIM_KEY_topt                               = 0x9f7,
    SCIM_KEY_vertbar                            = 0x9f8,

    /*
     * Publishing
     * Byte 3 = a
     */
    SCIM_KEY_emspace                            = 0xaa1,
    SCIM_KEY_enspace                            = 0xaa2,
    SCIM_KEY_em3space                           = 0xaa3,
    SCIM_KEY_em4space                           = 0xaa4,
    SCIM_KEY_digitspace                         = 0xaa5,
    SCIM_KEY_punctspace                         = 0xaa6,
    SCIM_KEY_thinspace                          = 0xaa7,
    SCIM_KEY_hairspace                          = 0xaa8,
    SCIM_KEY_emdash                             = 0xaa9,
    SCIM_KEY_endash                             = 0xaaa,
    SCIM_KEY_signifblank                        = 0xaac,
    SCIM_KEY_ellipsis                           = 0xaae,
    SCIM_KEY_doubbaselinedot                    = 0xaaf,
    SCIM_KEY_onethird                           = 0xab0,
    SCIM_KEY_twothirds                          = 0xab1,
    SCIM_KEY_onefifth                           = 0xab2,
    SCIM_KEY_twofifths                          = 0xab3,
    SCIM_KEY_threefifths                        = 0xab4,
    SCIM_KEY_fourfifths                         = 0xab5,
    SCIM_KEY_onesixth                           = 0xab6,
    SCIM_KEY_fivesixths                         = 0xab7,
    SCIM_KEY_careof                             = 0xab8,
    SCIM_KEY_figdash                            = 0xabb,
    SCIM_KEY_leftanglebracket                   = 0xabc,
    SCIM_KEY_decimalpoint                       = 0xabd,
    SCIM_KEY_rightanglebracket                  = 0xabe,
    SCIM_KEY_marker                             = 0xabf,
    SCIM_KEY_oneeighth                          = 0xac3,
    SCIM_KEY_threeeighths                       = 0xac4,
    SCIM_KEY_fiveeighths                        = 0xac5,
    SCIM_KEY_seveneighths                       = 0xac6,
    SCIM_KEY_trademark                          = 0xac9,
    SCIM_KEY_signaturemark                      = 0xaca,
    SCIM_KEY_trademarkincircle                  = 0xacb,
    SCIM_KEY_leftopentriangle                   = 0xacc,
    SCIM_KEY_rightopentriangle                  = 0xacd,
    SCIM_KEY_emopencircle                       = 0xace,
    SCIM_KEY_emopenrectangle                    = 0xacf,
    SCIM_KEY_leftsinglequotemark                = 0xad0,
    SCIM_KEY_rightsinglequotemark               = 0xad1,
    SCIM_KEY_leftdoublequotemark                = 0xad2,
    SCIM_KEY_rightdoublequotemark               = 0xad3,
    SCIM_KEY_prescription                       = 0xad4,
    SCIM_KEY_minutes                            = 0xad6,
    SCIM_KEY_seconds                            = 0xad7,
    SCIM_KEY_latincross                         = 0xad9,
    SCIM_KEY_hexagram                           = 0xada,
    SCIM_KEY_filledrectbullet                   = 0xadb,
    SCIM_KEY_filledlefttribullet                = 0xadc,
    SCIM_KEY_filledrighttribullet               = 0xadd,
    SCIM_KEY_emfilledcircle                     = 0xade,
    SCIM_KEY_emfilledrect                       = 0xadf,
    SCIM_KEY_enopencircbullet                   = 0xae0,
    SCIM_KEY_enopensquarebullet                 = 0xae1,
    SCIM_KEY_openrectbullet                     = 0xae2,
    SCIM_KEY_opentribulletup                    = 0xae3,
    SCIM_KEY_opentribulletdown                  = 0xae4,
    SCIM_KEY_openstar                           = 0xae5,
    SCIM_KEY_enfilledcircbullet                 = 0xae6,
    SCIM_KEY_enfilledsqbullet                   = 0xae7,
    SCIM_KEY_filledtribulletup                  = 0xae8,
    SCIM_KEY_filledtribulletdown                = 0xae9,
    SCIM_KEY_leftpointer                        = 0xaea,
    SCIM_KEY_rightpointer                       = 0xaeb,
    SCIM_KEY_club                               = 0xaec,
    SCIM_KEY_diamond                            = 0xaed,
    SCIM_KEY_heart                              = 0xaee,
    SCIM_KEY_maltesecross                       = 0xaf0,
    SCIM_KEY_dagger                             = 0xaf1,
    SCIM_KEY_doubledagger                       = 0xaf2,
    SCIM_KEY_checkmark                          = 0xaf3,
    SCIM_KEY_ballotcross                        = 0xaf4,
    SCIM_KEY_musicalsharp                       = 0xaf5,
    SCIM_KEY_musicalflat                        = 0xaf6,
    SCIM_KEY_malesymbol                         = 0xaf7,
    SCIM_KEY_femalesymbol                       = 0xaf8,
    SCIM_KEY_telephone                          = 0xaf9,
    SCIM_KEY_telephonerecorder                  = 0xafa,
    SCIM_KEY_phonographcopyright                = 0xafb,
    SCIM_KEY_caret                              = 0xafc,
    SCIM_KEY_singlelowquotemark                 = 0xafd,
    SCIM_KEY_doublelowquotemark                 = 0xafe,
    SCIM_KEY_cursor                             = 0xaff,

    /*
     * APL
     * Byte 3 = b
     */
    SCIM_KEY_leftcaret                          = 0xba3,
    SCIM_KEY_rightcaret                         = 0xba6,
    SCIM_KEY_downcaret                          = 0xba8,
    SCIM_KEY_upcaret                            = 0xba9,
    SCIM_KEY_overbar                            = 0xbc0,
    SCIM_KEY_downtack                           = 0xbc2,
    SCIM_KEY_upshoe                             = 0xbc3,
    SCIM_KEY_downstile                          = 0xbc4,
    SCIM_KEY_underbar                           = 0xbc6,
    SCIM_KEY_jot                                = 0xbca,
    SCIM_KEY_quad                               = 0xbcc,
    SCIM_KEY_uptack                             = 0xbce,
    SCIM_KEY_circle                             = 0xbcf,
    SCIM_KEY_upstile                            = 0xbd3,
    SCIM_KEY_downshoe                           = 0xbd6,
    SCIM_KEY_rightshoe                          = 0xbd8,
    SCIM_KEY_leftshoe                           = 0xbda,
    SCIM_KEY_lefttack                           = 0xbdc,
    SCIM_KEY_righttack                          = 0xbfc,

    /*
     * Hebrew
     * Byte 3 = c
     */
    SCIM_KEY_hebrew_doublelowline               = 0xcdf,
    SCIM_KEY_hebrew_aleph                       = 0xce0,
    SCIM_KEY_hebrew_bet                         = 0xce1,
    SCIM_KEY_hebrew_beth                        = 0xce1,  /* deprecated */
    SCIM_KEY_hebrew_gimel                       = 0xce2,
    SCIM_KEY_hebrew_gimmel                      = 0xce2,  /* deprecated */
    SCIM_KEY_hebrew_dalet                       = 0xce3,
    SCIM_KEY_hebrew_daleth                      = 0xce3,  /* deprecated */
    SCIM_KEY_hebrew_he                          = 0xce4,
    SCIM_KEY_hebrew_waw                         = 0xce5,
    SCIM_KEY_hebrew_zain                        = 0xce6,
    SCIM_KEY_hebrew_zayin                       = 0xce6,  /* deprecated */
    SCIM_KEY_hebrew_chet                        = 0xce7,
    SCIM_KEY_hebrew_het                         = 0xce7,  /* deprecated */
    SCIM_KEY_hebrew_tet                         = 0xce8,
    SCIM_KEY_hebrew_teth                        = 0xce8,  /* deprecated */
    SCIM_KEY_hebrew_yod                         = 0xce9,
    SCIM_KEY_hebrew_finalkaph                   = 0xcea,
    SCIM_KEY_hebrew_kaph                        = 0xceb,
    SCIM_KEY_hebrew_lamed                       = 0xcec,
    SCIM_KEY_hebrew_finalmem                    = 0xced,
    SCIM_KEY_hebrew_mem                         = 0xcee,
    SCIM_KEY_hebrew_finalnun                    = 0xcef,
    SCIM_KEY_hebrew_nun                         = 0xcf0,
    SCIM_KEY_hebrew_samech                      = 0xcf1,
    SCIM_KEY_hebrew_samekh                      = 0xcf1,  /* deprecated */
    SCIM_KEY_hebrew_ayin                        = 0xcf2,
    SCIM_KEY_hebrew_finalpe                     = 0xcf3,
    SCIM_KEY_hebrew_pe                          = 0xcf4,
    SCIM_KEY_hebrew_finalzade                   = 0xcf5,
    SCIM_KEY_hebrew_finalzadi                   = 0xcf5,  /* deprecated */
    SCIM_KEY_hebrew_zade                        = 0xcf6,
    SCIM_KEY_hebrew_zadi                        = 0xcf6,  /* deprecated */
    SCIM_KEY_hebrew_qoph                        = 0xcf7,
    SCIM_KEY_hebrew_kuf                         = 0xcf7,  /* deprecated */
    SCIM_KEY_hebrew_resh                        = 0xcf8,
    SCIM_KEY_hebrew_shin                        = 0xcf9,
    SCIM_KEY_hebrew_taw                         = 0xcfa,
    SCIM_KEY_hebrew_taf                         = 0xcfa,  /* deprecated */
    SCIM_KEY_Hebrew_switch                      = 0xFF7E,  /* Alias for mode_switch */

    /*
     * Thai
     * Byte 3 = d
     */
    SCIM_KEY_Thai_kokai                         = 0xda1,
    SCIM_KEY_Thai_khokhai                       = 0xda2,
    SCIM_KEY_Thai_khokhuat                      = 0xda3,
    SCIM_KEY_Thai_khokhwai                      = 0xda4,
    SCIM_KEY_Thai_khokhon                       = 0xda5,
    SCIM_KEY_Thai_khorakhang                    = 0xda6,
    SCIM_KEY_Thai_ngongu                        = 0xda7,
    SCIM_KEY_Thai_chochan                       = 0xda8,
    SCIM_KEY_Thai_choching                      = 0xda9,
    SCIM_KEY_Thai_chochang                      = 0xdaa,
    SCIM_KEY_Thai_soso                          = 0xdab,
    SCIM_KEY_Thai_chochoe                       = 0xdac,
    SCIM_KEY_Thai_yoying                        = 0xdad,
    SCIM_KEY_Thai_dochada                       = 0xdae,
    SCIM_KEY_Thai_topatak                       = 0xdaf,
    SCIM_KEY_Thai_thothan                       = 0xdb0,
    SCIM_KEY_Thai_thonangmontho                 = 0xdb1,
    SCIM_KEY_Thai_thophuthao                    = 0xdb2,
    SCIM_KEY_Thai_nonen                         = 0xdb3,
    SCIM_KEY_Thai_dodek                         = 0xdb4,
    SCIM_KEY_Thai_totao                         = 0xdb5,
    SCIM_KEY_Thai_thothung                      = 0xdb6,
    SCIM_KEY_Thai_thothahan                     = 0xdb7,
    SCIM_KEY_Thai_thothong                      = 0xdb8,
    SCIM_KEY_Thai_nonu                          = 0xdb9,
    SCIM_KEY_Thai_bobaimai                      = 0xdba,
    SCIM_KEY_Thai_popla                         = 0xdbb,
    SCIM_KEY_Thai_phophung                      = 0xdbc,
    SCIM_KEY_Thai_fofa                          = 0xdbd,
    SCIM_KEY_Thai_phophan                       = 0xdbe,
    SCIM_KEY_Thai_fofan                         = 0xdbf,
    SCIM_KEY_Thai_phosamphao                    = 0xdc0,
    SCIM_KEY_Thai_moma                          = 0xdc1,
    SCIM_KEY_Thai_yoyak                         = 0xdc2,
    SCIM_KEY_Thai_rorua                         = 0xdc3,
    SCIM_KEY_Thai_ru                            = 0xdc4,
    SCIM_KEY_Thai_loling                        = 0xdc5,
    SCIM_KEY_Thai_lu                            = 0xdc6,
    SCIM_KEY_Thai_wowaen                        = 0xdc7,
    SCIM_KEY_Thai_sosala                        = 0xdc8,
    SCIM_KEY_Thai_sorusi                        = 0xdc9,
    SCIM_KEY_Thai_sosua                         = 0xdca,
    SCIM_KEY_Thai_hohip                         = 0xdcb,
    SCIM_KEY_Thai_lochula                       = 0xdcc,
    SCIM_KEY_Thai_oang                          = 0xdcd,
    SCIM_KEY_Thai_honokhuk                      = 0xdce,
    SCIM_KEY_Thai_paiyannoi                     = 0xdcf,
    SCIM_KEY_Thai_saraa                         = 0xdd0,
    SCIM_KEY_Thai_maihanakat                    = 0xdd1,
    SCIM_KEY_Thai_saraaa                        = 0xdd2,
    SCIM_KEY_Thai_saraam                        = 0xdd3,
    SCIM_KEY_Thai_sarai                         = 0xdd4,
    SCIM_KEY_Thai_saraii                        = 0xdd5,
    SCIM_KEY_Thai_saraue                        = 0xdd6,
    SCIM_KEY_Thai_sarauee                       = 0xdd7,
    SCIM_KEY_Thai_sarau                         = 0xdd8,
    SCIM_KEY_Thai_sarauu                        = 0xdd9,
    SCIM_KEY_Thai_phinthu                       = 0xdda,
    SCIM_KEY_Thai_maihanakat_maitho             = 0xdde,
    SCIM_KEY_Thai_baht                          = 0xddf,
    SCIM_KEY_Thai_sarae                         = 0xde0,
    SCIM_KEY_Thai_saraae                        = 0xde1,
    SCIM_KEY_Thai_sarao                         = 0xde2,
    SCIM_KEY_Thai_saraaimaimuan                 = 0xde3,
    SCIM_KEY_Thai_saraaimaimalai                = 0xde4,
    SCIM_KEY_Thai_lakkhangyao                   = 0xde5,
    SCIM_KEY_Thai_maiyamok                      = 0xde6,
    SCIM_KEY_Thai_maitaikhu                     = 0xde7,
    SCIM_KEY_Thai_maiek                         = 0xde8,
    SCIM_KEY_Thai_maitho                        = 0xde9,
    SCIM_KEY_Thai_maitri                        = 0xdea,
    SCIM_KEY_Thai_maichattawa                   = 0xdeb,
    SCIM_KEY_Thai_thanthakhat                   = 0xdec,
    SCIM_KEY_Thai_nikhahit                      = 0xded,
    SCIM_KEY_Thai_leksun                        = 0xdf0,
    SCIM_KEY_Thai_leknung                       = 0xdf1,
    SCIM_KEY_Thai_leksong                       = 0xdf2,
    SCIM_KEY_Thai_leksam                        = 0xdf3,
    SCIM_KEY_Thai_leksi                         = 0xdf4,
    SCIM_KEY_Thai_lekha                         = 0xdf5,
    SCIM_KEY_Thai_lekhok                        = 0xdf6,
    SCIM_KEY_Thai_lekchet                       = 0xdf7,
    SCIM_KEY_Thai_lekpaet                       = 0xdf8,
    SCIM_KEY_Thai_lekkao                        = 0xdf9,

    /*
     *   Korean
     *   Byte 3 = e
     */
    SCIM_KEY_Hangul                             = 0xff31,    /* Hangul start/stop(toggle) */
    SCIM_KEY_Hangul_Start                       = 0xff32,    /* Hangul start */
    SCIM_KEY_Hangul_End                         = 0xff33,    /* Hangul end, English start */
    SCIM_KEY_Hangul_Hanja                       = 0xff34,    /* Start Hangul->Hanja Conversion */
    SCIM_KEY_Hangul_Jamo                        = 0xff35,    /* Hangul Jamo mode */
    SCIM_KEY_Hangul_Romaja                      = 0xff36,    /* Hangul Romaja mode */
    SCIM_KEY_Hangul_Codeinput                   = 0xff37,    /* Hangul code input mode */
    SCIM_KEY_Hangul_Jeonja                      = 0xff38,    /* Jeonja mode */
    SCIM_KEY_Hangul_Banja                       = 0xff39,    /* Banja mode */
    SCIM_KEY_Hangul_PreHanja                    = 0xff3a,    /* Pre Hanja conversion */
    SCIM_KEY_Hangul_PostHanja                   = 0xff3b,    /* Post Hanja conversion */
    SCIM_KEY_Hangul_SingleCandidate             = 0xff3c,    /* Single candidate */
    SCIM_KEY_Hangul_MultipleCandidate           = 0xff3d,    /* Multiple candidate */
    SCIM_KEY_Hangul_PreviousCandidate           = 0xff3e,    /* Previous candidate */
    SCIM_KEY_Hangul_Special                     = 0xff3f,    /* Special symbols */
    SCIM_KEY_Hangul_switch                      = 0xFF7E,    /* Alias for mode_switch */

    /* Hangul Consonant Characters */
    SCIM_KEY_Hangul_Kiyeog                      = 0xea1,
    SCIM_KEY_Hangul_SsangKiyeog                 = 0xea2,
    SCIM_KEY_Hangul_KiyeogSios                  = 0xea3,
    SCIM_KEY_Hangul_Nieun                       = 0xea4,
    SCIM_KEY_Hangul_NieunJieuj                  = 0xea5,
    SCIM_KEY_Hangul_NieunHieuh                  = 0xea6,
    SCIM_KEY_Hangul_Dikeud                      = 0xea7,
    SCIM_KEY_Hangul_SsangDikeud                 = 0xea8,
    SCIM_KEY_Hangul_Rieul                       = 0xea9,
    SCIM_KEY_Hangul_RieulKiyeog                 = 0xeaa,
    SCIM_KEY_Hangul_RieulMieum                  = 0xeab,
    SCIM_KEY_Hangul_RieulPieub                  = 0xeac,
    SCIM_KEY_Hangul_RieulSios                   = 0xead,
    SCIM_KEY_Hangul_RieulTieut                  = 0xeae,
    SCIM_KEY_Hangul_RieulPhieuf                 = 0xeaf,
    SCIM_KEY_Hangul_RieulHieuh                  = 0xeb0,
    SCIM_KEY_Hangul_Mieum                       = 0xeb1,
    SCIM_KEY_Hangul_Pieub                       = 0xeb2,
    SCIM_KEY_Hangul_SsangPieub                  = 0xeb3,
    SCIM_KEY_Hangul_PieubSios                   = 0xeb4,
    SCIM_KEY_Hangul_Sios                        = 0xeb5,
    SCIM_KEY_Hangul_SsangSios                   = 0xeb6,
    SCIM_KEY_Hangul_Ieung                       = 0xeb7,
    SCIM_KEY_Hangul_Jieuj                       = 0xeb8,
    SCIM_KEY_Hangul_SsangJieuj                  = 0xeb9,
    SCIM_KEY_Hangul_Cieuc                       = 0xeba,
    SCIM_KEY_Hangul_Khieuq                      = 0xebb,
    SCIM_KEY_Hangul_Tieut                       = 0xebc,
    SCIM_KEY_Hangul_Phieuf                      = 0xebd,
    SCIM_KEY_Hangul_Hieuh                       = 0xebe,

    /* Hangul Vowel Characters */
    SCIM_KEY_Hangul_A                           = 0xebf,
    SCIM_KEY_Hangul_AE                          = 0xec0,
    SCIM_KEY_Hangul_YA                          = 0xec1,
    SCIM_KEY_Hangul_YAE                         = 0xec2,
    SCIM_KEY_Hangul_EO                          = 0xec3,
    SCIM_KEY_Hangul_E                           = 0xec4,
    SCIM_KEY_Hangul_YEO                         = 0xec5,
    SCIM_KEY_Hangul_YE                          = 0xec6,
    SCIM_KEY_Hangul_O                           = 0xec7,
    SCIM_KEY_Hangul_WA                          = 0xec8,
    SCIM_KEY_Hangul_WAE                         = 0xec9,
    SCIM_KEY_Hangul_OE                          = 0xeca,
    SCIM_KEY_Hangul_YO                          = 0xecb,
    SCIM_KEY_Hangul_U                           = 0xecc,
    SCIM_KEY_Hangul_WEO                         = 0xecd,
    SCIM_KEY_Hangul_WE                          = 0xece,
    SCIM_KEY_Hangul_WI                          = 0xecf,
    SCIM_KEY_Hangul_YU                          = 0xed0,
    SCIM_KEY_Hangul_EU                          = 0xed1,
    SCIM_KEY_Hangul_YI                          = 0xed2,
    SCIM_KEY_Hangul_I                           = 0xed3,

    /* Hangul syllable-final (JongSeong) Characters */
    SCIM_KEY_Hangul_J_Kiyeog                    = 0xed4,
    SCIM_KEY_Hangul_J_SsangKiyeog               = 0xed5,
    SCIM_KEY_Hangul_J_KiyeogSios                = 0xed6,
    SCIM_KEY_Hangul_J_Nieun                     = 0xed7,
    SCIM_KEY_Hangul_J_NieunJieuj                = 0xed8,
    SCIM_KEY_Hangul_J_NieunHieuh                = 0xed9,
    SCIM_KEY_Hangul_J_Dikeud                    = 0xeda,
    SCIM_KEY_Hangul_J_Rieul                     = 0xedb,
    SCIM_KEY_Hangul_J_RieulKiyeog               = 0xedc,
    SCIM_KEY_Hangul_J_RieulMieum                = 0xedd,
    SCIM_KEY_Hangul_J_RieulPieub                = 0xede,
    SCIM_KEY_Hangul_J_RieulSios                 = 0xedf,
    SCIM_KEY_Hangul_J_RieulTieut                = 0xee0,
    SCIM_KEY_Hangul_J_RieulPhieuf               = 0xee1,
    SCIM_KEY_Hangul_J_RieulHieuh                = 0xee2,
    SCIM_KEY_Hangul_J_Mieum                     = 0xee3,
    SCIM_KEY_Hangul_J_Pieub                     = 0xee4,
    SCIM_KEY_Hangul_J_PieubSios                 = 0xee5,
    SCIM_KEY_Hangul_J_Sios                      = 0xee6,
    SCIM_KEY_Hangul_J_SsangSios                 = 0xee7,
    SCIM_KEY_Hangul_J_Ieung                     = 0xee8,
    SCIM_KEY_Hangul_J_Jieuj                     = 0xee9,
    SCIM_KEY_Hangul_J_Cieuc                     = 0xeea,
    SCIM_KEY_Hangul_J_Khieuq                    = 0xeeb,
    SCIM_KEY_Hangul_J_Tieut                     = 0xeec,
    SCIM_KEY_Hangul_J_Phieuf                    = 0xeed,
    SCIM_KEY_Hangul_J_Hieuh                     = 0xeee,

    /* Ancient Hangul Consonant Characters */
    SCIM_KEY_Hangul_RieulYeorinHieuh            = 0xeef,
    SCIM_KEY_Hangul_SunkyeongeumMieum           = 0xef0,
    SCIM_KEY_Hangul_SunkyeongeumPieub           = 0xef1,
    SCIM_KEY_Hangul_PanSios                     = 0xef2,
    SCIM_KEY_Hangul_KkogjiDalrinIeung           = 0xef3,
    SCIM_KEY_Hangul_SunkyeongeumPhieuf          = 0xef4,
    SCIM_KEY_Hangul_YeorinHieuh                 = 0xef5,

    /* Ancient Hangul Vowel Characters */
    SCIM_KEY_Hangul_AraeA                       = 0xef6,
    SCIM_KEY_Hangul_AraeAE                      = 0xef7,

    /* Ancient Hangul syllable-final (JongSeong) Characters */
    SCIM_KEY_Hangul_J_PanSios                   = 0xef8,
    SCIM_KEY_Hangul_J_KkogjiDalrinIeung         = 0xef9,
    SCIM_KEY_Hangul_J_YeorinHieuh               = 0xefa,

    /* Korean currency symbol */
    SCIM_KEY_Korean_Won                         = 0xeff,


    /*
     *   Armenian
     *   Byte 3 = 0x14
     */
    SCIM_KEY_Armenian_eternity                  = 0x14a1,
    SCIM_KEY_Armenian_ligature_ew               = 0x14a2,
    SCIM_KEY_Armenian_full_stop                 = 0x14a3,
    SCIM_KEY_Armenian_verjaket                  = 0x14a3,
    SCIM_KEY_Armenian_parenright                = 0x14a4,
    SCIM_KEY_Armenian_parenleft                 = 0x14a5,
    SCIM_KEY_Armenian_guillemotright            = 0x14a6,
    SCIM_KEY_Armenian_guillemotleft             = 0x14a7,
    SCIM_KEY_Armenian_em_dash                   = 0x14a8,
    SCIM_KEY_Armenian_dot                       = 0x14a9,
    SCIM_KEY_Armenian_mijaket                   = 0x14a9,
    SCIM_KEY_Armenian_separation_mark           = 0x14aa,
    SCIM_KEY_Armenian_but                       = 0x14aa,
    SCIM_KEY_Armenian_comma                     = 0x14ab,
    SCIM_KEY_Armenian_en_dash                   = 0x14ac,
    SCIM_KEY_Armenian_hyphen                    = 0x14ad,
    SCIM_KEY_Armenian_yentamna                  = 0x14ad,
    SCIM_KEY_Armenian_ellipsis                  = 0x14ae,
    SCIM_KEY_Armenian_exclam                    = 0x14af,
    SCIM_KEY_Armenian_amanak                    = 0x14af,
    SCIM_KEY_Armenian_accent                    = 0x14b0,
    SCIM_KEY_Armenian_shesht                    = 0x14b0,
    SCIM_KEY_Armenian_question                  = 0x14b1,
    SCIM_KEY_Armenian_paruyk                    = 0x14b1,
    SCIM_KEY_Armenian_AYB                       = 0x14b2,
    SCIM_KEY_Armenian_ayb                       = 0x14b3,
    SCIM_KEY_Armenian_BEN                       = 0x14b4,
    SCIM_KEY_Armenian_ben                       = 0x14b5,
    SCIM_KEY_Armenian_GIM                       = 0x14b6,
    SCIM_KEY_Armenian_gim                       = 0x14b7,
    SCIM_KEY_Armenian_DA                        = 0x14b8,
    SCIM_KEY_Armenian_da                        = 0x14b9,
    SCIM_KEY_Armenian_YECH                      = 0x14ba,
    SCIM_KEY_Armenian_yech                      = 0x14bb,
    SCIM_KEY_Armenian_ZA                        = 0x14bc,
    SCIM_KEY_Armenian_za                        = 0x14bd,
    SCIM_KEY_Armenian_E                         = 0x14be,
    SCIM_KEY_Armenian_e                         = 0x14bf,
    SCIM_KEY_Armenian_AT                        = 0x14c0,
    SCIM_KEY_Armenian_at                        = 0x14c1,
    SCIM_KEY_Armenian_TO                        = 0x14c2,
    SCIM_KEY_Armenian_to                        = 0x14c3,
    SCIM_KEY_Armenian_ZHE                       = 0x14c4,
    SCIM_KEY_Armenian_zhe                       = 0x14c5,
    SCIM_KEY_Armenian_INI                       = 0x14c6,
    SCIM_KEY_Armenian_ini                       = 0x14c7,
    SCIM_KEY_Armenian_LYUN                      = 0x14c8,
    SCIM_KEY_Armenian_lyun                      = 0x14c9,
    SCIM_KEY_Armenian_KHE                       = 0x14ca,
    SCIM_KEY_Armenian_khe                       = 0x14cb,
    SCIM_KEY_Armenian_TSA                       = 0x14cc,
    SCIM_KEY_Armenian_tsa                       = 0x14cd,
    SCIM_KEY_Armenian_KEN                       = 0x14ce,
    SCIM_KEY_Armenian_ken                       = 0x14cf,
    SCIM_KEY_Armenian_HO                        = 0x14d0,
    SCIM_KEY_Armenian_ho                        = 0x14d1,
    SCIM_KEY_Armenian_DZA                       = 0x14d2,
    SCIM_KEY_Armenian_dza                       = 0x14d3,
    SCIM_KEY_Armenian_GHAT                      = 0x14d4,
    SCIM_KEY_Armenian_ghat                      = 0x14d5,
    SCIM_KEY_Armenian_TCHE                      = 0x14d6,
    SCIM_KEY_Armenian_tche                      = 0x14d7,
    SCIM_KEY_Armenian_MEN                       = 0x14d8,
    SCIM_KEY_Armenian_men                       = 0x14d9,
    SCIM_KEY_Armenian_HI                        = 0x14da,
    SCIM_KEY_Armenian_hi                        = 0x14db,
    SCIM_KEY_Armenian_NU                        = 0x14dc,
    SCIM_KEY_Armenian_nu                        = 0x14dd,
    SCIM_KEY_Armenian_SHA                       = 0x14de,
    SCIM_KEY_Armenian_sha                       = 0x14df,
    SCIM_KEY_Armenian_VO                        = 0x14e0,
    SCIM_KEY_Armenian_vo                        = 0x14e1,
    SCIM_KEY_Armenian_CHA                       = 0x14e2,
    SCIM_KEY_Armenian_cha                       = 0x14e3,
    SCIM_KEY_Armenian_PE                        = 0x14e4,
    SCIM_KEY_Armenian_pe                        = 0x14e5,
    SCIM_KEY_Armenian_JE                        = 0x14e6,
    SCIM_KEY_Armenian_je                        = 0x14e7,
    SCIM_KEY_Armenian_RA                        = 0x14e8,
    SCIM_KEY_Armenian_ra                        = 0x14e9,
    SCIM_KEY_Armenian_SE                        = 0x14ea,
    SCIM_KEY_Armenian_se                        = 0x14eb,
    SCIM_KEY_Armenian_VEV                       = 0x14ec,
    SCIM_KEY_Armenian_vev                       = 0x14ed,
    SCIM_KEY_Armenian_TYUN                      = 0x14ee,
    SCIM_KEY_Armenian_tyun                      = 0x14ef,
    SCIM_KEY_Armenian_RE                        = 0x14f0,
    SCIM_KEY_Armenian_re                        = 0x14f1,
    SCIM_KEY_Armenian_TSO                       = 0x14f2,
    SCIM_KEY_Armenian_tso                       = 0x14f3,
    SCIM_KEY_Armenian_VYUN                      = 0x14f4,
    SCIM_KEY_Armenian_vyun                      = 0x14f5,
    SCIM_KEY_Armenian_PYUR                      = 0x14f6,
    SCIM_KEY_Armenian_pyur                      = 0x14f7,
    SCIM_KEY_Armenian_KE                        = 0x14f8,
    SCIM_KEY_Armenian_ke                        = 0x14f9,
    SCIM_KEY_Armenian_O                         = 0x14fa,
    SCIM_KEY_Armenian_o                         = 0x14fb,
    SCIM_KEY_Armenian_FE                        = 0x14fc,
    SCIM_KEY_Armenian_fe                        = 0x14fd,
    SCIM_KEY_Armenian_apostrophe                = 0x14fe,
    SCIM_KEY_Armenian_section_sign              = 0x14ff,

    /*
     * Georgian
     * Byte 3 = 0x15
     */

    SCIM_KEY_Georgian_an                        = 0x15d0,
    SCIM_KEY_Georgian_ban                       = 0x15d1,
    SCIM_KEY_Georgian_gan                       = 0x15d2,
    SCIM_KEY_Georgian_don                       = 0x15d3,
    SCIM_KEY_Georgian_en                        = 0x15d4,
    SCIM_KEY_Georgian_vin                       = 0x15d5,
    SCIM_KEY_Georgian_zen                       = 0x15d6,
    SCIM_KEY_Georgian_tan                       = 0x15d7,
    SCIM_KEY_Georgian_in                        = 0x15d8,
    SCIM_KEY_Georgian_kan                       = 0x15d9,
    SCIM_KEY_Georgian_las                       = 0x15da,
    SCIM_KEY_Georgian_man                       = 0x15db,
    SCIM_KEY_Georgian_nar                       = 0x15dc,
    SCIM_KEY_Georgian_on                        = 0x15dd,
    SCIM_KEY_Georgian_par                       = 0x15de,
    SCIM_KEY_Georgian_zhar                      = 0x15df,
    SCIM_KEY_Georgian_rae                       = 0x15e0,
    SCIM_KEY_Georgian_san                       = 0x15e1,
    SCIM_KEY_Georgian_tar                       = 0x15e2,
    SCIM_KEY_Georgian_un                        = 0x15e3,
    SCIM_KEY_Georgian_phar                      = 0x15e4,
    SCIM_KEY_Georgian_khar                      = 0x15e5,
    SCIM_KEY_Georgian_ghan                      = 0x15e6,
    SCIM_KEY_Georgian_qar                       = 0x15e7,
    SCIM_KEY_Georgian_shin                      = 0x15e8,
    SCIM_KEY_Georgian_chin                      = 0x15e9,
    SCIM_KEY_Georgian_can                       = 0x15ea,
    SCIM_KEY_Georgian_jil                       = 0x15eb,
    SCIM_KEY_Georgian_cil                       = 0x15ec,
    SCIM_KEY_Georgian_char                      = 0x15ed,
    SCIM_KEY_Georgian_xan                       = 0x15ee,
    SCIM_KEY_Georgian_jhan                      = 0x15ef,
    SCIM_KEY_Georgian_hae                       = 0x15f0,
    SCIM_KEY_Georgian_he                        = 0x15f1,
    SCIM_KEY_Georgian_hie                       = 0x15f2,
    SCIM_KEY_Georgian_we                        = 0x15f3,
    SCIM_KEY_Georgian_har                       = 0x15f4,
    SCIM_KEY_Georgian_hoe                       = 0x15f5,
    SCIM_KEY_Georgian_fi                        = 0x15f6,

    /*
     * Azeri (and other Turkic or Caucasian languages of ex-USSR)
     * Byte 3 = 0x16
     */

    /* latin */
    SCIM_KEY_Ccedillaabovedot                   = 0x16a2,
    SCIM_KEY_Xabovedot                          = 0x16a3,
    SCIM_KEY_Qabovedot                          = 0x16a5,
    SCIM_KEY_Ibreve                             = 0x16a6,
    SCIM_KEY_IE                                 = 0x16a7,
    SCIM_KEY_UO                                 = 0x16a8,
    SCIM_KEY_Zstroke                            = 0x16a9,
    SCIM_KEY_Gcaron                             = 0x16aa,
    SCIM_KEY_Obarred                            = 0x16af,
    SCIM_KEY_ccedillaabovedot                   = 0x16b2,
    SCIM_KEY_xabovedot                          = 0x16b3,
    SCIM_KEY_Ocaron                             = 0x16b4,
    SCIM_KEY_qabovedot                          = 0x16b5,
    SCIM_KEY_ibreve                             = 0x16b6,
    SCIM_KEY_ie                                 = 0x16b7,
    SCIM_KEY_uo                                 = 0x16b8,
    SCIM_KEY_zstroke                            = 0x16b9,
    SCIM_KEY_gcaron                             = 0x16ba,
    SCIM_KEY_ocaron                             = 0x16bd,
    SCIM_KEY_obarred                            = 0x16bf,
    SCIM_KEY_SCHWA                              = 0x16c6,
    SCIM_KEY_schwa                              = 0x16f6,
    /* those are not really Caucasus, but I put them here for now */
    /* For Inupiak */
    SCIM_KEY_Lbelowdot                          = 0x16d1,
    SCIM_KEY_Lstrokebelowdot                    = 0x16d2,
    SCIM_KEY_lbelowdot                          = 0x16e1,
    SCIM_KEY_lstrokebelowdot                    = 0x16e2,
    /* For Guarani */
    SCIM_KEY_Gtilde                             = 0x16d3,
    SCIM_KEY_gtilde                             = 0x16e3,

    /*
     * Vietnamese
     * Byte 3 = 0x1e
     */

    SCIM_KEY_Abelowdot                          = 0x1ea0,
    SCIM_KEY_abelowdot                          = 0x1ea1,
    SCIM_KEY_Ahook                              = 0x1ea2,
    SCIM_KEY_ahook                              = 0x1ea3,
    SCIM_KEY_Acircumflexacute                   = 0x1ea4,
    SCIM_KEY_acircumflexacute                   = 0x1ea5,
    SCIM_KEY_Acircumflexgrave                   = 0x1ea6,
    SCIM_KEY_acircumflexgrave                   = 0x1ea7,
    SCIM_KEY_Acircumflexhook                    = 0x1ea8,
    SCIM_KEY_acircumflexhook                    = 0x1ea9,
    SCIM_KEY_Acircumflextilde                   = 0x1eaa,
    SCIM_KEY_acircumflextilde                   = 0x1eab,
    SCIM_KEY_Acircumflexbelowdot                = 0x1eac,
    SCIM_KEY_acircumflexbelowdot                = 0x1ead,
    SCIM_KEY_Abreveacute                        = 0x1eae,
    SCIM_KEY_abreveacute                        = 0x1eaf,
    SCIM_KEY_Abrevegrave                        = 0x1eb0,
    SCIM_KEY_abrevegrave                        = 0x1eb1,
    SCIM_KEY_Abrevehook                         = 0x1eb2,
    SCIM_KEY_abrevehook                         = 0x1eb3,
    SCIM_KEY_Abrevetilde                        = 0x1eb4,
    SCIM_KEY_abrevetilde                        = 0x1eb5,
    SCIM_KEY_Abrevebelowdot                     = 0x1eb6,
    SCIM_KEY_abrevebelowdot                     = 0x1eb7,
    SCIM_KEY_Ebelowdot                          = 0x1eb8,
    SCIM_KEY_ebelowdot                          = 0x1eb9,
    SCIM_KEY_Ehook                              = 0x1eba,
    SCIM_KEY_ehook                              = 0x1ebb,
    SCIM_KEY_Etilde                             = 0x1ebc,
    SCIM_KEY_etilde                             = 0x1ebd,
    SCIM_KEY_Ecircumflexacute                   = 0x1ebe,
    SCIM_KEY_ecircumflexacute                   = 0x1ebf,
    SCIM_KEY_Ecircumflexgrave                   = 0x1ec0,
    SCIM_KEY_ecircumflexgrave                   = 0x1ec1,
    SCIM_KEY_Ecircumflexhook                    = 0x1ec2,
    SCIM_KEY_ecircumflexhook                    = 0x1ec3,
    SCIM_KEY_Ecircumflextilde                   = 0x1ec4,
    SCIM_KEY_ecircumflextilde                   = 0x1ec5,
    SCIM_KEY_Ecircumflexbelowdot                = 0x1ec6,
    SCIM_KEY_ecircumflexbelowdot                = 0x1ec7,
    SCIM_KEY_Ihook                              = 0x1ec8,
    SCIM_KEY_ihook                              = 0x1ec9,
    SCIM_KEY_Ibelowdot                          = 0x1eca,
    SCIM_KEY_ibelowdot                          = 0x1ecb,
    SCIM_KEY_Obelowdot                          = 0x1ecc,
    SCIM_KEY_obelowdot                          = 0x1ecd,
    SCIM_KEY_Ohook                              = 0x1ece,
    SCIM_KEY_ohook                              = 0x1ecf,
    SCIM_KEY_Ocircumflexacute                   = 0x1ed0,
    SCIM_KEY_ocircumflexacute                   = 0x1ed1,
    SCIM_KEY_Ocircumflexgrave                   = 0x1ed2,
    SCIM_KEY_ocircumflexgrave                   = 0x1ed3,
    SCIM_KEY_Ocircumflexhook                    = 0x1ed4,
    SCIM_KEY_ocircumflexhook                    = 0x1ed5,
    SCIM_KEY_Ocircumflextilde                   = 0x1ed6,
    SCIM_KEY_ocircumflextilde                   = 0x1ed7,
    SCIM_KEY_Ocircumflexbelowdot                = 0x1ed8,
    SCIM_KEY_ocircumflexbelowdot                = 0x1ed9,
    SCIM_KEY_Ohornacute                         = 0x1eda,
    SCIM_KEY_ohornacute                         = 0x1edb,
    SCIM_KEY_Ohorngrave                         = 0x1edc,
    SCIM_KEY_ohorngrave                         = 0x1edd,
    SCIM_KEY_Ohornhook                          = 0x1ede,
    SCIM_KEY_ohornhook                          = 0x1edf,
    SCIM_KEY_Ohorntilde                         = 0x1ee0,
    SCIM_KEY_ohorntilde                         = 0x1ee1,
    SCIM_KEY_Ohornbelowdot                      = 0x1ee2,
    SCIM_KEY_ohornbelowdot                      = 0x1ee3,
    SCIM_KEY_Ubelowdot                          = 0x1ee4,
    SCIM_KEY_ubelowdot                          = 0x1ee5,
    SCIM_KEY_Uhook                              = 0x1ee6,
    SCIM_KEY_uhook                              = 0x1ee7,
    SCIM_KEY_Uhornacute                         = 0x1ee8,
    SCIM_KEY_uhornacute                         = 0x1ee9,
    SCIM_KEY_Uhorngrave                         = 0x1eea,
    SCIM_KEY_uhorngrave                         = 0x1eeb,
    SCIM_KEY_Uhornhook                          = 0x1eec,
    SCIM_KEY_uhornhook                          = 0x1eed,
    SCIM_KEY_Uhorntilde                         = 0x1eee,
    SCIM_KEY_uhorntilde                         = 0x1eef,
    SCIM_KEY_Uhornbelowdot                      = 0x1ef0,
    SCIM_KEY_uhornbelowdot                      = 0x1ef1,
    SCIM_KEY_Ybelowdot                          = 0x1ef4,
    SCIM_KEY_ybelowdot                          = 0x1ef5,
    SCIM_KEY_Yhook                              = 0x1ef6,
    SCIM_KEY_yhook                              = 0x1ef7,
    SCIM_KEY_Ytilde                             = 0x1ef8,
    SCIM_KEY_ytilde                             = 0x1ef9,
    SCIM_KEY_Ohorn                              = 0x1efa, /* U+01a0 */
    SCIM_KEY_ohorn                              = 0x1efb, /* U+01a1 */
    SCIM_KEY_Uhorn                              = 0x1efc, /* U+01af */
    SCIM_KEY_uhorn                              = 0x1efd, /* U+01b0 */

    SCIM_KEY_combining_tilde                    = 0x1e9f, /* U+0303 */
    SCIM_KEY_combining_grave                    = 0x1ef2, /* U+0300 */
    SCIM_KEY_combining_acute                    = 0x1ef3, /* U+0301 */
    SCIM_KEY_combining_hook                     = 0x1efe, /* U+0309 */
    SCIM_KEY_combining_belowdot                 = 0x1eff, /* U+0323 */

    SCIM_KEY_EcuSign                            = 0x20a0,
    SCIM_KEY_ColonSign                          = 0x20a1,
    SCIM_KEY_CruzeiroSign                       = 0x20a2,
    SCIM_KEY_FFrancSign                         = 0x20a3,
    SCIM_KEY_LiraSign                           = 0x20a4,
    SCIM_KEY_MillSign                           = 0x20a5,
    SCIM_KEY_NairaSign                          = 0x20a6,
    SCIM_KEY_PesetaSign                         = 0x20a7,
    SCIM_KEY_RupeeSign                          = 0x20a8,
    SCIM_KEY_WonSign                            = 0x20a9,
    SCIM_KEY_NewSheqelSign                      = 0x20aa,
    SCIM_KEY_DongSign                           = 0x20ab,
    SCIM_KEY_EuroSign                           = 0x20ac
};

/**
 * @brief Enum values of all valid Keyboard Layout type.
 *
 * This types are used by the Keyboard Layout mapping
 * function of KeyEvent class to map a KeyEvent of a
 * certain Keyboard Layout to another.
 *
 * But this function only covers the key range of
 * default US PC keyboard. All additional keys
 * in other layouts will never be mapped.
 *
 * The default layout is standard US PC keyboard.
 *
 * If the layout of a KeyEvent is Unknown, then it'll never
 * be mapped.
 */
enum KeyboardLayout
{
    SCIM_KEYBOARD_Unknown                       = 0,
    SCIM_KEYBOARD_Default                       = 1,
    SCIM_KEYBOARD_US                            = 1,
    SCIM_KEYBOARD_Belgian                       = 2,
    SCIM_KEYBOARD_Croatian                      = 3,
    SCIM_KEYBOARD_Czech                         = 4,
    SCIM_KEYBOARD_Czech_Qwerty                  = 5,
    SCIM_KEYBOARD_Danish                        = 6,
    SCIM_KEYBOARD_Dutch                         = 7,
    SCIM_KEYBOARD_Dvorak                        = 8,
    SCIM_KEYBOARD_Estonian                      = 9,
    SCIM_KEYBOARD_Finnish                       = 10,
    SCIM_KEYBOARD_French                        = 11,
    SCIM_KEYBOARD_French_Canadian               = 12,
    SCIM_KEYBOARD_French_Switzerland            = 13,
    SCIM_KEYBOARD_German                        = 14,
    SCIM_KEYBOARD_German_Deadkeys               = 15,
    SCIM_KEYBOARD_German_Swiss                  = 16,
    SCIM_KEYBOARD_Greek                         = 17,
    SCIM_KEYBOARD_Hungarian                     = 18,
    SCIM_KEYBOARD_Italian                       = 19,
    SCIM_KEYBOARD_Japanese                      = 20,
    SCIM_KEYBOARD_Norwegian                     = 21,
    SCIM_KEYBOARD_Polish                        = 22,
    SCIM_KEYBOARD_Portuguese                    = 23,
    SCIM_KEYBOARD_Portuguese_Brazil             = 24,
    SCIM_KEYBOARD_Portuguese_Brazil_US_Accents  = 25, 
    SCIM_KEYBOARD_Russian                       = 26,
    SCIM_KEYBOARD_Slovak                        = 27,
    SCIM_KEYBOARD_Slovak_Qwerty                 = 28,
    SCIM_KEYBOARD_Slovene                       = 29,
    SCIM_KEYBOARD_Spanish                       = 30,
    SCIM_KEYBOARD_Spanish_CP850                 = 31,
    SCIM_KEYBOARD_Spanish_Latin_America         = 32,
    SCIM_KEYBOARD_Swedish                       = 33,
    SCIM_KEYBOARD_Turkish                       = 34,
    SCIM_KEYBOARD_UK                            = 35,
    SCIM_KEYBOARD_Icelandic                     = 36,
    SCIM_KEYBOARD_Lithuanian                    = 37,
    SCIM_KEYBOARD_Ukrainian                     = 38,
    SCIM_KEYBOARD_NUM_LAYOUTS                   = 39
};

struct KeyEvent;

/**
 * @typedef typedef std::vector<KeyEvent> KeyEventList
 * @brief The container to store a set of KeyEvent objects.
 *
 * You should use the STL container methods to manipulate its objects.
 */
typedef std::vector<KeyEvent> KeyEventList;

/**
 * @brief Convert a key event to a string.
 * @param str - the result string will be stored here.
 * @param key - the KeyEvent to be converted.
 * @return true if success.
 */
bool scim_key_to_string (String &str, const KeyEvent & key);

/**
 * @brief Convert a string to a KeyEvent.
 * @param key - the result KeyEvent will be stored here.
 * @param str - the string to be converted.
 * @return true if success.
 */
bool scim_string_to_key (KeyEvent &key, const String & str);

/**
 * @brief Convert a set of KeyEvents to a string.
 * @param str - the result string will be stored here.
 * @param keylist - the keys to be converted.
 * @return true if success.
 */
bool scim_key_list_to_string (String &str, const KeyEventList & keylist);

/**
 * @brief Covnert a string to a set of KeyEvents.
 * @param keylist - the result KeyEvents will be stored here.
 * @param str - the string to be converted.
 * @return true if success.
 */
bool scim_string_to_key_list (KeyEventList &keylist, const String &str);

/**
 * @brief Convert a Keyboard Layout enum value to its String name.
 * @param layout The Keyboard Layout type.
 * @return The name of this layout.
 */
String scim_keyboard_layout_to_string (KeyboardLayout layout);

/**
 * @brief Convert a String name to the corresponding Keyboard Layout value.
 * @param str The Keyboard Layout name.
 * @return The Keyboard Layout type corresponding to this name, or SCIM_KEYBOARD_Unknown.
 */
KeyboardLayout scim_string_to_keyboard_layout (const String &str);

/**
 * @brief Get the display name of a Keyboard Layout enum value.
 * @param layout The Keyboard Layout type.
 * @return The localized display name of this layout.
 */
String scim_keyboard_layout_get_display_name (KeyboardLayout layout);

/**
 * @brief Get default Keyboard Layout setting.
 *
 * This function return the Keyboard Layout setting stored
 * in global config repository as /DefaultKeyboardLayout key.
 *
 * This function is mainly used by FrontEnds and Setup tools.
 * IMEngines should not use it.
 */ 
KeyboardLayout scim_get_default_keyboard_layout ();

/**
 * @brief Change the default Keyboard Layout setting.
 *
 * The default layout setting will be stored in global config
 * repository as /DefaultKeyboardLayout key.
 *
 * This function is mainly used by FrontEnds and Setup tools.
 * IMEngines should not use it.
 */
void scim_set_default_keyboard_layout (KeyboardLayout layout);

/**
 * @brief The class to store a keyboard event.
 *
 * A keyboard event contains a key code and a set of key masks.
 * The key masks indicate which modifier keys are pressed down and
 * if it's a key release event.
 *
 * It also contains the Keyboard Layout information. IMEngines can
 * map a KeyEvent to another layout by calling method #map_to_layout().
 */
struct KeyEvent
{
    uint32 code;    /**< key code */
    uint16 mask;    /**< modifier keys' mask */
    uint16 layout;  /**< keyboard layout identifier */

    /**
     * @brief Default constructor.
     * @param c - the key code.
     * @param m - the key masks.
     * @param l - the keyboard layout
     */
    KeyEvent (uint32 c = 0, uint16 m = 0, uint16 l = 0)
        : code (c), mask (m), layout (l) { }

    /**
     * @brief Constructor, construct a key event from a string.
     *
     * @param str the key string, eg. "Control+space"
     */
    KeyEvent (const String &str)
        : code (0), mask (0), layout (0) { scim_string_to_key (*this, str); }

    /**
     * @brief Check if this KeyEvent is empty.
     * @return true if this is a empty event.
     */
    bool empty () const { return mask == 0 && code == 0; }

    /**
     * @brief Get the ascii code of this key event.
     *
     * Not all key events have ascii codes.
     *
     * @return the ascii code of this key event.
     *         Zero means no ascii code.
     */
    char get_ascii_code () const;

    /**
     * @brief Get the Unicode code of this key event.
     *
     * Not all key events have unicode codes.
     *
     * @return The Unicode code of this key event.
     *         Zero means no unicode code.
     */
    ucs4_t get_unicode_code () const;

    /**
     * @brief Get the string of this key event.
     *
     * Not all key events can be converted to string.
     *
     * @return The string of this key event.
     */
    String get_key_string () const;

    /**
     * @brief Map this KeyEvent to another Keyboard Layout.
     *
     * This function only covers the key range of
     * default US PC keyboard. All additional keys
     * in other layouts will never be mapped.
     *
     * If the current layout is Unknown or the new_layout is
     * invalid or it's an Unicode embedded KeyEvent,
     * then it'll not be mapped and the identical KeyEvent
     * will be returned.
     *
     * If you want to check if the mapping is successful, 
     * just compare the layout of returned KeyEvent and new_layout
     * to see if they are equal.
     *
     * @param new_layout The Keyboard Layout to be mapped to.
     *
     * @return The new KeyEvent according to the given layout.
     */ 
    KeyEvent map_to_layout (KeyboardLayout new_layout) const;

    /**
     * @brief Check if the Shift key is pressed down.
     */
    bool is_shift_down () const { return (mask & SCIM_KEY_ShiftMask) != 0; }

    /**
     * @brief Check if the CapsLock key is pressed down.
     */
    bool is_caps_lock_down () const { return (mask & SCIM_KEY_CapsLockMask) != 0; }

    /**
     * @brief Check if the Ctrl key is pressed down.
     */
    bool is_control_down () const { return (mask & SCIM_KEY_ControlMask) != 0; }

    /**
     * @brief Check if the Alt key is pressed down.
     */
    bool is_alt_down () const { return (mask & SCIM_KEY_AltMask) != 0; }

    /**
     * @brief Check if the Meta key is pressed down.
     */
    bool is_meta_down () const { return (mask & SCIM_KEY_MetaMask) != 0; }

    /**
     * @brief Check if the Super key is pressed down.
     */
    bool is_super_down () const { return (mask & SCIM_KEY_SuperMask) != 0; }

    /**
     * @brief Check if the Hyper key is pressed down.
     */
    bool is_hyper_down () const { return (mask & SCIM_KEY_HyperMask) != 0; }

    /**
     * @brief Check if the num lock key is pressed down.
     */
    bool is_num_lock_down () const { return (mask & SCIM_KEY_NumLockMask) != 0; }

    /**
     * @brief Check if it's a key press event.
     */
    bool is_key_press () const { return (mask & SCIM_KEY_ReleaseMask) == 0; }

    /**
     * @brief Check if it's a key release event.
     */
    bool is_key_release () const { return (mask & SCIM_KEY_ReleaseMask) != 0; }

    /**
     * @brief Compare two key events.
     * @return true if they are equal.
     */
    bool operator == (const KeyEvent & key) const {
        return code == key.code && mask == key.mask;
    }

    /**
     * @brief Compare two key events.
     * @return true if they are not equal.
     */
    bool operator != (const KeyEvent & key) const {
        return code != key.code || mask != key.mask;
    }

    /**
     * @brief Compare two key events.
     *
     * This operator is mainly for sorting.
     * 
     * @return true if the first is smaller.
     */
    bool operator < (const KeyEvent & key) const {
        return code < key.code || (code == key.code && mask < key.mask);
    }


    /**
     * @brief Check if the lock key is pressed down.
     *
     * For backward API compatibility, do not use it in new code.
     */
    bool is_lock_down () const { return (mask & SCIM_KEY_LockMask) != 0; }

    /**
     * @brief Check if the mod1 key is pressed down.
     *
     * For backward API compatibility, do not use it in new code.
     */
    bool is_mod1_down () const { return (mask & SCIM_KEY_Mod1Mask) != 0; }

    /**
     * @brief Check if the mod2 key is pressed down.
     *
     * For backward API compatibility, do not use it in new code.
     */
    bool is_mod2_down () const { return (mask & SCIM_KEY_Mod2Mask) != 0; }

    /**
     * @brief Check if the mod3 key is pressed down.
     *
     * For backward API compatibility, do not use it in new code.
     */
    bool is_mod3_down () const { return (mask & SCIM_KEY_Mod3Mask) != 0; }

    /**
     * @brief Check if the mod4 key is pressed down.
     *
     * For backward API compatibility, do not use it in new code.
     */
    bool is_mod4_down () const { return (mask & SCIM_KEY_Mod4Mask) != 0; }

    /**
     * @brief Check if the mod5 key is pressed down.
     *
     * For backward API compatibility, do not use it in new code.
     */
    bool is_mod5_down () const { return (mask & SCIM_KEY_Mod5Mask) != 0; }

    /**
     * @brief Check if the scroll lock key is pressed down.
     *
     * For backward API compatibility, do not use it in new code.
     */
    bool is_scroll_lock_down () const { return (mask & SCIM_KEY_ScrollLockMask) != 0; }

};

/** @} */

} // namespace scim

#endif //__SCIM_EVENT_H

/*
vi:ts=4:nowrap:ai:expandtab
*/

