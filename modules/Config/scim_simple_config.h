/** @file scim_simple_config.h
 * definition of SimpleConfig class.
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
 * $Id: scim_simple_config.h,v 1.22 2005/07/06 03:57:04 suzhe Exp $
 */

#pragma once

#include <sys/time.h>
#include "scim_stl_map.h"

namespace scim {

const int SCIM_MAX_CONFIG_LINE_LENGTH = 16384;

class SimpleConfig : public ConfigBase
{
typedef scim_map <String, String> KeyValueRepository;

    KeyValueRepository       m_config;
    KeyValueRepository       m_new_config;
    std::vector <String>     m_erased_keys;
    timeval                  m_update_timestamp;
    bool                     m_need_reload;

public:
    SimpleConfig ();

    virtual ~SimpleConfig ();

    virtual bool valid () const;

    virtual String get_name () const;
    
    // String
    virtual bool read (const String& key, String *pStr) const;

    // int
    virtual bool read (const String& key, int *pl) const;

    // double
    virtual bool read (const String& key, double* val) const;

    // bool
    virtual bool read (const String& key, bool* val) const;

    //String list
    virtual bool read (const String& key, std::vector <String>* val) const;

    //int list
    virtual bool read (const String& key, std::vector <int>* val) const;

    // write the value (return true on success)
    virtual bool write (const String& key, const String& value);
    virtual bool write (const String& key, int value);
    virtual bool write (const String& key, double value);
    virtual bool write (const String& key, bool value);
    virtual bool write (const String& key, const std::vector <String>& value);
    virtual bool write (const String& key, const std::vector <int>& value);

    // permanently writes all changes
    virtual bool flush();

    // delete entries
    virtual bool erase (const String& key );

    // reload the configurations.
    virtual bool reload ();
private:
    String get_sysconf_dir ();
    String get_userconf_dir ();
    String get_sysconf_filename ();
    String get_userconf_filename ();
    String get_userstate_dir ();
    String get_userstate_filename ();

    // Keys under "/State/" are volatile state, stored in the state file
    // ($XDG_STATE_HOME) rather than the config file ($XDG_CONFIG_HOME).
    static bool is_state_key (const String &key);

    String trim_blank (const String &str);
    String get_param_portion (const String &str);
    String get_value_portion (const String &str);

    void parse_config (std::istream &is, KeyValueRepository &config);

    // Write the config keys (state_only=false) or the state keys
    // (state_only=true) from m_config to the stream.
    void save_config (std::ostream &os, bool state_only);

    bool load_all_config ();

    void remove_key_from_erased_list (const String &key);
};

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
