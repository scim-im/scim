/** @file scim_table_imengine.cpp
 * implementation of class TableInstance.
 */

/*
 * Smart Common Input Method
 * 
 * Copyright (c) 2002-2005 James Su <suzhe@tsinghua.org.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: scim_table_imengine.cpp,v 1.12 2006/01/12 08:43:29 suzhe Exp $
 *
 */

#define Uses_STL_AUTOPTR
#define Uses_STL_FUNCTIONAL
#define Uses_STL_VECTOR
#define Uses_STL_IOSTREAM
#define Uses_STL_FSTREAM
#define Uses_STL_ALGORITHM
#define Uses_STL_MAP
#define Uses_STL_UTILITY
#define Uses_STL_IOMANIP
#define Uses_C_STDIO
#define Uses_SCIM_UTILITY
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_ICONV
#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_LOOKUP_TABLE

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <scim.h>
#include "scim_table_imengine.h"
#include "scim_table_private.h"

#define scim_module_init table_LTX_scim_module_init
#define scim_module_exit table_LTX_scim_module_exit
#define scim_imengine_module_init table_LTX_scim_imengine_module_init
#define scim_imengine_module_create_factory table_LTX_scim_imengine_module_create_factory

#define SCIM_TABLE_SAVE_PERIOD       300

#define SCIM_TABLE_MAX_TABLE_NUMBER  256
#define SCIM_TABLE_MAX_INPUTTED_KEYS  16

#define SCIM_CONFIG_IMENGINE_TABLE_FULL_WIDTH_PUNCT_KEY   "/IMEngine/Table/FullWidthPunctKey"
#define SCIM_CONFIG_IMENGINE_TABLE_FULL_WIDTH_LETTER_KEY  "/IMEngine/Table/FullWidthLetterKey"
#define SCIM_CONFIG_IMENGINE_TABLE_MODE_SWITCH_KEY        "/IMEngine/Table/ModeSwitchKey"
#define SCIM_CONFIG_IMENGINE_TABLE_ADD_PHRASE_KEY         "/IMEngine/Table/AddPhraseKey"
#define SCIM_CONFIG_IMENGINE_TABLE_DEL_PHRASE_KEY         "/IMEngine/Table/DeletePhraseKey"
#define SCIM_CONFIG_IMENGINE_TABLE_SHOW_PROMPT            "/IMEngine/Table/ShowPrompt"
#define SCIM_CONFIG_IMENGINE_TABLE_USER_TABLE_BINARY      "/IMEngine/Table/UserTableBinary"
#define SCIM_CONFIG_IMENGINE_TABLE_USER_PHRASE_FIRST      "/IMEngine/Table/UserPhraseFirst"
#define SCIM_CONFIG_IMENGINE_TABLE_LONG_PHRASE_FIRST      "/IMEngine/Table/LongPhraseFirst"
#define SCIM_CONFIG_IMENGINE_TABLE_SHOW_KEY_HINT          "/IMEngine/Table/ShowKeyHint"

#define SCIM_PROP_STATUS                                  "/IMEngine/Table/Status"
#define SCIM_PROP_LETTER                                  "/IMEngine/Table/Letter"
#define SCIM_PROP_PUNCT                                   "/IMEngine/Table/Punct"

#define SCIM_FULL_LETTER_ICON                             (SCIM_ICONDIR "/full-letter.png")
#define SCIM_HALF_LETTER_ICON                             (SCIM_ICONDIR "/half-letter.png")
#define SCIM_FULL_PUNCT_ICON                              (SCIM_ICONDIR "/full-punct.png")
#define SCIM_HALF_PUNCT_ICON                              (SCIM_ICONDIR "/half-punct.png")

#define SCIM_TABLE_ICON_FILE                              (SCIM_ICONDIR "/table.png")


using namespace scim;

static unsigned int _scim_number_of_tables = 0;

static Pointer <TableFactory> _scim_table_factories [SCIM_TABLE_MAX_TABLE_NUMBER];

static std::vector<String> _scim_sys_table_list;
static std::vector<String> _scim_user_table_list;

static ConfigPointer _scim_config;

static void
_get_table_list (std::vector<String> &table_list, const String &path)
{
    table_list.clear ();

    DIR *dir = opendir (path.c_str ());
    if (dir != NULL) {
        struct dirent *file = readdir (dir);
        while (file != NULL) {
            struct stat filestat;
            String absfn = path + SCIM_PATH_DELIM_STRING + file->d_name;
            stat (absfn.c_str (), &filestat);

            if (S_ISREG (filestat.st_mode))
                table_list.push_back (absfn);

            file = readdir (dir);
        }
        closedir (dir);        
    }
}

extern "C" {
    void scim_module_init (void)
    {
        bindtextdomain (GETTEXT_PACKAGE, SCIM_TABLE_LOCALEDIR);
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    }

    void scim_module_exit (void)
    {
        for (int i=0; i<_scim_number_of_tables; ++i)
            _scim_table_factories [i].reset ();

        _scim_config.reset ();
    }

    unsigned int scim_imengine_module_init (const ConfigPointer &config)
    {
        _scim_config = config;

        _get_table_list (_scim_sys_table_list, SCIM_TABLE_SYSTEM_TABLE_DIR);
        _get_table_list (_scim_user_table_list, scim_get_home_dir () + SCIM_TABLE_USER_TABLE_DIR);

        _scim_number_of_tables = _scim_sys_table_list.size () + _scim_user_table_list.size (); 

        return _scim_number_of_tables; 
    }

    IMEngineFactoryPointer scim_imengine_module_create_factory (unsigned int index)
    {
        if (index >= _scim_number_of_tables) return 0;

        TableFactory *factory = 0;

        try {
            factory = new TableFactory (_scim_config);

            if (index < _scim_sys_table_list.size ())
                factory->load_table (_scim_sys_table_list [index], false);
            else
                factory->load_table (_scim_user_table_list [index - _scim_sys_table_list.size ()], true);

            if (!factory->valid ())
                throw IMEngineError ("Table load failed!");

            return IMEngineFactoryPointer (factory);

        } catch (...) {
            delete factory;
            return IMEngineFactoryPointer (0);
        }
    }
}

// implementation of Table
TableFactory::TableFactory (const ConfigPointer &config)
    : m_config (config),
      m_is_user_table (false),
      m_show_prompt (false),
      m_show_key_hint (false),
      m_user_table_binary (false),
      m_user_phrase_first (false),
      m_long_phrase_first (false),
      m_last_time ((time_t)0),
      m_status_property (SCIM_PROP_STATUS, ""),
      m_letter_property (SCIM_PROP_LETTER, _("Full/Half Letter")),
      m_punct_property (SCIM_PROP_PUNCT, _("Full/Half Punct"))
{
    init (m_config);

    m_status_property.set_tip (_("The status of the current input method. Click to change it."));
    m_letter_property.set_tip (_("The input mode of the letters. Click to toggle between half and full."));
    m_punct_property.set_tip (_("The input mode of the puncutations. Click to toggle between half and full."));

    if (!m_config.null ())
        m_reload_signal_connection = m_config->signal_connect_reload (slot (this, &TableFactory::init));
}

