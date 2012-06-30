/** @file scim_generic_table.cpp
 * Implementation of class GenericKeyIndexLib and GenericTablePhraseLib.
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
 * $Id: scim_generic_table.cpp,v 1.10 2006/01/12 08:43:29 suzhe Exp $
 *
 */

#define Uses_STL_FUNCTIONAL
#define Uses_STL_VECTOR
#define Uses_STL_IOSTREAM
#define Uses_STL_FSTREAM
#define Uses_STL_ALGORITHM
#define Uses_STL_UTILITY
#define Uses_STL_IOMANIP
#define Uses_C_STDIO
#define Uses_C_CTYPE
#define Uses_C_STDLIB
#define Uses_SCIM_UTILITY
#define Uses_SCIM_LOOKUP_TABLE

#include <unistd.h>
#include <scim.h>
#include <sys/mman.h>
#include "scim_generic_table.h"
#include "scim_table_private.h"

using namespace scim;

// Helper functions
static inline String
_trim_blank (const String &str)
{
    String::size_type begin, len;

    begin = str.find_first_not_of (" \t\n\v");

    if (begin == String::npos)
        return String ();

    len = str.find_last_not_of (" \t\n\v");

    if (len != String::npos)
        len = len - begin + 1;

    return str.substr (begin, len);
}

static inline String
_get_param_portion (const String &str, const String &delim = "=")
{
    String ret = str;
    String::size_type pos = ret.find_first_of (delim);

    if (pos != String::npos)
        ret.erase (pos, String::npos);

    return _trim_blank (ret);
}

static inline String
_get_value_portion (const String &str, const String &delim = "=")
{
    String ret = str;
    String::size_type pos;

    pos = ret.find_first_of (delim);

    if (pos != String::npos)
        ret.erase (0, pos + 1);
    else
        return String();

    return _trim_blank (ret);
}

static inline String  
_get_line (FILE *fp)
{
    char temp [4096];
    String res;

    while (fp && !feof (fp)) {
        if (!fgets (temp, 4096, fp)) break;

        res = _trim_blank (String (temp));

        if (res.length () &&
            !(res.length () >= 3 && res.substr (0, 3) == String ("###")))
            return res;
    }

    return String ();
}

static inline WideString
_hex_to_wide_string (const String &str)
{
    WideString ret;
    uint32 i = 0;

    while (i <= str.length () - 6 && str [i] == '0' && tolower (str [i+1]) == 'x') {
        ucs4_t wc = (ucs4_t) strtol (str.substr (i+2, 4).c_str (), NULL, 16);

        if (wc)
            ret.push_back (wc);

        i += 6;
    }
    return ret;
}

// Implementations of GenericTableHeader members.
GenericTableHeader::GenericTableHeader ()
    : m_keyboard_layout (SCIM_KEYBOARD_Unknown),
      m_max_key_length (0),
      m_show_key_prompt (false),
      m_auto_select (false),
      m_auto_wildcard (false),
      m_auto_commit (false),
      m_auto_split (true),
      m_auto_fill (false),
      m_discard_invalid_key (false),
      m_dynamic_adjust (false),
      m_always_show_lookup (true),
      m_use_full_width_punct (true),
      m_def_full_width_punct (true),
      m_use_full_width_letter (true),
      m_def_full_width_letter (false),
      m_updated (false)
{
}

GenericTableHeader::~GenericTableHeader ()
{
}

void
GenericTableHeader::clear ()
{
    m_uuid = String ();
    m_icon_file = String ();
    m_serial_number = String ();
    m_author = String ();
    m_languages = String ();
    m_status_prompt = String ();
    m_valid_input_chars = String ();
    m_key_end_chars = String ();
    m_single_wildcard_chars = String ();
    m_multi_wildcard_chars = String ();
    m_default_name = String ();

    m_local_names.clear ();
    m_char_prompts.clear ();

    m_split_keys.clear ();
    m_commit_keys.clear ();
    m_forward_keys.clear ();
    m_page_up_keys.clear ();
    m_page_down_keys.clear ();
    m_select_keys.clear ();
    m_keyboard_layout = SCIM_KEYBOARD_Unknown;
    m_max_key_length = 0;
    m_auto_select = false;
    m_auto_wildcard = false;
    m_auto_commit = false;
    m_auto_split = true;
    m_auto_fill = false;
    m_dynamic_adjust = false;
    m_always_show_lookup = true;
    m_use_full_width_punct = true;
    m_def_full_width_punct = true;
    m_use_full_width_letter = true;
    m_def_full_width_letter = false;
    m_updated = false;
}

bool
GenericTableHeader::is_valid_input_char (char input) const
{
    return std::binary_search (m_valid_input_chars.begin (), m_valid_input_chars.end (), input);
}

bool
GenericTableHeader::is_key_end_char (char input) const
{
    return std::binary_search (m_key_end_chars.begin (), m_key_end_chars.end (), input);
}

bool
GenericTableHeader::is_single_wildcard_char (char single) const
{
    return std::binary_search (m_single_wildcard_chars.begin (), m_single_wildcard_chars.end (), single);
}

bool
GenericTableHeader::is_multi_wildcard_char (char multi) const
{
    return std::binary_search (m_multi_wildcard_chars.begin (), m_multi_wildcard_chars.end (), multi);
}

bool
GenericTableHeader::is_split_char (char split) const
{
    if (split) {
        for (size_t i = 0; i < m_split_keys.size (); ++ i)
            if (m_split_keys [i].get_ascii_code () == split)
                return true;
    }

    return false;
}

