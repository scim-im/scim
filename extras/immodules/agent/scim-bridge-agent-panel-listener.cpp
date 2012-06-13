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
 */

#include <assert.h>
#include <unistd.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#define Uses_SCIM_IMENGINE
#define Uses_SCIM_IMENGINE_MODULE
#define Uses_SCIM_PANEL_CLIENT

#include <scim.h>

#include "scim-bridge-agent-imcontext.h"
#include "scim-bridge-agent-panel-listener.h"
#include "scim-bridge-agent-panel-listener-protected.h"
#include "scim-bridge-agent-protected.h"

#include "scim-bridge-display.h"
#include "scim-bridge-output.h"
#include "scim-bridge-path.h"

#include "utils/scim-bridge-agent-panel-client.h"

using std::vector;

using namespace scim;

/* Class definition */
class ScimBridgeAgentPanelListenerImpl: public ScimBridgeAgentPanelListener
{

    public:

        ScimBridgeAgentPanelListenerImpl (const String &config_name, const ScimBridgeDisplay *new_display, ScimBridgeAgentProtected *new_agent);
        ~ScimBridgeAgentPanelListenerImpl ();

        retval_t initialize ();

        int get_socket_fd () const;

        scim_bridge_agent_event_type_t get_trigger_events () const;

        bool handle_event (scim_bridge_agent_event_type_t event_type);

        /* The following functions are virtually protected */
        void prepare (scim_bridge_imcontext_id_t imcontext_id);
        void send ();

        void focus_in (const String &factory_uuid);
        void focus_out ();

        void update_screen ();
        void update_cursor_location (int x, int y);
        void update_factory_info (const PanelFactoryInfo &factory_info);

        void turn_on ();
        void turn_off ();

        void set_aux_string (const WideString &str, const AttributeList &attrs);
        void show_aux_string ();
        void hide_aux_string ();

        void set_lookup_table (const LookupTable &table);
        void show_lookup_table ();
        void hide_lookup_table ();

        void set_preedit_cursor_position (int cursor_pos);
        void set_preedit_string (const WideString &str, const AttributeList &attrs);
        void show_preedit ();
        void hide_preedit ();

        void start_helper (const String &helper_uuid);
        void stop_helper (const String &helper_uuid);
        void send_helper_event (const String &helper_uuid, const Transaction &trans);

        void register_properties (const PropertyList &properties);
        void update_property (const Property &property);

        void show_factory_menu (const vector<PanelFactoryInfo> &menu);
        void show_help (const String &string);

        void register_input_context (const String &factory_uuid);
        void deregister_input_context ();

    private:

        ScimBridgeAgentProtected *agent;

        ScimBridgeAgentPanelClient *scim_panel_client;

        int panel_fd;

        scim_bridge_imcontext_id_t prepared_imcontext_id;

        retval_t open_panel_client ();
        retval_t close_panel_client ();

        /* Panel related callbacks */
        void slot_reload_config (scim_bridge_imcontext_id_t id);
        void slot_exit (scim_bridge_imcontext_id_t id);
        void slot_update_lookup_table_page_size (scim_bridge_imcontext_id_t id, int page_size);
        void slot_lookup_table_page_up (scim_bridge_imcontext_id_t id);
        void slot_lookup_table_page_down (scim_bridge_imcontext_id_t id);
        void slot_trigger_property (scim_bridge_imcontext_id_t id, const scim::String &property);
        void slot_process_helper_event (scim_bridge_imcontext_id_t id, const String &target_uuid, const String &helper_uuid, const Transaction &trans);
        void slot_move_preedit_caret (scim_bridge_imcontext_id_t id, int caret_pos);
        void slot_select_candidate (scim_bridge_imcontext_id_t id, int cand_index);
        void slot_process_key_event (scim_bridge_imcontext_id_t id, const KeyEvent &key_event);
        void slot_commit (scim_bridge_imcontext_id_t id, const WideString &wstr);
        void slot_forward_key_event (scim_bridge_imcontext_id_t id, const KeyEvent &key_event);
        void slot_request_help (scim_bridge_imcontext_id_t id);
        void slot_request_factory_menu (scim_bridge_imcontext_id_t id);
        void slot_change_factory (scim_bridge_imcontext_id_t id, const String &uuid);

};

