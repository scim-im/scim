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
#include <libintl.h>

#include <sys/time.h>

#include <list>
#include <vector>

#include "scim-bridge-output.h"

#include "scim-bridge-agent-client-listener.h"
#include "scim-bridge-agent-imcontext.h"

using std::list;
using std::vector;

using namespace scim;

class ScimBridgeAgentIMContextImpl;

static ScimBridgeAgentPanelListenerProtected *panel_listener;

static vector<ScimBridgeAgentIMContextImpl*> imcontexts;

static list<scim_bridge_imcontext_id_t> free_imcontexts;

static BackEndPointer scim_backend = NULL;

static IMEngineFactoryPointer fallback_imengine_factory = NULL;

static IMEngineInstancePointer shared_imengine = NULL;
static IMEngineInstancePointer fallback_imengine = NULL;

static String scim_language = "";

static bool imengine_shared = false;

static bool enabled_by_default = false;

static unsigned int imengine_id = 0;

static bool on_the_spot_enabled = true;

static String help_hotkeys = "";

/* Class definition */
class ScimBridgeAgentIMContextImpl: public ScimBridgeAgentIMContext
{

    public:

        static void attach_imengine (IMEngineInstancePointer imengine);

        static void slot_show_preedit (IMEngineInstanceBase *imengine);
        static void slot_hide_preedit (IMEngineInstanceBase *imengine);
        static void slot_update_preedit_string (IMEngineInstanceBase *imengine, const WideString &str, const AttributeList &attrs);
        static void slot_update_preedit_caret (IMEngineInstanceBase *imengine, int caret);

        static void slot_commit (IMEngineInstanceBase *imengine, const WideString &str);

        static void slot_show_aux_string (IMEngineInstanceBase *imengine);
        static void slot_hide_aux_string (IMEngineInstanceBase *imengine);
        static void slot_update_aux_string (IMEngineInstanceBase *imengine, const WideString &str, const AttributeList &attrs);

        static void slot_show_lookup_table (IMEngineInstanceBase *imengine);
        static void slot_hide_lookup_table (IMEngineInstanceBase *imengine);
        static void slot_update_lookup_table (IMEngineInstanceBase *imengine, const LookupTable &table);

        static void slot_register_properties (IMEngineInstanceBase *imengine, const PropertyList &properties);
        static void slot_update_property (IMEngineInstanceBase *imengine, const Property &property);

        static void slot_beep (IMEngineInstanceBase *imengine);

        static void slot_start_helper (IMEngineInstanceBase *imengine, const String &helper_uuid);
        static void slot_stop_helper (IMEngineInstanceBase *imengine, const String &helper_uuid);
        static void slot_send_helper_event (IMEngineInstanceBase *imengine, const String &helper_uuid, const Transaction &trans);

        static bool slot_get_surrounding_text (IMEngineInstanceBase *imengine, WideString &text, int &cursor, int maxlen_before, int maxlen_after);
        static bool slot_delete_surrounding_text (IMEngineInstanceBase *imengine, int offset, int length);

        static void slot_fallback_commit (IMEngineInstanceBase *imengine, const WideString &commit_string);
        static void slot_forward_key_event (IMEngineInstanceBase *imengine, const KeyEvent &key_event);

        ScimBridgeAgentIMContextImpl (ScimBridgeAgentClientListener *new_client_listener);
        ~ScimBridgeAgentIMContextImpl ();

        scim_bridge_preedit_mode_t get_preedit_mode () const;
        void set_preedit_mode (scim_bridge_preedit_mode_t new_preedit_mode);

        bool is_enabled ();
        void set_enabled (bool new_state);

        scim_bridge_imcontext_id_t get_id ();

        ScimBridgeAgentClientListener *get_client_listener ();

        void focus_in ();
        void focus_out ();

        void reset ();

        bool filter_key_event (const KeyEvent &key_event);
        void forward_key_event (const KeyEvent &key_event);

        void set_cursor_location (int x, int y);

        void update_lookup_table_page_size (int page_size);
        void lookup_table_page_up ();
        void lookup_table_page_down ();
        void lookup_table_select_candidate (int candidate_index);
        void trigger_property (const String &property);
        void process_helper_event (const String &target_uuid, const String &helper_uuid, const Transaction &trans);
        void panel_move_preedit_caret (int caret_pos);
        void panel_process_key_event (const KeyEvent &key_event);
        void panel_commit_string (const WideString &wstr);
        void panel_request_help ();
        void panel_change_factory (const String &uuid);

        /* Semi private functions */
        void alloc_imengine ();
        void free_imengine ();

    private:

        scim_bridge_imcontext_id_t id;

        ScimBridgeAgentClientListener *client_listener;

        IMEngineInstancePointer imengine;

        bool enabled;
        bool focused;

        scim_bridge_preedit_mode_t preedit_mode;
        bool preedit_shown;
        int preedit_cursor_position;

        WideString preedit_string;
        AttributeList preedit_attributes;

        int cursor_x;
        int cursor_y;

        IMEngineInstancePointer get_imengine ();

        void open_next_imengine ();
        void open_previous_imengine ();
        void open_imengine_by_uuid (const String &uuid);
        void open_imengine (IMEngineFactoryPointer factory);

        void show_preedit ();
        void hide_preedit ();
        void update_preedit_string (const WideString &str, const AttributeList &attrs);
        void update_preedit_caret (int cursor_pos);

