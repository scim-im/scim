/** @file gtkimcontextscim.cpp
 *  @brief immodule for GTK.
 */

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
 * $Id: gtkimcontextscim.cpp,v 1.170.2.13 2007/06/16 06:23:38 suzhe Exp $
 */

#define Uses_SCIM_DEBUG
#define Uses_SCIM_BACKEND
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_IMENGINE_MODULE
#define Uses_SCIM_CONFIG
#define Uses_SCIM_CONFIG_MODULE
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_TRANSACTION
#define Uses_SCIM_HOTKEY
#define Uses_SCIM_PANEL_CLIENT
#define Uses_C_STRING
#define Uses_C_STDIO
#define Uses_C_STDLIB
#define Uses_STL_IOSTREAM

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#ifdef GDK_WINDOWING_X11 
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <gdk/gdkx.h>
#endif

#include "scim_private.h"
#include "scim.h"

#ifdef GDK_WINDOWING_X11 
#include "scim_x11_utils.h"
#endif

#define SEND_EVENT_MASK 0x02

using namespace scim;

#include "gtkimcontextscim.h"

#define GTK_TYPE_IM_CONTEXT_SCIM             _gtk_type_im_context_scim
#define GTK_IM_CONTEXT_SCIM(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_IM_CONTEXT_SCIM, GtkIMContextSCIM))
#define GTK_IM_CONTEXT_SCIM_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_IM_CONTEXT_SCIM, GtkIMContextSCIMClass))
#define GTK_IS_IM_CONTEXT_SCIM(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_IM_CONTEXT_SCIM))
#define GTK_IS_IM_CONTEXT_SCIM_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_IM_CONTEXT_SCIM))
#define GTK_IM_CONTEXT_SCIM_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_IM_CONTEXT_SCIM, GtkIMContextSCIMClass))

#ifndef SCIM_KEYBOARD_ICON_FILE
  #define SCIM_KEYBOARD_ICON_FILE            (SCIM_ICONDIR "/keyboard.png")
#endif

#define SCIM_CONFIG_FRONTEND_GTK_IMMODULE_USE_KEY_SNOOPER  "/FrontEnd/GtkIMModule/UseKeySnooper"

/* Typedef */
struct _GtkIMContextSCIMImpl
{
    GtkIMContextSCIM        *parent;
    IMEngineInstancePointer  si;
    GdkWindow               *client_window;
    WideString               preedit_string;
    AttributeList            preedit_attrlist;
    gint                     preedit_caret;
    gint                     cursor_x;
    gint                     cursor_y;
    gboolean                 use_preedit;
    bool                     is_on;
    bool                     shared_si;
    bool                     preedit_started;
    bool                     preedit_updating;

    GtkIMContextSCIMImpl    *next;
};

/* Input Context handling functions. */
static GtkIMContextSCIMImpl * new_ic_impl               (GtkIMContextSCIM       *parent);
static void                   delete_ic_impl            (GtkIMContextSCIMImpl   *impl);
static void                   delete_all_ic_impl        ();

static GtkIMContextSCIM     * find_ic                   (int                     siid);

/* Methods declaration */
static void     gtk_im_context_scim_class_init          (GtkIMContextSCIMClass  *klass,
                                                         gpointer               *klass_data);
static void     gtk_im_context_scim_init                (GtkIMContextSCIM       *context_scim,
                                                         GtkIMContextSCIMClass  *klass);
static void     gtk_im_context_scim_finalize            (GObject                *obj);
static void     gtk_im_context_scim_finalize_partial    (GtkIMContextSCIM       *context_scim);
static void     gtk_im_context_scim_set_client_window   (GtkIMContext           *context,
                                                         GdkWindow              *client_window);
static gboolean gtk_im_context_scim_filter_keypress     (GtkIMContext           *context,
                                                         GdkEventKey            *key);
static void     gtk_im_context_scim_reset               (GtkIMContext           *context);
static void     gtk_im_context_scim_focus_in            (GtkIMContext           *context);
static void     gtk_im_context_scim_focus_out           (GtkIMContext           *context);
static void     gtk_im_context_scim_set_cursor_location (GtkIMContext           *context,
                                                         GdkRectangle           *area);
static void     gtk_im_context_scim_set_use_preedit     (GtkIMContext           *context,
                                                         gboolean                use_preedit);
static void     gtk_im_context_scim_get_preedit_string  (GtkIMContext           *context,
                                                         gchar                 **str,
                                                         PangoAttrList         **attrs,
                                                         gint                   *cursor_pos);

static gboolean gtk_scim_key_snooper                    (GtkWidget              *grab_widget,
                                                         GdkEventKey            *event,
                                                         gpointer                data);

static void     gtk_im_slave_commit_cb                  (GtkIMContext           *context,
                                                         const char             *str,
                                                         GtkIMContextSCIM       *context_scim);
static void     gtk_im_slave_preedit_changed_cb         (GtkIMContext           *context,
                                                         GtkIMContextSCIM       *context_scim);
static void     gtk_im_slave_preedit_start_cb           (GtkIMContext           *context,
                                                         GtkIMContextSCIM       *context_scim);
static void     gtk_im_slave_preedit_end_cb             (GtkIMContext           *context,
                                                         GtkIMContextSCIM       *context_scim);

/* private functions */
static void     panel_slot_reload_config                (int                     context);
static void     panel_slot_exit                         (int                     context);
static void     panel_slot_update_lookup_table_page_size(int                     context,
                                                         int                     page_size);
static void     panel_slot_lookup_table_page_up         (int                     context);
static void     panel_slot_lookup_table_page_down       (int                     context);
static void     panel_slot_trigger_property             (int                     context,
                                                         const String           &property);
static void     panel_slot_process_helper_event         (int                     context,
                                                         const String           &target_uuid,
                                                         const String           &helper_uuid,
                                                         const Transaction      &trans);
static void     panel_slot_move_preedit_caret           (int                     context,
                                                         int                     caret_pos);
static void     panel_slot_select_candidate             (int                     context,
                                                         int                     cand_index);
static void     panel_slot_process_key_event            (int                     context,
                                                         const KeyEvent         &key);
static void     panel_slot_commit_string                (int                     context,
                                                         const WideString       &wstr);
static void     panel_slot_forward_key_event            (int                     context,
                                                         const KeyEvent         &key);
static void     panel_slot_request_help                 (int                     context);
static void     panel_slot_request_factory_menu         (int                     context);
static void     panel_slot_change_factory               (int                     context,
                                                         const String           &uuid);

static void     panel_req_focus_in                      (GtkIMContextSCIM       *ic);
static void     panel_req_update_screen                 (GtkIMContextSCIM       *ic);
static void     panel_req_update_factory_info           (GtkIMContextSCIM       *ic);
static void     panel_req_update_spot_location          (GtkIMContextSCIM       *ic);
static void     panel_req_show_help                     (GtkIMContextSCIM       *ic);
static void     panel_req_show_factory_menu             (GtkIMContextSCIM       *ic);

/* Panel iochannel handler*/
static bool     panel_initialize                        ();
static void     panel_finalize                          ();
static gboolean panel_iochannel_handler                 (GIOChannel             *source,
                                                         GIOCondition            condition,
                                                         gpointer                user_data);

/* utility functions */
static bool     filter_hotkeys                          (GtkIMContextSCIM       *ic,
                                                         const KeyEvent         &key);
static void     turn_on_ic                              (GtkIMContextSCIM       *ic);
static void     turn_off_ic                             (GtkIMContextSCIM       *ic);

static void     set_ic_capabilities                     (GtkIMContextSCIM       *ic);

static KeyEvent keyevent_gdk_to_scim                    (const GtkIMContextSCIM *ic,
                                                         const GdkEventKey      &gdkevent);

static GdkEventKey keyevent_scim_to_gdk                 (const GtkIMContextSCIM *ic,
                                                         const KeyEvent         &scimkey);

static void     initialize                              (void);

static void     finalize                                (void);

static void     open_next_factory                       (GtkIMContextSCIM       *ic);
static void     open_previous_factory                   (GtkIMContextSCIM       *ic);
static void     open_specific_factory                   (GtkIMContextSCIM       *ic,
                                                         const String           &uuid);

static void     attach_instance                         (const IMEngineInstancePointer &si);

static IMEngineInstancePointer get_default_instance     ();

/* slot functions */
static void     slot_show_preedit_string                (IMEngineInstanceBase   *si);
static void     slot_show_aux_string                    (IMEngineInstanceBase   *si);
static void     slot_show_lookup_table                  (IMEngineInstanceBase   *si);

static void     slot_hide_preedit_string                (IMEngineInstanceBase   *si);
static void     slot_hide_aux_string                    (IMEngineInstanceBase   *si);
static void     slot_hide_lookup_table                  (IMEngineInstanceBase   *si);

static void     slot_update_preedit_caret               (IMEngineInstanceBase   *si,
                                                         int                     caret);
static void     slot_update_preedit_string              (IMEngineInstanceBase   *si,
                                                         const WideString       &str,
                                                         const AttributeList    &attrs);
static void     slot_update_aux_string                  (IMEngineInstanceBase   *si,
                                                         const WideString       &str,
                                                         const AttributeList    &attrs);
static void     slot_commit_string                      (IMEngineInstanceBase   *si,
                                                         const WideString       &str);
static void     slot_forward_key_event                  (IMEngineInstanceBase   *si,
                                                         const KeyEvent         &key);
static void     slot_update_lookup_table                (IMEngineInstanceBase   *si,
                                                         const LookupTable      &table);

static void     slot_register_properties                (IMEngineInstanceBase   *si,
                                                         const PropertyList     &properties);
static void     slot_update_property                    (IMEngineInstanceBase   *si,
                                                         const Property         &property);
static void     slot_beep                               (IMEngineInstanceBase   *si);
static void     slot_start_helper                       (IMEngineInstanceBase   *si,
                                                         const String           &helper_uuid);
static void     slot_stop_helper                        (IMEngineInstanceBase   *si,
                                                         const String           &helper_uuid);
static void     slot_send_helper_event                  (IMEngineInstanceBase   *si,
                                                         const String           &helper_uuid,
                                                         const Transaction      &trans);
static bool     slot_get_surrounding_text               (IMEngineInstanceBase   *si,
                                                         WideString             &text,
                                                         int                    &cursor,
                                                         int                     maxlen_before,
                                                         int                     maxlen_after);
static bool     slot_delete_surrounding_text            (IMEngineInstanceBase   *si,
                                                         int                     offset,
                                                         int                     len);

static void     reload_config_callback                  (const ConfigPointer    &config);

static void     fallback_commit_string_cb               (IMEngineInstanceBase   *si,
                                                         const WideString       &str);




/* Local variables declaration */
static String                                           _language;
static GtkIMContextSCIMImpl                            *_used_ic_impl_list          = 0;
static GtkIMContextSCIMImpl                            *_free_ic_impl_list          = 0;

static GType                                            _gtk_type_im_context_scim   = 0;
static GObjectClass                                    *_parent_klass               = 0;

