/** @file scim_helper_manager_server.cpp
 *  @brief Implementation of Helper Manager Server.
 */

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
 * $Id: scim_helper_manager_server.cpp,v 1.6 2005/01/30 13:24:12 suzhe Exp $
 *
 */

#define Uses_SCIM_TRANSACTION
#define Uses_SCIM_TRANS_COMMANDS
#define Uses_SCIM_HELPER
#define Uses_SCIM_HELPER_MODULE
#define Uses_SCIM_SOCKET
#define Uses_SCIM_EVENT

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include "scim_private.h"
#include "scim.h"
#include "scim_stl_map.h"

using namespace scim;

//////////////////////////////////////////////////////////////////////////////
// Data type
//////////////////////////////////////////////////////////////////////////////
enum ClientType {
    UNKNOWN_CLIENT,
    HELPER_MANAGER_CLIENT,
};

struct ClientInfo {
    uint32     key;
    ClientType type;
};

#if SCIM_USE_STL_EXT_HASH_MAP
typedef __gnu_cxx::hash_map <int, ClientInfo, __gnu_cxx::hash <int> >       ClientRepository;
#elif SCIM_USE_STL_HASH_MAP
typedef std::hash_map <int, ClientInfo, std::hash <int> >                   ClientRepository;
#else
typedef std::map <int, ClientInfo>                                          ClientRepository;
#endif

typedef std::vector < std::pair <HelperInfo, String> >                      HelperRepository;

//////////////////////////////////////////////////////////////////////////////
// Data definition. 
//////////////////////////////////////////////////////////////////////////////
static ClientRepository     __client_repository;
static int                  __socket_timeout        = -1;
static bool                 __should_exit           = false;
static Transaction          __recv_trans;
static Transaction          __send_trans;
static HelperRepository     __helpers;
static SocketServer         __socket_server;

//////////////////////////////////////////////////////////////////////////////
// Function definition. 
//////////////////////////////////////////////////////////////////////////////

void load_helper_modules (void)
{
    SCIM_DEBUG_MAIN (1) << "load_helper_modules ()\n";

    std::vector <String> mod_list;

    scim_get_helper_module_list (mod_list);

    // NOTE on FreeBSD if some module is loaded and unloaded right away here the following module crashes for some unknown reason
    //      seems like some damage is being done by module.unload();
    //      so I added a workaround: have an array of modules and unload them all together in the end only.
    //      TODO Need to figure out what's going on with this issue.

    if (mod_list.size ()) {

        HelperModule *module = new HelperModule[mod_list.size ()];

        for (size_t i = 0; i < mod_list.size (); ++i) {

            SCIM_DEBUG_MAIN (2) << " Load module: " << mod_list [i] << "\n";

            if (module[i].load (mod_list [i]) && module[i].valid ()) {
                HelperInfo info;
                size_t num = module[i].number_of_helpers ();

                SCIM_DEBUG_MAIN (2) << " Find " << num << " Helpers:\n";

                for (size_t j = 0; j < num; ++j) {
                    if (module[i].get_helper_info (j, info)) {
                        SCIM_DEBUG_MAIN (3) << "  " << info.uuid << ": " << info.name << "\n";
                        __helpers.push_back ( std::make_pair (info, mod_list [i]) );
                    }
                }
            }
        }
        for (size_t i = 0; i < mod_list.size (); ++i) {
            module[i].unload ();
        }
        delete[] module;
    }
}

void get_helper_list (const Socket &client)
{
    HelperRepository::iterator it = __helpers.begin ();

    __send_trans.clear ();
    __send_trans.put_command (SCIM_TRANS_CMD_REPLY);
    __send_trans.put_data ((uint32)__helpers.size ());

    for (; it != __helpers.end (); ++it) {
        __send_trans.put_data (it->first.uuid);
        __send_trans.put_data (it->first.name);
        __send_trans.put_data (it->first.icon);
        __send_trans.put_data (it->first.description);
        __send_trans.put_data (it->first.option);
    }

    __send_trans.write_to_socket (client);
}

#ifndef SCIM_HELPER_LAUNCHER_PROGRAM
  #define SCIM_HELPER_LAUNCHER_PROGRAM  (SCIM_LIBEXECDIR "/scim-helper-launcher")
#endif

void run_helper (const String &uuid, const String &config, const String &display)
{
    SCIM_DEBUG_MAIN(1) << "run_helper (" << uuid << "," << config << "," << display << ")\n";

    for (size_t i = 0; i < __helpers.size (); ++i) {
        if (__helpers [i].first.uuid == uuid && __helpers [i].second.length ()) {

            int pid;

            pid = fork ();

            if (pid < 0) return;

            if (pid == 0) {
                char * argv [] = { const_cast<char*> (SCIM_HELPER_LAUNCHER_PROGRAM),
                                   const_cast<char*> ("--daemon"),
                                   const_cast<char*> ("--config"), const_cast<char*> (config.c_str ()),
                                   const_cast<char*> ("--display"), const_cast<char*> (display.c_str ()),
                                   const_cast<char*> (__helpers [i].second.c_str ()),
                                   const_cast<char*> (__helpers [i].first.uuid.c_str ()),
                                   0};

                SCIM_DEBUG_MAIN(2) << " Call scim-helper-launcher.\n";

                execv (SCIM_HELPER_LAUNCHER_PROGRAM, argv);
                exit (-1);
            }

            int status;
            waitpid (pid, &status, 0);

            break;
        }
    }

    SCIM_DEBUG_MAIN(2) << " exit run_helper ().\n";
}

const ClientInfo & get_client_info (int client)
{
    static ClientInfo null_client = { 0, UNKNOWN_CLIENT };

    ClientRepository::iterator it = __client_repository.find (client);

    if (it != __client_repository.end ())
        return it->second;

    return null_client;
}