        void commit (const WideString &str);

        void show_aux_string ();
        void hide_aux_string ();
        void update_aux_string (const WideString &str, const AttributeList &attrs);

        void show_lookup_table ();
        void hide_lookup_table ();
        void update_lookup_table (const LookupTable &table);

        void register_properties (const PropertyList &properties);
        void update_property (const Property &property);

        void beep ();

        void start_helper (const String &helper_uuid);
        void stop_helper (const String &helper_uuid);
        void send_helper_event (const String &helper_uuid, const Transaction &trans);

        bool get_surrounding_text (WideString &text, int &cursor, int maxlen_before, int maxlen_after);
        bool delete_surrounding_text (int offset, int length);
        bool replace_surrounding_text (const WideString &string, int cursor_position);

};

/* Implementations */
void ScimBridgeAgentIMContext::static_initialize (ScimBridgeAgentPanelListenerProtected *new_panel_listener, const String &new_language, BackEndPointer new_backend)
{
    panel_listener = new_panel_listener;
    scim_language = new_language;
    scim_backend = new_backend;

    fallback_imengine_factory = scim_backend->get_factory (SCIM_COMPOSE_KEY_FACTORY_UUID);
    if (fallback_imengine_factory.null ()) fallback_imengine_factory = new DummyIMEngineFactory ();

    fallback_imengine = fallback_imengine_factory->create_instance (String ("UTF-8"), 0);
    fallback_imengine->signal_connect_commit_string (slot (&ScimBridgeAgentIMContextImpl::slot_fallback_commit));

    set_imengine_shared (imengine_shared);
}


void ScimBridgeAgentIMContext::static_finalize ()
{
    panel_listener = NULL;

    shared_imengine = NULL;

    fallback_imengine.reset ();
    fallback_imengine = NULL;

    fallback_imengine_factory.reset ();
    fallback_imengine_factory = NULL;
    
    scim_backend = NULL;

    for (vector<ScimBridgeAgentIMContextImpl*>::iterator i = imcontexts.begin (); i != imcontexts.end (); ++i) {
        ScimBridgeAgentIMContext *imcontext = *i;
        if (imcontext != NULL) delete imcontext;
    }
    imcontexts.clear ();
    free_imcontexts.clear ();
}


ScimBridgeAgentIMContext *ScimBridgeAgentIMContext::alloc (ScimBridgeAgentClientListener *new_client_listener)
{
    return new ScimBridgeAgentIMContextImpl (new_client_listener);
}


void ScimBridgeAgentIMContext::free_by_client (const ScimBridgeAgentClientListener *client_listener)
{
    for (vector<ScimBridgeAgentIMContextImpl*>::iterator i = imcontexts.begin (); i != imcontexts.end (); ++i) {
        ScimBridgeAgentIMContext *imcontext = *i;
        if (imcontext != NULL && imcontext->get_client_listener () == client_listener) {
            delete imcontext;
        }
    }
}


ScimBridgeAgentIMContext *ScimBridgeAgentIMContext::find (scim_bridge_imcontext_id_t imcontext_id)
{
    if (imcontext_id < 0 || imcontext_id >= static_cast<ssize_t> (imcontexts.size ())) {
        return NULL;
    } else {
        return imcontexts[imcontext_id];
    }
}


bool ScimBridgeAgentIMContext::is_on_the_spot_enabled ()
{
    return on_the_spot_enabled;
}


void ScimBridgeAgentIMContext::set_on_the_spot_enabled (bool enabled)
{
    on_the_spot_enabled = enabled;
}


void ScimBridgeAgentIMContext::set_help_hotkeys (const String &hotkey_str)
{
    help_hotkeys = hotkey_str;
}


ScimBridgeAgentIMContextImpl::ScimBridgeAgentIMContextImpl (ScimBridgeAgentClientListener *new_client_listener):
client_listener (new_client_listener), imengine (NULL), enabled (false), focused(false) ,preedit_mode (PREEDIT_ANY),
preedit_shown (false), preedit_cursor_position (0)
{
    if (free_imcontexts.empty ()) {
        free_imcontexts.push_back (imcontexts.size ());
        imcontexts.push_back (NULL);
    }

    const scim_bridge_imcontext_id_t imcontext_id = free_imcontexts.back ();
    free_imcontexts.pop_back ();
    imcontexts[imcontext_id] = this;
    id = imcontext_id;
}


ScimBridgeAgentIMContextImpl::~ScimBridgeAgentIMContextImpl ()
{
    if (scim_backend != NULL) {
        panel_listener->deregister_input_context ();

        preedit_string.clear ();
        preedit_cursor_position = 0;
        preedit_shown = false;
        preedit_attributes.clear ();

        if (imengine_shared && static_cast<ScimBridgeAgentIMContextImpl*> (shared_imengine->get_frontend_data ()) == this) {
            shared_imengine->reset ();
            shared_imengine->set_frontend_data (NULL);
        } else if (imengine != NULL) {
            imengine->reset ();
            free_imengine ();
        }

        client_listener = NULL;
    }

    free_imcontexts.push_back (id);
    imcontexts[id] = NULL;
}