static KeyboardLayout                                   _keyboard_layout            = SCIM_KEYBOARD_Default;
static int                                              _valid_key_mask             = SCIM_KEY_AllMasks;

static FrontEndHotkeyMatcher                            _frontend_hotkey_matcher;
static IMEngineHotkeyMatcher                            _imengine_hotkey_matcher;

static IMEngineInstancePointer                          _default_instance;

static ConfigModule                                    *_config_module              = 0;
static ConfigPointer                                    _config;
static BackEndPointer                                   _backend;

static GtkIMContextSCIM                                *_focused_ic                 = 0;
static GtkWidget                                       *_focused_widget             = 0;

static bool                                             _scim_initialized           = false;

static GdkColor                                         _normal_bg;
static GdkColor                                         _normal_text;
static GdkColor                                         _active_bg;
static GdkColor                                         _active_text;

static gint                                             _snooper_id                 = 0;
static bool                                             _snooper_installed          = false;

static int                                              _instance_count             = 0;
static int                                              _context_count              = 0;

static IMEngineFactoryPointer                           _fallback_factory;
static IMEngineInstancePointer                          _fallback_instance;

static PanelClient                                      _panel_client;

static GIOChannel                                      *_panel_iochannel            = 0;
static guint                                            _panel_iochannel_read_source= 0;
static guint                                            _panel_iochannel_err_source = 0;
static guint                                            _panel_iochannel_hup_source = 0;

static bool                                             _on_the_spot                = true;
static bool                                             _shared_input_method        = false;
static bool                                             _use_key_snooper            = false;

// A hack to shutdown the immodule cleanly even if im_module_exit () is not called when exiting.
class FinalizeHandler
{
public:
    FinalizeHandler () {
        SCIM_DEBUG_FRONTEND(1) << "FinalizeHandler::FinalizeHandler ()\n";
    }
    ~FinalizeHandler () {
        if (_scim_initialized) {
            SCIM_DEBUG_FRONTEND(1) << "FinalizeHandler::~FinalizeHandler ()\n";
            gtk_im_context_scim_shutdown ();
        }
    }
};

static FinalizeHandler                                  _finalize_handler;

/* Function Implementations */

static GtkIMContextSCIMImpl *
new_ic_impl (GtkIMContextSCIM *parent)
{
    GtkIMContextSCIMImpl *impl;

    if (_free_ic_impl_list != NULL) {
        impl = _free_ic_impl_list;
        _free_ic_impl_list = _free_ic_impl_list->next;
    } else {
        impl = new GtkIMContextSCIMImpl;
    }

    impl->next = _used_ic_impl_list;
    _used_ic_impl_list = impl;

    impl->parent = parent;

    return impl;
}

static void
delete_ic_impl (GtkIMContextSCIMImpl   *impl)
{
    GtkIMContextSCIMImpl *rec, *last = 0;

    for (rec = _used_ic_impl_list; rec != 0; last = rec, rec = rec->next) {
        if (rec == impl) {
            if (last != 0)
                last->next = rec->next;
            else
                _used_ic_impl_list = rec->next;

            rec->next = _free_ic_impl_list;
            _free_ic_impl_list = rec;

            rec->parent = 0;
            rec->si.reset ();
            rec->client_window = 0;
            rec->preedit_string = WideString ();
            rec->preedit_attrlist.clear ();

            return;
        }
    }
}


static void
delete_all_ic_impl ()
{
    GtkIMContextSCIMImpl *it;

    it = _used_ic_impl_list;
    while (it != 0) {
        _used_ic_impl_list = it->next;
        delete it;
        it = _used_ic_impl_list;
    }

    it = _free_ic_impl_list;
    while (it != 0) {
        _free_ic_impl_list = it->next;
        delete it;
        it = _free_ic_impl_list;
    }
}

static GtkIMContextSCIM *
find_ic (int id)
{
    GtkIMContextSCIMImpl *rec = _used_ic_impl_list;
 
    while (rec != 0) {
        if (rec->parent && rec->parent->id == id)
            return rec->parent;
        rec = rec->next;
    }

    return 0;
}


/* Public functions */
GtkIMContext *
gtk_im_context_scim_new (void)
{
    SCIM_DEBUG_FRONTEND(1) << "gtk_im_context_scim_new...\n";

    GtkIMContextSCIM *result;
 
    result = GTK_IM_CONTEXT_SCIM (g_object_new (GTK_TYPE_IM_CONTEXT_SCIM, NULL));

    return GTK_IM_CONTEXT (result);
}

void
gtk_im_context_scim_register_type (GTypeModule *type_module)
{
    static const GTypeInfo im_context_scim_info =
    {
        sizeof               (GtkIMContextSCIMClass),
        (GBaseInitFunc)       NULL,
        (GBaseFinalizeFunc)   NULL,
        (GClassInitFunc)      gtk_im_context_scim_class_init,
        NULL,                 /* class_finalize */
        NULL,                 /* class_data */
        sizeof               (GtkIMContextSCIM),
        0,
        (GInstanceInitFunc)  gtk_im_context_scim_init,
    };

    SCIM_DEBUG_FRONTEND(1) << "gtk_im_context_scim_register_type...\n";

    if (!_gtk_type_im_context_scim) {
        _gtk_type_im_context_scim = 
            g_type_module_register_type (type_module,
                                     GTK_TYPE_IM_CONTEXT,
                                     "GtkIMContextSCIM",
                                     &im_context_scim_info,
                                     (GTypeFlags) 0);
        g_type_module_use (type_module);
    }
}

void
gtk_im_context_scim_shutdown (void)
{
    SCIM_DEBUG_FRONTEND(1) << "gtk_im_context_scim_shutdown...\n";

    if (_scim_initialized) {
        SCIM_DEBUG_FRONTEND(1) << "gtk_im_context_scim_shutdown: call finalize ()...\n";
        finalize ();
        _scim_initialized = false;
    }
}

/* Private functions */
static void
gtk_im_context_scim_class_init (GtkIMContextSCIMClass *klass,
                                gpointer              *klass_data)
{
    SCIM_DEBUG_FRONTEND(1) << "gtk_im_context_scim_class_init...\n";

    GtkIMContextClass *im_context_klass = GTK_IM_CONTEXT_CLASS (klass);
    GObjectClass      *gobject_klass    = G_OBJECT_CLASS (klass);

    _parent_klass = (GObjectClass *) g_type_class_peek_parent (klass);

    im_context_klass->set_client_window   = gtk_im_context_scim_set_client_window;
    im_context_klass->filter_keypress     = gtk_im_context_scim_filter_keypress;
    im_context_klass->reset               = gtk_im_context_scim_reset;
    im_context_klass->get_preedit_string  = gtk_im_context_scim_get_preedit_string;
    im_context_klass->focus_in            = gtk_im_context_scim_focus_in;
    im_context_klass->focus_out           = gtk_im_context_scim_focus_out;
    im_context_klass->set_cursor_location = gtk_im_context_scim_set_cursor_location;
    im_context_klass->set_use_preedit     = gtk_im_context_scim_set_use_preedit;
    gobject_klass->finalize               = gtk_im_context_scim_finalize;

    if (!_scim_initialized) {
        initialize ();
        _scim_initialized = true;
    }
}

static void
gtk_im_context_scim_init (GtkIMContextSCIM      *context_scim,
                          GtkIMContextSCIMClass *klass)
{
    SCIM_DEBUG_FRONTEND(1) << "gtk_im_context_scim_init...\n";

    context_scim->impl = NULL;

    /* slave exists for using gtk+'s table based input method */
    context_scim->slave_preedit = false;
    context_scim->slave = gtk_im_context_simple_new ();
    g_signal_connect(G_OBJECT(context_scim->slave),
                     "commit",
                     G_CALLBACK(gtk_im_slave_commit_cb),
                     context_scim);
    
    g_signal_connect(G_OBJECT(context_scim->slave),
                     "preedit-changed",
                     G_CALLBACK(gtk_im_slave_preedit_changed_cb),
                     context_scim);
    
    g_signal_connect(G_OBJECT(context_scim->slave),
                     "preedit-start",
                     G_CALLBACK(gtk_im_slave_preedit_start_cb),
                     context_scim);
    
    g_signal_connect(G_OBJECT(context_scim->slave),
                     "preedit-end",
                     G_CALLBACK(gtk_im_slave_preedit_end_cb),
                     context_scim);




    if (_backend.null ()) return;

    IMEngineInstancePointer si;

    // Use the default instance if "shared input method" mode is enabled.
    if (_shared_input_method && !_default_instance.null ()) {
        si = _default_instance;
        SCIM_DEBUG_FRONTEND(2) << "use default instance: " << si->get_id () << " " << si->get_factory_uuid () << "\n";
    }

    // Not in "shared input method" mode, or no default instance, create an instance.
    if (si.null ()) {
        IMEngineFactoryPointer factory = _backend->get_default_factory (_language, "UTF-8");
        if (factory.null ()) return;
        si = factory->create_instance ("UTF-8", _instance_count++);
        if (si.null ()) return;
        attach_instance (si);
        SCIM_DEBUG_FRONTEND(2) << "create new instance: " << si->get_id () << " " << si->get_factory_uuid () << "\n";
    }

    // If "shared input method" mode is enabled, and there is no default instance,
    // then store this instance as default one.
    if (_shared_input_method && _default_instance.null ()) {
        SCIM_DEBUG_FRONTEND(2) << "update default instance.\n";
        _default_instance = si;
    }

    context_scim->impl = new_ic_impl (context_scim);

    context_scim->impl->si = si;
    context_scim->impl->client_window = NULL;
    context_scim->impl->preedit_caret = 0;
    context_scim->impl->cursor_x = 0;
    context_scim->impl->cursor_y = 0;
    context_scim->impl->is_on = FALSE;
    context_scim->impl->shared_si = _shared_input_method;
    context_scim->impl->use_preedit = _on_the_spot;
    context_scim->impl->preedit_started = false;
    context_scim->impl->preedit_updating = false;

    context_scim->id = _context_count++;

    if (_shared_input_method)
        context_scim->impl->is_on = _config->read (String (SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), context_scim->impl->is_on);

    _panel_client.prepare (context_scim->id);
    _panel_client.register_input_context (context_scim->id, si->get_factory_uuid ());
    set_ic_capabilities (context_scim);
    _panel_client.send ();

    SCIM_DEBUG_FRONTEND(2) << "input context created: id = " << context_scim->id << "\n";
}

static void
gtk_im_context_scim_finalize_partial (GtkIMContextSCIM *context_scim)
{
    SCIM_DEBUG_FRONTEND(1) << "gtk_im_context_scim_finalize_partial id=" << context_scim->id << "\n";

    if (context_scim->impl) {
        _panel_client.prepare (context_scim->id);

        if (context_scim == _focused_ic)
            context_scim->impl->si->focus_out ();
 
        // Delete the instance.
        // FIXME:
        // In case the instance send out some helper event,
        // and this context has been focused out,
        // we need set the focused_ic to this context temporary.
        GtkIMContextSCIM *old_focused = _focused_ic;
        _focused_ic = context_scim;
        context_scim->impl->si.reset ();
        _focused_ic = old_focused;

        if (context_scim == _focused_ic) {
            _panel_client.turn_off (context_scim->id);
            _panel_client.focus_out (context_scim->id);
        }

        _panel_client.remove_input_context (context_scim->id);
        _panel_client.send ();

        if (context_scim->impl->client_window)
            g_object_unref (context_scim->impl->client_window);

        delete_ic_impl (context_scim->impl);

        context_scim->impl = 0;
    }

    if (context_scim == _focused_ic)
        _focused_ic = 0;
}

