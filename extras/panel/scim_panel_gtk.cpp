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

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#ifdef GDK_WINDOWING_X11
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

#include "icons/up.xpm"
#include "icons/down.xpm"
#include "icons/left.xpm"
#include "icons/right.xpm"
#include "icons/setup.xpm"
#include "icons/help.xpm"
#include "icons/trademark.xpm"
#include "icons/pin-up.xpm"
#include "icons/pin-down.xpm"
#include "icons/menu.xpm"

#define SCIM_CONFIG_PANEL_GTK_FONT                      "/Panel/Gtk/Font"
#define SCIM_CONFIG_PANEL_GTK_COLOR_NORMAL_BG           "/Panel/Gtk/Color/NormalBackground"
#define SCIM_CONFIG_PANEL_GTK_COLOR_ACTIVE_BG           "/Panel/Gtk/Color/ActiveBackground"
#define SCIM_CONFIG_PANEL_GTK_COLOR_NORMAL_TEXT         "/Panel/Gtk/Color/NormalText"
#define SCIM_CONFIG_PANEL_GTK_COLOR_ACTIVE_TEXT         "/Panel/Gtk/Color/ActiveText"
#define SCIM_CONFIG_PANEL_GTK_TOOLBAR_ALWAYS_SHOW       "/Panel/Gtk/ToolBar/AlwaysShow"
#define SCIM_CONFIG_PANEL_GTK_TOOLBAR_ALWAYS_HIDDEN     "/Panel/Gtk/ToolBar/AlwaysHidden"
#define SCIM_CONFIG_PANEL_GTK_TOOLBAR_AUTO_SNAP         "/Panel/Gtk/ToolBar/AutoSnap"
#define SCIM_CONFIG_PANEL_GTK_TOOLBAR_HIDE_TIMEOUT      "/Panel/Gtk/ToolBar/HideTimeout"
#define SCIM_CONFIG_PANEL_GTK_TOOLBAR_POS_X             "/Panel/Gtk/ToolBar/POS_X"
#define SCIM_CONFIG_PANEL_GTK_TOOLBAR_POS_Y             "/Panel/Gtk/ToolBar/POS_Y"
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
    DRAG_TARGET_INPUT = 0,
    DRAG_TARGET_TOOLBAR,
    DRAG_TARGET_LOOKUP
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
static void       ui_initialize                        (void);

static void       ui_settle_input_window               (bool            relative = false,
                                                        bool            force    = false);
static void       ui_settle_lookup_table_window        (bool            force    = false);
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
static GtkWidget* ui_create_up_icon                    (void);
static GtkWidget* ui_create_down_icon                  (void);
static GtkWidget* ui_create_left_icon                  (void);
static GtkWidget* ui_create_right_icon                 (void);

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
static void       ui_preedit_area_move_cursor_cb       (ScimStringView *view,
                                                        guint           position);

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

static void       ui_lookup_table_vertical_pressed_cb  (GtkGestureClick *gesture,
                                                        int             n_press,
                                                        double          x,
                                                        double          y,
                                                        gpointer        user_data);

static void       ui_lookup_table_horizontal_click_cb  (GtkWidget      *item,
                                                        guint           position);

static void       ui_lookup_table_up_button_click_cb   (GtkButton      *button,
                                                        gpointer        user_data);