void ScimBridgeAgentIMContextImpl::attach_imengine (IMEngineInstancePointer new_imengine)
{
    new_imengine->signal_connect_show_preedit_string (slot (&ScimBridgeAgentIMContextImpl::slot_show_preedit));
    new_imengine->signal_connect_show_aux_string (slot (&ScimBridgeAgentIMContextImpl::slot_show_aux_string));
    new_imengine->signal_connect_show_lookup_table (slot (&ScimBridgeAgentIMContextImpl::slot_show_lookup_table));

    new_imengine->signal_connect_hide_preedit_string (slot (&ScimBridgeAgentIMContextImpl::slot_hide_preedit));
    new_imengine->signal_connect_hide_aux_string (slot (&ScimBridgeAgentIMContextImpl::slot_hide_aux_string));
    new_imengine->signal_connect_hide_lookup_table (slot (&ScimBridgeAgentIMContextImpl::slot_hide_lookup_table));

    new_imengine->signal_connect_update_preedit_caret (slot (&ScimBridgeAgentIMContextImpl::slot_update_preedit_caret));
    new_imengine->signal_connect_update_preedit_string (slot (&ScimBridgeAgentIMContextImpl::slot_update_preedit_string));
    new_imengine->signal_connect_update_aux_string (slot (&ScimBridgeAgentIMContextImpl::slot_update_aux_string));
    new_imengine->signal_connect_update_lookup_table (slot (&ScimBridgeAgentIMContextImpl::slot_update_lookup_table));
    new_imengine->signal_connect_commit_string (slot (&ScimBridgeAgentIMContextImpl::slot_commit));
    new_imengine->signal_connect_forward_key_event (slot (&ScimBridgeAgentIMContextImpl::slot_forward_key_event));

    new_imengine->signal_connect_register_properties (slot (&ScimBridgeAgentIMContextImpl::slot_register_properties));
    new_imengine->signal_connect_update_property (slot (&ScimBridgeAgentIMContextImpl::slot_update_property));

    new_imengine->signal_connect_beep (slot (&ScimBridgeAgentIMContextImpl::slot_beep));

    new_imengine->signal_connect_start_helper (slot (&ScimBridgeAgentIMContextImpl::slot_start_helper));
    new_imengine->signal_connect_stop_helper (slot (&ScimBridgeAgentIMContextImpl::slot_stop_helper));
    new_imengine->signal_connect_send_helper_event (slot (&ScimBridgeAgentIMContextImpl::slot_send_helper_event));

    new_imengine->signal_connect_get_surrounding_text (slot (&ScimBridgeAgentIMContextImpl::slot_get_surrounding_text));
    new_imengine->signal_connect_delete_surrounding_text (slot (&ScimBridgeAgentIMContextImpl::slot_delete_surrounding_text));
}


scim_bridge_imcontext_id_t ScimBridgeAgentIMContextImpl::get_id ()
{
    return id;
}


ScimBridgeAgentClientListener *ScimBridgeAgentIMContextImpl::get_client_listener ()
{
    return client_listener;
}


void ScimBridgeAgentIMContextImpl::reset ()
{
    if (get_imengine () != NULL) {
        ScimBridgeAgentIMContext* focused_imcontext = static_cast<ScimBridgeAgentIMContext*> (get_imengine ()->get_frontend_data ());
        if (focused_imcontext == this) get_imengine ()->reset ();
    }
}


bool ScimBridgeAgentIMContext::is_enabled_by_default ()
{
    return enabled_by_default;
}


void ScimBridgeAgentIMContext::set_enabled_by_default (bool enabled)
{
    enabled_by_default = enabled;
}


bool ScimBridgeAgentIMContext::is_imengine_shared ()
{
    return imengine_shared;
}


void ScimBridgeAgentIMContext::set_imengine_shared (bool shared)
{
    imengine_shared = shared;

    if (scim_backend == NULL) return;

    if (imengine_shared) {
        IMEngineFactoryPointer factory = scim_backend->get_default_factory (scim_language, "UTF-8");
        if (factory != NULL) {
            shared_imengine = factory->create_instance ("UTF-8", imengine_id);
            ++imengine_id;
        }
        if (shared_imengine == NULL) {
            shared_imengine = fallback_imengine;
        } else {
            ScimBridgeAgentIMContextImpl::attach_imengine (shared_imengine);
        }

        for (vector<ScimBridgeAgentIMContextImpl*>::iterator i = imcontexts.begin (); i != imcontexts.end (); ++i) {
            ScimBridgeAgentIMContextImpl *imcontext = *i;

            if (imcontext != NULL) {
                panel_listener->prepare (imcontext->get_id ());
                imcontext->free_imengine ();
                panel_listener->register_input_context (shared_imengine->get_factory_uuid ());
                panel_listener->send ();
            }
        }
    } else {
        shared_imengine = NULL;

        for (vector<ScimBridgeAgentIMContextImpl*>::iterator i = imcontexts.begin (); i != imcontexts.end (); ++i) {
            ScimBridgeAgentIMContextImpl *imcontext = *i;
            if (imcontext != NULL) imcontext->alloc_imengine ();
        }
    }
}


bool ScimBridgeAgentIMContextImpl::is_enabled ()
{
    return enabled;
}