static void
gtk_im_context_scim_finalize (GObject *obj)
{
    GtkIMContextSCIM *context_scim = GTK_IM_CONTEXT_SCIM (obj);

    SCIM_DEBUG_FRONTEND(1) << "gtk_im_context_scim_finalize id=" << context_scim->id << "\n";

    g_signal_handlers_disconnect_by_func(context_scim->slave,
                                         (void *)gtk_im_slave_commit_cb,
                                         (void *)context_scim);
    g_signal_handlers_disconnect_by_func(context_scim->slave,
                                         (void *)gtk_im_slave_preedit_changed_cb,
                                         (void *)context_scim);
    g_signal_handlers_disconnect_by_func(context_scim->slave,
                                         (void *)gtk_im_slave_preedit_start_cb,
                                         (void *)context_scim);
    g_signal_handlers_disconnect_by_func(context_scim->slave,
                                         (void *)gtk_im_slave_preedit_end_cb,
                                         (void *)context_scim);
    g_object_unref(context_scim->slave);

    gtk_im_context_scim_finalize_partial (context_scim);

    _parent_klass->finalize (obj);
}

static void
gtk_im_context_scim_set_client_window (GtkIMContext *context,
                                       GdkWindow    *client_window)
{
    SCIM_DEBUG_FRONTEND(1) << "gtk_im_context_scim_set_client_window...\n";

    GtkIMContextSCIM *context_scim = GTK_IM_CONTEXT_SCIM (context);

    if (context_scim && context_scim->impl) {
        if (client_window)
            g_object_ref (client_window);

        if (context_scim->impl->client_window)
            g_object_unref (context_scim->impl->client_window);

        context_scim->impl->client_window = client_window;
    }
}

static gboolean
gtk_im_context_scim_filter_keypress (GtkIMContext *context,
                                     GdkEventKey  *event)
{
    SCIM_DEBUG_FRONTEND(1) << "gtk_im_context_scim_filter_keypress...\n";

    GtkIMContextSCIM *context_scim = GTK_IM_CONTEXT_SCIM (context);

    gboolean ret = FALSE;

    if (context_scim) {
        if (!_snooper_installed)
            ret = gtk_scim_key_snooper (0, event, 0);

        if (context_scim->slave) {
            if (!ret ) {
                ret = gtk_im_context_filter_keypress (context_scim->slave, event);
            }
            else if (context_scim->slave_preedit) {
                context_scim->slave_preedit = false;
                gtk_im_context_reset (context_scim->slave);
            }
        }
    }

    return ret;
}

static void
gtk_im_context_scim_reset (GtkIMContext *context)
{
    SCIM_DEBUG_FRONTEND(1) << "gtk_im_context_scim_reset...\n";

    GtkIMContextSCIM *context_scim = GTK_IM_CONTEXT_SCIM (context);

    if (context_scim && context_scim->impl && context_scim == _focused_ic) {
        _panel_client.prepare (context_scim->id);
        context_scim->impl->si->reset ();
        _panel_client.send ();
    }
}

static void
gtk_im_context_scim_focus_in (GtkIMContext *context)
{
    GtkIMContextSCIM *context_scim = GTK_IM_CONTEXT_SCIM (context);

    SCIM_DEBUG_FRONTEND(1) << "gtk_im_context_scim_focus_in(" << context_scim->id << ")...\n";

    if (_focused_ic) {
        if (_focused_ic == context_scim) {
            SCIM_DEBUG_FRONTEND(1) << "It's already focused.\n";
            return;
        }
        SCIM_DEBUG_FRONTEND(1) << "Focus out previous IC first: " << _focused_ic->id << "\n";
        gtk_im_context_scim_focus_out (GTK_IM_CONTEXT (_focused_ic));
    }

    // Only use key snooper when use_key_snooper option is enabled and a gtk main loop is running.
    if (_use_key_snooper && !_snooper_installed && gtk_main_level () > 0) {
        SCIM_DEBUG_FRONTEND(2) << "Install key snooper.\n";
        _snooper_id = gtk_key_snooper_install ((GtkKeySnoopFunc)gtk_scim_key_snooper, NULL);
        _snooper_installed = true;
    }

    bool need_cap = false;
    bool need_reset = false;
    bool need_reg = false;

    if (context_scim && context_scim->impl) {
        _focused_ic = context_scim;
        _panel_client.prepare (context_scim->id);

        // Handle the "Shared Input Method" mode.
        if (_shared_input_method) {
            SCIM_DEBUG_FRONTEND(2) << "shared input method.\n";
            IMEngineFactoryPointer factory = _backend->get_default_factory (_language, "UTF-8");
            if (!factory.null ()) {
                if (_default_instance.null () || _default_instance->get_factory_uuid () != factory->get_uuid ()) {
                    _default_instance = factory->create_instance ("UTF-8", _default_instance.null () ? _instance_count++ : _default_instance->get_id ());
                    attach_instance (_default_instance);
                    SCIM_DEBUG_FRONTEND(2) << "create new default instance: " << _default_instance->get_id () << " " << _default_instance->get_factory_uuid () << "\n";
                }

                context_scim->impl->shared_si = true;
                context_scim->impl->si = _default_instance;

                context_scim->impl->is_on = _config->read (String (SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), context_scim->impl->is_on);
                context_scim->impl->preedit_string.clear ();
                context_scim->impl->preedit_attrlist.clear ();
                context_scim->impl->preedit_caret = 0;
                context_scim->impl->preedit_started = false;
                need_cap = true;
                need_reset = true;
                need_reg = true;
            }
        } else if (context_scim->impl->shared_si) {
            SCIM_DEBUG_FRONTEND(2) << "exit shared input method.\n";
            IMEngineFactoryPointer factory = _backend->get_default_factory (_language, "UTF-8");
            if (!factory.null ()) {
                context_scim->impl->si = factory->create_instance ("UTF-8", _instance_count++);
                context_scim->impl->preedit_string.clear ();
                context_scim->impl->preedit_attrlist.clear ();
                context_scim->impl->preedit_caret = 0;
                context_scim->impl->preedit_started = false;
                attach_instance (context_scim->impl->si);
                need_cap = true;
                need_reg = true;
                context_scim->impl->shared_si = false;
                SCIM_DEBUG_FRONTEND(2) << "create new instance: " << context_scim->impl->si->get_id () << " " << context_scim->impl->si->get_factory_uuid () << "\n";
            }
        }

        context_scim->impl->si->set_frontend_data (static_cast <void*> (context_scim));

        if (need_reg) _panel_client.register_input_context (context_scim->id, context_scim->impl->si->get_factory_uuid ());
        if (need_cap) set_ic_capabilities (context_scim);
        if (need_reset) context_scim->impl->si->reset ();

        panel_req_focus_in (context_scim);
        panel_req_update_screen (context_scim);
        panel_req_update_spot_location (context_scim);
        panel_req_update_factory_info (context_scim);

        if (context_scim->impl->is_on) {
            _panel_client.turn_on (context_scim->id);
            _panel_client.hide_preedit_string (context_scim->id);
            _panel_client.hide_aux_string (context_scim->id);
            _panel_client.hide_lookup_table (context_scim->id);
            context_scim->impl->si->focus_in ();
        } else {
            _panel_client.turn_off (context_scim->id);
        }

        _panel_client.send ();
    }
}

static void
gtk_im_context_scim_focus_out (GtkIMContext *context)
{
    GtkIMContextSCIM *context_scim = GTK_IM_CONTEXT_SCIM (context);

    SCIM_DEBUG_FRONTEND(1) << "gtk_im_context_scim_focus_out(" << context_scim->id << ")...\n";

    if (context_scim && context_scim->impl && context_scim == _focused_ic) {

        if (_snooper_installed) {
            SCIM_DEBUG_FRONTEND(2) << "Remove key snooper.\n";
            gtk_key_snooper_remove (_snooper_id);
            _snooper_installed = false;
        }

        _panel_client.prepare (context_scim->id);
        context_scim->impl->si->focus_out ();
        if (context_scim->impl->shared_si) context_scim->impl->si->reset ();
        _panel_client.turn_off (context_scim->id);
        _panel_client.focus_out (context_scim->id);
        _panel_client.send ();
        _focused_ic = 0;
    }
}

static void
gtk_im_context_scim_set_cursor_location (GtkIMContext *context,
                                         GdkRectangle *area)
{
    SCIM_DEBUG_FRONTEND(4) << "gtk_im_context_scim_set_cursor_location...\n";

    gint x, y;
    GtkIMContextSCIM *context_scim = GTK_IM_CONTEXT_SCIM (context);
 
    if (context_scim && context_scim->impl && context_scim->impl->client_window && context_scim == _focused_ic) {
        // Don't update spot location while updating preedit string.
        if (context_scim->impl->preedit_updating)
            return;

        gdk_window_get_origin(context_scim->impl->client_window, &x, &y);
        if (context_scim->impl->cursor_x != x + area->x + area->width ||
            context_scim->impl->cursor_y != y + area->y + area->height + 8) {
            context_scim->impl->cursor_x = x + area->x + area->width;
            context_scim->impl->cursor_y = y + area->y + area->height + 8;
            _panel_client.prepare (context_scim->id);
            panel_req_update_spot_location (context_scim);
            _panel_client.send ();
            SCIM_DEBUG_FRONTEND(2) << "new cursor location = " << context_scim->impl->cursor_x << "," << context_scim->impl->cursor_y << "\n";
        } 
    }
}

static void
gtk_im_context_scim_set_use_preedit (GtkIMContext *context,
                                     gboolean      use_preedit)
{
    SCIM_DEBUG_FRONTEND(1) << "gtk_im_context_scim_set_use_preedit = " << (use_preedit ? "true" : "false") << "...\n";

    GtkIMContextSCIM *context_scim = GTK_IM_CONTEXT_SCIM (context);

    if (!_on_the_spot) return;

    if (context_scim && context_scim->impl) {
        bool old = context_scim->impl->use_preedit;
        context_scim->impl->use_preedit = use_preedit;
        if (context_scim == _focused_ic) {
            _panel_client.prepare (context_scim->id);

            if (old != use_preedit)
                set_ic_capabilities (context_scim);

            if (context_scim->impl->preedit_string.length ()) 
                slot_show_preedit_string (context_scim->impl->si);

            _panel_client.send ();
        }
    }
}

