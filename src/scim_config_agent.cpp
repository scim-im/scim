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
 * $Id: scim_config_agent.cpp,v 1.10 2005/07/03 08:36:42 suzhe Exp $
 *
 */

#define Uses_STL_IOSTREAM
#define Uses_C_LOCALE
#define Uses_SCIM_CONFIG_MODULE
#define Uses_SCIM_HELPER
#define Uses_SCIM_CONFIG_PATH

#include "scim_private.h"
#include <scim.h>
#include <stdlib.h>

using namespace scim;
using std::cout;
using std::cerr;
using std::endl;

enum DataType
{
    DATA_TYPE_STRING,
    DATA_TYPE_INT,
    DATA_TYPE_DOUBLE,
    DATA_TYPE_BOOL,
    DATA_TYPE_STRING_LIST,
    DATA_TYPE_INT_LIST
};

enum Command
{
    DO_NOTHING,
    GET_DATA,
    SET_DATA,
    DEL_KEY
};

static String trim_blank (const String &str)
{
    String::size_type begin, len;

    begin = str.find_first_not_of (" \t\n\v");

    if (begin == String::npos)
        return String ();

    len = str.find_last_not_of (" \t\n\v") - begin + 1;

    return str.substr (begin, len);
}

static String get_param_portion (const String &str)
{
    String::size_type begin = str.find_first_of (" \t\n\v=");

    if (begin == String::npos) return str;

    return str.substr (0, begin);
}

static String get_value_portion (const String &str)
{
    String::size_type begin = str.find_first_of ("=");

    if (begin == String::npos || (begin + 1) == str.length ()) return String ("");

    return trim_blank (str.substr (begin + 1, String::npos));
}