void
TableFactory::init (const ConfigPointer &config)
{
    String str;

    SCIM_DEBUG_IMENGINE (1) << "Load configuration.\n";

    if (!config.null ()) {
        //Read full width punctuation keys
        str = config->read (String (SCIM_CONFIG_IMENGINE_TABLE_FULL_WIDTH_PUNCT_KEY), String (""));

        scim_string_to_key_list (m_full_width_punct_keys, str);

        //Read full width letter keys
        str = config->read (String (SCIM_CONFIG_IMENGINE_TABLE_FULL_WIDTH_LETTER_KEY), String (""));

        scim_string_to_key_list (m_full_width_letter_keys, str);

        //Read mode switch keys
        str = config->read (String (SCIM_CONFIG_IMENGINE_TABLE_MODE_SWITCH_KEY), String (""));

        scim_string_to_key_list (m_mode_switch_keys, str);

        //Read add phrase keys
        str = config->read (String (SCIM_CONFIG_IMENGINE_TABLE_ADD_PHRASE_KEY), String ("Control+a,Control+equal"));

        scim_string_to_key_list (m_add_phrase_keys, str);

        //Read delete phrase keys
        str = config->read (String (SCIM_CONFIG_IMENGINE_TABLE_DEL_PHRASE_KEY), String ("Control+d,Control+minus"));

        scim_string_to_key_list (m_del_phrase_keys, str);

        m_show_prompt = config->read (String (SCIM_CONFIG_IMENGINE_TABLE_SHOW_PROMPT), false);

        m_show_key_hint = config->read (String (SCIM_CONFIG_IMENGINE_TABLE_SHOW_KEY_HINT), false);

        m_user_phrase_first = config->read (String (SCIM_CONFIG_IMENGINE_TABLE_USER_PHRASE_FIRST), false);

        m_long_phrase_first = config->read (String (SCIM_CONFIG_IMENGINE_TABLE_LONG_PHRASE_FIRST), false);

        m_user_table_binary = config->read (String (SCIM_CONFIG_IMENGINE_TABLE_USER_TABLE_BINARY), false);
    }

    m_last_time = time (NULL);
}

TableFactory::~TableFactory ()
{
    save ();
    m_reload_signal_connection.disconnect ();
}

WideString
TableFactory::get_name () const
{
    return m_table.get_name (scim_get_current_locale ());
}

WideString
TableFactory::get_authors () const
{
    return m_table.get_author ();
}

WideString
TableFactory::get_credits () const
{
    return WideString ();
}

WideString
TableFactory::get_help () const
{
    WideString help;

    std::vector<KeyEvent> keys, keys2;

    String full_width_letter;
    String full_width_punct;
    String mode_switch;
    String add_phrase;
    String del_phrase;

    keys = m_full_width_letter_keys;
    keys2 = m_table.get_full_width_letter_keys ();
    keys.insert (keys.end (), keys2.begin (), keys2.end ());
    keys.erase (std::unique (keys.begin (), keys.end ()), keys.end ());
    scim_key_list_to_string (full_width_letter, keys);

    keys = m_full_width_punct_keys;
    keys2 = m_table.get_full_width_punct_keys ();
    keys.insert (keys.end (), keys2.begin (), keys2.end ());
    keys.erase (std::unique (keys.begin (), keys.end ()), keys.end ());
    scim_key_list_to_string (full_width_punct, keys);

    keys = m_mode_switch_keys;
    keys2 = m_table.get_mode_switch_keys ();
    keys.insert (keys.end (), keys2.begin (), keys2.end ());
    keys.erase (std::unique (keys.begin (), keys.end ()), keys.end ());
    scim_key_list_to_string (mode_switch, keys);

    scim_key_list_to_string (add_phrase, m_add_phrase_keys);
    scim_key_list_to_string (del_phrase, m_del_phrase_keys);

    return utf8_mbstowcs (
        String (_("Hot Keys:\n\n  ")) +
        full_width_letter + String (":\n") +
        String (_("    Switch between full/half width letter mode.\n\n  ")) +
        full_width_punct + String (":\n") +
        String (_("    Switch between full/half width punctuation mode.\n\n  ")) +
        mode_switch + String (":\n") +
        String (_("    Switch between Forward/Input mode.\n\n  ")) +
        add_phrase + String (":\n") +
        String (_("    Add a new phrase.\n\n  ")) +
        del_phrase + String (":\n") +
        String (_("    Delete the selected phrase.\n\n")) +
        String (_("  Control+Down:\n    Move lookup cursor to next shorter phrase\n"
                  "    Only available when LongPhraseFirst option is set.\n\n")) +
        String (_("  Control+Up:\n    Move lookup cursor to previous longer phrase\n"
                  "    Only available when LongPhraseFirst option is set.\n\n")) +
        String (_("  Esc:\n    reset the input method.\n\n\n")) +
        String (_("How to add a phrase:\n"
                  "    Input the new phrase as normal, then press the\n"
                  "  hot key. A hint will be shown to let you input a key\n"
                  "  for this phrase.\n"
                  "    Input a key then press the space bar.\n"
                  "  A hint will be shown to indicate whether\n"
                  "  the phrase was added sucessfully.\n"))
        );
}

String
TableFactory::get_uuid () const
{
    return m_table.get_uuid ();
}

String
TableFactory::get_icon_file () const
{
    String file = m_table.get_icon_file ();

    return file.length () ? file : String (SCIM_TABLE_ICON_FILE);
}

IMEngineInstancePointer
TableFactory::create_instance (const String& encoding, int id)
{
    return new TableInstance (this, encoding, id);
}

bool
TableFactory::load_table (const String &table_file, bool user_table)
{
    if (!table_file.length ()) return false;

    m_table_filename = table_file;
    m_is_user_table = user_table;

    if (user_table) {
        if (!m_table.init ("", m_table_filename, "")) return false;
    } else {
        if (!m_table.init (m_table_filename,
                           get_sys_table_user_file (),
                           get_sys_table_freq_file ()))
            return false;
    }

    set_languages (m_table.get_languages ());

    return m_table.valid ();
}

void
TableFactory::refresh (bool rightnow)
{
    time_t cur_time = time (NULL);

    if (rightnow || cur_time < m_last_time || cur_time - m_last_time > SCIM_TABLE_SAVE_PERIOD) {
        m_last_time = cur_time;
        save ();
    }
}

void
TableFactory::save ()
{
    if (!m_table.valid () || !m_table.updated ()) return;

    if (m_is_user_table)
        m_table.save ("", m_table_filename, "", m_user_table_binary);
    else
        m_table.save ("", get_sys_table_user_file (), get_sys_table_freq_file (), m_user_table_binary);
}