static void
gtk_im_context_scim_get_preedit_string (GtkIMContext   *context,
                                        gchar         **str,
                                        PangoAttrList **attrs,
                                        gint           *cursor_pos)
{
    SCIM_DEBUG_FRONTEND(1) << "gtk_im_context_scim_get_preedit_string...\n";

    GtkIMContextSCIM *context_scim = GTK_IM_CONTEXT_SCIM (context);
    
    if (context_scim->slave_preedit == true) {
        gtk_im_context_get_preedit_string (context_scim->slave, str, attrs, cursor_pos);
        return;
    }

    if (context_scim && context_scim->impl && context_scim->impl->is_on) {
        String mbs = utf8_wcstombs (context_scim->impl->preedit_string);

        if (str) {
            if (mbs.length ())
                *str = g_strdup (mbs.c_str ());
            else
                *str = g_strdup ("");
        }

        if (cursor_pos) {
            *cursor_pos = context_scim->impl->preedit_caret;
        }

        if (attrs) {
            *attrs = pango_attr_list_new ();

            if (mbs.length ()) {
                guint start_index, end_index;
                guint wlen = context_scim->impl->preedit_string.length ();

                PangoAttribute *attr;
                AttributeList::const_iterator i;
                bool underline = false;
                bool *attrs_flag = new bool [mbs.length ()];

                memset (attrs_flag, 0, mbs.length () * sizeof (bool));

                for (i = context_scim->impl->preedit_attrlist.begin ();
                     i != context_scim->impl->preedit_attrlist.end (); ++i) {
                    start_index = i->get_start ();
                    end_index = i->get_end ();

                    if (end_index <= wlen && start_index < end_index && i->get_type () != SCIM_ATTR_DECORATE_NONE) {
                        start_index = g_utf8_offset_to_pointer (mbs.c_str (), i->get_start ()) - mbs.c_str ();
                        end_index = g_utf8_offset_to_pointer (mbs.c_str (), i->get_end ()) - mbs.c_str ();

                        if (i->get_type () == SCIM_ATTR_DECORATE) {
                            if (i->get_value () == scim::SCIM_ATTR_DECORATE_UNDERLINE) {
                                attr = pango_attr_underline_new (PANGO_UNDERLINE_SINGLE);
                                attr->start_index = start_index;
                                attr->end_index = end_index;
                                pango_attr_list_insert (*attrs, attr);
                                underline = true;
                            } else if (i->get_value () == SCIM_ATTR_DECORATE_REVERSE) {
                                attr = pango_attr_foreground_new (_normal_bg.red, _normal_bg.green, _normal_bg.blue);
                                attr->start_index = start_index;
                                attr->end_index = end_index;
                                pango_attr_list_insert (*attrs, attr);
   
                                attr = pango_attr_background_new (_normal_text.red, _normal_text.green, _normal_text.blue);
                                attr->start_index = start_index;
                                attr->end_index = end_index;
                                pango_attr_list_insert (*attrs, attr);
                            } else if (i->get_value () == SCIM_ATTR_DECORATE_HIGHLIGHT) {
                                attr = pango_attr_foreground_new (_active_text.red, _active_text.green, _active_text.blue);
                                attr->start_index = start_index;
                                attr->end_index = end_index;
                                pango_attr_list_insert (*attrs, attr);
   
                                attr = pango_attr_background_new (_active_bg.red, _active_bg.green, _active_bg.blue);
                                attr->start_index = start_index;
                                attr->end_index = end_index;
                                pango_attr_list_insert (*attrs, attr);
                            }
                        } else if (i->get_type () == SCIM_ATTR_FOREGROUND) {
                            unsigned int color = i->get_value ();
   
                            attr = pango_attr_foreground_new (SCIM_RGB_COLOR_RED(color) * 256, SCIM_RGB_COLOR_GREEN(color) * 256, SCIM_RGB_COLOR_BLUE(color) * 256);
                            attr->start_index = start_index;
                            attr->end_index = end_index;
                            pango_attr_list_insert (*attrs, attr);
                        } else if (i->get_type () == SCIM_ATTR_BACKGROUND) {
                            unsigned int color = i->get_value ();
   
                            attr = pango_attr_background_new (SCIM_RGB_COLOR_RED(color) * 256, SCIM_RGB_COLOR_GREEN(color) * 256, SCIM_RGB_COLOR_BLUE(color) * 256);
                            attr->start_index = start_index;
                            attr->end_index = end_index;
                            pango_attr_list_insert (*attrs, attr);
                        }

                        // Record which character has attribute.
                        for (guint pos = start_index; pos < end_index; ++pos)
                            attrs_flag [pos] = 1;
                    }
                }

                // Add underline for all characters which don't have attribute.
                for (guint pos = 0; pos < mbs.length (); ++pos) {
                    if (!attrs_flag [pos]) {
                        guint begin_pos = pos;

                        for (; pos < mbs.length () && !attrs_flag [pos]; ++pos)
                            NULL;

                        attr = pango_attr_underline_new (PANGO_UNDERLINE_SINGLE);
                        attr->start_index = begin_pos;
                        attr->end_index = pos;
                        pango_attr_list_insert (*attrs, attr);
                    }
                }
            }
        }
    } else {
        if (str)
            *str = g_strdup ("");

        if (cursor_pos)
            *cursor_pos = 0;

        if (attrs)
            *attrs = pango_attr_list_new ();
    }
}

static gboolean
gtk_scim_key_snooper (GtkWidget    *grab_widget,
                      GdkEventKey  *event,
                      gpointer      data)
{
    SCIM_DEBUG_FRONTEND(3) << "gtk_scim_key_snooper...\n";

    gboolean ret = FALSE;

    if (_focused_ic && _focused_ic->impl && (event->type == GDK_KEY_PRESS || event->type == GDK_KEY_RELEASE) && !(event->send_event & SEND_EVENT_MASK)) {
        _focused_widget = grab_widget;

        KeyEvent key = keyevent_gdk_to_scim (_focused_ic, *event);

        key.mask &= _valid_key_mask;

        key.layout = _keyboard_layout;

        _panel_client.prepare (_focused_ic->id);

        if (!filter_hotkeys (_focused_ic, key)) {
            if (!_focused_ic->impl->is_on || !_focused_ic->impl->si->process_key_event (key)) {
                SCIM_DEBUG_FRONTEND(3) << "Failed to process: "
                                       << (!_focused_ic->impl->is_on ? "IC is off" : "IC doesn't process")
                                       << ".\n";
                ret = _fallback_instance->process_key_event (key);
            } else {
                ret = TRUE;
            }
        } else {
            ret = TRUE;
        }

        _panel_client.send ();

        _focused_widget = 0;
    } else {
        SCIM_DEBUG_FRONTEND(3) << "Failed snooper: "
                               << ((!_focused_ic || !_focused_ic->impl) ? "Invalid focused ic" :
                                   ((event->send_event & SEND_EVENT_MASK) ? "send event is set" : "unknown"))
                               << "\n";
    }

    return ret;
}

static void
gtk_im_slave_commit_cb (GtkIMContext     *context,
                        const char       *str,
                        GtkIMContextSCIM *context_scim)
{
    g_return_if_fail(str);
    g_signal_emit_by_name(context_scim, "commit", str);
}

static void
gtk_im_slave_preedit_changed_cb (GtkIMContext     *context,
                        GtkIMContextSCIM *context_scim)
{
    context_scim->slave_preedit = true;
    g_signal_emit_by_name(context_scim, "preedit-changed");
}

static void
gtk_im_slave_preedit_start_cb (GtkIMContext     *context,
                        GtkIMContextSCIM *context_scim)
{
    context_scim->slave_preedit = true;
    g_signal_emit_by_name(context_scim, "preedit-start");
}

static void
gtk_im_slave_preedit_end_cb (GtkIMContext     *context,
                        GtkIMContextSCIM *context_scim)
{
    context_scim->slave_preedit = false;
    g_signal_emit_by_name(context_scim, "preedit-end");
}

/* Panel Slot functions */
static void
panel_slot_reload_config (int context)
{
    SCIM_DEBUG_FRONTEND(1) << "panel_slot_reload_config...\n";
    _config->reload ();
}

static void
panel_slot_exit (int /* context */)
{
    SCIM_DEBUG_FRONTEND(1) << "panel_slot_exit...\n";

    finalize ();
}

static void
panel_slot_update_lookup_table_page_size (int context, int page_size)
{
    GtkIMContextSCIM *ic = find_ic (context);
    SCIM_DEBUG_FRONTEND(1) << "panel_slot_update_lookup_table_page_size context=" << context << " page_size=" << page_size << " ic=" << ic << "\n";
    if (ic && ic->impl) {
        _panel_client.prepare (ic->id);
        ic->impl->si->update_lookup_table_page_size (page_size);
        _panel_client.send ();
    }
}

static void
panel_slot_lookup_table_page_up (int context)
{
    GtkIMContextSCIM *ic = find_ic (context);
    SCIM_DEBUG_FRONTEND(1) << "panel_slot_lookup_table_page_up context=" << context << " ic=" << ic << "\n";
    if (ic && ic->impl) {
        _panel_client.prepare (ic->id);
        ic->impl->si->lookup_table_page_up ();
        _panel_client.send ();
    }
}

static void
panel_slot_lookup_table_page_down (int context)
{
    GtkIMContextSCIM *ic = find_ic (context);
    SCIM_DEBUG_FRONTEND(1) << "panel_slot_lookup_table_page_down context=" << context << " ic=" << ic << "\n";
    if (ic && ic->impl) {
        _panel_client.prepare (ic->id);
        ic->impl->si->lookup_table_page_down ();
        _panel_client.send ();
    }
}

static void
panel_slot_trigger_property (int context, const String &property)
{
    GtkIMContextSCIM *ic = find_ic (context);
    SCIM_DEBUG_FRONTEND(1) << "panel_slot_trigger_property context=" << context << " property=" << property << " ic=" << ic << "\n";
    if (ic && ic->impl) {
        _panel_client.prepare (ic->id);
        ic->impl->si->trigger_property (property);
        _panel_client.send ();
    }
}

static void
panel_slot_process_helper_event (int context, const String &target_uuid, const String &helper_uuid, const Transaction &trans)
{
    GtkIMContextSCIM *ic = find_ic (context);
    SCIM_DEBUG_FRONTEND(1) << "panel_slot_process_helper_event context=" << context << " target=" << target_uuid
                           << " helper=" << helper_uuid << " ic=" << ic << " ic->impl=" << (ic ? ic->impl : 0) << " ic-uuid="
                           << ((ic && ic->impl) ? ic->impl->si->get_factory_uuid () : "" ) << "\n";
    if (ic && ic->impl && ic->impl->si->get_factory_uuid () == target_uuid) {
        _panel_client.prepare (ic->id);
        SCIM_DEBUG_FRONTEND(2) << "call process_helper_event\n";
        ic->impl->si->process_helper_event (helper_uuid, trans);
        _panel_client.send ();
    }
}