void ScimBridgeAgentIMContextImpl::set_enabled (bool new_state)
{
    if (enabled != new_state) {
        client_listener->imengine_status_changed (id, new_state);
    }
    
    enabled = new_state;

    if (imengine_shared) enabled_by_default = enabled;

    if (enabled) {
        IMEngineFactoryPointer factory = scim_backend->get_factory (get_imengine ()->get_factory_uuid ());
        if (factory.null ()) factory = fallback_imengine_factory;
        PanelFactoryInfo info = PanelFactoryInfo (factory->get_uuid (), utf8_wcstombs (factory->get_name ()), factory->get_language (), factory->get_icon_file ());
        panel_listener->update_factory_info (info);

        panel_listener->hide_preedit ();
        panel_listener->hide_aux_string ();
        panel_listener->hide_lookup_table ();

        panel_listener->turn_on ();
        get_imengine ()->focus_in ();
    } else {
        get_imengine ()->focus_out ();

        hide_preedit ();

        PanelFactoryInfo info = PanelFactoryInfo (String (""), String ("English/Keyboard"), String ("C"), String (SCIM_KEYBOARD_ICON_FILE));
        panel_listener->update_factory_info (info);

        panel_listener->turn_off ();
    }
}


scim_bridge_preedit_mode_t ScimBridgeAgentIMContextImpl::get_preedit_mode () const
{
    return preedit_mode;
}


void ScimBridgeAgentIMContextImpl::set_preedit_mode (scim_bridge_preedit_mode_t new_preedit_mode)
{
    if (preedit_shown) {
        if ((preedit_mode == PREEDIT_ANY && on_the_spot_enabled) || preedit_mode == PREEDIT_EMBEDDED) {
            client_listener->set_preedit_shown (id, false);
            client_listener->update_preedit (id);
        } else {
            panel_listener->hide_preedit ();
        }
    }

    preedit_mode = new_preedit_mode;

    if (preedit_shown) show_preedit ();
}


void ScimBridgeAgentIMContextImpl::alloc_imengine ()
{
    imengine = NULL;

    IMEngineFactoryPointer factory = scim_backend->get_default_factory (scim_language, "UTF-8");
    if (factory != NULL) {
        imengine = factory->create_instance ("UTF-8", imengine_id);
        ++imengine_id;
    }
    if (imengine == NULL) {
        imengine = fallback_imengine;
    } else {
        attach_imengine (imengine);
        panel_listener->register_input_context (imengine->get_factory_uuid ());
    }

    enabled = false;
}


void ScimBridgeAgentIMContextImpl::free_imengine ()
{
    imengine = NULL;
}


IMEngineInstancePointer ScimBridgeAgentIMContextImpl::get_imengine ()
{
    if (imengine_shared) {
        return shared_imengine;
    } else {
        return imengine;
    }
}


bool ScimBridgeAgentIMContextImpl::filter_key_event (const KeyEvent &key_event)
{
    IMEngineInstancePointer imengine = get_imengine ();
    if (!imengine.null ()) {
        ScimBridgeAgentIMContext *focused_imcontext = static_cast<ScimBridgeAgentIMContext*> (imengine->get_frontend_data ());
        if (focused_imcontext != this) focus_in ();
    } else {
        focus_in ();
    }
    
    if (!is_enabled ()) {
        return false;
    } else if (get_imengine ()->process_key_event (key_event)) {
        return true;
    } else {
        return fallback_imengine->process_key_event (key_event);
    }
}


void ScimBridgeAgentIMContextImpl::open_next_imengine ()
{
    open_imengine (scim_backend->get_next_factory ("", "UTF-8", get_imengine ()->get_factory_uuid ()));
}


void ScimBridgeAgentIMContextImpl::open_previous_imengine ()
{
    open_imengine (scim_backend->get_previous_factory ("", "UTF-8", get_imengine ()->get_factory_uuid ()));
}


void ScimBridgeAgentIMContextImpl::open_imengine_by_uuid (const String &uuid)
{
    IMEngineFactoryPointer factory = scim_backend->get_factory (uuid);
    open_imengine (factory);
}


void ScimBridgeAgentIMContextImpl::open_imengine (IMEngineFactoryPointer factory)
{
    focus_out ();

    hide_preedit ();
    preedit_string.clear ();
    preedit_attributes.clear ();

    if (!factory.null ()) {
        set_enabled (true);
        if (imengine_shared) set_enabled_by_default (true);
        if (get_imengine ()->get_factory_uuid () == factory->get_uuid ()) return;

        IMEngineInstancePointer new_imengine = factory->create_instance ("UTF-8", get_imengine ()->get_id ());
        if (imengine_shared) {
            shared_imengine = new_imengine;
            shared_imengine->set_frontend_data (static_cast<ScimBridgeAgentIMContext*> (this));
        } else {
            imengine = new_imengine;
            imengine->set_frontend_data (static_cast<ScimBridgeAgentIMContext*> (this));
        }
        attach_imengine (new_imengine);
        scim_backend->set_default_factory (scim_language, factory->get_uuid ());
        panel_listener->register_input_context (factory->get_uuid ());
        focus_in ();
    } else {
        set_enabled (false);
        if (imengine_shared) set_enabled_by_default (false);
    }
}


void ScimBridgeAgentIMContextImpl::beep ()
{
    client_listener->beep (id);
}