bool
GenericTableHeader::load (FILE *fp)
{
    if (!fp || feof (fp)) return false;

    clear ();

    String temp;
    String paramstr;
    String valuestr;
    String single_wildcard;
    String multi_wildcard;
    String key_end_chars;

    std::vector <KeyEvent> split_keys;

    //Read definitions
    if (_get_line (fp) != String ("BEGIN_DEFINITION"))
        return false;

    while (1) {
        temp = _get_line (fp);

        if (temp.length () == 0) return false;
        if (temp == String ("END_DEFINITION")) break;

        paramstr = _get_param_portion (temp);
        valuestr = _get_value_portion (temp);

        if (paramstr.length () == 0 && valuestr.length () == 0) {
            std::cerr << "Invalid line in header: " << temp << "\n";
            continue;
        }

        if (paramstr == "NAME") { // Get table default name.
            m_default_name = valuestr;
        } else if (paramstr.substr (0,5) == "NAME.") { //Get table name for each locales.
            m_local_names.push_back (
                paramstr.substr (5,paramstr.length () - 5) + " = " + valuestr);
        } else if (paramstr == "UUID") {
            m_uuid = valuestr;
        } else if (paramstr == "ICON") {
            m_icon_file = valuestr;
        } else if (paramstr == "LANGUAGES") { //Get supported languages.
            m_languages = valuestr;
        } else if (paramstr == "AUTHOR") { //Get the table author.
            m_author = valuestr;
        } else if (paramstr == "SERIAL_NUMBER") {
            m_serial_number = valuestr;
        } else if (paramstr == "KEYBOARD_LAYOUT") {
            m_keyboard_layout = scim_string_to_keyboard_layout (valuestr);
        } else if (paramstr == "STATUS_PROMPT") {
            m_status_prompt = valuestr;
        } else if (paramstr == "SHOW_KEY_PROMPT") { //Get show_key_prompt value.
            if (valuestr == "TRUE" || valuestr == "true" || valuestr == "True")
                m_show_key_prompt = true;
            else
                m_show_key_prompt = false;
        } else if (paramstr == "AUTO_SELECT") { //Get auto_select value.
            if (valuestr == "TRUE" || valuestr == "true" || valuestr == "True")
                m_auto_select = true;
            else
                m_auto_select = false;
        } else if (paramstr == "AUTO_WILDCARD") { //Get auto wildcard value.
            if (valuestr == "TRUE" || valuestr == "true" || valuestr == "True")
                m_auto_wildcard = true;
            else
                m_auto_wildcard = false;
        } else if (paramstr == "AUTO_COMMIT") { //Get auto commit value.
            if (valuestr == "TRUE" || valuestr == "true" || valuestr == "True")
                m_auto_commit = true;
            else
                m_auto_commit = false;
        } else if (paramstr == "AUTO_SPLIT") { //Get auto split value.
            if (valuestr == "TRUE" || valuestr == "true" || valuestr == "True")
                m_auto_split = true;
            else
                m_auto_split = false;
        } else if (paramstr == "AUTO_FILL") { //Get auto fill value.
            if (valuestr == "TRUE" || valuestr == "true" || valuestr == "True")
                m_auto_fill = true;
            else
                m_auto_fill = false;
        } else if (paramstr == "DISCARD_INVALID_KEY") { //Get auto fill value.
            if (valuestr == "TRUE" || valuestr == "true" || valuestr == "True")
                m_discard_invalid_key = true;
            else
                m_discard_invalid_key = false;
        } else if (paramstr == "DYNAMIC_ADJUST") { //Get dynamic_adjust value.
            if (valuestr == "TRUE" || valuestr == "true" || valuestr == "True")
                m_dynamic_adjust = true;
            else
                m_dynamic_adjust = false;
        } else if (paramstr == "ALWAYS_SHOW_LOOKUP") { //Get always_show_lookup value.
            if (valuestr == "TRUE" || valuestr == "true" || valuestr == "True")
                m_always_show_lookup = true;
            else
                m_always_show_lookup = false;
        } else if (paramstr == "USE_FULL_WIDTH_PUNCT") { //Get use_full_width_punct value.
            if (valuestr == "TRUE" || valuestr == "true" || valuestr == "True")
                m_use_full_width_punct = true;
            else
                m_use_full_width_punct = false;
        } else if (paramstr == "DEF_FULL_WIDTH_PUNCT") { //Get def_full_width_punct value.
            if (valuestr == "TRUE" || valuestr == "true" || valuestr == "True")
                m_def_full_width_punct = true;
            else
                m_def_full_width_punct = false;
        } else if (paramstr == "USE_FULL_WIDTH_LETTER") { //Get def_full_width_letter value.
            if (valuestr == "TRUE" || valuestr == "true" || valuestr == "True")
                m_use_full_width_letter = true;
            else
                m_use_full_width_letter = false;
        } else if (paramstr == "DEF_FULL_WIDTH_LETTER") { //Get def_full_width_letter value.
            if (valuestr == "TRUE" || valuestr == "true" || valuestr == "True")
                m_def_full_width_letter = true;
            else
                m_def_full_width_letter = false;
        } else if (paramstr == "VALID_INPUT_CHARS") { //Get valid input chars.
            m_valid_input_chars = valuestr;
        } else if (paramstr == "KEY_END_CHARS") { //Get valid input chars.
            key_end_chars = valuestr;
        } else if (paramstr == "SINGLE_WILDCARD_CHAR") { //Get single wildcard char.
            single_wildcard = valuestr;
        } else if (paramstr == "MULTI_WILDCARD_CHAR") { //Get multi wildcard char.
            multi_wildcard = valuestr;
        } else if (paramstr == "SPLIT_KEYS") { //Get split keys.
            scim_string_to_key_list (split_keys, valuestr);
        } else if (paramstr == "COMMIT_KEYS") { //Get commit keys.
            scim_string_to_key_list (m_commit_keys, valuestr);
        } else if (paramstr == "FORWARD_KEYS") { //Get forward keys.
            scim_string_to_key_list (m_forward_keys, valuestr);
        } else if (paramstr == "SELECT_KEYS") { //Get select keys.
            scim_string_to_key_list (m_select_keys, valuestr);
        } else if (paramstr == "PAGE_UP_KEYS") {
            scim_string_to_key_list (m_page_up_keys, valuestr);
        } else if (paramstr == "PAGE_DOWN_KEYS") {
            scim_string_to_key_list (m_page_down_keys, valuestr);
        } else if (paramstr == "MODE_SWITCH_KEYS") {
            scim_string_to_key_list (m_mode_switch_keys, valuestr);
        } else if (paramstr == "FULL_WIDTH_PUNCT_KEYS") {
            scim_string_to_key_list (m_full_width_punct_keys, valuestr);
        } else if (paramstr == "FULL_WIDTH_LETTER_KEYS") {
            scim_string_to_key_list (m_full_width_letter_keys, valuestr);
        } else if (paramstr == "MAX_KEY_LENGTH") {
            m_max_key_length = atoi (valuestr.c_str ());
        } else if (paramstr == "BEGIN_CHAR_PROMPTS_DEFINITION") { //Read char names.
            while (1) {
                temp = _get_line (fp);

                if (temp == String ("END_CHAR_PROMPTS_DEFINITION"))
                    break;

                if (temp.length () < 3 || temp [1] != ' ')
                    return false;

                m_char_prompts.push_back (String (temp));
            }
        } else {
            std::cerr << "Invalid line in header: " << temp << "\n";
        }
    }

    //Post process inputted information.

    if (!m_uuid.length () || !m_serial_number.length ())
        return false;

    if (m_max_key_length <= 0 || m_max_key_length > SCIM_GT_MAX_KEY_LENGTH)
        return false;

    if (!m_valid_input_chars.length ())
        return false;

    if (m_default_name.length () == 0) {
        if (m_local_names.size ())
            m_default_name = _get_value_portion (m_local_names [0]);
        else
            return false;
    }

    size_t i;

    std::sort (m_valid_input_chars.begin (), m_valid_input_chars.end ());

    for (i=0; i<single_wildcard.length (); ++i)
        if (!is_valid_input_char (single_wildcard [i]))
            m_single_wildcard_chars.push_back (single_wildcard [i]);

    std::sort (m_single_wildcard_chars.begin (), m_single_wildcard_chars.end ());

    for (i=0; i<multi_wildcard.length (); ++i)
        if (!is_valid_input_char (multi_wildcard [i]) && !is_single_wildcard_char (multi_wildcard [i]))
            m_multi_wildcard_chars.push_back (multi_wildcard [i]);

    for (i=0; i<key_end_chars.length (); ++i)
        if (is_valid_input_char (key_end_chars [i]))
            m_key_end_chars.push_back (key_end_chars [i]);

    for (i=0; i<split_keys.size (); ++i)
        if (!is_valid_input_char (split_keys [i].get_ascii_code ()))
            m_split_keys.push_back (split_keys [i]);

    std::sort (m_key_end_chars.begin (), m_key_end_chars.end ());
    std::sort (m_multi_wildcard_chars.begin (), m_multi_wildcard_chars.end ());
    std::sort (m_char_prompts.begin (), m_char_prompts.end ());
    std::sort (m_local_names.begin (), m_local_names.end ());

    if (m_select_keys.size () > SCIM_LOOKUP_TABLE_MAX_PAGESIZE)
        m_select_keys.erase (m_select_keys.begin () + SCIM_LOOKUP_TABLE_MAX_PAGESIZE,
                             m_select_keys.end ());

    return true;
}

bool
GenericTableHeader::save (FILE *fp)
{
    size_t i;
    String temp;

    if (!fp) return false;

    fprintf (fp, "### Begin Table definition.\n");
    fprintf (fp, "BEGIN_DEFINITION\n");

    fprintf (fp, "UUID = %s\n", m_uuid.c_str ());

    fprintf (fp, "SERIAL_NUMBER = %s\n", m_serial_number.c_str ());

    if (m_icon_file.length ())
        fprintf (fp, "ICON = %s\n", m_icon_file.c_str ());
    else
        fprintf (fp, "### ICON =\n");

    if (m_default_name.length ())
        fprintf (fp, "NAME = %s\n", m_default_name.c_str ());
    else
        fprintf (fp, "### NAME =\n");

    for (i = 0; i < m_local_names.size (); ++i)
        fprintf (fp, "NAME.%s\n", m_local_names [i].c_str ());

    if (m_languages.length ())
        fprintf (fp, "LANGUAGES = %s\n", m_languages.c_str ());
    else
        fprintf (fp, "### LOCALES =\n");

    if (m_author.length ())
        fprintf (fp, "AUTHOR = %s\n", m_author.c_str ());
    else
        fprintf (fp, "### AUTHOR =\n");

    if (m_status_prompt.length ())
        fprintf (fp, "STATUS_PROMPT = %s\n", m_status_prompt.c_str ());
    else
        fprintf (fp, "### STATUS_PROMPT =\n");

    fprintf (fp, "KEYBOARD_LAYOUT = %s\n", scim_keyboard_layout_to_string (m_keyboard_layout).c_str ());
    fprintf (fp, "VALID_INPUT_CHARS = %s\n", m_valid_input_chars.c_str ());

    if (m_key_end_chars.length ())
        fprintf (fp, "KEY_END_CHARS = %s\n", m_key_end_chars.c_str ());
    else
        fprintf (fp, "### KEY_END_CHARS =\n");

    if (m_single_wildcard_chars.length ())
        fprintf (fp, "SINGLE_WILDCARD_CHAR = %s\n", m_single_wildcard_chars.c_str ());
    else
        fprintf (fp, "### SINGLE_WILDCARD_CHAR =\n");

    if (m_multi_wildcard_chars.length ())
        fprintf (fp, "MULTI_WILDCARD_CHAR = %s\n", m_multi_wildcard_chars.c_str ());
    else
        fprintf (fp, "### MULTI_WILDCARD_CHAR =\n");

    scim_key_list_to_string (temp, m_split_keys);
    if (temp.length ())
        fprintf (fp, "SPLIT_KEYS = %s\n", temp.c_str ());
    else
        fprintf (fp, "### SPLIT_KEYS =\n");

    scim_key_list_to_string (temp, m_commit_keys);
    if (temp.length ())
        fprintf (fp, "COMMIT_KEYS = %s\n", temp.c_str ());
    else
        fprintf (fp, "### COMMIT_KEYS =\n");

    scim_key_list_to_string (temp, m_forward_keys);
    if (temp.length ())
        fprintf (fp, "FORWARD_KEYS = %s\n", temp.c_str ());
    else
        fprintf (fp, "### FORWARD_KEYS =\n");

    scim_key_list_to_string (temp, m_select_keys);
    if (temp.length ())
        fprintf (fp, "SELECT_KEYS = %s\n", temp.c_str ());
    else
        fprintf (fp, "### SELECT_KEYS =\n");

    scim_key_list_to_string (temp, m_page_up_keys);
    if (temp.length ())
        fprintf (fp, "PAGE_UP_KEYS = %s\n", temp.c_str ());
    else
        fprintf (fp, "### PAGE_UP_KEYS =\n");

    scim_key_list_to_string (temp, m_page_down_keys);
    if (temp.length ())
        fprintf (fp, "PAGE_DOWN_KEYS = %s\n", temp.c_str ());
    else
        fprintf (fp, "### PAGE_DOWN_KEYS =\n");

    scim_key_list_to_string (temp, m_mode_switch_keys);
    if (temp.length ())
        fprintf (fp, "MODE_SWITCH_KEYS = %s\n", temp.c_str ());
    else
        fprintf (fp, "### MODE_SWITCH_KEYS =\n");

    scim_key_list_to_string (temp, m_full_width_punct_keys);
    if (temp.length ())
        fprintf (fp, "FULL_WIDTH_PUNCT_KEYS = %s\n", temp.c_str ());
    else
        fprintf (fp, "### FULL_WIDTH_PUNCT_KEYS =\n");

    scim_key_list_to_string (temp, m_full_width_letter_keys);
    if (temp.length ())
        fprintf (fp, "FULL_WIDTH_LETTER_KEYS = %s\n", temp.c_str ());
    else
        fprintf (fp, "### FULL_WIDTH_LETTER_KEYS =\n");

    fprintf (fp, "MAX_KEY_LENGTH = %u\n", m_max_key_length);

    fprintf (fp, "SHOW_KEY_PROMPT = %s\n", (m_show_key_prompt?"TRUE":"FALSE"));
    fprintf (fp, "AUTO_SELECT = %s\n", (m_auto_select?"TRUE":"FALSE"));
    fprintf (fp, "AUTO_WILDCARD = %s\n", (m_auto_wildcard?"TRUE":"FALSE"));
    fprintf (fp, "AUTO_COMMIT = %s\n", (m_auto_commit?"TRUE":"FALSE"));
    fprintf (fp, "AUTO_SPLIT = %s\n", (m_auto_split?"TRUE":"FALSE"));
    fprintf (fp, "AUTO_FILL = %s\n", (m_auto_fill?"TRUE":"FALSE"));
    fprintf (fp, "DISCARD_INVALID_KEY = %s\n", (m_discard_invalid_key?"TRUE":"FALSE"));
    fprintf (fp, "DYNAMIC_ADJUST = %s\n", (m_dynamic_adjust?"TRUE":"FALSE"));
    fprintf (fp, "ALWAYS_SHOW_LOOKUP = %s\n", (m_always_show_lookup?"TRUE":"FALSE"));
    fprintf (fp, "USE_FULL_WIDTH_PUNCT = %s\n", (m_use_full_width_punct?"TRUE":"FALSE"));
    fprintf (fp, "DEF_FULL_WIDTH_PUNCT = %s\n", (m_def_full_width_punct?"TRUE":"FALSE"));
    fprintf (fp, "USE_FULL_WIDTH_LETTER = %s\n", (m_use_full_width_letter?"TRUE":"FALSE"));
    fprintf (fp, "DEF_FULL_WIDTH_LETTER = %s\n", (m_def_full_width_letter?"TRUE":"FALSE"));

    if (m_char_prompts.size ()) {
        fprintf (fp, "BEGIN_CHAR_PROMPTS_DEFINITION\n");
        for (i = 0; i < m_char_prompts.size (); ++ i)
            fprintf (fp, "%s\n", m_char_prompts [i].c_str ());
        fprintf (fp, "END_CHAR_PROMPTS_DEFINITION\n");
    }

    fprintf (fp, "END_DEFINITION\n\n");

    m_updated = false;

    return true;
}

