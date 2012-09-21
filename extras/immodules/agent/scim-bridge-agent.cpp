/*
 * SCIM Bridge
 *
 * Copyright (c) 2006 Ryo Dairiki <ryo-dairiki@users.sourceforge.net>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation and 
 * appearing in the file LICENSE.LGPL included in the package of this file.
 * You can also redistribute it and/or modify it under the terms of 
 * the GNU General Public License as published by the Free Software Foundation and 
 * appearing in the file LICENSE.GPL included in the package of this file.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */;

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>

#include <list>
#include <vector>

#define Uses_SCIM_BACKEND
#define Uses_SCIM_CONFIG
#define Uses_SCIM_CONFIG_MODULE
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_EVENT
#define Uses_SCIM_FRONTEND
#define Uses_SCIM_PANEL_CLIENT
#define Uses_SCIM_HOTKEY
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_IMENGINE_MODULE

#include <scim.h>

#include "scim-bridge-agent.h"
#include "scim-bridge-agent-accept-listener.h"
#include "scim-bridge-agent-client-listener.h"
#include "scim-bridge-agent-imcontext.h"
#include "scim-bridge-agent-interruption-listener.h"
#include "scim-bridge-agent-panel-listener.h"
#include "scim-bridge-agent-protected.h"
#include "scim-bridge-agent-signal-listener.h"
#include "scim-bridge-agent-socket-client.h"

#include "scim-bridge-debug.h"
#include "scim-bridge-display.h"
#include "scim-bridge-output.h"
#include "scim-bridge-path.h"

using std::endl;
using std::ifstream;
using std::list;
using std::ofstream;
using std::vector;

using namespace scim;

/* Macros */
#ifndef SCIM_KEYBOARD_ICON_FILE
#define SCIM_KEYBOARD_ICON_FILE (SCIM_ICONDIR "/keyboard.png")
#endif


/* Class definition */
class ScimBridgeAgentImpl: public ScimBridgeAgent, public ScimBridgeAgentProtected
{

    public:

        ScimBridgeAgentImpl ();
        ~ScimBridgeAgentImpl ();

        retval_t launch ();
        void set_noexit_enabled (bool enabled);
        void set_standalone_enabled (bool enabled);

        /* Semi public funcitons */
        retval_t initialize ();
        retval_t finalize ();

        ScimBridgeAgentPanelListener *get_panel_listener (const ScimBridgeDisplay *new_display);

        void interrupt ();

        void quit ();
        void load_config ();
        void save_config ();

        void add_client_listener (ScimBridgeAgentClientListener *client_listener);
        void remove_client_listener (ScimBridgeAgentClientListener *client_listener);

        bool filter_hotkeys (scim_bridge_imcontext_id_t imcontext_id, const KeyEvent &key_event);
        virtual bool filter_key_event (scim_bridge_imcontext_id_t imcontext_id, const KeyEvent &key_event);

        void request_factory_menu ();

        scim_bridge_imcontext_id_t alloc_imcontext (ScimBridgeAgentClientListener *client_listener);

        void free_imcontext (scim_bridge_imcontext_id_t imcontext_id, const ScimBridgeAgentClientListener *client_listener);

        void reset_imcontext (scim_bridge_imcontext_id_t imcontext_id);

        void enable_imcontext (scim_bridge_imcontext_id_t imcontext_id);

        void disable_imcontext (scim_bridge_imcontext_id_t imcontext_id);

        void set_cursor_location (scim_bridge_imcontext_id_t imcontext_id, int cursor_x, int cursor_y);

        void set_preedit_mode (scim_bridge_imcontext_id_t imcontext_id, scim_bridge_preedit_mode_t preedit_mode);

        void change_focus (scim_bridge_imcontext_id_t imcontext_id, bool focus_in);

    private:

        bool running;

        bool noexit_enabled;
        bool standalone_enabled;

        list<ScimBridgeAgentSocketClient*> clients;
        size_t client_app_count;

        String scim_language;

        ConfigModule *scim_config_module;
        ConfigPointer scim_config;
        BackEndPointer scim_backend;

        KeyboardLayout scim_keyboard_layout;

        FrontEndHotkeyMatcher scim_frontend_hotkey_matcher;
        IMEngineHotkeyMatcher scim_imengine_hotkey_matcher;