static void       ui_lookup_table_down_button_click_cb (GtkButton      *button,
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

static bool       ui_can_hide_input_window             (void);

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
static void       slot_update_spot_location            (int x, int y);
static void       slot_update_factory_info             (const PanelFactoryInfo &info);
static void       slot_show_help                       (const String &help);
static void       slot_show_factory_menu               (const std::vector <PanelFactoryInfo> &menu);
static void       slot_show_preedit_string             (void);
static void       slot_show_aux_string                 (void);
static void       slot_show_lookup_table               (void);
static void       slot_hide_preedit_string             (void);
static void       slot_hide_aux_string                 (void);
static void       slot_hide_lookup_table               (void);
static void       slot_update_preedit_string           (const String &str, const AttributeList &attrs);
static void       slot_update_preedit_caret            (int caret);
static void       slot_update_aux_string               (const String &str, const AttributeList &attrs);
static void       slot_update_lookup_table             (const LookupTable &table);
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
static void       do_slot_update_spot_location         (int x, int y);
static void       do_slot_update_factory_info          (const PanelFactoryInfo &info);
static void       do_slot_show_help                    (const String &help);
static void       do_slot_show_factory_menu            (const std::vector <PanelFactoryInfo> &menu);
static void       do_slot_show_preedit_string          (void);
static void       do_slot_show_aux_string              (void);
static void       do_slot_show_lookup_table            (void);
static void       do_slot_hide_preedit_string          (void);
static void       do_slot_hide_aux_string              (void);
static void       do_slot_hide_lookup_table            (void);
static void       do_slot_update_preedit_string        (const String &str, const AttributeList &attrs);
static void       do_slot_update_preedit_caret         (int caret);
static void       do_slot_update_aux_string            (const String &str, const AttributeList &attrs);
static void       do_slot_update_lookup_table          (const LookupTablePayload &table);
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
static GtkWidget         *_input_window                = 0;
static GtkWidget         *_preedit_area                = 0;
static GtkWidget         *_aux_area                    = 0;

static GtkWidget         *_lookup_table_window         = 0;
static GtkWidget         *_lookup_table_up_button      = 0;
static GtkWidget         *_lookup_table_down_button    = 0;
static GtkWidget         *_lookup_table_items [SCIM_LOOKUP_TABLE_MAX_PAGESIZE];

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

static gboolean           _input_window_draging        = FALSE;

static gint               _input_window_x              = 0;
static gint               _input_window_y              = 0;

static gboolean           _toolbar_window_draging      = FALSE;

static gboolean           _lookup_table_window_draging = FALSE;
static gint               _lookup_table_window_x       = 0;
static gint               _lookup_table_window_y       = 0;

// The logical position captured at drag-begin (offsets are added to it).
static gint               _drag_start_x                = 0;
static gint               _drag_start_y                = 0;

static bool               _lookup_table_embedded       = true;
static bool               _lookup_table_vertical       = false;
static bool               _window_sticked              = false;

static bool               _toolbar_always_show         = false;
static bool               _toolbar_always_hidden       = false;
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

static int                _spot_location_x             = -1;
static int                _spot_location_y             = -1;

static int                _toolbar_window_x            = -1;
static int                _toolbar_window_y            = -1;
static int                _toolbar_hide_timeout_max    = 0;
static int                _toolbar_hide_timeout_count  = 0;
static guint              _toolbar_hide_timeout        = 0;

static bool               _ui_initialized              = false;

static int                _lookup_table_index [SCIM_LOOKUP_TABLE_MAX_PAGESIZE+1];

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

        _lookup_table_vertical =
            _config->read (String (SCIM_CONFIG_PANEL_GTK_LOOKUP_TABLE_VERTICAL),
                           _lookup_table_vertical);

        _lookup_table_embedded =
            _config->read (String (SCIM_CONFIG_PANEL_GTK_LOOKUP_TABLE_EMBEDDED),
                           _lookup_table_embedded);

        _toolbar_always_show =
            _config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_ALWAYS_SHOW),
                           _toolbar_always_show);

        _toolbar_always_hidden =
            _config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_ALWAYS_HIDDEN),
                           _toolbar_always_hidden);

        // Impossible
        if (_toolbar_always_show && _toolbar_always_hidden)
            _toolbar_always_hidden = false;

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
}

