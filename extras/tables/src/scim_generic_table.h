/** @file scim_generic_table.h
 * definition of GenericTableLib related classes.
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
 * $Id: scim_generic_table.h,v 1.7 2006/08/23 10:25:32 suzhe Exp $
 */

#if !defined (__SCIM_GENERIC_TABLE_H)
#define __SCIM_GENERIC_TABLE_H

#include <scim_types.h>
#include <scim_utility.h>
#include <scim_event.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include <stdio.h>

#define SCIM_GT_MAX_KEY_LENGTH           63

#ifndef SCIM_TABLE_SYSTEM_TABLE_DIR
  #define SCIM_TABLE_SYSTEM_TABLE_DIR (SCIM_DATADIR SCIM_PATH_DELIM_STRING "tables")
#endif

#ifndef SCIM_TABLE_USER_TABLE_DIR
  #define SCIM_TABLE_USER_TABLE_DIR (SCIM_PATH_DELIM_STRING ".scim" SCIM_PATH_DELIM_STRING "user-tables")
#endif

#ifndef SCIM_TABLE_SYSTEM_UPDATE_TABLE_DIR
  #define SCIM_TABLE_SYSTEM_UPDATE_TABLE_DIR (SCIM_PATH_DELIM_STRING ".scim" SCIM_PATH_DELIM_STRING "sys-tables")
#endif

using namespace scim;

class GenericTableHeader
{
    String                 m_uuid;
    String                 m_icon_file;
    String                 m_serial_number;
    String                 m_author;

    String                 m_languages;
    String                 m_status_prompt;

    String                 m_valid_input_chars;
    String                 m_key_end_chars;
    String                 m_single_wildcard_chars;
    String                 m_multi_wildcard_chars;

    String                 m_default_name;
    std::vector <String>   m_local_names;

    std::vector <String>   m_char_prompts;

    std::vector <KeyEvent> m_split_keys;
    std::vector <KeyEvent> m_commit_keys;
    std::vector <KeyEvent> m_forward_keys;

    std::vector <KeyEvent> m_page_up_keys;
    std::vector <KeyEvent> m_page_down_keys;
    std::vector <KeyEvent> m_select_keys;
    std::vector <KeyEvent> m_mode_switch_keys;
    std::vector <KeyEvent> m_full_width_punct_keys;
    std::vector <KeyEvent> m_full_width_letter_keys;

    KeyboardLayout         m_keyboard_layout;

    size_t                 m_max_key_length;

    bool                   m_show_key_prompt;
    bool                   m_auto_select;
    bool                   m_auto_wildcard;
    bool                   m_auto_commit;
    bool                   m_auto_split;
    bool                   m_auto_fill;
    bool                   m_discard_invalid_key;
    bool                   m_dynamic_adjust;
    bool                   m_always_show_lookup;
    bool                   m_use_full_width_punct;
    bool                   m_def_full_width_punct;
    bool                   m_use_full_width_letter;
    bool                   m_def_full_width_letter;

    mutable bool           m_updated;

public:
    GenericTableHeader ();
    ~GenericTableHeader ();

    String     get_uuid              () const { return m_uuid; }
    String     get_icon_file         () const { return m_icon_file; }
    String     get_serial_number     () const { return m_serial_number; }
    WideString get_author            () const { return utf8_mbstowcs (m_author); }

    String     get_languages         () const { return m_languages; }
    WideString get_status_prompt     () const { return utf8_mbstowcs (m_status_prompt); }

    String     get_valid_input_chars () const { return m_valid_input_chars; }
    String     get_key_end_chars     () const { return m_key_end_chars; }
    String     get_single_wildcard_chars () const { return m_single_wildcard_chars; }
    String     get_multi_wildcard_chars  () const { return m_multi_wildcard_chars; }

    const std::vector <KeyEvent> & get_split_keys     () const { return m_split_keys; }
    const std::vector <KeyEvent> & get_commit_keys    () const { return m_commit_keys; }
    const std::vector <KeyEvent> & get_forward_keys   () const { return m_forward_keys; }

