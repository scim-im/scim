/** @file scim_panel_gtk.cpp
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
 * $Id: scim_panel_gtk.cpp,v 1.118.2.15 2007/04/11 11:30:31 suzhe Exp $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#if defined(GDK_WINDOWING_X11) && defined(SCIM_ENABLE_X11)
#include <gdk/x11/gdkx.h>
#include <X11/Xlib.h>
#endif
#include <stdlib.h>
#include <list>
#include <vector>
#include <memory>
#include <functional>

#define Uses_C_STDIO
#define Uses_C_STDLIB
#define Uses_SCIM_LOOKUP_TABLE
#define Uses_SCIM_SOCKET
#define Uses_SCIM_TRANSACTION
#define Uses_SCIM_TRANS_COMMANDS
#define Uses_SCIM_CONFIG
#define Uses_SCIM_CONFIG_MODULE
#define Uses_SCIM_DEBUG
#define Uses_SCIM_HELPER
#define Uses_SCIM_HELPER_MODULE
#define Uses_SCIM_PANEL_AGENT

#include "scim_private.h"
#include "scim.h"
#include "scim_stl_map.h"

#include "scimstringview.h"

using namespace scim;

#include "icons/setup.xpm"
#include "icons/help.xpm"
#include "icons/trademark.xpm"
#include "icons/pin-up.xpm"
#include "icons/pin-down.xpm"
#include "icons/menu.xpm"

#ifdef SCIM_HAS_SNI
#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/menuitem.h>
#endif

#ifdef SCIM_HAS_LAYER_SHELL
#include <gtk4-layer-shell.h>
#endif

#define SCIM_CONFIG_PANEL_GTK_FONT                      "/Panel/Gtk/Font"
#define SCIM_CONFIG_PANEL_GTK_COLOR_NORMAL_BG           "/Panel/Gtk/Color/NormalBackground"
#define SCIM_CONFIG_PANEL_GTK_COLOR_ACTIVE_BG           "/Panel/Gtk/Color/ActiveBackground"
#define SCIM_CONFIG_PANEL_GTK_COLOR_NORMAL_TEXT         "/Panel/Gtk/Color/NormalText"
#define SCIM_CONFIG_PANEL_GTK_COLOR_ACTIVE_TEXT         "/Panel/Gtk/Color/ActiveText"
#define SCIM_CONFIG_PANEL_GTK_TOOLBAR_ALWAYS_SHOW       "/Panel/Gtk/ToolBar/AlwaysShow"
#define SCIM_CONFIG_PANEL_GTK_TOOLBAR_ALWAYS_HIDDEN     "/Panel/Gtk/ToolBar/AlwaysHidden"
#define SCIM_CONFIG_PANEL_GTK_TOOLBAR_AUTO_SNAP         "/Panel/Gtk/ToolBar/AutoSnap"
#define SCIM_CONFIG_PANEL_GTK_TOOLBAR_HIDE_TIMEOUT      "/Panel/Gtk/ToolBar/HideTimeout"
#define SCIM_CONFIG_PANEL_GTK_TOOLBAR_POS_X             "/State/Panel/Gtk/ToolBar/POS_X"
#define SCIM_CONFIG_PANEL_GTK_TOOLBAR_POS_Y             "/State/Panel/Gtk/ToolBar/POS_Y"
#define SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_FACTORY_ICON "/Panel/Gtk/ToolBar/ShowFactoryIcon"
#define SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_FACTORY_NAME "/Panel/Gtk/ToolBar/ShowFactoryName"
#define SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_STICK_ICON   "/Panel/Gtk/ToolBar/ShowStickIcon"
#define SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_HELP_ICON    "/Panel/Gtk/ToolBar/ShowHelpIcon"
#define SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_MENU_ICON    "/Panel/Gtk/ToolBar/ShowMenuIcon"
#define SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_PROPERTY_LABEL "/Panel/Gtk/ToolBar/ShowPropertyLabel"
#define SCIM_CONFIG_PANEL_GTK_LOOKUP_TABLE_EMBEDDED     "/Panel/Gtk/LookupTableEmbedded"
#define SCIM_CONFIG_PANEL_GTK_LOOKUP_TABLE_VERTICAL     "/Panel/Gtk/LookupTableVertical"
#define SCIM_CONFIG_PANEL_GTK_DEFAULT_STICKED           "/Panel/Gtk/DefaultSticked"

#define SCIM_KEYBOARD_ICON_FILE     (SCIM_ICONDIR "/keyboard.png")
#define SCIM_TRADEMARK_ICON_FILE    (SCIM_ICONDIR "/trademark.png")
#define SCIM_SETUP_ICON_FILE        (SCIM_ICONDIR "/setup.png")
#define SCIM_HELP_ICON_FILE         (SCIM_ICONDIR "/help.png")
#define SCIM_MENU_ICON_FILE         (SCIM_ICONDIR "/menu.png")
#define SCIM_UP_ICON_FILE           (SCIM_ICONDIR "/up.png")
#define SCIM_DOWN_ICON_FILE         (SCIM_ICONDIR "/down.png")
#define SCIM_LEFT_ICON_FILE         (SCIM_ICONDIR "/left.png")
#define SCIM_RIGHT_ICON_FILE        (SCIM_ICONDIR "/right.png")
#define SCIM_PIN_UP_ICON_FILE       (SCIM_ICONDIR "/pin-up.png")
#define SCIM_PIN_DOWN_ICON_FILE     (SCIM_ICONDIR "/pin-down.png")

#define TOOLBAR_ICON_SIZE                     16
#define LOOKUP_ICON_SIZE                      12
// GtkIconSize/gtk_icon_size_lookup are gone in GTK4; use a fixed menu icon size.
#define MENU_ICON_SIZE                        16

// Drag targets for the window-move gestures.
enum {
    DRAG_TARGET_TOOLBAR = 0
};

/////////////////////////////////////////////////////////////////////////////
// Declaration of internal data types.
/////////////////////////////////////////////////////////////////////////////
struct PropertyInfo {
    Property   property;
    GtkWidget *widget;

    PropertyInfo () : widget (0) { }
};

typedef std::vector <PropertyInfo>               PropertyRepository;

struct HelperPropertyInfo {
    GtkWidget           *holder;
    PropertyRepository   repository;

    HelperPropertyInfo () : holder (0) { }
};

typedef scim_map <int, HelperPropertyInfo>                          HelperPropertyRepository;
typedef scim_map <String, std::vector <size_t> >                    MapStringVectorSizeT;

// Snapshot of a LookupTable's current-page contents, so that it can be
// marshaled to the main thread (LookupTable itself is non-copyable).
struct LookupTablePayload {
    std::vector<WideString>    candidates;
    std::vector<WideString>    labels;
    std::vector<AttributeList> attrs;
    unsigned int cursor_pos       = 0;
    bool         cursor_visible   = false;
    unsigned int page_start       = 0;
    unsigned int num_candidates   = 0;
    bool         page_size_fixed  = false;
    unsigned int page_size        = 0;
};

/////////////////////////////////////////////////////////////////////////////
// Declaration of internal functions.
/////////////////////////////////////////////////////////////////////////////
static void       ui_config_reload_callback            (const ConfigPointer &config);
static void       ui_load_config                       (void);
static void       ui_apply_panel_style                 (void);
static void       ui_initialize                        (void);

// CSS class tagging the panel text widgets (preedit/aux/lookup items/property
// labels) that ui_apply_panel_style () styles with the configured font/colors.
#define SCIM_PANEL_TEXT_CSS_CLASS "scim-panel-text"

static void       ui_settle_toolbar_window             (bool            force    = false);

static bool       ui_get_screen_rect                   (GdkRectangle &rect);
static int        ui_screen_width                      (void);
static int        ui_screen_height                     (void);
static void       ui_get_workarea                      (int            &x,
                                                        int            &y,
                                                        int            &width,
                                                        int            &height);
static void       ui_switch_screen                     (void);

// Absolute window positioning (X11 native; Wayland no-op).
static void       panel_window_move                    (GtkWidget      *w,
                                                        int             x,
                                                        int             y);

static GdkPixbuf* ui_scale_pixbuf                      (GdkPixbuf      *pixbuf,
                                                        int             width,
                                                        int             height);
static GtkWidget* ui_image_from_pixbuf                 (GdkPixbuf      *pixbuf,
                                                        int             pixel_size);

static GtkWidget* ui_create_label                      (const String   &name,
                                                        const String   &iconfile,
                                                        const char    **xpm,
                                                        bool            show_icon_only = false,
                                                        bool            force_icon = false);

static GtkWidget* ui_create_icon                       (const String   &iconfile,
                                                        const char    **xpm = NULL,
                                                        int             width = -1,
                                                        int             height = -1,
                                                        bool            force_create = false);

static GtkWidget* ui_create_trademark_icon             (void);
static GtkWidget* ui_create_stick_icon                 (bool            sticked);
static GtkWidget* ui_create_help_icon                  (void);
static GtkWidget* ui_create_menu_icon                  (void);

// Popover-menu helpers (GTK4 has no GtkMenu).
static GtkWidget* ui_menu_new                          (void);
static GtkWidget* ui_menu_get_box                      (GtkWidget      *popover);
static GtkWidget* ui_menu_append_button                (GtkWidget      *box,
                                                        const char     *label,
                                                        GtkWidget      *icon,
                                                        GCallback       cb,
                                                        gpointer        data);
static void       ui_menu_append_separator             (GtkWidget      *box);
static void       ui_menu_popup_at                     (GtkWidget      *popover,
                                                        GtkWidget      *anchor);
static void       panel_widget_destroy                 (GtkWidget      *w);

static GtkWidget* ui_create_factory_menu_entry         (const PanelFactoryInfo &info,
                                                        int                    id,
                                                        GtkWidget             *box,
                                                        bool                   show_lang,
                                                        bool                   show_name);

// callback functions
static void       ui_help_button_click_cb              (GtkButton      *button,
                                                        gpointer        user_data);
static void       ui_menu_button_click_cb              (GtkButton      *button,
                                                        gpointer        user_data);
static void       ui_factory_button_released_cb        (GtkGestureClick *gesture,
                                                        int             n_press,
                                                        double          x,
                                                        double          y,
                                                        gpointer        user_data);
static void       ui_factory_menu_activate_cb          (GtkButton      *item,
                                                        gpointer        user_data);
static void       ui_factory_menu_deactivate_cb        (GtkWidget      *item,
                                                        gpointer        user_data);
static void       ui_submenu_button_cb                 (GtkButton      *button,
                                                        gpointer        user_data);


static void       ui_window_stick_button_click_cb      (GtkButton      *button,
                                                        gpointer        user_data);

// Window dragging (GtkGestureDrag).
static void       ui_window_drag_begin_cb              (GtkGestureDrag *gesture,
                                                        double          start_x,
                                                        double          start_y,
                                                        gpointer        user_data);
static void       ui_window_drag_update_cb             (GtkGestureDrag *gesture,
                                                        double          offset_x,
                                                        double          offset_y,
                                                        gpointer        user_data);
static void       ui_window_drag_end_cb                (GtkGestureDrag *gesture,
                                                        double          offset_x,
                                                        double          offset_y,
                                                        gpointer        user_data);
static void       ui_toolbar_secondary_pressed_cb      (GtkGestureClick *gesture,
                                                        int             n_press,
                                                        double          x,
                                                        double          y,
                                                        gpointer        user_data);
static void       ui_toolbar_enter_cb                  (GtkEventControllerMotion *controller,
                                                        double          x,
                                                        double          y,
                                                        gpointer        user_data);
static void       ui_toolbar_leave_cb                  (GtkEventControllerMotion *controller,
                                                        gpointer        user_data);
static void       ui_toolbar_add_drag_controllers      (GtkWidget      *window,
                                                        int             drag_target);

static gboolean   ui_hide_window_timeout_cb            (gpointer data);

static void       ui_command_menu_exit_activate_cb     (GtkWidget      *item,
                                                        gpointer        user_data);
static void       ui_command_menu_reload_activate_cb   (GtkWidget      *item,
                                                        gpointer        user_data);
static void       ui_command_menu_stick_activate_cb    (GtkWidget      *item,
                                                        gpointer        user_data);
static void       ui_command_menu_hide_toolbar_toggled_cb (GtkWidget    *item,
                                                           gpointer      user_data);
static void       ui_command_menu_help_activate_cb     (GtkWidget      *item,
                                                        gpointer        user_data);
static void       ui_command_menu_helper_activate_cb   (GtkWidget      *item,
                                                        gpointer        user_data);
static void       ui_command_menu_deactivate_cb        (GtkWidget      *item,
                                                        gpointer        user_data);

// Client Property Callback
static void       ui_property_activate_cb              (GtkWidget      *widget,
                                                        gpointer        user_data);

static void       ui_property_menu_deactivate_cb       (GtkWidget      *item,
                                                        gpointer        user_data);

static gboolean   ui_help_close_request_cb             (GtkWindow      *window,
                                                        gpointer        user_data);


static bool       ui_any_menu_activated                (void);

static void       ui_show_help                         (const String   &help);

static PangoAttrList * create_pango_attrlist           (const String    &str,
                                                        const AttributeList &attrs);

// Action function
static void       action_request_help                  (void);
static void       action_toggle_window_stick           (void);
static void       action_show_command_menu             (void);

// PanelAgent related functions
static bool       initialize_panel_agent               (const String &config, const String &display, bool resident);
static bool       run_panel_agent                      (void);
static gpointer   panel_agent_thread_func              (gpointer data);
static void       start_auto_start_helpers             (void);

// Cross-thread marshaling: run a callable on the GLib main context.
static void       marshal_to_main                      (std::function<void()> fn);

// slot_* run on the PanelAgent thread; they marshal the real work
// (do_slot_*) onto the main loop.  Data is copied by value into the closure.
static void       slot_transaction_start               (void);
static void       slot_transaction_end                 (void);
static void       slot_reload_config                   (void);
static void       slot_turn_on                         (void);
static void       slot_turn_off                        (void);
static void       slot_update_screen                   (int screen);
static void       slot_update_factory_info             (const PanelFactoryInfo &info);
static void       slot_show_help                       (const String &help);
static void       slot_show_factory_menu               (const std::vector <PanelFactoryInfo> &menu);
static void       slot_register_properties             (const PropertyList &props);
static void       slot_update_property                 (const Property &prop);
static void       slot_register_helper_properties      (int id, const PropertyList &props);
static void       slot_update_helper_property          (int id, const Property &prop);
static void       slot_register_helper                 (int id, const HelperInfo &helper);
static void       slot_remove_helper                   (int id);
static void       slot_lock                            (void);
static void       slot_unlock                          (void);

// The actual work, always run on the main thread.
static void       do_slot_reload_config                (void);
static void       do_slot_turn_on                      (void);
static void       do_slot_turn_off                     (void);
static void       do_slot_update_screen                (int screen);
static void       do_slot_update_factory_info          (const PanelFactoryInfo &info);
static void       do_slot_show_help                    (const String &help);
static void       do_slot_show_factory_menu            (const std::vector <PanelFactoryInfo> &menu);
static void       do_slot_register_properties          (const PropertyList &props);
static void       do_slot_update_property              (const Property &prop);
static void       do_slot_register_helper_properties   (int id, const PropertyList &props);
static void       do_slot_update_helper_property       (int id, const Property &prop);
static void       do_slot_remove_helper                (int id);


static void       create_properties                    (GtkWidget            *container,
                                                        PropertyRepository &repository,
                                                        const PropertyList   &properties,
                                                        int                   client,
                                                        int                   level);

static GtkWidget* create_properties_node               (PropertyRepository         &repository,
                                                        PropertyList::const_iterator  begin,
                                                        PropertyList::const_iterator  end,
                                                        int                           client,
                                                        int                           level);

static void       register_frontend_properties         (const PropertyList &properties);
static void       update_frontend_property             (const Property     &property);
static void       register_helper_properties           (int                 client,
                                                        const PropertyList &properties);
static void       update_helper_property               (int                 client,
                                                        const Property     &property);

static void       update_property                      (PropertyRepository &repository,
                                                        const Property       &property);

static void       restore_properties                   (void);

static gboolean   check_exit_timeout_cb                (gpointer data);


/////////////////////////////////////////////////////////////////////////////
// Declaration of internal variables.
/////////////////////////////////////////////////////////////////////////////


static GtkWidget         *_toolbar_window              = 0;
static GtkWidget         *_toolbar_hbox                = 0;
static GtkWidget         *_window_stick_button         = 0;
static GtkWidget         *_factory_button              = 0;
static GtkWidget         *_factory_menu                = 0;
static GtkWidget         *_help_button                 = 0;
static GtkWidget         *_menu_button                 = 0;
static GtkWidget         *_client_properties_area      = 0;
static GtkWidget         *_frontend_properties_area    = 0;

static GtkWidget         *_help_dialog                 = 0;
static GtkWidget         *_help_scroll                 = 0;
static GtkWidget         *_help_area                   = 0;
static GtkWidget         *_command_menu                = 0;

static PangoFontDescription *_default_font_desc        = 0;
static GtkCssProvider       *_css_provider             = 0;



static gboolean           _toolbar_window_draging      = FALSE;


// The logical position captured at drag-begin (offsets are added to it).
static gint               _drag_start_x                = 0;
static gint               _drag_start_y                = 0;

static bool               _window_sticked              = false;

static bool               _toolbar_always_show         = false;
static bool               _toolbar_always_hidden       = false;
#ifdef SCIM_HAS_SNI
static bool               _sni_enabled                 = false;  // an SNI host was found
#endif
static bool               _toolbar_auto_snap           = true;
static bool               _toolbar_show_factory_icon   = true;
static bool               _toolbar_show_factory_name   = false;
static bool               _toolbar_show_stick_icon     = false;
static bool               _toolbar_show_help_icon      = false;
static bool               _toolbar_show_menu_icon      = false;
static bool               _toolbar_show_property_label = false;

static bool               _toolbar_should_hide         = false;
static bool               _toolbar_hidden              = false;
static bool               _factory_menu_activated      = false;
static bool               _command_menu_activated      = false;
static bool               _property_menu_activated     = false;


static int                _toolbar_window_x            = -1;
static int                _toolbar_window_y            = -1;
static int                _toolbar_hide_timeout_max    = 0;
static int                _toolbar_hide_timeout_count  = 0;
static guint              _toolbar_hide_timeout        = 0;

static bool               _ui_initialized              = false;


static GdkRGBA            _normal_bg;
static GdkRGBA            _normal_text;
static GdkRGBA            _active_bg;
static GdkRGBA            _active_text;

static ConfigModule      *_config_module               = 0;
static ConfigPointer      _config;

static guint              _check_exit_timeout          = 0;

static bool               _should_exit                 = false;

static bool               _panel_is_on                 = false;

static GThread           *_panel_agent_thread          = 0;

static PanelAgent        *_panel_agent                 = 0;

static GMainLoop         *_main_loop                   = 0;

static std::vector<String> _factory_menu_uuids;

static std::list<String>  _recent_factory_uuids;

static struct timeval     _last_menu_deactivate_time = {0, 0};

static bool               _multi_monitors              = false;

// client repository
static PropertyRepository            _frontend_property_repository;
static HelperPropertyRepository      _helper_property_repository;
static std::vector<HelperInfo>       _helper_list;

G_LOCK_DEFINE_STATIC     (_global_resource_lock);
G_LOCK_DEFINE_STATIC     (_panel_agent_lock);


/////////////////////////////////////////////////////////////////////////////
// Cross-thread marshaling helper.
/////////////////////////////////////////////////////////////////////////////
namespace {
struct MainThreadCall {
    std::function<void()> fn;
};
}

static gboolean
run_main_thread_call (gpointer data)
{
    MainThreadCall *c = static_cast<MainThreadCall*> (data);
    if (c->fn) c->fn ();
    return G_SOURCE_REMOVE;
}

static void
free_main_thread_call (gpointer data)
{
    delete static_cast<MainThreadCall*> (data);
}

static void
marshal_to_main (std::function<void()> fn)
{
    MainThreadCall *c = new MainThreadCall { std::move (fn) };
    g_main_context_invoke_full (NULL, G_PRIORITY_DEFAULT,
                                run_main_thread_call, c,
                                free_main_thread_call);
}


/////////////////////////////////////////////////////////////////////////////
// Implementation of internal functions.
/////////////////////////////////////////////////////////////////////////////
static void
ui_config_reload_callback (const ConfigPointer &config)
{
    _config = config;
    ui_initialize ();
    restore_properties ();
}

static void
ui_load_config (void)
{
    String str;

    // Read configurations.
    gdk_rgba_parse (&_normal_bg,   "gray92");
    gdk_rgba_parse (&_normal_text, "black");
    gdk_rgba_parse (&_active_bg,   "light blue");
    gdk_rgba_parse (&_active_text, "black");

    if (_default_font_desc) {
        pango_font_description_free (_default_font_desc);
        _default_font_desc = 0;
    }

    if (!_config.null ()) {
        str = _config->read (String (SCIM_CONFIG_PANEL_GTK_FONT),
                              String ("default"));

        if (str != String ("default"))
            _default_font_desc = pango_font_description_from_string (str.c_str ());

        str = _config->read (String (SCIM_CONFIG_PANEL_GTK_COLOR_NORMAL_BG),
                             String ("gray92"));
        gdk_rgba_parse (&_normal_bg, str.c_str ());

        str = _config->read (String (SCIM_CONFIG_PANEL_GTK_COLOR_NORMAL_TEXT),
                             String ("black"));
        gdk_rgba_parse (&_normal_text, str.c_str ());

        str = _config->read (String (SCIM_CONFIG_PANEL_GTK_COLOR_ACTIVE_BG),
                             String ("light blue"));
        gdk_rgba_parse (&_active_bg, str.c_str ());

        str = _config->read (String (SCIM_CONFIG_PANEL_GTK_COLOR_ACTIVE_TEXT),
                             String ("black"));
        gdk_rgba_parse (&_active_text, str.c_str ());

        _toolbar_window_x = _config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_POS_X),
                                           _toolbar_window_x);

        _toolbar_window_y = _config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_POS_Y),
                                           _toolbar_window_y);

        _window_sticked  =
            _config->read (String (SCIM_CONFIG_PANEL_GTK_DEFAULT_STICKED),
                           _window_sticked);

        _toolbar_always_show =
            _config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_ALWAYS_SHOW),
                           _toolbar_always_show);

        _toolbar_always_hidden =
            _config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_ALWAYS_HIDDEN),
                           _toolbar_always_hidden);

        // Impossible
        if (_toolbar_always_show && _toolbar_always_hidden)
            _toolbar_always_hidden = false;

#ifdef SCIM_HAS_SNI
        // The tray replaces the toolbar window entirely. Applied here rather
        // than once at startup so a config reload cannot bring the toolbar back
        // while we are sitting in the tray.
        if (_sni_enabled) {
            _toolbar_always_show   = false;
            _toolbar_always_hidden = true;
        }
#endif

        _toolbar_auto_snap =
            _config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_AUTO_SNAP),
                           _toolbar_auto_snap);

        _toolbar_show_factory_icon =
            _config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_FACTORY_ICON),
                           _toolbar_show_factory_icon);

        _toolbar_show_factory_name =
            _config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_FACTORY_NAME),
                           _toolbar_show_factory_name);

        _toolbar_show_stick_icon =
            _config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_STICK_ICON),
                           _toolbar_show_stick_icon);

        _toolbar_show_help_icon =
            _config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_HELP_ICON),
                           _toolbar_show_help_icon);

        _toolbar_show_menu_icon =
            _config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_MENU_ICON),
                           _toolbar_show_menu_icon);

        _toolbar_show_property_label =
            _config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_PROPERTY_LABEL),
                           _toolbar_show_property_label);

        _toolbar_hide_timeout_max =
            _config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_HIDE_TIMEOUT),
                           _toolbar_hide_timeout_max);
    }

    ui_apply_panel_style ();
}

// GTK4 removed gtk_widget_modify_font/fg/bg; push the configured panel font and
// normal fg/bg to the text widgets (tagged SCIM_PANEL_TEXT_CSS_CLASS) through a
// display-wide CSS provider instead. Called from ui_load_config (), so it also
// re-applies on the fly when the config is reloaded. The per-run reverse and
// highlight candidate colors are still applied via Pango attributes at draw
// time (see create_pango_attrlist); this only sets the base font and the colors
// those widgets would otherwise inherit from the GTK theme.
static void
ui_apply_panel_style (void)
{
    GString *css = g_string_new ("." SCIM_PANEL_TEXT_CSS_CLASS " {\n");

    gchar *bg = gdk_rgba_to_string (&_normal_bg);
    gchar *fg = gdk_rgba_to_string (&_normal_text);
    g_string_append_printf (css, "  background-color: %s;\n  color: %s;\n", bg, fg);
    g_free (bg);
    g_free (fg);

    if (_default_font_desc) {
        PangoFontMask mask = pango_font_description_get_set_fields (_default_font_desc);

        if (mask & PANGO_FONT_MASK_FAMILY) {
            const char *family = pango_font_description_get_family (_default_font_desc);
            if (family && *family)
                g_string_append_printf (css, "  font-family: \"%s\";\n", family);
        }
        if (mask & PANGO_FONT_MASK_SIZE) {
            int size = pango_font_description_get_size (_default_font_desc);
            if (size > 0) {
                if (pango_font_description_get_size_is_absolute (_default_font_desc))
                    g_string_append_printf (css, "  font-size: %dpx;\n", size / PANGO_SCALE);
                else
                    g_string_append_printf (css, "  font-size: %dpt;\n", size / PANGO_SCALE);
            }
        }
        if (mask & PANGO_FONT_MASK_WEIGHT)
            g_string_append_printf (css, "  font-weight: %d;\n",
                                    (int) pango_font_description_get_weight (_default_font_desc));
        if (mask & PANGO_FONT_MASK_STYLE) {
            PangoStyle style = pango_font_description_get_style (_default_font_desc);
            g_string_append_printf (css, "  font-style: %s;\n",
                                    style == PANGO_STYLE_ITALIC  ? "italic"  :
                                    style == PANGO_STYLE_OBLIQUE ? "oblique" : "normal");
        }
    }

    g_string_append (css, "}\n");

    if (!_css_provider) {
        _css_provider = gtk_css_provider_new ();
        GdkDisplay *display = gdk_display_get_default ();
        if (display)
            gtk_style_context_add_provider_for_display (
                display, GTK_STYLE_PROVIDER (_css_provider),
                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

    gtk_css_provider_load_from_string (_css_provider, css->str);

    g_string_free (css, TRUE);
}

// Absolute window positioning.  GTK4 removed gtk_window_move; on X11 we move
// the underlying override-ish toplevel with XMoveWindow.
#ifdef SCIM_HAS_LAYER_SHELL
// True when the compositor implements zwlr_layer_shell_v1 (KDE, wlroots). mutter
// does not, so this is always false on GNOME and the toolbar stays an ordinary
// toplevel there.
static bool
layer_shell_usable (void)
{
    return gtk_layer_is_supported () ? true : false;
}

// Wayland gives a client no way to place a toplevel, so the toolbar cannot honor
// its saved position and cannot stay above other windows. layer-shell does both:
// anchor it to a screen edge and put it on the overlay layer. Must run before the
// window is realized.
static void
layer_shell_setup_toolbar (GtkWidget *win)
{
    if (!win || !layer_shell_usable ()) return;

    gtk_layer_init_for_window (GTK_WINDOW (win));
    gtk_layer_set_namespace (GTK_WINDOW (win), "scim-panel");
    gtk_layer_set_layer (GTK_WINDOW (win), GTK_LAYER_SHELL_LAYER_OVERLAY);
    // Bottom-right, matching where the toolbar defaults to on X11.
    gtk_layer_set_anchor (GTK_WINDOW (win), GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
    gtk_layer_set_anchor (GTK_WINDOW (win), GTK_LAYER_SHELL_EDGE_RIGHT,  TRUE);
    // A status toolbar must never take keyboard focus from the text field.
    gtk_layer_set_keyboard_mode (GTK_WINDOW (win), GTK_LAYER_SHELL_KEYBOARD_MODE_NONE);
}

// On layer-shell the position is expressed as margins from the anchored edges.
static void
layer_shell_move_toolbar (GtkWidget *win, int x, int y)
{
    if (!win || !layer_shell_usable ()) return;

    int sw = ui_screen_width ();
    int sh = ui_screen_height ();
    GtkRequisition ws;
    gtk_widget_get_preferred_size (win, &ws, NULL);

    int right  = sw - x - ws.width;
    int bottom = sh - y - ws.height;
    gtk_layer_set_margin (GTK_WINDOW (win), GTK_LAYER_SHELL_EDGE_RIGHT,
                          right  > 0 ? right  : 0);
    gtk_layer_set_margin (GTK_WINDOW (win), GTK_LAYER_SHELL_EDGE_BOTTOM,
                          bottom > 0 ? bottom : 0);
}
#endif // SCIM_HAS_LAYER_SHELL

static void
panel_window_move (GtkWidget *w, int x, int y)
{
    if (!w) return;

#ifdef SCIM_HAS_LAYER_SHELL
    if (layer_shell_usable ()) {
        layer_shell_move_toolbar (w, x, y);
        return;
    }
#endif

#if defined(GDK_WINDOWING_X11) && defined(SCIM_ENABLE_X11)
    GdkDisplay *display = gtk_widget_get_display (w);
    if (display && GDK_IS_X11_DISPLAY (display)) {
        if (!gtk_widget_get_realized (w))
            gtk_widget_realize (w);

        GdkSurface *s = gtk_native_get_surface (GTK_NATIVE (w));
        if (s && GDK_IS_X11_SURFACE (s)) {
            XMoveWindow (GDK_SURFACE_XDISPLAY (s),
                         gdk_x11_surface_get_xid (s), x, y);
            XFlush (GDK_SURFACE_XDISPLAY (s));
        }
        return;
    }
#endif
    // TODO(wayland): position via text-input protocol (phase 5)
    (void) w; (void) x; (void) y;
}

static void
ui_initialize (void)
{
    SCIM_DEBUG_MAIN (1) << "Initialize UI...\n";


    ui_load_config ();
    _toolbar_hidden = false;

    if (_toolbar_window) gtk_window_destroy (GTK_WINDOW (_toolbar_window));
    if (_help_dialog) gtk_window_destroy (GTK_WINDOW (_help_dialog));

    _toolbar_window = 0;
    _toolbar_hbox = 0;
    _help_dialog = 0;
    _command_menu = 0;
    _factory_menu = 0;
    _frontend_properties_area = 0;

    //Create toolbar window
    {
        GtkWidget *hbox;
        GtkWidget *frame;
        GtkWidget *image;

        _toolbar_window = gtk_window_new ();
        gtk_window_set_decorated (GTK_WINDOW (_toolbar_window), FALSE);
        gtk_window_set_resizable (GTK_WINDOW (_toolbar_window), FALSE);
#ifdef SCIM_HAS_LAYER_SHELL
        // Before realization, so the surface is created as a layer surface.
        layer_shell_setup_toolbar (_toolbar_window);
#endif

        ui_toolbar_add_drag_controllers (_toolbar_window, DRAG_TARGET_TOOLBAR);

        frame = gtk_frame_new (0);
        gtk_window_set_child (GTK_WINDOW (_toolbar_window), frame);

        hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_frame_set_child (GTK_FRAME (frame), hbox);
        _toolbar_hbox = hbox;

        //New trademark pixmap
        image = ui_create_trademark_icon ();
        if (image)
            gtk_box_append (GTK_BOX (hbox), image);

        //New stick button
        if (_toolbar_show_stick_icon) {
            image = ui_create_stick_icon (_window_sticked);
            _window_stick_button = gtk_button_new ();
            gtk_widget_add_css_class (_window_stick_button, "flat");
            gtk_button_set_child (GTK_BUTTON (_window_stick_button), image);
            gtk_widget_set_hexpand (_window_stick_button, TRUE);
            gtk_box_append (GTK_BOX (hbox), _window_stick_button);
            g_signal_connect (G_OBJECT (_window_stick_button), "clicked",
                              G_CALLBACK (ui_window_stick_button_click_cb),
                              0);
        }

        //New factory button
        if (_toolbar_show_factory_icon || _toolbar_show_factory_name) {
            _factory_button = gtk_button_new ();
            gtk_widget_add_css_class (_factory_button, "flat");
            gtk_widget_set_hexpand (_factory_button, TRUE);
            gtk_box_append (GTK_BOX (hbox), _factory_button);

            GtkGesture *click = gtk_gesture_click_new ();
            gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (click), 0);
            g_signal_connect (click, "released",
                              G_CALLBACK (ui_factory_button_released_cb), 0);
            gtk_widget_add_controller (_factory_button, GTK_EVENT_CONTROLLER (click));
        }

        // Put all properties here
        _client_properties_area = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_hexpand (_client_properties_area, TRUE);
        gtk_box_append (GTK_BOX (hbox), _client_properties_area);
        gtk_widget_show (_client_properties_area);

        //New menu button
        if (_toolbar_show_menu_icon) {
            image = ui_create_menu_icon ();
            _menu_button = gtk_button_new ();
            gtk_widget_add_css_class (_menu_button, "flat");
            gtk_button_set_child (GTK_BUTTON (_menu_button), image);
            gtk_widget_set_hexpand (_menu_button, TRUE);
            gtk_box_append (GTK_BOX (hbox), _menu_button);
            g_signal_connect (G_OBJECT (_menu_button), "clicked",
                              G_CALLBACK (ui_menu_button_click_cb),
                              image);
        }

        //New help button
        if (_toolbar_show_help_icon) {
            image = ui_create_help_icon ();
            _help_button = gtk_button_new ();
            gtk_widget_add_css_class (_help_button, "flat");
            gtk_button_set_child (GTK_BUTTON (_help_button), image);
            gtk_widget_set_hexpand (_help_button, TRUE);
            gtk_box_append (GTK_BOX (hbox), _help_button);
            g_signal_connect (G_OBJECT (_help_button), "clicked",
                              G_CALLBACK (ui_help_button_click_cb),
                              image);
        }

        panel_window_move (_toolbar_window, ui_screen_width (), ui_screen_height ());

        ui_settle_toolbar_window ();
    }

    // Create help window
    {
        _help_dialog = gtk_window_new ();
        gtk_window_set_title (GTK_WINDOW (_help_dialog), _("SCIM Help"));
        gtk_window_set_resizable (GTK_WINDOW (_help_dialog), TRUE);

        GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
        gtk_window_set_child (GTK_WINDOW (_help_dialog), vbox);

        _help_scroll = gtk_scrolled_window_new ();
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (_help_scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
        gtk_widget_set_vexpand (_help_scroll, TRUE);
        gtk_box_append (GTK_BOX (vbox), _help_scroll);

        _help_area = gtk_label_new ("");
        gtk_label_set_justify (GTK_LABEL (_help_area), GTK_JUSTIFY_LEFT);
        gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (_help_scroll), _help_area);

        GtkWidget *ok = gtk_button_new_with_mnemonic (_("_OK"));
        gtk_widget_set_halign (ok, GTK_ALIGN_END);
        gtk_box_append (GTK_BOX (vbox), ok);
        g_signal_connect_swapped (ok, "clicked",
                                  G_CALLBACK (gtk_widget_hide), _help_dialog);

        g_signal_connect (_help_dialog, "close-request",
                          G_CALLBACK (ui_help_close_request_cb), NULL);
    }

    // TODO: reimplement the tray via a hand-rolled GDBus StatusNotifierItem (GTK4-safe; libayatana is gtk3-only).


    //Init timeout callback
    if (_toolbar_hide_timeout != 0) {
        g_source_remove (_toolbar_hide_timeout);
        _toolbar_hide_timeout = 0;

    }
    if (_toolbar_always_show && _toolbar_hide_timeout_max > 0) {
        _toolbar_hide_timeout =
            g_timeout_add (1000, ui_hide_window_timeout_cb, NULL);

        GtkEventController *motion = gtk_event_controller_motion_new ();
        g_signal_connect (motion, "enter", G_CALLBACK (ui_toolbar_enter_cb), 0);
        g_signal_connect (motion, "leave", G_CALLBACK (ui_toolbar_leave_cb), 0);
        gtk_widget_add_controller (_toolbar_window, motion);
    }

    // Init the tooltips
    {
        if (_window_stick_button)
            gtk_widget_set_tooltip_text (_window_stick_button,
                                  _("Stick/unstick the input window and the toolbar."));

        if (_help_button)
            gtk_widget_set_tooltip_text (_help_button,
                                  _("Show a brief help about SCIM and the current input method."));

        if (_menu_button)
            gtk_widget_set_tooltip_text (_menu_button,
                                  _("Show command menu."));
    }

    // TODO(x11): observe the X root window's _NET_WORKAREA / _NET_CURRENT_DESKTOP
    // properties (gdk_window_add_filter was X11-only and is gone in GTK4).

    _ui_initialized = true;
}



static void
ui_settle_toolbar_window (bool force)
{
    SCIM_DEBUG_MAIN (2) << " Settle toolbar window...\n";

    if (_window_sticked) {
        if (force)
            panel_window_move (_toolbar_window, _toolbar_window_x, _toolbar_window_y);
        return;
    }

    gint workarea_x, workarea_y, workarea_width, workarea_height;
    ui_get_workarea (workarea_x, workarea_y, workarea_width, workarea_height);

    GtkRequisition ws;
    gint pos_x, pos_y;

    gtk_widget_get_preferred_size (_toolbar_window, &ws, NULL);

    pos_x = _config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_POS_X),
                           workarea_x + workarea_width - ws.width);
    pos_y = _config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_POS_Y),
                           workarea_y + workarea_height - ws.height);
    if (_multi_monitors) {
       pos_x = -1;
       pos_y = -1;
    }
    if (pos_x == -1 && pos_y == -1) {
        pos_x = workarea_x + workarea_width  - ws.width;
        pos_y = workarea_y + workarea_height - ws.height;
    }

    if (_toolbar_auto_snap) {
        if ((ui_screen_width () - (pos_x + ws.width)) < pos_x)
            pos_x = ui_screen_width () - ws.width;
        else
            pos_x = 0;
    } else if (pos_x + ws.width > ui_screen_width ()) {
        pos_x = ui_screen_width () - ws.width;
    } else if (pos_x < 0) {
        pos_x = 0;
    }

    if (pos_y + ws.height > ui_screen_height ())
        pos_y = ui_screen_height () - ws.height;
    else if (pos_y < 0)
        pos_y = 0;

    if (_toolbar_window_x != pos_x || _toolbar_window_y != pos_y || force) {
        panel_window_move (_toolbar_window, pos_x, pos_y);
        _toolbar_window_x = pos_x;
        _toolbar_window_y = pos_y;
    }
}

// Fill rect with the geometry of the first/primary monitor.  Also sets
// _multi_monitors.  GTK4 has no work-area API, so callers use this instead.
static bool
ui_get_screen_rect (GdkRectangle &rect)
{
    rect.x = 0;
    rect.y = 0;
    rect.width = 0;
    rect.height = 0;

    GdkDisplay *display = gdk_display_get_default ();
    if (!display)
        return false;

    GListModel *monitors = gdk_display_get_monitors (display);
    if (!monitors)
        return false;

    guint n = g_list_model_get_n_items (monitors);
    _multi_monitors = (n > 1);

    if (n == 0)
        return false;

    GdkMonitor *mon = GDK_MONITOR (g_list_model_get_item (monitors, 0));
    if (!mon)
        return false;

    gdk_monitor_get_geometry (mon, &rect);
    g_object_unref (mon);
    return true;
}

static int
ui_screen_width (void)
{
    GdkRectangle rect;
    if (ui_get_screen_rect (rect) && rect.width > 0)
        return rect.x + rect.width;
    return 1024;
}

static int
ui_screen_height (void)
{
    GdkRectangle rect;
    if (ui_get_screen_rect (rect) && rect.height > 0)
        return rect.y + rect.height;
    return 768;
}

static void
ui_get_workarea (int &x, int &y, int &width, int &height)
{
    // GTK4 removed the _NET_WORKAREA X11 property helpers; fall back to the
    // primary monitor geometry.
    // TODO(x11): read _NET_WORKAREA / _NET_CURRENT_DESKTOP for the real work area.
    GdkRectangle rect;

    if (ui_get_screen_rect (rect) && rect.width > 0 && rect.height > 0) {
        x = rect.x;
        y = rect.y;
        width = rect.width;
        height = rect.height;
    } else {
        x = 0;
        y = 0;
        width = ui_screen_width ();
        height = ui_screen_height ();
    }
}

static void
ui_switch_screen (void)
{
    ui_settle_toolbar_window ();
}

static GdkPixbuf *
ui_scale_pixbuf (GdkPixbuf *pixbuf,
                 int        width,
                 int        height)
{
    if (pixbuf) {
        if (gdk_pixbuf_get_width (pixbuf) != width ||
            gdk_pixbuf_get_height (pixbuf) != height) {
            GdkPixbuf *dest = gdk_pixbuf_scale_simple (pixbuf, width, height, GDK_INTERP_BILINEAR);
            g_object_unref (pixbuf);
            pixbuf = dest;
        }
    }
    return pixbuf;
}

static GtkWidget *
ui_image_from_pixbuf (GdkPixbuf *pixbuf, int pixel_size)
{
    GdkTexture *tex = gdk_texture_new_for_pixbuf (pixbuf);
    GtkWidget *img = gtk_image_new_from_paintable (GDK_PAINTABLE (tex));
    if (pixel_size > 0)
        gtk_image_set_pixel_size (GTK_IMAGE (img), pixel_size);
    g_object_unref (tex);
    return img;
}

static GtkWidget *
ui_create_label (const String   &name,
                 const String   &iconfile,
                 const char    **xpm,
                 bool            show_icon_only,
                 bool            force_icon)
{
    GtkWidget * hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget * label = gtk_label_new (name.c_str ());

    // Font/colors come from the display CSS provider (ui_apply_panel_style).
    gtk_widget_add_css_class (label, SCIM_PANEL_TEXT_CSS_CLASS);

    GtkWidget *icon = ui_create_icon (iconfile,
                                      xpm,
                                      MENU_ICON_SIZE,
                                      MENU_ICON_SIZE,
                                      force_icon);

    if (icon) {
        gtk_box_append (GTK_BOX (hbox), icon);
        if (!show_icon_only)
            gtk_box_set_spacing (GTK_BOX (hbox), 4);
    }

    if (!show_icon_only || !icon) {
        gtk_box_append (GTK_BOX (hbox), label);
    } else {
        // Not used: sink the floating ref and drop it.
        g_object_ref_sink (label);
        g_object_unref (label);
    }

    return hbox;
}

static GtkWidget *
ui_create_icon (const String  &iconfile,
                const char   **xpm,
                int            width,
                int            height,
                bool           force_create)
{
    String path = iconfile;
    GdkPixbuf *pixbuf = 0;

    if (path.length ()) {
        // Not a absolute path, prepend SCIM_ICONDIR
        if (path [0] != SCIM_PATH_DELIM)
            path = String (SCIM_ICONDIR) + String (SCIM_PATH_DELIM_STRING) + path;

        pixbuf = gdk_pixbuf_new_from_file (path.c_str (), 0);
    }

    if (!pixbuf && xpm) {
        pixbuf = gdk_pixbuf_new_from_xpm_data (xpm);
    }

    if (!pixbuf && force_create) {
        if (width <= 0 || height <= 0)
            return 0;

        pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, true, 8, width, height);

        if (!pixbuf)
            return 0;

        gdk_pixbuf_fill (pixbuf, 0);
    }

    if (pixbuf) {
        if (width <= 0) width = gdk_pixbuf_get_width (pixbuf);
        if (height <= 0) height = gdk_pixbuf_get_height (pixbuf);

        pixbuf = ui_scale_pixbuf (pixbuf, width, height);

        GtkWidget *icon = ui_image_from_pixbuf (pixbuf, width);

        g_object_unref (pixbuf);

        return icon;
    }
    return 0;
}

static GtkWidget *
ui_create_trademark_icon (void)
{
    return ui_create_icon (SCIM_TRADEMARK_ICON_FILE,
                           (const char **) trademark_xpm,
                           TOOLBAR_ICON_SIZE + 4,
                           TOOLBAR_ICON_SIZE + 4);
}

static GtkWidget *
ui_create_stick_icon (bool sticked)
{
    return ui_create_icon ((sticked ? SCIM_PIN_DOWN_ICON_FILE : SCIM_PIN_UP_ICON_FILE),
                           (const char **) (sticked ? pin_down_xpm : pin_up_xpm),
                           TOOLBAR_ICON_SIZE,
                           TOOLBAR_ICON_SIZE);
}

static GtkWidget *
ui_create_help_icon (void)
{
    return ui_create_icon (SCIM_HELP_ICON_FILE,
                           (const char **) help_xpm,
                           TOOLBAR_ICON_SIZE,
                           TOOLBAR_ICON_SIZE);
}

static GtkWidget *
ui_create_menu_icon (void)
{
    return ui_create_icon (SCIM_MENU_ICON_FILE,
                           (const char **) menu_xpm,
                           TOOLBAR_ICON_SIZE,
                           TOOLBAR_ICON_SIZE);
}

// Lookup-table navigation icons -- only needed by the panel's own lookup
// window, which is not built when candidates are drawn in-process.

/////////////////////////////////////////////////////////////////////////////
// Popover-based menu helpers (replacing GtkMenu, removed in GTK4).
/////////////////////////////////////////////////////////////////////////////
static GtkWidget *
ui_menu_new (void)
{
    GtkWidget *popover = gtk_popover_new ();
    gtk_popover_set_has_arrow (GTK_POPOVER (popover), FALSE);
    gtk_popover_set_position (GTK_POPOVER (popover), GTK_POS_TOP);

    GtkWidget *scroll = gtk_scrolled_window_new ();
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
                                    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_propagate_natural_height (GTK_SCROLLED_WINDOW (scroll), TRUE);
    gtk_scrolled_window_set_propagate_natural_width (GTK_SCROLLED_WINDOW (scroll), TRUE);

    GtkWidget *box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scroll), box);
    gtk_popover_set_child (GTK_POPOVER (popover), scroll);

    g_object_set_data (G_OBJECT (popover), "menu_box", box);
    return popover;
}

static GtkWidget *
ui_menu_get_box (GtkWidget *popover)
{
    return GTK_WIDGET (g_object_get_data (G_OBJECT (popover), "menu_box"));
}

static GtkWidget *
ui_menu_append_button (GtkWidget *box,
                       const char *label,
                       GtkWidget *icon,
                       GCallback cb,
                       gpointer data)
{
    GtkWidget *btn = gtk_button_new ();
    gtk_widget_add_css_class (btn, "flat");

    GtkWidget *h = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
    if (icon)
        gtk_box_append (GTK_BOX (h), icon);
    if (label) {
        GtkWidget *l = gtk_label_new (label);
        gtk_widget_set_halign (l, GTK_ALIGN_START);
        gtk_widget_set_hexpand (l, TRUE);
        gtk_box_append (GTK_BOX (h), l);
    }
    gtk_button_set_child (GTK_BUTTON (btn), h);

    if (cb)
        g_signal_connect (btn, "clicked", cb, data);

    gtk_box_append (GTK_BOX (box), btn);
    return btn;
}

static void
ui_menu_append_separator (GtkWidget *box)
{
    GtkWidget *sep = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_box_append (GTK_BOX (box), sep);
}

static void
ui_menu_popup_at (GtkWidget *popover, GtkWidget *anchor)
{
    if (!popover || !anchor)
        return;

    if (!gtk_widget_get_parent (popover))
        gtk_widget_set_parent (popover, anchor);

    gtk_popover_popup (GTK_POPOVER (popover));
}

// Destroy any widget: windows via gtk_window_destroy, everything else by
// removing it from its parent (which drops the last ref).
static void
panel_widget_destroy (GtkWidget *w)
{
    if (!w) return;
    if (GTK_IS_WINDOW (w))
        gtk_window_destroy (GTK_WINDOW (w));
    else if (gtk_widget_get_parent (w))
        gtk_widget_unparent (w);
}

// Destroy notify for a popover stored as widget data.
static void
ui_destroy_popover_notify (gpointer data)
{
    GtkWidget *w = GTK_WIDGET (data);
    if (w)
        panel_widget_destroy (w);
}

static GtkWidget*
ui_create_factory_menu_entry (const PanelFactoryInfo &info,
                              int                    id,
                              GtkWidget             *box,
                              bool                   show_lang,
                              bool                   show_name)
{
    GtkWidget *menu_item;
    GtkWidget *icon_image;
    String text, tooltip;

    if ((!show_lang && show_name) || (show_lang && !show_name && (info.lang == "C" || info.lang == "~other"))) {
        text = info.name;
        tooltip = "";
    } else if (show_lang && !show_name) {
        text = scim_get_language_name (info.lang);
        tooltip = info.name;
    } else {
        text = scim_get_language_name (info.lang) + " - " + info.name;
        tooltip = "";
    }

    icon_image = ui_create_icon (info.icon, NULL, MENU_ICON_SIZE, MENU_ICON_SIZE, false);

    menu_item = ui_menu_append_button (box, text.c_str (), icon_image,
                                       G_CALLBACK (ui_factory_menu_activate_cb),
                                       GINT_TO_POINTER ((int) id));

    if (tooltip != "")
        gtk_widget_set_tooltip_text (menu_item, tooltip.c_str ());

    return menu_item;
}

/* Implementation of callback functions */
static void
ui_help_button_click_cb (GtkButton *button,
                         gpointer   user_data)
{
    SCIM_DEBUG_MAIN (3) << "  ui_help_button_click_cb...\n";

    if (gtk_widget_get_visible (_help_dialog)) {
        gtk_widget_hide (_help_dialog);
    } else {
        action_request_help ();
    }
}