WideString
GenericTableHeader::get_name (const String & locale) const
{
    if (locale.length () == 0)
        return utf8_mbstowcs (m_default_name);

    String lang, param, value;
    String::size_type dot;

    dot = locale.find_first_of ('.');
    if (dot != String::npos)
        lang = locale.substr (0, dot);
    else
        lang = locale;

    for (size_t i=0; i<m_local_names.size (); ++i) {
        param = _get_param_portion (m_local_names [i]);
        value = _get_value_portion (m_local_names [i]);
        if ((param.length () > lang.length () && param.substr (0, lang.length ()) == lang) ||
            (param.length () < lang.length () && lang.substr (0, param.length ()) == param) ||
            (param == lang))
            return utf8_mbstowcs (value);
    }
    return utf8_mbstowcs (m_default_name);
}

class __StringLessThanByFirstChar
{
public:
    bool operator () (const String & lhs, char rhs) const {
        return lhs [0] < rhs;
    }
    bool operator () (char lhs, const String & rhs) const {
        return lhs < rhs [0];
    }
    bool operator () (const String & lhs, const String & rhs) const {
        return lhs [0] < rhs [0];
    }
};

WideString
GenericTableHeader::get_char_prompt (char input) const
{
    std::vector <String>::const_iterator it = 
        std::lower_bound (m_char_prompts.begin (),
                          m_char_prompts.end (),
                          input,
                          __StringLessThanByFirstChar ());

    if (it != m_char_prompts.end () && (*it) [0] == input)
        return utf8_mbstowcs (it->substr (2, it->length () - 2));

    return utf8_mbstowcs (&input, 1);
}

WideString
GenericTableHeader::get_key_prompt (const String &key) const
{
    WideString prompt;

    for (uint32 i=0; i<key.length (); ++i)
        prompt += get_char_prompt (key [i]);

    return prompt;
}


// Implementations of GenericTableContent members.

class OffsetLessByPhrase
{
    const unsigned char *m_ptr;
public:
    OffsetLessByPhrase (const unsigned char *p) : m_ptr (p) {}

    bool operator () (uint32 lhs, uint32 rhs) const {
        const unsigned char *l = m_ptr + lhs;
        const unsigned char *r = m_ptr + rhs;

        size_t lklen = (size_t) (*(l++) & 0x3F);
        size_t rklen = (size_t) (*(r++) & 0x3F);

        size_t llen = (size_t) (*l);
        size_t rlen = (size_t) (*r);

        for (l += (3+lklen), r += (3+rklen); llen && rlen; --llen, --rlen, ++l, ++r)
            if (*l != *r) return *l < *r;

        return llen < rlen;
    }

    bool operator () (uint32 lhs, const String &rhs) const {
        const unsigned char *l = m_ptr + lhs;
        const unsigned char *r = (const unsigned char *) rhs.c_str ();

        size_t lklen = (size_t) (*(l++) & 0x3F);
        size_t llen = (size_t) (*l);
        size_t rlen = rhs.length ();

        for (l += (3+lklen); llen && rlen; --llen, --rlen, ++l, ++r)
            if (*l != *r) return *l < *r;

        return llen < rlen;
    }

    bool operator () (const String &lhs, uint32 rhs) const {
        const unsigned char *l = (const unsigned char *) lhs.c_str ();
        const unsigned char *r = m_ptr + rhs;

        size_t llen = lhs.length ();
        size_t rklen = (size_t) (*(r++) & 0x3F);
        size_t rlen = (size_t) (*r);

        for (r += (3+rklen); llen && rlen; --llen, --rlen, ++l, ++r)
            if (*l != *r) return *l < *r;

        return llen < rlen;
    }
};

class OffsetLessByKeyFixedLen
{
    const unsigned char *m_ptr;
    size_t               m_len;
public:
    OffsetLessByKeyFixedLen (const unsigned char *p, size_t len) : m_ptr (p), m_len (len) {}

    bool operator () (uint32 lhs, uint32 rhs) const {
        const unsigned char *l = m_ptr + lhs + 4;
        const unsigned char *r = m_ptr + rhs + 4;

        for (size_t i = 0; i < m_len; ++i, ++l, ++r)
            if (*l != *r) return *l < *r;

        return false;
    }

    bool operator () (uint32 lhs, const String &rhs) const {
        const unsigned char *l = m_ptr + lhs + 4;
        const unsigned char *r = (const unsigned char *) rhs.c_str ();

        for (size_t i = 0; i < m_len; ++i, ++l, ++r)
            if (*l != *r) return *l < *r;

        return false;
    }

    bool operator () (const String &lhs, uint32 rhs) const {
        const unsigned char *l = (const unsigned char *) lhs.c_str ();
        const unsigned char *r = m_ptr + rhs + 4;

        for (size_t i = 0; i < m_len; ++i, ++l, ++r)
            if (*l != *r) return *l < *r;

        return false;
    }
};

class OffsetLessByKeyFixedLenMask
{
    const unsigned char *m_ptr;
    size_t               m_len;
    int                  m_mask [SCIM_GT_MAX_KEY_LENGTH];
public:
    OffsetLessByKeyFixedLenMask (const unsigned char *p,
                                 size_t               len,
                                 const String        &key,
                                 unsigned char        wc)
        : m_ptr (p), m_len (len) {
        for (size_t i = 0; i < len; ++i) {
            if (key [i] == wc) m_mask [i] = 0;
            else m_mask [i] = 1;
        }
    }

    bool operator () (uint32 lhs, uint32 rhs) const {
        const unsigned char *l = m_ptr + lhs + 4;
        const unsigned char *r = m_ptr + rhs + 4;

        for (size_t i = 0; i < m_len; ++i, ++l, ++r)
            if (m_mask [i] && *l != *r) return *l < *r;

        return false;
    }

    bool operator () (uint32 lhs, const String &rhs) const {
        const unsigned char *l = m_ptr + lhs + 4;
        const unsigned char *r = (const unsigned char *) rhs.c_str ();

        for (size_t i = 0; i < m_len; ++i, ++l, ++r)
            if (m_mask [i] && *l != *r) return *l < *r;

        return false;
    }

    bool operator () (const String &lhs, uint32 rhs) const {
        const unsigned char *l = (const unsigned char *) lhs.c_str ();
        const unsigned char *r = m_ptr + rhs + 4;

        for (size_t i = 0; i < m_len; ++i, ++l, ++r)
            if (m_mask [i] && *l != *r) return *l < *r;

        return false;
    }
};

class OffsetCompareByKeyLenAndFreq
{
    const unsigned char *m_ptr;
public:
    OffsetCompareByKeyLenAndFreq (const unsigned char *p) : m_ptr (p) {}

    bool operator () (uint32 lhs, uint32 rhs) const {
        const unsigned char *l = m_ptr + lhs;
        const unsigned char *r = m_ptr + rhs;

        size_t llen = (*l & 0x3F);
        size_t rlen = (*r & 0x3F);

        if (llen < rlen)
            return true;
        else if (llen == rlen)
            return scim_bytestouint16 (l + 2) > scim_bytestouint16 (r + 2);

        return false;
    }
};

class OffsetGreaterByPhraseLength
{
    const unsigned char *m_ptr;
public:
    OffsetGreaterByPhraseLength (const unsigned char *p) : m_ptr (p) {}