static void
panel_slot_move_preedit_caret (int context, int caret_pos)
{
    GtkIMContextSCIM *ic = find_ic (context);
    SCIM_DEBUG_FRONTEND(1) << "panel_slot_move_preedit_caret context=" << context << " caret=" << caret_pos << " ic=" << ic << "\n";
    if (ic && ic->impl) {
        _panel_client.prepare (ic->id);
        ic->impl->si->move_preedit_caret (caret_pos);
        _panel_client.send ();
    }
}

static void
panel_slot_select_candidate (int context, int cand_index)
{
    GtkIMContextSCIM *ic = find_ic (context);
    SCIM_DEBUG_FRONTEND(1) << "panel_slot_select_candidate context=" << context << " candidate=" << cand_index << " ic=" << ic << "\n";
    if (ic && ic->impl) {
        _panel_client.prepare (ic->id);
        ic->impl->si->select_candidate (cand_index);
        _panel_client.send ();
    }
}

static void
panel_slot_process_key_event (int context, const KeyEvent &key)
{
    GtkIMContextSCIM *ic = find_ic (context);
    SCIM_DEBUG_FRONTEND(1) << "panel_slot_process_key_event context=" << context << " key=" << key.get_key_string () << " ic=" << ic << "\n";
    if (ic && ic->impl) {
        _panel_client.prepare (ic->id);

        if (!filter_hotkeys (ic, key)) {
            if (!_focused_ic || !_focused_ic->impl->is_on ||
                !_focused_ic->impl->si->process_key_event (key)) {
                if (!_fallback_instance->process_key_event (key)) {
                    // Just send it to key_snooper and bypass to client directly (because send_event is set to TRUE).
                    GdkEventKey gdkevent = keyevent_scim_to_gdk (ic, key);
                    gdk_event_put ((GdkEvent *)&gdkevent);
                }
            }
        }

        _panel_client.send ();
    }
}

static void
panel_slot_commit_string (int context, const WideString &wstr)
{
    GtkIMContextSCIM *ic = find_ic (context);
    SCIM_DEBUG_FRONTEND(1) << "panel_slot_commit_string context=" << context << " str=" << utf8_wcstombs (wstr) << " ic=" << ic << "\n";
    if (ic && ic->impl)
        g_signal_emit_by_name (ic, "commit", utf8_wcstombs (wstr).c_str ());
}

static void
panel_slot_forward_key_event (int context, const KeyEvent &key)
{
    GtkIMContextSCIM *ic = find_ic (context);
    SCIM_DEBUG_FRONTEND(1) << "panel_slot_forward_key_event context=" << context << " key=" << key.get_key_string () << " ic=" << ic << "\n";
    if (ic && ic->impl) {
        // Just send it to key_snooper and bypass to client directly (because send_event is set to TRUE).
        GdkEventKey gdkevent = keyevent_scim_to_gdk (ic, key);
        gdk_event_put ((GdkEvent *)&gdkevent);
    }
}

static void
panel_slot_request_help (int context)
{
    GtkIMContextSCIM *ic = find_ic (context);
    SCIM_DEBUG_FRONTEND(1) << "panel_slot_request_help context=" << context << " ic=" << ic << "\n";
    if (ic && ic->impl) {
        _panel_client.prepare (ic->id);
        panel_req_show_help (ic);
        _panel_client.send ();
    }
}

static void
panel_slot_request_factory_menu (int context)
{
    GtkIMContextSCIM *ic = find_ic (context);
    SCIM_DEBUG_FRONTEND(1) << "panel_slot_request_factory_menu context=" << context << " ic=" << ic << "\n";
    if (ic && ic->impl) {
        _panel_client.prepare (ic->id);
        panel_req_show_factory_menu (ic);
        _panel_client.send ();
    }
}

static void
panel_slot_change_factory (int context, const String &uuid)
{
    GtkIMContextSCIM *ic = find_ic (context);
    SCIM_DEBUG_FRONTEND(1) << "panel_slot_change_factory context=" << context << " factory=" << uuid << " ic=" << ic << "\n";
    if (ic && ic->impl) {
        _panel_client.prepare (ic->id);
        open_specific_factory (ic, uuid);
        _panel_client.send ();
    }
}


/* Panel Requestion functions. */
static void
panel_req_update_screen (GtkIMContextSCIM *ic)
{
#if GTK_CHECK_VERSION(2, 2, 0)
    if (ic->impl->client_window) {
#if GTK_CHECK_VERSION(2, 24, 0)
        GdkScreen *screen = gdk_window_get_screen (ic->impl->client_window);
#else
        GdkScreen *screen = gdk_drawable_get_screen (GDK_DRAWABLE (ic->impl->client_window));
#endif
        if (screen) {
            int number = gdk_screen_get_number (screen);
            _panel_client.update_screen (ic->id, number);
        }
    }
#endif
}

static void
panel_req_show_help (GtkIMContextSCIM *ic)
{
    String help;

    help =  String (_("Smart Common Input Method platform ")) +
            String (SCIM_VERSION) +
            String (_("\n(C) 2002-2005 James Su <suzhe@tsinghua.org.cn>\n\n"));

    if (ic && ic->impl) {
        IMEngineFactoryPointer sf = _backend->get_factory (ic->impl->si->get_factory_uuid ());
        help += utf8_wcstombs (sf->get_name ());
        help += String (_(":\n\n"));

        help += utf8_wcstombs (sf->get_authors ());
        help += String (_("\n\n"));

        help += utf8_wcstombs (sf->get_help ());
        help += String (_("\n\n"));

        help += utf8_wcstombs (sf->get_credits ());
    }

    _panel_client.show_help (ic->id, help);
}

static void
panel_req_show_factory_menu (GtkIMContextSCIM *ic)
{
    std::vector<IMEngineFactoryPointer> factories;
    std::vector <PanelFactoryInfo> menu;

    _backend->get_factories_for_encoding (factories, "UTF-8");

    for (size_t i = 0; i < factories.size (); ++ i) {
        menu.push_back (PanelFactoryInfo (
                                    factories [i]->get_uuid (),
                                    utf8_wcstombs (factories [i]->get_name ()),
                                    factories [i]->get_language (),
                                    factories [i]->get_icon_file ()));
    }

    if (menu.size ())
        _panel_client.show_factory_menu (ic->id, menu);
}

static void
panel_req_update_factory_info (GtkIMContextSCIM *ic)
{
    if (ic && ic->impl && ic == _focused_ic) {
        PanelFactoryInfo info;
        if (ic->impl->is_on) {
            IMEngineFactoryPointer sf = _backend->get_factory (ic->impl->si->get_factory_uuid ());
            info = PanelFactoryInfo (sf->get_uuid (), utf8_wcstombs (sf->get_name ()), sf->get_language (), sf->get_icon_file ());
        } else {
            info = PanelFactoryInfo (String (""), String (_("English/Keyboard")), String ("C"), String (SCIM_KEYBOARD_ICON_FILE));
        }
        _panel_client.update_factory_info (ic->id, info);
    }
}

static void
panel_req_focus_in (GtkIMContextSCIM *ic)
{
    _panel_client.focus_in (ic->id, ic->impl->si->get_factory_uuid ());
}

static void
panel_req_update_spot_location (GtkIMContextSCIM *ic)
{
    _panel_client.update_spot_location (ic->id, ic->impl->cursor_x, ic->impl->cursor_y);
}


static bool
filter_hotkeys (GtkIMContextSCIM *ic, const KeyEvent &key)
{
    bool ret = false;

    _frontend_hotkey_matcher.push_key_event (key);
    _imengine_hotkey_matcher.push_key_event (key);

    FrontEndHotkeyAction hotkey_action = _frontend_hotkey_matcher.get_match_result ();

    if (hotkey_action == SCIM_FRONTEND_HOTKEY_TRIGGER) {
        if (!ic->impl->is_on)
            turn_on_ic (ic);
        else
            turn_off_ic (ic);
        ret = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_ON) {
        if (!ic->impl->is_on)
            turn_on_ic (ic);
        ret = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_OFF) {
        if (ic->impl->is_on)
            turn_off_ic (ic);
        ret = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_NEXT_FACTORY) {
        open_next_factory (ic);
        ret = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_PREVIOUS_FACTORY) {
        open_previous_factory (ic);
        ret = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_SHOW_FACTORY_MENU) {
        panel_req_show_factory_menu (ic);
        ret = true;
    } else if (_imengine_hotkey_matcher.is_matched ()) {
        String sfid = _imengine_hotkey_matcher.get_match_result ();
        open_specific_factory (ic, sfid);
        ret = true;
    } 
    return ret;
}

static bool
panel_initialize ()
{
    String display_name;

    {
#if GTK_CHECK_VERSION(2, 2, 0)
        const char *p = gdk_display_get_name (gdk_display_get_default ());
#else
        const char *p = getenv ("DISPLAY");
#endif
        if (p) display_name = String (p);
    }

    SCIM_DEBUG_FRONTEND(1) << "panel_initialize..\n";

    if (_panel_client.open_connection (_config->get_name (), display_name) >= 0) {
        int fd = _panel_client.get_connection_number ();
        _panel_iochannel = g_io_channel_unix_new (fd);

        _panel_iochannel_read_source = g_io_add_watch (_panel_iochannel, G_IO_IN,  panel_iochannel_handler, 0);
        _panel_iochannel_err_source  = g_io_add_watch (_panel_iochannel, G_IO_ERR, panel_iochannel_handler, 0);
        _panel_iochannel_hup_source  = g_io_add_watch (_panel_iochannel, G_IO_HUP, panel_iochannel_handler, 0);

        SCIM_DEBUG_FRONTEND(2) << " Panel FD= " << fd << "\n";

        return true;
    }
    return false;
}

static void
panel_finalize ()
{
    _panel_client.close_connection ();

    if (_panel_iochannel) {
        g_io_channel_unref (_panel_iochannel);
        g_source_remove (_panel_iochannel_read_source);
        g_source_remove (_panel_iochannel_err_source);
        g_source_remove (_panel_iochannel_hup_source);
        _panel_iochannel = 0;
        _panel_iochannel_read_source = 0;
        _panel_iochannel_err_source = 0;
        _panel_iochannel_hup_source = 0;
    }
}

static gboolean
panel_iochannel_handler (GIOChannel *source, GIOCondition condition, gpointer user_data)
{
    if (condition == G_IO_IN) {
        if (!_panel_client.filter_event ()) {
            panel_finalize ();
            panel_initialize ();
            return FALSE;
        }
    } else if (condition == G_IO_ERR || condition == G_IO_HUP) {
        panel_finalize ();
        panel_initialize ();
        return FALSE;
    }
    return TRUE;
}

static void
turn_on_ic (GtkIMContextSCIM *ic)
{
    if (ic && ic->impl && !ic->impl->is_on) {
        ic->impl->is_on = true;

        if (ic == _focused_ic) {
            panel_req_focus_in (ic);
            panel_req_update_screen (ic);
            panel_req_update_spot_location (ic);
            panel_req_update_factory_info (ic);
            _panel_client.turn_on (ic->id);
            _panel_client.hide_preedit_string (ic->id);
            _panel_client.hide_aux_string (ic->id);
            _panel_client.hide_lookup_table (ic->id);
            ic->impl->si->focus_in ();
        }

        //Record the IC on/off status
        if (_shared_input_method)
            _config->write (String (SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), true);

        if (ic->impl->use_preedit && ic->impl->preedit_string.length ()) {
            g_signal_emit_by_name(ic, "preedit-start");
            g_signal_emit_by_name(ic, "preedit-changed");
            ic->impl->preedit_started = true;
        }
    }
}