String
TableFactory::get_sys_table_freq_file ()
{
    String fn, tf;
    String::size_type pos;

    if (m_table_filename.length ()) {
        pos = m_table_filename.rfind (SCIM_PATH_DELIM);

        if (pos != String::npos)
            tf = m_table_filename.substr (pos+1);
        else
            tf = m_table_filename;

        fn = scim_get_home_dir () + SCIM_TABLE_SYSTEM_UPDATE_TABLE_DIR;

        if (access (fn.c_str (), R_OK | W_OK) != 0) {
            if (!scim_make_dir (fn))
                return String ();
        }

        fn = fn + SCIM_PATH_DELIM_STRING + tf + ".freq";
    }
    return fn;
}

String
TableFactory::get_sys_table_user_file ()
{
    String fn, tf;
    String::size_type pos;

    if (m_table_filename.length ()) {
        pos = m_table_filename.rfind (SCIM_PATH_DELIM);

        if (pos != String::npos)
            tf = m_table_filename.substr (pos+1);
        else
            tf = m_table_filename;

        fn = scim_get_home_dir () + SCIM_TABLE_SYSTEM_UPDATE_TABLE_DIR;

        if (access (fn.c_str (), R_OK | W_OK) != 0) {
            if (!scim_make_dir (fn))
                return String ();
        }

        fn = fn + SCIM_PATH_DELIM_STRING + tf + ".user";
    }
    return fn;
}


// implementation of TableInstance
TableInstance::TableInstance (TableFactory *factory,
                                            const String& encoding,
                                            int id)
    : IMEngineInstanceBase (factory, encoding, id),
      m_factory (factory),
      m_double_quotation_state (false),
      m_single_quotation_state (false),
      m_forward (false),
      m_focused (false),
      m_inputing_caret (0),
      m_inputing_key (0),
      m_iconv (encoding)
{
    m_full_width_letter [0] = m_factory->m_table.is_def_full_width_letter ();
    m_full_width_letter [1] = false;

    m_full_width_punct [0] = m_factory->m_table.is_def_full_width_punct ();
    m_full_width_punct [1] = false;

    char buf [2] = { 0, 0 };

    std::vector <KeyEvent>   keys = m_factory->m_table.get_select_keys ();
    std::vector <WideString> labels;

    for (size_t i = 0; i < keys.size (); ++i) {
        buf [0] = keys [i].get_ascii_code ();
        labels.push_back (utf8_mbstowcs (buf));
    }

    m_lookup_table.set_candidate_labels (labels);
    m_lookup_table.set_page_size        (keys.size ());
    m_lookup_table.show_cursor ();
}

TableInstance::~TableInstance ()
{
}

bool
TableInstance::process_key_event (const KeyEvent& rawkey)
{
    KeyEvent key = rawkey.map_to_layout (m_factory->m_table.get_keyboard_layout ());

    bool ret = false;

    if (!m_focused) return false;

    // capture the mode switch key events
    if (match_key_event (m_factory->m_mode_switch_keys, key) ||
        match_key_event (m_factory->m_table.get_mode_switch_keys (), key)) {
        m_forward = !m_forward;
        refresh_status_property ();
        refresh_letter_property ();
        refresh_punct_property ();
        reset ();
        ret = true;
    }

    // toggle full width punctuation mode
    else if (match_key_event (m_factory->m_full_width_punct_keys, key) ||
             match_key_event (m_factory->m_table.get_full_width_punct_keys (), key)) {
        trigger_property (SCIM_PROP_PUNCT);
        ret = true;
    }

    // toggle full width letter mode
    else if (match_key_event (m_factory->m_full_width_letter_keys, key) ||
             match_key_event (m_factory->m_table.get_full_width_letter_keys (), key)) {
        trigger_property (SCIM_PROP_LETTER);
        ret = true;
    }

    // discard the key release event.
    else if (key.is_key_release ()) {
        ret = true;
    }

    // process the key press event, if not in forward mode.
    else if (!m_forward) {
        // First reset add phrase mode.
        if (m_add_phrase_mode > 1) {
            m_add_phrase_mode = 0;
            refresh_aux_string ();
        }

        //reset key
        if (key.code == SCIM_KEY_Escape && key.mask == 0) {
            if (m_inputted_keys.size () == 0 && m_add_phrase_mode != 1)
                ret = false;
            else {
                reset ();
                ret = true;
            }
        }

        //caret left
        else if (key.code == SCIM_KEY_Left && key.mask == 0)
            ret = caret_left ();

        //caret right
        else if (key.code == SCIM_KEY_Right && key.mask == 0)
            ret = caret_right ();

        //caret home 
        else if (key.code == SCIM_KEY_Home && key.mask == 0)
            ret = caret_home ();

        //caret end
        else if (key.code == SCIM_KEY_End && key.mask == 0)
            ret = caret_end ();

        //lookup table cursor up
        else if (key.code == SCIM_KEY_Up && key.mask == 0)
            ret = lookup_cursor_up ();

        //lookup table cursor down
        else if (key.code == SCIM_KEY_Down && key.mask == 0)
            ret = lookup_cursor_down ();

        //lookup table cursor up to longer phrase
        else if (key.code == SCIM_KEY_Up && key.mask == SCIM_KEY_ControlMask &&
                 m_factory->m_long_phrase_first && !m_factory->m_user_phrase_first)
            ret = lookup_cursor_up_to_longer ();

        //lookup table cursor down
        else if (key.code == SCIM_KEY_Down && key.mask == SCIM_KEY_ControlMask &&
                 m_factory->m_long_phrase_first && !m_factory->m_user_phrase_first)
            ret = lookup_cursor_down_to_shorter ();

        //backspace key
        else if (key.code == SCIM_KEY_BackSpace && key.mask == 0)
            ret = erase ();

        //delete key
        else if (key.code == SCIM_KEY_Delete && key.mask == 0)
            ret = erase (false);

        //add new phrase
        else if (!m_inputted_keys.size () && m_last_committed.length () &&
            match_key_event (m_factory->m_add_phrase_keys, key)) {
            m_add_phrase_mode = 1;
            refresh_aux_string ();
            ret = true;
        }

        //delete phrase
        else if (match_key_event (m_factory->m_del_phrase_keys, key)) {
            if (delete_phrase ())
                ret = true;
        }

        // other situation
        else {
            ret = false;

            //select lookup table
            int pos = m_factory->m_table.get_select_key_pos (key);

            // If there is a new empty key (a split char was inserted),
            // then try to select lookup table first.
            // Otherwise try to insert the char first.
            if (m_inputted_keys.size () && !m_inputted_keys [m_inputing_key].length ()) {
                if (pos >= 0 && pos < m_lookup_table.get_current_page_size ())
                    ret = lookup_select (pos);

                // insert char
                if (!ret && (key.mask & (~ (SCIM_KEY_ShiftMask + SCIM_KEY_CapsLockMask))) == 0)
                    ret = insert (key.get_ascii_code ());
            } else {
                // insert char
                if ((key.mask & (~ (SCIM_KEY_ShiftMask + SCIM_KEY_CapsLockMask))) == 0 &&
                    test_insert (key.get_ascii_code ()))
                    ret = insert (key.get_ascii_code ());

                if (!ret && pos >= 0 && pos < m_lookup_table.get_current_page_size ())
                    ret = lookup_select (pos);

                // insert char finally.
                if (!ret && (key.mask & (~ (SCIM_KEY_ShiftMask + SCIM_KEY_CapsLockMask))) == 0)
                    ret = insert (key.get_ascii_code ());
            }

            //lookup table page up
            if (!ret && match_key_event (m_factory->m_table.get_page_up_keys (), key))
                ret = lookup_page_up ();

            //lookup table page down
            if (!ret && match_key_event (m_factory->m_table.get_page_down_keys (), key))
                ret = lookup_page_down ();

            //space hit
            if (!ret && match_key_event (m_factory->m_table.get_commit_keys (), key))
                ret = space_hit ();

            //enter hit
            if (!ret && match_key_event (m_factory->m_table.get_forward_keys (), key))
                ret = enter_hit ();
        }
    }

    if (!ret && (key.mask & (~ (SCIM_KEY_ShiftMask + SCIM_KEY_CapsLockMask))) == 0 &&
        key.get_ascii_code ())
        ret = post_process (key.get_ascii_code ());

    m_prev_key = key;

    return ret;
}

