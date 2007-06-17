/*
 * Smart Common Input Method
 * 
 * Copyright (c) 2004-2005 James Su <suzhe@tsinghua.org.cn>
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
 * $Id: scim_global_config.cpp,v 1.7 2005/09/29 10:51:25 suzhe Exp $
 */

#define Uses_SCIM_UTILITY
#define Uses_SCIM_GLOBAL_CONFIG
#define Uses_C_STDLIB
#define Uses_C_STDIO
#define Uses_C_STRING
#define Uses_STL_FSTREAM

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "scim_private.h"
#include "scim.h"
#include "scim_stl_map.h"

namespace scim {

#if SCIM_USE_STL_EXT_HASH_MAP
typedef __gnu_cxx::hash_map <String, String, scim_hash_string> KeyValueRepository;
#elif SCIM_USE_STL_HASH_MAP
typedef std::hash_map <String, String, scim_hash_string> KeyValueRepository;
#else
typedef std::map <String, String> KeyValueRepository;
#endif

class GlobalConfigRepository
{
public:
    KeyValueRepository         sys;
    KeyValueRepository         usr;
    KeyValueRepository         updated;

    bool                       initialized;

public:
    GlobalConfigRepository () : initialized (false) { }
    ~GlobalConfigRepository () { scim_global_config_flush (); }

};

static GlobalConfigRepository __config_repository;

static String
__trim_blank (const String &str)
{
    String::size_type begin, len;

    begin = str.find_first_not_of (" \t\n\v");

    if (begin == String::npos)
        return String ();

    len = str.find_last_not_of (" \t\n\v") - begin + 1;

    return str.substr (begin, len);
}

static String
__get_param_portion (const String &str)
{
    String ret = str;
    return (ret.erase (ret.find_first_of (" \t\n\v="), ret.length() - 1));
}

static String
__get_value_portion (const String &str)
{
    String ret = str;
    ret.erase (0, ret.find_first_of ("=") + 1);
    ret.erase (0, ret.find_first_not_of(" \n\t\v"));
    return (ret.erase (ret.find_last_not_of(" \t\n\v") + 1));
}

static void
__parse_config (std::ifstream &is, KeyValueRepository &repository)
{
    char *conf_line = new char [10000];

    while (!is.eof ()) {
        is.getline (conf_line, 10000);
        String normalized_line = __trim_blank(conf_line);
        if ((normalized_line.find_first_of("#") > 0) && (normalized_line.length() != 0)) {
            if (normalized_line.find_first_of("=") == String::npos) {
                SCIM_DEBUG_MAIN (2) << " Invalid global config line : " << normalized_line << "\n";
                continue;
            }

            if (normalized_line[0] == '=') {
                SCIM_DEBUG_MAIN (2) << " Invalid global config line : " << normalized_line << "\n";
                continue;
            }

            String param = __get_param_portion(normalized_line);
            String value = __get_value_portion (normalized_line); 
            repository [param] = value;

            SCIM_DEBUG_MAIN (2) << " Global config entry " << param << "=" << value << " is successfully read.\n";
        }
    }

    delete [] conf_line;
}

static void
__initialize_config ()
{
    __config_repository.sys.clear ();
    __config_repository.usr.clear ();

    String sys_conf_file = String (SCIM_SYSCONFDIR) +
                           String (SCIM_PATH_DELIM_STRING) +
                           String ("scim") +
                           String (SCIM_PATH_DELIM_STRING) +
                           String ("global");

    String usr_conf_file = scim_get_home_dir () +
                           String (SCIM_PATH_DELIM_STRING) +
                           String (".scim") +
                           String (SCIM_PATH_DELIM_STRING) +
                           String ("global");

    std::ifstream sys_is (sys_conf_file.c_str ());
    std::ifstream usr_is (usr_conf_file.c_str ());

    if (sys_is) {
        __parse_config (sys_is, __config_repository.sys);
        __config_repository.initialized = true;
    }

    if (usr_is) {
        __parse_config (usr_is, __config_repository.usr);
        __config_repository.initialized = true;
    }
}

String
scim_global_config_read (const String &key, const String &defVal)
{
    if (!__config_repository.initialized) __initialize_config ();

    if (__config_repository.initialized) {
        KeyValueRepository::iterator it = __config_repository.usr.find (key);

        if (it == __config_repository.usr.end ()) {
            it = __config_repository.sys.find (key);
            if (it == __config_repository.sys.end ())
                return defVal;
        }

        return it->second;
    }

    return defVal;
}

int
scim_global_config_read (const String &key, int defVal)
{
    if (!__config_repository.initialized) __initialize_config ();

    if (__config_repository.initialized) {
        KeyValueRepository::iterator it = __config_repository.usr.find (key);

        if (it == __config_repository.usr.end ()) {
            it = __config_repository.sys.find (key);
            if (it == __config_repository.sys.end ())
                return defVal;
        }

        if (it->second.length ())
            return strtol (it->second.c_str (), (char **) NULL, 10);
    }

    return defVal;
}

bool
scim_global_config_read (const String &key, bool defVal)
{
    if (!__config_repository.initialized) __initialize_config ();

    if (__config_repository.initialized) {
        KeyValueRepository::iterator it = __config_repository.usr.find (key);

        if (it == __config_repository.usr.end ()) {
            it = __config_repository.sys.find (key);
            if (it == __config_repository.sys.end ())
                return defVal;
        }

        if (it->second.length ()) {
            if (it->second == "true" || it->second == "TRUE" || it->second == "True" || it->second == "1")
                return true;
            else if (it->second == "false" || it->second == "FALSE" || it->second == "False" || it->second == "0")
                return false;
        }
    }

    return defVal;
}

double
scim_global_config_read (const String &key, double defVal)
{
    if (!__config_repository.initialized) __initialize_config ();

    if (__config_repository.initialized) {
        KeyValueRepository::iterator it = __config_repository.usr.find (key);

        if (it == __config_repository.usr.end ()) {
            it = __config_repository.sys.find (key);
            if (it == __config_repository.sys.end ())
                return defVal;
        }

        if (it->second.length ())
            return strtod (it->second.c_str (), (char **) NULL);
    }

    return defVal;
}

std::vector <String>
scim_global_config_read (const String &key, const std::vector <String> &defVal)
{
    if (!__config_repository.initialized) __initialize_config ();

    if (__config_repository.initialized) {
        KeyValueRepository::iterator it = __config_repository.usr.find (key);

        if (it == __config_repository.usr.end ()) {
            it = __config_repository.sys.find (key);
            if (it == __config_repository.sys.end ())
                return defVal;
        }

        if (it->second.length ()) {
            std::vector <String> strs;
            scim_split_string_list (strs, it->second, ',');
            return strs;
        }
    }

    return defVal;
}

std::vector <int>
scim_global_config_read (const String &key, const std::vector <int> &defVal)
{
    if (!__config_repository.initialized) __initialize_config ();

    if (__config_repository.initialized) {
        KeyValueRepository::iterator it = __config_repository.usr.find (key);

        if (it == __config_repository.usr.end ()) {
            it = __config_repository.sys.find (key);
            if (it == __config_repository.sys.end ())
                return defVal;
        }

        if (it->second.length ()) {
            std::vector <String> strs;
            std::vector <int> ints;

            scim_split_string_list (strs, it->second, ',');

            for (std::vector <String>::iterator i = strs.begin (); i != strs.end (); ++i)
                ints.push_back (strtol (i->c_str (), (char **)NULL, 10));

            return ints;
        }
    }

    return defVal;
}

void
scim_global_config_write (const String &key, const String &val)
{
    if (!__config_repository.initialized) __initialize_config ();

    if (__config_repository.initialized && key.length ()) {
        __config_repository.usr [key] = val;
        __config_repository.updated [key] = "updated";
    }
}

void
scim_global_config_write (const String &key, int val)
{
    if (!__config_repository.initialized) __initialize_config ();

    if (__config_repository.initialized && key.length ()) {
        char buf [80];
        snprintf (buf, 80, "%d", val);
        __config_repository.usr [key] = String (buf);
        __config_repository.updated [key] = "updated";
    }
}

void
scim_global_config_write (const String &key, bool val)
{
    if (!__config_repository.initialized) __initialize_config ();

    if (__config_repository.initialized && key.length ()) {
        __config_repository.usr [key] = (val ? "true" : "false");
        __config_repository.updated [key] = "updated";
    }
}

void
scim_global_config_write (const String &key, double val)
{
    if (!__config_repository.initialized) __initialize_config ();

    if (__config_repository.initialized && key.length ()) {
        char buf [80];
        snprintf (buf, 80, "%lf", val);
        __config_repository.usr [key] = String (buf);
        __config_repository.updated [key] = "updated";
    }
}

void
scim_global_config_write (const String &key, const std::vector <String> &val)
{
    if (!__config_repository.initialized) __initialize_config ();

    if (__config_repository.initialized && key.length ()) {
        __config_repository.usr [key] = scim_combine_string_list (val, ',');
        __config_repository.updated [key] = "updated";
    }
}

void
scim_global_config_write (const String &key, const std::vector <int> &val)
{
    if (!__config_repository.initialized) __initialize_config ();

    if (__config_repository.initialized && key.length ()) {
        std::vector <String> strvec;
        char buf [80];
        for (size_t i = 0; i < val.size (); ++i) {
            snprintf (buf, 80, "%d", val [i]);
            strvec.push_back (buf);
        }
        __config_repository.usr [key] = scim_combine_string_list (strvec, ',');
        __config_repository.updated [key] = "updated";
    }
}

void
scim_global_config_reset (const String &key)
{
    if (!__config_repository.initialized) __initialize_config ();

    if (__config_repository.initialized && key.length ()) {
        __config_repository.usr.erase (key);
        __config_repository.updated [key] = "erased";
    }
}

bool
scim_global_config_flush ()
{
    if (!__config_repository.initialized)
        return false;

    if (!__config_repository.updated.size ())
        return true;

    String usr_conf_dir  = scim_get_home_dir () + 
                           String (SCIM_PATH_DELIM_STRING) +
                           String (".scim");

    String usr_conf_file = usr_conf_dir +
                           String (SCIM_PATH_DELIM_STRING) +
                           String ("global");

    if (access (usr_conf_dir.c_str (), R_OK | W_OK) != 0) {
        mkdir (usr_conf_dir.c_str (), S_IRUSR | S_IWUSR | S_IXUSR);
        if (access (usr_conf_dir.c_str (), R_OK | W_OK) != 0)
            return false;
    }

    KeyValueRepository backup_usr = __config_repository.usr;

    // Reload all configuration.
    __initialize_config ();

    for (KeyValueRepository::iterator it = __config_repository.updated.begin ();
         it != __config_repository.updated.end (); ++it) {
         if (it->second == "updated")
             __config_repository.usr [it->first] = backup_usr [it->first];
         else if (it->second == "erased")
             __config_repository.usr.erase (it->first);
    }

    std::ofstream usr_os (usr_conf_file.c_str ());

    if (usr_os) {
        for (KeyValueRepository::iterator it = __config_repository.usr.begin ();
             it != __config_repository.usr.end (); ++it) {
            usr_os << it->first << " = " << it->second << "\n";
        }
        __config_repository.updated.clear ();
        return true;
    }

    return false;
}

} // namespace scim

/*
vi:ts=4:ai:nowrap:expandtab
*/
