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
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <cstring>
#include <ctime>

using namespace scim;

ConfigModule   *config_module = 0;
ConfigPointer   config;

// Set by the handler, read by the run loop below. volatile sig_atomic_t is what
// a handler is allowed to touch.
static volatile sig_atomic_t _signal_received = 0;

// Self-pipe, so the handler can wake a blocked select () by writing a byte --
// which is async-signal-safe, where anything that would otherwise do the job is
// not.
static int _signal_pipe [2] = { -1, -1 };

// Nothing here but a flag and one byte down the pipe.
//
// This used to flush the config, write to std::cerr and exit (0). None of the
// three may be called from a handler: they take locks and run buffered I/O, and
// exit () additionally runs the static destructors -- so a signal arriving
// inside any of that deadlocked or double-freed on the way out, which is a
// logout that hangs rather than ends. The shutdown now happens in main (),
// where it is merely ordinary code. The flush is not lost either way: returning
// from main () destroys the config, and ~SimpleConfig () flushes.
static void signalhandler (int sig)
{
    _signal_received = sig;

    if (_signal_pipe [1] >= 0) {
        char byte = 1;
        ssize_t ignored = write (_signal_pipe [1], &byte, 1);
        (void) ignored;   // a full pipe already carries the message
    }
}

static void install_signal_handlers ()
{
    if (pipe (_signal_pipe) == 0) {
        fcntl (_signal_pipe [0], F_SETFD, FD_CLOEXEC);
        fcntl (_signal_pipe [1], F_SETFD, FD_CLOEXEC);
        // The handler must never block on a pipe nobody is draining.
        fcntl (_signal_pipe [1], F_SETFL, O_NONBLOCK);
    }

    struct sigaction sa;
    memset (&sa, 0, sizeof (sa));
    sa.sa_handler = signalhandler;
    sigemptyset (&sa.sa_mask);
    // Deliberately no SA_RESTART, which is what signal (2) gives on glibc and
    // the reason the old handler had to exit () rather than return: a frontend
    // blocked in a select () of its own reads no flag of ours, and only an
    // interrupted syscall hands control back to main ().
    sa.sa_flags = 0;

    sigaction (SIGQUIT, &sa, 0);
    sigaction (SIGTERM, &sa, 0);
    sigaction (SIGINT,  &sa, 0);
    sigaction (SIGHUP,  &sa, 0);
}

