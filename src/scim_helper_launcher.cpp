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
 * $Id: scim_helper_launcher.cpp,v 1.6 2005/05/16 01:25:46 suzhe Exp $
 *
 */

#define Uses_SCIM_HELPER_MODULE
#define Uses_SCIM_GLOBAL_CONFIG
#define Uses_SCIM_CONFIG_PATH
#include <stdlib.h>
#include "scim_private.h"
#include "scim.h"

using namespace scim;

int main (int argc, char *argv [])
{
    int i = 0;
    int j = 0;
    String config;
    String display;
    String helper;
    String uuid;
    bool   daemon = false;

    char *p =  getenv ("DISPLAY");
    if (p) display = String (p);

    config = scim_global_config_read (SCIM_GLOBAL_CONFIG_DEFAULT_CONFIG_MODULE, String ("simple"));

    while (i < argc) {
        if (++i >= argc) break;

        if (String ("-c") == argv [i] ||
            String ("--config") == argv [i]) {
            if (++i >= argc) {
                std::cerr << "No argument for option " << argv [i-1] << "\n";
                exit (-1);
            }
            config = argv [i];
            continue;
        }

        if (String ("-d") == argv [i] ||
            String ("--daemon") == argv [i]) {
            daemon = true;
            continue;
        }

        if (String ("--display") == argv [i]) {
            if (++i >= argc) {
                std::cerr << "No argument for option " << argv [i-1] << "\n";
                exit (-1);
            }
            display = argv [i];
            continue;
        }

        if (String ("-h") == argv [i] ||
            String ("--help") == argv [i]) {
            std::cout << "Usage: " << argv [0] << " [options] module uuid\n\n"
                      << "The options are:\n"
                      << "  -c, --config name    Use specified config module, default is \"simple\".\n"
                      << "  -d, --daemon         Run as daemon.\n"
                      << "  --display name       run setup on a specified DISPLAY.\n"
                      << "  -h, --help           Show this help message.\n"
                      << "module                 The name of the Helper module\n"
                      << "uuid                   The uuid of the Helper to be launched.\n";
            return 0;
        }

        if (String ("-v") == argv [i] ||
            String ("--verbose") == argv [i]) {
            if (++i >= argc) {
                std::cerr << "No argument for option " << argv [i-1] << "\n";
                return -1;
            }
            DebugOutput::set_verbose_level (atoi (argv [i]));
            continue;
        }

        if (String ("-m") == argv [i] ||
            String ("--mask") == argv [i]) {
            if (++i >= argc) {
                std::cerr << "No argument for option " << argv [i-1] << "\n";
                return -1;
            }
            if (String (argv [i]) != "none") {
                std::vector<String> debug_mask_list;
                scim_split_string_list (debug_mask_list, argv [i], ',');
                DebugOutput::disable_debug (SCIM_DEBUG_AllMask);
                for (size_t j=0; j<debug_mask_list.size (); j++)
                    DebugOutput::enable_debug_by_name (debug_mask_list [j]);
            }
            continue;
        }

        if (String ("-o") == argv [i] ||
            String ("--output") == argv [i]) {
            if (++i >= argc) {
                std::cerr << "No argument for option " << argv [i-1] << "\n";
                return -1;
            }
            DebugOutput::set_output (String (argv [i]));
            continue;
        }

        ++j;

        if (j == 1) {
            helper = String (argv [i]);
            continue;
        } else if (j == 2) {
            uuid = String (argv [i]);
            continue;
        }

        std::cerr << "Invalid command line option: " << argv [i] << "\n";
        return -1;
    }

    SCIM_DEBUG_MAIN(1) << "scim-helper-launcher: " << config << " " << display << " " << helper << " " << uuid << "\n";

    if (!helper.length () || !uuid.length ()) {
        std::cerr << "Module name or Helper UUID is missing.\n";
        return -1;
    }

    HelperModule helper_module (helper);

    if (!helper_module.valid () || helper_module.number_of_helpers () == 0) {
        std::cerr << "Unable to load Helper module: " << helper << "\n";
        return -1;
    }

    ConfigModule config_module (config);

    ConfigPointer config_pointer;

    if (config_module.valid ()) {
        config_pointer = config_module.create_config ();
    }

    if (config_pointer.null ()) {
        config_pointer = new DummyConfig ();
    }

    if (daemon) scim_daemon ();

    helper_module.run_helper (uuid, config_pointer, display);
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