// Absolute window positioning.  GTK4 removed gtk_window_move; on X11 we move
// the underlying override-ish toplevel with XMoveWindow.
static void
panel_window_move (GtkWidget *w, int x, int y)
{
    if (!w) return;

#ifdef GDK_WINDOWING_X11
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

    GtkWidget *input_window_vbox;

    ui_load_config ();
    _toolbar_hidden = false;

    if (_lookup_table_window && GTK_IS_WINDOW (_lookup_table_window))
        gtk_window_destroy (GTK_WINDOW (_lookup_table_window));
    if (_input_window) gtk_window_destroy (GTK_WINDOW (_input_window));
    if (_toolbar_window) gtk_window_destroy (GTK_WINDOW (_toolbar_window));
    if (_help_dialog) gtk_window_destroy (GTK_WINDOW (_help_dialog));

    _lookup_table_window = 0;
    _input_window = 0;
    _toolbar_window = 0;
    _toolbar_hbox = 0;
    _help_dialog = 0;
    _command_menu = 0;
    _factory_menu = 0;
    _frontend_properties_area = 0;

    // Create input window
    {
        GtkWidget *vbox;
        GtkWidget *hbox;
        GtkWidget *frame;

        _input_window = gtk_window_new ();
        gtk_window_set_decorated (GTK_WINDOW (_input_window), FALSE);
        gtk_window_set_resizable (GTK_WINDOW (_input_window), FALSE);

        // TODO(gtk4): per-widget bg/fg/font via CSS provider (was gtk_widget_modify_*).

        frame = gtk_frame_new (0);
        gtk_window_set_child (GTK_WINDOW (_input_window), frame);

        hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_frame_set_child (GTK_FRAME (frame), hbox);

        vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
        gtk_widget_set_hexpand (vbox, TRUE);
        gtk_box_append (GTK_BOX (hbox), vbox);
        input_window_vbox = vbox;

        //Create preedit area
        _preedit_area = scim_string_view_new ();
        scim_string_view_set_width_chars (SCIM_STRING_VIEW (_preedit_area), 24);
        scim_string_view_set_forward_event (SCIM_STRING_VIEW (_preedit_area), TRUE);
        scim_string_view_set_auto_resize (SCIM_STRING_VIEW (_preedit_area), TRUE);
        scim_string_view_set_has_frame (SCIM_STRING_VIEW (_preedit_area), FALSE);
        g_signal_connect (G_OBJECT (_preedit_area), "move_cursor",
                          G_CALLBACK (ui_preedit_area_move_cursor_cb),
                          0);
        gtk_widget_set_hexpand (_preedit_area, TRUE);
        gtk_box_append (GTK_BOX (vbox), _preedit_area);

        //Create aux area
        _aux_area = scim_string_view_new ();
        scim_string_view_set_width_chars (SCIM_STRING_VIEW (_aux_area), 24);
        scim_string_view_set_draw_cursor (SCIM_STRING_VIEW (_aux_area), FALSE);
        scim_string_view_set_forward_event (SCIM_STRING_VIEW (_aux_area), TRUE);
        scim_string_view_set_auto_resize (SCIM_STRING_VIEW (_aux_area), TRUE);
        scim_string_view_set_has_frame (SCIM_STRING_VIEW (_aux_area), FALSE);
        gtk_widget_set_hexpand (_aux_area, TRUE);
        gtk_box_append (GTK_BOX (vbox), _aux_area);

        // dragging support
        ui_toolbar_add_drag_controllers (_input_window, DRAG_TARGET_INPUT);

        panel_window_move (_input_window, ui_screen_width (), ui_screen_height ());
    }

    //Create lookup table window
    {
        GtkWidget *vbox;
        GtkWidget *hbox;
        GtkWidget *frame;
        GtkWidget *lookup_table_parent;
        GtkWidget *image;
        GtkWidget *separator;

        if (_lookup_table_embedded) {
            _lookup_table_window = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
            gtk_widget_set_hexpand (_lookup_table_window, TRUE);
            gtk_box_append (GTK_BOX (input_window_vbox), _lookup_table_window);
            lookup_table_parent = _lookup_table_window;
            separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
            gtk_box_append (GTK_BOX (lookup_table_parent), separator);
        } else {
            _lookup_table_window = gtk_window_new ();
            gtk_window_set_decorated (GTK_WINDOW (_lookup_table_window), FALSE);
            gtk_window_set_resizable (GTK_WINDOW (_lookup_table_window), FALSE);

            ui_toolbar_add_drag_controllers (_lookup_table_window, DRAG_TARGET_LOOKUP);

            frame = gtk_frame_new (0);
            gtk_window_set_child (GTK_WINDOW (_lookup_table_window), frame);
            lookup_table_parent = frame;
        }

        //Vertical lookup table
        if (_lookup_table_vertical) {
            vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
            if (GTK_IS_FRAME (lookup_table_parent))
                gtk_frame_set_child (GTK_FRAME (lookup_table_parent), vbox);
            else
                gtk_box_append (GTK_BOX (lookup_table_parent), vbox);

            //New table items
            for (int i=0; i<SCIM_LOOKUP_TABLE_MAX_PAGESIZE; ++i) {
                _lookup_table_items [i] = scim_string_view_new ();
                scim_string_view_set_width_chars (SCIM_STRING_VIEW (_lookup_table_items [i]), 80);
                scim_string_view_set_has_frame (SCIM_STRING_VIEW (_lookup_table_items [i]), FALSE);
                scim_string_view_set_forward_event (SCIM_STRING_VIEW (_lookup_table_items [i]), TRUE);
                scim_string_view_set_auto_resize (SCIM_STRING_VIEW (_lookup_table_items [i]), TRUE);
                scim_string_view_set_draw_cursor (SCIM_STRING_VIEW (_lookup_table_items [i]), FALSE);
                scim_string_view_set_auto_move_cursor (SCIM_STRING_VIEW (_lookup_table_items [i]), FALSE);

                GtkGesture *click = gtk_gesture_click_new ();
                gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (click), 0);
                g_signal_connect (click, "pressed",
                                  G_CALLBACK (ui_lookup_table_vertical_pressed_cb),
                                  GINT_TO_POINTER (i));
                gtk_widget_add_controller (_lookup_table_items [i], GTK_EVENT_CONTROLLER (click));

                gtk_widget_set_hexpand (_lookup_table_items [i], TRUE);
                gtk_box_append (GTK_BOX (vbox), _lookup_table_items [i]);
            }

            separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
            gtk_box_append (GTK_BOX (vbox), separator);

            hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
            gtk_widget_set_hexpand (hbox, TRUE);
            gtk_box_append (GTK_BOX (vbox), hbox);

            //New up button (leftmost)
            image = ui_create_up_icon ();
            _lookup_table_up_button = gtk_button_new ();
            gtk_button_set_child (GTK_BUTTON (_lookup_table_up_button), image);
            gtk_widget_set_halign (_lookup_table_up_button, GTK_ALIGN_END);
            gtk_box_append (GTK_BOX (hbox), _lookup_table_up_button);
            g_signal_connect (G_OBJECT (_lookup_table_up_button), "clicked",
                                G_CALLBACK (ui_lookup_table_up_button_click_cb),
                                image);

            //New down button
            image = ui_create_down_icon ();
            _lookup_table_down_button = gtk_button_new ();
            gtk_button_set_child (GTK_BUTTON (_lookup_table_down_button), image);
            gtk_widget_set_halign (_lookup_table_down_button, GTK_ALIGN_END);
            gtk_box_append (GTK_BOX (hbox), _lookup_table_down_button);
            g_signal_connect (G_OBJECT (_lookup_table_down_button), "clicked",
                                G_CALLBACK (ui_lookup_table_down_button_click_cb),
                                image);

        } else {
            hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
            if (GTK_IS_FRAME (lookup_table_parent))
                gtk_frame_set_child (GTK_FRAME (lookup_table_parent), hbox);
            else
                gtk_box_append (GTK_BOX (lookup_table_parent), hbox);

            _lookup_table_items [0] = scim_string_view_new ();
            scim_string_view_set_forward_event (SCIM_STRING_VIEW (_lookup_table_items [0]), TRUE);
            scim_string_view_set_auto_resize (SCIM_STRING_VIEW (_lookup_table_items [0]), TRUE);
            scim_string_view_set_has_frame (SCIM_STRING_VIEW (_lookup_table_items [0]), FALSE);
            scim_string_view_set_draw_cursor (SCIM_STRING_VIEW (_lookup_table_items [0]), FALSE);
            scim_string_view_set_auto_move_cursor (SCIM_STRING_VIEW (_lookup_table_items [0]), FALSE);
            g_signal_connect (G_OBJECT (_lookup_table_items [0]), "move_cursor",
                            G_CALLBACK (ui_lookup_table_horizontal_click_cb),
                            0);
            gtk_widget_set_hexpand (_lookup_table_items [0], TRUE);
            gtk_box_append (GTK_BOX (hbox), _lookup_table_items [0]);

            separator = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
            gtk_box_append (GTK_BOX (hbox), separator);

            //New left button
            image = ui_create_left_icon ();
            _lookup_table_up_button = gtk_button_new ();
            gtk_button_set_child (GTK_BUTTON (_lookup_table_up_button), image);
            gtk_box_append (GTK_BOX (hbox), _lookup_table_up_button);
            g_signal_connect (G_OBJECT (_lookup_table_up_button), "clicked",
                                G_CALLBACK (ui_lookup_table_up_button_click_cb),
                                image);

            //New right button
            image = ui_create_right_icon ();
            _lookup_table_down_button = gtk_button_new ();
            gtk_button_set_child (GTK_BUTTON (_lookup_table_down_button), image);
            gtk_box_append (GTK_BOX (hbox), _lookup_table_down_button);
            g_signal_connect (G_OBJECT (_lookup_table_down_button), "clicked",
                                G_CALLBACK (ui_lookup_table_down_button_click_cb),
                                image);
        }

        gtk_widget_add_css_class (_lookup_table_up_button, "flat");
        gtk_widget_add_css_class (_lookup_table_down_button, "flat");

        if (!_lookup_table_embedded)
            panel_window_move (_lookup_table_window, ui_screen_width (), ui_screen_height ());
    }

    //Create toolbar window
    {
        GtkWidget *hbox;
        GtkWidget *frame;
        GtkWidget *image;

        _toolbar_window = gtk_window_new ();
        gtk_window_set_decorated (GTK_WINDOW (_toolbar_window), FALSE);
        gtk_window_set_resizable (GTK_WINDOW (_toolbar_window), FALSE);

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

    //Settle input/lookup windows to default position
    {
        int spot_x, spot_y;

        spot_x = ui_screen_width () / 2 - 64;
        spot_y = ui_screen_height () * 3 / 4;
        panel_window_move (_input_window, spot_x, spot_y);
        _input_window_x = spot_x;
        _input_window_y = spot_y;

        if (!_lookup_table_embedded) {
            panel_window_move (_lookup_table_window, spot_x, spot_y + 32);
            _lookup_table_window_x = spot_x;
            _lookup_table_window_y = spot_y + 32;
        }
    }

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
ui_settle_input_window (bool relative, bool force)
{
    SCIM_DEBUG_MAIN (2) << " Settle input window...\n";

    if (_window_sticked) {
        if (force) {
            panel_window_move (_input_window, _input_window_x, _input_window_y);
        }
        return;
    }

    GtkRequisition ws;
    gint spot_x, spot_y;

    gtk_widget_get_preferred_size (_input_window, &ws, NULL);

    if (!relative) {
        spot_x = _spot_location_x;
        spot_y = _spot_location_y;
    } else {
        spot_x = _input_window_x;
        spot_y = _input_window_y;
    }

    if (spot_x < 0) spot_x = 0;
    if (spot_y < 0) spot_y = 0;

    if (spot_x + ws.width > ui_screen_width () - 4)
        spot_x = ui_screen_width () - ws.width - 4;
    if (spot_y + ws.height + 8 > ui_screen_height () - 4)
        spot_y = ui_screen_height () - ws.height - 4;

    if (spot_x != _input_window_x || spot_y != _input_window_y || force) {
        panel_window_move (_input_window, spot_x, spot_y);
        _input_window_x = spot_x;
        _input_window_y = spot_y;
    }
}

static void
ui_settle_lookup_table_window(bool force)
{
    SCIM_DEBUG_MAIN (2) << " Settle lookup table window...\n";

    if (_lookup_table_embedded)
        return;

    if (_window_sticked) {
        if (force)
            panel_window_move (_lookup_table_window, _lookup_table_window_x, _lookup_table_window_y);
        return;
    }

    gint pos_x, pos_y;

    GtkRequisition iws;
    GtkRequisition ws;

    gtk_widget_get_preferred_size (_input_window, &iws, NULL);
    gtk_widget_get_preferred_size (_lookup_table_window, &ws, NULL);

    pos_x = _input_window_x;
    pos_y = _input_window_y + iws.height + 8;

    if (pos_x + ws.width > ui_screen_width () - 8) {
        pos_x = ui_screen_width () - ws.width - 8;
    }

    if (pos_y + ws.height > ui_screen_height () - 8) {
        pos_y = ui_screen_height () - ws.height - 40;
    }

    // input window and lookup table window are overlapped.
    if (pos_y < _input_window_y + iws.height && pos_y + ws.height > _input_window_y) {
        pos_y = _input_window_y - ws.height - 8;
    }

    if (_lookup_table_window_x != pos_x || _lookup_table_window_y != pos_y || force) {
        panel_window_move (_lookup_table_window, pos_x, pos_y);
        _lookup_table_window_x = pos_x;
        _lookup_table_window_y = pos_y;
    }
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
    ui_settle_input_window ();
    ui_settle_lookup_table_window ();
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

    // TODO(gtk4): apply _default_font_desc to the label via CSS provider.

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

static GtkWidget *
ui_create_up_icon (void)
{
    return ui_create_icon (SCIM_UP_ICON_FILE,
                           (const char **) up_xpm,
                           LOOKUP_ICON_SIZE,
                           LOOKUP_ICON_SIZE);
}

static GtkWidget *
ui_create_left_icon (void)
{
    return ui_create_icon (SCIM_LEFT_ICON_FILE,
                           (const char **) left_xpm,
                           LOOKUP_ICON_SIZE,
                           LOOKUP_ICON_SIZE);
}

static GtkWidget *
ui_create_right_icon (void)
{
    return ui_create_icon (SCIM_RIGHT_ICON_FILE,
                           (const char **) right_xpm,
                           LOOKUP_ICON_SIZE,
                           LOOKUP_ICON_SIZE);
}

static GtkWidget *
ui_create_down_icon (void)
{
    return ui_create_icon (SCIM_DOWN_ICON_FILE,
                           (const char **) down_xpm,
                           LOOKUP_ICON_SIZE,
                           LOOKUP_ICON_SIZE);
}

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
ui_preedit_area_move_cursor_cb (ScimStringView *view,
                                guint           position)
{
    SCIM_DEBUG_MAIN (3) << "  ui_preedit_area_move_cursor_cb...\n";

    _panel_agent->move_preedit_caret (position);
}

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

static void
ui_lookup_table_vertical_pressed_cb (GtkGestureClick *gesture,
                                     int              n_press,
                                     double           x,
                                     double           y,
                                     gpointer         user_data)
{
    SCIM_DEBUG_MAIN (3) << "  ui_lookup_table_vertical_pressed_cb...\n";

    _panel_agent->select_candidate ((uint32)GPOINTER_TO_INT (user_data));
}

static void
ui_lookup_table_horizontal_click_cb (GtkWidget *item,
                                     guint      position)
{
    SCIM_DEBUG_MAIN (3) << "  ui_lookup_table_horizontal_click_cb...\n";

    int *index = _lookup_table_index;
    int pos = (int) position;

    for (int i=0; i<SCIM_LOOKUP_TABLE_MAX_PAGESIZE && index [i] >= 0; ++i) {
        if (pos >= index [i] && pos < index [i+1]) {
            _panel_agent->select_candidate ((uint32) i);
            return;
        }
    }
}

static void
ui_lookup_table_up_button_click_cb (GtkButton *button,
                                    gpointer user_data)
{
    SCIM_DEBUG_MAIN (3) << "  ui_lookup_table_up_button_click_cb...\n";

    _panel_agent->lookup_table_page_up ();
}

static void
ui_lookup_table_down_button_click_cb (GtkButton *button,
                                      gpointer user_data)
{
    SCIM_DEBUG_MAIN (3) << "  ui_lookup_table_down_button_click_cb...\n";

    _panel_agent->lookup_table_page_down ();
}

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
    switch (target) {
    case DRAG_TARGET_INPUT:
        *win = _input_window;   *px = &_input_window_x;        *py = &_input_window_y;        break;
    case DRAG_TARGET_TOOLBAR:
        *win = _toolbar_window; *px = &_toolbar_window_x;      *py = &_toolbar_window_y;      break;
    default:
        *win = _lookup_table_window; *px = &_lookup_table_window_x; *py = &_lookup_table_window_y; break;
    }
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

    if (target == DRAG_TARGET_INPUT)        _input_window_draging = TRUE;
    else if (target == DRAG_TARGET_TOOLBAR) _toolbar_window_draging = TRUE;
    else                                    _lookup_table_window_draging = TRUE;
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

    if (target == DRAG_TARGET_INPUT) {
        _input_window_draging = FALSE;
    } else if (target == DRAG_TARGET_TOOLBAR) {
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
    } else {
        _lookup_table_window_draging = FALSE;
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
ui_can_hide_input_window (void)
{
    if (!_panel_is_on) return true;

    if (gtk_widget_get_visible (_preedit_area) ||
        gtk_widget_get_visible (_aux_area) ||
        (_lookup_table_embedded && gtk_widget_get_visible (_lookup_table_window)))
        return false;
    return true;
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
    _panel_agent->signal_connect_update_spot_location       (slot (slot_update_spot_location));
    _panel_agent->signal_connect_update_factory_info        (slot (slot_update_factory_info));
    _panel_agent->signal_connect_show_help                  (slot (slot_show_help));
    _panel_agent->signal_connect_show_factory_menu          (slot (slot_show_factory_menu));
    _panel_agent->signal_connect_show_preedit_string        (slot (slot_show_preedit_string));
    _panel_agent->signal_connect_show_aux_string            (slot (slot_show_aux_string));
    _panel_agent->signal_connect_show_lookup_table          (slot (slot_show_lookup_table));
    _panel_agent->signal_connect_hide_preedit_string        (slot (slot_hide_preedit_string));
    _panel_agent->signal_connect_hide_aux_string            (slot (slot_hide_aux_string));
    _panel_agent->signal_connect_hide_lookup_table          (slot (slot_hide_lookup_table));
    _panel_agent->signal_connect_update_preedit_string      (slot (slot_update_preedit_string));
    _panel_agent->signal_connect_update_preedit_caret       (slot (slot_update_preedit_caret));
    _panel_agent->signal_connect_update_aux_string          (slot (slot_update_aux_string));
    _panel_agent->signal_connect_update_lookup_table        (slot (slot_update_lookup_table));
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
slot_update_spot_location (int x, int y)
{
    marshal_to_main ([x, y]{ do_slot_update_spot_location (x, y); });
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
slot_show_preedit_string (void)
{
    marshal_to_main ([]{ do_slot_show_preedit_string (); });
}

static void
slot_show_aux_string (void)
{
    marshal_to_main ([]{ do_slot_show_aux_string (); });
}

static void
slot_show_lookup_table (void)
{
    marshal_to_main ([]{ do_slot_show_lookup_table (); });
}

static void
slot_hide_preedit_string (void)
{
    marshal_to_main ([]{ do_slot_hide_preedit_string (); });
}

static void
slot_hide_aux_string (void)
{
    marshal_to_main ([]{ do_slot_hide_aux_string (); });
}

static void
slot_hide_lookup_table (void)
{
    marshal_to_main ([]{ do_slot_hide_lookup_table (); });
}

static void
slot_update_preedit_string (const String &str, const AttributeList &attrs)
{
    String s = str;
    AttributeList a = attrs;
    marshal_to_main ([s, a]{ do_slot_update_preedit_string (s, a); });
}

static void
slot_update_preedit_caret (int caret)
{
    marshal_to_main ([caret]{ do_slot_update_preedit_caret (caret); });
}

static void
slot_update_aux_string (const String &str, const AttributeList &attrs)
{
    String s = str;
    AttributeList a = attrs;
    marshal_to_main ([s, a]{ do_slot_update_aux_string (s, a); });
}

static void
slot_update_lookup_table (const LookupTable &table)
{
    // LookupTable is non-copyable; snapshot the current page here (agent
    // thread) and marshal the plain data to the main thread.
    auto p = std::make_shared<LookupTablePayload> ();

    size_t n = table.get_current_page_size ();
    p->page_size       = n;
    p->cursor_pos      = table.get_cursor_pos_in_current_page ();
    p->cursor_visible  = table.is_cursor_visible ();
    p->page_start      = table.get_current_page_start ();
    p->num_candidates  = table.number_of_candidates ();
    p->page_size_fixed = table.is_page_size_fixed ();

    for (size_t i = 0; i < n; ++i) {
        p->candidates.push_back (table.get_candidate_in_current_page (i));
        p->labels.push_back (table.get_candidate_label (i));
        p->attrs.push_back (table.get_attributes_in_current_page (i));
    }

    marshal_to_main ([p]{ do_slot_update_lookup_table (*p); });
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

    gtk_widget_hide (_lookup_table_window);
    gtk_widget_hide (_input_window);
    gtk_widget_hide (_preedit_area);
    gtk_widget_hide (_aux_area);

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

    gtk_widget_hide (_input_window);
    gtk_widget_hide (_lookup_table_window);

    gtk_widget_hide (_preedit_area);
    gtk_widget_hide (_aux_area);

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
        info = PanelFactoryInfo (String (""), String (_("English/Keyboard")), String ("C"), String (SCIM_KEYBOARD_ICON_FILE));
        ui_create_factory_menu_entry (info, -1, box, false, true);

        g_signal_connect (G_OBJECT (_factory_menu), "closed",
                          G_CALLBACK (ui_factory_menu_deactivate_cb), NULL);

        GtkWidget *anchor = _factory_button ? _factory_button : _toolbar_hbox;
        ui_menu_popup_at (_factory_menu, anchor);
    }
}

static void
do_slot_update_spot_location (int x, int y)
{
    if (x > 0 && x < ui_screen_width () && y > 0 && y < ui_screen_height ()) {
        _spot_location_x = x;
        _spot_location_y = y;

        ui_settle_input_window ();
        ui_settle_lookup_table_window ();
    }
}

static void
do_slot_show_preedit_string (void)
{
    gtk_widget_show (_preedit_area);

    if (_panel_is_on && !gtk_widget_get_visible (_input_window))
        gtk_widget_show (_input_window);

    ui_settle_input_window (true, true);
    ui_settle_lookup_table_window ();
}

static void
do_slot_show_aux_string (void)
{
    gtk_widget_show (_aux_area);

    if (_panel_is_on && !gtk_widget_get_visible (_input_window))
        gtk_widget_show (_input_window);

    ui_settle_input_window (true, true);
    ui_settle_lookup_table_window ();
}

static void
do_slot_show_lookup_table (void)
{
    gtk_widget_show (_lookup_table_window);

    if (_panel_is_on && _lookup_table_embedded && !gtk_widget_get_visible (_input_window)) {
        gtk_widget_show (_input_window);
        ui_settle_input_window (true, true);
    }

    ui_settle_lookup_table_window (true);
}

static void
do_slot_hide_preedit_string (void)
{
    gtk_widget_hide (_preedit_area);
    scim_string_view_set_text (SCIM_STRING_VIEW (_preedit_area), "");

    if (ui_can_hide_input_window ())
        gtk_widget_hide (_input_window);

    ui_settle_lookup_table_window ();
}

static void
do_slot_hide_aux_string (void)
{
    gtk_widget_hide (_aux_area);
    scim_string_view_set_text (SCIM_STRING_VIEW (_aux_area), "");

    if (ui_can_hide_input_window ())
        gtk_widget_hide (_input_window);

    ui_settle_lookup_table_window ();
}

static void
do_slot_hide_lookup_table (void)
{
    gtk_widget_hide (_lookup_table_window);

    if (_lookup_table_embedded && ui_can_hide_input_window ())
        gtk_widget_hide (_input_window);
}

static void
do_slot_update_preedit_string (const String &str, const AttributeList &attrs)
{
    PangoAttrList  *attrlist = create_pango_attrlist (str, attrs);

    scim_string_view_set_attributes (SCIM_STRING_VIEW (_preedit_area), attrlist);
    scim_string_view_set_text (SCIM_STRING_VIEW (_preedit_area), str.c_str ());

    pango_attr_list_unref (attrlist);

    ui_settle_input_window (true);

    ui_settle_lookup_table_window ();
}

static void
do_slot_update_preedit_caret (int caret)
{
    scim_string_view_set_position (SCIM_STRING_VIEW (_preedit_area), caret);
}

static void
do_slot_update_aux_string (const String &str, const AttributeList &attrs)
{
    PangoAttrList  *attrlist = create_pango_attrlist (str, attrs);

    scim_string_view_set_attributes (SCIM_STRING_VIEW (_aux_area), attrlist);
    scim_string_view_set_text (SCIM_STRING_VIEW (_aux_area), str.c_str ());

    pango_attr_list_unref (attrlist);

    ui_settle_input_window (true);

    ui_settle_lookup_table_window ();
}

static void
do_slot_update_lookup_table (const LookupTablePayload &table)
{
    size_t i;
    size_t item_num = table.page_size;

    String         mbs;
    WideString     wcs;
    WideString     label;
    GtkRequisition size;
    AttributeList  attrs;
    PangoAttrList  *attrlist;

    if (_lookup_table_vertical) {
        for (i = 0; i < SCIM_LOOKUP_TABLE_MAX_PAGESIZE; ++ i) {
            if (i < item_num) {
                mbs = String ();

                wcs = table.candidates [i];

                label = table.labels [i];

                if (label.length ()) {
                    label += utf8_mbstowcs (". ");
                } else {
                    label = utf8_mbstowcs (" ");
                }

                mbs = utf8_wcstombs (label+wcs);

                scim_string_view_set_text (SCIM_STRING_VIEW (_lookup_table_items [i]),
                                           mbs.c_str ());

                // Update attributes;
                attrs = table.attrs [i];

                if (attrs.size ()) {
                    for (AttributeList::iterator ait = attrs.begin (); ait != attrs.end (); ++ait)
                        ait->set_start (ait->get_start () + label.length ());

                    attrlist = create_pango_attrlist (mbs, attrs);
                    scim_string_view_set_attributes (SCIM_STRING_VIEW (_lookup_table_items [i]), attrlist);
                    pango_attr_list_unref (attrlist);
                } else {
                    scim_string_view_set_attributes (SCIM_STRING_VIEW (_lookup_table_items [i]), 0);
                }

                if (i == table.cursor_pos && table.cursor_visible)
                    scim_string_view_set_highlight (SCIM_STRING_VIEW (_lookup_table_items [i]),
                                                    0, wcs.length () + 3);
                else
                    scim_string_view_set_highlight (SCIM_STRING_VIEW (_lookup_table_items [i]),
                                                    -1, -1);

                gtk_widget_show (_lookup_table_items [i]);
            } else {
                gtk_widget_hide (_lookup_table_items [i]);
            }
        }
    } else {
        _lookup_table_index [0] = 0;
        for (i=0; i<SCIM_LOOKUP_TABLE_MAX_PAGESIZE; ++i) {
            if (i<item_num) {
                // Update attributes
                AttributeList item_attrs = table.attrs [i];
                size_t attr_start, attr_end;

                label = table.labels [i];

                if (label.length ()) {
                    label += utf8_mbstowcs (".");
                }

                wcs += label;

                attr_start = wcs.length ();

                wcs += table.candidates [i];

                attr_end = wcs.length ();

                wcs.push_back (0x20);

                _lookup_table_index [i+1] = wcs.length ();

                mbs = utf8_wcstombs (wcs);

                scim_string_view_set_text (SCIM_STRING_VIEW (_lookup_table_items [0]),
                                           mbs.c_str ());

                gtk_widget_get_preferred_size (_lookup_table_window, &size, NULL);

                if (size.width >= ui_screen_width () / 3 && !table.page_size_fixed) {
                    item_num = i+1;
                }

                if (item_attrs.size ()) {
                    for (AttributeList::iterator ait = item_attrs.begin (); ait != item_attrs.end (); ++ait) {
                        ait->set_start (ait->get_start () + attr_start);
                        if (ait->get_end () + attr_start > attr_end)
                            ait->set_length (attr_end - ait->get_start ());
                    }

                    attrs.insert (attrs.end (), item_attrs.begin (), item_attrs.end ());
                }

            } else {
                _lookup_table_index [i+1] = -1;
            }
        }

        if (attrs.size ()) {
            attrlist = create_pango_attrlist (mbs, attrs);
            scim_string_view_set_attributes (SCIM_STRING_VIEW (_lookup_table_items [0]), attrlist);
            pango_attr_list_unref (attrlist);
        } else {
            scim_string_view_set_attributes (SCIM_STRING_VIEW (_lookup_table_items [0]), 0);
        }

        if (table.cursor_visible) {
            int start = _lookup_table_index [table.cursor_pos];
            int end = _lookup_table_index [table.cursor_pos+1] - 1;
            scim_string_view_set_highlight (SCIM_STRING_VIEW (_lookup_table_items [0]), start, end);
        } else {
            scim_string_view_set_highlight (SCIM_STRING_VIEW (_lookup_table_items [0]), -1, -1);
        }
    }

    if (table.page_start)
        gtk_widget_set_sensitive (_lookup_table_up_button, TRUE);
    else
        gtk_widget_set_sensitive (_lookup_table_up_button, FALSE);

    if (table.page_start + item_num < table.num_candidates)
        gtk_widget_set_sensitive (_lookup_table_down_button, TRUE);
    else
        gtk_widget_set_sensitive (_lookup_table_down_button, FALSE);

    if (item_num < table.page_size)
        _panel_agent->update_lookup_table_page_size (item_num);

    if (_lookup_table_embedded)
        ui_settle_input_window (true);
    else
        ui_settle_lookup_table_window ();
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
}

static void
update_frontend_property (const Property &property)
{
    update_property (_frontend_property_repository, property);
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
                 << "  --display DISPLAY    Run on display DISPLAY.\n"
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

        if (String ("--display") == argv [i]) {
            if (++i >= argc) {
                std::cerr << "No argument for option " << argv [i-1] << "\n";
                return -1;
            }
            display_name = argv [i];
            continue;
        }

        if (String ("--") == argv [i])
            break;

        std::cerr << "Invalid command line option: " << argv [i] << "\n";
        return -1;
    } //End of command line parsing.

    // Make up DISPLAY env; GTK4's gtk_init() takes no arguments and reads
    // the environment.
    if (display_name.length ()) {
        setenv ("DISPLAY", display_name.c_str (), 1);
    }

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

    gtk_init ();

    ui_initialize ();

    // get current display.
    {
        const char *p = gdk_display_get_name (gdk_display_get_default ());
        if (p) display_name = String (p);
    }

    if (!initialize_panel_agent (config_name, display_name, should_resident)) {
        std::cerr << "Failed to initialize Panel Agent!\n";
        return -1;
    }

    if (daemon)
        scim_daemon ();

    // connect the configuration reload signal.
    _config->signal_connect_reload (slot (ui_config_reload_callback));

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
