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
 * $Id: scim_launcher.cpp,v 1.9 2005/06/15 00:19:08 suzhe Exp $
 *
 */

#define Uses_SCIM_FRONTEND_MODULE
#define Uses_SCIM_IMENGINE_MODULE
#define Uses_SCIM_BACKEND
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_CONFIG
#define Uses_C_LOCALE
#include "scim_private.h"
#include "scim.h"
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

using namespace scim;

FrontEndModule *frontend_module = 0;
ConfigModule   *config_module = 0;
ConfigPointer   config;

void signalhandler(int sig)
{
    if (config != NULL) {
        config->flush ();
    }

    std::cerr << "SCIM successfully exited.\n";

    exit (0);
}

int main (int argc, char *argv [])
{
    BackEndPointer      backend;

    std::vector<String> engine_list;

    String config_name   ("simple");
    String frontend_name ("socket");

    int   new_argc = 0;
    char *new_argv [40];

    int i = 0;
    bool daemon = false;

    new_argv [new_argc ++] = argv [0];

    while (i<argc) {
        if (++i >= argc) break;

        if (String ("-f") == argv [i] ||
            String ("--frontend") == argv [i]) {
            if (++i >= argc) {
                std::cerr << "No argument for option " << argv [i-1] << "\n";
                return -1;
            }
            frontend_name = argv [i];
            continue;
        }

        if (String ("-c") == argv [i] ||
            String ("--config") == argv [i]) {
            if (++i >= argc) {
                std::cerr << "No argument for option " << argv [i-1] << "\n";
                return -1;
            }
            config_name = argv [i];
            continue;
        }

        if (String ("-d") == argv [i] ||
            String ("--daemon") == argv [i]) {
            daemon = true;
            continue;
        }

        if (String ("-e") == argv [i] ||
            String ("--engines") == argv [i]) {
            if (++i >= argc) {
                std::cerr << "No argument for option " << argv [i-1] << "\n";
                return -1;
            }
            if (String (argv [i]) == "all") {
                scim_get_imengine_module_list (engine_list);
                for (size_t j = 0; j < engine_list.size (); ++j) {
                    if (engine_list [j] == "socket") {
                        engine_list.erase (engine_list.begin () + j);
                        break;
                    }
                }
            } else if (String (argv [i]) != "none") {
                scim_split_string_list (engine_list, String (argv [i]), ',');
            }
            continue;
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

        if (String ("--") == argv [i])
            break;

        new_argv [new_argc ++] = argv [i];
    } //End of command line parsing.

    // Construct new argv array for FrontEnd.
    new_argv [new_argc ++] = const_cast <char *> ("-c");
    new_argv [new_argc ++] = const_cast <char *> (config_name.c_str ());

    // Store the rest argvs into new_argv.
    for (++i; i < argc && new_argc < 40; ++i) {
        new_argv [new_argc ++] = argv [i];
    }

    new_argv [new_argc] = 0;

    try {
        // Try to load config module
        std::cerr << "Loading " << config_name << " Config module ...\n";
        if (config_name != "dummy") {
            //load config module
            config_module = new ConfigModule (config_name);

            if (!config_module->valid ()) {
                std::cerr << "Can not load " << config_name << " Config module. Using dummy module instead.\n";
                delete config_module;
                config_module = 0;
            }

        }

        if (config_module) {
            config = config_module->create_config ();
        } else {
            config = new DummyConfig ();
        }

        if (config.null ()) {
            std::cerr << "Can not create Config Object!\n";
            return 1;
        }

        // create backend
        std::cerr << "Creating backend ...\n";
        backend = new CommonBackEnd (config, engine_list);

        //load FrontEnd module
        std::cerr << "Loading " << frontend_name << " FrontEnd module ...\n";
        frontend_module = new FrontEndModule (frontend_name, backend, config, new_argc, new_argv);

        if (!frontend_module || !frontend_module->valid ()) {
            std::cerr << "Failed to load " << frontend_name << " FrontEnd module.\n";
            return 1;
        }

        //reset backend pointer, in order to destroy backend automatically.
        backend.reset ();

        signal(SIGQUIT, signalhandler);
        signal(SIGTERM, signalhandler);
        signal(SIGINT,  signalhandler);
        signal(SIGHUP,  signalhandler);

        if (daemon) {
            std::cerr << "Starting SCIM as daemon ...\n";
            scim_daemon ();
        } else {
            std::cerr << "Starting SCIM ...\n";
        }

        frontend_module->run ();
    } catch (const std::exception & err) {
        std::cerr << err.what () << "\n";
        return 1;
    }

    return 0;
}

/*
vi:ts=4:ai:nowrap:expandtab
*/
