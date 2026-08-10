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
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <functional>

using namespace scim;
using std::cout;
using std::cerr;
using std::endl;

// Run @launch (which blocks until the child exits, returning its status). When
// @supervise is set, restart a crashed child with exponential backoff; a clean
// exit (status 0) stops supervision. Without @supervise, run once and return.
static int run_supervised (bool supervise, const std::function<int ()> &launch)
{
    unsigned backoff = 1;
    for (;;) {
        time_t start = time (0);
        int rc = launch ();
        if (!supervise)
            return rc;
        if (rc == 0) {
            cerr << "SCIM: supervised child exited cleanly; stopping.\n";
            return 0;
        }
        if (time (0) - start >= 3)
            backoff = 1;   // ran a while before dying: reset the backoff
        cerr << "SCIM: supervised child died (rc=" << rc << "); restarting in "
             << backoff << "s.\n";
        sleep (backoff);
        if (backoff < 30)
            backoff *= 2;
    }
}

// Whether anything is listening on the backend socket. Answers "is a backend
// already running", i.e. whether we need to launch one.
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
    bool ibus_session = false;    // GNOME / ibus: run the sync coordinator
    bool frontend_forced = false; // an explicit -f overrides env detection

    // Grown as needed: the loop that fills this from the arguments after "--"
    // had no bound, and the terminating null went one past the end once the
    // array was full. Only scim_launch () reads it, and only to the null.
    std::vector<char *> new_argv;

    //Display version info (to stderr, so stdout stays clean for --xml / --list)
    cerr << "Smart Common Input Method " << SCIM_VERSION << "\n\n";

    //get modules list
    scim_get_frontend_module_list (frontend_list);
    scim_get_imengine_module_list (engine_list);
    scim_get_config_module_list   (config_list);

    //Choose the default FrontEnd(s) from the environment.
    if (frontend_list.size ()) {
        bool have_x11 = std::find (frontend_list.begin (), frontend_list.end (),
                                   String ("x11")) != frontend_list.end ();
        bool have_wayland = std::find (frontend_list.begin (), frontend_list.end (),
                                       String ("wayland")) != frontend_list.end ();

        const char *wl_display = getenv ("WAYLAND_DISPLAY");
        const char *session    = getenv ("XDG_SESSION_TYPE");
        bool wayland_session = (wl_display && *wl_display) ||
                               (session && String (session) == "wayland");

        // On a Wayland session prefer running wayland.so (native apps) and
        // x11.so (XWayland apps) together in one daemon. The launcher skips
        // wayland.so if it can't bind input-method-v2 (e.g. GNOME), leaving
        // x11.so, so "wayland,x11" is safe to default to everywhere.
        if (wayland_session && have_wayland)
            def_frontend = have_x11 ? String ("wayland,x11") : String ("wayland");
        else if (have_x11)
            def_frontend = String ("x11");
        else
            def_frontend = frontend_list [0];
    }

    // GNOME (and any ibus session) serves the IME through ibus.so, launched by
    // ibus-daemon -- not through our own frontends. There, this process's job
    // is the persistent, backend-free engine-list sync coordinator instead. If
    // the user explicitly forces our GTK module (GTK_IM_MODULE=scim), honor
    // that and stay a normal daemon.
    {
        const char *desktop   = getenv ("XDG_CURRENT_DESKTOP");
        const char *gtk_im    = getenv ("GTK_IM_MODULE");
        const char *ibus_addr = getenv ("IBUS_ADDRESS");
        bool force_scim = gtk_im && String (gtk_im) == "scim";
        ibus_session = !force_scim &&
            ((desktop   && strstr (desktop, "GNOME")) ||
             (gtk_im    && String (gtk_im) == "ibus") ||
             (ibus_addr && *ibus_addr));
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
    while (i < (size_t) argc) {
        if (++i >= (size_t) argc) break;

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
            if (++i >= (size_t) argc) {
                cerr << "No argument for option " << argv [i-1] << endl;
                return -1;
            }
            def_frontend = argv [i];
            frontend_forced = true;
            continue;
        }

        if (String ("-c") == argv [i] ||
            String ("--config") == argv [i]) {
            if (++i >= (size_t) argc) {
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
                 << "  -l, --list              List all of the available modules.\n"
                 << "  -f, --frontend name     Use the specified FrontEnd module (e.g. x11,\n"
                 << "                          socket, wayland, ibus); auto-detected from the\n"
                 << "                          session when not given.\n"
                 << "  -c, --config name       Use the specified Config module.\n"
                 << "  -e, --engines name      Load the specified set of IMEngines.\n"
                 << "  -ne,--no-engines name   Do not load that set of IMEngines.\n"
                 << "  -d, --daemon            Run in the background as a supervisor (restarts\n"
                 << "                          its children; on a GNOME/IBus session it runs the\n"
                 << "                          input-source coordinator instead of a frontend).\n"
                 << "  --no-socket             Do not try to start a SCIM SocketFrontEnd daemon.\n"
                 << "  --xml                   With '-f ibus' and passed after '--', print the\n"
                 << "                          IBus component manifest for the installed engines\n"
                 << "                          and exit.\n"
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
            if (++i >= (size_t) argc) {
                cerr << "No argument for option " << argv [i-1] << endl;
                return -1;
            }
            scim_split_string_list (load_engine_list, String (argv [i]), ',');
            manual = true;
            continue;
        }

        if (String ("-ne") == argv [i] || String ("-ns") == argv [i] ||
            String ("--no-engines") == argv [i] || String ("-no-servers") == argv [i]) {
            if (++i >= (size_t) argc) {
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
    for (++i; i < (size_t) argc; ++i)
        new_argv.push_back (argv [i]);

    new_argv.push_back (0);

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

    // GNOME / any ibus session: the IME is served by ibus.so via ibus-daemon,
    // not by our own input frontends. Here this process runs the shared backend
    // daemon that ibus.so relays to: a SocketFrontEnd (a warm backend, so engine
    // activation is instant and learned state survives ibus-daemon killing and
    // relaunching ibus.so) plus the "ibussync" coordinator frontend, which keeps
    // the enabled engines in sync with GNOME's input-source list off that same
    // backend.
    //
    // The two cannot share a launcher: SocketFrontEnd drives its own blocking
    // SocketServer::run() loop and does not implement the cooperative
    // (poll-fds / process-events) frontend interface, so scim-launcher refuses
    // "socket,ibussync" outright. Run the backend as its own daemon and
    // supervise the coordinator as a separate process that reaches the backend
    // over the socket (ibussync supports being loaded alone).
    //
    // This is decided BEFORE the frontend/config availability checks; skipped
    // when -f was given.
    if (ibus_session && !frontend_forced) {
        bool have_socket =
            std::find (frontend_list.begin (), frontend_list.end (),
                       String ("socket")) != frontend_list.end ();
        bool have_ibussync =
            std::find (frontend_list.begin (), frontend_list.end (),
                       String ("ibussync")) != frontend_list.end ();

        if (have_socket || have_ibussync) {
            cerr << "GNOME/ibus session: running the shared SCIM backend daemon"
                 << (have_socket   ? " (socket)" : "")
                 << (have_ibussync ? " with the ibussync coordinator" : "")
                 << "...\n";

            if (daemon)
                scim_daemon ();   // background ourselves; become the supervisor

            if (!have_ibussync) {
                // Nothing to coordinate: just keep the warm backend running.
                int rc = run_supervised (daemon, [&] () {
                    return scim_launch (false, def_config, "all", "socket", new_argv.data ());
                });
                return rc == 0 ? 0 : rc;
            }

            // Bring up (or adopt) the warm SocketFrontEnd. Launched with
            // daemon=true so it backgrounds itself and outlives any single
            // coordinator restart, and without --no-stay so it does not exit
            // while ibus-daemon is between ibus.so instances.
            std::function<bool ()> ensure_backend = [&] () -> bool {
                if (!have_socket)
                    return true;
                if (scim_socket_frontend_ready ())
                    return true;
                if (!check_socket_frontend ())
                    scim_launch (true, def_config, "all", "socket", 0);
                for (int i = 0; i < 100; ++i) {
                    if (scim_socket_frontend_ready ())
                        return true;
                    scim_usleep (100000);
                }
                cerr << "SCIM: the socket backend did not come up.\n";
                return false;
            };

            // With the backend up, the coordinator reaches the engine list and
            // config through it rather than loading its own copy.
            String co_config  = have_socket ? String ("socket") : def_config;
            String co_engines = have_socket ? String ("socket") : String ("all");

            int rc = run_supervised (daemon, [&] () {
                ensure_backend ();
                return scim_launch (false, co_config, co_engines,
                                    String ("ibussync"), new_argv.data ());
            });
            return rc == 0 ? 0 : rc;
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

    // Native path: become the supervisor -- in daemon mode background ourselves
    // once, then hold and restart the frontend worker below (run with
    // daemon=false so it stays our child and we can waitpid it).
    if (daemon)
        scim_daemon ();

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
            // No --no-stay: that flag means "exit once the last client goes",
            // which suits an im module launching a backend on demand for one
            // application, not a session daemon. Here it would let the backend
            // die whenever the frontend this same process supervises is
            // restarted, leaving the restarted frontend with no engines. The
            // ibus path already launches it this way for the same reason.
            scim_launch (true,
                         def_config,
                         (load_engine_list.size () ? scim_combine_string_list (load_engine_list, ',') : "all"),
                         "socket",
                         0);
            manual = false;
        }

        // If there is one Socket FrontEnd running and it's not manual mode,
        // then just use this Socket Frontend. Wait for it to be able to serve
        // its engine list, not merely to accept us: the frontend we are about to
        // launch asks for that list immediately, and gets one chance at it.
        if (!manual) {
            for (int i = 0; i < 100; ++i) {
                if (scim_socket_frontend_ready ()) {
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

    // Launch (and, in daemon mode, supervise) the frontend worker. The worker
    // runs with daemon=false so it stays our child; we restart it on a crash.
    String engines = load_engine_list.size ()
                     ? scim_combine_string_list (load_engine_list, ',') : "all";
    int rc = run_supervised (daemon, [&] () {
        return scim_launch (false, def_config, engines, def_frontend, new_argv.data ());
    });

    if (rc == 0) {
        cerr << "SCIM has exited successfully.\n";
        return 0;
    }

    cerr << "SCIM has exited abnormally.\n";
    return 1;
}

/*
vi:ts=4:ai:nowrap:expandtab
*/
