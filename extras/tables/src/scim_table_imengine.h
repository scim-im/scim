/** @file scim_table_imengine.h
 * definition of Table related classes.
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
 * $Id: scim_table_imengine.h,v 1.3 2005/10/26 07:53:53 suzhe Exp $
 */

#if !defined (__SCIM_TABLE_IMENGINE_H)
#define __SCIM_TABLE_IMENGINE_H
#include "scim_generic_table.h"

/* phrase frequency cannot larger than this value (2^16 - 1) */
#define SCIM_GT_MAX_PHRASE_FREQ            0xFFFF

/* when a phrase is input,
 * increase the freq by ((max_freq - cur_freq) >> delta)
 */
#define SCIM_GT_PHRASE_FREQ_DELTA_SHIFT    10

using namespace scim;

class TableFactory : public IMEngineFactoryBase
{
    GenericTableLibrary   m_table;

    ConfigPointer         m_config;

    std::vector<KeyEvent> m_full_width_punct_keys;
    std::vector<KeyEvent> m_full_width_letter_keys;
    std::vector<KeyEvent> m_mode_switch_keys;
    std::vector<KeyEvent> m_add_phrase_keys;
    std::vector<KeyEvent> m_del_phrase_keys;

    String                m_table_filename;

    bool                  m_is_user_table;

    bool                  m_show_prompt;

    bool                  m_show_key_hint;

    bool                  m_user_table_binary;

    bool                  m_user_phrase_first;

    bool                  m_long_phrase_first;

    time_t                m_last_time;

    Connection            m_reload_signal_connection;

    Property              m_status_property;
    Property              m_letter_property;
    Property              m_punct_property;

    friend class TableInstance;

public:
    TableFactory (const ConfigPointer &config);

    virtual ~TableFactory ();

    virtual WideString  get_name () const;
    virtual WideString  get_authors () const;
    virtual WideString  get_credits () const;
    virtual WideString  get_help () const;
    virtual String      get_uuid () const;
    virtual String      get_icon_file () const;

    virtual IMEngineInstancePointer create_instance (const String& encoding, int id = -1);

    bool load_table (const String &table_file, bool user_table=false);

    bool valid () const {
        return m_table.valid ();
    }

private:
    void init (const ConfigPointer &config);

    void save ();

    String get_sys_table_freq_file ();
    String get_sys_table_user_file ();

    void refresh (bool rightnow = false);
};

class TableInstance : public IMEngineInstanceBase
{
    Pointer <TableFactory>  m_factory;

    bool m_double_quotation_state;
    bool m_single_quotation_state;

    bool m_full_width_punct [2];
    bool m_full_width_letter [2];

    bool m_forward;
    bool m_focused;

    std::vector<String>     m_inputted_keys;
    std::vector<WideString> m_converted_strings;
    std::vector<uint32>     m_converted_indexes;

    CommonLookupTable       m_lookup_table;
    std::vector <uint32>    m_lookup_table_indexes;

    uint32                  m_inputing_caret;
    uint32                  m_inputing_key;

    IConvert                m_iconv;

    KeyEvent                m_prev_key;

    // 0 : Normal input mode
    // 1 : Add phrase mode
    // 2 : Add phrase success
    // 3 : Add phrase failed
    int                     m_add_phrase_mode;

    WideString              m_last_committed;

public:
    TableInstance (TableFactory *factory,
                         const String& encoding,
                         int id = -1);
    virtual ~TableInstance ();

    virtual bool process_key_event (const KeyEvent& key);
    virtual void move_preedit_caret (unsigned int pos);
    virtual void select_candidate (unsigned int item);
    virtual void update_lookup_table_page_size (unsigned int page_size);
    virtual void lookup_table_page_up ();
    virtual void lookup_table_page_down ();
    virtual void reset ();
    virtual void focus_in ();
    virtual void focus_out ();
    virtual void trigger_property (const String &property);

private:
    bool caret_left ();
    bool caret_right ();
    bool caret_home ();
    bool caret_end ();
    bool insert (char key);
    bool test_insert (char key);
    bool erase (bool backspace = true);
    bool space_hit ();
    bool enter_hit ();
    bool lookup_cursor_up ();
    bool lookup_cursor_down ();
    bool lookup_cursor_up_to_longer ();
    bool lookup_cursor_down_to_shorter ();
    bool lookup_page_up ();
    bool lookup_page_down ();
    bool lookup_select (int index);
    bool delete_phrase ();
    bool post_process (char key);
    void lookup_to_converted (int index);
    void commit_converted ();

    void refresh_preedit ();
    /**
     * @param show if show the lookup table
     * @param refresh if refresh the content of lookup table.
     */
    void refresh_lookup_table (bool show=true, bool refresh=true);
    void refresh_aux_string ();

    void initialize_properties ();

    void refresh_status_property ();
    void refresh_letter_property ();
    void refresh_punct_property ();

    bool match_key_event (const std::vector<KeyEvent>& keyvec, const KeyEvent& key);
};

#endif
/*
vi:ts=4:nowrap:ai:expandtab
*/