        ScimBridgeAgentAcceptListener *accept_listener;
        ScimBridgeAgentInterruptionListener *interruption_listener;
        ScimBridgeAgentPanelListener *panel_listener;
        ScimBridgeAgentSignalListener *signal_listener;

        ScimBridgeDisplay *display;
        int valid_key_mask;

        retval_t initialize_scim ();
        retval_t finalize_scim ();

        retval_t run_event_loop ();

        retval_t check_socket ();

        void slot_reload_config (const ConfigPointer &config);

};

/* Helper functions */
static bool is_socket_frontend_ready ()
{
    SocketAddress address;
    address.set_address (scim_get_default_socket_frontend_address ());

    SocketClient socket_client;
    if (!socket_client.connect (address)) {
        scim_bridge_pdebugln (8, "There is no socket frontend at all.");        
        return false;
    } else {
        uint32_t magic;
        if (!scim_socket_open_connection (magic, "ConnectionTester", "SocketFrontEnd", socket_client, 1000)) {
            scim_bridge_pdebugln (8, "Failed to connect the socket fronent.");
            return false;
        } else {
            return true;
        }
    }
}


/* Implimentations */
ScimBridgeAgent *ScimBridgeAgent::alloc ()
{
    ScimBridgeAgentImpl *agent = new ScimBridgeAgentImpl ();
    if (agent->initialize ()) {
        delete agent;
        return NULL;
    } else {
        return agent;
    }
}


ScimBridgeAgentImpl::ScimBridgeAgentImpl ():
running (true), noexit_enabled (false), standalone_enabled (false), client_app_count (0),
scim_config_module(0),
accept_listener (NULL), interruption_listener (NULL), panel_listener (NULL), signal_listener (NULL), display (NULL)
{
}


ScimBridgeAgentImpl::~ScimBridgeAgentImpl ()
{
    finalize ();
}


void ScimBridgeAgentImpl::set_noexit_enabled (bool enabled)
{
    noexit_enabled = enabled;
}


void ScimBridgeAgentImpl::set_standalone_enabled (bool enabled)
{
    standalone_enabled = enabled;
}