void
TableInstance::select_candidate (unsigned int item)
{
    lookup_select (item);
}

void
TableInstance::update_lookup_table_page_size (unsigned int page_size)
{
    if (page_size > 0)
        m_lookup_table.set_page_size (page_size);
}

void
TableInstance::lookup_table_page_up ()
{
    lookup_page_up ();
}

void
TableInstance::lookup_table_page_down ()
{
    lookup_page_down ();
}

void
TableInstance::move_preedit_caret (unsigned int pos)
{
    uint32 len = 0;
    size_t i;

    for (i=0; i<m_converted_strings.size (); ++i) {
        if (pos >= len && pos < len + m_converted_strings [i].length ()) {
            m_inputing_key = i;
            m_inputing_caret = m_inputted_keys [i].length ();

            m_converted_strings.erase (m_converted_strings.begin () + i, m_converted_strings.end ());
            m_converted_indexes.erase (m_converted_indexes.begin () + i, m_converted_indexes.end ());

            refresh_lookup_table ();
            refresh_preedit ();
            refresh_aux_string ();

            return;
        }
        len += m_converted_strings [i].length ();
    }

    if (m_factory->m_table.is_auto_fill () &&
        m_inputing_key == m_inputted_keys.size () - 1 &&
        m_inputing_caret == m_inputted_keys [m_inputing_key].length () &&
        m_converted_strings.size () == m_inputing_key &&
        m_lookup_table.number_of_candidates ()) {

        uint32 offset = m_lookup_table_indexes [m_lookup_table.get_cursor_pos ()];
        size_t phlen  = m_factory->m_table.get_phrase_length (offset);

        if (pos >= len && pos < len + phlen) {
            m_inputing_caret = 0;
            refresh_lookup_table (true, false);
            refresh_preedit ();
            return;
        }
    } else {
        if (m_converted_strings.size ()) {
            ++len;
            if (pos < len) ++pos;
        }

        for (i=m_converted_strings.size (); i<m_inputted_keys.size (); ++i) {
            if (pos >= len && pos <= len + m_inputted_keys [i].length ()) {
                m_inputing_key = i;
                m_inputing_caret = pos - len;

                refresh_lookup_table (true, false);
                refresh_preedit ();
                refresh_aux_string ();
                return;
            }

            len += (m_inputted_keys [i].length () +1);
        }
    }
}

void
TableInstance::reset ()
{
    m_double_quotation_state = false;
    m_single_quotation_state = false;

    m_lookup_table.clear ();

    std::vector<String> ().swap (m_inputted_keys);

    std::vector<WideString> ().swap (m_converted_strings);

    std::vector<uint32> ().swap (m_converted_indexes);

    std::vector<uint32> ().swap (m_lookup_table_indexes);

    m_add_phrase_mode = 0;

    m_last_committed = WideString ();

    m_inputing_caret = 0;
    m_inputing_key = 0;

    m_iconv.set_encoding (get_encoding ());

    hide_lookup_table ();
    hide_preedit_string ();
    hide_aux_string ();
}

void
TableInstance::focus_in ()
{
    m_focused = true;

    if (m_add_phrase_mode != 1) {
        m_last_committed = WideString ();
        m_add_phrase_mode = 0;
    }

    refresh_lookup_table (true, false);
    refresh_preedit ();
    refresh_aux_string ();

    initialize_properties ();
}

void
TableInstance::focus_out ()
{
    m_focused = false;
}

void
TableInstance::trigger_property (const String &property)
{
    if (property == SCIM_PROP_STATUS) {
        m_forward = !m_forward;
        refresh_status_property ();
        refresh_letter_property ();
        refresh_punct_property ();
        reset ();
    } else if (property == SCIM_PROP_LETTER && m_factory->m_table.is_use_full_width_letter ()) {
        m_full_width_letter [m_forward?1:0] =
            !m_full_width_letter [m_forward?1:0];
        refresh_letter_property ();
    } else if (property == SCIM_PROP_PUNCT && m_factory->m_table.is_use_full_width_punct ()) {
        m_full_width_punct [m_forward?1:0] = 
            !m_full_width_punct [m_forward?1:0];
        refresh_punct_property ();
    }
}
 
void
TableInstance::initialize_properties ()
{
    PropertyList proplist;

    proplist.push_back (m_factory->m_status_property);

    if (m_factory->m_table.is_use_full_width_letter ())
        proplist.push_back (m_factory->m_letter_property);

    if (m_factory->m_table.is_use_full_width_punct ())
        proplist.push_back (m_factory->m_punct_property);

    register_properties (proplist);

    refresh_status_property ();
    refresh_letter_property ();
    refresh_punct_property ();
}

void
TableInstance::refresh_status_property ()
{
    if (m_focused) {
        if (m_forward)
            m_factory->m_status_property.set_label (_("En"));
        else
            m_factory->m_status_property.set_label (utf8_wcstombs (m_factory->m_table.get_status_prompt ()));

        update_property (m_factory->m_status_property);
    }
}

void
TableInstance::refresh_letter_property ()
{
    if (m_focused && m_factory->m_table.is_use_full_width_letter ()) {
        m_factory->m_letter_property.set_icon (
            m_full_width_letter [m_forward?1:0] ? SCIM_FULL_LETTER_ICON : SCIM_HALF_LETTER_ICON);
        update_property (m_factory->m_letter_property);
    }
}

void
TableInstance::refresh_punct_property ()
{
    if (m_focused && m_factory->m_table.is_use_full_width_punct ()) {
        m_factory->m_punct_property.set_icon (
            m_full_width_punct [m_forward?1:0] ? SCIM_FULL_PUNCT_ICON : SCIM_HALF_PUNCT_ICON);
        update_property (m_factory->m_punct_property);
    }
}