static void
ui_menu_button_click_cb (GtkButton *button,
                         gpointer   user_data)
{
    SCIM_DEBUG_MAIN (3) << "  ui_menu_button_click_cb...\n";

    struct timeval cur_time;
    gettimeofday (&cur_time, 0);

    if (cur_time.tv_sec < _last_menu_deactivate_time.tv_sec ||
        (cur_time.tv_sec == _last_menu_deactivate_time.tv_sec &&
         cur_time.tv_usec < _last_menu_deactivate_time.tv_usec + 200000))
        return;

    action_show_command_menu ();
}

static void
ui_factory_button_released_cb (GtkGestureClick *gesture,
                               int              n_press,
                               double           x,
                               double           y,
                               gpointer         user_data)
{
    SCIM_DEBUG_MAIN (3) << "  ui_factory_button_released_cb...\n";

    guint button = gtk_gesture_single_get_current_button (GTK_GESTURE_SINGLE (gesture));

    struct timeval cur_time;
    gettimeofday (&cur_time, 0);

    if (cur_time.tv_sec < _last_menu_deactivate_time.tv_sec ||
        (cur_time.tv_sec == _last_menu_deactivate_time.tv_sec &&
         cur_time.tv_usec < _last_menu_deactivate_time.tv_usec + 200000))
        return;

    if (button <= 1)
        _panel_agent->request_factory_menu ();
    else
        action_show_command_menu ();
}

