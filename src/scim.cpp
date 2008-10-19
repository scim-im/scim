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
 * $Id: scim.cpp,v 1.51 2005/06/15 00:19:08 suzhe Exp $
 *
 */

#define Uses_SCIM_FRONTEND_MODULE
#define Uses_SCIM_IMENGINE_MODULE
#define Uses_SCIM_BACKEND
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_TRANSACTION
#define Uses_C_LOCALE
#include "scim_private.h"
#include <scim.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

using namespace scim;
using std::cout;
using std::cerr;
using std::endl;

bool check_socket_frontend ()
{
    SocketAddress address;
    SocketClient client;

    uint32 magic;

    address.set_address (scim_get_default_socket_frontend_address ());

    if (!client.connect (address))
        return false;

    if (!scim_socket_open_connection (magic,
                                      String ("ConnectionTester"),
                                      String ("SocketFrontEnd"),
                                      client,
                                      1000)) {
        return false;
    }

    return true;
}

int main (int argc, char *argv [])
{
    BackEndPointer       backend;

    std::vector<String>  frontend_list;
    std::vector<String>  config_list;
    std::vector<String>  engine_list;
    std::vector<String>  exclude_engine_list;
    std::vector<String>  load_engine_list;

    String def_frontend;
    String def_config;

    size_t i;
    bool daemon = false;
    bool socket = true;
    bool manual = false;

    int   new_argc = 0;
    char *new_argv [80];

    //Display version info
    cout << "Smart Common Input Method " << SCIM_VERSION << "\n\n";

    //get modules list
    scim_get_frontend_module_list (frontend_list);
    scim_get_imengine_module_list (engine_list);
    scim_get_config_module_list   (config_list);

    //Use x11 FrontEnd as default if available.
    if (frontend_list.size ()) {
        def_frontend = String ("x11");
        if (std::find (frontend_list.begin (),
                       frontend_list.end (),
                       def_frontend) == frontend_list.end ())
            def_frontend = frontend_list [0];
    }

    //Add a dummy config module, it's not really a module!
    config_list.push_back ("dummy");

    //Use simple Config module as default if available.
    def_config = scim_global_config_read (SCIM_GLOBAL_CONFIG_DEFAULT_CONFIG_MODULE, String ("simple"));
    if (std::find (config_list.begin (),
                   config_list.end (),
                   def_config) == config_list.end ())
        def_config = config_list [0];

    // If no Socket Config/IMEngine/FrontEnd modules
    // then do not try to start a SocketFrontEnd.
    if (std::find (frontend_list.begin (), frontend_list.end (), "socket") == frontend_list.end () ||
        std::find (config_list.begin (), config_list.end (), "socket") == config_list.end () ||
        std::find (engine_list.begin (), engine_list.end (), "socket") == engine_list.end ())
        socket = false;

    //parse command options
    i = 0;
    while (i<argc) {
        if (++i >= argc) break;

        if (String ("-l") == argv [i] ||
            String ("--list") == argv [i]) {
            std::vector<String>::iterator it;

            cout << endl;
            cout << "Available FrontEnd module:\n";
            for (it = frontend_list.begin (); it != frontend_list.end (); it++)
                cout << "    " << *it << endl;

            cout << endl;
            cout << "Available Config module:\n";
            for (it = config_list.begin (); it != config_list.end (); it++)
                cout << "    " << *it << endl;

            cout << endl;
            cout << "Available IMEngine module:\n";
            for (it = engine_list.begin (); it != engine_list.end (); it++)
                cout << "    " << *it << endl;

            return 0;
        }

        if (String ("-f") == argv [i] ||
            String ("--frontend") == argv [i]) {
            if (++i >= argc) {
                cerr << "No argument for option " << argv [i-1] << endl;
                return -1;
            }
            def_frontend = argv [i];
            continue;
        }

        if (String ("-c") == argv [i] ||
            String ("--config") == argv [i]) {
            if (++i >= argc) {
                cerr << "No argument for option " << argv [i-1] << endl;
                return -1;
            }
            def_config = argv [i];
            continue;
        }

        if (String ("-h") == argv [i] ||
            String ("--help") == argv [i]) {
            cout << "Usage: " << argv [0] << " [option]...\n\n"
                 << "The options are: \n"
                 << "  -l, --list              List all of available modules.\n"
                 << "  -f, --frontend name     Use specified FrontEnd module.\n"
                 << "  -c, --config name       Use specified Config module.\n"
                 << "  -e, --engines name      Load specified set of IMEngines.\n"
                 << "  -ne,--no-engines name   Do not load those set of IMEngines.\n"
                 << "  -d, --daemon            Run " << argv [0] << " as a daemon.\n"
                 << "  --no-socket             Do not try to start a SCIM SocketFrontEnd daemon.\n"
                 << "  -h, --help              Show this help message.\n";
            return 0;
        }

        if (String ("-d") == argv [i] ||
            String ("--daemon") == argv [i]) {
            daemon = true;
            continue;
        }

        if (String ("-e") == argv [i] || String ("-s") == argv [i] ||
            String ("--engines") == argv [i] || String ("--servers") == argv [i]) {
            if (++i >= argc) {
                cerr << "No argument for option " << argv [i-1] << endl;
                return -1;
            }
            scim_split_string_list (load_engine_list, String (argv [i]), ',');
            manual = true;
            continue;
        }

        if (String ("-ne") == argv [i] || String ("-ns") == argv [i] ||
            String ("--no-engines") == argv [i] || String ("-no-servers") == argv [i]) {
            if (++i >= argc) {
                cerr << "No argument for option " << argv [i-1] << endl;
                return -1;
            }
            scim_split_string_list (exclude_engine_list, String (argv [i]), ',');
            manual = true;
            continue;
        }

        if (String ("--no-socket") == argv [i]) {
            socket = false;
            continue;
        }

        if (String ("--") == argv [i])
            break;

        cerr << "Invalid command line option: " << argv [i] << "\n";
        return -1;
    } //End of command line parsing.

    // Store the rest argvs into new_argv.
    for (++i; i < argc; ++i) {
        new_argv [new_argc ++] = argv [i];
    }

    new_argv [new_argc] = 0;

    // Get the imengine module list which should be loaded.
    if (exclude_engine_list.size ()) {
        load_engine_list.clear ();
        for (i = 0; i < engine_list.size (); ++i) {
            if (std::find (exclude_engine_list.begin (),
                           exclude_engine_list.end (),
                           engine_list [i]) == exclude_engine_list.end () &&
                engine_list [i] != "socket")
                load_engine_list.push_back (engine_list [i]);
        }
    }

    if (!def_frontend.length ()) {
        cerr << "No FrontEnd module is available!\n";
        return -1;
    }

    if (!def_config.length ()) {
        cerr << "No Config module is available!\n";
        return -1;
    }

    // If you try to use the socket feature manually,
    // then let you do it by yourself.
    if (def_frontend == "socket" || def_config == "socket" ||
        std::find (load_engine_list.begin (), load_engine_list.end (), "socket") != load_engine_list.end ())
        socket = false;

    // If the socket address of SocketFrontEnd and SocketIMEngine/SocketConfig are different,
    // then do not try to start the SocketFrontEnd instance automatically.
    if (scim_get_default_socket_frontend_address () != scim_get_default_socket_imengine_address () ||
        scim_get_default_socket_frontend_address () != scim_get_default_socket_config_address ())
        socket = false;

    // Try to start a SCIM SocketFrontEnd daemon first.
    if (socket) {
        // If no Socket FrontEnd is running, then launch one.
        // And set manual to false.
        if (!check_socket_frontend ()) {
            cerr << "Launching a SCIM daemon with Socket FrontEnd...\n";
            char *no_stay_argv [] = { const_cast<char*> ("--no-stay"), 0 };
            scim_launch (true,
                         def_config,
                         (load_engine_list.size () ? scim_combine_string_list (load_engine_list, ',') : "all"),
                         "socket",
                         no_stay_argv);
            manual = false;
        }

        // If there is one Socket FrontEnd running and it's not manual mode,
        // then just use this Socket Frontend.
        if (!manual) {
            for (int i = 0; i < 100; ++i) {
                if (check_socket_frontend ()) {
                    def_config = "socket";
                    load_engine_list.clear ();
                    load_engine_list.push_back ("socket");
                    break;
                }
                scim_usleep (100000);
            }
        }
    }

    cerr << "Launching a SCIM process with " << def_frontend << "...\n";

    // Launch the scim process.
    if (scim_launch (daemon,
                     def_config,
                     (load_engine_list.size () ? scim_combine_string_list (load_engine_list, ',') : "all"),
                     def_frontend,
                     new_argv) == 0) {
        if (daemon)
            cerr << "SCIM has been successfully launched.\n";
        else
            cerr << "SCIM has exited successfully.\n";

        return 0;
    }

    if (daemon)
        cerr << "Failed to launch SCIM.\n";
    else
        cerr << "SCIM has exited abnormally.\n";

    return 1;
}

/*
vi:ts=4:ai:nowrap:expandtab
*/