int main (int argc, char *argv [])
{
    static ConfigModule  config_module;

    ConfigPointer        config;
    std::vector<String>  config_list;
    String               def_config;
    String               key;
    String               value;
    String               display;

    DataType             type = DATA_TYPE_STRING;
    Command              cmd = DO_NOTHING;
    bool                 reload = false;
    bool                 global = false;

    int                  i;

    char *p =  getenv ("DISPLAY");
    if (p) display = String (p);

    //get modules list
    scim_get_config_module_list (config_list);

    //Use simple Config module as default if available.
    if (config_list.size ()) {
        def_config = scim_global_config_read (SCIM_GLOBAL_CONFIG_DEFAULT_CONFIG_MODULE, String ("simple"));
        if (std::find (config_list.begin (),
                       config_list.end (),
                       def_config) == config_list.end ())
            def_config = config_list [0];
    } else {
        cerr << "No config module found.\n";
        return -1;
    }

    // parse command options
    i = 0;
    while (i<argc) {
        if (++i >= argc) break;

        if (String ("-h") == argv [i] ||
            String ("--help") == argv [i]) {
            cout << "Usage: " << argv [0] << " <option>...\n\n"
                 << "The options are:\n"
                 << "  -g, --get key        Get the value of this key.\n"
                 << "  -s, --set key=value  Set the value of this key.\n"
                 << "  -d, --del key        Delete the key and its data\n"
                 << "                       from user config file.\n"
                 << "  -t, --type type      The key value type, valid types are:\n"
                 << "                       string, int, double, bool, string-list,\n"
                 << "                       int-list. The default type is string.\n"
                 << "  -c, --config name    Use specified Config module,\n"
                 << "                       use simple module by default.\n"
                 << "                       Use \"global\" instead of a real config module name,\n"
                 << "                       if you want to access the global configuration file.\n"
                 << "                       (Normally they are /etc/scim/global and ~/.scim/global).\n"
                 << "  --reload             Force the running scim to reload configuration.\n"
                 << "  --display display    The display which scim Panel is running on,\n"
                 << "                       it's only useful when --reload is used.\n"
                 << "  -h, --help           Show this help.\n";
            return 0;
        }

        if (String ("-g") == argv [i] ||
            String ("--get") == argv [i]) {
            if (++i >= argc) {
                cerr << "No argument for option " << argv [i-1] << endl;
                return -1;
            }
            if (cmd != DO_NOTHING) {
                cerr << "You can only do one thing at the same time!\n";
                return -1;
            }
            key = String (argv [i]);
            cmd = GET_DATA;
            continue;
        }

        if (String ("-s") == argv [i] ||
            String ("--set") == argv [i]) {
            if (++i >= argc) {
                cerr << "No argument for option " << argv [i-1] << endl;
                return -1;
            }
            if (cmd != DO_NOTHING) {
                cerr << "You can only do one thing at the same time!\n";
                return -1;
            }

            String str (argv [i]);
            str = trim_blank (str);

            key = get_param_portion (str);
            value = get_value_portion (str);

            if (!key.length ()) {
                cerr << "Bad argument for option " << argv [i-1] << endl;
                return -1;
            }

            cmd = SET_DATA;
            continue;
        }

        if (String ("-d") == argv [i] ||
            String ("--del") == argv [i]) {
            if (++i >= argc) {
                cerr << "No argument for option " << argv [i-1] << endl;
                return -1;
            }
            if (cmd != DO_NOTHING) {
                cerr << "You can only do one thing at the same time!\n";
                return -1;
            }

            key = String (argv [i]);
            cmd = DEL_KEY;
            continue;
        }

        if (String ("-t") == argv [i] ||
            String ("--type") == argv [i]) {
            if (++i >= argc) {
                cerr << "No argument for option " << argv [i-1] << endl;
                return -1;
            }
            if (String (argv [i]) == "string")
                type = DATA_TYPE_STRING;
            else if (String (argv [i]) == "int")
                type = DATA_TYPE_INT;
            else if (String (argv [i]) == "double")
                type = DATA_TYPE_DOUBLE;
            else if (String (argv [i]) == "bool")
                type = DATA_TYPE_BOOL;
            else if (String (argv [i]) == "string-list")
                type = DATA_TYPE_STRING_LIST;
            else if (String (argv [i]) == "int-list")
                type = DATA_TYPE_STRING_LIST;
            else {
                cerr << "Bad argument for option " << argv [i-1] << endl;
                return -1;
            }
            continue;
        }

        if (String ("-c") == argv [i] ||
            String ("--config") == argv [i]) {
            if (++i >= argc) {
                cerr << "No argument for option " << argv [i-1] << endl;
                return -1;
            }
            def_config = String (argv [i]);
            continue;
        }

        if (String ("--display") == argv [i]) {
            if (++i >= argc) {
                cerr << "No argument for option " << argv [i-1] << endl;
                return -1;
            }
            display = String (argv [i]);
            continue;
        }

        if (String ("--reload") == argv [i]) {
            reload = true;
            continue;
        }

        cerr << "Unknown option " << argv [i] << endl;
        return -1;
    }

    if ((cmd == DO_NOTHING || !key.length ()) && reload == false) {
        cerr << "What do you want to do?\n";
        return -1;
    }

    if (cmd != DO_NOTHING) {
        if (def_config == "global") {
            global = true;
        } else {
            if (!config_module.load (def_config)) {
                cerr << "Failed to load config module " << def_config << endl;
                return -1;
            }
 
            config = config_module.create_config ();
 
            if (config.null ()) {
                cerr << "Failed to create config object.\n";
                return -1;
            }
        }
    }

    // Get data
    if (cmd == GET_DATA) {
        bool ok = false;
        if (type == DATA_TYPE_STRING) {
            if (global) {
                value = scim_global_config_read (key, String (""));
                ok = (scim_global_config_read (key, String ("Invalid")) == value);
            } else {
                ok = config->read (key, &value);
            }
            if (ok) cout << value << endl;
        } else if (type == DATA_TYPE_INT) {
            int intval;
            if (global) {
                intval = scim_global_config_read (key, (int) 0); 
                ok = (scim_global_config_read (key, (int) 0) == intval); 
            } else {
                ok = config->read (key, &intval);
            }
            if (ok) cout << intval << endl;
        } else if (type == DATA_TYPE_DOUBLE) {
            double doubleval;
            if (global) {
                doubleval = scim_global_config_read (key, (double) 0);
                ok = (scim_global_config_read (key, (double) 1) == doubleval);
            } else {
                ok = config->read (key, &doubleval);
            }
            if (ok) cout << doubleval << endl;
        } else if (type == DATA_TYPE_BOOL) {
            bool boolval;
            if (global) {
                boolval = scim_global_config_read (key, (bool) false);
                ok = (scim_global_config_read (key, (bool) true) == boolval);
            } else {
                ok = config->read (key, &boolval);
            }
            if (ok) cout << (boolval ? "true" : "false") << endl;
        } else if (type == DATA_TYPE_STRING_LIST) {
            std::vector <String> strlistval;
            if (global) {
                strlistval = scim_global_config_read (key, strlistval);
                ok = (strlistval.size () > 0);
            } else {
                ok = config->read (key, &strlistval);
            }
            if (ok) cout << scim_combine_string_list (strlistval, ',') << endl;
        } else if (type == DATA_TYPE_INT_LIST) {
            std::vector <int> intlistval;
            if (global) {
                intlistval = scim_global_config_read (key, intlistval);
                ok = (intlistval.size () > 0);
            } else {
                ok = config->read (key, &intlistval);
            }

            if (ok) {
                for (size_t i = 0; i<intlistval.size (); ++i) {
                    cout << intlistval [i];
                    if (i < intlistval.size () - 1)
                        cout << ",";
                }
                cout << endl;
            }
        }

        if (!ok) {
            cerr << "Failed to get key value.\n";
            return -1;
        }
    }
 
    // Set data
    else if (cmd == SET_DATA) {
        bool ok = true;
        if (type != DATA_TYPE_STRING && !value.length ()) {
            ok = false;
        } else if (type == DATA_TYPE_STRING) {
            if (global) {
                scim_global_config_write (key, value);
            } else {
                ok = config->write (key, value);
            }
        } else if (type == DATA_TYPE_INT) {
            int intval = strtol (value.c_str (), 0, 10);
            if (global) {
                scim_global_config_write (key, intval);
            } else {
                ok = config->write (key, intval);
            }
        } else if (type == DATA_TYPE_DOUBLE) {
            double doubleval = strtod (value.c_str (), 0);
            if (global) {
                scim_global_config_write (key, doubleval);
            } else {
                ok = config->write (key, doubleval);
            }
        } else if (type == DATA_TYPE_BOOL) {
            bool boolval = false;
            if (value == "true" || value == "True" || value == "TRUE" || value == "1")
                boolval = true;

            if (global) {
                scim_global_config_write (key, boolval);
            } else {
                ok = config->write (key, boolval);
            }
        } else if (type == DATA_TYPE_STRING_LIST) {
            std::vector <String> strlistval;
            scim_split_string_list (strlistval, value, ',');

            if (global) {
                scim_global_config_write (key, strlistval);
            } else {
                ok = config->write (key, strlistval);
            }
        } else if (type == DATA_TYPE_INT_LIST) {
            std::vector <int> intlistval;
            std::vector <String> strlist;
            scim_split_string_list (strlist, value, ',');
            for (size_t i = 0; i<strlist.size (); ++i)
                intlistval.push_back (strtol (strlist[i].c_str (), 0, 10));

            if (global) {
                scim_global_config_write (key, intlistval);
            } else {
                ok = config->write (key, intlistval);
            }
        }

        if (global)
            ok = scim_global_config_flush ();

        if (!ok) {
            cerr << "Failed to set key value.\n";
            return -1;
        } else {
            cout << "Set data success.\n";
            if (!global) config->flush ();
        }
    }

    // Delete key
    else if (cmd == DEL_KEY) {
        bool ok = false;

        if (global) {
            scim_global_config_reset (key); 
            ok = scim_global_config_flush ();
        } else {
            ok = config->erase (key);
        }

        if (ok) {
            cout << "Delete key success.\n";
            if (!global) config->flush ();
        } else {
            cerr << "Failed to delete the key.\n";
            return -1;
        }
    }

    if (reload) {
        HelperInfo   helper_info ("41b79480-c5d2-4929-9456-11d519c26b87", "scim-config-agent", "", "", SCIM_HELPER_STAND_ALONE);
        HelperAgent  helper_agent;

        int          id;

        id = helper_agent.open_connection (helper_info, display);

        if (id < 0) {
            cerr << "Unable to open the connection to scim Panel.\n";
            return -1;
        }

        helper_agent.reload_config ();

        cout << "Configuration reload request has been sent to the running scim.\n";

        helper_agent.close_connection ();
    }

    return 0;
}

/*
vi:ts=4:ai:nowrap:expandtab
*/