static void
turn_off_ic (GtkIMContextSCIM *ic)
{
    if (ic && ic->impl && ic->impl->is_on) {
        ic->impl->is_on = false;

        if (ic == _focused_ic) {
            ic->impl->si->focus_out ();

            panel_req_update_factory_info (ic);
            _panel_client.turn_off (ic->id);
        }

        //Record the IC on/off status
        if (_shared_input_method)
            _config->write (String (SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), false);

        if (ic->impl->use_preedit && ic->impl->preedit_string.length ()) {
            g_signal_emit_by_name(ic, "preedit-changed");
            g_signal_emit_by_name(ic, "preedit-end");
            ic->impl->preedit_started = false;
        }
    }
}

static void
set_ic_capabilities (GtkIMContextSCIM *ic)
{
    if (ic && ic->impl) {
        unsigned int cap = SCIM_CLIENT_CAP_ALL_CAPABILITIES;

        if (!_on_the_spot || !ic->impl->use_preedit)
            cap -= SCIM_CLIENT_CAP_ONTHESPOT_PREEDIT;

        ic->impl->si->update_client_capabilities (cap);
    }
}

static KeyEvent
keyevent_gdk_to_scim (const GtkIMContextSCIM *ic, const GdkEventKey &gdkevent)
{
    KeyEvent key;

    // Use Key Symbole provided by gtk.
    key.code = gdkevent.keyval;

#ifdef GDK_WINDOWING_X11 
    Display *display = 0;

    if (ic && ic->impl && ic->impl->client_window)
        display = GDK_WINDOW_XDISPLAY (ic->impl->client_window);
    else
        display = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());

    key.mask = scim_x11_keymask_x11_to_scim (display, gdkevent.state);

    // Special treatment for two backslash keys on jp106 keyboard.
    if (key.code == SCIM_KEY_backslash) {
        int keysym_size = 0;
        KeySym *keysyms = XGetKeyboardMapping (display, gdkevent.hardware_keycode, 1, &keysym_size);
        if (keysyms != NULL) {
            if (keysyms[0] == XK_backslash &&
                (keysym_size > 1 && keysyms[1] == XK_underscore))
                key.mask |= SCIM_KEY_QuirkKanaRoMask;
            XFree (keysyms);
        }
    }
#else
    if (gdkevent.state & GDK_SHIFT_MASK) key.mask |=SCIM_KEY_ShiftMask;
    if (gdkevent.state & GDK_LOCK_MASK) key.mask |=SCIM_KEY_CapsLockMask;
    if (gdkevent.state & GDK_CONTROL_MASK) key.mask |=SCIM_KEY_ControlMask;
    if (gdkevent.state & GDK_MOD1_MASK) key.mask |=SCIM_KEY_AltMask;
    if (gdkevent.state & GDK_MOD2_MASK) key.mask |=SCIM_KEY_NumLockMask;
#endif

    if (gdkevent.type == GDK_KEY_RELEASE) key.mask |= SCIM_KEY_ReleaseMask;

    return key;
}

static GdkKeymap *
get_gdk_keymap (GdkWindow *window)
{
    GdkKeymap *keymap;

#if GTK_CHECK_VERSION(2, 2, 0)
    if (window)
#if GTK_CHECK_VERSION(2, 24, 0)
        keymap = gdk_keymap_get_for_display (gdk_window_get_display (window));
#else
        keymap = gdk_keymap_get_for_display (gdk_drawable_get_display (window));
#endif
    else
#endif
        keymap = gdk_keymap_get_default ();

    return keymap;
}

static guint32
get_time (void)
{
    int tint;
    struct timeval tv;
    struct timezone tz;           /* is not used since ages */
    gettimeofday (&tv, &tz); 
    tint = (int) tv.tv_sec * 1000;
    tint = tint / 1000 * 1000; 
    tint = tint + tv.tv_usec / 1000;
    return ((guint32) tint);
}

static GdkEventKey
keyevent_scim_to_gdk (const GtkIMContextSCIM *ic,
                      const KeyEvent   &scimkey)
{
    GdkEventKey gdkevent;

#ifdef GDK_WINDOWING_X11
    Display *display = 0;

    if (ic && ic->impl && ic->impl->client_window)
        display = GDK_WINDOW_XDISPLAY (ic->impl->client_window);
    else
        display = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());

    gdkevent.state = scim_x11_keymask_scim_to_x11 (display, scimkey.mask);
#else
    gdkevent.state = 0;
    if (scimkey.is_shift_down ()) gdkevent.state |= GDK_SHIFT_MASK;
    if (scimkey.is_caps_lock_down ()) gdkevent.state |= GDK_LOCK_MASK;
    if (scimkey.is_control_down ()) gdkevent.state |= GDK_CONTROL_MASK;
    if (scimkey.is_alt_down ()) gdkevent.state |= GDK_MOD1_MASK;
    if (scimkey.is_num_lock_down ()) gdkevent.state |= GDK_MOD2_MASK;
#endif

    if (scimkey.is_key_release ()) gdkevent.state |= GDK_RELEASE_MASK;

    gdkevent.type = (scimkey.is_key_release () ? GDK_KEY_RELEASE : GDK_KEY_PRESS);
    gdkevent.window = ((ic && ic->impl) ? ic->impl->client_window : 0);
    gdkevent.send_event |= SEND_EVENT_MASK;
    gdkevent.time = get_time ();
    gdkevent.keyval = scimkey.code;
    gdkevent.length = 0;
    gdkevent.string = 0;

    GdkKeymap *keymap = get_gdk_keymap (gdkevent.window);

    GdkKeymapKey *keys;
    gint          n_keys;

    if (gdk_keymap_get_entries_for_keyval (keymap, gdkevent.keyval, &keys, &n_keys)) {
        gdkevent.hardware_keycode = keys [0].keycode;
        gdkevent.group = keys [0].group;
    } else {
        gdkevent.hardware_keycode = 0;
        gdkevent.group = 0;
    }

    if (keys) g_free (keys);

    return gdkevent;
}

static bool
check_socket_frontend ()
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

static void
initialize (void)
{
    std::vector<String>     config_list;
    std::vector<String>     engine_list;
    std::vector<String>     load_engine_list;

    std::vector<String>     debug_mask_list;
    int                     debug_verbose = 0;

    size_t                  i;

    bool                    manual = false;

    bool                    socket = true;

    String                  config_module_name;

    // Get debug info
    {
        char *str;

        str = getenv ("GTK_IM_SCIM_DEBUG_VERBOSE");

        if (str != NULL)
            debug_verbose = atoi (str);

        DebugOutput::set_verbose_level (debug_verbose);

        str = getenv ("GTK_IM_SCIM_DEBUG_MASK");

        if (str != NULL) {
            scim_split_string_list (debug_mask_list, String (str), ',');
    		if (debug_mask_list.size ()) {
                DebugOutput::disable_debug (SCIM_DEBUG_AllMask);
		    	for (i=0; i<debug_mask_list.size (); i++)
			    	DebugOutput::enable_debug_by_name (debug_mask_list [i]);
    		}
        }
    }

    SCIM_DEBUG_FRONTEND(1) << "Initializing GTK SCIM IMModule...\n";

    // Get system language.
    _language = scim_get_locale_language (scim_get_current_locale ());

    //get modules list
    scim_get_imengine_module_list (engine_list);
    scim_get_config_module_list (config_list);

    if (std::find (engine_list.begin (), engine_list.end (), "socket") == engine_list.end () ||
        std::find (config_list.begin (), config_list.end (), "socket") == config_list.end ())
        socket = false;

    //Use simple config module as default if available.
    if (config_list.size ()) {
        config_module_name = scim_global_config_read (SCIM_GLOBAL_CONFIG_DEFAULT_CONFIG_MODULE, String ("simple"));
        if (std::find (config_list.begin (), config_list.end (), config_module_name) == config_list.end ())
            config_module_name = config_list [0];
    } else {
        config_module_name = "dummy";
    }

    const char *engine_list_str = getenv ("GTK_IM_SCIM_IMENGINE_MODULES");
    if (engine_list_str != NULL) {
        std::vector <String> spec_engine_list;
        scim_split_string_list (spec_engine_list, engine_list_str, ',');

        load_engine_list.clear ();

        for (size_t i = 0; i < spec_engine_list.size (); ++i)
            if (std::find (engine_list.begin (), engine_list.end (), spec_engine_list [i]) != engine_list.end ())
                load_engine_list.push_back (spec_engine_list [i]);

        manual = true;
    }

    // If you try to use the socket feature manually,
    // then let you do it by yourself.
    if (config_module_name == "socket" ||
        std::find (load_engine_list.begin (), load_engine_list.end (), "socket") != load_engine_list.end ())
        socket = false;

    // Try to start a SCIM SocketFrontEnd process if necessary.
    if (scim_get_default_socket_frontend_address () != scim_get_default_socket_imengine_address () &&
        scim_get_default_socket_frontend_address () != scim_get_default_socket_config_address ())
        socket = false;

    if (socket) {
        // If no Socket FrontEnd is running, then launch one.
        // And set manual to false.
        if (!check_socket_frontend ()) {
            std::cerr << "Launching a SCIM daemon with Socket FrontEnd...\n";
            char *new_argv [] = { const_cast<char*> ("--no-stay"), 0 };
            scim_launch (true,
                         config_module_name,
                         (load_engine_list.size () ? scim_combine_string_list (load_engine_list, ',') : "all"),
                         "socket",
                         new_argv);
            manual = false;
        }

        // If there is one Socket FrontEnd running and it's not manual mode,
        // then just use this Socket Frontend.
        if (!manual) {
            for (int i = 0; i < 100; ++i) {
                if (check_socket_frontend ()) {
                    config_module_name = "socket";
                    load_engine_list.clear ();
                    load_engine_list.push_back ("socket");
                    break;
                }
                scim_usleep (100000);
            }
        }
    }

    if (config_module_name != "dummy") {
        //load config module
        SCIM_DEBUG_FRONTEND(1) << "Loading Config module: " << config_module_name << "...\n";
        _config_module = new ConfigModule (config_module_name);

        //create config instance
        if (_config_module != NULL && _config_module->valid ())
            _config = _config_module->create_config ();
    }

    if (_config.null ()) {
        SCIM_DEBUG_FRONTEND(1) << "Config module cannot be loaded, using dummy Config.\n";

        if (_config_module) delete _config_module;
        _config_module = NULL;

        _config = new DummyConfig ();
        config_module_name = "dummy";
    }

    // Init colors.
    gdk_color_parse ("gray92",     &_normal_bg);
    gdk_color_parse ("black",      &_normal_text);
    gdk_color_parse ("light blue", &_active_bg);
    gdk_color_parse ("black",      &_active_text);

    reload_config_callback (_config); 
    _config->signal_connect_reload (slot (reload_config_callback));

    // create backend
    _backend = new CommonBackEnd (_config, load_engine_list.size () ? load_engine_list : engine_list);

    if (_backend.null ()) {
        fprintf (stderr, "GTK IM Module SCIM: Cannot create BackEnd Object!\n");
    } else {
        _fallback_factory = _backend->get_factory (SCIM_COMPOSE_KEY_FACTORY_UUID);
    }

    if (_fallback_factory.null ()) _fallback_factory = new DummyIMEngineFactory ();

    _fallback_instance = _fallback_factory->create_instance (String ("UTF-8"), 0);
    _fallback_instance->signal_connect_commit_string (slot (fallback_commit_string_cb));

    // Attach Panel Client signal.
    _panel_client.signal_connect_reload_config                 (slot (panel_slot_reload_config));
    _panel_client.signal_connect_exit                          (slot (panel_slot_exit));
    _panel_client.signal_connect_update_lookup_table_page_size (slot (panel_slot_update_lookup_table_page_size));
    _panel_client.signal_connect_lookup_table_page_up          (slot (panel_slot_lookup_table_page_up));
    _panel_client.signal_connect_lookup_table_page_down        (slot (panel_slot_lookup_table_page_down));
    _panel_client.signal_connect_trigger_property              (slot (panel_slot_trigger_property));
    _panel_client.signal_connect_process_helper_event          (slot (panel_slot_process_helper_event));
    _panel_client.signal_connect_move_preedit_caret            (slot (panel_slot_move_preedit_caret));
    _panel_client.signal_connect_select_candidate              (slot (panel_slot_select_candidate));
    _panel_client.signal_connect_process_key_event             (slot (panel_slot_process_key_event));
    _panel_client.signal_connect_commit_string                 (slot (panel_slot_commit_string));
    _panel_client.signal_connect_forward_key_event             (slot (panel_slot_forward_key_event));
    _panel_client.signal_connect_request_help                  (slot (panel_slot_request_help));
    _panel_client.signal_connect_request_factory_menu          (slot (panel_slot_request_factory_menu));
    _panel_client.signal_connect_change_factory                (slot (panel_slot_change_factory));

    if (!panel_initialize ()) {
        fprintf (stderr, "GTK IM Module SCIM: Cannot connect to Panel!\n");
    }
}