    const std::vector <KeyEvent> & get_page_up_keys   () const { return m_page_up_keys; }
    const std::vector <KeyEvent> & get_page_down_keys () const { return m_page_down_keys; }
    const std::vector <KeyEvent> & get_select_keys    () const { return m_select_keys; }
    const std::vector <KeyEvent> & get_mode_switch_keys       () const { return m_mode_switch_keys; }
    const std::vector <KeyEvent> & get_full_width_punct_keys  () const { return m_full_width_punct_keys; }
    const std::vector <KeyEvent> & get_full_width_letter_keys () const { return m_full_width_letter_keys; }

    KeyboardLayout                 get_keyboard_layout() const { return m_keyboard_layout; }

    size_t get_max_key_length   () const { return m_max_key_length; }

    bool is_show_key_prompt     () const { return m_show_key_prompt; }
    bool is_auto_select         () const { return m_auto_select; }
    bool is_auto_wildcard       () const { return m_auto_wildcard; }
    bool is_auto_commit         () const { return m_auto_select && m_auto_commit; }
    bool is_auto_split          () const { return m_auto_split; }
    bool is_auto_fill           () const { return m_auto_select && m_auto_fill; }
    bool is_discard_invalid_key () const { return is_auto_commit () && m_discard_invalid_key; }
    bool is_dynamic_adjust      () const { return m_dynamic_adjust; }
    bool is_always_show_lookup  () const { return !is_auto_fill () || m_always_show_lookup; }
    bool is_use_full_width_punct  () const { return m_use_full_width_punct; }
    bool is_def_full_width_punct  () const { return m_def_full_width_punct; }
    bool is_use_full_width_letter () const { return m_use_full_width_letter; }
    bool is_def_full_width_letter () const { return m_def_full_width_letter; }

    WideString get_name         (const String & locale) const;
    WideString get_char_prompt  (char           input) const;
    WideString get_key_prompt   (const String & key) const;

    bool is_valid_input_char     (char input) const;
    bool is_key_end_char         (char input) const;
    bool is_single_wildcard_char (char single) const;
    bool is_multi_wildcard_char  (char multi) const;
    bool is_split_char           (char split) const;

    void set_show_key_prompt (bool show_key_prompt) {
        m_updated = true;
        m_show_key_prompt = show_key_prompt;
    }

    void set_auto_select    (bool auto_select) {
        m_updated = true;
        m_auto_select = auto_select;
    }

    void set_auto_wildcard  (bool auto_wildcard) {
        m_updated = true;
        m_auto_wildcard = auto_wildcard;
    }

    void set_auto_commit    (bool auto_commit) {
        m_updated = true;
        m_auto_commit = auto_commit;
    }

    void set_auto_split     (bool auto_split) {
        m_updated = true;
        m_auto_split = auto_split;
    }

    void set_auto_fill      (bool auto_fill) {
        m_updated = true;
        m_auto_fill = auto_fill;
    }

    void set_discard_invalid_key (bool discard) {
        m_updated = true;
        m_discard_invalid_key = discard;
    }

    void set_dynamic_adjust (bool dynamic_adjust) {
        m_updated = true;
        m_dynamic_adjust = dynamic_adjust;
    }

    void set_always_show_lookup (bool always) {
        m_updated = true;
        m_always_show_lookup = always;
    }

    void set_use_full_width_punct (bool full) {
        m_updated = true;
        m_use_full_width_punct = full;
    }

    void set_def_full_width_punct (bool full) {
        m_updated = true;
        m_def_full_width_punct = full;
    }

    void set_use_full_width_letter (bool full) {
        m_updated = true;
        m_use_full_width_letter = full;
    }

    void set_def_full_width_letter (bool full) {
        m_updated = true;
        m_def_full_width_letter = full;
    }

    void set_icon_file      (const String & icon_file) {
        m_updated = true;
        m_icon_file = icon_file;
    }

    void set_languages      (const String & languages) {
        m_updated = true;
        m_languages = languages;
    }

    void set_status_prompt  (const WideString & prompt) {
        m_updated = true;
        m_status_prompt = utf8_wcstombs (prompt);
    }

    void set_single_wildcard_chars (const String & single) {
        m_updated = true;
        m_single_wildcard_chars = single;
    }

    void set_multi_wildcard_chars (const String & multi) {
        m_updated = true;
        m_multi_wildcard_chars = multi;
    }

    void set_split_keys  (const std::vector <KeyEvent> & keys) {
        m_updated = true;
        m_split_keys = keys;
    }