void ScimBridgeAgentIMContextImpl::focus_in ()
{
    if (!imengine_shared && imengine == NULL) alloc_imengine ();

    ScimBridgeAgentIMContext *focused_imcontext = static_cast<ScimBridgeAgentIMContext*> (get_imengine ()->get_frontend_data ());
    if (focused_imcontext != NULL) focused_imcontext->focus_out ();

    get_imengine ()->set_frontend_data (static_cast<ScimBridgeAgentIMContext*> (this));
    panel_listener->register_input_context (get_imengine ()->get_factory_uuid ());

    if (imengine_shared) {
        if (enabled_by_default) {
            enabled = true;
        } else {
            enabled = false;
        }
        client_listener->imengine_status_changed (id, enabled);
    }

    panel_listener->focus_in (get_imengine ()->get_factory_uuid ());
    panel_listener->update_screen ();
    panel_listener->update_cursor_location (cursor_x, cursor_y);

    PanelFactoryInfo info;
    if (enabled) {
        IMEngineFactoryPointer factory = scim_backend->get_factory (get_imengine ()->get_factory_uuid ());
        if (factory.null ()) factory = fallback_imengine_factory;
        info = PanelFactoryInfo (factory->get_uuid (), utf8_wcstombs (factory->get_name ()), factory->get_language (), factory->get_icon_file ());
    } else {
        info = PanelFactoryInfo (String (""), String ("English/Keyboard"), String ("C"), String (SCIM_KEYBOARD_ICON_FILE));
    }
    panel_listener->update_factory_info (info);

    preedit_shown = false;

    if (enabled) {
        panel_listener->turn_on ();
        get_imengine ()->focus_in ();
    } else {
        panel_listener->turn_off ();
    }
    panel_listener->hide_preedit ();
    panel_listener->hide_aux_string ();
    panel_listener->hide_lookup_table ();
    hide_preedit ();

    focused = true;
}


void ScimBridgeAgentIMContextImpl::focus_out ()
{
    if (!imengine_shared && imengine == NULL) alloc_imengine ();

    ScimBridgeAgentIMContext *focused_imcontext = static_cast<ScimBridgeAgentIMContext*> (get_imengine ()->get_frontend_data ());

	if ( !focused )
         return;

    get_imengine ()->set_frontend_data (static_cast<ScimBridgeAgentIMContext*> (this));

    hide_preedit ();
    get_imengine ()->focus_out ();
    if (imengine_shared) shared_imengine->reset ();
    panel_listener->turn_off ();
    panel_listener->focus_out ();

    if (imengine_shared) {
        if (focused_imcontext != this) get_imengine ()->set_frontend_data (static_cast<ScimBridgeAgentIMContext*> (focused_imcontext));
    } else {
        imengine->set_frontend_data (NULL);
    }

    focused = false;
}


void ScimBridgeAgentIMContextImpl::set_cursor_location (int x, int y)
{
    if (preedit_mode != PREEDIT_FLOATING) {
        cursor_x = x;
        cursor_y = y;
        panel_listener->update_cursor_location (cursor_x, cursor_y);
    }
}


void ScimBridgeAgentIMContextImpl::show_preedit ()
{
    if ((preedit_mode == PREEDIT_ANY && on_the_spot_enabled) || preedit_mode == PREEDIT_EMBEDDED) {
        preedit_shown = true;
        client_listener->set_preedit_shown (id, true);
        client_listener->set_preedit_cursor_position (id, preedit_cursor_position);
        client_listener->set_preedit_string (id, preedit_string);
        client_listener->set_preedit_attributes (id, preedit_attributes);
        client_listener->update_preedit (id);
    } else {
        panel_listener->show_preedit ();
    }
}


void ScimBridgeAgentIMContextImpl::hide_preedit ()
{
    if ((preedit_mode == PREEDIT_ANY && on_the_spot_enabled) || preedit_mode == PREEDIT_EMBEDDED) {
        if (preedit_shown) {
            client_listener->set_preedit_shown (id, false);
            client_listener->update_preedit (id);
        }
        preedit_shown = false;
        preedit_cursor_position = 0;
        preedit_string.clear ();
        preedit_attributes.clear ();
    } else {
        panel_listener->hide_preedit ();
    }
}


void ScimBridgeAgentIMContextImpl::show_aux_string ()
{
    panel_listener->show_aux_string ();
}


void ScimBridgeAgentIMContextImpl::hide_aux_string ()
{
    panel_listener->hide_aux_string ();
}


void ScimBridgeAgentIMContextImpl::update_aux_string (const WideString &str, const AttributeList &attrs)
{
    panel_listener->set_aux_string (str, attrs);
}


void ScimBridgeAgentIMContextImpl::show_lookup_table ()
{
    panel_listener->show_lookup_table ();
}


void ScimBridgeAgentIMContextImpl::hide_lookup_table ()
{
    panel_listener->hide_lookup_table ();
}


void ScimBridgeAgentIMContextImpl::update_lookup_table (const LookupTable &table)
{
    panel_listener->set_lookup_table (table);
}


void ScimBridgeAgentIMContextImpl::update_preedit_caret (int cursor_pos)
{
    if (preedit_cursor_position == cursor_pos) return;

    preedit_cursor_position = cursor_pos;

    if (is_on_the_spot_enabled ()) {
        if (!preedit_shown) {
            client_listener->set_preedit_shown (id, true);
            preedit_shown = true;
        }
        client_listener->set_preedit_cursor_position (id, preedit_cursor_position);
        client_listener->update_preedit (id);
    } else {
        panel_listener->set_preedit_cursor_position (cursor_pos);
    }
}