bool
TableInstance::caret_left ()
{
    if (m_inputted_keys.size ()) {
        if (m_inputing_caret > 0) {
            --m_inputing_caret;
            refresh_lookup_table (true, false);
        } else if (m_inputing_key > 0) {
            --m_inputing_key;
            m_inputing_caret = m_inputted_keys [m_inputing_key].length ();

            if (m_converted_strings.size () > m_inputing_key) {
                m_converted_strings.pop_back ();
                m_converted_indexes.pop_back ();
                refresh_lookup_table ();
            } else {
                refresh_lookup_table (true, false);
            }
        } else {
            return caret_end ();
        }

        refresh_preedit ();
        refresh_aux_string ();
        return true;
    }
    return false;
}

bool
TableInstance::caret_right ()
{
    if (m_inputted_keys.size ()) {
        if (m_inputing_caret < m_inputted_keys [m_inputing_key].size ()) {
            ++m_inputing_caret;
        } else if (m_inputing_key < m_inputted_keys.size () - 1) {
            ++m_inputing_key;
            m_inputing_caret = 0;
        } else {
            return caret_home ();
        }
        refresh_lookup_table (true, false);
        refresh_preedit ();
        refresh_aux_string ();
        return true;
    }
    return false;
}

bool
TableInstance::caret_home ()
{
    if (m_inputted_keys.size ()) {
        m_inputing_key = 0;
        m_inputing_caret = 0;

        if (m_converted_strings.size ()) {
            m_converted_strings.clear ();
            m_converted_indexes.clear ();
            refresh_lookup_table ();
        } else {
            refresh_lookup_table (true, false);
        }

        refresh_preedit ();
        refresh_aux_string ();
        return true;
    }
    return false;
}

bool
TableInstance::caret_end ()
{
    if (m_inputted_keys.size ()) {
        m_inputing_key = m_inputted_keys.size () - 1;
        m_inputing_caret = m_inputted_keys [m_inputing_key].length ();

        refresh_lookup_table (true, false);
        refresh_preedit ();
        refresh_aux_string ();
        return true;
    }
    return false;
}

bool
TableInstance::test_insert (char key)
{
    if (m_factory->m_table.is_valid_char (key)) {
        String newkey;
        if (m_inputted_keys.size ()) {
            newkey = m_inputted_keys [m_inputing_key];
            newkey.insert (newkey.begin () + m_inputing_caret, key);
        } else {
            newkey.push_back (key);
        }

        return m_factory->m_table.is_defined_key (newkey);
    }
    return false;
}

bool
TableInstance::insert (char ch)
{
    if (!ch) return false;

    String newkey;
    uint32 old_inputing_key = m_inputing_key;
    bool insert_ok = false;

    if (m_inputted_keys.size () - m_converted_strings.size () >
        SCIM_TABLE_MAX_INPUTTED_KEYS)
        return false;

    // If current inputing key is empty, then the last inputing key is the previous key.
    if (m_inputted_keys.size () && m_inputing_key && !m_inputted_keys [m_inputing_key].length ())
        -- old_inputing_key;

    if (m_factory->m_table.is_split_char (ch)) {
        // split char is invalid during add phrase mode.
        if (m_add_phrase_mode == 1)
            return true;
        else if (m_inputted_keys.size () == 0)
            return false;
        else if (m_inputing_key == m_inputted_keys.size () -1 &&
                m_inputted_keys [m_inputing_key].length () &&
                m_inputing_caret == m_inputted_keys [m_inputing_key].length ()) {
            ++m_inputing_key;
            m_inputing_caret = 0;
            m_inputted_keys.insert (m_inputted_keys.begin () + m_inputing_key, String ());
        } else {
            return false;
        }

        insert_ok = true;
    } else if (m_factory->m_table.is_valid_char (ch)) {
        if (m_add_phrase_mode == 1) {
            m_inputing_key = 0;
            if (!m_inputted_keys.size ()) {
                m_inputted_keys.push_back (String (""));
                m_inputing_caret = 0;
            } else if (m_inputted_keys [0].length () >= m_factory->m_table.get_max_key_length ()) {
                return true;
            }

            m_inputted_keys [0].insert (m_inputted_keys [0].begin () + m_inputing_caret, ch);
            ++m_inputing_caret;
            insert_ok = true;
        } else if (m_inputted_keys.size ()) {
            newkey = m_inputted_keys [m_inputing_key];
            newkey.insert (newkey.begin () + m_inputing_caret, ch);

            if ((m_factory->m_table.is_auto_split () == false ||
                 m_factory->m_table.is_defined_key (newkey)) &&
                newkey.length () <= m_factory->m_table.get_max_key_length ()) {
                m_inputted_keys [m_inputing_key] = newkey;
                ++m_inputing_caret;
                insert_ok = true;
            } else if (m_inputing_caret == m_inputted_keys [m_inputing_key].length ()) {
                newkey = String ();
                newkey.push_back (ch);

                if (m_factory->m_table.is_defined_key (newkey)) {
                    ++m_inputing_key;
                    m_inputing_caret = 1;
                    m_inputted_keys.insert (m_inputted_keys.begin () + m_inputing_key, newkey);
                    insert_ok = true;
                }
            } else if (m_inputing_caret == 0) {
                newkey = String ();
                newkey.push_back (ch);

                if (m_factory->m_table.is_defined_key (newkey)) {
                    m_inputing_caret = 1;
                    m_inputted_keys.insert (m_inputted_keys.begin () + m_inputing_key, newkey);
                    insert_ok = true;
                }
            }
        } else if (!m_factory->m_table.is_multi_wildcard_char (ch)) {
            newkey = String ();
            newkey.push_back (ch);

            if (m_factory->m_table.is_defined_key (newkey)) {
                m_inputted_keys.push_back (newkey);
                m_inputing_key = 0;
                m_inputing_caret = 1; 
                insert_ok = true;
            }
        }
    }

    if (insert_ok) {
        //Do some extra work in normal mode.
        if (m_add_phrase_mode != 1) {
            //auto select
            if (m_factory->m_table.is_auto_select () &&
                m_converted_strings.size () == old_inputing_key &&
                old_inputing_key + 1 == m_inputing_key &&
                m_lookup_table.number_of_candidates () &&
                m_inputted_keys [m_inputing_key].length ()) {
                lookup_to_converted (m_lookup_table.get_cursor_pos ());
            }

            //discard invalid key
            if (m_factory->m_table.is_discard_invalid_key () &&
                m_converted_strings.size () == old_inputing_key &&
                old_inputing_key + 1 == m_inputing_key &&
                m_lookup_table.number_of_candidates () == 0 &&
                m_inputted_keys [m_inputing_key].length ()) {
                m_inputted_keys.erase (m_inputted_keys.begin () + old_inputing_key);
                m_inputing_key --;
            }

            if (m_converted_strings.size () == m_inputing_key) {
                refresh_lookup_table (false, true);

                // If auto commit is true, then do auto select when
                // there is only one candidate for this key.
                if (m_lookup_table.number_of_candidates () == 1 &&
                    m_factory->m_table.is_auto_commit () &&
                    !m_factory->m_table.is_defined_key (
                        m_inputted_keys [m_inputing_key],
                        GT_SEARCH_ONLY_LONGER)) {
                    lookup_to_converted (m_lookup_table.get_cursor_pos ());
                    refresh_lookup_table ();
                } else {
                    refresh_lookup_table (true, false);
                }
            }

            if (m_inputted_keys.size () > SCIM_TABLE_MAX_INPUTTED_KEYS ||
                m_factory->m_table.is_auto_commit ())
                commit_converted ();

            // If it's a key end char, then append an empty key.
            if (m_factory->m_table.is_key_end_char (ch) &&
                m_inputing_key == m_inputted_keys.size () -1 &&
                m_inputted_keys [m_inputing_key].length () &&
                m_inputing_caret == m_inputted_keys [m_inputing_key].length ()) {
                ++m_inputing_key;
                m_inputing_caret = 0;
                m_inputted_keys.insert (m_inputted_keys.begin () + m_inputing_key, String ());
            }
        }

        refresh_preedit ();
        refresh_aux_string ();

        return true;
    }

    return false;
}