static void
finalize (void)
{
    SCIM_DEBUG_FRONTEND(1) << "Finalizing GTK SCIM IMModule...\n";

    if (_snooper_installed) {
        gtk_key_snooper_remove(_snooper_id);
        _snooper_installed=false;
        _snooper_id = 0;
    }

    // Reset this first so that the shared instance could be released correctly afterwards.
    _default_instance.reset ();

    SCIM_DEBUG_FRONTEND(2) << "Finalize all IC partially.\n";
    while (_used_ic_impl_list) {
        // In case in "shared input method" mode,
        // all contexts share only one instance,
        // so we need point the reference pointer correctly before finalizing.
        _used_ic_impl_list->si->set_frontend_data (static_cast <void*> (_used_ic_impl_list->parent));
        gtk_im_context_scim_finalize_partial (_used_ic_impl_list->parent);
    }

    delete_all_ic_impl ();

    _fallback_instance.reset ();
    _fallback_factory.reset ();

    SCIM_DEBUG_FRONTEND(2) << " Releasing BackEnd...\n";
    _backend.reset ();

    SCIM_DEBUG_FRONTEND(2) << " Releasing Config...\n";
    _config.reset ();

    if (_config_module) {
        SCIM_DEBUG_FRONTEND(2) << " Deleting _config_module...\n";
        delete _config_module;
        _config_module = 0;
    }

    _focused_ic = 0;
    _focused_widget = 0;

    _scim_initialized = false;

    panel_finalize ();
}

static void
open_next_factory (GtkIMContextSCIM *ic)
{
    SCIM_DEBUG_FRONTEND(2) << "open_next_factory context=" << ic->id << "\n";
    IMEngineFactoryPointer sf = _backend->get_next_factory ("", "UTF-8", ic->impl->si->get_factory_uuid ());

    if (!sf.null ()) {
        turn_off_ic (ic);
        ic->impl->si = sf->create_instance ("UTF-8", ic->impl->si->get_id ());
        ic->impl->si->set_frontend_data (static_cast <void*> (ic));
        ic->impl->preedit_string = WideString ();
        ic->impl->preedit_caret = 0;
        attach_instance (ic->impl->si);
        _backend->set_default_factory (_language, sf->get_uuid ());
        _panel_client.register_input_context (ic->id, sf->get_uuid ());
        set_ic_capabilities (ic);
        turn_on_ic (ic);

        if (_shared_input_method) {
            _default_instance = ic->impl->si;
            ic->impl->shared_si = true;
        }
    }
}

static void
open_previous_factory (GtkIMContextSCIM *ic)
{
    SCIM_DEBUG_FRONTEND(2) << "open_previous_factory context=" << ic->id << "\n";
    IMEngineFactoryPointer sf = _backend->get_previous_factory ("", "UTF-8", ic->impl->si->get_factory_uuid ());

    if (!sf.null ()) {
        turn_off_ic (ic);
        ic->impl->si = sf->create_instance ("UTF-8", ic->impl->si->get_id ());
        ic->impl->si->set_frontend_data (static_cast <void*> (ic));
        ic->impl->preedit_string = WideString ();
        ic->impl->preedit_caret = 0;
        attach_instance (ic->impl->si);
        _backend->set_default_factory (_language, sf->get_uuid ());
        _panel_client.register_input_context (ic->id, sf->get_uuid ());
        set_ic_capabilities (ic);
        turn_on_ic (ic);

        if (_shared_input_method) {
            _default_instance = ic->impl->si;
            ic->impl->shared_si = true;
        }
    }
}

static void
open_specific_factory (GtkIMContextSCIM *ic,
                       const String     &uuid)
{
    SCIM_DEBUG_FRONTEND(2) << "open_specific_factory context=" << ic->id << "\n";

    // The same input method is selected, just turn on the IC.
    if (ic->impl->si->get_factory_uuid () == uuid) {
        turn_on_ic (ic);
        return;
    }

    IMEngineFactoryPointer sf = _backend->get_factory (uuid);

    if (uuid.length () && !sf.null ()) {
        turn_off_ic (ic);
        ic->impl->si = sf->create_instance ("UTF-8", ic->impl->si->get_id ());
        ic->impl->si->set_frontend_data (static_cast <void*> (ic));
        ic->impl->preedit_string = WideString ();
        ic->impl->preedit_caret = 0;
        attach_instance (ic->impl->si);
        _backend->set_default_factory (_language, sf->get_uuid ());
        _panel_client.register_input_context (ic->id, sf->get_uuid ());
        set_ic_capabilities (ic);
        turn_on_ic (ic);

        if (_shared_input_method) {
            _default_instance = ic->impl->si;
            ic->impl->shared_si = true;
        }
    } else {
        turn_off_ic (ic);
    }
}

static void
attach_instance (const IMEngineInstancePointer &si)
{
    si->signal_connect_show_preedit_string (
        slot (slot_show_preedit_string));
    si->signal_connect_show_aux_string (
        slot (slot_show_aux_string));
    si->signal_connect_show_lookup_table (
        slot (slot_show_lookup_table));

    si->signal_connect_hide_preedit_string (
        slot (slot_hide_preedit_string));
    si->signal_connect_hide_aux_string (
        slot (slot_hide_aux_string));
    si->signal_connect_hide_lookup_table (
        slot (slot_hide_lookup_table));

    si->signal_connect_update_preedit_caret (
        slot (slot_update_preedit_caret));
    si->signal_connect_update_preedit_string (
        slot (slot_update_preedit_string));
    si->signal_connect_update_aux_string (
        slot (slot_update_aux_string));
    si->signal_connect_update_lookup_table (
        slot (slot_update_lookup_table));

    si->signal_connect_commit_string (
        slot (slot_commit_string));

    si->signal_connect_forward_key_event (
        slot (slot_forward_key_event));

    si->signal_connect_register_properties (
        slot (slot_register_properties));

    si->signal_connect_update_property (
        slot (slot_update_property));

    si->signal_connect_beep (
        slot (slot_beep));

    si->signal_connect_start_helper (
        slot (slot_start_helper));

    si->signal_connect_stop_helper (
        slot (slot_stop_helper));

    si->signal_connect_send_helper_event (
        slot (slot_send_helper_event));

    si->signal_connect_get_surrounding_text (
        slot (slot_get_surrounding_text));

    si->signal_connect_delete_surrounding_text (
        slot (slot_delete_surrounding_text));
}

// Implementation of slot functions
static void
slot_show_preedit_string (IMEngineInstanceBase *si)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_show_preedit_string...\n";

    GtkIMContextSCIM *ic = static_cast<GtkIMContextSCIM *> (si->get_frontend_data ());

    if (ic && ic->impl && _focused_ic == ic) {
        if (ic->impl->use_preedit) {
            if (!ic->impl->preedit_started) {
                g_signal_emit_by_name(_focused_ic, "preedit-start");
                ic->impl->preedit_started = true;
            }
            if (ic->impl->preedit_string.length ())
                g_signal_emit_by_name(_focused_ic, "preedit-changed");
        } else {
            _panel_client.show_preedit_string (ic->id);
        }
    }
}

static void 
slot_show_aux_string (IMEngineInstanceBase *si)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_show_aux_string...\n";

    GtkIMContextSCIM *ic = static_cast<GtkIMContextSCIM *> (si->get_frontend_data ());

    if (ic && ic->impl && _focused_ic == ic)
        _panel_client.show_aux_string (ic->id);
}

static void 
slot_show_lookup_table (IMEngineInstanceBase *si)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_show_lookup_table...\n";

    GtkIMContextSCIM *ic = static_cast<GtkIMContextSCIM *> (si->get_frontend_data ());

    if (ic && ic->impl && _focused_ic == ic)
        _panel_client.show_lookup_table (ic->id);
}

static void 
slot_hide_preedit_string (IMEngineInstanceBase *si)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_hide_preedit_string...\n";

    GtkIMContextSCIM *ic = static_cast<GtkIMContextSCIM *> (si->get_frontend_data ());

    if (ic && ic->impl && _focused_ic == ic) {
        bool emit=false;
        if (ic->impl->preedit_string.length ()) {
            ic->impl->preedit_string = WideString ();
            ic->impl->preedit_caret = 0;
            ic->impl->preedit_attrlist.clear ();
            emit = true;
        }
        if (ic->impl->use_preedit) {
            if (emit) g_signal_emit_by_name(ic, "preedit-changed");
            if (ic->impl->preedit_started) {
                g_signal_emit_by_name(ic, "preedit-end");
                ic->impl->preedit_started = false;
            }
        } else {
            _panel_client.hide_preedit_string (ic->id);
        }
    }
}