    bool operator () (uint32 lhs, uint32 rhs) const {
        const unsigned char *l = m_ptr + lhs + 1;
        const unsigned char *r = m_ptr + rhs + 1;

        if (*l > *r)
            return true;
        else if (*l == *r)
            return scim_bytestouint16 (l + 1) > scim_bytestouint16 (r + 1);

        return false;
    }
};

GenericTableContent::GenericTableContent ()
    : m_single_wildcard_char (0),
      m_multi_wildcard_char (0),
      m_max_key_length (0),
      m_mmapped (false),
      m_mmapped_size (0),
      m_mmapped_ptr (0),
      m_content (0),
      m_content_size (0),
      m_content_allocated_size (0),
      m_updated (false),
      m_offsets (0),
      m_offsets_attrs (0),
      m_offsets_by_phrases_inited (false)
{
}

bool
GenericTableContent::init (const GenericTableHeader &header)
{
    size_t i;

    clear ();

    for (i = 0; i < 256; ++ i)
        m_char_attrs [i] = 0;
 
    m_single_wildcard_char = 0;
    m_multi_wildcard_char = 0;

    m_max_key_length = std::min (header.get_max_key_length (), (size_t) SCIM_GT_MAX_KEY_LENGTH);

    if (m_max_key_length) {
        if (m_offsets) delete [] m_offsets;
        if (m_offsets_attrs) delete [] m_offsets_attrs;

        m_offsets = new(std::nothrow) std::vector <uint32> [m_max_key_length];
        if (!m_offsets) return false;

        m_offsets_attrs = new(std::nothrow) std::vector <OffsetGroupAttr> [m_max_key_length];
        if (!m_offsets_attrs) {
            delete [] m_offsets;
            return false;
        }

        String chars = header.get_valid_input_chars ();

        for (i = 0; i < chars.length (); ++ i)
            m_char_attrs [(size_t) ((unsigned char) chars [i])] = GT_CHAR_ATTR_VALID_CHAR;

        chars = header.get_key_end_chars ();
        for (i = 0; i < chars.length (); ++ i)
            m_char_attrs [(size_t) ((unsigned char) chars [i])] |= GT_CHAR_ATTR_KEY_END_CHAR;

        set_single_wildcard_chars (header.get_single_wildcard_chars ());
        set_multi_wildcard_chars (header.get_multi_wildcard_chars ());

        return true;
    }
    return false;
}

GenericTableContent::~GenericTableContent ()
{
    if (m_mmapped) {
        munmap (m_mmapped_ptr, m_mmapped_size);
    } else if (m_content) {
        delete [] m_content;
    }

    delete [] m_offsets;
    delete [] m_offsets_attrs;
}

void
GenericTableContent::set_single_wildcard_chars (const String &single)
{
    if (m_max_key_length) {
        size_t i;
        for (i = 0; i < 256; ++ i) {
            if (is_single_wildcard_char (m_char_attrs [i]))
                m_char_attrs [i] = 0;
        }

        m_single_wildcard_char = 0;

        for (i = 0; i < single.length (); ++ i)
            if (!m_char_attrs [(size_t) ((unsigned char) single[i])])
                m_char_attrs [(size_t) ((unsigned char) single[i])] = GT_CHAR_ATTR_SINGLE_WILDCARD;

        for (i = 0; i < 256; ++ i) {
            if (m_char_attrs [i] == GT_CHAR_ATTR_SINGLE_WILDCARD) {
                m_single_wildcard_char = (char) i;
                break;
            }
        }

        //No defined single wildcard char, choose one
        if (!m_single_wildcard_char) {
            for (i = 1; i < 256; ++ i) {
                if (!m_char_attrs [i]) {
                    m_single_wildcard_char = (char) i;
                    m_char_attrs [i] = GT_CHAR_ATTR_SINGLE_WILDCARD;
                    break;
                }
            }
        }
    }
}

void
GenericTableContent::set_multi_wildcard_chars (const String &multi)
{
    if (m_max_key_length) {
        size_t i;
        for (i = 0; i < 256; ++ i) {
            if (is_multi_wildcard_char (m_char_attrs [i]))
                m_char_attrs [i] = 0;
        }

        m_multi_wildcard_char = 0;

        for (i = 0; i < multi.length (); ++ i)
            if (!m_char_attrs [(uint32) multi[i]])
                m_char_attrs [(uint32) multi[i]] = GT_CHAR_ATTR_MULTI_WILDCARD;

        for (i = 0; i < 256; ++ i) {
            if (m_char_attrs [i] == GT_CHAR_ATTR_MULTI_WILDCARD) {
                m_multi_wildcard_char = (char) i;
                break;
            }
        }

        //No defined multi wildcard char, choose one
        if (!m_multi_wildcard_char) {
            for (i = 1; i < 256; ++ i) {
                if (!m_char_attrs [i]) {
                    m_multi_wildcard_char = (char) i;
                    m_char_attrs [i] = GT_CHAR_ATTR_MULTI_WILDCARD;
                    break;
                }
            }
        }
    }
}

void
GenericTableContent::set_max_key_length (size_t max_key_length)
{
    if (m_max_key_length && m_offsets && m_offsets_attrs && max_key_length > m_max_key_length) {
        std::vector<uint32> *offsets;
        std::vector<OffsetGroupAttr> *offsets_attrs;

        offsets = new(std::nothrow) std::vector <uint32> [max_key_length];
        if (!offsets) return;

        offsets_attrs = new(std::nothrow) std::vector <OffsetGroupAttr> [max_key_length];
        if (!offsets_attrs) {
            delete offsets;
            return;
        }

        for (size_t i = 0; i < m_max_key_length; ++i) {
            offsets [i] = m_offsets [i];
            offsets_attrs [i] = m_offsets_attrs [i];
        }

        delete [] m_offsets;
        delete [] m_offsets_attrs;

        m_offsets = offsets;
        m_offsets_attrs = offsets_attrs;
        m_max_key_length = max_key_length;
    }
}

bool
GenericTableContent::load_text (FILE *fp)
{
    if (!fp || feof (fp) || !m_max_key_length || !m_offsets)
        return false;

    String temp;
    String paramstr;
    String valuestr;
    String phrasestr;
    String freqstr;

    WideString wide_phrase;

    std::vector <String> keys;
    std::vector <String> phrases;
    std::vector <uint32> frequencies;

    uint32 freq;
    uint32 lengths_count [SCIM_GT_MAX_KEY_LENGTH];

    size_t i;

    clear ();

    for (i = 0; i < SCIM_GT_MAX_KEY_LENGTH; ++i)
        lengths_count [i] = 0;

    if (_get_line (fp) != String ("BEGIN_TABLE"))
        return false;

    while (!feof (fp)) {
        temp = _get_line (fp);

        if (temp.length () == 0) return false;
        if (temp == String ("END_TABLE")) break;
 
        paramstr = _get_param_portion (temp, " \t");
        valuestr = _get_value_portion (temp, " \t");

        if (paramstr.length () == 0 || valuestr.length () == 0) {
            std::cerr << "Invalid line in content: " << temp << "\n";
            continue;
        }

        phrasestr = _get_param_portion (valuestr, " \t");
        freqstr   = _get_value_portion (valuestr, " \t");

        if (phrasestr.length () >= 6 && phrasestr [0] == '0' && tolower (phrasestr[1]) == 'x')
            wide_phrase = _hex_to_wide_string (phrasestr);
        else
            wide_phrase = utf8_mbstowcs (phrasestr);

        if (!wide_phrase.length ()) continue;

        if (freqstr.length ())
            freq = atoi (freqstr.c_str ());
        else
            freq = ~0;

        phrasestr = utf8_wcstombs (wide_phrase);
        while (phrasestr.length () > 255) {
            wide_phrase.erase (wide_phrase.length () - 1);
            phrasestr = utf8_wcstombs (wide_phrase);
        }

        if (paramstr.length () > m_max_key_length)
            paramstr.erase (m_max_key_length);

        if (is_valid_no_wildcard_key (paramstr)) {
            keys.push_back (paramstr);
            phrases.push_back (phrasestr);
            frequencies.push_back (freq);
            ++lengths_count [paramstr.length ()];
        }
    }

    // Use phrase sequence as frequency to retain the correct order, if the frequency is missing.
    for (i = 0; i < frequencies.size (); ++i) {
        if (frequencies [i] == (uint32) ~0)
            frequencies [i] = lengths_count [keys [i].length ()] --;

        if (frequencies [i] > 65535)
            frequencies [i] = 65535;
    }

    // Calculate the content size.
    uint32 content_size = 0;
    for (i = 0; i < keys.size (); ++i) {
        content_size += keys [i].length ();
        content_size += phrases [i].length ();
        content_size += 4;
    }

    // The content can not be larger than 2GB
    if (content_size >= 0x7FFFFFFF)
        return false;

    m_content = new(std::nothrow) unsigned char [content_size];

    if (!m_content) return false;

    m_content_allocated_size = content_size;
    m_content_size           = content_size;

    // Store the phrases and build index
    unsigned char *p = m_content;
    uint32 key_length, phrase_length;
    for (i = 0; i < keys.size (); ++i) {
        key_length    = keys [i].length ();
        phrase_length = phrases [i].length ();

        m_offsets [key_length - 1].push_back ((uint32) (p - m_content));

        *(p++) = static_cast <unsigned char> (0x80 | (key_length & 0x3F));
        *(p++) = static_cast <unsigned char> (phrase_length);

        scim_uint16tobytes (p, frequencies [i]);
        p += 2;

        memcpy (p, keys [i].c_str (), key_length);
        p += key_length;

        memcpy (p, phrases [i].c_str (), phrase_length);
        p += phrase_length;
    }

    sort_all_offsets ();

    return true;
}