bool
TableInstance::erase (bool backspace)
{
    if (m_inputted_keys.size ()) {
        if (backspace && (m_inputing_key > 0 || m_inputing_caret > 0)) {
            if (m_inputing_caret > 0) {
                --m_inputing_caret;
                m_inputted_keys [m_inputing_key].erase (m_inputing_caret, 1);
            } else {
                if (m_inputted_keys [m_inputing_key].length () == 0)
                    m_inputted_keys.erase (m_inputted_keys.begin () + m_inputing_key);

                --m_inputing_key;
                m_inputing_caret = m_inputted_keys [m_inputing_key].length ();

                if (m_inputing_caret > 0) {
                    --m_inputing_caret;
                    m_inputted_keys [m_inputing_key].erase (m_inputing_caret, 1);
                }
            }

            if (m_inputted_keys [m_inputing_key].length () == 0) {
                m_inputted_keys.erase (m_inputted_keys.begin () + m_inputing_key);

                if (m_inputing_key > 0) {
                    --m_inputing_key;
                    m_inputing_caret = m_inputted_keys [m_inputing_key].length ();
                }
            }
        } else if (!backspace) {
            if (m_inputing_caret < m_inputted_keys [m_inputing_key].length ()) {
                m_inputted_keys [m_inputing_key].erase (m_inputing_caret, 1);
            }
            if (m_inputted_keys [m_inputing_key].length () == 0) {
                m_inputted_keys.erase (m_inputted_keys.begin () + m_inputing_key);
            }
            if (m_inputing_key == m_inputted_keys.size () && m_inputing_key > 0) {
                --m_inputing_key;
                m_inputing_caret = m_inputted_keys [m_inputing_key].length ();
            }
        } else {
            return true;
        }

        if (m_inputted_keys.size () == 1 && m_inputted_keys [0].length () == 0) {
            m_inputted_keys.clear ();
            m_inputing_key = 0;
            m_inputing_caret = 0;
        }

        if (m_add_phrase_mode != 1) {
            if (m_converted_strings.size () > m_inputing_key) {
                m_converted_strings.erase (m_converted_strings.begin () + m_inputing_key, m_converted_strings.end ());
                m_converted_indexes.erase (m_converted_indexes.begin () + m_inputing_key, m_converted_indexes.end ());
            }
            refresh_lookup_table ();
        }

        refresh_preedit ();
        refresh_aux_string ();
        return true;
    }
    return false;
}

bool
TableInstance::space_hit ()
{
    if (m_inputted_keys.size ()) {
        if (m_add_phrase_mode == 1) {
            if (m_factory->m_table.add_phrase (m_inputted_keys [0], m_last_committed)) {
                m_add_phrase_mode = 2;
                m_factory->refresh (true);
            } else {
                m_add_phrase_mode = 3;
            }

            m_inputted_keys.clear ();
            m_last_committed = WideString ();
            m_inputing_caret = m_inputing_key = 0;
        } else {
            if (m_converted_strings.size () == 0 && m_lookup_table.number_of_candidates () == 0)
                return true;

            if (m_lookup_table.number_of_candidates () && m_converted_strings.size () < m_inputted_keys.size ()) {
                lookup_to_converted (m_lookup_table.get_cursor_pos ());
                refresh_lookup_table ();
            }

            if (m_converted_strings.size () == m_inputted_keys.size () ||
                (m_converted_strings.size () == m_inputted_keys.size () - 1 &&
                 m_inputted_keys [m_inputing_key].length () == 0))
                commit_converted ();
        }

        refresh_preedit ();
        refresh_aux_string ();

        return true;
    }
    return false;
}

bool
TableInstance::enter_hit ()
{
    if (m_inputted_keys.size ()) {
        if (m_add_phrase_mode == 1) {
            if (m_factory->m_table.add_phrase (m_inputted_keys [0], m_last_committed)) {
                m_add_phrase_mode = 2;
                m_factory->refresh (true);
            } else {
                m_add_phrase_mode = 3;
            }

            m_inputted_keys.clear ();
            m_last_committed = WideString ();
            m_inputing_caret = m_inputing_key = 0;

            refresh_preedit ();
            refresh_aux_string ();
        } else {
            WideString str;

            for (size_t i=0; i<m_inputted_keys.size (); ++i)
                str += utf8_mbstowcs (m_inputted_keys [i]);

            reset ();
            commit_string (str);
        }
        return true;
    }

    m_last_committed = WideString ();

    return false;
}

bool
TableInstance::lookup_cursor_up ()
{
    if (m_inputted_keys.size () && m_lookup_table.number_of_candidates ()) {
        m_lookup_table.cursor_up ();
        refresh_lookup_table (true, false);
        refresh_preedit ();
        refresh_aux_string ();
        return true;
    }
    return false;
}

bool
TableInstance::lookup_cursor_down ()
{
    if (m_inputted_keys.size () && m_lookup_table.number_of_candidates ()) {
        m_lookup_table.cursor_down ();
        refresh_lookup_table (true, false);
        refresh_preedit ();
        refresh_aux_string ();
        return true;
    }
    return false;
}

bool
TableInstance::lookup_cursor_up_to_longer ()
{
    if (m_inputted_keys.size () && m_lookup_table.number_of_candidates ()) {
        //Get current lookup table cursor
        uint32 cursor = m_lookup_table.get_cursor_pos ();
        //Get current phrase length
        uint32 curlen = m_factory->m_table.get_phrase_length (m_lookup_table_indexes [cursor]);

        do {
            m_lookup_table.cursor_up ();
            cursor = m_lookup_table.get_cursor_pos ();
            if (curlen < m_factory->m_table.get_phrase_length (m_lookup_table_indexes [cursor]))
                break;
        } while (cursor);

        refresh_lookup_table (true, false);
        refresh_preedit ();
        refresh_aux_string ();
        return true;
    }
    return false;
}