static void
ui_factory_menu_activate_cb (GtkButton *item,
                             gpointer     user_data)
{
    int id = GPOINTER_TO_INT (user_data);

    if (_factory_menu)
        gtk_popover_popdown (GTK_POPOVER (_factory_menu));

    if (id >= 0 && id < (int) _factory_menu_uuids.size ())
        _panel_agent->change_factory (_factory_menu_uuids [id]);
    else
        _panel_agent->change_factory ("");
}

static void
ui_factory_menu_deactivate_cb (GtkWidget *item,
                               gpointer     user_data)
{
    _factory_menu_activated = false;
    gettimeofday (&_last_menu_deactivate_time, 0);
}

// Open a nested submenu popover attached to a submenu button.
static void
ui_submenu_button_cb (GtkButton *button,
                      gpointer   user_data)
{
    GtkWidget *submenu = (GtkWidget *) g_object_get_data (G_OBJECT (button), "submenu");
    if (submenu)
        gtk_popover_popup (GTK_POPOVER (submenu));
}

// Lookup-table click/paging callbacks -- wired only to the panel's own lookup
// window, which is not built when candidates are drawn in-process.

static void
ui_window_stick_button_click_cb (GtkButton *button,
                                 gpointer user_data)
{
    action_toggle_window_stick ();
}