retval_t ScimBridgeAgentImpl::run_event_loop ()
{
    scim_bridge_pdebugln (5, "run_event_loop ()");

    while (running) {
        int max_fd = -1;

        fd_set read_set;
        fd_set write_set;
        fd_set error_set;
        FD_ZERO (&read_set);
        FD_ZERO (&write_set);
        FD_ZERO (&error_set);
        for (list<ScimBridgeAgentSocketClient*>::iterator i = clients.begin (); running && i != clients.end ();) {
            ScimBridgeAgentSocketClient *client = *i;

            const scim_bridge_agent_event_type_t triggers = client->get_trigger_events ();
            const int socket_fd = client->get_socket_fd ();
            if (socket_fd < 0) {
                if (triggers & SCIM_BRIDGE_AGENT_EVENT_ERROR && !client->handle_event (SCIM_BRIDGE_AGENT_EVENT_ERROR)) {                        
                    i = clients.erase (i);
                    delete client;
                    continue;
                }
            } else {
                if (socket_fd > max_fd) max_fd = socket_fd;
                if (triggers & SCIM_BRIDGE_AGENT_EVENT_READ) {
                    scim_bridge_pdebugln (1, "FD (%d) is registred as a reading socket", socket_fd);
                    FD_SET (socket_fd, &read_set);
                }
                if (triggers & SCIM_BRIDGE_AGENT_EVENT_WRITE) {
                    FD_SET (socket_fd, &write_set);
                    scim_bridge_pdebugln (1, "FD (%d) is registred as a writing socket", socket_fd);
                }
                if (triggers & SCIM_BRIDGE_AGENT_EVENT_ERROR) {
                    FD_SET (socket_fd, &error_set);
                    scim_bridge_pdebugln (1, "FD (%d) is registred as a error socket", socket_fd);
                }
            }
            ++i;
        }

        scim_bridge_pdebugln (2, "Waiting for an event...");
        const int retval = select (max_fd + 1, &read_set, &write_set, &error_set, NULL);
        if (retval < 0 && errno != EINTR) {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            } else {
                scim_bridge_perrorln ("An exception occurred at selecting the sockets: %s", strerror (errno));
            }
        }

        const bool interrupted = interruption_listener->is_interrupted ();
        interruption_listener->clear_interruption ();
        if (interrupted) scim_bridge_pdebugln (3, "Caught an interruption");

        for (list<ScimBridgeAgentSocketClient*>::iterator i = clients.begin (); i != clients.end ();) {
            ScimBridgeAgentSocketClient *client = *i;

            const int socket_fd = client->get_socket_fd ();
            const scim_bridge_agent_event_type_t triggers = client->get_trigger_events ();

            scim_bridge_agent_event_type_t events = SCIM_BRIDGE_AGENT_EVENT_NONE;

            if (socket_fd >= 0 && FD_ISSET (socket_fd, &read_set) && (triggers & SCIM_BRIDGE_AGENT_EVENT_READ)) {
                if (events == SCIM_BRIDGE_AGENT_EVENT_NONE) {
                    scim_bridge_pdebug (2, "Invoked triggers: READ");
                } else {
                    scim_bridge_pdebug (2, ", READ");
                }
                events |= SCIM_BRIDGE_AGENT_EVENT_READ;
            }
            if (socket_fd >= 0 && FD_ISSET (socket_fd, &write_set) && (triggers & SCIM_BRIDGE_AGENT_EVENT_WRITE)) {
                if (events == SCIM_BRIDGE_AGENT_EVENT_NONE) {
                    scim_bridge_pdebug (2, "Invoked triggers: WRITE");
                } else {
                    scim_bridge_pdebug (2, ", WRITE");
                }
                events |= SCIM_BRIDGE_AGENT_EVENT_WRITE;
            }
            if ((socket_fd < 0 || FD_ISSET (socket_fd, &error_set)) && (triggers & SCIM_BRIDGE_AGENT_EVENT_ERROR)) {
                if (events == SCIM_BRIDGE_AGENT_EVENT_NONE) {
                    scim_bridge_pdebug (2, "Invoked triggers: ERROR");
                } else {
                    scim_bridge_pdebug (2, ", ERROR");
                }
                events |= SCIM_BRIDGE_AGENT_EVENT_ERROR;
            }
            if (interrupted && (triggers & SCIM_BRIDGE_AGENT_EVENT_INTERRUPT)) {
                if (events == SCIM_BRIDGE_AGENT_EVENT_NONE) {
                    scim_bridge_pdebug (2, "Invoked triggers: INTERRUPT");
                } else {
                    scim_bridge_pdebug (2, ", INTERRUPT");
                }
                events |= SCIM_BRIDGE_AGENT_EVENT_INTERRUPT;
            }

            if (events != SCIM_BRIDGE_AGENT_EVENT_NONE) {
                scim_bridge_pdebug (2, "\n");
                if (!client->handle_event (events)) {
                    i = clients.erase (i);
                    delete client;
                    continue;
                }
            }
            ++i;
        }
    }

    return RETVAL_SUCCEEDED;
}


retval_t ScimBridgeAgentImpl::launch ()
{
    scim_bridge_pdebugln (5, "launch ()");

    if (!standalone_enabled) {
        scim_bridge_pdebugln (5, "Daemonizing...");
        const pid_t pid = fork ();

        if (pid < 0) {
            scim_bridge_perrorln ("Cannot fork myself: %s", strerror (errno));
            return RETVAL_FAILED;
        } else if (pid > 0) {
            _exit (0);
        } else {
            // This is the child process.
            if (chdir ("/tmp")) {
                scim_bridge_perrorln ("Cannot change the working directory: %s", strerror (errno));
                return RETVAL_FAILED;
            }
            close (STDIN_FILENO);
            close (STDOUT_FILENO);
            close (STDERR_FILENO);
        }

        scim_bridge_pdebugln (5, "Daemonize done");
    }

    run_event_loop ();
    return RETVAL_SUCCEEDED;
}