static void 
slot_hide_aux_string (IMEngineInstanceBase *si)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_hide_aux_string...\n";

    GtkIMContextSCIM *ic = static_cast<GtkIMContextSCIM *> (si->get_frontend_data ());

    if (ic && ic->impl && _focused_ic == ic)
        _panel_client.hide_aux_string (ic->id);
}

static void 
slot_hide_lookup_table (IMEngineInstanceBase *si)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_hide_lookup_table...\n";

    GtkIMContextSCIM *ic = static_cast<GtkIMContextSCIM *> (si->get_frontend_data ());

    if (ic && ic->impl && _focused_ic == ic)
        _panel_client.hide_lookup_table (ic->id);
}

static void 
slot_update_preedit_caret (IMEngineInstanceBase *si, int caret)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_update_preedit_caret...\n";

    GtkIMContextSCIM *ic = static_cast<GtkIMContextSCIM *> (si->get_frontend_data ());

    if (ic && ic->impl && _focused_ic == ic && ic->impl->preedit_caret != caret) {
        ic->impl->preedit_caret = caret;
        if (ic->impl->use_preedit) {
            if (!ic->impl->preedit_started) {
                g_signal_emit_by_name(_focused_ic, "preedit-start");
                ic->impl->preedit_started = true;
            }
            g_signal_emit_by_name(ic, "preedit-changed");
        } else {
            _panel_client.update_preedit_caret (ic->id, caret);
        }
    }
}

static void 
slot_update_preedit_string (IMEngineInstanceBase *si,
                            const WideString & str,
                            const AttributeList & attrs)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_update_preedit_string...\n";

    GtkIMContextSCIM *ic = static_cast<GtkIMContextSCIM *> (si->get_frontend_data ());

    if (ic && ic->impl && _focused_ic == ic && (ic->impl->preedit_string != str || str.length ())) {
        ic->impl->preedit_string   = str;
        ic->impl->preedit_attrlist = attrs;
        if (ic->impl->use_preedit) {
            if (!ic->impl->preedit_started) {
                g_signal_emit_by_name(_focused_ic, "preedit-start");
                ic->impl->preedit_started = true;
            }
            ic->impl->preedit_caret    = str.length ();
            ic->impl->preedit_updating = true;
            g_signal_emit_by_name(ic, "preedit-changed");
            ic->impl->preedit_updating = false;
        } else {
            _panel_client.update_preedit_string (ic->id, str, attrs);
        }
    }
}

static void 
slot_update_aux_string (IMEngineInstanceBase *si,
                        const WideString & str,
                        const AttributeList & attrs)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_update_aux_string...\n";

    GtkIMContextSCIM *ic = static_cast<GtkIMContextSCIM *> (si->get_frontend_data ());

    if (ic && ic->impl && _focused_ic == ic)
        _panel_client.update_aux_string (ic->id, str, attrs);
}

static void 
slot_commit_string (IMEngineInstanceBase *si,
                    const WideString & str)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_commit_string...\n";

    GtkIMContextSCIM *ic = static_cast<GtkIMContextSCIM *> (si->get_frontend_data ());

    if (ic)
        g_signal_emit_by_name (ic, "commit", utf8_wcstombs (str).c_str ());
}

static void 
slot_forward_key_event (IMEngineInstanceBase *si,
                        const KeyEvent & key)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_forward_key_event...\n";

    GtkIMContextSCIM *ic = static_cast<GtkIMContextSCIM *> (si->get_frontend_data ());

    if (ic && _focused_ic == ic) {
        GdkEventKey gdkevent = keyevent_scim_to_gdk (ic, key);
        if (!_fallback_instance->process_key_event (key) &&
            !gtk_im_context_filter_keypress (GTK_IM_CONTEXT (ic->slave), &gdkevent)) {

            // To avoid timing issue, we need emit the signal directly, rather than put the event into the queue.
            if (_focused_widget) {
                gboolean result;
                g_signal_emit_by_name(_focused_widget, key.is_key_press () ? "key-press-event" : "key-release-event", &gdkevent, &result);
            } else {
                gdk_event_put ((GdkEvent *) &gdkevent);
            }
        }
    }
}

static void 
slot_update_lookup_table (IMEngineInstanceBase *si,
                          const LookupTable & table)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_update_lookup_table...\n";

    GtkIMContextSCIM *ic = static_cast<GtkIMContextSCIM *> (si->get_frontend_data ());

    if (ic && ic->impl && _focused_ic == ic)
        _panel_client.update_lookup_table (ic->id, table);
}

static void 
slot_register_properties (IMEngineInstanceBase *si,
                          const PropertyList & properties)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_register_properties...\n";

    GtkIMContextSCIM *ic = static_cast<GtkIMContextSCIM *> (si->get_frontend_data ());

    if (ic && ic->impl && _focused_ic == ic)
        _panel_client.register_properties (ic->id, properties);
}

static void 
slot_update_property (IMEngineInstanceBase *si,
                      const Property & property)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_update_property ...\n";

    GtkIMContextSCIM *ic = static_cast<GtkIMContextSCIM *> (si->get_frontend_data ());

    if (ic && ic->impl && _focused_ic == ic)
        _panel_client.update_property (ic->id, property);
}

static void
slot_beep (IMEngineInstanceBase *si)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_beep ...\n";

    GtkIMContextSCIM *ic = static_cast<GtkIMContextSCIM *> (si->get_frontend_data ());

    if (ic && ic->impl && _focused_ic == ic)
        gdk_beep ();
}

static void
slot_start_helper (IMEngineInstanceBase *si,
                   const String &helper_uuid)
{
    GtkIMContextSCIM *ic = static_cast<GtkIMContextSCIM *> (si->get_frontend_data ());

    SCIM_DEBUG_FRONTEND(1) << "slot_start_helper helper= " << helper_uuid << " context="
                           << (ic ? ic->id : -1) << " ic=" << ic
                           << " ic-uuid=" << ((ic && ic->impl) ? ic->impl->si->get_factory_uuid () : "") << "...\n";

    if (ic && ic->impl)
        _panel_client.start_helper (ic->id, helper_uuid);
}

static void
slot_stop_helper (IMEngineInstanceBase *si,
                  const String &helper_uuid)
{
    GtkIMContextSCIM *ic = static_cast<GtkIMContextSCIM *> (si->get_frontend_data ());

    SCIM_DEBUG_FRONTEND(1) << "slot_stop_helper helper= " << helper_uuid << " context=" << (ic ? ic->id : -1) << " ic=" << ic << "...\n";

    if (ic && ic->impl)
        _panel_client.stop_helper (ic->id, helper_uuid);
}

static void
slot_send_helper_event (IMEngineInstanceBase *si,
                        const String      &helper_uuid,
                        const Transaction &trans)
{
    GtkIMContextSCIM *ic = static_cast<GtkIMContextSCIM *> (si->get_frontend_data ());

    SCIM_DEBUG_FRONTEND(1) << "slot_send_helper_event helper= " << helper_uuid << " context="
                           << (ic ? ic->id : -1) << " ic=" << ic
                           << " ic-uuid=" << ((ic && ic->impl) ? ic->impl->si->get_factory_uuid () : "") << "...\n";

    if (ic && ic->impl)
        _panel_client.send_helper_event (ic->id, helper_uuid, trans);
}

static bool
slot_get_surrounding_text (IMEngineInstanceBase *si,
                           WideString            &text,
                           int                   &cursor,
                           int                    maxlen_before,
                           int                    maxlen_after)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_get_surrounding_text ...\n";

    GtkIMContextSCIM *ic = static_cast<GtkIMContextSCIM *> (si->get_frontend_data ());

    if (ic && ic->impl && _focused_ic == ic) {
        gchar *surrounding;
        gint   cursor_index;
        if (gtk_im_context_get_surrounding (GTK_IM_CONTEXT (_focused_ic), &surrounding, &cursor_index)) {
            SCIM_DEBUG_FRONTEND(2) << "Surrounding text: " << surrounding <<"\n";
            SCIM_DEBUG_FRONTEND(2) << "Cursor Index    : " << cursor_index <<"\n";
            WideString before (utf8_mbstowcs (String (surrounding, surrounding + cursor_index)));
            WideString after (utf8_mbstowcs (String (surrounding + cursor_index)));
            if (maxlen_before > 0 && maxlen_before < before.length ())
                before = WideString (before.begin () + (before.length () - maxlen_before), before.end ());
            else if (maxlen_before == 0) before = WideString ();
            if (maxlen_after > 0 && maxlen_after < after.length ())
                after = WideString (after.begin (), after.begin () + maxlen_after);
            else if (maxlen_after == 0) after = WideString ();
            text = before + after;
            cursor = before.length ();
            return true;
        }
    }
    return false;
}

static bool
slot_delete_surrounding_text (IMEngineInstanceBase *si,
                              int                    offset,
                              int                    len)
{
    SCIM_DEBUG_FRONTEND(1) << "slot_delete_surrounding_text ...\n";

    GtkIMContextSCIM *ic = static_cast<GtkIMContextSCIM *> (si->get_frontend_data ());

    if (ic && ic->impl && _focused_ic == ic)
        return gtk_im_context_delete_surrounding (GTK_IM_CONTEXT (_focused_ic), offset, len);
    return false;
}

static void
reload_config_callback (const ConfigPointer &config)
{
    SCIM_DEBUG_FRONTEND(1) << "reload_config_callback...\n";

    _frontend_hotkey_matcher.load_hotkeys (config);
    _imengine_hotkey_matcher.load_hotkeys (config);

    KeyEvent key;

    scim_string_to_key (key,
        config->read (String (SCIM_CONFIG_HOTKEYS_FRONTEND_VALID_KEY_MASK),
                      String ("Shift+Control+Alt+Lock")));

    _valid_key_mask = (key.mask > 0)?(key.mask):0xFFFF;
    _valid_key_mask |= SCIM_KEY_ReleaseMask;
    // Special treatment for two backslash keys on jp106 keyboard.
    _valid_key_mask |= SCIM_KEY_QuirkKanaRoMask;

    _on_the_spot = config->read (String (SCIM_CONFIG_FRONTEND_ON_THE_SPOT), _on_the_spot);
    _shared_input_method = config->read (String (SCIM_CONFIG_FRONTEND_SHARED_INPUT_METHOD), _shared_input_method);
    _use_key_snooper = config->read (String (SCIM_CONFIG_FRONTEND_GTK_IMMODULE_USE_KEY_SNOOPER), _use_key_snooper);

    // Get keyboard layout setting
    // Flush the global config first, in order to load the new configs from disk.
    scim_global_config_flush ();

    _keyboard_layout = scim_get_default_keyboard_layout ();
}

static void
fallback_commit_string_cb (IMEngineInstanceBase  *si,
                           const WideString      &str)
{
    if (_focused_ic && _focused_ic->impl)
        g_signal_emit_by_name (_focused_ic, "commit", utf8_wcstombs (str).c_str ());
}

/*
vi:ts=4:expandtab:nowrap
*/