void ScimBridgeAgentIMContextImpl::update_preedit_string (const WideString &str, const AttributeList &attrs)
{
    if (preedit_string.empty () && str.empty ()) return;

    preedit_string = str;
    preedit_attributes = attrs;

    if (is_on_the_spot_enabled ()) {
        if (!preedit_shown) {
            client_listener->set_preedit_shown (id, true);
            preedit_shown = true;
        }
        client_listener->set_preedit_string (id, preedit_string);

        // This will cause very annoying cursor movements for Japanese.
        //preedit_cursor_position = str.length ();
        //client_listener->push_event (new ScimBridgeAgentSetPreeditCursorPositionEvent (id, preedit_cursor_position));

        client_listener->set_preedit_attributes (id, preedit_attributes);
        client_listener->update_preedit (id);
    } else {
        panel_listener->set_preedit_string (str, attrs);
    }
}


void ScimBridgeAgentIMContextImpl::commit (const WideString &str)
{
    client_listener->commit_string (id, str);
}


void ScimBridgeAgentIMContextImpl::forward_key_event (const KeyEvent &key_event)
{
    client_listener->forward_key_event (id, key_event);
}


void ScimBridgeAgentIMContextImpl::register_properties (const PropertyList &properties)
{
    panel_listener->register_properties (properties);
}


void ScimBridgeAgentIMContextImpl::update_property (const Property &property)
{
    panel_listener->update_property (property);
}


void ScimBridgeAgentIMContextImpl::start_helper (const String &helper_uuid)
{
    panel_listener->start_helper (helper_uuid);
}


void ScimBridgeAgentIMContextImpl::stop_helper (const String &helper_uuid)
{
    panel_listener->stop_helper (helper_uuid);
}


void ScimBridgeAgentIMContextImpl::send_helper_event (const String &helper_uuid, const Transaction &trans)
{
    panel_listener->send_helper_event (helper_uuid, trans);
}


bool ScimBridgeAgentIMContextImpl::get_surrounding_text (WideString &string, int &cursor_pos, int max_before, int max_after)
{
    if (client_listener->get_surrounding_string (id, max_before, max_after, string, cursor_pos)) {
        return false;
    } else {
        return true;
    }
}


bool ScimBridgeAgentIMContextImpl::delete_surrounding_text (int offset, int length)
{
    if (client_listener->delete_surrounding_string (id, offset, length)) {
        return false;
    } else {
        return true;
    }
}


bool ScimBridgeAgentIMContextImpl::replace_surrounding_text (const WideString &string, int cursor_position)
{
    if (client_listener->replace_surrounding_string (id, string, cursor_position)) {
        return false;
    } else {
        return true;
    }
}


/* Called from panel_client? */
void ScimBridgeAgentIMContextImpl::update_lookup_table_page_size (int page_size)
{
    get_imengine ()->update_lookup_table_page_size (page_size);
}


void ScimBridgeAgentIMContextImpl::lookup_table_page_up ()
{
    get_imengine ()->lookup_table_page_up ();
}


void ScimBridgeAgentIMContextImpl::lookup_table_page_down ()
{
    get_imengine ()->lookup_table_page_down ();
}


void ScimBridgeAgentIMContextImpl::lookup_table_select_candidate (int candidate_index)
{
    get_imengine ()->select_candidate (candidate_index);
}


void ScimBridgeAgentIMContextImpl::trigger_property (const String &property)
{
    get_imengine ()->trigger_property (property);
}


void ScimBridgeAgentIMContextImpl::process_helper_event (const String &target_uuid, const String &helper_uuid, const Transaction &trans)
{
    get_imengine ()->process_helper_event (helper_uuid, trans);
}


void ScimBridgeAgentIMContextImpl::panel_move_preedit_caret (int caret_pos)
{
    get_imengine ()->move_preedit_caret (caret_pos);
}


void ScimBridgeAgentIMContextImpl::panel_commit_string (const WideString &wstr)
{
    commit (wstr);
}


void ScimBridgeAgentIMContextImpl::panel_request_help ()
{
    String help =  String ("SCIM Bridge") + 
        String (VERSION) +
        String ("\n(C) 2006-2008 Ryo Dairiki <ryo-dairiki@users.sourceforge.net>\n") +
        help_hotkeys +
        String ("\n\n");

        IMEngineFactoryPointer factory = scim_backend->get_factory (get_imengine ()->get_factory_uuid ());
        if (factory.null ()) factory = fallback_imengine_factory;
            help += utf8_wcstombs (factory->get_name ());
            help += String (":\n\n");

            help += utf8_wcstombs (factory->get_authors ());
            help += String ("\n\n");

            help += utf8_wcstombs (factory->get_help ());
            help += String ("\n\n");

            help += utf8_wcstombs (factory->get_credits ());

            panel_listener->show_help (help);
}


void ScimBridgeAgentIMContextImpl::panel_change_factory (const String &uuid)
{
    open_imengine_by_uuid (uuid);
}


/* Slot implementations */
void ScimBridgeAgentIMContextImpl::slot_show_preedit (IMEngineInstanceBase *imengine)
{
    scim_bridge_pdebugln (3, "slot_show_preedit...");

    ScimBridgeAgentIMContextImpl *imcontext = static_cast<ScimBridgeAgentIMContextImpl*> (imengine->get_frontend_data ());
    if (imcontext == NULL) {
        scim_bridge_pdebugln (8, "No imcontext is connected");
    } else {
        imcontext->show_preedit ();
    }
}