bool
TableInstance::lookup_cursor_down_to_shorter ()
{
    if (m_inputted_keys.size () && m_lookup_table.number_of_candidates ()) {
        uint32 entries = m_lookup_table.number_of_candidates ();
        //Get current lookup table cursor
        uint32 cursor = m_lookup_table.get_cursor_pos ();
        //Get current phrase length
        uint32 curlen = m_factory->m_table.get_phrase_length (m_lookup_table_indexes [cursor]);

        do {
            m_lookup_table.cursor_down ();
            cursor = m_lookup_table.get_cursor_pos ();
            if (curlen > m_factory->m_table.get_phrase_length (m_lookup_table_indexes [cursor]))
                break;
        } while (cursor < entries - 1);

        refresh_lookup_table (true, false);
        refresh_preedit ();
        refresh_aux_string ();
        return true;
    }
    return false;
}

bool
TableInstance::lookup_page_up ()
{
    if (m_inputted_keys.size () &&
         m_lookup_table.get_current_page_size () <
         m_lookup_table.number_of_candidates ()) {

        m_lookup_table.page_up ();
        refresh_lookup_table (true, false);
        refresh_preedit ();
        refresh_aux_string ();
        return true;
    }
    return false;
}

bool
TableInstance::lookup_page_down ()
{
    if (m_inputted_keys.size () && 
         m_lookup_table.get_current_page_size () <
         m_lookup_table.number_of_candidates ()) {

        if (!m_lookup_table.page_down ())
            while (m_lookup_table.page_up ()) NULL;

        refresh_lookup_table (true, false);
        refresh_preedit ();
        refresh_aux_string ();
        return true;
    }
    return false;
}

bool
TableInstance::lookup_select (int index)
{
    if (m_inputted_keys.size ()) {
        if (m_lookup_table.number_of_candidates () == 0)
            return true;

        index += m_lookup_table.get_current_page_start ();

        lookup_to_converted (index);

        if (m_converted_strings.size () == m_inputted_keys.size () ||
            (m_converted_strings.size () == m_inputted_keys.size () - 1 &&
             m_inputted_keys [m_inputing_key].length () == 0))
            commit_converted ();

        refresh_lookup_table ();
        refresh_preedit ();
        refresh_aux_string ();

        return true;
    }

    return false;
}

bool
TableInstance::post_process (char key)
{
    // Auto select and commit the candidate item when an invalid key is pressed.
    if (m_factory->m_table.is_auto_commit () &&
        m_converted_strings.size () == m_inputing_key &&
        m_inputing_key + 1 == m_inputted_keys.size () &&
        m_inputing_caret == m_inputted_keys [m_inputing_key].length () &&
        m_lookup_table.number_of_candidates ()) {

        lookup_to_converted (m_lookup_table.get_cursor_pos ());
        commit_converted ();

        refresh_lookup_table ();
        refresh_preedit ();
        refresh_aux_string ();
    }

    if (m_inputted_keys.size ()) return true;

    if ((ispunct (key) && m_full_width_punct [m_forward?1:0]) ||
        ((isalnum (key) || key == 0x20) && m_full_width_letter [m_forward?1:0])) {
        WideString str;
        if (key == '.')
            str.push_back (0x3002);
        else if (key == '\\')
            str.push_back (0x3001);
        else if (key == '^') {
            str.push_back (0x2026);
            str.push_back (0x2026);
        } else if (key == '\"') {
            if (!m_double_quotation_state)
                str.push_back (0x201c);
            else
                str.push_back (0x201d);
            m_double_quotation_state = !m_double_quotation_state;
        } else if (key == '\'') {
            if (!m_single_quotation_state)
                str.push_back (0x2018);
            else
                str.push_back (0x2019);
            m_single_quotation_state = !m_single_quotation_state;
        } else {
            str.push_back (scim_wchar_to_full_width (key));
        }

        commit_string (str);

        m_last_committed = WideString ();

        return true;
    }

    return false;
}

bool
TableInstance::delete_phrase ()
{
    if (m_lookup_table.number_of_candidates ()) {
        int pos       = m_lookup_table.get_cursor_pos ();
        uint32 offset = m_lookup_table_indexes [pos];

        if (m_factory->m_table.delete_phrase (offset)) {
            m_factory->refresh (true);
            refresh_lookup_table ();
        }
        return true;
    }
    return false;
}

void
TableInstance::lookup_to_converted (int index)
{
    if (index < 0 || index >= m_lookup_table.number_of_candidates ())
        return;

    uint32 offset  = m_lookup_table_indexes [index];
    WideString str = m_factory->m_table.get_phrase (offset);

    m_converted_strings.push_back (str);
    m_converted_indexes.push_back (offset);

    if (m_inputing_key < m_converted_strings.size ()) {
        m_inputing_key = m_converted_strings.size ();
        if (m_inputing_key >= m_inputted_keys.size ())
            m_inputted_keys.push_back (String (""));
        m_inputing_caret = 0;
    }
}

void
TableInstance::commit_converted ()
{
    if (m_converted_strings.size ()) {
        WideString res;

        for (size_t i=0; i<m_converted_strings.size (); ++i)
            res += m_converted_strings [i];

        // Hide preedit string before committing string,
        // to prevent some buggy clients from inserting the string into wrong place.
        // Preedit string will be refreshed after return from commit_converted ().
        hide_preedit_string ();
        commit_string (res);

        if (utf8_wcstombs (m_last_committed).length () >= 255)
            m_last_committed = WideString ();

        m_last_committed += res;

        m_inputted_keys.erase (m_inputted_keys.begin (), m_inputted_keys.begin () + m_converted_strings.size ());
        m_inputing_key -= m_converted_strings.size ();

        if (m_inputted_keys.size () == 1 && m_inputted_keys [0].length () == 0) {
            m_inputted_keys.clear ();
            m_inputing_key = 0;
            m_inputing_caret = 0;
        }

        if (m_inputted_keys.size ()) {
            m_inputing_key = m_inputted_keys.size () - 1;
            m_inputing_caret = m_inputted_keys [m_inputing_key].length ();
        }

        if (m_factory->m_table.is_dynamic_adjust ()){
            for (size_t i = 0; i < m_converted_indexes.size (); ++i) {
                uint32 freq = m_factory->m_table.get_phrase_frequency (m_converted_indexes [i]);
                if (freq < SCIM_GT_MAX_PHRASE_FREQ) {
                    uint32 delta = ((SCIM_GT_MAX_PHRASE_FREQ - freq) >> SCIM_GT_PHRASE_FREQ_DELTA_SHIFT);
                    freq += (delta ? delta : 1);
                    m_factory->m_table.set_phrase_frequency (m_converted_indexes [i], freq);
                }
            }
            m_factory->refresh (false);
        }

        m_converted_strings.clear ();
        m_converted_indexes.clear ();
    }
}