/* Implementations */
ScimBridgeAgentPanelListener *ScimBridgeAgentPanelListener::alloc (const String &config_name, const ScimBridgeDisplay *new_display, ScimBridgeAgentProtected *new_agent)
{
    ScimBridgeAgentPanelListenerImpl *panel_listener = new ScimBridgeAgentPanelListenerImpl (config_name, new_display, new_agent);
    if (panel_listener->initialize ()) {
        delete panel_listener;
        return NULL;
    } else {
        return panel_listener;
    }
}


ScimBridgeAgentPanelListenerImpl::ScimBridgeAgentPanelListenerImpl (const String &config_name, const ScimBridgeDisplay *display, ScimBridgeAgentProtected *new_agent): agent (new_agent), panel_fd (-1), prepared_imcontext_id (-1)
{
    scim_panel_client = ScimBridgeAgentPanelClient::alloc (config_name, display);

    scim_panel_client->signal_connect_reload_config (slot (this, &ScimBridgeAgentPanelListenerImpl::slot_reload_config));
    scim_panel_client->signal_connect_exit (slot (this, &ScimBridgeAgentPanelListenerImpl::slot_exit));
    scim_panel_client->signal_connect_update_lookup_table_page_size (slot (this, &ScimBridgeAgentPanelListenerImpl::slot_update_lookup_table_page_size));
    scim_panel_client->signal_connect_lookup_table_page_up (slot (this, &ScimBridgeAgentPanelListenerImpl::slot_lookup_table_page_up));
    scim_panel_client->signal_connect_lookup_table_page_down (slot (this, &ScimBridgeAgentPanelListenerImpl::slot_lookup_table_page_down));
    scim_panel_client->signal_connect_trigger_property (slot (this, &ScimBridgeAgentPanelListenerImpl::slot_trigger_property));
    scim_panel_client->signal_connect_process_helper_event (slot (this, &ScimBridgeAgentPanelListenerImpl::slot_process_helper_event));
    scim_panel_client->signal_connect_move_preedit_caret (slot (this, &ScimBridgeAgentPanelListenerImpl::slot_move_preedit_caret));
    scim_panel_client->signal_connect_select_candidate (slot (this, &ScimBridgeAgentPanelListenerImpl::slot_select_candidate));
    scim_panel_client->signal_connect_process_key_event (slot (this, &ScimBridgeAgentPanelListenerImpl::slot_process_key_event));
    scim_panel_client->signal_connect_commit_string (slot (this, &ScimBridgeAgentPanelListenerImpl::slot_commit));
    scim_panel_client->signal_connect_forward_key_event (slot (this, &ScimBridgeAgentPanelListenerImpl::slot_forward_key_event));
    scim_panel_client->signal_connect_request_help (slot (this, &ScimBridgeAgentPanelListenerImpl::slot_request_help));
    scim_panel_client->signal_connect_request_factory_menu (slot (this, &ScimBridgeAgentPanelListenerImpl::slot_request_factory_menu));
    scim_panel_client->signal_connect_change_factory (slot (this, &ScimBridgeAgentPanelListenerImpl::slot_change_factory));
}


ScimBridgeAgentPanelListenerImpl::~ScimBridgeAgentPanelListenerImpl ()
{
    delete scim_panel_client;

    if (panel_fd >= 0) close (panel_fd);
}


retval_t ScimBridgeAgentPanelListenerImpl::initialize ()
{
    for (int i = 0; i < 10; ++i) {
        if (open_panel_client ()) {
            usleep (1000);
        } else {
            return RETVAL_SUCCEEDED;
        }
    }
    return RETVAL_FAILED;
}


retval_t ScimBridgeAgentPanelListenerImpl::open_panel_client ()
{
    if (scim_panel_client->open_connection () >= 0) {
        panel_fd = scim_panel_client->get_connection_number ();
        scim_bridge_pdebugln (3, "Panel fd = %d", panel_fd);
        return RETVAL_SUCCEEDED;
    } else {
        scim_bridge_perrorln ("Failed to open the panel socket");
        panel_fd = -1;
        return RETVAL_FAILED;
    }
}


