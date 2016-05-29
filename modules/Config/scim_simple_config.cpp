/** @file scim_simple_config.cpp
 * implementation of SimpleConfig class.
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
 * $Id: scim_simple_config.cpp,v 1.35 2005/07/06 03:57:04 suzhe Exp $
 */

#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_PATH
#define Uses_STL_IOSTREAM
#define Uses_STL_FSTREAM
#define Uses_C_STDIO

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include "scim_private.h"
#include "scim.h"
#include "scim_simple_config.h"

#ifndef SCIM_SYSCONFDIR
  #define SCIM_SYSCONFDIR "/etc"
#endif

#define scim_module_init simple_LTX_scim_module_init
#define scim_module_exit simple_LTX_scim_module_exit
#define scim_config_module_init simple_LTX_scim_config_module_init
#define scim_config_module_create_config simple_LTX_scim_config_module_create_config

using namespace scim;

extern "C" {
    void scim_module_init (void)
    {
        SCIM_DEBUG_CONFIG(1) << "Initializing Simple Config module...\n";
    }
    
    void scim_module_exit (void)
    {
        SCIM_DEBUG_CONFIG(1) << "Exiting Simple Config module...\n";
    }

    void scim_config_module_init ()
    {
        SCIM_DEBUG_CONFIG(1) << "Initializing Simple Config module (more)...\n";
    }

    ConfigPointer scim_config_module_create_config ()
    {
        SCIM_DEBUG_CONFIG(1) << "Creating a Simple Config instance...\n";
        return new SimpleConfig ();
    }
}