retval_t ScimBridgeAgentImpl::initialize ()
{
    accept_listener = ScimBridgeAgentAcceptListener::alloc (this);
    if (accept_listener == NULL) return RETVAL_FAILED;
    clients.push_back (accept_listener);
    
    display = scim_bridge_alloc_display ();
    if (scim_bridge_display_fetch_current (display)) {
        scim_bridge_perrorln ("Failed to allocate the current display");
        return RETVAL_FAILED;
    }

    if (initialize_scim ()) {
        scim_bridge_perrorln ("Failed to initialize scim");
        return RETVAL_FAILED;
    }

    scim_bridge_pdebugln (4, "Loading configurations...");
    slot_reload_config (scim_config);
    scim_config->signal_connect_reload (slot (this, &ScimBridgeAgentImpl::slot_reload_config));

    interruption_listener = ScimBridgeAgentInterruptionListener::alloc ();
    if (interruption_listener == NULL) return RETVAL_FAILED;
    clients.push_back (interruption_listener);

    panel_listener = ScimBridgeAgentPanelListener::alloc (scim_config->get_name (), display, this);
    if (panel_listener == NULL) return RETVAL_FAILED;
    clients.push_back (panel_listener);

    signal_listener = ScimBridgeAgentSignalListener::alloc (this);
    if (signal_listener == NULL) return RETVAL_FAILED;
    clients.push_back (signal_listener);

    ScimBridgeAgentIMContext::static_initialize (panel_listener, scim_language, scim_backend);

    scim_bridge_pdebugln (4, "ScimBridgeAgent is now ready");
    return RETVAL_SUCCEEDED;
}


retval_t ScimBridgeAgentImpl::finalize ()
{
    for (list<ScimBridgeAgentSocketClient*>::iterator i = clients.begin (); i != clients.end (); ++i) {
        ScimBridgeAgentSocketClient *client = *i;
        delete client;
    }

    clients.clear ();

    ScimBridgeAgentIMContext::static_finalize ();

    finalize_scim ();

    if (display != NULL) {
        scim_bridge_free_display (display);
        display = NULL;
    }

    scim_bridge_pdebugln (4, "Finalize, done");
    return RETVAL_SUCCEEDED;
}


retval_t ScimBridgeAgentImpl::initialize_scim ()
{
    scim_bridge_pdebugln (6, "Initializing scim...");

    // Get system language.
    scim_language = scim_get_locale_language (scim_get_current_locale ());

    // Get imengine modules
    vector<String> imengine_module_names;
    scim_get_imengine_module_list (imengine_module_names);
    if (find (imengine_module_names.begin (), imengine_module_names.end (), "socket") == imengine_module_names.end ()) {
        scim_bridge_perrorln ("There is no socket frontend of IMEngines for SCIM...");
        return RETVAL_FAILED;
    }

	imengine_module_names.clear();
	imengine_module_names.push_back("socket");

    // Get config modules
    vector<String> config_module_names;
    scim_get_config_module_list (config_module_names);

    if (find (config_module_names.begin (), config_module_names.end (), "socket") == config_module_names.end ()) {
        scim_bridge_perrorln ("There is no socket frontend of config module for SCIM...");
        return RETVAL_FAILED;
    }
    
    // If there is no socket frontend running, launch one.
    if (!is_socket_frontend_ready ()) {
        scim_bridge_pdebugln (8, "Launching a SCIM daemon with Socket FrontEnd...");
        const String server_config_module_name = scim_global_config_read (SCIM_GLOBAL_CONFIG_DEFAULT_CONFIG_MODULE, String ("simple"));
        char* new_argv [] = {const_cast<char*>("--no-stay"),const_cast<char*>("-d"), NULL};
        scim_launch (true, server_config_module_name.c_str (), "all", "socket", new_argv);

        // Wait until the connection is established.
        for (int i = 0; i < 100; ++i) {
            if (is_socket_frontend_ready ()) {
                break;
            } else if (i < 99) {
                usleep (100000);
            } else {
                scim_bridge_perrorln ("Cannot establish the socket connection...");
                return RETVAL_FAILED;
            }
        }
    }
    
    {
    	scim_bridge_pdebugln (8, "Launching a SCIM daemon with x11 FrontEnd...");
    	char* new_argv [] = {const_cast<char*>("--no-stay"),const_cast<char*>("-d"), NULL};
    	scim_launch (true, "socket", "socket", "x11", new_argv);
    }

    // load config module
    scim_bridge_pdebugln (2, "Loading Config module...: socket");
    scim_config_module = new ConfigModule ("socket");

    //create config instance
    if (scim_config_module != NULL && scim_config_module->valid ()) {
        scim_config = scim_config_module->create_config ();
    } else {
        scim_bridge_pdebugln (2, "Cannot load the socket config module...");
        return RETVAL_FAILED;
    }

    // create backend
    scim_backend = new CommonBackEnd (scim_config, imengine_module_names);

    scim_bridge_pdebugln (4, "Initialize scim, done!");
    return RETVAL_SUCCEEDED;
}