    void set_commit_keys  (const std::vector <KeyEvent> & keys) {
        m_updated = true;
        m_commit_keys = keys;
    }

    void set_forward_keys  (const std::vector <KeyEvent> & keys) {
        m_updated = true;
        m_forward_keys = keys;
    }

    void set_select_keys (const std::vector <KeyEvent> & keys) {
        m_updated = true;
        m_select_keys = keys;
    }

    void set_page_up_keys (const std::vector <KeyEvent> & keys) {
        m_updated = true;
        m_page_up_keys = keys;
    }

    void set_page_down_keys (const std::vector <KeyEvent> & keys) {
        m_updated = true;
        m_page_down_keys = keys;
    }

    void set_mode_switch_keys (const std::vector <KeyEvent> & keys) {
        m_updated = true;
        m_mode_switch_keys = keys;
    }

    void set_full_width_punct_keys (const std::vector <KeyEvent> & keys) {
        m_updated = true;
        m_full_width_punct_keys = keys;
    }

    void set_full_width_letter_keys (const std::vector <KeyEvent> & keys) {
        m_updated = true;
        m_full_width_letter_keys = keys;
    }

    void set_keyboard_layout (KeyboardLayout layout) {
        m_updated = true;
        m_keyboard_layout = layout;
    }

    void set_max_key_length (size_t max_key_length) {
        m_updated = true;
        m_max_key_length = max_key_length;
    }

    bool load (FILE *fp);
    bool save (FILE *fp);

    bool updated () const {
        return m_updated;
    }

    bool valid () const {
        return m_uuid.length () && m_max_key_length && m_valid_input_chars.length ();
    }

    void clear ();
};

/**
 * class to store a generic table.
 *
 * The structure of m_content is:
 *
 *         Phrase 1             Phrase x
 *  +-------+-------+-------+...+-------+-------+-------+...
 *  |4 Chars|n Chars|m Chars|   |4 Chars|n Chars|m Chars|
 *  |Header |  Key  |Content|   |Header |  Key  |Content|
 *  +-------+-------+-------+...+-------+-------+-------+...
 *
 *  The content and key must be in UTF-8 encoding.
 * 
 *  Format of Header:
 *    Byte 1   Byte 2   Byte 3   Byte 4
 *   76543210 76543210 76543210 76543210
 *  +--------+--------+--------+--------+
 *  |FMKKKKKK|PPPPPPPP|CCCCCCCC|CCCCCCCC|
 *  +--------+--------+--------+--------+
 *  
 *  The meaning of the bits:
 *  F: this must be set to 1 indicating that this phrase is ok.
 *  M: this is set when the phrase's frequency is changed.
 *  K: 6 bits to store the key length in bytes.
 *  P: 8 bits to store the phrase content length in bytes.
 *  C: 16 bits to store the phrase frequency.
 *  
 */
const int GT_SEARCH_NO_LONGER = 0,
          GT_SEARCH_INCLUDE_LONGER = 1,
          GT_SEARCH_ONLY_LONGER = 2;

const int GT_CHAR_ATTR_VALID_CHAR = 0x01,
          GT_CHAR_ATTR_SINGLE_WILDCARD = 0x03,
          GT_CHAR_ATTR_MULTI_WILDCARD = 0x05,
          GT_CHAR_ATTR_KEY_END_CHAR = 0x81;

class GenericTableContent
{

    #define OFFSET_GROUP_SIZE 32

    // Helper classes
    /*
     * In order to speed up the search process, the offsets array
     * are splitted into several vectors by different key length.
     * Each offsets vector is divided into several groups with small size.
     * Each group has a KeyBitMask to check if a group is probably contain a given key.
     */
    class CharBitMask
    {
        uint32 m_mask [8];
    public:
        CharBitMask () {
            memset ((void *)m_mask, 0, sizeof (m_mask));
        }

        void set (unsigned char ch) {
            m_mask [ch >> 5] |= (1 << (ch & 0x1F));
        }

        bool check (unsigned char ch) const {
            return m_mask [ch >> 5] & (1 << (ch & 0x1F));
        }

        void clear () {
            memset ((void *)m_mask, 0, sizeof (m_mask));
        }
    };