retval_t ScimBridgeAgentPanelListenerImpl::close_panel_client ()
{
    scim_bridge_pdebugln (4, "Close the panel client...");

    shutdown (panel_fd, SHUT_RDWR);
    scim_panel_client->close_connection ();
    panel_fd = -1;
    prepared_imcontext_id = -1;

    return RETVAL_FAILED;
}


int ScimBridgeAgentPanelListenerImpl::get_socket_fd () const
{
    return panel_fd;
}


scim_bridge_agent_event_type_t ScimBridgeAgentPanelListenerImpl::get_trigger_events () const
{
    return SCIM_BRIDGE_AGENT_EVENT_READ | SCIM_BRIDGE_AGENT_EVENT_ERROR;
}


bool ScimBridgeAgentPanelListenerImpl::handle_event (scim_bridge_agent_event_type_t event_type)
{
    scim_bridge_pdebugln (2, "handle_event ()");

    if ((event_type & SCIM_BRIDGE_AGENT_EVENT_ERROR) || scim_panel_client->filter_event ()) {
        close_panel_client ();
        usleep (500);
        open_panel_client ();
    }

    return true;
}


void ScimBridgeAgentPanelListenerImpl::prepare (scim_bridge_imcontext_id_t imcontext_id)
{
    scim_bridge_pdebugln (5, "prepare ()");
    if (panel_fd < 0) {
        scim_bridge_perrorln ("Connection lost with the panel client");
        return;
    }

    if (prepared_imcontext_id != -1) {
        scim_bridge_pdebugln (9, "The panel client has been already prepared");
        return;
    }

    prepared_imcontext_id = imcontext_id;
    scim_panel_client->prepare (prepared_imcontext_id);
}


void ScimBridgeAgentPanelListenerImpl::send ()
{
    scim_bridge_pdebugln (5, "send ()");

    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
    } else {
        scim_panel_client->send ();
        prepared_imcontext_id = -1;
    }
}


void ScimBridgeAgentPanelListenerImpl::focus_in (const String &factory_uuid)
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }
    scim_panel_client->focus_in (factory_uuid);
}


void ScimBridgeAgentPanelListenerImpl::focus_out ()
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }
    scim_panel_client->focus_out ();
}


void ScimBridgeAgentPanelListenerImpl::update_screen ()
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }
    scim_panel_client->update_screen ();
}


void ScimBridgeAgentPanelListenerImpl::update_cursor_location (int x, int y)
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }
    scim_panel_client->update_spot_location (x, y);
}


void ScimBridgeAgentPanelListenerImpl::update_factory_info (const PanelFactoryInfo &factory_info)
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }
    scim_panel_client->update_factory_info (factory_info);
}


void ScimBridgeAgentPanelListenerImpl::turn_on ()
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }
    scim_panel_client->turn_on ();
}


void ScimBridgeAgentPanelListenerImpl::turn_off ()
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }
    scim_panel_client->turn_off ();
}


void ScimBridgeAgentPanelListenerImpl::show_aux_string ()
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }
    scim_panel_client->show_aux_string ();
}


void ScimBridgeAgentPanelListenerImpl::hide_aux_string ()
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }
    scim_panel_client->hide_aux_string ();
}


void ScimBridgeAgentPanelListenerImpl::set_aux_string (const WideString &str, const AttributeList &attrs)
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }
    scim_panel_client->update_aux_string (str, attrs);
}


void ScimBridgeAgentPanelListenerImpl::set_lookup_table (const LookupTable &table)
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }
    scim_panel_client->update_lookup_table (table);
}


void ScimBridgeAgentPanelListenerImpl::show_lookup_table ()
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }
    scim_panel_client->show_lookup_table ();
}


void ScimBridgeAgentPanelListenerImpl::hide_lookup_table ()
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }
    scim_panel_client->hide_lookup_table ();
}