retval_t ScimBridgeAgentImpl::finalize_scim ()
{
    scim_bridge_pdebugln (6, "Finalizing scim...");
    
    scim_backend.reset ();
    scim_config.reset ();

    if (scim_config_module != NULL) {
        delete scim_config_module;
        scim_config_module = NULL;
    }

    return RETVAL_SUCCEEDED;
}


void ScimBridgeAgentImpl::slot_reload_config (const ConfigPointer &config)
{
    scim_bridge_pdebugln (8, "slot_reload_config ()");

    scim_frontend_hotkey_matcher.load_hotkeys (scim_config);
    scim_imengine_hotkey_matcher.load_hotkeys (scim_config);
    
    KeyEvent key;
    scim_string_to_key (key,
        config->read (String (SCIM_CONFIG_HOTKEYS_FRONTEND_VALID_KEY_MASK),
                      String ("Shift+Control+Alt+Lock")));

    int key_mask = (key.mask > 0) ? key.mask : 0xFFFF;
    key_mask |= SCIM_KEY_ReleaseMask;
    // Special treatment for two backslash keys on jp106 keyboard.
    key_mask |= SCIM_KEY_QuirkKanaRoMask;
    
    valid_key_mask = key_mask;

    ScimBridgeAgentIMContext::set_enabled_by_default (scim_config->read (String (SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), ScimBridgeAgentIMContext::is_enabled_by_default ()));
    ScimBridgeAgentIMContext::set_imengine_shared (scim_config->read (String (SCIM_CONFIG_FRONTEND_SHARED_INPUT_METHOD), ScimBridgeAgentIMContext::is_imengine_shared ()));
    ScimBridgeAgentIMContext::set_on_the_spot_enabled (scim_config->read (String (SCIM_CONFIG_FRONTEND_ON_THE_SPOT), ScimBridgeAgentIMContext::is_on_the_spot_enabled ()));

    // Get keyboard layout setting
    // Flush the global config first, in order to load the new configs from disk.
    scim_global_config_flush ();

    scim_keyboard_layout = scim_get_default_keyboard_layout ();

    // Hot key name, hot key config key.
    // ! Update hotkey_list_length according to updated list length.
    int hotkey_list_length = 6;
    String hotkey_list[][2] = {
        { "Toggle on/off - ", "/Hotkeys/FrontEnd/Trigger" },
        { "Turn on - ", "/Hotkeys/FrontEnd/On" },
        { "Turn off - ", "/Hotkeys/FrontEnd/Off" },
        { "Next input method - ", "/Hotkeys/FrontEnd/NextFactory" },
        { "Previous input method - ", "/Hotkeys/FrontEnd/PreviousFactory" },
        { "Show input method menu - ", "/Hotkeys/FrontEnd/ShowFactoryMenu" }
    };
                           
    // Undefined hot keys are hidden from help window.
    String help_hotkeys = "\nGlobal Hotkeys:";
    for ( int i = 0; i < hotkey_list_length; i++ ) {
        String tmp_hotkeys = scim_config->read (String (hotkey_list[i][1]), String (""));
        if ( tmp_hotkeys != "" )
            help_hotkeys += "\n" + hotkey_list[i][0] + "<" + tmp_hotkeys + ">";
    }
    ScimBridgeAgentIMContext::set_help_hotkeys (help_hotkeys);
}


ScimBridgeAgentPanelListener *ScimBridgeAgentImpl::get_panel_listener (const ScimBridgeDisplay *new_display)
{
    if (scim_bridge_display_equals (display, new_display)) {
        return panel_listener;
    } else {
        return NULL;
    }
}


void ScimBridgeAgentImpl::quit ()
{
    scim_bridge_pdebugln (5, "quit ()");

    running = false;
}


void ScimBridgeAgentImpl::interrupt ()
{
    scim_bridge_pdebugln (2, "interrupt ()");

    interruption_listener->interrupt ();
}


void ScimBridgeAgentImpl::add_client_listener (ScimBridgeAgentClientListener *client_listener)
{
    scim_bridge_pdebugln (8, "add_client_listener ()");

    clients.push_back (client_listener);
    ++client_app_count;
}