    class KeyBitMask
    {
        CharBitMask   *m_masks;
        size_t         m_len;
    public:
        KeyBitMask (size_t len = 0)
            : m_masks (len ? (new CharBitMask [len]) : 0), m_len (len) {
        }

        KeyBitMask (const KeyBitMask &copy)
            : m_masks (copy.m_len ? (new CharBitMask [copy.m_len]) : 0), m_len (copy.m_len) {
            if (m_len) memcpy (m_masks, copy.m_masks, sizeof (CharBitMask) * m_len);
        }

        ~KeyBitMask () {
            delete [] m_masks;
        }

        void set_len (size_t len) {
            if (len > 0) {
                delete [] m_masks;
                m_masks = new CharBitMask [len];
                m_len = len;
            }
        }

        void swap (KeyBitMask &copy) {
            std::swap (m_masks, copy.m_masks);
            std::swap (m_len, copy.m_len);
        }

        KeyBitMask & operator = (const KeyBitMask &copy) {
            KeyBitMask temp (copy);
            swap (temp);
            return *this;
        }

        void set (const String &key) {
            if (key.length () == m_len) {
                CharBitMask *p = m_masks;
                String::const_iterator i = key.begin ();
                for (;i != key.end (); ++p, ++i)
                    p->set (*i);
            }
        }

        void clear () {
            for (size_t i = 0; i < m_len; ++i)
                m_masks [i].clear ();
        }

        bool check (const String &key) const {
            if (key.length () > m_len) return false;
            CharBitMask *p = m_masks;
            String::const_iterator i = key.begin ();
            for (; i != key.end (); ++p, ++i)
                if (!p->check (*i)) return false;
            return true;
        }
    };

    struct OffsetGroupAttr
    {
        KeyBitMask mask;
        uint32     begin;
        uint32     end;
        bool       dirty;

        OffsetGroupAttr () : begin (0), end (0), dirty (false) { }
        OffsetGroupAttr (size_t len) : mask (len), begin (0), end (0), dirty (false) { }
    };

    // Data members
    uint32                         m_char_attrs [256];
    char                           m_single_wildcard_char;
    char                           m_multi_wildcard_char;

    size_t                         m_max_key_length;

    bool                           m_mmapped;
    size_t                         m_mmapped_size;
    void                          *m_mmapped_ptr;

    unsigned char                 *m_content;
    size_t                         m_content_size;
    size_t                         m_content_allocated_size;

    mutable bool                   m_updated;

    mutable std::vector <uint32>          *m_offsets;
    mutable std::vector <OffsetGroupAttr> *m_offsets_attrs;

    mutable std::vector <uint32>           m_offsets_by_phrases;
    mutable bool                           m_offsets_by_phrases_inited;

public:
    GenericTableContent ();
    ~GenericTableContent ();

    bool init           (const GenericTableHeader& header);

    bool load_text (FILE *fp);
    bool load_binary (FILE *fp, bool mmapped = false);
 
    bool load_freq_text (FILE *fp);
    bool load_freq_binary (FILE *fp);

    bool save_text (FILE *fp) const;
    bool save_binary (FILE *fp) const;

    bool save_freq_text (FILE *fp) const;
    bool save_freq_binary (FILE *fp) const;

    bool valid () const;

    bool updated () const { return m_updated; }

    void clear ();

public:
    bool is_defined_char         (char ch) const { return m_char_attrs [(size_t)((unsigned char) ch)] != 0; }
    bool is_valid_char           (char ch) const { return (m_char_attrs [(size_t)((unsigned char) ch)] & GT_CHAR_ATTR_VALID_CHAR) == GT_CHAR_ATTR_VALID_CHAR; }
    bool is_single_wildcard_char (char ch) const { return m_char_attrs [(size_t)((unsigned char) ch)] == GT_CHAR_ATTR_SINGLE_WILDCARD; }
    bool is_multi_wildcard_char  (char ch) const { return m_char_attrs [(size_t)((unsigned char) ch)] == GT_CHAR_ATTR_MULTI_WILDCARD; }
    bool is_wildcard_char        (char ch) const { return is_single_wildcard_char (ch) || is_multi_wildcard_char (ch); }
    bool is_key_end_char         (char ch) const { return is_valid_char (ch) && (m_char_attrs [(size_t)((unsigned char) ch)] & GT_CHAR_ATTR_KEY_END_CHAR) == GT_CHAR_ATTR_KEY_END_CHAR; }