bool
GenericTableContent::load_binary (FILE *fp, bool mmapped)
{
    if (!fp || feof (fp) || !m_max_key_length || !m_offsets)
        return false;

    clear ();

    if (_get_line (fp) != String ("BEGIN_TABLE"))
        return false;

    unsigned char buff [4];
    long cur_pos, end_pos;

    if (fread (buff, 4, 1, fp) != 1)
        return false;

    uint32 content_size = scim_bytestouint32 (buff);

    if (!content_size || content_size >= 0x7FFFFFFF)
        return false;

    cur_pos = ftell (fp);
    fseek (fp, 0, SEEK_END);
    end_pos = ftell (fp);
    fseek (fp, cur_pos, SEEK_SET);

    if (end_pos < content_size)
        return false;

    if (mmapped) {
        m_mmapped_ptr = mmap (0, (size_t) end_pos, PROT_READ | PROT_WRITE, MAP_PRIVATE, fileno (fp), 0);
        if (m_mmapped_ptr != MAP_FAILED) {
            m_mmapped = true;
            m_mmapped_size = end_pos;
            m_content_size = content_size;
            m_content = static_cast<unsigned char*>(m_mmapped_ptr) + cur_pos;
        } else {
            m_mmapped_ptr = 0;
            m_mmapped_size = 0;
            m_mmapped = false;
        }
    }

    // if not mapped, or map failed, then load the content directly.
    if (!m_mmapped) {
        m_content = new(std::nothrow) unsigned char [content_size];

        if (!m_content) return false;

        m_content_allocated_size = content_size;
        m_content_size           = content_size;

        if (fread (m_content, m_content_size, 1, fp) != 1) {
            clear ();
            return false;
        }
    }

    // Now create the index
    unsigned char *p = m_content;
    uint32 key_length;
    uint32 phrase_length;

    while (p - m_content < m_content_size) {
        key_length = static_cast<uint32> ((*p) & 0x3F);
        phrase_length = static_cast<uint32> (*(p+1));

        if (!key_length || !phrase_length) {
            clear ();
            return false;
        }

        if ((*p) & 0x80)
            m_offsets [key_length - 1].push_back (static_cast<uint32>(p - m_content));

        p += (4 + key_length + phrase_length);
    }

    sort_all_offsets ();

    return true;
}

bool
GenericTableContent::load_freq_text (FILE *fp)
{
    if (!valid () || !fp || feof (fp)) return false;

    String temp;
    String paramstr;
    String valuestr;

    uint32 offset;
    uint32 freq;

    if (_get_line (fp) != String ("BEGIN_FREQUENCY_TABLE"))
        return false;

    while (!feof (fp)) {
        temp = _get_line (fp);

        if (temp.length () == 0) return false;
        if (temp == String ("END_FREQUENCY_TABLE")) break;
 
        paramstr = _get_param_portion (temp, " \t");
        valuestr = _get_value_portion (temp, " \t");
 
        if (paramstr.length () == 0 || valuestr.length () == 0)
            return false;

        offset = atoi (paramstr.c_str ());
        freq   = atoi (valuestr.c_str ());

        if (!set_phrase_frequency (offset, freq))
            return false;
    }

    m_updated = true;

    return true;
}

bool
GenericTableContent::load_freq_binary (FILE *fp)
{
    if (!valid () || !fp || feof (fp)) return false;

    String temp;

    uint32 offset;
    uint32 freq;

    unsigned char buf [8];

    if (_get_line (fp) != String ("BEGIN_FREQUENCY_TABLE"))
        return false;

    while (!feof (fp)) {
        if (fread (buf, 8, 1, fp) != 1)
            return false;

        offset = scim_bytestouint32 (buf);
        freq   = scim_bytestouint32 (buf+4);
 
        if (offset == 0xFFFF && freq == 0xFFFF)
            break;

        if (!set_phrase_frequency (offset, freq))
            return false;
    }

    m_updated = true;

    return true;
}

bool
GenericTableContent::save_text (FILE *fp) const
{
    if (!fp || !valid ())
        return false;

    uint32 key_length;
    uint32 phrase_length;
    uint16 freq;
    const unsigned char *p;

    if (fprintf (fp, "### Begin Table data.\n") < 0 || fprintf (fp, "BEGIN_TABLE\n") < 0)
        return false;

    for (size_t i = 0; i < m_max_key_length; ++i) {
        for (std::vector <uint32>::const_iterator it = m_offsets [i].begin (); it != m_offsets [i].end (); ++ it) {
            p = m_content + *it;
            if ((*p) & 0x80) {
                key_length = static_cast<uint32> ((*(p++) & 0x3F));
                phrase_length = static_cast<uint32> (*(p++));
                freq = scim_bytestouint16 (p);
                p += 2;

                if (fwrite (p, key_length, 1, fp) != 1)
                    return false;

                if (fputc ('\t', fp) == EOF)
                    return false;

                p += key_length;

                if (fwrite (p, phrase_length, 1, fp) != 1)
                    return false;

                p += phrase_length;

                if (fputc ('\t', fp) == EOF)
                    return false;

                if (fprintf (fp, "%u\n", freq) < 0)
                    return false;
            }
        }
    }

    if (fprintf (fp, "END_TABLE\n") < 0)
        return false;

    m_updated = false;

    return true;
}

bool
GenericTableContent::save_binary (FILE *fp) const
{
    if (!fp || !valid ())
        return false;

    uint32 key_length;
    uint32 phrase_length;
    const unsigned char *p;
    size_t i;

    unsigned char buf [4];
    uint32 content_size = 0;

    // Calculate the content size
    for (i = 0; i < m_max_key_length; ++i) {
        for (std::vector <uint32>::const_iterator it = m_offsets [i].begin (); it != m_offsets [i].end (); ++ it) {
            p = m_content + *it;
            if ((*p) & 0x80) {
                key_length = static_cast<uint32> ((*(p++) & 0x3F));
                phrase_length = static_cast<uint32> (*p);
                content_size += (key_length + phrase_length + 4);
            }
        }
    }

    if (fprintf (fp, "### Begin Table data.\n") < 0 || fprintf (fp, "BEGIN_TABLE\n") < 0)
        return false;

    scim_uint32tobytes (buf, content_size);

    // Write the content size first.
    if (fwrite (buf, 4, 1, fp) != 1)
        return false;

    // Save the content
    for (i = 0; i < m_max_key_length; ++i) {
        for (std::vector <uint32>::const_iterator it = m_offsets [i].begin (); it != m_offsets [i].end (); ++ it) {
            p = m_content + *it;
            if ((*p) & 0x80) {
                key_length = static_cast<uint32> ((*p) & 0x3F);
                phrase_length = static_cast<uint32> (*(p + 1));

                if (fwrite (p, 4 + key_length + phrase_length, 1, fp) != 1)
                    return false;
            }
        }
    }

    if (fprintf (fp, "END_TABLE\n") < 0)
        return false;

    m_updated = false;

    return true;
}

bool
GenericTableContent::save_freq_text (FILE *fp) const
{
    if (!fp || !valid ()) return false;

    if (fprintf (fp, "### Begin Frequency data.\n") < 0 || fprintf (fp, "BEGIN_FREQUENCY_TABLE\n") < 0)
        return false;

    const unsigned char *p;
    uint16 freq;

    for (size_t i = 0; i < m_max_key_length; ++i) {
        for (std::vector <uint32>::const_iterator it = m_offsets [i].begin (); it != m_offsets [i].end (); ++ it) {
            p = m_content + *it;

            // Test if the phrase is valid and modified (0x80+0x40).
            if (((*p) & 0xC0) == 0xC0) {
                freq = scim_bytestouint16 (p+2);

                if (fprintf (fp, "%u\t%u\n", *it, freq) < 0)
                    return false;
            }
        }
    }

    if (fprintf (fp, "END_FREQUENCY_TABLE\n") < 0)
        return false;

    m_updated = false;

    return true;
}

bool
GenericTableContent::save_freq_binary (FILE *fp) const
{
    if (!fp || !valid ()) return false;

    if (fprintf (fp, "### Begin Frequency Table data.\n") < 0 || fprintf (fp, "BEGIN_FREQUENCY_TABLE\n") < 0)
        return false;

    const unsigned char *p;
    unsigned char buf [8];

    for (size_t i = 0; i < m_max_key_length; ++i) {
        for (std::vector <uint32>::const_iterator it = m_offsets [i].begin (); it != m_offsets [i].end (); ++ it) {
            p = m_content + *it;

            // Test if the phrase is valid and modified (0x80+0x40).
            if (((*p) & 0xC0) == 0xC0) {
                scim_uint32tobytes (buf, *it);                          // Offset
                scim_uint32tobytes (buf+4, scim_bytestouint16 (p+2));   // Frequency

                if (fwrite (buf, 8, 1, fp) != 1)
                    return false;
            }
        }
    }

    scim_uint32tobytes (buf, 0xFFFF);
    scim_uint32tobytes (buf+4, 0xFFFF);

    if (fwrite (buf, 8, 1, fp) != 1)
        return false;

    if (fprintf (fp, "END_FREQUENCY_TABLE\n") < 0)
        return false;

    m_updated = false;

    return true;
}

bool
GenericTableContent::valid () const
{
    return m_content && m_content_size && m_offsets && m_offsets_attrs && m_max_key_length;
}

bool
GenericTableContent::is_valid_key (const String & key) const
{
    int multi_wildcard_count = 0;

    if (key.length () > m_max_key_length)
        return false;

    for (String::const_iterator i = key.begin (); i != key.end (); ++ i) {
        if (!is_defined_char (*i))
            return false;
        else if (is_multi_wildcard_char (*i))
            multi_wildcard_count ++;
    }

    return multi_wildcard_count < 2;
}