void ScimBridgeAgentPanelListenerImpl::set_preedit_cursor_position (int cursor_pos)
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }
    scim_panel_client->update_preedit_caret (cursor_pos);
}


void ScimBridgeAgentPanelListenerImpl::set_preedit_string (const WideString &str, const AttributeList &attrs)
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }
    scim_panel_client->update_preedit_string (str, attrs);
}


void ScimBridgeAgentPanelListenerImpl::show_preedit ()
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }
    scim_panel_client->show_preedit_string ();
}


void ScimBridgeAgentPanelListenerImpl::hide_preedit ()
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }
    scim_panel_client->hide_preedit_string ();
}


void ScimBridgeAgentPanelListenerImpl::register_properties (const PropertyList &properties)
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }
    scim_panel_client->register_properties (properties);
}


void ScimBridgeAgentPanelListenerImpl::update_property (const Property &property)
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }
    scim_panel_client->update_property (property);
}


void ScimBridgeAgentPanelListenerImpl::start_helper (const String &helper_uuid)
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }
    scim_panel_client->start_helper (helper_uuid);
}


void ScimBridgeAgentPanelListenerImpl::stop_helper (const String &helper_uuid)
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }
    scim_panel_client->stop_helper (helper_uuid);
}


void ScimBridgeAgentPanelListenerImpl::send_helper_event (const String &helper_uuid, const Transaction &trans)
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }
    scim_panel_client->send_helper_event (helper_uuid, trans);
}


void ScimBridgeAgentPanelListenerImpl::show_factory_menu (const vector<PanelFactoryInfo> &menu)
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }
    scim_panel_client->show_factory_menu (menu);
}


void ScimBridgeAgentPanelListenerImpl::show_help (const scim::String &string)
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }
    scim_panel_client->show_help (string);
}


void ScimBridgeAgentPanelListenerImpl::register_input_context (const String &factory_uuid)
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }

    scim_panel_client->register_input_context (factory_uuid);
}


void ScimBridgeAgentPanelListenerImpl::deregister_input_context ()
{
    if (prepared_imcontext_id == -1) {
        scim_bridge_pdebugln (8, "Panel client has not yet been prepared");
        return;
    }

    scim_panel_client->remove_input_context ();
}


/* Slot functions */
void ScimBridgeAgentPanelListenerImpl::slot_reload_config (scim_bridge_imcontext_id_t imcontext_id)
{
    agent->load_config ();
}


void ScimBridgeAgentPanelListenerImpl::slot_exit (scim_bridge_imcontext_id_t imcontext_id)
{
    scim_bridge_perrorln ("FIXME: not implemented yet: ScimBridgeAgentPanelListenerImpl::slot_exit ()");
}


void ScimBridgeAgentPanelListenerImpl::slot_update_lookup_table_page_size (scim_bridge_imcontext_id_t imcontext_id, int page_size)
{
    ScimBridgeAgentIMContext *imcontext = ScimBridgeAgentIMContext::find (imcontext_id);
    if (imcontext != NULL) {
        prepare (imcontext_id);
        imcontext->update_lookup_table_page_size (page_size);
        send ();
    }
}


void ScimBridgeAgentPanelListenerImpl::slot_lookup_table_page_up (scim_bridge_imcontext_id_t imcontext_id)
{
    ScimBridgeAgentIMContext *imcontext = ScimBridgeAgentIMContext::find (imcontext_id);
    if (imcontext != NULL) {
        prepare (imcontext_id);
        imcontext->lookup_table_page_up ();
        send ();
    }
}


void ScimBridgeAgentPanelListenerImpl::slot_lookup_table_page_down (scim_bridge_imcontext_id_t imcontext_id)
{
    ScimBridgeAgentIMContext *imcontext = ScimBridgeAgentIMContext::find (imcontext_id);
    if (imcontext != NULL) {
        prepare (imcontext_id);
        imcontext->lookup_table_page_down ();
        send ();
    }
}


void ScimBridgeAgentPanelListenerImpl::slot_trigger_property (scim_bridge_imcontext_id_t imcontext_id, const scim::String &property)
{
    ScimBridgeAgentIMContext *imcontext = ScimBridgeAgentIMContext::find (imcontext_id);
    if (imcontext != NULL) {
        prepare (imcontext_id);
        imcontext->trigger_property (property);
        send ();
    }
}