    bool is_valid_key             (const String & key) const;
    bool is_wildcard_key          (const String & key) const;
    bool is_pure_wildcard_key     (const String & key) const;
    bool is_valid_no_wildcard_key (const String & key) const;

    void set_single_wildcard_chars (const String &single);
    void set_multi_wildcard_chars (const String &single);
    void set_max_key_length (size_t max_key_length);

public:
    String get_key (uint32 offset) const {
        const unsigned char *p = m_content + offset;
        return (*p & 0x80)?String ((const char *)(p + 4), (String::size_type)(*p & 0x3F)):String ();
    }

    size_t get_key_length (uint32 offset) const {
        const unsigned char *p = m_content + offset;
        return (*p & 0x80)?(*p & 0x3F):0;
    }

    WideString get_phrase (uint32 offset) const {
        const unsigned char *p = m_content + offset;
        return (*p & 0x80)?utf8_mbstowcs((const char *)(p + (size_t)(*p & 0x3F) + 4), *(p+1)):WideString ();
    }

    String get_phrase_utf8 (uint32 offset) const {
        const unsigned char *p = m_content + offset;
        return (*p & 0x80)?String((const char *)(p + (size_t)(*p & 0x3F) + 4), (String::size_type) (*(p+1))):String ();
    }

    size_t get_phrase_length (uint32 offset) const {
        const unsigned char *p = m_content + offset;
        return (*p & 0x80)?(*(p+1)):0;
    }

    int get_phrase_frequency (uint32 offset) const {
        const unsigned char *p = m_content + offset;
        return (*p & 0x80)?scim_bytestouint16 (p+2):0;
    }

    bool set_phrase_frequency (uint32 offset, int freq) {
        unsigned char *p = m_content + offset;

        if (offset < m_content_size && (*p & 0x80)) {
            scim_uint16tobytes (p+2, (freq>0xFFFF)?0xFFFF:freq);
            *p |= 0x40;
            m_updated = true;
            return true;
        }
        return false;
    }

    bool delete_phrase (uint32 offset);
    bool add_phrase (const String &key, const WideString &phrase, int freq = 0);

    int get_max_phrase_length () const;

public:
    bool find (std::vector <uint32> &offsets,
               const String         &key,
               bool                  auto_wildcard = false,
               bool                  do_sort = true,
               bool                  sort_by_length = false) const;

    bool find_phrase (std::vector <uint32> &offsets, const WideString &phrase) const;

    bool search (const String &key, int search_type = GT_SEARCH_NO_LONGER) const;

    bool search_phrase (const String &key, const WideString &phrase) const;

private:
    void init_offsets_by_phrases () const;

    void sort_all_offsets ();

    void init_all_offsets_attrs ();
    void init_offsets_attrs (size_t len);

    bool expand_content_space (uint32 add);

    void expand_multi_wildcard_key (std::vector <String> &keys, const String &key) const;

    bool transform_single_wildcard (String &key) const;

    bool find_no_wildcard_key (std::vector <uint32> &offsets, const String &key, size_t len = 0) const;
    bool find_wildcard_key (std::vector <uint32> &offsets, const String &key) const;

    bool search_no_wildcard_key (const String &key, size_t len = 0) const;
    bool search_wildcard_key (const String &key) const;
};

class GenericTableLibrary
{
    GenericTableHeader  m_header;
    mutable GenericTableContent m_sys_content;
    mutable GenericTableContent m_usr_content;

    String              m_sys_file;
    String              m_usr_file;
    String              m_freq_file;

    bool                m_header_loaded;
    mutable bool        m_content_loaded;

public:
    GenericTableLibrary ();

    bool init (const String &sys, const String &usr = "", const String &freq = "", bool all = false);
    bool save (const String &sys, const String &usr = "", const String &freq = "", bool binary = false);

    bool valid () const {
        return m_header_loaded && m_header.valid ();
    }

    bool updated () const {
        return m_header.updated () ||
               m_sys_content.updated () ||
               m_usr_content.updated ();
    }

private:
    bool load_header ();
    bool load_content () const;

public:
    bool is_valid_input_char (char ch) const {
        return m_header.is_valid_input_char (ch);
    }