void ScimBridgeAgentImpl::remove_client_listener (ScimBridgeAgentClientListener *client_listener)
{
    scim_bridge_pdebugln (8, "remove_client_listener ()");

    --client_app_count;
    if (!noexit_enabled && client_app_count <= 0) quit ();

    ScimBridgeAgentIMContext::free_by_client (client_listener);
}


scim_bridge_imcontext_id_t ScimBridgeAgentImpl::alloc_imcontext (ScimBridgeAgentClientListener *client_listener)
{
    scim_bridge_pdebugln (7, "alloc_imcontext ()");
    ScimBridgeAgentIMContext *imcontext = ScimBridgeAgentIMContext::alloc (client_listener);

    return imcontext->get_id ();
}


void ScimBridgeAgentImpl::free_imcontext (scim_bridge_imcontext_id_t imcontext_id, const ScimBridgeAgentClientListener *client_listener)
{
    scim_bridge_pdebugln (7, "free_imcontext ()");

    ScimBridgeAgentIMContext *imcontext = ScimBridgeAgentIMContext::find (imcontext_id);
    if (imcontext == NULL) {
        scim_bridge_perrorln ("No such IMContext");
    } else if (imcontext->get_client_listener () == client_listener) {
        panel_listener->prepare (imcontext_id);
        delete imcontext;
        panel_listener->send ();
    }
}


void ScimBridgeAgentImpl::change_focus (scim_bridge_imcontext_id_t imcontext_id, bool focus_in)
{
    scim_bridge_pdebugln (6, "change_focus ()");

    ScimBridgeAgentIMContext *imcontext = ScimBridgeAgentIMContext::find (imcontext_id);
    if (imcontext == NULL) {
        scim_bridge_perrorln ("No such IMContext");
    } else {
        panel_listener->prepare (imcontext_id);
        if (focus_in) {
            imcontext->focus_in ();
        } else {
            imcontext->focus_out ();
        }
        panel_listener->send ();
    }
}


bool ScimBridgeAgentImpl::filter_hotkeys (scim_bridge_imcontext_id_t imcontext_id, const KeyEvent &key_event)
{
    scim_bridge_pdebugln (6, "filter_hotkeys ()");

    ScimBridgeAgentIMContext *imcontext = ScimBridgeAgentIMContext::find (imcontext_id);
    if (imcontext == NULL) {
        scim_bridge_perrorln ("No such IMContext");
        return false;
    }

    scim_frontend_hotkey_matcher.push_key_event (key_event);

    const FrontEndHotkeyAction hotkey_action = scim_frontend_hotkey_matcher.get_match_result ();

    if (hotkey_action == SCIM_FRONTEND_HOTKEY_TRIGGER) {
        const bool new_status = !imcontext->is_enabled ();
        imcontext->set_enabled (new_status);
        save_config ();
        return true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_ON) {
        const bool new_status = true;
        imcontext->set_enabled (new_status);
        save_config ();
        return true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_OFF) {
        const bool new_status = false;
        imcontext->set_enabled (new_status);
        save_config ();
        return true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_NEXT_FACTORY) {
        imcontext->open_next_imengine ();
        return true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_PREVIOUS_FACTORY) {
        imcontext->open_previous_imengine ();
        return true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_SHOW_FACTORY_MENU) {
        request_factory_menu ();
        return true;
    } else {
        scim_imengine_hotkey_matcher.push_key_event (key_event);
        if (scim_imengine_hotkey_matcher.is_matched ()) {
            const String factory_uuid = scim_imengine_hotkey_matcher.get_match_result ();
            imcontext->open_imengine_by_uuid (factory_uuid);
            return true;
        }
    }

    return false;
}


bool ScimBridgeAgentImpl::filter_key_event (scim_bridge_imcontext_id_t imcontext_id, const KeyEvent &key_event)
{
    scim_bridge_pdebugln (5, "filter_key_event ()");
    
    KeyEvent new_key_event = key_event;
    new_key_event.mask &= valid_key_mask;
    new_key_event.layout = scim_keyboard_layout;

    ScimBridgeAgentIMContext *imcontext = ScimBridgeAgentIMContext::find (imcontext_id);
    if (imcontext == NULL) {
        scim_bridge_perrorln ("No such IMContext");
        return false;
    } else {
        bool consumed = false;
        panel_listener->prepare (imcontext_id);
        if (filter_hotkeys (imcontext_id, new_key_event)) {
            consumed = true;
        } else if (imcontext->filter_key_event (new_key_event)) {
            consumed = true;
        } else {
            consumed = false;
        }
        panel_listener->send ();

        return consumed;
    }
}