bool check_client_connection (const Socket &client)
{
    SCIM_DEBUG_MAIN (3) << "check_client_connection (" << client.get_id () << ")\n";

    unsigned char buf [sizeof(uint32)];

    int nbytes = client.read_with_timeout (buf, sizeof(uint32), __socket_timeout);

    if (nbytes == sizeof (uint32))
        return true;

    if (nbytes < 0) {
        SCIM_DEBUG_MAIN (4) << "Error occurred when reading socket: " << client.get_error_message () << ".\n";
    } else {
        SCIM_DEBUG_MAIN (4) << "Timeout when reading socket.\n";
    }

    return false;
}

bool socket_open_connection    (SocketServer *server, const Socket &client)
{
    SCIM_DEBUG_MAIN (3) << "socket_open_connection (" << client.get_id () << ")\n";

    uint32 key;
    String type = scim_socket_accept_connection (key,
                                                 String ("HelperLauncher"), 
                                                 String ("HelperManager"),
                                                 client,
                                                 __socket_timeout);

    if (type.length ()) {
        ClientInfo info;
        info.key  = key;
        info.type = HELPER_MANAGER_CLIENT;

        __client_repository [client.get_id ()] = info;

        return true;
    }

    SCIM_DEBUG_MAIN (4) << "Close client connection " << client.get_id () << "\n";
    server->close_connection (client);
    return false;
}

void socket_close_connection   (SocketServer *server, const Socket &client)
{
    ClientInfo client_info = get_client_info (client.get_id ());

    __client_repository.erase (client.get_id ());

    server->close_connection (client);

    if (client_info.type != UNKNOWN_CLIENT && __client_repository.size () == 0) {
        SCIM_DEBUG_MAIN (4) << "Exit Socket Server.\n";
        server->shutdown ();
    }
}

void socket_accept_callback    (SocketServer *server, const Socket &client)
{
    SCIM_DEBUG_MAIN (2) << "socket_accept_callback (" << client.get_id () << ")\n";

    if (__should_exit) {
        SCIM_DEBUG_MAIN (3) << "Exit Socket Server.\n";
        server->shutdown ();
    }
}

void socket_receive_callback   (SocketServer *server, const Socket &client)
{
    int     id      = client.get_id ();
    int     cmd     = 0;
    uint32  key     = 0;

    ClientInfo client_info;

    SCIM_DEBUG_MAIN (2) << "socket_receive_callback (" << id << ")\n";

    // If the connection is closed then close this client.
    if (!check_client_connection (client)) {
        socket_close_connection (server, client);
        return;
    }

    client_info = get_client_info (id);

    // If it's a new client, then request to open the connection first.
    if (client_info.type == UNKNOWN_CLIENT) {
        socket_open_connection (server, client);
        return;
    }

    // If can not read the transaction,
    // or the transaction is not started with SCIM_TRANS_CMD_REQUEST,
    // or the key is mismatch,
    // just return.
    if (!__recv_trans.read_from_socket (client, __socket_timeout) ||
        !__recv_trans.get_command (cmd) || cmd != SCIM_TRANS_CMD_REQUEST ||
        !__recv_trans.get_data (key)    || key != (uint32) client_info.key)
        return;
 
    while (__recv_trans.get_command (cmd)) {
        if (cmd == SCIM_TRANS_CMD_HELPER_MANAGER_GET_HELPER_LIST) {    
            get_helper_list (client);
        } else if (cmd == SCIM_TRANS_CMD_HELPER_MANAGER_RUN_HELPER) {
            String uuid;
            String config;
            String display;
            if (__recv_trans.get_data (uuid) && uuid.length () &&
                __recv_trans.get_data (config) &&
                __recv_trans.get_data (display)) {
                run_helper (uuid, config, display);
            }
        }
    }
}

void socket_exception_callback (SocketServer *server, const Socket &client)
{
    SCIM_DEBUG_MAIN (2) << "socket_exception_callback (" << client.get_id () << ")\n";

    socket_close_connection (server, client);
}

bool initialize_socket_server ()
{
    String address = scim_get_default_helper_manager_socket_address ();
    __socket_timeout = scim_get_default_socket_timeout ();

    if (!__socket_server.create (SocketAddress (address)))
        return false;

    __socket_server.signal_connect_accept (slot (socket_accept_callback));
    __socket_server.signal_connect_receive (slot (socket_receive_callback));
    __socket_server.signal_connect_exception (slot (socket_exception_callback));
    return true;
}

void signalhandler(int sig)
{
    SCIM_DEBUG_MAIN (1) << "signalhandler ()\n";

    __socket_server.shutdown ();
}

int main (int argc, char * argv [])
{
    int i = 0;
    bool daemon = true;

    while (i<argc) {
        if (++i >= argc) break;

        if (String ("-nd") == argv [i] ||
            String ("--no-daemon") == argv [i]) {
            daemon = false;
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

        std::cerr << "Invalid command line option: " << argv [i] << "\n";
        return -1;
    } //End of command line parsing.

    load_helper_modules ();

    if (!initialize_socket_server ()) {
        std::cerr << "Can't initialize SocketServer.\n";
        return -1;
    }

    if (daemon) scim_daemon ();

    signal(SIGQUIT, signalhandler);
    signal(SIGTERM, signalhandler);
    signal(SIGINT,  signalhandler);
    signal(SIGHUP,  signalhandler);

    __socket_server.run ();

    SCIM_DEBUG_MAIN (1) << "exit scim-helper-manager.\n";
}

/*
vi:ts=4:nowrap:ai:expandtab
*/