    bool is_valid_char (char ch) const {
        if (load_content ()) {
            if (m_sys_content.valid ())
                return m_sys_content.is_valid_char (ch);
            else
                return m_usr_content.is_valid_char (ch);
        }
        return false;
    }

    bool is_key_end_char (char ch) const {
        if (load_content ()) {
            if (m_sys_content.valid ())
                return m_sys_content.is_key_end_char (ch);
            else
                return m_usr_content.is_key_end_char (ch);
        }
        return false;
    }

    bool is_wildcard_char (char ch) const {
        if (load_content ()) {
            if (m_sys_content.valid ())
                return m_sys_content.is_wildcard_char (ch);
            else
                return m_usr_content.is_wildcard_char (ch);
        }
        return false;
    }

    bool is_multi_wildcard_char (char ch) const {
        if (load_content ()) {
            if (m_sys_content.valid ())
                return m_sys_content.is_multi_wildcard_char (ch);
            else
                return m_usr_content.is_multi_wildcard_char (ch);
        }
        return false;
    }

    bool is_single_wildcard_char (char ch) const {
        if (load_content ()) {
            if (m_sys_content.valid ())
                return m_sys_content.is_single_wildcard_char (ch);
            else
                return m_usr_content.is_single_wildcard_char (ch);
        }
        return false;
    }

    bool is_valid_key (const String &key) const {
        if (load_content ()) {
            if (m_sys_content.valid ())
                return m_sys_content.is_valid_key (key);
            else
                return m_usr_content.is_valid_key (key);
        }
        return false;
    }

    bool is_defined_key (const String &key, int search_type = GT_SEARCH_INCLUDE_LONGER) const {
        if (load_content ()) {
            if (m_sys_content.valid ())
                return m_sys_content.search (key, search_type) ||
                       m_usr_content.search (key, search_type);
            else
                return m_usr_content.search (key, search_type);
        }
        return false;
    }

    bool is_wildcard_key (const String &key) const {
        if (load_content ()) {
            if (m_sys_content.valid ())
                return m_sys_content.is_wildcard_key (key);
            else
                return m_usr_content.is_wildcard_key (key);
        }
        return false;
    }

    bool is_split_char (char ch) const {
        if (load_content ()) {
            return m_header.is_split_char (ch);
        }
        return false;
    }

public:
    // The indexes stores the matched indexes.
    // The highest bit of the indexes from m_usr_content will be set to 1.
    bool find (std::vector <uint32> &indexes,
               const String         &key,
               bool                  user_first = false,
               bool                  sort_by_length = false) const;

    bool delete_phrase (uint32 index) {
        if (load_content ()) {
            // User phrase
            if (index & 0x80000000)
                return m_usr_content.delete_phrase (index & 0x7FFFFFFF);
            // Sys phrase
            else
                return m_sys_content.delete_phrase (index);
        }
        return false;
    }

    bool add_phrase (const String &key, const WideString &phrase, int freq = 0) {
        if (load_content ()) {
            if (!m_sys_content.search_phrase (key, phrase))
                return m_usr_content.add_phrase (key, phrase, freq);
        }
        return false;
    }

    bool find_phrase (std::vector <uint32> &indexes, const WideString &phrase) const;

public:
    WideString get_key_prompt (const String &key) const {
        return m_header.get_key_prompt (key);
    }

    WideString get_char_prompt (char ch) const {
        return m_header.get_char_prompt (ch);
    }

    WideString get_name (const String &locale) const {
        return m_header.get_name (locale);
    }

    bool is_user_phrase (uint32 index) const {
        // User phrase
        if (index & 0x80000000)
            return true;
        return false;
    }

    String get_key (uint32 index) const {
        if (load_content ()) {
            // User phrase
            if (index & 0x80000000)
                return m_usr_content.get_key (index & 0x7FFFFFFF);
            // Sys phrase
            else
                return m_sys_content.get_key (index);
        }
        return String ();
    }

    size_t get_key_length (uint32 index) const {
        if (load_content ()) {
            // User phrase
            if (index & 0x80000000)
                return m_usr_content.get_key_length (index & 0x7FFFFFFF);
            // Sys phrase
            else
                return m_sys_content.get_key_length (index);
        }
        return 0;
    }