bool
GenericTableContent::is_wildcard_key (const String & key) const
{
    for (String::const_iterator i = key.begin (); i != key.end (); ++ i) {
        if (is_wildcard_char (*i))
            return true;
    }

    return false;
}

bool
GenericTableContent::is_pure_wildcard_key (const String & key) const
{
    for (String::const_iterator i = key.begin (); i != key.end (); ++ i) {
        if (!is_wildcard_char (*i))
            return false;
    }

    return true;
}

bool
GenericTableContent::is_valid_no_wildcard_key (const String & key) const
{
    if (key.length () > m_max_key_length)
        return false;

    for (String::const_iterator i = key.begin (); i != key.end (); ++ i)
        if (is_wildcard_char (*i) || !is_valid_char (*i))
            return false;

    return true;
}

void
GenericTableContent::clear ()
{
    if (m_mmapped) {
        munmap (m_mmapped_ptr, m_mmapped_size);
    } else if (m_content) {
        delete [] m_content;
    }

    m_content = 0;
    m_content_size = 0;
    m_content_allocated_size = 0;
    m_mmapped = false;
    m_mmapped_ptr = 0;
    m_mmapped_size = 0;
    m_updated = false;

    if (m_offsets) {
        for (size_t i = 0; i < m_max_key_length; ++ i)
            m_offsets [i].clear ();
    }

    if (m_offsets_attrs) {
        for (size_t i = 0; i < m_max_key_length; ++ i)
            m_offsets_attrs [i].clear ();
    }
}

bool
GenericTableContent::expand_content_space (uint32 add)
{
    bool result = false;
    if (!m_mmapped) {
        if (m_content_allocated_size - m_content_size < add) {
            uint32 new_size = m_content_size * 2 + 1;

            while (new_size - m_content_size < add)
                new_size = new_size * 2;

            unsigned char *new_space = new (std::nothrow) unsigned char [new_size];

            if (new_space) {
                m_content_allocated_size = new_size;
                if (m_content) {
                    memcpy (new_space, m_content, m_content_size);
                    delete [] m_content;
                }
                m_content = new_space;
                result = true;
            }
        } else {
            result = true;
        }
    }

    return result;
}

void
GenericTableContent::sort_all_offsets ()
{
    if (valid ()) {
        for (size_t i = 0; i < m_max_key_length; ++i)
            std::stable_sort (m_offsets [i].begin (),
                              m_offsets [i].end (),
                              OffsetLessByKeyFixedLen (m_content, i + 1));
        init_all_offsets_attrs ();
    }
}

void
GenericTableContent::init_all_offsets_attrs ()
{
    for (size_t i = 1; i <= m_max_key_length; ++i)
        init_offsets_attrs (i);
}

void
GenericTableContent::init_offsets_attrs (size_t len)
{
    if (valid () && len && len <= m_max_key_length) {

        -- len;

        m_offsets_attrs [len].clear ();

        std::vector <uint32>::const_iterator i;
        size_t count = 0;

        OffsetGroupAttr attr (len+1);

        String wildcard;

        wildcard.insert (wildcard.begin (), len+1, m_single_wildcard_char);

        // Set the wildcard chars
        attr.mask.set (wildcard);

        for (i = m_offsets [len].begin (); i != m_offsets [len].end (); ++ i) {
            attr.mask.set (get_key (*i));
            if (++count == OFFSET_GROUP_SIZE) {
                attr.end = i - m_offsets [len].begin () + 1;
                m_offsets_attrs [len].push_back (attr);
                attr.mask.clear ();
                attr.begin = attr.end;
                count = 0;
                attr.mask.set (wildcard);
            }
        }

        if (count) {
            attr.end = i - m_offsets [len].begin ();
            m_offsets_attrs [len].push_back (attr);
        }

//        fprintf (stderr, "%d Groups for len %d\n", m_offsets_attrs [len].size (), len + 1);
    }
}

void
GenericTableContent::expand_multi_wildcard_key (std::vector <String> &keys, const String &key) const
{
    keys.clear ();

    String::const_iterator begin1 = key.begin ();
    String::const_iterator begin2 = key.begin ();
    String::const_iterator end1 = key.end ();
    String::const_iterator end2 = key.end ();

    for (; begin2 != end2; ++ begin2)
        if (is_multi_wildcard_char (*begin2))
            break;

    // No multi wildcard char
    if (begin2 == end2) {
        keys.push_back (key);
        return;
    }

    String wildcard (&m_single_wildcard_char, 1);
    uint32 remain = m_max_key_length - key.length ();

    end1 = begin2;
    ++begin2;
 
    keys.push_back (String (begin1, end1) + wildcard + String (begin2, end2));

    for (; remain; -- remain) {
        wildcard.push_back (m_single_wildcard_char);
        keys.push_back (String (begin1, end1) + wildcard + String (begin2, end2));
    }
}

bool
GenericTableContent::transform_single_wildcard (String &key) const
{
    bool result = false;
    for (String::iterator i = key.begin (); i != key.end (); ++i) {
        if (is_single_wildcard_char (*i)) {
            *i = m_single_wildcard_char;
            result = true;
        }
    }
    return result;
}

bool
GenericTableContent::find_no_wildcard_key (std::vector <uint32> &offsets, const String &key, size_t len) const
{
    size_t key_len = key.length () - 1;

    size_t old_size = offsets.size ();

    if (!len) len = key_len;
    else --len;

    if (valid ()) {
        OffsetLessByKeyFixedLen find_op (m_content, key_len + 1);
        OffsetLessByKeyFixedLen sort_op (m_content, len + 1);

        std::vector <OffsetGroupAttr>::iterator i = m_offsets_attrs [len].begin ();
        for (; i != m_offsets_attrs [len].end (); ++ i) {
            if (i->mask.check (key)) {
                if (i->dirty) {
                    std::stable_sort (m_offsets [len].begin () + i->begin,
                               m_offsets [len].begin () + i->end,
                               sort_op);
                    i->dirty = false;
                }

                std::vector<uint32>::const_iterator lb, ub;
                lb = std::lower_bound (m_offsets [len].begin () + i->begin,
                                       m_offsets [len].begin () + i->end,
                                       key,
                                       find_op);
                ub = std::upper_bound (m_offsets [len].begin () + i->begin,
                                       m_offsets [len].begin () + i->end,
                                       key,
                                       find_op);
                offsets.insert (offsets.end (), lb, ub);
            }
        }
    }

    return offsets.size () > old_size;
}

bool
GenericTableContent::find_wildcard_key (std::vector <uint32> &offsets, const String &key) const
{
    size_t idx = key.length () - 1;

    size_t old_size = offsets.size ();

    if (valid ()) {
        std::vector <OffsetGroupAttr>::iterator i = m_offsets_attrs [idx].begin ();
        OffsetLessByKeyFixedLenMask less_op (m_content, idx+1, key, m_single_wildcard_char);

        for (; i != m_offsets_attrs [idx].end (); ++ i) {
            if (i->mask.check (key)) {
                i->dirty = true;
                std::stable_sort (m_offsets [idx].begin () + i->begin,
                                  m_offsets [idx].begin () + i->end,
                                  less_op);

                std::vector<uint32>::const_iterator lb, ub;
                lb = std::lower_bound (m_offsets [idx].begin () + i->begin,
                                       m_offsets [idx].begin () + i->end,
                                       key,
                                       less_op);
                ub = std::upper_bound (m_offsets [idx].begin () + i->begin,
                                       m_offsets [idx].begin () + i->end,
                                       key,
                                       less_op);
                offsets.insert (offsets.end (), lb, ub);
            }
        }
    }

    return offsets.size () > old_size;
}

bool
GenericTableContent::find (std::vector <uint32> &offsets,
                           const String         &key,
                           bool                  auto_wildcard,
                           bool                  do_sort,
                           bool                  sort_by_length) const
{
    if (!valid () || key.length () > m_max_key_length) return false;

    String nkey (key);

    transform_single_wildcard (nkey);

    size_t old_size = offsets.size ();

    std::vector <uint32>::size_type begin = offsets.size ();

    if (!is_wildcard_key (nkey)) {
        find_no_wildcard_key (offsets, nkey);

        if (auto_wildcard) {
            for (size_t len = nkey.length () + 1; len <= m_max_key_length; ++ len)
                find_no_wildcard_key (offsets, nkey, len);
        }

    } else {
        std::vector <String> nkeys;

        expand_multi_wildcard_key (nkeys, nkey);

        for (std::vector <String>::iterator i = nkeys.begin (); i != nkeys.end (); ++i) {
            // A pure wildcard key, copy all offsets with equal length directly.
            if (is_pure_wildcard_key (*i)) {
                offsets.insert (offsets.end (),
                                m_offsets [i->length () - 1].begin (),
                                m_offsets [i->length () - 1].end ());
            } else {
                find_wildcard_key (offsets, *i);
            }
        }
    }

    // Sort all matched offsets by phrase length.
    if (do_sort) {
        if (sort_by_length)
            std::stable_sort (offsets.begin () + begin,
                              offsets.end (),
                              OffsetGreaterByPhraseLength (m_content));
        else
            std::stable_sort (offsets.begin () + begin,
                              offsets.end (),
                              OffsetCompareByKeyLenAndFreq (m_content));
    }

    return offsets.size () > old_size;
}