void ScimBridgeAgentImpl::set_cursor_location (scim_bridge_imcontext_id_t imcontext_id, int cursor_x, int cursor_y)
{
    scim_bridge_pdebugln (7, "set_cursor_location ()");

    ScimBridgeAgentIMContext *imcontext = ScimBridgeAgentIMContext::find (imcontext_id);
    if (imcontext == NULL) {
        scim_bridge_perrorln ("No such IMContext");
    } else {
        panel_listener->prepare (imcontext_id);
        imcontext->set_cursor_location (cursor_x, cursor_y);
        panel_listener->send ();
    }
}


void ScimBridgeAgentImpl::set_preedit_mode (scim_bridge_imcontext_id_t imcontext_id, scim_bridge_preedit_mode_t preedit_mode)
{
    scim_bridge_pdebugln (6, "set_preedit_mode ()");

    ScimBridgeAgentIMContext *imcontext = ScimBridgeAgentIMContext::find (imcontext_id);
    if (imcontext == NULL) {
        scim_bridge_perrorln ("No such IMContext");
    } else {
        panel_listener->prepare (imcontext_id);
        imcontext->set_preedit_mode (preedit_mode);
        panel_listener->send ();
    }
}


void ScimBridgeAgentImpl::reset_imcontext (scim_bridge_imcontext_id_t imcontext_id)
{
    scim_bridge_pdebugln (6, "reset_imcontext ()");

    ScimBridgeAgentIMContext *imcontext = ScimBridgeAgentIMContext::find (imcontext_id);
    if (imcontext == NULL) {
        scim_bridge_perrorln ("No such IMContext");
    } else {
        panel_listener->prepare (imcontext_id);
        imcontext->reset ();
        panel_listener->send ();
    }
}


void ScimBridgeAgentImpl::enable_imcontext (scim_bridge_imcontext_id_t imcontext_id)
{
    scim_bridge_pdebugln (6, "enable_imcontext ()");

    ScimBridgeAgentIMContext *imcontext = ScimBridgeAgentIMContext::find (imcontext_id);
    if (imcontext == NULL) {
        scim_bridge_perrorln ("No such IMContext");
    } else {
        panel_listener->prepare (imcontext_id);
        imcontext->set_enabled (TRUE);
        save_config ();
        panel_listener->send ();
    }
}


void ScimBridgeAgentImpl::disable_imcontext (scim_bridge_imcontext_id_t imcontext_id)
{
    scim_bridge_pdebugln (6, "disable_imcontext ()");

    ScimBridgeAgentIMContext *imcontext = ScimBridgeAgentIMContext::find (imcontext_id);
    if (imcontext == NULL) {
        scim_bridge_perrorln ("No such IMContext");
    } else {
        panel_listener->prepare (imcontext_id);
        imcontext->set_enabled (FALSE);
        save_config ();
        panel_listener->send ();
    }
}


void ScimBridgeAgentImpl::request_factory_menu ()
{
    scim_bridge_pdebugln (6, "request_factory_menu ()");

    vector<IMEngineFactoryPointer> factories;
    scim_backend->get_factories_for_encoding (factories, "UTF-8");

    vector<PanelFactoryInfo> menu;
    for (size_t i = 0; i < factories.size (); ++i) {
        menu.push_back (PanelFactoryInfo (factories [i]->get_uuid (), utf8_wcstombs (factories [i]->get_name ()), factories [i]->get_language (), factories [i]->get_icon_file ()));
    }
    if (!menu.empty ()) panel_listener->show_factory_menu (menu);
}


void ScimBridgeAgentImpl::load_config ()
{
    scim_bridge_pdebugln (6, "load_config ()");

    scim_config->reload ();
}


void ScimBridgeAgentImpl::save_config ()
{
    scim_bridge_pdebugln (6, "save_config ()");

    scim_config->write (String (SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), ScimBridgeAgentIMContext::is_enabled_by_default ());
}