/////////////////////////////////////////////////////////////////////////////
// Window dragging (GtkGestureDrag) + toolbar crossing (motion controller).
/////////////////////////////////////////////////////////////////////////////
static void
ui_drag_get_context (int target, GtkWidget **win, gint **px, gint **py)
{
    // The toolbar is the only window the panel owns now: preedit, aux and
    // candidates are drawn in-process by the transports via libscim-candidates.
    (void) target;
    *win = _toolbar_window; *px = &_toolbar_window_x; *py = &_toolbar_window_y;
}

static void
ui_window_drag_begin_cb (GtkGestureDrag *gesture,
                         double          start_x,
                         double          start_y,
                         gpointer        user_data)
{
    int target = GPOINTER_TO_INT (user_data);
    GtkWidget *win; gint *px, *py;
    ui_drag_get_context (target, &win, &px, &py);

    _drag_start_x = *px;
    _drag_start_y = *py;

    _toolbar_window_draging = TRUE;
}

static void
ui_window_drag_update_cb (GtkGestureDrag *gesture,
                          double          offset_x,
                          double          offset_y,
                          gpointer        user_data)
{
    int target = GPOINTER_TO_INT (user_data);
    GtkWidget *win; gint *px, *py;
    ui_drag_get_context (target, &win, &px, &py);

    int nx = _drag_start_x + (int) offset_x;
    int ny = _drag_start_y + (int) offset_y;

    panel_window_move (win, nx, ny);
    *px = nx;
    *py = ny;
}

static void
ui_window_drag_end_cb (GtkGestureDrag *gesture,
                       double          offset_x,
                       double          offset_y,
                       gpointer        user_data)
{
    int target = GPOINTER_TO_INT (user_data);
    GtkWidget *win; gint *px, *py;
    ui_drag_get_context (target, &win, &px, &py);

    int nx = _drag_start_x + (int) offset_x;
    int ny = _drag_start_y + (int) offset_y;
    *px = nx;
    *py = ny;

    _toolbar_window_draging = FALSE;

    int pos_x = nx, pos_y = ny;
    if (!_config.null ()) {
        if (_multi_monitors) {
            pos_x = -1;
            pos_y = -1;
        }
        _config->write (SCIM_CONFIG_PANEL_GTK_TOOLBAR_POS_X, pos_x);
        _config->write (SCIM_CONFIG_PANEL_GTK_TOOLBAR_POS_Y, pos_y);
    }
}

static void
ui_toolbar_secondary_pressed_cb (GtkGestureClick *gesture,
                                 int              n_press,
                                 double           x,
                                 double           y,
                                 gpointer         user_data)
{
    action_show_command_menu ();
}

static void
ui_toolbar_add_drag_controllers (GtkWidget *window, int drag_target)
{
    GtkGesture *drag = gtk_gesture_drag_new ();
    gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (drag), GDK_BUTTON_PRIMARY);
    g_signal_connect (drag, "drag-begin",  G_CALLBACK (ui_window_drag_begin_cb),  GINT_TO_POINTER (drag_target));
    g_signal_connect (drag, "drag-update", G_CALLBACK (ui_window_drag_update_cb), GINT_TO_POINTER (drag_target));
    g_signal_connect (drag, "drag-end",    G_CALLBACK (ui_window_drag_end_cb),    GINT_TO_POINTER (drag_target));
    gtk_widget_add_controller (window, GTK_EVENT_CONTROLLER (drag));

    // The toolbar also shows the command menu on secondary-button click.
    if (drag_target == DRAG_TARGET_TOOLBAR) {
        GtkGesture *click = gtk_gesture_click_new ();
        gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (click), GDK_BUTTON_SECONDARY);
        g_signal_connect (click, "pressed", G_CALLBACK (ui_toolbar_secondary_pressed_cb), 0);
        gtk_widget_add_controller (window, GTK_EVENT_CONTROLLER (click));
    }
}

static void
ui_toolbar_enter_cb (GtkEventControllerMotion *controller,
                     double x, double y, gpointer user_data)
{
    if (!_toolbar_always_show || _panel_is_on || _toolbar_window_draging)
        return;

    if (_toolbar_hidden) {
        if (_window_stick_button)
            gtk_widget_show (_window_stick_button);

        if (_factory_button)
            gtk_widget_show (_factory_button);

        if (_client_properties_area)
            gtk_widget_show (_client_properties_area);

        if (_menu_button)
            gtk_widget_show (_menu_button);

        if (_help_button)
            gtk_widget_show (_help_button);

        _toolbar_hidden = false;
        ui_settle_toolbar_window ();
    }
    _toolbar_should_hide = false;
}

static void
ui_toolbar_leave_cb (GtkEventControllerMotion *controller,
                     gpointer user_data)
{
    if (!_toolbar_always_show || _panel_is_on || _toolbar_window_draging)
        return;

    _toolbar_should_hide = true;
}

static gboolean
ui_hide_window_timeout_cb (gpointer data)
{
    if (!_toolbar_always_show) {
        return TRUE;
    }

    if (!_toolbar_should_hide || _panel_is_on ||
        _toolbar_window_draging || _toolbar_hidden ||
        ui_any_menu_activated ()) {
        _toolbar_hide_timeout_count = 0;
        return TRUE;
    }

    _toolbar_hide_timeout_count ++;

    if (_toolbar_hide_timeout_count > _toolbar_hide_timeout_max) {
        _toolbar_hide_timeout_count = 0;

        if (_help_button)
            gtk_widget_hide (_help_button);

        if (_menu_button)
            gtk_widget_hide (_menu_button);

        if (_client_properties_area)
            gtk_widget_hide (_client_properties_area);

        if (_factory_button)
            gtk_widget_hide (_factory_button);

        if (_window_stick_button)
            gtk_widget_hide (_window_stick_button);

        _toolbar_hidden = true;
        ui_settle_toolbar_window ();
    }

    return TRUE;
}


static bool
ui_any_menu_activated (void)
{
    return _factory_menu_activated || _command_menu_activated || _property_menu_activated;
}

static gboolean
ui_help_close_request_cb (GtkWindow *window, gpointer user_data)
{
    gtk_widget_hide (GTK_WIDGET (window));
    return TRUE; // do not destroy
}