void ScimBridgeAgentIMContextImpl::slot_hide_preedit (IMEngineInstanceBase *imengine)
{
    scim_bridge_pdebugln (3, "slot_hide_preedit...");

    ScimBridgeAgentIMContextImpl *imcontext = static_cast<ScimBridgeAgentIMContextImpl*> (imengine->get_frontend_data ());
    if (imcontext == NULL) {
        scim_bridge_pdebugln (8, "No imcontext is connected");
    } else {
        imcontext->hide_preedit ();
    }
}


void ScimBridgeAgentIMContextImpl::slot_show_aux_string (IMEngineInstanceBase *imengine)
{
    scim_bridge_pdebugln (3, "slot_show_aux_string...");

    ScimBridgeAgentIMContextImpl *imcontext = static_cast<ScimBridgeAgentIMContextImpl*> (imengine->get_frontend_data ());
    if (imcontext == NULL) {
        scim_bridge_pdebugln (8, "No imcontext is connected");
    } else {
        imcontext->show_aux_string ();
    }
}


void ScimBridgeAgentIMContextImpl::slot_hide_aux_string (IMEngineInstanceBase *imengine)
{
    scim_bridge_pdebugln (3, "slot_hide_aux_string...");

    ScimBridgeAgentIMContextImpl *imcontext = static_cast<ScimBridgeAgentIMContextImpl*> (imengine->get_frontend_data ());
    if (imcontext == NULL) {
        scim_bridge_pdebugln (8, "No imcontext is connected");
    } else {
        imcontext->hide_aux_string ();
    }
}


void ScimBridgeAgentIMContextImpl::slot_update_aux_string (IMEngineInstanceBase *imengine, const WideString &str, const AttributeList &attrs)
{
    scim_bridge_pdebugln (3, "slot_update_aux_string...");

    ScimBridgeAgentIMContextImpl *imcontext = static_cast<ScimBridgeAgentIMContextImpl*> (imengine->get_frontend_data ());
    if (imcontext == NULL) {
        scim_bridge_pdebugln (8, "No imcontext is connected");
    } else {
        imcontext->update_aux_string (str, attrs);
    }
}


void ScimBridgeAgentIMContextImpl::slot_show_lookup_table (IMEngineInstanceBase *imengine)
{
    scim_bridge_pdebugln (3, "slot_show_lookup_table...");

    ScimBridgeAgentIMContextImpl *imcontext = static_cast<ScimBridgeAgentIMContextImpl*> (imengine->get_frontend_data ());
    if (imcontext == NULL) {
        scim_bridge_pdebugln (8, "No imcontext is connected");
    } else {
        imcontext->show_lookup_table ();
    }
}


void ScimBridgeAgentIMContextImpl::slot_hide_lookup_table (IMEngineInstanceBase *imengine)
{
    scim_bridge_pdebugln (3, "slot_hide_lookup_table...");

    ScimBridgeAgentIMContextImpl *imcontext = static_cast<ScimBridgeAgentIMContextImpl*> (imengine->get_frontend_data ());
    if (imcontext == NULL) {
        scim_bridge_pdebugln (8, "No imcontext is connected");
    } else {
        imcontext->hide_lookup_table ();
    }
}


void ScimBridgeAgentIMContextImpl::slot_update_lookup_table (IMEngineInstanceBase *imengine, const LookupTable &table)
{
    scim_bridge_pdebugln (3, "slot_update_lookup_table...");

    ScimBridgeAgentIMContextImpl *imcontext = static_cast<ScimBridgeAgentIMContextImpl*> (imengine->get_frontend_data ());
    if (imcontext == NULL) {
        scim_bridge_pdebugln (8, "No imcontext is connected");
    } else {
        imcontext->update_lookup_table (table);
    }
}


void ScimBridgeAgentIMContextImpl::slot_update_preedit_caret (IMEngineInstanceBase *imengine, int cursor_pos)
{
    scim_bridge_pdebugln (3, "slot_update_preedit_caret...");

    ScimBridgeAgentIMContextImpl *imcontext = static_cast<ScimBridgeAgentIMContextImpl*> (imengine->get_frontend_data ());
    if (imcontext == NULL) {
        scim_bridge_pdebugln (8, "No imcontext is connected");
    } else {
        imcontext->update_preedit_caret (cursor_pos);
    }
}


void ScimBridgeAgentIMContextImpl::slot_update_preedit_string (IMEngineInstanceBase *imengine, const WideString &str, const AttributeList &attrs)
{
    scim_bridge_pdebugln (3, "slot_update_preedit_string...");

    ScimBridgeAgentIMContextImpl *imcontext = static_cast<ScimBridgeAgentIMContextImpl*> (imengine->get_frontend_data ());
    if (imcontext == NULL) {
        scim_bridge_pdebugln (8, "No imcontext is connected");
    } else {
        imcontext->update_preedit_string (str, attrs);
    }
}


void ScimBridgeAgentIMContextImpl::slot_commit (IMEngineInstanceBase *imengine, const WideString &str)
{
    scim_bridge_pdebugln (3, "slot_commit_string...");

    ScimBridgeAgentIMContextImpl *imcontext = static_cast<ScimBridgeAgentIMContextImpl*> (imengine->get_frontend_data ());
    if (imcontext == NULL) {
        scim_bridge_pdebugln (8, "No imcontext is connected");
    } else {
        imcontext->commit (str);
    }
}