// Service several frontends in one process via a shared select() loop, so a
// Wayland session can run wayland.so (native apps) and x11.so (XWayland apps)
// against one backend. Each frontend drains its own fds non-blocking.
//
// A frontend that asks to stop is dropped from the loop, not taken as a reason
// to stop the others: the two serve different sets of applications, and the
// wayland one exits for causes that say nothing about the x11 one -- the seat
// going away, a dispatch error, another input method claiming the seat. Letting
// it end the process took XWayland and XIM input down with it. The loop ends
// when the last frontend is gone.
//
// @return true if it ended for any reason other than a signal, i.e. the
// frontends ran out. main () turns that into a non-zero exit so the supervisor
// restarts us -- a compositor being replaced under a running session is the
// ordinary way every frontend goes at once, and it used to leave the session
// with no input method until the next login, because returning 0 read as a
// clean shutdown.
static bool
run_frontends_cooperatively (const std::vector<FrontEndModule *> &modules,
                             const std::vector<String> &names)
{
    // Indices into modules/names of the frontends still running.
    std::vector<size_t> live;
    for (size_t i = 0; i < modules.size (); ++i)
        live.push_back (i);

    while (!live.empty () && !_signal_received) {
        std::vector<int> fds;

        // Drain anything already buffered, then gather the fds to watch.
        for (size_t i = 0; i < live.size (); ) {
            modules[live[i]]->process_events ();
            if (modules[live[i]]->has_exited ()) {
                std::cerr << names[live[i]] << " FrontEnd has stopped"
                          << (live.size () > 1 ? "; continuing with the others.\n"
                                               : ".\n");
                live.erase (live.begin () + i);
            } else {
                ++i;
            }
        }
        if (live.empty ())
            break;

        for (size_t i = 0; i < live.size (); ++i)
            modules[live[i]]->poll_fds (fds);

        if (fds.empty ())
            break;

        fd_set read_fds;
        FD_ZERO (&read_fds);
        int max_fd = -1;
        for (size_t i = 0; i < fds.size (); ++i) {
            FD_SET (fds[i], &read_fds);
            if (fds[i] > max_fd) max_fd = fds[i];
        }

        // Watched alongside the frontends: a signal that arrives while we are
        // already inside select () shows up as EINTR, but one that arrives
        // between the flag test above and the call would otherwise wait for
        // unrelated input before being noticed.
        if (_signal_pipe [0] >= 0) {
            FD_SET (_signal_pipe [0], &read_fds);
            if (_signal_pipe [0] > max_fd) max_fd = _signal_pipe [0];
        }

        if (select (max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            if (errno == EINTR)
                continue;
            std::cerr << "SCIM: select failed: " << strerror (errno) << "\n";
            break;
        }
        // Ready events are handled by process_events() at the top of the loop.
    }

    return !_signal_received;
}

int main (int argc, char *argv [])
{
    BackEndPointer      backend;

    std::vector<String> engine_list;

    String config_name   ("simple");
    String frontend_name ("socket");   // may be a comma-separated list

    // Grown as needed. As a fixed array this had no bound at all on the loop
    // below that appends unrecognised options, and the two loops that did check
    // still let new_argc reach the array's length, so the terminating null went
    // one past the end exactly when the array was full.
    std::vector<char *> new_argv;

    int i = 0;
    bool daemon = false;

    new_argv.push_back (argv [0]);

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

        new_argv.push_back (argv [i]);
    } //End of command line parsing.

    // Construct new argv array for FrontEnd. config_name is settled by the
    // parsing above and not touched again, so holding its buffer is safe.
    new_argv.push_back (const_cast <char *> ("-c"));
    new_argv.push_back (const_cast <char *> (config_name.c_str ()));

    // Store the rest argvs into new_argv.
    for (++i; i < argc; ++i)
        new_argv.push_back (argv [i]);

    // Counted before the terminator goes on: the null ends the arguments, it is
    // not one of them.
    int new_argc = (int) new_argv.size ();
    new_argv.push_back (0);

    // Both socket clients ask the backend one question at startup and neither
    // retries: SocketConfig for the configuration, SocketIMEngine for the
    // factory list. Starting before the backend can answer therefore leaves this
    // process running with no engines and no config, silently, for as long as it
    // lives -- and the failed connect is not even logged.
    //
    // The scim wrapper waits on our behalf, but nothing does when the launcher
    // is exec'd directly, which is how the compositor's input method is started:
    // scim-virtual-keyboard.desktop names the launcher so that KWin's
    // pre-connected wayland fd survives (a second fork/exec would lose it), and
    // KWin starts it during compositor startup, before the session's autostart
    // entry has run "scim -d". That ordering is not a race we sometimes lose.
    //
    // Wait, never launch: "scim -d" is on its way, and an engineless backend
    // already owns the socket, so launching a second one would only collide.
    // Give up after a while and carry on degraded, which is today's behaviour --
    // a session with no backend at all should not hang the compositor's input
    // method forever.
    bool needs_socket_backend = (config_name == "socket");
    for (size_t n = 0; !needs_socket_backend && n < engine_list.size (); ++n)
        if (engine_list [n] == "socket")
            needs_socket_backend = true;

    if (needs_socket_backend && !scim_socket_frontend_ready ()) {
        std::cerr << "Waiting for the SCIM backend to serve its engines ...\n";

        bool ready = false;
        time_t deadline = time (0) + 10;

        while (time (0) < deadline) {
            scim_usleep (100000);
            if (scim_socket_frontend_ready ()) {
                ready = true;
                break;
            }
        }

        if (!ready)
            std::cerr << "The SCIM backend is not serving engines; "
                         "continuing without them.\n";
    }

    // Set when the run loop ends because every frontend stopped, rather than
    // because we were signalled. Declared out here so the exit code below can
    // see it.
    bool frontends_exhausted = false;

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

        //load FrontEnd module(s) -- "-f" may name several, comma-separated,
        //to run e.g. wayland + x11 concurrently against one backend.
        std::vector<String> frontend_names;
        scim_split_string_list (frontend_names, frontend_name, ',');

        // When several frontends are requested (e.g. "wayland,x11"), one that
        // cannot initialize on this session is skipped rather than fatal, so
        // the list degrades gracefully (e.g. wayland.so is dropped on a pure
        // X11 or GNOME session, leaving x11.so). A lone frontend that fails is
        // still a hard error.
        bool multi = frontend_names.size () > 1;

        std::vector<FrontEndModule *> frontend_modules;
        std::vector<String>           loaded_names;   // parallel to the above
        for (size_t n = 0; n < frontend_names.size (); ++n) {
            std::cerr << "Loading " << frontend_names[n] << " FrontEnd module ...\n";
            FrontEndModule *fem =
                new FrontEndModule (frontend_names[n], backend, config,
                                    new_argc, new_argv.data ());

            if (!fem || !fem->valid ()) {
                std::cerr << "Failed to load " << frontend_names[n]
                          << " FrontEnd module" << (multi ? " (skipping).\n" : ".\n");
                // Deliberately leaked, not deleted: FrontEndModule::load ()
                // keeps a module whose init () threw mapped on purpose, and
                // deleting it here would dlclose it anyway. A frontend that got
                // as far as connecting a config-reload slot would then leave that
                // slot pointing into unmapped code, and destroying the signal at
                // exit () would fault. The process is either exiting or carrying
                // on with the other frontends, so the handle costs nothing.
                if (!multi) return 1;
                continue;
            }
            frontend_modules.push_back (fem);
            loaded_names.push_back (frontend_names[n]);
        }

        if (frontend_modules.empty ()) {
            std::cerr << "No FrontEnd module could be loaded.\n";
            return 1;
        }

        // Running several frontends at once requires each to support the
        // cooperative (poll-fds / process-events) interface.
        //
        // Used for a lone frontend too, where it is not about sharing the loop:
        // a frontend blocked in its own run () sees no termination flag of ours,
        // and x11.so and wayland.so both retry their select () on EINTR, so a
        // signal would leave them running. This loop is the one that checks.
        // It is the same loop either way -- X11FrontEnd::run () is
        // process_events / poll_fds / select, exactly as below.
        bool cooperative = true;
        for (size_t n = 0; n < frontend_modules.size (); ++n) {
            if (!frontend_modules[n]->supports_cooperative_run ())
                cooperative = false;
        }
        if (frontend_modules.size () > 1 && !cooperative) {
            std::cerr << "Cannot run the requested frontends together: at least "
                         "one does not support cooperative running.\n";
            for (size_t j = 0; j < frontend_modules.size (); ++j)
                delete frontend_modules[j];
            return 1;
        }

        //reset backend pointer, in order to destroy backend automatically.
        backend.reset ();

        install_signal_handlers ();

        if (daemon) {
            std::cerr << "Starting SCIM as daemon ...\n";
            scim_daemon ();
        } else {
            std::cerr << "Starting SCIM ...\n";
        }

        if (cooperative)
            frontends_exhausted =
                run_frontends_cooperatively (frontend_modules, loaded_names);
        else
            frontend_modules[0]->run ();
    } catch (const std::exception & err) {
        std::cerr << err.what () << "\n";
        return 1;
    }

    // The shutdown the handler used to do, now that we are back in ordinary
    // code and may take a lock. Returning would flush anyway, through the
    // config's own destructor; doing it here keeps that from depending on when
    // a static is torn down.
    if (config != NULL)
        config->flush ();

    if (_signal_received)
        std::cerr << "SCIM successfully exited.\n";

    if (frontends_exhausted) {
        std::cerr << "SCIM: no FrontEnd is left running.\n";
        return 1;
    }

    return 0;
}

/*
vi:ts=4:ai:nowrap:expandtab
*/