static void
ui_show_help (const String &help)
{
    if (!help.length () || !_help_dialog || !_help_scroll || !_help_area)
        return;

    GtkRequisition size;

    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (_help_scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    gtk_label_set_text (GTK_LABEL (_help_area), help.c_str ());

    gtk_widget_get_preferred_size (_help_area, &size, NULL);

    if (size.width > ui_screen_width ()/2) {
        size.width = ui_screen_width ()/2;
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (_help_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    }

    if (size.height > ui_screen_height ()/2)
        size.height = ui_screen_height ()/2;

    if (size.height < size.width/2)
        size.height = size.width/2;

    gtk_widget_set_size_request (_help_scroll, size.width, size.height);

    // TODO(gtk4): center on screen (GTK_WIN_POS_CENTER_ALWAYS is gone).
    gtk_window_present (GTK_WINDOW (_help_dialog));
}

static PangoAttrList *
create_pango_attrlist (const String        &mbs,
                       const AttributeList &attrs)
{
    PangoAttrList  *attrlist = pango_attr_list_new ();
    PangoAttribute *attr;

    guint start_index, end_index;
    guint wlen = g_utf8_strlen (mbs.c_str (), mbs.length ());

    guint16 _normal_bg_rgb[] = { (guint16)(65536*_normal_bg.red), (guint16)(65536*_normal_bg.green), (guint16)(65536*_normal_bg.blue) };
    guint16 _active_bg_rgb[] = { (guint16)(65536*_active_bg.red), (guint16)(65536*_active_bg.green), (guint16)(65536*_active_bg.blue) };
    guint16 _normal_text_rgb[] = { (guint16)(65536*_normal_text.red), (guint16)(65536*_normal_text.green), (guint16)(65536*_normal_text.blue) };
    guint16 _active_text_rgb[] = { (guint16)(65536*_active_text.red), (guint16)(65536*_active_text.green), (guint16)(65536*_active_text.blue) };

    for (int i=0; i < (int) attrs.size (); ++i) {
        start_index = attrs[i].get_start ();
        end_index = attrs[i].get_end ();

        if (end_index <= wlen && start_index < end_index) {
            start_index = g_utf8_offset_to_pointer (mbs.c_str (), attrs[i].get_start ()) - mbs.c_str ();
            end_index = g_utf8_offset_to_pointer (mbs.c_str (), attrs[i].get_end ()) - mbs.c_str ();

            if (attrs[i].get_type () == SCIM_ATTR_DECORATE) {
                if (attrs[i].get_value () == SCIM_ATTR_DECORATE_UNDERLINE) {
                    attr = pango_attr_underline_new (PANGO_UNDERLINE_SINGLE);
                    attr->start_index = start_index;
                    attr->end_index = end_index;
                    pango_attr_list_insert (attrlist, attr);
                } else if (attrs[i].get_value () == SCIM_ATTR_DECORATE_REVERSE) {
                    attr = pango_attr_foreground_new (_normal_bg_rgb[0], _normal_bg_rgb[1], _normal_bg_rgb[2]);
                    attr->start_index = start_index;
                    attr->end_index = end_index;
                    pango_attr_list_insert (attrlist, attr);

                    attr = pango_attr_background_new (_normal_text_rgb[0], _normal_text_rgb[1], _normal_text_rgb[2]);
                    attr->start_index = start_index;
                    attr->end_index = end_index;
                    pango_attr_list_insert (attrlist, attr);
                } else if (attrs[i].get_value () == SCIM_ATTR_DECORATE_HIGHLIGHT) {
                    attr = pango_attr_foreground_new (_active_text_rgb[0], _active_text_rgb[1], _active_text_rgb[2]);
                    attr->start_index = start_index;
                    attr->end_index = end_index;
                    pango_attr_list_insert (attrlist, attr);

                    attr = pango_attr_background_new (_active_bg_rgb[0], _active_bg_rgb[1], _active_bg_rgb[2]);
                    attr->start_index = start_index;
                    attr->end_index = end_index;
                    pango_attr_list_insert (attrlist, attr);
                }
            } else if (attrs[i].get_type () == SCIM_ATTR_FOREGROUND) {
                unsigned int color = attrs[i].get_value ();

                attr = pango_attr_foreground_new (SCIM_RGB_COLOR_RED(color) * 256, SCIM_RGB_COLOR_GREEN(color) * 256, SCIM_RGB_COLOR_BLUE(color) * 256);
                attr->start_index = start_index;
                attr->end_index = end_index;
                pango_attr_list_insert (attrlist, attr);
            } else if (attrs[i].get_type () == SCIM_ATTR_BACKGROUND) {
                unsigned int color = attrs[i].get_value ();

                attr = pango_attr_background_new (SCIM_RGB_COLOR_RED(color) * 256, SCIM_RGB_COLOR_GREEN(color) * 256, SCIM_RGB_COLOR_BLUE(color) * 256);
                attr->start_index = start_index;
                attr->end_index = end_index;
                pango_attr_list_insert (attrlist, attr);
            }
        }
    }
    return attrlist;
}

static void
ui_command_menu_exit_activate_cb (GtkWidget *item,
                                  gpointer     user_data)
{
    if (_command_menu)
        gtk_popover_popdown (GTK_POPOVER (_command_menu));
    _panel_agent->exit ();
}

static void
ui_command_menu_reload_activate_cb (GtkWidget *item,
                                    gpointer     user_data)
{
    if (_command_menu)
        gtk_popover_popdown (GTK_POPOVER (_command_menu));

    _panel_agent->reload_config ();

    if (!_config.null ()) _config->reload ();
}

static void
ui_command_menu_stick_activate_cb (GtkWidget *item,
                                   gpointer     user_data)
{
    action_toggle_window_stick ();
}

static void
ui_command_menu_hide_toolbar_toggled_cb (GtkWidget *item,
                                         gpointer     user_data)
{
    _toolbar_always_hidden = ! _toolbar_always_hidden;

    if (_toolbar_always_hidden && !_toolbar_hidden) {
        gtk_widget_hide (_toolbar_window);
        _toolbar_hidden = true;
    } else if (!_toolbar_always_hidden && _panel_is_on) {
        gtk_widget_show (_toolbar_window);
        _toolbar_hidden = false;
    }
}

static void
ui_command_menu_help_activate_cb (GtkWidget *item,
                                  gpointer     user_data)
{
    if (_command_menu)
        gtk_popover_popdown (GTK_POPOVER (_command_menu));

    if (gtk_widget_get_visible (_help_dialog)) {
        gtk_widget_hide (_help_dialog);
    } else {
        action_request_help ();
    }
}

static void
ui_command_menu_helper_activate_cb (GtkWidget *item,
                                    gpointer   user_data)
{
    size_t i = (size_t) GPOINTER_TO_INT (user_data);

    if (_command_menu)
        gtk_popover_popdown (GTK_POPOVER (_command_menu));

    if (i < _helper_list.size ())
        _panel_agent->start_helper (_helper_list [i].uuid);
}

static void
ui_command_menu_deactivate_cb (GtkWidget   *item,
                               gpointer     user_data)
{
    _command_menu_activated = false;
    gettimeofday (&_last_menu_deactivate_time, 0);
}

static void
ui_property_activate_cb (GtkWidget      *widget,
                         gpointer        user_data)
{
    GtkWidget *submenu = (GtkWidget *) g_object_get_data (G_OBJECT (widget), "property_submenu");

    if (submenu) {
        _property_menu_activated = true;
        gtk_popover_popup (GTK_POPOVER (submenu));
        return;
    }

    gchar * key = (gchar *) g_object_get_data (G_OBJECT (widget), "property_key");

    if (key) {
        int client = GPOINTER_TO_INT (user_data);

        if (client < 0)
            _panel_agent->trigger_property (key);
        else
            _panel_agent->trigger_helper_property (client, key);
    }
}

static void
ui_property_menu_deactivate_cb (GtkWidget   *item,
                                gpointer     user_data)
{
    _property_menu_activated = false;
}

//Implementation of the action functions
static void
action_request_help (void)
{
    if (!_panel_agent->request_help ()) {
        String help;

        help =  String (_("Smart Common Input Method platform ")) +
                String (SCIM_VERSION) +
                String (_("\n(C) 2002-2005 James Su <suzhe@tsinghua.org.cn>"));

        ui_show_help (help);
    }
}

static void
action_toggle_window_stick (void)
{
    GtkWidget *image;

    _window_sticked = ! _window_sticked;

    if (_window_stick_button) {
        image = ui_create_stick_icon (_window_sticked);
        gtk_button_set_child (GTK_BUTTON (_window_stick_button), image);
    }
}

static void
action_show_command_menu (void)
{
    if (_command_menu_activated)
        return;

    _command_menu_activated = true;

    if (_command_menu) {
        panel_widget_destroy (_command_menu);
        _command_menu = 0;
    }

    _command_menu = ui_menu_new ();
    GtkWidget *box = ui_menu_get_box (_command_menu);

    GtkWidget *icon;

    // Add Helper object items.
    for (size_t i = 0; i < _helper_list.size (); ++i) {
        if ((_helper_list [i].option & SCIM_HELPER_STAND_ALONE) != 0 &&
            (_helper_list [i].option & SCIM_HELPER_AUTO_START) == 0) {
            icon = ui_create_icon (_helper_list [i].icon, NULL, MENU_ICON_SIZE, MENU_ICON_SIZE, false);
            GtkWidget *item = ui_menu_append_button (box, _helper_list [i].name.c_str (), icon,
                                                     G_CALLBACK (ui_command_menu_helper_activate_cb),
                                                     GINT_TO_POINTER ((int)i));
            gtk_widget_set_tooltip_text (item, _helper_list [i].description.c_str ());
        }
    }

    if (_helper_list.size ()) {
        ui_menu_append_separator (box);
    }

    //Reload Configuration.
    ui_menu_append_button (box, _("Reload Configuration"), 0,
                           G_CALLBACK (ui_command_menu_reload_activate_cb), 0);

    //Stick
    {
        GtkWidget *check = gtk_check_button_new_with_label (_("Stick Windows"));
        gtk_check_button_set_active (GTK_CHECK_BUTTON (check), _window_sticked);
        g_signal_connect (G_OBJECT (check), "toggled",
                          G_CALLBACK (ui_command_menu_stick_activate_cb), 0);
        gtk_box_append (GTK_BOX (box), check);
    }

    //Toolbar
    {
        GtkWidget *check = gtk_check_button_new_with_label (_("Hide Toolbar"));
        gtk_check_button_set_active (GTK_CHECK_BUTTON (check), _toolbar_always_hidden);
        g_signal_connect (G_OBJECT (check), "toggled",
                          G_CALLBACK (ui_command_menu_hide_toolbar_toggled_cb), 0);
        gtk_box_append (GTK_BOX (box), check);
    }

    //Help
    ui_menu_append_button (box, _("Help ..."), 0,
                           G_CALLBACK (ui_command_menu_help_activate_cb), 0);

    ui_menu_append_separator (box);

    //Clients exit.
    ui_menu_append_button (box, _("Exit"), 0,
                           G_CALLBACK (ui_command_menu_exit_activate_cb), 0);

    g_signal_connect (G_OBJECT (_command_menu), "closed",
                      G_CALLBACK (ui_command_menu_deactivate_cb), NULL);

    GtkWidget *anchor = _menu_button ? _menu_button
                        : (_factory_button ? _factory_button : _toolbar_hbox);
    ui_menu_popup_at (_command_menu, anchor);
}


#ifdef SCIM_HAS_SNI
/////////////////////////////////////////////////////////////////////////////
// StatusNotifierItem tray
//
// On a desktop with an SNI host (KDE, XFCE, ...) the panel presents itself as a
// tray item instead of a floating toolbar: wayland gives a client no way to
// place or raise a toplevel, so the toolbar is unusable there, while a tray item
// is drawn by the host and needs no positioning at all.
//
// The icon identifies the active engine, the tooltip carries its status, and the
// exported dbusmenu carries the engine's properties. ItemIsMenu tells the host
// to open that menu on primary click, which is what clicking the old toolbar's
// factory button did.
/////////////////////////////////////////////////////////////////////////////

static guint             _sni_name_id       = 0;   // our own bus name
static guint             _sni_watch_id      = 0;   // watching for a host
static guint             _sni_object_id     = 0;   // exported SNI object
static GDBusConnection  *_sni_conn          = 0;
static DbusmenuServer   *_sni_menu          = 0;
static bool              _sni_registered    = false;
static String            _sni_bus_name;
static String            _sni_icon;                // absolute path of the engine icon
static String            _sni_symbol;               // engine symbol, drawn as the tray icon
static String            _sni_icon_name;            // themed name, when one applies
static bool              _sni_is_keyboard   = false; // is the IM off (plain keyboard)?
static bool              _sni_prefer_dark   = false; // is the tray dark?
static bool              _sni_dark_from_theme = false; // ... and did the GTK theme say so
static String            _sni_title;                // engine name
static String            _sni_status_text;          // status property label, for the tooltip
static std::vector<PanelFactoryInfo> _sni_factories;  // engine list, for the menu

#define SCIM_SNI_OBJECT_PATH  "/StatusNotifierItem"
#define SCIM_SNI_MENU_PATH    "/StatusNotifierItem/Menu"
#define SCIM_SNI_WATCHER      "org.kde.StatusNotifierWatcher"

static const gchar _sni_introspection_xml[] =
    "<node>"
    "  <interface name='org.kde.StatusNotifierItem'>"
    "    <property name='Category'   type='s' access='read'/>"
    "    <property name='Id'         type='s' access='read'/>"
    "    <property name='Title'      type='s' access='read'/>"
    "    <property name='Status'     type='s' access='read'/>"
    "    <property name='IconName'   type='s' access='read'/>"
    "    <property name='IconPixmap' type='a(iiay)' access='read'/>"
    "    <property name='ItemIsMenu' type='b' access='read'/>"
    "    <property name='Menu'       type='o' access='read'/>"
    "    <property name='ToolTip'    type='(sa(iiay)ss)' access='read'/>"
    "    <method name='Activate'>"
    "      <arg name='x' type='i' direction='in'/>"
    "      <arg name='y' type='i' direction='in'/>"
    "    </method>"
    "    <method name='SecondaryActivate'>"
    "      <arg name='x' type='i' direction='in'/>"
    "      <arg name='y' type='i' direction='in'/>"
    "    </method>"
    "    <method name='ContextMenu'>"
    "      <arg name='x' type='i' direction='in'/>"
    "      <arg name='y' type='i' direction='in'/>"
    "    </method>"
    "    <signal name='NewIcon'/>"
    "    <signal name='NewTitle'/>"
    "    <signal name='NewToolTip'/>"
    "    <signal name='NewStatus'><arg name='status' type='s'/></signal>"
    "  </interface>"
    "</node>";

static void sni_rebuild_menu (void);

// Tell the host something changed. Hosts re-read the properties themselves.
static void
sni_emit (const char *signal_name)
{
    if (!_sni_conn || !_sni_object_id) return;

    g_dbus_connection_emit_signal (_sni_conn, 0, SCIM_SNI_OBJECT_PATH,
                                   "org.kde.StatusNotifierItem", signal_name,
                                   0, 0);
}

static void
sni_method_call (GDBusConnection * /*conn*/, const gchar * /*sender*/,
                 const gchar * /*path*/, const gchar * /*iface*/,
                 const gchar *method, GVariant * /*params*/,
                 GDBusMethodInvocation *invocation, gpointer /*data*/)
{
    // ItemIsMenu is true, so a well-behaved host opens the menu itself on
    // primary click and never calls Activate. Hosts that call it anyway get the
    // same thing the old toolbar did on a left click: the engine list, which
    // lives in the menu we already export.
    // SecondaryActivate (middle click) is deliberately inert: cycling engines
    // needs the factory list, which only arrives asynchronously after
    // request_factory_menu (), and the toolbar's reply path pops a GTK window
    // that has nothing to anchor to in tray mode. The engine list belongs in the
    // exported menu instead.

    g_dbus_method_invocation_return_value (invocation, 0);
}

#define SNI_ICON_SIZE 22

// Draw the engine symbol as the tray image.
//
// The point of a symbol is that it survives both a light and a dark tray, which
// a fixed PNG cannot: an engine icon drawn in a light ink vanishes on a light
// panel and vice versa. Note that SNI does not actually let us hand over text
// -- it has no label property at all, and hosts recolor only themed symbolic
// icon *names*, which cannot express a glyph like a Han character. So we still
// choose the ink ourselves. What a glyph buys us is that it can be drawn in a
// way that works either way: a dark fill inside a light outline stays legible
// against any background, which is the same trick subtitles use.
static bool
sni_render_symbol (const String &symbol, int size,
                   std::vector<guchar> &argb, int &out_w, int &out_h)
{
    if (!symbol.length ())
        return false;

    cairo_surface_t *surf =
        cairo_image_surface_create (CAIRO_FORMAT_ARGB32, size, size);
    if (cairo_surface_status (surf) != CAIRO_STATUS_SUCCESS) {
        cairo_surface_destroy (surf);
        return false;
    }

    cairo_t *cr = cairo_create (surf);
    PangoLayout *layout = pango_cairo_create_layout (cr);

    // Take the family from the desktop's UI font, so the symbol looks like the
    // panel's own text. Naming no family at all is not neutral: pango then falls
    // back to its default, which resolves to a serif face (DejaVu Serif for
    // latin, a Ming style for Han) -- wrong for a tray indicator, and thin
    // strokes lose against a sans/Hei face at 22 pixels. The family is only a
    // starting point either way; fontconfig still falls back per script, so a
    // CJK or Indic symbol resolves whatever the UI font itself lacks.
    PangoFontDescription *desc = 0;
    {
        gchar *ui_font = 0;
        GtkSettings *settings = gtk_settings_get_default ();
        if (settings)
            g_object_get (settings, "gtk-font-name", &ui_font, NULL);
        if (ui_font && *ui_font)
            desc = pango_font_description_from_string (ui_font);
        g_free (ui_font);
    }
    if (!desc)
        desc = pango_font_description_from_string ("Sans");

    // The size carried by gtk-font-name is irrelevant: the fitting loop below
    // sets an absolute size to fill the box.
    //
    // Draw at a regular weight, whatever weight the UI font itself names. A bold
    // face is what turns a dense glyph into a smudge at this size: the gaps
    // inside a 15 or 20 stroke Han character are about a pixel wide when it is
    // fitted to 20 pixels, and the extra stroke weight closes them, so the
    // character collapses into a block. A regular face keeps the strokes apart.
    pango_font_description_set_weight (desc, PANGO_WEIGHT_NORMAL);

    // Keep the glyph off the very edge of the box.
    double margin = size * 0.07;
    if (margin < 1.0) margin = 1.0;
    double avail = size - 2.0 * margin;

    // Fit the text to the box, then drop a character and retry if fitting drove
    // the font below what can still be read: three latin letters in a 22 pixel
    // tray come out five pixels tall, which is a smudge rather than a symbol.
    // The engine's full symbol survives untouched everywhere it is drawn as
    // real text; this only bounds what we rasterize.
    String text = symbol;
    double px = avail;
    PangoRectangle ink;

    for (;;) {
        pango_layout_set_text (layout, text.c_str (), -1);

        // Start from a guess and correct once against the measured ink; text
        // scales closely enough to linearly for one correction to land.
        px = avail;
        for (int pass = 0; pass < 2; ++pass) {
            pango_font_description_set_absolute_size (desc, px * PANGO_SCALE);
            pango_layout_set_font_description (layout, desc);

            pango_layout_get_pixel_extents (layout, &ink, 0);
            if (ink.width <= 0 || ink.height <= 0)
                break;

            double sx = avail / (double) ink.width;
            double sy = avail / (double) ink.height;
            double scale = (sx < sy) ? sx : sy;
            if (pass == 0 || scale < 1.0)
                px *= scale;
        }

        long chars = g_utf8_strlen (text.c_str (), -1);
        if (px >= avail * 0.5 || chars <= 1)
            break;

        const char *end = g_utf8_offset_to_pointer (text.c_str (), chars - 1);
        text.erase (end - text.c_str ());
    }

    pango_layout_get_pixel_extents (layout, &ink, 0);

    // Center on the ink, not the logical box: line spacing and side bearings
    // differ per font, and an off-center glyph is obvious at this size.
    cairo_move_to (cr,
                   (size - ink.width) / 2.0 - ink.x,
                   (size - ink.height) / 2.0 - ink.y);
    pango_cairo_layout_path (cr, layout);

    // Ink follows the tray's shade; the halo is always its opposite, so the
    // symbol survives a wrong guess. See sni_set_dark ().
    double ink_v  = _sni_prefer_dark ? 0.93 : 0.09;
    double halo_v = _sni_prefer_dark ? 0.05 : 1.00;

    // The outline has to scale with the glyph's own stroke weight, not with the
    // box: sized off the box it swallows small text whole -- a two letter symbol
    // at a 12 pixel font has stems barely 2 pixels wide, so a 4 pixel outline
    // merges them into one blob with the letters showing through as holes.
    // A hairline is all the insurance we need. Keep it thin for the same reason
    // the fill is not bold: half the line width lies inside the path, so a fat
    // halo eats into a dense glyph's gaps from both sides.
    double outline = px * 0.07;
    if (outline < 0.7) outline = 0.7;
    if (outline > 1.4) outline = 1.4;

    cairo_set_line_width (cr, outline);      // half of it lies outside the path
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_source_rgba (cr, halo_v, halo_v, halo_v, 0.94);
    cairo_stroke_preserve (cr);
    cairo_set_source_rgb (cr, ink_v, ink_v, ink_v);
    cairo_fill (cr);

    g_object_unref (layout);
    pango_font_description_free (desc);
    cairo_destroy (cr);
    cairo_surface_flush (surf);

    const guchar *data = cairo_image_surface_get_data (surf);
    int stride = cairo_image_surface_get_stride (surf);
    if (!data) {
        cairo_surface_destroy (surf);
        return false;
    }

    out_w = size;
    out_h = size;
    argb.resize ((size_t) size * (size_t) size * 4);
    guchar *o = &argb [0];

    for (int y = 0; y < size; ++y) {
        // Cairo keeps ARGB32 as native-endian premultiplied words; SNI wants
        // plain ARGB bytes, so unpack and undo the premultiplication.
        const guint32 *row = (const guint32 *) (data + (size_t) y * stride);
        for (int x = 0; x < size; ++x) {
            guint32 p = row [x];
            guchar a = (p >> 24) & 0xFF;
            guchar r = (p >> 16) & 0xFF;
            guchar g = (p >>  8) & 0xFF;
            guchar bl = p & 0xFF;
            if (a && a != 0xFF) {
                r = (guchar) ((r * 255 + a / 2) / a);
                g = (guchar) ((g * 255 + a / 2) / a);
                bl = (guchar) ((bl * 255 + a / 2) / a);
            }
            *o++ = a;
            *o++ = r;
            *o++ = g;
            *o++ = bl;
        }
    }

    cairo_surface_destroy (surf);
    return true;
}

// Publish the engine icon as raw pixels. IconName cannot carry it: that
// property is a freedesktop icon-theme *name*, which hosts resolve through the
// theme (QIcon::fromTheme on Plasma), so an absolute path like
// /usr/local/share/scim/icons/Array30.png resolves to nothing and the item
// renders blank. IconPixmap is the spec's answer for icons that live at an
// arbitrary path, and every SNI host supports it.
static GVariant *
sni_icon_pixmap (void)
{
    // a(iiay): width, height, ARGB32 in network byte order.
    GVariantBuilder b;
    g_variant_builder_init (&b, G_VARIANT_TYPE ("a(iiay)"));

    // Prefer the symbol: see sni_render_symbol ().
    {
        std::vector<guchar> sym;
        int sw = 0, sh = 0;
        if (sni_render_symbol (_sni_symbol, SNI_ICON_SIZE, sym, sw, sh)) {
            guchar *copy = (guchar *) g_memdup2 (&sym [0], sym.size ());
            g_variant_builder_add (&b, "(ii@ay)", sw, sh,
                                   g_variant_new_from_data (G_VARIANT_TYPE ("ay"),
                                                            copy, sym.size (), TRUE,
                                                            g_free, copy));
            return g_variant_builder_end (&b);
        }
    }

    String path = _sni_icon;
    if (path.length () && path [0] != SCIM_PATH_DELIM)
        path = String (SCIM_ICONDIR) + String (SCIM_PATH_DELIM_STRING) + path;

    GdkPixbuf *pb = 0;
    if (path.length ())
        pb = gdk_pixbuf_new_from_file_at_size (path.c_str (), SNI_ICON_SIZE, SNI_ICON_SIZE, 0);
    // Always show something: an engine with no icon of its own, or a missing
    // file, would otherwise leave an invisible tray item.
    if (!pb)
        pb = gdk_pixbuf_new_from_file_at_size (SCIM_TRADEMARK_ICON_FILE, SNI_ICON_SIZE, SNI_ICON_SIZE, 0);
    if (!pb)
        return g_variant_builder_end (&b);

    int w      = gdk_pixbuf_get_width (pb);
    int h      = gdk_pixbuf_get_height (pb);
    int stride = gdk_pixbuf_get_rowstride (pb);
    int nch    = gdk_pixbuf_get_n_channels (pb);
    const guchar *px = gdk_pixbuf_get_pixels (pb);

    gsize n = (gsize) w * (gsize) h * 4;
    guchar *argb = (guchar *) g_malloc (n);
    guchar *o = argb;
    for (int y = 0; y < h; ++y) {
        const guchar *row = px + (gsize) y * stride;
        for (int x = 0; x < w; ++x) {
            const guchar *s = row + (gsize) x * nch;
            *o++ = (nch == 4) ? s [3] : 0xFF;   // A
            *o++ = s [0];                       // R
            *o++ = s [1];                       // G
            *o++ = s [2];                       // B
        }
    }
    g_object_unref (pb);

    g_variant_builder_add (&b, "(ii@ay)", w, h,
                           g_variant_new_from_data (G_VARIANT_TYPE ("ay"),
                                                    argb, n, TRUE,
                                                    g_free, argb));
    return g_variant_builder_end (&b);
}

static GVariant *
sni_get_property (GDBusConnection * /*conn*/, const gchar * /*sender*/,
                  const gchar * /*path*/, const gchar * /*iface*/,
                  const gchar *name, GError ** /*error*/, gpointer /*data*/)
{
    if (!g_strcmp0 (name, "Category"))   return g_variant_new_string ("SystemServices");
    if (!g_strcmp0 (name, "Id"))         return g_variant_new_string ("scim");
    // Constant on purpose. Hosts sort tray items by title, so returning the
    // engine name here made our icon hop between neighbouring items on every
    // engine switch. Which engine is active is shown by the icon, and named in
    // the tooltip; the title is our identity, not our state.
    if (!g_strcmp0 (name, "Title"))      return g_variant_new_string ("SCIM");
    if (!g_strcmp0 (name, "Status"))     return g_variant_new_string ("Active");
    // A themed name only for the keyboard state, and only when the theme really
    // has it; empty otherwise. The pixmap stays published either way: the spec
    // tells hosts to prefer names, so a conforming host shows the themed icon,
    // and one that only reads pixmaps still shows the rendered symbol instead of
    // an empty item.
    if (!g_strcmp0 (name, "IconName"))
        return g_variant_new_string (_sni_icon_name.c_str ());
    if (!g_strcmp0 (name, "IconPixmap")) return sni_icon_pixmap ();
    if (!g_strcmp0 (name, "ItemIsMenu")) return g_variant_new_boolean (TRUE);
    if (!g_strcmp0 (name, "Menu"))       return g_variant_new_object_path (SCIM_SNI_MENU_PATH);

    if (!g_strcmp0 (name, "ToolTip")) {
        // (icon name, pixmap array, title, body). The body carries the status so
        // hovering shows the current mode without opening the menu.
        GVariantBuilder pix;
        g_variant_builder_init (&pix, G_VARIANT_TYPE ("a(iiay)"));
        return g_variant_new ("(sa(iiay)ss)",
                              "",
                              &pix,
                              _sni_title.length () ? _sni_title.c_str () : "SCIM",
                              _sni_status_text.c_str ());
    }

    return 0;
}

static const GDBusInterfaceVTable _sni_vtable = {
    sni_method_call, sni_get_property, 0, { 0, 0, 0, 0, 0, 0, 0, 0 }
};

// A property was chosen in the tray menu: hand the key back to the engine, which
// advances it and reports the new label through update_property ().
static void
sni_menu_item_activated (DbusmenuMenuitem *item, guint /*timestamp*/, gpointer /*data*/)
{
    const gchar *key = (const gchar *) g_object_get_data (G_OBJECT (item), "scim-property-key");
    if (key && _panel_agent)
        _panel_agent->trigger_property (String (key));
}

// Mirror scim's property tree into dbusmenu items. scim ships a flat, key-sorted
// list whose nesting is implied by the keys (Property::is_a_leaf_of ()), the same
// walk the toolbar and the ibus frontend use.
//
// The entries carry no icons. The ones scim has are fixed bitmaps at a fixed
// size, so they neither recolour for the desktop's theme nor scale for its
// display, and several only repeat what the label already says -- the hanconv
// filter's icons against labels that read "SC->TC". A themed name (ICON_NAME) is
// the only kind of icon that belongs in a tray menu, and it is what the
// English/Keyboard entry below uses.
static void
sni_add_properties (DbusmenuMenuitem *parent,
                    PropertyRepository::const_iterator begin,
                    PropertyRepository::const_iterator end)
{
    PropertyRepository::const_iterator it = begin;

    while (it < end) {
        PropertyRepository::const_iterator child = it + 1;
        while (child < end && child->property.is_a_leaf_of (it->property))
            ++ child;

        DbusmenuMenuitem *item = dbusmenu_menuitem_new ();
        dbusmenu_menuitem_property_set (item, DBUSMENU_MENUITEM_PROP_LABEL,
                                        it->property.get_label ().c_str ());
        dbusmenu_menuitem_property_set_bool (item, DBUSMENU_MENUITEM_PROP_VISIBLE,
                                             it->property.visible ());
        dbusmenu_menuitem_property_set_bool (item, DBUSMENU_MENUITEM_PROP_ENABLED,
                                             it->property.active ());

        g_object_set_data_full (G_OBJECT (item), "scim-property-key",
                                g_strdup (it->property.get_key ().c_str ()), g_free);
        g_signal_connect (item, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                          G_CALLBACK (sni_menu_item_activated), 0);

        if (child > it + 1) {
            dbusmenu_menuitem_property_set (item, DBUSMENU_MENUITEM_PROP_CHILD_DISPLAY,
                                            DBUSMENU_MENUITEM_CHILD_DISPLAY_SUBMENU);
            sni_add_properties (item, it + 1, child);
        }

        dbusmenu_menuitem_child_append (parent, item);
        it = child;
    }
}

// An engine was chosen in the tray menu. The empty uuid is the forward /
// English-keyboard mode, exactly as the toolbar's factory menu treats it.
static void
sni_engine_activated (DbusmenuMenuitem *item, guint /*timestamp*/, gpointer /*data*/)
{
    const gchar *uuid = (const gchar *) g_object_get_data (G_OBJECT (item), "scim-factory-uuid");
    if (uuid && _panel_agent)
        _panel_agent->change_factory (String (uuid));
}

static void
sni_reload_activated (DbusmenuMenuitem * /*item*/, guint /*timestamp*/, gpointer /*data*/)
{
    if (_panel_agent) _panel_agent->reload_config ();
    if (!_config.null ()) _config->reload ();
}

static void
sni_exit_activated (DbusmenuMenuitem * /*item*/, guint /*timestamp*/, gpointer /*data*/)
{
    if (_panel_agent) _panel_agent->exit ();
}

// Opening the menu is a good moment to refresh the engine list for next time;
// the reply arrives asynchronously via do_slot_show_factory_menu ().
static void
sni_menu_about_to_show (DbusmenuMenuitem * /*item*/, gpointer /*data*/)
{
    if (_panel_agent) _panel_agent->request_factory_menu ();
}

static DbusmenuMenuitem *
sni_append_separator (DbusmenuMenuitem *root)
{
    DbusmenuMenuitem *sep = dbusmenu_menuitem_new ();
    dbusmenu_menuitem_property_set (sep, DBUSMENU_MENUITEM_PROP_TYPE, "separator");
    dbusmenu_menuitem_child_append (root, sep);
    return sep;
}

static void
sni_rebuild_menu (void)
{
    if (!_sni_menu) return;

    DbusmenuMenuitem *root = dbusmenu_menuitem_new ();

    // Turning the engine off comes first. With only engines listed there was no
    // way back to a plain keyboard from the tray at all -- the trigger hotkey was
    // the only route. An empty uuid is the agent's "turn off" signal, which the
    // frontends map to turn_off_im (), so sni_engine_activated () needs no special
    // case. The label is the same string the panel already shows as the engine
    // name while off, so it reads consistently and needs no new translation.
    {
        DbusmenuMenuitem *off = dbusmenu_menuitem_new ();
        dbusmenu_menuitem_property_set (off, DBUSMENU_MENUITEM_PROP_LABEL,
                                        _("English/Keyboard"));
        // A themed name, not a path: ICON_NAME is resolved through the icon
        // theme, and libdbusmenu-glib has no way to carry an arbitrary file
        // (property_set_image () lives in libdbusmenu-gtk, which we do not link).
        dbusmenu_menuitem_property_set (off, DBUSMENU_MENUITEM_PROP_ICON_NAME,
                                        "input-keyboard");
        g_object_set_data_full (G_OBJECT (off), "scim-factory-uuid",
                                g_strdup (""), g_free);
        g_signal_connect (off, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                          G_CALLBACK (sni_engine_activated), 0);
        dbusmenu_menuitem_child_append (root, off);
        sni_append_separator (root);
    }

    // Then the engines: this is what clicking the old toolbar's factory button
    // offered, and it is the only way to switch engines from the tray.
    for (size_t i = 0; i < _sni_factories.size (); ++i) {
        DbusmenuMenuitem *item = dbusmenu_menuitem_new ();
        dbusmenu_menuitem_property_set (item, DBUSMENU_MENUITEM_PROP_LABEL,
                                        _sni_factories[i].name.c_str ());
        g_object_set_data_full (G_OBJECT (item), "scim-factory-uuid",
                                g_strdup (_sni_factories[i].uuid.c_str ()), g_free);
        g_signal_connect (item, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                          G_CALLBACK (sni_engine_activated), 0);
        dbusmenu_menuitem_child_append (root, item);
    }

    if (_sni_factories.size ())
        sni_append_separator (root);

    sni_add_properties (root, _frontend_property_repository.begin (),
                        _frontend_property_repository.end ());

    sni_append_separator (root);

    DbusmenuMenuitem *reload = dbusmenu_menuitem_new ();
    dbusmenu_menuitem_property_set (reload, DBUSMENU_MENUITEM_PROP_LABEL, _("Reload Configuration"));
    g_signal_connect (reload, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                      G_CALLBACK (sni_reload_activated), 0);
    dbusmenu_menuitem_child_append (root, reload);

    DbusmenuMenuitem *quit = dbusmenu_menuitem_new ();
    dbusmenu_menuitem_property_set (quit, DBUSMENU_MENUITEM_PROP_LABEL, _("Exit"));
    g_signal_connect (quit, DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
                      G_CALLBACK (sni_exit_activated), 0);
    dbusmenu_menuitem_child_append (root, quit);

    g_signal_connect (root, DBUSMENU_MENUITEM_SIGNAL_ABOUT_TO_SHOW,
                      G_CALLBACK (sni_menu_about_to_show), 0);

    dbusmenu_server_set_root (_sni_menu, root);
    g_object_unref (root);
}

// The engine changed: refresh what the tray shows about it.
// The keyboard (IM off) state is the one case where a themed icon name applies,
// and a name beats anything we can rasterize: the host resolves it through the
// icon theme and recolors the symbolic variant for its own panel, so it is
// always in the desktop's own idiom. "input-keyboard" is a standard name -- the
// Icon Naming Specification lists it under Devices as "the icon used for the
// keyboard input device" -- and Adwaita and Breeze both ship it. The symbolic
// variant is not in the spec but is common and looks better in a panel, so try
// it first.
//
// Still verified rather than assumed: a theme may be incomplete, or the
// configured theme may not even be installed, and an unresolvable IconName
// leaves an invisible tray item. When nothing resolves we fall back to drawing
// the symbol.
static String
sni_themed_keyboard_icon (void)
{
    static const char * const names[] = {
        "input-keyboard-symbolic",
        "input-keyboard",
        0
    };

    GdkDisplay *display = gdk_display_get_default ();
    if (!display)
        return String ();

    GtkIconTheme *theme = gtk_icon_theme_get_for_display (display);
    if (!theme)
        return String ();

    for (int i = 0; names [i]; ++i)
        if (gtk_icon_theme_has_icon (theme, names [i]))
            return String (names [i]);

    return String ();
}

static void
sni_set_engine (const String &name, const String &icon, const String &symbol,
                bool is_keyboard)
{
    _sni_title       = name;
    _sni_icon        = icon;
    _sni_symbol      = symbol;
    _sni_is_keyboard = is_keyboard;
    // Engines carry icon *files*, which IconName cannot express, so only the
    // keyboard state can use a themed name.
    _sni_icon_name   = is_keyboard ? sni_themed_keyboard_icon () : String ();

    sni_emit ("NewIcon");
    sni_emit ("NewToolTip");
}

static void
sni_icon_theme_changed_cb (GObject * /*settings*/, GParamSpec * /*pspec*/,
                           gpointer /*data*/)
{
    // A theme switch can make the keyboard icon appear or disappear.
    String was = _sni_icon_name;
    _sni_icon_name = _sni_is_keyboard ? sni_themed_keyboard_icon () : String ();
    if (_sni_icon_name != was)
        sni_emit ("NewIcon");
}

static void
sni_set_status (const String &text)
{
    _sni_status_text = text;
    sni_emit ("NewToolTip");
}

static void
sni_register_with_host (void)
{
    if (!_sni_conn || _sni_registered) return;

    g_dbus_connection_call (_sni_conn, SCIM_SNI_WATCHER,
                            "/StatusNotifierWatcher", SCIM_SNI_WATCHER,
                            "RegisterStatusNotifierItem",
                            g_variant_new ("(s)", _sni_bus_name.c_str ()),
                            0, G_DBUS_CALL_FLAGS_NONE, -1, 0, 0, 0);
    _sni_registered = true;
}

#define SNI_PORTAL_NAME  "org.freedesktop.portal.Desktop"
#define SNI_PORTAL_PATH  "/org/freedesktop/portal/desktop"
#define SNI_PORTAL_IFACE "org.freedesktop.portal.Settings"

// Which way to ink the symbol.
//
// Nothing tells us the tray's actual background color -- SNI hands the host
// pixels and never discusses colors -- so this is a hint, not a fact, and it
// can disagree with the panel we end up sitting in. sni_render_symbol () draws
// the halo in the opposite shade for exactly that reason: when the hint is
// right the symbol looks native, and when it is wrong it is still readable.
static void
sni_set_dark (bool dark, bool from_theme)
{
    // The GTK theme wins. It is the better signal by a wide margin: the theme's
    // own text color is what every widget on the panel is drawn with, whereas
    // the portal is only a stated preference and can flatly contradict the
    // desktop -- on an XFCE session with the light "Xfce" theme it reports
    // "prefer dark", which inked the symbol light on a light panel.
    if (!from_theme && _sni_dark_from_theme)
        return;
    if (from_theme)
        _sni_dark_from_theme = true;

    if (dark == _sni_prefer_dark)
        return;

    _sni_prefer_dark = dark;
    sni_emit ("NewIcon");
}

// Ask the GTK theme for its text color and infer the tray's shade from it.
//
// The widget has to be realized inside a root before the theme applies -- a
// detached one reports the CSS default (opaque white) and would fool us every
// time. Realizing is not showing: the window is never presented, so nothing
// appears on screen, and it is destroyed immediately.
static void
sni_query_theme_dark (void)
{
    GtkWidget *win = gtk_window_new ();
    GtkWidget *label = gtk_label_new ("");

    gtk_window_set_child (GTK_WINDOW (win), label);
    gtk_widget_realize (win);

    GdkRGBA c;
    gtk_widget_get_color (label, &c);
    gtk_window_destroy (GTK_WINDOW (win));

    if (c.alpha < 0.1)
        return;                     // nothing usable; leave the portal's answer

    // Light text means a dark theme, and vice versa.
    double lum = 0.30 * c.red + 0.59 * c.green + 0.11 * c.blue;
    sni_set_dark (lum > 0.5, true);
}

static void
sni_theme_changed_cb (GObject * /*settings*/, GParamSpec * /*pspec*/,
                      gpointer /*data*/)
{
    sni_query_theme_dark ();
}

static void
sni_color_scheme_read_cb (GObject *src, GAsyncResult *res, gpointer /*data*/)
{
    GError *err = 0;
    GVariant *r = g_dbus_connection_call_finish (G_DBUS_CONNECTION (src), res, &err);
    if (!r) {
        // No portal (or an old one without this key): keep the default, which
        // inks the symbol dark with a light halo.
        if (err) g_error_free (err);
        return;
    }

    // Read () returns the value boxed twice: (v) holding a v holding the uint32.
    GVariant *outer = 0;
    g_variant_get (r, "(v)", &outer);
    if (outer) {
        GVariant *val = g_variant_is_of_type (outer, G_VARIANT_TYPE_VARIANT)
                      ? g_variant_get_variant (outer) : g_variant_ref (outer);
        if (val && g_variant_is_of_type (val, G_VARIANT_TYPE_UINT32))
            sni_set_dark (g_variant_get_uint32 (val) == 1, false);
        if (val) g_variant_unref (val);
        g_variant_unref (outer);
    }
    g_variant_unref (r);
}

static void
sni_setting_changed_cb (GDBusConnection * /*conn*/, const gchar * /*sender*/,
                        const gchar * /*path*/, const gchar * /*iface*/,
                        const gchar * /*signal*/, GVariant *params,
                        gpointer /*data*/)
{
    const gchar *ns = 0, *key = 0;
    GVariant *val = 0;
    g_variant_get (params, "(&s&sv)", &ns, &key, &val);

    if (ns && key && val &&
        !g_strcmp0 (ns, "org.freedesktop.appearance") &&
        !g_strcmp0 (key, "color-scheme") &&
        g_variant_is_of_type (val, G_VARIANT_TYPE_UINT32))
        sni_set_dark (g_variant_get_uint32 (val) == 1, false);

    if (val) g_variant_unref (val);
}

static void
sni_watch_color_scheme (GDBusConnection *conn)
{
    // Preferred source first, and follow it when the user switches theme.
    sni_query_theme_dark ();
    GtkSettings *settings = gtk_settings_get_default ();
    if (settings) {
        g_signal_connect (settings, "notify::gtk-theme-name",
                          G_CALLBACK (sni_theme_changed_cb), 0);
        g_signal_connect (settings, "notify::gtk-application-prefer-dark-theme",
                          G_CALLBACK (sni_theme_changed_cb), 0);
        g_signal_connect (settings, "notify::gtk-icon-theme-name",
                          G_CALLBACK (sni_icon_theme_changed_cb), 0);
    }

    // Asynchronous on purpose: the portal may be slow to start, or absent, and
    // the tray must not wait for it.
    g_dbus_connection_call (conn, SNI_PORTAL_NAME, SNI_PORTAL_PATH,
                            SNI_PORTAL_IFACE, "Read",
                            g_variant_new ("(ss)", "org.freedesktop.appearance",
                                           "color-scheme"),
                            G_VARIANT_TYPE ("(v)"), G_DBUS_CALL_FLAGS_NONE,
                            -1, 0, sni_color_scheme_read_cb, 0);

    g_dbus_connection_signal_subscribe (conn, SNI_PORTAL_NAME, SNI_PORTAL_IFACE,
                                        "SettingChanged", SNI_PORTAL_PATH, 0,
                                        G_DBUS_SIGNAL_FLAGS_NONE,
                                        sni_setting_changed_cb, 0, 0);
}

static void
sni_name_acquired (GDBusConnection *conn, const gchar * /*name*/, gpointer /*data*/)
{
    _sni_conn = conn;

    GDBusNodeInfo *info = g_dbus_node_info_new_for_xml (_sni_introspection_xml, 0);
    if (info) {
        _sni_object_id = g_dbus_connection_register_object (
            conn, SCIM_SNI_OBJECT_PATH, info->interfaces[0],
            &_sni_vtable, 0, 0, 0);
        g_dbus_node_info_unref (info);
    }

    if (!_sni_menu) {
        _sni_menu = dbusmenu_server_new (SCIM_SNI_MENU_PATH);
        sni_rebuild_menu ();
    }

    sni_watch_color_scheme (conn);

    sni_register_with_host ();
}

// A host may appear after us (or restart), so registration is driven by the
// watcher's presence rather than done once at startup.
static void sni_enable (void);

static void
sni_watcher_appeared (GDBusConnection * /*conn*/, const gchar * /*name*/,
                      const gchar * /*owner*/, gpointer /*data*/)
{
    // Becoming a tray item has to be driven from here rather than from a check
    // at startup: at login the panel and the tray host come up together, and
    // losing that race once meant no tray for the rest of the session.
    sni_enable ();
    _sni_registered = false;
    sni_register_with_host ();
}

static void
sni_watcher_vanished (GDBusConnection * /*conn*/, const gchar * /*name*/, gpointer /*data*/)
{
    _sni_registered = false;
}

// True when some host is offering the SNI watcher, i.e. a tray exists to sit in.
static bool
sni_host_present (void)
{
    GError *err = 0;
    GDBusConnection *bus = g_bus_get_sync (G_BUS_TYPE_SESSION, 0, &err);
    if (!bus) { if (err) g_error_free (err); return false; }

    GVariant *r = g_dbus_connection_call_sync (
        bus, "org.freedesktop.DBus", "/org/freedesktop/DBus",
        "org.freedesktop.DBus", "NameHasOwner",
        g_variant_new ("(s)", SCIM_SNI_WATCHER),
        G_VARIANT_TYPE ("(b)"), G_DBUS_CALL_FLAGS_NONE, -1, 0, &err);

    bool present = false;
    if (r) { g_variant_get (r, "(b)", &present); g_variant_unref (r); }
    if (err) g_error_free (err);
    g_object_unref (bus);
    return present;
}

// Claim the item name and export the object, i.e. actually become a tray item.
// Idempotent: whichever of startup or the watcher-appeared callback gets here
// first does the work.
static void
sni_enable (void)
{
    if (_sni_enabled)
        return;
    _sni_enabled = true;

    ui_load_config ();               // re-apply with the tray override in effect

    gchar *n = g_strdup_printf ("org.kde.StatusNotifierItem-%d-1", (int) getpid ());
    _sni_bus_name = String (n);

    _sni_name_id = g_bus_own_name (G_BUS_TYPE_SESSION, n,
                                   G_BUS_NAME_OWNER_FLAGS_NONE,
                                   0, sni_name_acquired, 0, 0, 0);
    g_free (n);
}

// Watch unconditionally. Whether a tray exists is not settled at the moment the
// panel starts, so the watch -- not a one-shot probe -- is what decides.
static void
sni_start (void)
{
    _sni_watch_id = g_bus_watch_name (G_BUS_TYPE_SESSION, SCIM_SNI_WATCHER,
                                      G_BUS_NAME_WATCHER_FLAGS_NONE,
                                      sni_watcher_appeared, sni_watcher_vanished,
                                      0, 0);
}
#endif // SCIM_HAS_SNI

//////////////////////////////////////////////////////////////////////
// Start of PanelAgent Functions
//////////////////////////////////////////////////////////////////////
static bool
initialize_panel_agent (const String &config, const String &display, bool resident)
{
    _panel_agent = new PanelAgent ();

    if (!_panel_agent->initialize (config, display, resident))
        return false;

    _panel_agent->signal_connect_transaction_start          (slot (slot_transaction_start));
    _panel_agent->signal_connect_transaction_end            (slot (slot_transaction_end));
    _panel_agent->signal_connect_reload_config              (slot (slot_reload_config));
    _panel_agent->signal_connect_turn_on                    (slot (slot_turn_on));
    _panel_agent->signal_connect_turn_off                   (slot (slot_turn_off));
    _panel_agent->signal_connect_update_screen              (slot (slot_update_screen));
    _panel_agent->signal_connect_update_factory_info        (slot (slot_update_factory_info));
    _panel_agent->signal_connect_show_help                  (slot (slot_show_help));
    _panel_agent->signal_connect_show_factory_menu          (slot (slot_show_factory_menu));
    _panel_agent->signal_connect_register_properties        (slot (slot_register_properties));
    _panel_agent->signal_connect_update_property            (slot (slot_update_property));
    _panel_agent->signal_connect_register_helper_properties (slot (slot_register_helper_properties));
    _panel_agent->signal_connect_update_helper_property     (slot (slot_update_helper_property));
    _panel_agent->signal_connect_register_helper            (slot (slot_register_helper));
    _panel_agent->signal_connect_remove_helper              (slot (slot_remove_helper));
    _panel_agent->signal_connect_lock                       (slot (slot_lock));
    _panel_agent->signal_connect_unlock                     (slot (slot_unlock));

    _panel_agent->get_helper_list (_helper_list);

    return true;
}

static bool
run_panel_agent (void)
{
    SCIM_DEBUG_MAIN(1) << "run_panel_agent ()\n";

    _panel_agent_thread = NULL;

    if (_panel_agent && _panel_agent->valid ()) {
        _panel_agent_thread = g_thread_new ("panel_agent", panel_agent_thread_func, NULL);
    }

    return (_panel_agent_thread != NULL);
}

static gpointer
panel_agent_thread_func (gpointer data)
{
    SCIM_DEBUG_MAIN(1) << "panel_agent_thread_func ()\n";

    if (!_panel_agent->run ())
        std::cerr << "Failed to run Panel.\n";

    if (_main_loop)
        g_main_loop_quit (_main_loop);

    return ((gpointer) NULL);
}

static void
start_auto_start_helpers (void)
{
    SCIM_DEBUG_MAIN(1) << "start_auto_start_helpers ()\n";

    // Add Helper object items.
    for (size_t i = 0; i < _helper_list.size (); ++i) {
        if ((_helper_list [i].option & SCIM_HELPER_AUTO_START) != 0) {
            _panel_agent->start_helper (_helper_list [i].uuid);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// slot_* : invoked on the PanelAgent thread; marshal the work to the main loop.
// (The GDK global lock and gdk_threads_* are gone in GTK4.)
/////////////////////////////////////////////////////////////////////////////
static void
slot_transaction_start (void)
{
    // Was gdk_threads_enter(); no global lock in GTK4 - handled by marshaling.
}

static void
slot_transaction_end (void)
{
    // Was gdk_threads_leave(); no global lock in GTK4 - handled by marshaling.
}

static void
slot_reload_config (void)
{
    marshal_to_main ([]{ do_slot_reload_config (); });
}

static void
slot_turn_on (void)
{
    marshal_to_main ([]{ do_slot_turn_on (); });
}

static void
slot_turn_off (void)
{
    marshal_to_main ([]{ do_slot_turn_off (); });
}

static void
slot_update_screen (int num)
{
    marshal_to_main ([num]{ do_slot_update_screen (num); });
}


static void
slot_update_factory_info (const PanelFactoryInfo &info)
{
    PanelFactoryInfo copy = info;
    marshal_to_main ([copy]{ do_slot_update_factory_info (copy); });
}

static void
slot_show_help (const String &help)
{
    String copy = help;
    marshal_to_main ([copy]{ do_slot_show_help (copy); });
}

static void
slot_show_factory_menu (const std::vector <PanelFactoryInfo> &factories)
{
    std::vector<PanelFactoryInfo> copy = factories;
    marshal_to_main ([copy]{ do_slot_show_factory_menu (copy); });
}











static void
slot_register_properties (const PropertyList &props)
{
    PropertyList copy = props;
    marshal_to_main ([copy]{ do_slot_register_properties (copy); });
}

static void
slot_update_property (const Property &prop)
{
    Property copy = prop;
    marshal_to_main ([copy]{ do_slot_update_property (copy); });
}

static void
slot_register_helper_properties (int id, const PropertyList &props)
{
    PropertyList copy = props;
    marshal_to_main ([id, copy]{ do_slot_register_helper_properties (id, copy); });
}

static void
slot_update_helper_property (int id, const Property &prop)
{
    Property copy = prop;
    marshal_to_main ([id, copy]{ do_slot_update_helper_property (id, copy); });
}

static void
slot_register_helper (int id, const HelperInfo &helper)
{
}

static void
slot_remove_helper (int id)
{
    marshal_to_main ([id]{ do_slot_remove_helper (id); });
}

static void
slot_lock (void)
{
    G_LOCK (_panel_agent_lock);
}

static void
slot_unlock (void)
{
    G_UNLOCK (_panel_agent_lock);
}

/////////////////////////////////////////////////////////////////////////////
// do_slot_* : the real work, always run on the main thread.
/////////////////////////////////////////////////////////////////////////////
static void
do_slot_reload_config (void)
{
    if (!_config.null ()) _config->reload ();
}

static void
do_slot_turn_on (void)
{
    _toolbar_should_hide = false;
    _toolbar_hidden = false;
    _panel_is_on = true;

    if (_toolbar_always_hidden)
        return;

    if (_frontend_properties_area)
        gtk_widget_hide (_frontend_properties_area);

    if (_window_stick_button)
        gtk_widget_show (_window_stick_button);

    if (_factory_button)
        gtk_widget_show (_factory_button);

    if (_client_properties_area)
        gtk_widget_show (_client_properties_area);

    if (_menu_button)
        gtk_widget_show (_menu_button);

    if (_help_button)
        gtk_widget_show (_help_button);

    if (!_toolbar_always_hidden)
        gtk_widget_show (_toolbar_window);

    ui_settle_toolbar_window (true);
}

static void
do_slot_turn_off (void)
{
    if (ui_any_menu_activated ()) return;

    _panel_is_on = false;

    if (_frontend_properties_area)
        gtk_widget_hide (_frontend_properties_area);

    if (_toolbar_always_show) {
        if (!_toolbar_hidden) {
            if (_window_stick_button)
                gtk_widget_show (_window_stick_button);

            if (_factory_button)
                gtk_widget_show (_factory_button);

            if (_client_properties_area)
                gtk_widget_show (_client_properties_area);

            if (_menu_button)
                gtk_widget_show (_menu_button);

            if (_help_button)
                gtk_widget_show (_help_button);
        }
        gtk_widget_show (_toolbar_window);
        ui_settle_toolbar_window (true);
        _toolbar_should_hide = true;
    } else {
        gtk_widget_hide (_toolbar_window);
        _toolbar_hidden = true;
    }
}

static void
do_slot_update_screen (int num)
{
    // GTK4 has a single logical screen per display; just re-settle windows.
    (void) num;
    ui_switch_screen ();
}

static void
do_slot_update_factory_info (const PanelFactoryInfo &info)
{
#ifdef SCIM_HAS_SNI
    if (_sni_enabled) {
        sni_set_engine (info.name, info.icon, info.symbol,
                        info.uuid.length () == 0);
        if (_sni_factories.empty () && _panel_agent)
            _panel_agent->request_factory_menu ();   // warm the engine list
    }
#endif

    if (_factory_button) {
        GtkWidget * newlabel = 0;

        if (_toolbar_show_factory_icon) {
            newlabel = ui_create_label (info.name,
                                        info.icon,
                                        0,
                                        !_toolbar_show_factory_name,
                                        false);
        } else {
            newlabel = gtk_label_new (info.name.c_str ());
        }

        if (newlabel) {
            gtk_button_set_child (GTK_BUTTON (_factory_button), newlabel);
        }

        if (!gtk_widget_get_visible (_factory_button) && !_toolbar_hidden)
            gtk_widget_show (_factory_button);

        gtk_widget_set_tooltip_text (_factory_button, info.name.c_str ());

        ui_settle_toolbar_window ();
    }

    if (info.uuid != "") {
        _recent_factory_uuids.remove(info.uuid);
        _recent_factory_uuids.push_front(info.uuid);
        if (_recent_factory_uuids.size () > 5)
            _recent_factory_uuids.pop_back ();
    }
}

static void
do_slot_show_help (const String &help)
{
    ui_show_help (help);
}

static void
do_slot_show_factory_menu (const std::vector <PanelFactoryInfo> &factories)
{
#ifdef SCIM_HAS_SNI
    // Tray mode: cache the list for the exported menu. Popping the toolbar's
    // GTK menu here would put an unanchorable window on screen, which is the
    // very thing the tray avoids.
    if (_sni_enabled) {
        _sni_factories = factories;
        sni_rebuild_menu ();
        return;
    }
#endif

    if (!_factory_menu_activated && factories.size ()) {
        size_t i;

        MapStringVectorSizeT groups;
        std::map<String,size_t> langs, recents;


        _factory_menu_uuids.clear ();
        _factory_menu_activated = true;

        bool use_submenus = false;
        bool show_recent = (factories.size () > 5 && _recent_factory_uuids.size ());

        for (i = 0; i < factories.size (); ++i) {
            _factory_menu_uuids.push_back (factories [i].uuid);
            langs [factories [i].lang]++;

            if (show_recent &&
                std::find (_recent_factory_uuids.begin (), _recent_factory_uuids.end (),
                           factories [i].uuid) != _recent_factory_uuids.end ()) {
                recents [factories [i].uuid] = i;
            } else {
                groups [factories [i].lang].push_back (i);
                if (groups [factories [i].lang].size () > 1)
                    use_submenus = true;
            }
        }

        use_submenus = (use_submenus && factories.size () > 9);

        if (_factory_menu) {
            panel_widget_destroy (_factory_menu);
            _factory_menu = 0;
        }

        _factory_menu = ui_menu_new ();
        GtkWidget *box = ui_menu_get_box (_factory_menu);

        GtkWidget *submenu;
        GtkWidget *submenu_button;
        guint id;
        PanelFactoryInfo info;

        // recently used factories
        if (show_recent && recents.size ()) {
            for (std::list<String>::iterator it = _recent_factory_uuids.begin (); it != _recent_factory_uuids.end (); ++it) {

                id = recents [*it];
                info = factories [id];

                ui_create_factory_menu_entry (info, id, box, true, (langs [info.lang] > 1));

                if (use_submenus) {
                    MapStringVectorSizeT::iterator g = groups.find (info.lang);
                    if (g != groups.end () && g->second.size () >= 1) {
                        g->second.push_back (id);
                    }
                }
            }

            ui_menu_append_separator (box);
        }

        for (MapStringVectorSizeT::iterator it = groups.begin (); it != groups.end (); ++ it) {
            GtkWidget *target_box = box;
            submenu = 0;
            submenu_button = 0;

            if (use_submenus && it->second.size () > 1) {
                String lang = it->first;
                // A submenu button that opens a nested popover.
                submenu_button = ui_menu_append_button (box, scim_get_language_name (lang).c_str (),
                                                        0, G_CALLBACK (ui_submenu_button_cb), 0);
                submenu = ui_menu_new ();
                gtk_widget_set_parent (submenu, submenu_button);
                gtk_popover_set_position (GTK_POPOVER (submenu), GTK_POS_RIGHT);
                g_object_set_data_full (G_OBJECT (submenu_button), "submenu",
                                        submenu, ui_destroy_popover_notify);
                target_box = ui_menu_get_box (submenu);
            }

            for (i = 0; i < it->second.size (); ++i) {
                id = it->second [i];
                info = factories [id];
                ui_create_factory_menu_entry (info, id, target_box, submenu == 0, (langs [info.lang] > 1));
            }
        }

        //Append an entry for forward mode.
        info = PanelFactoryInfo (String (""), String (_("English/Keyboard")), String ("C"), String (SCIM_KEYBOARD_ICON_FILE), String (_("En")));
        ui_create_factory_menu_entry (info, -1, box, false, true);

        g_signal_connect (G_OBJECT (_factory_menu), "closed",
                          G_CALLBACK (ui_factory_menu_deactivate_cb), NULL);

        GtkWidget *anchor = _factory_button ? _factory_button : _toolbar_hbox;
        ui_menu_popup_at (_factory_menu, anchor);
    }
}












static void
do_slot_register_properties (const PropertyList &props)
{
    register_frontend_properties (props);
}

static void
do_slot_update_property (const Property &prop)
{
    update_frontend_property (prop);
}

static void
do_slot_register_helper_properties (int id, const PropertyList &props)
{
    register_helper_properties (id, props);
}

static void
do_slot_update_helper_property (int id, const Property &prop)
{
    update_helper_property (id, prop);
}

static void
do_slot_remove_helper (int id)
{
    HelperPropertyRepository::iterator it = _helper_property_repository.find (id);

    if (it != _helper_property_repository.end () && it->second.holder)
        panel_widget_destroy (it->second.holder);

    _helper_property_repository.erase (id);
}
//////////////////////////////////////////////////////////////////////
// End of PanelAgent-Functions
//////////////////////////////////////////////////////////////////////

static GtkWidget *
create_properties_node (PropertyRepository           &repository,
                        PropertyList::const_iterator  begin,
                        PropertyList::const_iterator  end,
                        int                           client,
                        int                           level)
{
    PropertyList::const_iterator it;
    PropertyList::const_iterator next;

    GtkWidget * node;
    PropertyInfo info;
    bool leaf = true;

    if (begin >= end) return 0;

    // Both toolbar entries (level 0) and menu entries (level > 0) are buttons
    // now; the difference is only where they get packed.
    if (!level) {
        GtkWidget * label = ui_create_label (begin->get_label (),
                                             begin->get_icon (),
                                             0,
                                             !_toolbar_show_property_label,
                                             false);

        node = gtk_button_new ();
        gtk_button_set_child (GTK_BUTTON (node), label);
        gtk_widget_add_css_class (node, "flat");
    } else {
        GtkWidget * icon = ui_create_icon (begin->get_icon (), NULL, MENU_ICON_SIZE, MENU_ICON_SIZE, false);
        node = gtk_button_new ();
        gtk_widget_add_css_class (node, "flat");
        GtkWidget *h = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
        if (icon)
            gtk_box_append (GTK_BOX (h), icon);
        GtkWidget *l = gtk_label_new (begin->get_label ().c_str ());
        gtk_widget_set_halign (l, GTK_ALIGN_START);
        gtk_widget_set_hexpand (l, TRUE);
        gtk_box_append (GTK_BOX (h), l);
        gtk_button_set_child (GTK_BUTTON (node), h);
    }

    if (begin->visible ())
        gtk_widget_show (node);
    else
        gtk_widget_hide (node);

    gtk_widget_set_sensitive (node, begin->active ());

    if (begin->get_tip ().length ())
        gtk_widget_set_tooltip_text (node, begin->get_tip ().c_str ());

    g_object_set_data_full (G_OBJECT (node), "property_key", g_strdup (begin->get_key ().c_str ()), g_free);

    info.property = *begin;
    info.widget = node;

    repository.push_back (info);

    it = begin + 1;

    if (it != end) {
        GtkWidget * submenu = ui_menu_new ();
        GtkWidget * submenu_box = ui_menu_get_box (submenu);
        GtkWidget * child;

        // Create all leafs of the first child.
        while (it != end) {
            // Find all leafs of the first child.
            for (next = it + 1; next != end; ++ next)
                if (!next->is_a_leaf_of (*it)) break;

            child = create_properties_node (repository, it, next, client, level + 1);
            if (child) {
                gtk_box_append (GTK_BOX (submenu_box), child);
            }

            it = next;
        }

        // The submenu popover is parented to this node and shown on click.
        gtk_widget_set_parent (submenu, node);
        gtk_popover_set_position (GTK_POPOVER (submenu),
                                  level ? GTK_POS_RIGHT : GTK_POS_TOP);
        g_object_set_data_full (G_OBJECT (node), "property_submenu",
                                submenu, ui_destroy_popover_notify);

        g_signal_connect (G_OBJECT (submenu), "closed",
                          G_CALLBACK (ui_property_menu_deactivate_cb), NULL);

        leaf = false;
    }

    if (leaf || level == 0) {
        g_signal_connect (G_OBJECT (node), "clicked",
                          G_CALLBACK (ui_property_activate_cb),
                          GINT_TO_POINTER (client));
    }

    return node;
}

static void
create_properties (GtkWidget *container,
                   PropertyRepository &repository,
                   const PropertyList &properties,
                   int client,
                   int level)
{

    PropertyList::const_iterator it;
    PropertyList::const_iterator next;
    PropertyList::const_iterator begin = properties.begin ();
    PropertyList::const_iterator end = properties.end ();

    if (begin == end) return;

    it = begin;
    next = begin + 1;

    while (it != end) {
        if (next == end || !next->is_a_leaf_of (*it)) {
            GtkWidget * node = create_properties_node (repository, it, next, client, level);

            if (node) {
                if (!level)
                    gtk_widget_set_hexpand (node, TRUE);
                gtk_box_append (GTK_BOX (container), node);
            }
            it = next;
        }
        ++ next;
    }
}

static void
register_frontend_properties (const PropertyList &properties)
{
    bool same = true;

    PropertyList::const_iterator pit = properties.begin ();

    if (properties.size () == 0) {
        same = false;
    } else if (properties.size () == _frontend_property_repository.size ()) {
        // Check if the properties are same as old ones.
        PropertyRepository::iterator it = _frontend_property_repository.begin ();

        for (; it != _frontend_property_repository.end (); ++it, ++pit) {
            if (it->property != *pit) {
                same = false;
                break;
            }
        }
    } else {
        same = false;
    }

    // Only update the properties.
    if (same) {
        for (pit = properties.begin (); pit != properties.end (); ++pit)
            update_frontend_property (*pit);

        gtk_widget_show (_frontend_properties_area);
    } else { // Construct all properties.
        if (_frontend_properties_area)
            panel_widget_destroy (_frontend_properties_area);

        _frontend_properties_area = 0;

        _frontend_property_repository.clear ();

        if (properties.size ()) {
            _frontend_properties_area = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

            create_properties (_frontend_properties_area,
                               _frontend_property_repository,
                               properties,
                               -1,
                               0);

            gtk_widget_show (_frontend_properties_area);

            gtk_widget_set_hexpand (_frontend_properties_area, TRUE);
            gtk_box_append (GTK_BOX (_client_properties_area), _frontend_properties_area);
        }
    }

    ui_settle_toolbar_window ();

#ifdef SCIM_HAS_SNI
    if (_sni_enabled)
        sni_rebuild_menu ();
#endif
}

static void
update_frontend_property (const Property &property)
{
    update_property (_frontend_property_repository, property);

#ifdef SCIM_HAS_SNI
    if (_sni_enabled) {
        // The status property is what the tooltip reports; everything else only
        // needs the menu redrawn with its new label.
        if (property.get_key ().find ("Status") != String::npos)
            sni_set_status (property.get_label ());
        sni_rebuild_menu ();
    }
#endif
}

static void
register_helper_properties (int client, const PropertyList &properties)
{
    HelperPropertyRepository::iterator it = _helper_property_repository.find (client);

    if (it == _helper_property_repository.end ()) {
        _helper_property_repository [client] = HelperPropertyInfo ();
        it = _helper_property_repository.find (client);
    }

    if (it->second.holder)
        panel_widget_destroy (it->second.holder);

    it->second.holder = 0;

    if (properties.size ()) {
        it->second.holder = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

        create_properties (it->second.holder,
                           it->second.repository,
                           properties,
                           client,
                           0);

        gtk_widget_show (it->second.holder);
        gtk_widget_set_hexpand (it->second.holder, TRUE);
        gtk_box_append (GTK_BOX (_client_properties_area), it->second.holder);
    }

    ui_settle_toolbar_window ();
}

static void
update_helper_property (int client, const Property &property)
{
    update_property (_helper_property_repository [client].repository, property);
}

static void
update_property (PropertyRepository &repository,
                 const Property       &property)
{
    PropertyRepository::iterator it = repository.begin ();

    for (; it != repository.end (); ++ it) {
        if (it->property == property) {

            if (!it->widget) break;

            if (it->property.get_label () != property.get_label () ||
                it->property.get_icon () != property.get_icon ()) {
                if (GTK_IS_BUTTON (it->widget)) {
                    GtkWidget *label = ui_create_label (property.get_label (),
                                                        property.get_icon (),
                                                        0,
                                                        !_toolbar_show_property_label,
                                                        false);
                    gtk_button_set_child (GTK_BUTTON (it->widget), label);
                }
            }

            if (property.visible ())
                gtk_widget_show (it->widget);
            else
                gtk_widget_hide (it->widget);

            gtk_widget_set_sensitive (it->widget, property.active ());

            if (property.get_tip ().length ())
                gtk_widget_set_tooltip_text (it->widget, property.get_tip ().c_str ());

            it->property = property;
            break;
        }
    }
    ui_settle_toolbar_window ();
}

static void
restore_properties (void)
{
    PropertyList properties;

    _frontend_properties_area = 0;

    PropertyRepository::iterator it = _frontend_property_repository.begin ();
    HelperPropertyRepository::iterator helper_it = _helper_property_repository.begin ();

    for (; it != _frontend_property_repository.end (); ++it)
        properties.push_back (it->property);

    if (properties.size ()) {
        _frontend_property_repository.clear ();
        register_frontend_properties (properties);
    }

    for (; helper_it != _helper_property_repository.end (); ++ helper_it) {

        helper_it->second.holder = 0;

        properties.clear ();

        for (it = helper_it->second.repository.begin (); it != helper_it->second.repository.end (); ++it)
            properties.push_back (it->property);

        if (properties.size ()) {
            helper_it->second.repository.clear ();
            register_helper_properties (helper_it->first, properties);
        }
    }
}

static gboolean
check_exit_timeout_cb (gpointer data)
{
    G_LOCK (_global_resource_lock);
    if (_should_exit) {
        if (_main_loop)
            g_main_loop_quit (_main_loop);
    }
    G_UNLOCK (_global_resource_lock);

    return TRUE;
}

static void
signalhandler(int sig)
{
    SCIM_DEBUG_MAIN (1) << "In signal handler...\n";
    if (_panel_agent != NULL) {
        _panel_agent->stop ();
    }
}

int main (int argc, char *argv [])
{
    std::vector<String>  config_list;

    int i;

    bool daemon = false;

    String config_name ("simple");
    String display_name;
    bool should_resident = true;

    //Display version info
    std::cerr << "GTK Panel of SCIM " << SCIM_VERSION << "\n\n";

    //get modules list
    scim_get_config_module_list (config_list);

    //Add a dummy config module, it's not really a module!
    config_list.push_back ("dummy");

    //Use socket Config module as default if available.
    if (config_list.size ()) {
        if (std::find (config_list.begin (),
                       config_list.end (),
                       config_name) == config_list.end ())
            config_name = config_list [0];
    }

    DebugOutput::disable_debug (SCIM_DEBUG_AllMask);
    DebugOutput::enable_debug (SCIM_DEBUG_MainMask);

    //parse command options
    i = 0;
    while (i<argc) {
        if (++i >= argc) break;

        if (String ("-l") == argv [i] ||
            String ("--list") == argv [i]) {
            std::vector<String>::iterator it;

            std::cout << "\n";
            std::cout << "Available Config module:\n";
            for (it = config_list.begin (); it != config_list.end (); it++)
                std::cout << "    " << *it << "\n";

            return 0;
        }

        if (String ("-c") == argv [i] ||
            String ("--config") == argv [i]) {
            if (++i >= argc) {
                std::cerr << "no argument for option " << argv [i-1] << "\n";
                return -1;
            }
            config_name = argv [i];
            continue;
        }

        if (String ("-h") == argv [i] ||
            String ("--help") == argv [i]) {
            std::cout << "Usage: " << argv [0] << " [option]...\n\n"
                 << "The options are: \n"
                 << "  -l, --list           List all of available config modules.\n"
                 << "  -c, --config NAME    Uses specified Config module.\n"
                 << "  -d, --daemon         Run " << argv [0] << " as a daemon.\n"
                 << "  -ns, --no-stay       Quit if no connected client.\n"
#if ENABLE_DEBUG
                 << "  -v, --verbose LEVEL  Enable debug info, to specific LEVEL.\n"
                 << "  -o, --output FILE    Output debug information into FILE.\n"
#endif
                 << "  -h, --help           Show this help message.\n";
            return 0;
        }

        if (String ("-d") == argv [i] ||
            String ("--daemon") == argv [i]) {
            daemon = true;
            continue;
        }

        if (String ("-ns") == argv [i] ||
            String ("--no-stay") == argv [i]) {
            should_resident = false;
            continue;
        }

        if (String ("-v") == argv [i] ||
            String ("--verbose") == argv [i]) {
            if (++i >= argc) {
                std::cerr << "no argument for option " << argv [i-1] << "\n";
                return -1;
            }
            DebugOutput::set_verbose_level (atoi (argv [i]));
            continue;
        }

        if (String ("-o") == argv [i] ||
            String ("--output") == argv [i]) {
            if (++i >= argc) {
                std::cerr << "No argument for option " << argv [i-1] << "\n";
                return -1;
            }
            DebugOutput::set_output (argv [i]);
            continue;
        }

        if (String ("--") == argv [i])
            break;

        std::cerr << "Invalid command line option: " << argv [i] << "\n";
        return -1;
    } //End of command line parsing.

    if (!config_name.length ()) {
        std::cerr << "No Config module is available!\n";
        return -1;
    }

    if (config_name != "dummy") {
        //load config module
        _config_module = new ConfigModule (config_name);

        if (!_config_module || !_config_module->valid ()) {
            std::cerr << "Can not load " << config_name << " Config module.\n";
            return -1;
        }

        //create config instance
        _config = _config_module->create_config ();
    } else {
        _config = new DummyConfig ();
    }

    if (_config.null ()) {
        std::cerr << "Failed to create Config instance from "
             << config_name << " Config module.\n";
        return -1;
    }

    signal(SIGQUIT, signalhandler);
    signal(SIGTERM, signalhandler);
    signal(SIGINT,  signalhandler);
    signal(SIGHUP,  signalhandler);

    // Daemonize before touching GTK or D-Bus. Both cache a shared session-bus
    // connection whose I/O is served by a worker thread, and fork () does not
    // carry threads over -- so a connection opened before the fork is inert in
    // the child, and every call on it stalls until it times out. That silently
    // disabled the StatusNotifierItem path: not only did the host probe fail,
    // but g_bus_own_name () and the exported tray object would have used the
    // same dead connection.
    if (daemon)
        scim_daemon ();

    gtk_init ();

    ui_initialize ();

    // Our own display, used only to launch helper GUIs on the same screen.
    // The panel socket address no longer depends on it.
    {
        const char *p = gdk_display_get_name (gdk_display_get_default ());
        if (p) display_name = String (p);
    }

    if (!initialize_panel_agent (config_name, display_name, should_resident)) {
        std::cerr << "Failed to initialize Panel Agent!\n";
        return -1;
    }

    // connect the configuration reload signal.
    _config->signal_connect_reload (slot (ui_config_reload_callback));

#ifdef SCIM_HAS_SNI
    // Prefer a tray item when the desktop offers a host: it needs no positioning
    // and stays reachable, unlike a wayland toplevel. Falling back to the
    // toolbar otherwise keeps X11 and hostless desktops working as before.
    sni_start ();                        // watch for a host, now or later
    if (sni_host_present ()) {
        sni_enable ();
        std::cerr << "SCIM Panel: StatusNotifierItem host found; using the tray.\n";
    }
#endif

    if (!run_panel_agent()) {
        std::cerr << "Failed to run Socket Server!\n";
        return -1;
    }

    start_auto_start_helpers ();

    // _check_exit_timeout = g_timeout_add (500, check_exit_timeout_cb, NULL);

    _main_loop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (_main_loop);
    g_main_loop_unref (_main_loop);
    _main_loop = 0;

    // Exiting...
    g_thread_join (_panel_agent_thread);
    _config.reset ();

    std::cerr << "Successfully exited.\n";

    return 0;
}

/*
vi:ts=4:nowrap:expandtab
*/