void ScimBridgeAgentIMContextImpl::slot_forward_key_event (IMEngineInstanceBase *imengine, const KeyEvent &key_event)
{
    scim_bridge_pdebugln (3, "slot_forward_key_event...");

    ScimBridgeAgentIMContextImpl *imcontext = static_cast<ScimBridgeAgentIMContextImpl*> (imengine->get_frontend_data ());
    if (imcontext == NULL) {
        scim_bridge_pdebugln (8, "No imcontext is connected");
    } else {
        imcontext->forward_key_event (key_event);
    }
}


void ScimBridgeAgentIMContextImpl::slot_register_properties (IMEngineInstanceBase *imengine, const PropertyList &properties)
{
    scim_bridge_pdebugln (3, "slot_register_properties...");

    ScimBridgeAgentIMContextImpl *imcontext = static_cast<ScimBridgeAgentIMContextImpl*> (imengine->get_frontend_data ());
    if (imcontext == NULL) {
        scim_bridge_pdebugln (8, "No imcontext is connected");
    } else {
        imcontext->register_properties (properties);
    }
}


void ScimBridgeAgentIMContextImpl::slot_update_property (IMEngineInstanceBase *imengine, const Property &property)
{
    scim_bridge_pdebugln (3, "slot_update_property...");

    ScimBridgeAgentIMContextImpl *imcontext = static_cast<ScimBridgeAgentIMContextImpl*> (imengine->get_frontend_data ());
    if (imcontext == NULL) {
        scim_bridge_pdebugln (8, "No imcontext is connected");
    } else {
        imcontext->update_property (property);
    }
}


void ScimBridgeAgentIMContextImpl::slot_beep (IMEngineInstanceBase *imengine)
{
    scim_bridge_pdebugln (3, "slot_beep...");

    ScimBridgeAgentIMContextImpl *imcontext = static_cast<ScimBridgeAgentIMContextImpl*> (imengine->get_frontend_data ());
    if (imcontext == NULL) {
        scim_bridge_pdebugln (8, "No imcontext is connected");
    } else {
        imcontext->beep ();
    }
}


void ScimBridgeAgentIMContextImpl::slot_start_helper (IMEngineInstanceBase *imengine, const String &helper_uuid)
{
    scim_bridge_pdebugln (3, "slot_start_helper...");

    ScimBridgeAgentIMContextImpl *imcontext = static_cast<ScimBridgeAgentIMContextImpl*> (imengine->get_frontend_data ());
    if (imcontext == NULL) {
        scim_bridge_pdebugln (8, "No imcontext is connected");
    } else {
        imcontext->start_helper (helper_uuid);
    }
}


void ScimBridgeAgentIMContextImpl::slot_stop_helper (IMEngineInstanceBase *imengine, const String &helper_uuid)
{
    scim_bridge_pdebugln (3, "slot_stop_helper...");

    ScimBridgeAgentIMContextImpl *imcontext = static_cast<ScimBridgeAgentIMContextImpl*> (imengine->get_frontend_data ());
    if (imcontext == NULL) {
        scim_bridge_pdebugln (8, "No imcontext is connected");
    } else {
        imcontext->stop_helper (helper_uuid);
    }
}


void ScimBridgeAgentIMContextImpl::slot_send_helper_event (IMEngineInstanceBase *imengine, const String &helper_uuid, const Transaction &trans)
{
    scim_bridge_pdebugln (3, "slot_send_helper_event...");

    ScimBridgeAgentIMContextImpl *imcontext = static_cast<ScimBridgeAgentIMContextImpl*> (imengine->get_frontend_data ());
    if (imcontext == NULL) {
        scim_bridge_pdebugln (8, "No imcontext is connected");
    } else {
        imcontext->send_helper_event (helper_uuid, trans);
    }
}


bool ScimBridgeAgentIMContextImpl::slot_get_surrounding_text (IMEngineInstanceBase *imengine, WideString &string, int &cursor, int maxlen_before, int maxlen_after)
{
    scim_bridge_pdebugln (3, "slot_get_surrounding_text...");

    ScimBridgeAgentIMContextImpl *imcontext = static_cast<ScimBridgeAgentIMContextImpl*> (imengine->get_frontend_data ());
    if (imcontext == NULL) {
        scim_bridge_pdebugln (8, "No imcontext is connected");
            return false;
    } else {
        return imcontext->get_surrounding_text (string, cursor, maxlen_before, maxlen_after);
    }
}


bool ScimBridgeAgentIMContextImpl::slot_delete_surrounding_text (IMEngineInstanceBase *imengine, int offset, int length)
{
    scim_bridge_pdebugln (3, "slot_delete_surrounding_text...");

    ScimBridgeAgentIMContextImpl *imcontext = static_cast<ScimBridgeAgentIMContextImpl*> (imengine->get_frontend_data ());
    if (imcontext == NULL) {
        scim_bridge_pdebugln (8, "No imcontext is connected");
            return false;
    } else {
        return imcontext->delete_surrounding_text (offset, length);
    }
}


void ScimBridgeAgentIMContextImpl::slot_fallback_commit (IMEngineInstanceBase *imengine, const WideString &commit_string)
{
    scim_bridge_pdebugln (3, "slot_fallback_commit_string...");

    ScimBridgeAgentIMContextImpl *imcontext = static_cast<ScimBridgeAgentIMContextImpl*> (imengine->get_frontend_data ());
    if (imcontext == NULL) {
        scim_bridge_pdebugln (8, "No imcontext is connected");
    } else {
        imcontext->commit (commit_string);
    }
}