bool
GenericTableContent::search (const String &key, int search_type) const
{
    if (!valid () || key.length () > m_max_key_length ||
        (key.length () == m_max_key_length && search_type == GT_SEARCH_ONLY_LONGER))
        return false;

    String nkey (key);

    transform_single_wildcard (nkey);

    if (!is_wildcard_key (nkey)) {
        if (search_type != GT_SEARCH_ONLY_LONGER && search_no_wildcard_key (nkey))
            return true;

        if (search_type != GT_SEARCH_NO_LONGER) {
            for (size_t len = nkey.length () + 1; len <= m_max_key_length; ++ len)
                if (search_no_wildcard_key (nkey, len))
                    return true;
        }
    } else {
        std::vector <String> nkeys;

        expand_multi_wildcard_key (nkeys, nkey);

        // Single wildcard key and need search longer
        if (search_type != GT_SEARCH_NO_LONGER &&
            nkey.length () < m_max_key_length &&
            nkeys.size () == 1) {
            nkey.push_back (m_multi_wildcard_char);

            expand_multi_wildcard_key (nkeys, nkey);

            if (search_type == GT_SEARCH_INCLUDE_LONGER)
                nkeys.push_back (nkey);
        }

        // It's multi wildcard key and only search longer
        else if (nkeys.size () > 1 && GT_SEARCH_ONLY_LONGER) {
            for (size_t i = 0; i < nkeys.size (); ++i)
                if (nkeys [i].length () < m_max_key_length)
                    nkeys [i].push_back (m_single_wildcard_char);
        }

        for (std::vector <String>::iterator i = nkeys.begin (); i != nkeys.end (); ++i) {
            // A pure wildcard key, copy all offsets with equal length directly.
            if (is_pure_wildcard_key (*i) && m_offsets [i->length () - 1].size ())
                return true;
            else if (search_wildcard_key (*i))
                return true;
        }
    }

    return false;
}

bool
GenericTableContent::search_no_wildcard_key (const String &key, size_t len) const
{
    size_t key_len = key.length () - 1;

    if (!len) len = key_len;
    else --len;

    if (valid ()) {
        OffsetLessByKeyFixedLen find_op (m_content, key_len + 1);
        OffsetLessByKeyFixedLen sort_op (m_content, len + 1);

        std::vector <OffsetGroupAttr>::iterator i = m_offsets_attrs [len].begin ();
        for (; i != m_offsets_attrs [len].end (); ++ i) {
            if (i->mask.check (key)) {
                if (i->dirty) {
                    std::stable_sort (m_offsets [len].begin () + i->begin,
                                      m_offsets [len].begin () + i->end,
                                      sort_op);
                    i->dirty = false;
                }

                if (std::binary_search (m_offsets [len].begin () + i->begin,
                                        m_offsets [len].begin () + i->end,
                                        key,
                                        find_op))
                    return true;
            }
        }
    }

    return false;
}

bool
GenericTableContent::search_wildcard_key (const String &key) const
{
    size_t idx = key.length () - 1;

    if (valid ()) {
        std::vector <OffsetGroupAttr>::iterator i = m_offsets_attrs [idx].begin ();
        OffsetLessByKeyFixedLenMask less_op (m_content, idx+1, key, m_single_wildcard_char);

        for (; i != m_offsets_attrs [idx].end (); ++ i) {
            if (i->mask.check (key)) {
                i->dirty = true;
                std::stable_sort (m_offsets [idx].begin () + i->begin,
                                  m_offsets [idx].begin () + i->end,
                                  less_op);

                if (std::binary_search (m_offsets [idx].begin () + i->begin,
                                        m_offsets [idx].begin () + i->end,
                                        key,
                                        less_op))
                    return true;
            }
        }
    }

    return false;
}

bool
GenericTableContent::search_phrase (const String &key, const WideString &phrase) const
{
    if (!valid () || key.length () > m_max_key_length) return false;

    if (!is_wildcard_key (key) && phrase.length ()) {
        std::vector <uint32> offsets;
        if (find_no_wildcard_key (offsets, key)) {
            String utf8_phrase = utf8_wcstombs (phrase);
            OffsetLessByPhrase op (m_content);

            std::sort (offsets.begin (), offsets.end (), op);
            return std::binary_search (offsets.begin (), offsets.end (), utf8_phrase, op);
        }
    }
    return false;
}

bool
GenericTableContent::delete_phrase (uint32 offset)
{
    size_t len = get_key_length (offset);
    if (!m_mmapped && len && len <= m_max_key_length) {
        *(m_content + offset) &= 0x7F;
        std::stable_sort (m_offsets [len - 1].begin (),
                          m_offsets [len - 1].end ());

        std::vector <uint32>::iterator lb, ub;

        lb = std::lower_bound (m_offsets [len - 1].begin (),
                               m_offsets [len - 1].end (),
                               offset);
        ub = std::upper_bound (m_offsets [len - 1].begin (),
                               m_offsets [len - 1].end (),
                               offset);
        if (lb < ub) {
            m_offsets [len - 1].erase (lb);

            std::stable_sort (m_offsets [len - 1].begin (),
                              m_offsets [len - 1].end (),
                              OffsetLessByKeyFixedLen (m_content, len));

            init_offsets_attrs (len);

            m_updated = true;

            return true;
        }

        std::stable_sort (m_offsets [len - 1].begin (),
                          m_offsets [len - 1].end (),
                          OffsetLessByKeyFixedLen (m_content, len));
    }
    return false;
}

bool
GenericTableContent::add_phrase (const String &key, const WideString &phrase, int freq)
{
    if (!m_mmapped && m_offsets &&
        is_valid_no_wildcard_key (key) && phrase.length () &&
        !search_phrase (key, phrase)) {

        String utf8_phrase = utf8_wcstombs (phrase);
        size_t key_len     = key.length ();
        size_t phrase_len  = utf8_phrase.length ();
        size_t add_size    = key_len + phrase_len + 4;

        if (phrase_len <= 255 && expand_content_space (add_size)) {
            unsigned char *ptr = m_content + m_content_size;

            if (freq > 0xFFFF) freq = 0xFFFF;

            // Add the phrase content
            *(ptr++) = (unsigned char) (0x80 | (key_len & 0x3F));
            *(ptr++) = (unsigned char) phrase_len;
            scim_uint16tobytes (ptr, (uint16) freq);
            ptr += 2;
            memcpy (ptr, key.c_str (), key_len);
            ptr += key_len;
            memcpy (ptr, utf8_phrase.c_str (), phrase_len);

            // Added the offset.
            m_offsets [key_len - 1].push_back (m_content_size);

            std::stable_sort (m_offsets [key_len - 1].begin (),
                              m_offsets [key_len - 1].end (),
                              OffsetLessByKeyFixedLen (m_content, key_len));

            m_content_size += add_size;

            init_offsets_attrs (key_len);

            if (m_offsets_by_phrases_inited)
                init_offsets_by_phrases ();

            m_updated = true;

            return true;
        }
    }
    return false;
}

void
GenericTableContent::init_offsets_by_phrases () const
{
    if (!valid ()) return;

    m_offsets_by_phrases.clear ();

    for (int i = 0; i < m_max_key_length; ++i) {
        m_offsets_by_phrases.insert (m_offsets_by_phrases.end (),
                                     m_offsets [i].begin (),
                                     m_offsets [i].end ());
    }

    std::stable_sort (m_offsets_by_phrases.begin (), m_offsets_by_phrases.end (), OffsetLessByPhrase (m_content));

    m_offsets_by_phrases_inited = true;
}

bool
GenericTableContent::find_phrase (std::vector <uint32> &offsets, const WideString &phrase) const
{
    if (!valid ()) return false;

    if (!m_offsets_by_phrases_inited)
        init_offsets_by_phrases ();

    String utf8 = utf8_wcstombs (phrase);

    if (!utf8.length ()) return false;

    std::vector <uint32>::const_iterator lb, ub;

    lb = std::lower_bound (m_offsets_by_phrases.begin (),
                           m_offsets_by_phrases.end (),
                           utf8,
                           OffsetLessByPhrase (m_content));
    ub = std::upper_bound (m_offsets_by_phrases.begin (),
                           m_offsets_by_phrases.end (),
                           utf8,
                           OffsetLessByPhrase (m_content));

    offsets.insert (offsets.end (), lb, ub);

    return ub > lb;
}

int
GenericTableContent::get_max_phrase_length () const
{
    if (!valid ()) return 0;

    int max_len = 0;

    for (int i = 0; i < m_max_key_length; ++i) {
        for (std::vector <uint32>::const_iterator j = m_offsets [i].begin (); j != m_offsets [i].end (); ++j)
            if (get_phrase_length (*j) > max_len)
                max_len = get_phrase_length (*j);
    }

    return max_len;
}

// Implementation of GenericTableLibrary
GenericTableLibrary::GenericTableLibrary ()
    : m_header_loaded (false),
      m_content_loaded (false)
{
}

static const char scim_generic_table_phrase_lib_text_header [] = "SCIM_Generic_Table_Phrase_Library_TEXT";
static const char scim_generic_table_phrase_lib_binary_header [] = "SCIM_Generic_Table_Phrase_Library_BINARY";
static const char scim_generic_table_phrase_lib_version [] = "VERSION_1_0";

static const char scim_generic_table_freq_lib_text_header [] = "SCIM_Generic_Table_Frequency_Library_TEXT";
static const char scim_generic_table_freq_lib_binary_header [] = "SCIM_Generic_Table_Frequency_Library_BINARY";
static const char scim_generic_table_freq_lib_version [] = "VERSION_1_0";

bool
GenericTableLibrary::init (const String &sys, const String &usr, const String &freq, bool all)
{
    // Can only be initialized one time.
    if (m_header_loaded || m_content_loaded) return false;

    // No file
    if (!sys.length () && !usr.length ()) return false;

    m_sys_file = sys;
    m_usr_file = usr;
    m_freq_file = freq;

    bool ok = load_header ();

    if (ok && all)
        ok = load_content ();

    return ok;
}