void ScimBridgeAgentPanelListenerImpl::slot_process_helper_event (scim_bridge_imcontext_id_t imcontext_id, const String &target_uuid, const String &helper_uuid, const Transaction &trans)
{
    ScimBridgeAgentIMContext *imcontext = ScimBridgeAgentIMContext::find (imcontext_id);
    if (imcontext != NULL) {
        prepare (imcontext_id);
        imcontext->process_helper_event (target_uuid, helper_uuid, trans);
        send ();
    }
}


void ScimBridgeAgentPanelListenerImpl::slot_move_preedit_caret (scim_bridge_imcontext_id_t imcontext_id, int caret_pos)
{
    ScimBridgeAgentIMContext *imcontext = ScimBridgeAgentIMContext::find (imcontext_id);
    if (imcontext != NULL) {
        prepare (imcontext_id);
        imcontext->panel_move_preedit_caret (caret_pos);
        send ();
    }
}


void ScimBridgeAgentPanelListenerImpl::slot_select_candidate (scim_bridge_imcontext_id_t imcontext_id, int cand_index)
{
    ScimBridgeAgentIMContext *imcontext = ScimBridgeAgentIMContext::find (imcontext_id);
    if (imcontext != NULL) {
        prepare (imcontext_id);
        imcontext->lookup_table_select_candidate (cand_index);
        send ();
    }
}


void ScimBridgeAgentPanelListenerImpl::slot_process_key_event (scim_bridge_imcontext_id_t imcontext_id, const KeyEvent &key_event)
{
    ScimBridgeAgentIMContext *imcontext = ScimBridgeAgentIMContext::find (imcontext_id);
    if (imcontext != NULL) {
        prepare (imcontext_id);
        if (!agent->filter_key_event (imcontext_id, key_event)) {
            imcontext->forward_key_event (key_event);
        }
        send ();
    }
}


void ScimBridgeAgentPanelListenerImpl::slot_commit (scim_bridge_imcontext_id_t imcontext_id, const WideString &wstr)
{
    ScimBridgeAgentIMContext *imcontext = ScimBridgeAgentIMContext::find (imcontext_id);
    if (imcontext != NULL) {
        prepare (imcontext_id);
        imcontext->panel_commit_string (wstr);
        send ();
    }
}


void ScimBridgeAgentPanelListenerImpl::slot_forward_key_event (scim_bridge_imcontext_id_t imcontext_id, const KeyEvent &key_event)
{
    ScimBridgeAgentIMContext *imcontext = ScimBridgeAgentIMContext::find (imcontext_id);
    if (imcontext != NULL) {
        prepare (imcontext_id);
        imcontext->forward_key_event (key_event);
        send ();
    }
}


void ScimBridgeAgentPanelListenerImpl::slot_request_help (scim_bridge_imcontext_id_t imcontext_id)
{
    ScimBridgeAgentIMContext *imcontext = ScimBridgeAgentIMContext::find (imcontext_id);
    if (imcontext != NULL) {
        prepare (imcontext_id);
        imcontext->panel_request_help ();
        send ();
    }
}


void ScimBridgeAgentPanelListenerImpl::slot_request_factory_menu (scim_bridge_imcontext_id_t imcontext_id)
{
    ScimBridgeAgentIMContext *imcontext = ScimBridgeAgentIMContext::find (imcontext_id);
    if (imcontext != NULL) {
        prepare (imcontext_id);
        agent->request_factory_menu ();
        send ();
    }
}


void ScimBridgeAgentPanelListenerImpl::slot_change_factory (scim_bridge_imcontext_id_t imcontext_id, const String &uuid)
{
    ScimBridgeAgentIMContext *imcontext = ScimBridgeAgentIMContext::find (imcontext_id);
    if (imcontext != NULL) {
        prepare (imcontext_id);
        imcontext->panel_change_factory (uuid);
        send ();

        agent->save_config ();
    }
}