namespace scim {

SimpleConfig::SimpleConfig ()
    : m_need_reload (false)
{
    m_update_timestamp.tv_sec = 0;
    m_update_timestamp.tv_usec = 0;

    load_all_config ();
}

SimpleConfig::~SimpleConfig ()
{
    flush ();
}

bool
SimpleConfig::valid () const
{
    return ConfigBase::valid();
}

String
SimpleConfig::get_name () const
{
    return "simple";
}

// String
bool
SimpleConfig::read (const String& key, String *pStr) const
{
    if (!valid () || !pStr || key.empty()) return false;

    KeyValueRepository::const_iterator i = m_new_config.find (key);
    KeyValueRepository::const_iterator end = m_new_config.end ();

    if (i == end) {
        i = m_config.find (key);
        end = m_config.end ();
    }

    if (i != end) {
        *pStr = i->second;
        return true;
    }

    *pStr = String ("");
    return false;
}

// int
bool
SimpleConfig::read (const String& key, int *pl) const
{
    if (!valid () || !pl || key.empty()) return false;

    KeyValueRepository::const_iterator i = m_new_config.find (key);
    KeyValueRepository::const_iterator end = m_new_config.end ();

    if (i == end || !i->second.length ()) {
        i = m_config.find (key);
        end = m_config.end ();
    }

    if (i != end && i->second.length ()) {
        *pl = strtol (i->second.c_str (), (char**) NULL, 10);
        return true;
    }

    *pl = 0;
    return false;
}

// double
bool
SimpleConfig::read (const String& key, double* val) const
{
    if (!valid () || !val || key.empty()) return false;

    KeyValueRepository::const_iterator i = m_new_config.find (key);
    KeyValueRepository::const_iterator end = m_new_config.end ();

    if (i == end || !i->second.length ()) {
        i = m_config.find (key);
        end = m_config.end ();
    }

    if (i != end && i->second.length ()) {
        *val = strtod (i->second.c_str (), (char**) NULL);
        return true;
    }

    *val = 0;
    return false;
}

// bool
bool
SimpleConfig::read (const String& key, bool* val) const
{
    if (!valid () || !val || key.empty()) return false;
    
    KeyValueRepository::const_iterator i = m_new_config.find (key);
    KeyValueRepository::const_iterator end = m_new_config.end ();

    if (i == end || !i->second.length ()) {
        i = m_config.find (key);
        end = m_config.end ();
    }

    if (i != end && i->second.length ()) {
        if (i->second == "true" || i->second == "TRUE" || i->second == "True" ||
            i->second == "1") {
            *val = true;
            return true;
        } else if (i->second == "false" || i->second == "FALSE" || i->second == "False" ||
            i->second == "0") {
            *val = false;
            return true;
        }
    }

    *val = false;
    return false;
}

//String list
bool
SimpleConfig::read (const String& key, std::vector <String>* val) const
{
    if (!valid () || !val || key.empty()) return false;
    
    KeyValueRepository::const_iterator i = m_new_config.find (key);
    KeyValueRepository::const_iterator end = m_new_config.end ();

    if (i == end) {
        i = m_config.find (key);
        end = m_config.end ();
    }

    val->clear ();

    if (i != end) {
        scim_split_string_list (*val, i->second, ',');
        return true;
    }

    return false;
}

//int list
bool
SimpleConfig::read (const String& key, std::vector <int>* val) const
{
    if (!valid () || !val || key.empty()) return false;

    KeyValueRepository::const_iterator i = m_new_config.find (key);
    KeyValueRepository::const_iterator end = m_new_config.end ();

    if (i == end) {
        i = m_config.find (key);
        end = m_config.end ();
    }

    val->clear();

    if (i != end) {
        std::vector <String> vec;
        scim_split_string_list (vec, i->second, ',');

        for (std::vector <String>::iterator j = vec.begin (); j != vec.end (); ++j) {
            int result = strtol (j->c_str (), (char**)NULL, 10);
            val->push_back (result);
        }
        return true;
    }

    return false;
}

// write the value (return true on success)
bool
SimpleConfig::write (const String& key, const String& value)
{
    if (!valid () || key.empty()) return false;

    m_new_config [key] = value;

    remove_key_from_erased_list (key);

    m_need_reload = true;

    return true;
}

bool
SimpleConfig::write (const String& key, int value)
{
    if (!valid () || key.empty()) return false;

    char buf [256];

    snprintf (buf, 255, "%d", value);

    m_new_config [key] = String (buf);

    remove_key_from_erased_list (key);

    m_need_reload = true;

    return true;
}

bool
SimpleConfig::write (const String& key, double value)
{
    if (!valid () || key.empty()) return false;
    
    char buf [256];

    snprintf (buf, 255, "%lf", value);

    m_new_config [key] = String (buf);

    remove_key_from_erased_list (key);

    m_need_reload = true;

    return true;
}

bool
SimpleConfig::write (const String& key, bool value)
{
    if (!valid () || key.empty()) return false;

    if (value)
        m_new_config [key] = String ("true");
    else
        m_new_config [key] = String ("false");

    remove_key_from_erased_list (key);

    m_need_reload = true;

    return true;
}

bool
SimpleConfig::write (const String& key, const std::vector <String>& value)
{
    if (!valid () || key.empty()) return false;

    m_new_config [key] = scim_combine_string_list (value, ',');

    remove_key_from_erased_list (key);

    m_need_reload = true;

    return true;
}

bool
SimpleConfig::write (const String& key, const std::vector <int>& value)
{
    if (!valid () || key.empty()) return false;

    std::vector <String> vec;
    char buf [256];

    for (std::vector <int>::const_iterator i = value.begin (); i != value.end (); ++i) {
        snprintf (buf, 255, "%d", *i);
        vec.push_back (String (buf));
    }

    m_new_config [key] = scim_combine_string_list (vec, ',');

    remove_key_from_erased_list (key);

    m_need_reload = true;

    return true;
}

// permanently writes all changes
bool
SimpleConfig::flush()
{
    if (!valid ()) return false;

    // If no config has been modified, then just return.
    if (!m_new_config.size () && !m_erased_keys.size ())
        return true;

    String userconf     = get_userconf_filename ();
    String userconf_dir = get_userconf_dir ();

    if (access (userconf_dir.c_str (), R_OK | W_OK) != 0) {
        mkdir (userconf_dir.c_str (), S_IRUSR | S_IWUSR | S_IXUSR);
        if (access (userconf_dir.c_str (), R_OK | W_OK) != 0)
            return false;
    }

    if (userconf.length ()) {
        // Reload config to ensure user made modification won't lost.
        load_all_config ();

        KeyValueRepository::iterator i;
        std::vector<String>::iterator j;

        // Merge new config with old ones.
        for (i = m_new_config.begin (); i != m_new_config.end (); ++i)
            m_config [i->first] = i->second;

        // Remove all erased keys.
        for (j = m_erased_keys.begin (); j != m_erased_keys.end (); ++j) {
            if ((i = m_config.find (*j)) != m_config.end ())
                m_config.erase (i);
        }

        m_new_config.clear ();
        m_erased_keys.clear ();

        gettimeofday (&m_update_timestamp, 0);

        char buf [128];
        snprintf (buf, 128, "%lu:%lu", m_update_timestamp.tv_sec, m_update_timestamp.tv_usec);

        m_config [String (SCIM_CONFIG_UPDATE_TIMESTAMP)] = String (buf);

        std::ofstream os (userconf.c_str ());
        if (!os) return false;
        save_config (os);
        return true;
    }

    return false;
}

// delete entries
bool
SimpleConfig::erase (const String& key)
{
    if (!valid ()) return false;

    KeyValueRepository::iterator i = m_new_config.find(key);
    KeyValueRepository::iterator j = m_config.find(key);
    bool ok = false;

    if (i != m_new_config.end ()) {
        m_new_config.erase (i);
        ok = true;
    }

    if (j != m_config.end ()) {
        m_config.erase (j);
        ok = true;
    }

    if (ok && std::find (m_erased_keys.begin (), m_erased_keys.end (), key) == m_erased_keys.end ())
        m_erased_keys.push_back (key);

    m_need_reload = true;

    return ok;
}

bool
SimpleConfig::reload ()
{
    if (!valid ()) return false;

    if (load_all_config ()) {
        m_new_config.clear ();
        m_erased_keys.clear ();
        m_need_reload = true;
    }

    if (m_need_reload) {
        m_need_reload = false;
        return ConfigBase::reload ();
    }

    return false;
}

String
SimpleConfig::get_sysconf_dir ()
{
    return String (SCIM_SYSCONFDIR) +
           String (SCIM_PATH_DELIM_STRING) +
           String ("scim");
}

String
SimpleConfig::get_userconf_dir ()
{
    return scim_get_user_data_dir ();
}

String
SimpleConfig::get_sysconf_filename ()
{
    return get_sysconf_dir () +
           String (SCIM_PATH_DELIM_STRING) +
           String ("config");
}

String
SimpleConfig::get_userconf_filename ()
{
    return get_userconf_dir () +
           String (SCIM_PATH_DELIM_STRING) +
           String ("config");
}

String
SimpleConfig::trim_blank (const String &str)
{
    String::size_type begin, len;

    begin = str.find_first_not_of (" \t\n\v");

    if (begin == String::npos)
        return String ();

    len = str.find_last_not_of (" \t\n\v") - begin + 1;

    return str.substr (begin, len);
}

String
SimpleConfig::get_param_portion (const String &str)
{
    String::size_type begin = str.find_first_of (" \t\n\v=");

    if (begin == String::npos) return str;

    return str.substr (0, begin);
}

String
SimpleConfig::get_value_portion (const String &str)
{
    String::size_type begin = str.find_first_of ("=");

    if (begin == String::npos || (begin + 1) == str.length ()) return String ("");

    return trim_blank (str.substr (begin + 1, String::npos));
}

void
SimpleConfig::parse_config (std::istream &is, KeyValueRepository &config)
{
    char *conf_line = new char [SCIM_MAX_CONFIG_LINE_LENGTH];

    while (!is.eof()) {
        is.getline(conf_line, SCIM_MAX_CONFIG_LINE_LENGTH);
        if (!is.eof()) {
            String normalized_line = trim_blank(conf_line);

            if ((normalized_line.find_first_of("#") > 0) && (normalized_line.length() != 0)) {
                if (normalized_line.find_first_of("=") == String::npos) {
                    SCIM_DEBUG_CONFIG(2) << " Invalid config line : " << normalized_line << "\n";
                    continue;
                }

                if (normalized_line[0] == '=') {
                    SCIM_DEBUG_CONFIG(2) << " Invalid config line : " << normalized_line << "\n";
                    continue;
                }

                String param = get_param_portion(normalized_line);
                KeyValueRepository::iterator i = config.find(param);

                if (i != config.end()) {
                    SCIM_DEBUG_CONFIG(2) << " Config entry " << normalized_line << " has been read.\n";
                } else {
                    String value = get_value_portion (normalized_line); 
                    config [param] = value;
                    SCIM_DEBUG_CONFIG(2) << " Config entry " << param << "=" << value << " is successfully read.\n";
                }
            }
        }
    }

    delete [] conf_line;
}

void
SimpleConfig::save_config (std::ostream &os)
{
    KeyValueRepository::iterator i;
    for (i = m_config.begin (); i != m_config.end (); ++i) {
        os << i->first << " = " << i->second << "\n";
    }
}

bool
SimpleConfig::load_all_config ()
{
    String sysconf = get_sysconf_filename ();
    String userconf = get_userconf_filename ();

    KeyValueRepository config;

    if (userconf.length ()) {
        std::ifstream is (userconf.c_str ());
        if (is) {
            SCIM_DEBUG_CONFIG(1) << "Parsing user config file: "
                                 << userconf << "\n";
            parse_config (is, config);
        }
    }

    if (sysconf.length ()) {
        std::ifstream is (sysconf.c_str ());
        if (is) {
            SCIM_DEBUG_CONFIG(1) << "Parsing system config file: "
                                 << sysconf << "\n";
            parse_config (is, config);
        }
    }

    if (!m_config.size () || (m_update_timestamp.tv_sec == 0 && m_update_timestamp.tv_usec == 0)) {
        m_config.swap (config);
        gettimeofday (&m_update_timestamp, 0);
        return true;
    }

    KeyValueRepository::iterator it = config.find (String (SCIM_CONFIG_UPDATE_TIMESTAMP));

    if (it != config.end ()) {
        std::vector <String> strs;
        if (scim_split_string_list (strs, it->second, ':') == 2) {
            time_t sec = (time_t) strtol (strs [0].c_str (), 0, 10);
            suseconds_t usec = (suseconds_t) strtol (strs [1].c_str (), 0, 10);

            // The config file is newer, so load it.
            if (m_update_timestamp.tv_sec < sec || (m_update_timestamp.tv_sec == sec && m_update_timestamp.tv_usec < usec)) {
                m_config.swap (config);
                m_update_timestamp.tv_sec = (time_t) sec;
                m_update_timestamp.tv_usec = (suseconds_t) usec;
                return true;
            }
        }
    }
    return false;
}

void
SimpleConfig::remove_key_from_erased_list (const String &key)
{
    std::vector <String>::iterator it = std::find (m_erased_keys.begin (), m_erased_keys.end (), key);

    if (it != m_erased_keys.end ())
        m_erased_keys.erase (it);
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