void
TableInstance::refresh_preedit ()
{
    WideString preedit_string;
    int start = 0;
    int length = 0;
    int caret = 0;
    size_t i;

    if (m_inputted_keys.size () == 0) {
        hide_preedit_string ();
        return;
    }
 
    for (i = 0; i<m_converted_strings.size (); ++i)
        preedit_string += m_converted_strings [i];

    int inputted_keys = m_inputted_keys.size ();

    if (m_inputted_keys [inputted_keys - 1].length () == 0)
        --inputted_keys;

    // Fill the preedit string.
    if (m_factory->m_table.is_auto_fill () &&
        m_converted_strings.size () == inputted_keys - 1 &&
        m_inputing_caret == m_inputted_keys [m_inputing_key].length () &&
        m_lookup_table.number_of_candidates ()) {

        uint32 offset = m_lookup_table_indexes [m_lookup_table.get_cursor_pos ()];
        WideString str = m_factory->m_table.get_phrase (offset);

        start = preedit_string.length ();
        preedit_string += str;
        length = str.length ();
        caret = preedit_string.length ();
    } else {
        i = m_converted_strings.size ();
        caret = start = preedit_string.length ();

        for (i = m_converted_strings.size (); i < inputted_keys; ++i) {
            if (m_factory->m_table.is_show_key_prompt ()) {
                preedit_string += m_factory->m_table.get_key_prompt (m_inputted_keys [i]);
                if (i == m_inputing_key)
                    caret += (m_factory->m_table.get_key_prompt (m_inputted_keys [i].substr (0, m_inputing_caret))).length ();
            } else {
                preedit_string += utf8_mbstowcs (m_inputted_keys [i]);
                if (i == m_inputing_key)
                    caret += m_inputing_caret;
            }

            if (i == m_converted_strings.size ())
                length = preedit_string.length () - start;

            if (i < inputted_keys - 1)
                preedit_string.push_back ((ucs4_t)' ');

            if (i < m_inputing_key)
                caret = preedit_string.length ();
        }
    }

    if (preedit_string.length () == 0) {
        hide_preedit_string ();
        return;
    }

    AttributeList attrs;

    if (length)
        attrs.push_back (Attribute(start, length, SCIM_ATTR_DECORATE, SCIM_ATTR_DECORATE_HIGHLIGHT));

    update_preedit_string (preedit_string, attrs);
    update_preedit_caret (caret);

    show_preedit_string ();
}

void
TableInstance::refresh_lookup_table (bool show, bool refresh)
{
    m_lookup_table.set_page_size (m_factory->m_table.get_select_keys ().size ());

    if (refresh) {
        std::vector <uint32> phrases;
        WideString str;

        m_lookup_table.clear ();
        m_lookup_table_indexes.clear ();

        if (m_converted_strings.size () < m_inputted_keys.size ()) {

            String key = m_inputted_keys [m_converted_strings.size ()];

            if (key.length () &&
                m_factory->m_table.find (phrases,
                                         key,
                                         m_factory->m_user_phrase_first,
                                         m_factory->m_long_phrase_first)) {

                bool show_full_hint = m_factory->m_table.is_wildcard_key (key);

                for (size_t i = 0; i < phrases.size (); ++i) {
                    str = m_factory->m_table.get_phrase (phrases [i]);

                    if (m_iconv.test_convert (str)) {
                        if (m_factory->m_show_key_hint) {
                            String hint = m_factory->m_table.get_key (phrases [i]);

                            if (show_full_hint)
                                str += utf8_mbstowcs (hint);
                            else if (hint.length () > key.length ())
                                str += utf8_mbstowcs (hint.substr (key.length ()));
                        }
#if 0
                        AttributeList attrs;

                        if (m_factory->m_table.is_user_phrase (phrases [i]))
                            attrs.push_back (Attribute (0, str.length (), SCIM_ATTR_FOREGROUND, SCIM_RGB_COLOR(32, 32, 255)));

                        m_lookup_table.append_candidate (str, attrs);
#endif
                        m_lookup_table.append_candidate (str);
                        m_lookup_table_indexes.push_back (phrases [i]);
                    }
                }
            }
        }
    }

    if (show) {
        if (m_lookup_table.number_of_candidates () &&
            (m_factory->m_table.is_always_show_lookup () ||
             m_inputing_key < m_inputted_keys.size () - 1 ||
             m_inputing_caret < m_inputted_keys [m_inputing_key].length () ||
             m_converted_strings.size () < m_inputted_keys.size () - 1)) {
            update_lookup_table (m_lookup_table);
            show_lookup_table ();
        } else {
            hide_lookup_table ();
        }
    }
}

void
TableInstance::refresh_aux_string ()
{
    WideString    prompt;
    AttributeList attributes;

    if (m_add_phrase_mode == 1) {
        prompt = utf8_mbstowcs (_("Input a key string for phrase: ")) + m_last_committed;
    } else if (m_add_phrase_mode == 2) {
        prompt = utf8_mbstowcs (_("Success."));
        attributes.push_back (Attribute (0, prompt.length (), SCIM_ATTR_FOREGROUND, SCIM_RGB_COLOR(32, 255, 32)));
    } else if (m_add_phrase_mode == 3) {
        prompt = utf8_mbstowcs (_("Failed."));
        attributes.push_back (Attribute (0, prompt.length (), SCIM_ATTR_FOREGROUND, SCIM_RGB_COLOR(255, 32, 32)));
    } else {
        if (!m_factory->m_show_prompt || m_inputted_keys.size () == 0) {
            hide_aux_string ();
            return;
        }

        if (!m_factory->m_table.is_show_key_prompt ())
            prompt = m_factory->m_table.get_key_prompt (m_inputted_keys [m_inputing_key]);

        if (m_lookup_table.number_of_candidates () && ! m_factory->m_show_key_hint) {
            prompt += utf8_mbstowcs (" <");
            unsigned int att_start = prompt.length ();

            if (m_factory->m_table.is_show_key_prompt ())
                prompt += m_factory->m_table.get_key_prompt (m_factory->m_table.get_key (
                            m_lookup_table_indexes [m_lookup_table.get_cursor_pos ()]));
            else
                prompt += utf8_mbstowcs (m_factory->m_table.get_key (
                            m_lookup_table_indexes [m_lookup_table.get_cursor_pos ()]));

            unsigned int att_length = prompt.length () - att_start;
            prompt += utf8_mbstowcs (">");
            attributes.push_back (Attribute (att_start, att_length, SCIM_ATTR_FOREGROUND, SCIM_RGB_COLOR(128, 128, 255)));
        }
    }

    if (prompt.length ()) {
        update_aux_string (prompt, attributes);
        show_aux_string ();
    } else {
        hide_aux_string ();
    }
}

bool
TableInstance::match_key_event (const std::vector<KeyEvent>& keyvec,
                                      const KeyEvent& key)
{
    std::vector<KeyEvent>::const_iterator kit; 

    for (kit = keyvec.begin (); kit != keyvec.end (); ++kit) {
        if (key.code == kit->code && key.mask == kit->mask)
            if (!(key.mask & SCIM_KEY_ReleaseMask) || m_prev_key.code == key.code)
                return true;
    }
    return false;
}
/*
vi:ts=4:nowrap:ai:expandtab
*/