    WideString get_phrase (uint32 index) const {
        if (load_content ()) {
            // User phrase
            if (index & 0x80000000)
                return m_usr_content.get_phrase (index & 0x7FFFFFFF);
            // Sys phrase
            else
                return m_sys_content.get_phrase (index);
        }
        return WideString ();
    }

    size_t get_phrase_length (uint32 index) const {
        if (load_content ()) {
            // User phrase
            if (index & 0x80000000)
                return m_usr_content.get_phrase_length (index & 0x7FFFFFFF);
            // Sys phrase
            else
                return m_sys_content.get_phrase_length (index);
        }
        return 0;
    }

    int get_phrase_frequency (uint32 index) const {
        if (load_content ()) {
            // User phrase
            if (index & 0x80000000)
                return m_usr_content.get_phrase_frequency (index & 0x7FFFFFFF);
            // Sys phrase
            else
                return m_sys_content.get_phrase_frequency (index);
        }
        return 0;
    }

    String get_uuid () const {
        return m_header.get_uuid ();
    }

    String get_serial_number () const {
        return m_header.get_serial_number ();
    }

    String get_icon_file () const {
        return m_header.get_icon_file ();
    }

    WideString get_author () const {
        return m_header.get_author ();
    }

    WideString get_status_prompt () const {
        return m_header.get_status_prompt ();
    }

    String get_language () const {
        String langs = m_header.get_languages ();
        return scim_validate_language (langs.substr (0, langs.find (',')));
    }

    String get_languages () const {
        return m_header.get_languages ();
    }

    String get_valid_input_chars () const {
        return m_header.get_valid_input_chars ();
    }

    String get_single_wildcard_chars () const {
        return m_header.get_single_wildcard_chars ();
    }

    String get_multi_wildcard_chars () const {
        return m_header.get_multi_wildcard_chars ();
    }

    const std::vector <KeyEvent> & get_split_keys () const {
        return m_header.get_split_keys ();
    }

    const std::vector <KeyEvent> & get_commit_keys () const {
        return m_header.get_commit_keys ();
    }

    const std::vector <KeyEvent> & get_forward_keys () const {
        return m_header.get_forward_keys ();
    }

    const std::vector <KeyEvent> & get_page_up_keys () const {
        return m_header.get_page_up_keys ();
    }

    const std::vector <KeyEvent> & get_page_down_keys () const {
        return m_header.get_page_down_keys ();
    }

    const std::vector <KeyEvent> & get_mode_switch_keys () const {
        return m_header.get_mode_switch_keys ();
    }

    const std::vector <KeyEvent> & get_full_width_punct_keys () const {
        return m_header.get_full_width_punct_keys ();
    }

    const std::vector <KeyEvent> & get_full_width_letter_keys () const {
        return m_header.get_full_width_letter_keys ();
    }

    const std::vector <KeyEvent> & get_select_keys () const {
        return m_header.get_select_keys ();
    }

    int get_select_key_pos (const KeyEvent &key) const {
        const std::vector <KeyEvent> & keys = m_header.get_select_keys ();
        for (size_t i = 0; i < keys.size (); ++i)
            if (keys [i] == key)
                return (int) i;
        return -1;
    }

    KeyboardLayout get_keyboard_layout () const {
        return m_header.get_keyboard_layout ();
    }

    size_t get_max_key_length () const {
        return m_header.get_max_key_length ();
    }

    bool is_show_key_prompt () const {
        return m_header.is_show_key_prompt ();
    }

    bool is_auto_select () const {
        return m_header.is_auto_select ();
    }

    bool is_auto_wildcard () const {
        return m_header.is_auto_wildcard ();
    }

    bool is_auto_commit () const {
        return m_header.is_auto_commit ();
    }

    bool is_auto_split () const {
        return m_header.is_auto_split ();
    }

    bool is_auto_fill () const {
        return m_header.is_auto_fill ();
    }

    bool is_discard_invalid_key () const {
        return m_header.is_discard_invalid_key ();
    }

    bool is_dynamic_adjust () const {
        return m_header.is_dynamic_adjust ();
    }

    bool is_always_show_lookup () const {
        return m_header.is_always_show_lookup ();
    }