bool
GenericTableLibrary::load_header ()
{
    if (m_header_loaded) return true;

    FILE *fp = 0;

    if (m_sys_file.length ())      fp = fopen (m_sys_file.c_str (), "rb");
    else if (m_usr_file.length ()) fp = fopen (m_usr_file.c_str (), "rb");
 
    if (!fp) return false;

    String magic;
    String version;

    GenericTableHeader header;

    bool ok = false;
    bool binary = false;

    magic   = _get_line (fp);
    version = _get_line (fp);

    if (version == String (scim_generic_table_phrase_lib_version) &&
        (magic == String (scim_generic_table_phrase_lib_text_header) ||
         magic == String (scim_generic_table_phrase_lib_binary_header)))
        ok = true;

    if (ok)
        ok = header.load (fp);

    if (ok)
        ok = m_sys_content.init (header);

    if (ok)
        ok = m_usr_content.init (header);

    if (ok) {
        m_header = header;
        m_header_loaded = true;
    }

    fclose (fp);
    return ok;
}

bool
GenericTableLibrary::load_content () const
{
    if (m_content_loaded) return true;

    if (!m_header_loaded) return false;

    FILE *sys_fp = 0, *usr_fp = 0, *freq_fp = 0;

    if (m_sys_file.length ())  sys_fp  = fopen (m_sys_file.c_str (), "rb");
    if (m_usr_file.length ())  usr_fp  = fopen (m_usr_file.c_str (), "rb");
    if (m_freq_file.length ()) freq_fp = fopen (m_freq_file.c_str (), "rb");

    String magic;
    String version;

    bool binary;

    bool ok = false;
    bool sys_loaded = false;
    bool usr_loaded = false;

    GenericTableHeader header;

    if (sys_fp) {
        // Load system table.
        magic   = _get_line (sys_fp);
        version = _get_line (sys_fp);

        if (version == String (scim_generic_table_phrase_lib_version)) {
            if (magic == String (scim_generic_table_phrase_lib_text_header)) {
                binary = false;
                ok = true;
            } else if (magic == String (scim_generic_table_phrase_lib_binary_header)) {
                binary = true;
                ok = true;
            }
        }

        if (ok)
            ok = header.load (sys_fp);

        if (ok)
            ok = (header.get_uuid () == m_header.get_uuid () &&
                  header.get_serial_number () == m_header.get_serial_number ());

        if (ok) {
            if (binary)
                ok = m_sys_content.load_binary (sys_fp, true);
            else
                ok = m_sys_content.load_text (sys_fp);
        }

        sys_loaded = ok;

        fclose (sys_fp);
    }

    if (usr_fp) {
        ok = false;

        // Load user table.
        magic   = _get_line (usr_fp);
        version = _get_line (usr_fp);

        if (version == String (scim_generic_table_phrase_lib_version)) {
            if (magic == String (scim_generic_table_phrase_lib_text_header)) {
                binary = false;
                ok = true;
            } else if (magic == String (scim_generic_table_phrase_lib_binary_header)) {
                binary = true;
                ok = true;
            }
        }

        if (ok)
            ok = header.load (usr_fp);

        if (ok)
            ok = (header.get_uuid () == m_header.get_uuid () &&
                  header.get_serial_number () == m_header.get_serial_number ());

        if (ok) {
            if (binary)
                ok = m_usr_content.load_binary (usr_fp, false);
            else
                ok = m_usr_content.load_text (usr_fp);
        }

        usr_loaded = ok;

        fclose (usr_fp);
    }

    if (sys_loaded && freq_fp) {
        ok = false;

        // Load user table.
        magic   = _get_line (freq_fp);
        version = _get_line (freq_fp);

        if (version == String (scim_generic_table_freq_lib_version)) {
            if (magic == String (scim_generic_table_freq_lib_text_header)) {
                binary = false;
                ok = true;
            } else if (magic == String (scim_generic_table_freq_lib_binary_header)) {
                binary = true;
                ok = true;
            }
        }

        if (ok)
            ok = header.load (freq_fp);

        if (ok)
            ok = (header.get_uuid () == m_header.get_uuid () &&
                  header.get_serial_number () == m_header.get_serial_number ());

        if (ok) {
            if (binary)
                ok = m_sys_content.load_freq_binary (freq_fp);
            else
                ok = m_sys_content.load_freq_text (freq_fp);
        }

        fclose (freq_fp);
    }

    m_content_loaded = (sys_loaded || usr_loaded);

    return m_content_loaded;
}

bool
GenericTableLibrary::save (const String &sys, const String &usr, const String &freq, bool binary)
{
    if (!load_content ()) return false;

    FILE *sys_fp = 0, *usr_fp = 0, *freq_fp = 0;
    bool sys_saved = false;
    bool usr_saved = false;
    bool freq_saved = false;
    bool ok = true;

    // First unlink the table files to avoid crashing the exsiting scim processes
    // which are mapping the same files.
    if (sys.length ()) unlink (sys.c_str ());
    if (usr.length ()) unlink (usr.c_str ());
    if (freq.length ()) unlink (freq.c_str ());

    if (sys.length () && m_sys_content.valid ())
        sys_fp  = fopen (sys.c_str (), "wb");

    if (usr.length () && m_usr_content.valid ())
        usr_fp  = fopen (usr.c_str (), "wb");

    if (freq.length () && m_sys_content.updated ())
        freq_fp = fopen (freq.c_str (), "wb");

    if (sys_fp) {
        ok = (fprintf (sys_fp, "%s\n%s\n",
                (binary ?
                 scim_generic_table_phrase_lib_binary_header :
                 scim_generic_table_phrase_lib_text_header),
                scim_generic_table_phrase_lib_version) > 0);

        if (ok)
            ok = m_header.save (sys_fp);

        if (ok) {
            if (binary)
                ok = m_sys_content.save_binary (sys_fp);
            else
                ok = m_sys_content.save_text (sys_fp);
        }

        sys_saved = ok;

        fclose (sys_fp);
    }

    if (usr_fp) {
        ok = (fprintf (usr_fp, "%s\n%s\n",
                (binary ?
                 scim_generic_table_phrase_lib_binary_header :
                 scim_generic_table_phrase_lib_text_header),
                scim_generic_table_phrase_lib_version) > 0);

        if (ok)
            ok = m_header.save (usr_fp);

        if (ok) {
            if (binary)
                ok = m_usr_content.save_binary (usr_fp);
            else
                ok = m_usr_content.save_text (usr_fp);
        }

        usr_saved = ok;

        fclose (usr_fp);
    }

    if (freq_fp) {
        ok = (fprintf (freq_fp, "%s\n%s\n",
                (binary ?
                 scim_generic_table_freq_lib_binary_header :
                 scim_generic_table_freq_lib_text_header),
                scim_generic_table_freq_lib_version) > 0);

        if (ok)
            ok = m_header.save (freq_fp);

        if (ok) {
            if (binary)
                ok = m_sys_content.save_freq_binary (freq_fp);
            else
                ok = m_sys_content.save_freq_text (freq_fp);
        }

        freq_saved = ok;

        fclose (freq_fp);
    }

    return sys_saved || usr_saved || freq_saved;
}

class IndexCompareByKeyLenAndFreqInLibrary
{
    const GenericTableLibrary *m_lib;
public:
    IndexCompareByKeyLenAndFreqInLibrary (const GenericTableLibrary *p) : m_lib (p) {}

    bool operator () (uint32 lhs, uint32 rhs) const {
        size_t llen = m_lib->get_key_length (lhs);
        size_t rlen = m_lib->get_key_length (rhs);

        if (llen < rlen)
            return true;
        else if (llen == rlen)
            return m_lib->get_phrase_frequency (lhs) > m_lib->get_phrase_frequency (rhs);

        return false;
    }
};

class IndexGreaterByPhraseLengthInLibrary
{
    const GenericTableLibrary *m_lib;
public:
    IndexGreaterByPhraseLengthInLibrary (const GenericTableLibrary *p) : m_lib (p) {}

    bool operator () (uint32 lhs, uint32 rhs) const {
        size_t llen = m_lib->get_phrase_length (lhs);
        size_t rlen = m_lib->get_phrase_length (rhs);

        if (llen > rlen)
            return true;
        else if (llen == rlen)
            return m_lib->get_phrase_frequency (lhs) > m_lib->get_phrase_frequency (rhs);

        return false;
    }
};

bool
GenericTableLibrary::find (std::vector <uint32> &indexes,
                           const String         &key,
                           bool                  user_first,
                           bool                  sort_by_length) const
{
    indexes.clear ();

    if (!load_content ()) return false;

    if (m_usr_content.valid ()) {
        m_usr_content.find (indexes, key, is_auto_wildcard (), user_first, sort_by_length);

        for (std::vector <uint32>::iterator i = indexes.begin (); i != indexes.end (); ++i)
            *i |= 0x80000000;
    }

    if (m_sys_content.valid ()) {
        m_sys_content.find (indexes, key, is_auto_wildcard (), user_first, sort_by_length);
    }

    if (!user_first) {
        if (sort_by_length)
            std::stable_sort (indexes.begin (),
                              indexes.end (),
                              IndexGreaterByPhraseLengthInLibrary (this));
        else
            std::stable_sort (indexes.begin (),
                              indexes.end (),
                              IndexCompareByKeyLenAndFreqInLibrary (this));
    }

    return indexes.size () > 0;
}

bool
GenericTableLibrary::find_phrase (std::vector <uint32> &indexes, const WideString &phrase) const
{
    indexes.clear ();

    if (!load_content ()) return false;

    if (m_usr_content.valid ()) {
        m_usr_content.find_phrase (indexes, phrase);

        for (std::vector <uint32>::iterator i = indexes.begin (); i != indexes.end (); ++i)
            *i |= 0x80000000;
    }

    if (m_sys_content.valid ()) {
        m_sys_content.find_phrase (indexes, phrase);
    }

    return indexes.size () > 0;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