    bool is_use_full_width_punct () const {
        return m_header.is_use_full_width_punct ();
    }

    bool is_def_full_width_punct () const {
        return m_header.is_def_full_width_punct ();
    }

    bool is_use_full_width_letter () const {
        return m_header.is_use_full_width_letter ();
    }

    bool is_def_full_width_letter () const {
        return m_header.is_def_full_width_letter ();
    }

    int get_max_phrase_length () const {
        if (load_content ())
            return std::max (m_usr_content.get_max_phrase_length (),
                             m_sys_content.get_max_phrase_length ());
        return 0;
    }

    bool set_phrase_frequency (uint32 index, int freq) {
        if (load_content ()) {
            // User phrase
            if (index & 0x80000000)
                return m_usr_content.set_phrase_frequency (index & 0x7FFFFFFF, freq);
            // Sys phrase
            else
                return m_sys_content.set_phrase_frequency (index, freq);
        }
        return false;
    }

    void set_show_key_prompt (bool show_key_prompt) {
        m_header.set_show_key_prompt (show_key_prompt);
    }

    void set_auto_select (bool auto_select) {
        m_header.set_auto_select (auto_select);
    }

    void set_auto_wildcard (bool auto_wildcard) {
        m_header.set_auto_wildcard (auto_wildcard);
    }

    void set_auto_commit (bool auto_commit) {
        m_header.set_auto_commit (auto_commit);
    }

    void set_auto_split (bool auto_split) {
        m_header.set_auto_split (auto_split);
    }

    void set_auto_fill (bool auto_fill) {
        m_header.set_auto_fill (auto_fill);
    }

    void set_discard_invalid_key (bool discard) {
        m_header.set_discard_invalid_key (discard);
    }

    void set_dynamic_adjust (bool dynamic_adjust) {
        m_header.set_dynamic_adjust (dynamic_adjust);
    }

    void set_always_show_lookup (bool always) {
        m_header.set_always_show_lookup (always);
    }

    void set_use_full_width_punct (bool full) {
        m_header.set_use_full_width_punct (full);
    }

    void set_def_full_width_punct (bool full) {
        m_header.set_def_full_width_punct (full);
    }

    void set_use_full_width_letter (bool full) {
        m_header.set_use_full_width_letter (full);
    }

    void set_def_full_width_letter (bool full) {
        m_header.set_def_full_width_letter (full);
    }

    void set_icon_file (const String & icon_file) {
        m_header.set_icon_file (icon_file);
    }

    void set_languages (const String & languages) {
        m_header.set_languages (languages);
    }

    void set_status_prompt (const WideString & prompt) {
        m_header.set_status_prompt (prompt);
    }

    void set_single_wildcard_chars (const String & single) {
        m_header.set_single_wildcard_chars (single);
        m_sys_content.set_single_wildcard_chars (single);
        m_usr_content.set_single_wildcard_chars (single);
    }

    void set_multi_wildcard_chars (const String & multi) {
        m_header.set_multi_wildcard_chars (multi);
        m_sys_content.set_multi_wildcard_chars (multi);
        m_usr_content.set_multi_wildcard_chars (multi);
    }

    void set_split_keys (const std::vector <KeyEvent> & keys) {
        m_header.set_split_keys (keys);
    }

    void set_commit_keys (const std::vector <KeyEvent> & keys) {
        m_header.set_commit_keys (keys);
    }

    void set_forward_keys (const std::vector <KeyEvent> & keys) {
        m_header.set_forward_keys (keys);
    }

    void set_select_keys (const std::vector <KeyEvent> & keys) {
        m_header.set_select_keys (keys);
    }

    void set_page_up_keys (const std::vector <KeyEvent> & keys) {
        m_header.set_page_up_keys (keys);
    }

    void set_page_down_keys (const std::vector <KeyEvent> & keys) {
        m_header.set_page_down_keys (keys);
    }

    void set_max_key_length (size_t max_key_length) {
        m_header.set_max_key_length (max_key_length);
        m_sys_content.set_max_key_length (max_key_length);
        m_usr_content.set_max_key_length (max_key_length);
    }

    void set_keyboard_layout (KeyboardLayout layout) {
        m_header.set_keyboard_layout (layout);
    }
};

#endif

/*
vi:ts=4:nowrap:ai:expandtab
*/
