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
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#endif
#include <gtk/gtk.h>
#include <stdlib.h>
#include <list>

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

#if ENABLE_TRAY_ICON
//  #include "scimtrayicon.h"
#endif

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
#define SCIM_CONFIG_PANEL_GTK_SHOW_TRAY_ICON            "/Panel/Gtk/ShowTrayIcon"

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
#define TRAY_ICON_SIZE                        11
#define LOOKUP_ICON_SIZE                      12

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

#if SCIM_USE_STL_EXT_HASH_MAP
typedef __gnu_cxx::hash_map <int, HelperPropertyInfo, __gnu_cxx::hash <int> > HelperPropertyRepository;
typedef __gnu_cxx::hash_map <String, std::vector <size_t>, scim_hash_string>  MapStringVectorSizeT;
#elif SCIM_USE_STL_HASH_MAP
typedef std::hash_map <int, HelperPropertyInfo, std::hash <int> >             HelperPropertyRepository;
typedef std::hash_map <String, std::vector <size_t>, scim_hash_string>        MapStringVectorSizeT;
#else
typedef std::map <int, HelperPropertyInfo>                                    HelperPropertyRepository;
typedef std::map <String, std::vector <size_t> >                              MapStringVectorSizeT;
#endif

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
static int        ui_multi_monitor_width               (void);
static int        ui_multi_monitor_height              (void);
static int        ui_screen_width                      (void);
static int        ui_screen_height                     (void);
static void       ui_get_workarea                      (int            &x,
                                                        int            &y,
                                                        int            &width,
                                                        int            &height);

#if ENABLE_TRAY_ICON
static gboolean   ui_create_tray_icon_when_idle        (gpointer        data);
#endif

#if GTK_CHECK_VERSION(2, 2, 0)
static void       ui_switch_screen                     (GdkScreen      *screen);
#endif

static GdkPixbuf* ui_scale_pixbuf                      (GdkPixbuf      *pixbuf,
                                                        int             width,
                                                        int             height);

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

static GtkWidget* ui_create_factory_menu_entry         (const PanelFactoryInfo &info,
                                                        int                    id,
                                                        GtkMenuShell           *menu,
                                                        bool                   show_lang,
                                                        bool                   show_name);

// callback functions
static void       ui_preedit_area_move_cursor_cb       (ScimStringView *view,
                                                        guint           position);

static void       ui_help_button_click_cb              (GtkButton      *button,
                                                        gpointer        user_data);
static void       ui_menu_button_click_cb              (GtkButton      *button,
                                                        gpointer        user_data);
static gboolean   ui_factory_button_click_cb           (GtkWidget      *button,
                                                        GdkEvent       *event,
                                                        gpointer        user_data);
static void       ui_factory_menu_activate_cb          (GtkMenuItem    *item,
                                                        gpointer        user_data);
static void       ui_factory_menu_deactivate_cb        (GtkMenuItem    *item,
                                                        gpointer        user_data);

static gboolean   ui_lookup_table_vertical_click_cb    (GtkWidget      *item,
                                                        GdkEventButton *event,
                                                        gpointer        user_data);

static void       ui_lookup_table_horizontal_click_cb  (GtkWidget      *item,
                                                        guint           position);

static void       ui_lookup_table_up_button_click_cb   (GtkButton      *button,
                                                        gpointer        user_data);
static void       ui_lookup_table_down_button_click_cb (GtkButton      *button,
                                                        gpointer        user_data);

static void       ui_window_stick_button_click_cb      (GtkButton      *button,
                                                        gpointer        user_data);

static gboolean   ui_input_window_motion_cb            (GtkWidget      *window,
                                                        GdkEventMotion *event,
                                                        gpointer        user_data);
static gboolean   ui_input_window_click_cb             (GtkWidget      *window,
                                                        GdkEventButton *event,
                                                        gpointer        user_data);
static gboolean   ui_toolbar_window_crossing_cb        (GtkWidget      *window,
                                                        GdkEventCrossing *event,
                                                        gpointer        user_data);
static gboolean   ui_toolbar_window_motion_cb          (GtkWidget      *window,
                                                        GdkEventMotion *event,
                                                        gpointer        user_data);
static gboolean   ui_toolbar_window_click_cb           (GtkWidget      *window,
                                                        GdkEventButton *event,
                                                        gpointer        user_data);
static gboolean   ui_lookup_table_window_motion_cb     (GtkWidget      *window,
                                                        GdkEventMotion *event,
                                                        gpointer        user_data);
static gboolean   ui_lookup_table_window_click_cb      (GtkWidget      *window,
                                                        GdkEventButton *event,
                                                        gpointer        user_data);

static gboolean   ui_hide_window_timeout_cb            (gpointer data);

static void       ui_command_menu_exit_activate_cb     (GtkMenuItem    *item,
                                                        gpointer        user_data);

static void       ui_command_menu_reload_activate_cb   (GtkMenuItem    *item,
                                                        gpointer        user_data);
static void       ui_command_menu_stick_activate_cb    (GtkMenuItem    *item,
                                                        gpointer        user_data);
static void       ui_command_menu_hide_toolbar_toggled_cb (GtkMenuItem    *item,
                                                           gpointer        user_data);
static void       ui_command_menu_help_activate_cb     (GtkMenuItem    *item,
                                                        gpointer        user_data);
static void       ui_command_menu_helper_activate_cb   (GtkWidget      *item,
                                                        gpointer        user_data);
static void       ui_command_menu_deactivate_cb        (GtkWidget      *item,
                                                        gpointer        user_data);

#if ENABLE_TRAY_ICON
#if GTK_CHECK_VERSION(3, 0, 0)
static void       ui_tray_icon_destroy_cb              (GtkWidget      *object,
#else
static void       ui_tray_icon_destroy_cb              (GtkObject      *object,
#endif
                                                        gpointer        user_data);
static void       ui_tray_icon_popup_menu_cb           (GtkStatusIcon  *status_icon,
                                                        guint           button,
                                                        guint           activate_time,
                                                        gpointer        user_data);

static void       ui_tray_icon_activate_cb             (GtkStatusIcon  *status_icon,
                                                        gpointer        user_data);
#endif

// Client Property Callback
static void       ui_property_activate_cb              (GtkWidget      *widget,
                                                        gpointer        user_data);

static void       ui_property_menu_deactivate_cb       (GtkWidget      *item,
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
#if GTK_CHECK_VERSION(2, 2, 0)
static GdkScreen         *_current_screen              = 0;
#endif

static GtkWidget         *_input_window                = 0;
static GtkWidget         *_preedit_area                = 0;
static GtkWidget         *_aux_area                    = 0;

static GtkWidget         *_lookup_table_window         = 0;
static GtkWidget         *_lookup_table_up_button      = 0;
static GtkWidget         *_lookup_table_down_button    = 0;
static GtkWidget         *_lookup_table_items [SCIM_LOOKUP_TABLE_MAX_PAGESIZE];

static GtkWidget         *_toolbar_window              = 0;
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

#if GTK_CHECK_VERSION(2, 12, 0)
#else
static GtkTooltips       *_tooltips                    = 0;
#endif

static PangoFontDescription *_default_font_desc        = 0;

#if ENABLE_TRAY_ICON
static GtkStatusIcon     *_tray_icon                   = 0;
// static GtkWidget         *_tray_icon_factory_button    = 0;
// static gulong             _tray_icon_destroy_signal_id = 0;
static bool              _tray_icon_clicked            = false;
static guint             _tray_icon_clicked_time       = 0;
#endif

static gboolean           _input_window_draging        = FALSE;
static gint               _input_window_drag_x         = 0;
static gint               _input_window_drag_y         = 0;

static gint               _input_window_x              = 0;
static gint               _input_window_y              = 0;

static gboolean           _toolbar_window_draging      = FALSE;
static gint               _toolbar_window_drag_x       = 0;
static gint               _toolbar_window_drag_y       = 0;

static gboolean           _lookup_table_window_draging = FALSE;
static gint               _lookup_table_window_drag_x  = 0;
static gint               _lookup_table_window_drag_y  = 0;
static gint               _lookup_table_window_x       = 0;
static gint               _lookup_table_window_y       = 0;

static bool               _lookup_table_embedded       = true;
static bool               _lookup_table_vertical       = false;
static bool               _window_sticked              = false;

#if ENABLE_TRAY_ICON
static bool               _show_tray_icon              = true;
#endif

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

#if GTK_CHECK_VERSION(3, 0, 0)
static GdkRGBA           _normal_bg;
static GdkRGBA           _normal_text;
static GdkRGBA           _active_bg;
static GdkRGBA           _active_text;
#else
static GdkColor           _normal_bg;
static GdkColor           _normal_text;
static GdkColor           _active_bg;
static GdkColor           _active_text;
#endif

static ConfigModule      *_config_module               = 0;
static ConfigPointer      _config;

static guint              _check_exit_timeout          = 0;

static bool               _should_exit                 = false;

static bool               _panel_is_on                 = false;

static GThread           *_panel_agent_thread          = 0;

static PanelAgent        *_panel_agent                 = 0;

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
#if GTK_CHECK_VERSION(3, 0, 0)
    gdk_rgba_parse (&_normal_bg,   "gray92");
    gdk_rgba_parse (&_normal_text, "black");
    gdk_rgba_parse (&_active_bg,   "light blue");
    gdk_rgba_parse (&_active_text, "black");
#else
    gdk_color_parse ("gray92",     &_normal_bg);
    gdk_color_parse ("black",      &_normal_text);
    gdk_color_parse ("light blue", &_active_bg);
    gdk_color_parse ("black",      &_active_text);
#endif

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
#if GTK_CHECK_VERSION(3, 0, 0)
        gdk_rgba_parse (&_normal_bg, str.c_str ());
#else
        gdk_color_parse (str.c_str (), &_normal_bg);
#endif

        str = _config->read (String (SCIM_CONFIG_PANEL_GTK_COLOR_NORMAL_TEXT),
                             String ("black"));
#if GTK_CHECK_VERSION(3, 0, 0)
        gdk_rgba_parse (&_normal_text, str.c_str ());
#else
        gdk_color_parse (str.c_str (), &_normal_text);
#endif

        str = _config->read (String (SCIM_CONFIG_PANEL_GTK_COLOR_ACTIVE_BG),
                             String ("light blue"));
#if GTK_CHECK_VERSION(3, 0, 0)
        gdk_rgba_parse (&_active_bg, str.c_str ());
#else
        gdk_color_parse (str.c_str (), &_active_bg);
#endif

        str = _config->read (String (SCIM_CONFIG_PANEL_GTK_COLOR_ACTIVE_TEXT),
                             String ("black"));
#if GTK_CHECK_VERSION(3, 0, 0)
        gdk_rgba_parse (&_active_text, str.c_str ());
#else
        gdk_color_parse (str.c_str (), &_active_text);
#endif

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

#if ENABLE_TRAY_ICON
        _show_tray_icon =
            _config->read (String (SCIM_CONFIG_PANEL_GTK_SHOW_TRAY_ICON),
                           _show_tray_icon);
#endif
    }
}

#ifdef GDK_WINDOWING_X11
static GdkFilterReturn
ui_event_filter (GdkXEvent *gdk_xevent, GdkEvent *event, gpointer data)
{
    g_return_val_if_fail (gdk_xevent, GDK_FILTER_CONTINUE);

    XEvent *xev = (XEvent*)gdk_xevent;

    if (xev->type == PropertyNotify) {
        if (xev->xproperty.atom == gdk_x11_get_xatom_by_name ("_NET_WORKAREA") ||
            xev->xproperty.atom == gdk_x11_get_xatom_by_name ("_NET_CURRENT_DESKTOP")) {
            ui_settle_toolbar_window ();
        }
    }

    return GDK_FILTER_CONTINUE;
}
#endif

static void
ui_initialize (void)
{
    SCIM_DEBUG_MAIN (1) << "Initialize UI...\n";

    GtkWidget *input_window_vbox;

    ui_load_config ();
    _toolbar_hidden = false;

    if (_lookup_table_window) gtk_widget_destroy (_lookup_table_window);
    if (_input_window) gtk_widget_destroy (_input_window);
    if (_toolbar_window) gtk_widget_destroy (_toolbar_window);
    if (_help_dialog) gtk_widget_destroy (_help_dialog);
#if GTK_CHECK_VERSION(2, 12, 0)
#else
    if (_tooltips) gtk_object_destroy (GTK_OBJECT (_tooltips));
#endif

#if ENABLE_TRAY_ICON
    if (_tray_icon) {
        // g_signal_handler_disconnect (G_OBJECT (_tray_icon),
        //                             _tray_icon_destroy_signal_id);
        g_object_unref (_tray_icon);
    }
    _tray_icon = 0;
#endif

    _lookup_table_window = 0;
    _input_window = 0;
    _toolbar_window = 0;
    _help_dialog = 0;
#if GTK_CHECK_VERSION(2, 12, 0)
#else
    _tooltips = 0;
#endif

#if GTK_CHECK_VERSION(2, 2, 0)
    // Initialize the Display and Screen.
    _current_screen  = gdk_screen_get_default ();
#endif

    // Create input window
    {
        GtkWidget *vbox;
        GtkWidget *hbox;
        GtkWidget *frame;

        _input_window = gtk_window_new (GTK_WINDOW_POPUP);
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_override_background_color (_input_window, GTK_STATE_FLAG_NORMAL, &_normal_bg);
#else
        gtk_widget_modify_bg (_input_window, GTK_STATE_NORMAL, &_normal_bg);
#endif
        gtk_window_set_resizable (GTK_WINDOW (_input_window), FALSE);
        gtk_widget_add_events (_input_window,GDK_BUTTON_PRESS_MASK);
        gtk_widget_add_events (_input_window,GDK_BUTTON_RELEASE_MASK);
        gtk_widget_add_events (_input_window,GDK_POINTER_MOTION_MASK);
        g_signal_connect (G_OBJECT (_input_window), "button-press-event",
                          G_CALLBACK (ui_input_window_click_cb),
                          GINT_TO_POINTER (0));
        g_signal_connect (G_OBJECT (_input_window), "button-release-event",
                          G_CALLBACK (ui_input_window_click_cb),
                          GINT_TO_POINTER (1));

        frame = gtk_frame_new (0);
        gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
        gtk_container_add (GTK_CONTAINER (_input_window), frame);

#if GTK_CHECK_VERSION(3, 2, 0)
        hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
#else
        hbox = gtk_hbox_new (FALSE, 0);
#endif
        gtk_container_add (GTK_CONTAINER (frame), hbox);

#if GTK_CHECK_VERSION(3, 2, 0)
        vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
#else
        vbox = gtk_vbox_new (FALSE, 0);
#endif
        gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
        input_window_vbox = vbox;

        //Create preedit area
        _preedit_area = scim_string_view_new ();
        if (_default_font_desc)
#if GTK_CHECK_VERSION(3, 0, 0)
            gtk_widget_override_font (_preedit_area, _default_font_desc);
#else
            gtk_widget_modify_font (_preedit_area, _default_font_desc);
#endif

#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_override_background_color (_preedit_area, GTK_STATE_FLAG_NORMAL, &_normal_bg);
        gtk_widget_override_background_color (_preedit_area, GTK_STATE_FLAG_ACTIVE, &_active_bg);
        gtk_widget_override_color (_preedit_area, GTK_STATE_FLAG_NORMAL, &_normal_text);
        gtk_widget_override_color (_preedit_area, GTK_STATE_FLAG_ACTIVE, &_active_text);
#else
        gtk_widget_modify_base (_preedit_area, GTK_STATE_NORMAL, &_normal_bg);
        gtk_widget_modify_base (_preedit_area, GTK_STATE_ACTIVE, &_active_bg);
        gtk_widget_modify_text (_preedit_area, GTK_STATE_NORMAL, &_normal_text);
        gtk_widget_modify_text (_preedit_area, GTK_STATE_ACTIVE, &_active_text);
#endif
        scim_string_view_set_width_chars (SCIM_STRING_VIEW (_preedit_area), 24);
        scim_string_view_set_forward_event (SCIM_STRING_VIEW (_preedit_area), TRUE);
        scim_string_view_set_auto_resize (SCIM_STRING_VIEW (_preedit_area), TRUE);
        scim_string_view_set_has_frame (SCIM_STRING_VIEW (_preedit_area), FALSE);
        g_signal_connect (G_OBJECT (_preedit_area), "move_cursor",
                          G_CALLBACK (ui_preedit_area_move_cursor_cb),
                          0);
        gtk_box_pack_start (GTK_BOX (vbox), _preedit_area, TRUE, TRUE, 0);

        //Create aux area
        _aux_area = scim_string_view_new ();
        if (_default_font_desc)
#if GTK_CHECK_VERSION(3, 0, 0)
            gtk_widget_override_font (_aux_area, _default_font_desc);
#else
            gtk_widget_modify_font (_aux_area, _default_font_desc);
#endif

#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_override_background_color (_aux_area, GTK_STATE_FLAG_NORMAL, &_normal_bg);
        gtk_widget_override_background_color (_aux_area, GTK_STATE_FLAG_ACTIVE, &_active_bg);
        gtk_widget_override_color (_aux_area, GTK_STATE_FLAG_NORMAL, &_normal_text);
        gtk_widget_override_color (_aux_area, GTK_STATE_FLAG_ACTIVE, &_active_text);
#else
        gtk_widget_modify_base (_aux_area, GTK_STATE_NORMAL, &_normal_bg);
        gtk_widget_modify_base (_aux_area, GTK_STATE_ACTIVE, &_active_bg);
        gtk_widget_modify_text (_aux_area, GTK_STATE_NORMAL, &_normal_text);
        gtk_widget_modify_text (_aux_area, GTK_STATE_ACTIVE, &_active_text);
#endif
        scim_string_view_set_width_chars (SCIM_STRING_VIEW (_aux_area), 24);
        scim_string_view_set_draw_cursor (SCIM_STRING_VIEW (_aux_area), FALSE);
        scim_string_view_set_forward_event (SCIM_STRING_VIEW (_aux_area), TRUE);
        scim_string_view_set_auto_resize (SCIM_STRING_VIEW (_aux_area), TRUE);
        scim_string_view_set_has_frame (SCIM_STRING_VIEW (_aux_area), FALSE);
        gtk_box_pack_start (GTK_BOX (vbox), _aux_area, TRUE, TRUE, 0);

        gtk_window_move (GTK_WINDOW (_input_window), ui_screen_width (), ui_screen_height ());

        gtk_widget_show_all (_input_window);
        gtk_widget_hide (_input_window);
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
#if GTK_CHECK_VERSION(3, 2, 0)
            _lookup_table_window = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
#else
            _lookup_table_window = gtk_vbox_new (FALSE, 0);
#endif
            gtk_box_pack_start (GTK_BOX (input_window_vbox), _lookup_table_window, TRUE, TRUE, 0);
            lookup_table_parent = _lookup_table_window;
#if GTK_CHECK_VERSION(3, 2, 0)
            separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
#else
            separator = gtk_hseparator_new ();
#endif
            gtk_box_pack_start (GTK_BOX (lookup_table_parent), separator, FALSE, FALSE, 0);
        } else {
            _lookup_table_window = gtk_window_new (GTK_WINDOW_POPUP);
#if GTK_CHECK_VERSION(3, 0, 0)
            gtk_widget_override_background_color (_lookup_table_window, GTK_STATE_FLAG_NORMAL, &_normal_bg);
#else
            gtk_widget_modify_bg (_lookup_table_window, GTK_STATE_NORMAL, &_normal_bg);
#endif
            gtk_window_set_resizable (GTK_WINDOW (_lookup_table_window), FALSE);
            gtk_widget_add_events (_lookup_table_window,GDK_BUTTON_PRESS_MASK);
            gtk_widget_add_events (_lookup_table_window,GDK_BUTTON_RELEASE_MASK);
            gtk_widget_add_events (_lookup_table_window,GDK_POINTER_MOTION_MASK);
            g_signal_connect (G_OBJECT (_lookup_table_window), "button-press-event",
                              G_CALLBACK (ui_lookup_table_window_click_cb),
                              GINT_TO_POINTER (0));
            g_signal_connect (G_OBJECT (_lookup_table_window), "button-release-event",
                              G_CALLBACK (ui_lookup_table_window_click_cb),
                              GINT_TO_POINTER (1));
            gtk_container_set_border_width (GTK_CONTAINER (_lookup_table_window), 0);

            frame = gtk_frame_new (0);
            gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
            gtk_container_add (GTK_CONTAINER (_lookup_table_window), frame);
            lookup_table_parent = frame;
        }

        //Vertical lookup table
        if (_lookup_table_vertical) {
#if GTK_CHECK_VERSION(3, 2, 0)
            vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
#else
            vbox = gtk_vbox_new (FALSE, 0);
#endif
            gtk_container_add (GTK_CONTAINER (lookup_table_parent), vbox);

            //New table items
            for (int i=0; i<SCIM_LOOKUP_TABLE_MAX_PAGESIZE; ++i) {
                _lookup_table_items [i] = scim_string_view_new ();
                if (_default_font_desc)
#if GTK_CHECK_VERSION(3, 0, 0)
                    gtk_widget_override_font (_lookup_table_items [i], _default_font_desc);
#else
                    gtk_widget_modify_font (_lookup_table_items [i], _default_font_desc);
#endif

#if GTK_CHECK_VERSION(3, 0, 0)
                gtk_widget_override_background_color (_lookup_table_items [i], GTK_STATE_FLAG_NORMAL, &_normal_bg);
                gtk_widget_override_background_color (_lookup_table_items [i], GTK_STATE_FLAG_ACTIVE, &_active_bg);
                gtk_widget_override_color (_lookup_table_items [i], GTK_STATE_FLAG_NORMAL, &_normal_text);
                gtk_widget_override_color (_lookup_table_items [i], GTK_STATE_FLAG_ACTIVE, &_active_text);
#else
                gtk_widget_modify_base (_lookup_table_items [i], GTK_STATE_NORMAL, &_normal_bg);
                gtk_widget_modify_base (_lookup_table_items [i], GTK_STATE_ACTIVE, &_active_bg);
                gtk_widget_modify_text (_lookup_table_items [i], GTK_STATE_NORMAL, &_normal_text);
                gtk_widget_modify_text (_lookup_table_items [i], GTK_STATE_ACTIVE, &_active_text);
#endif
                scim_string_view_set_width_chars (SCIM_STRING_VIEW (_lookup_table_items [i]), 80);
                scim_string_view_set_has_frame (SCIM_STRING_VIEW (_lookup_table_items [i]), FALSE);
                scim_string_view_set_forward_event (SCIM_STRING_VIEW (_lookup_table_items [i]), TRUE);
                scim_string_view_set_auto_resize (SCIM_STRING_VIEW (_lookup_table_items [i]), TRUE);
                scim_string_view_set_draw_cursor (SCIM_STRING_VIEW (_lookup_table_items [i]), FALSE);
                scim_string_view_set_auto_move_cursor (SCIM_STRING_VIEW (_lookup_table_items [i]), FALSE);
                g_signal_connect (G_OBJECT (_lookup_table_items [i]), "button-press-event",
                                  G_CALLBACK (ui_lookup_table_vertical_click_cb),
                                  GINT_TO_POINTER (i));
                gtk_box_pack_start (GTK_BOX (vbox), _lookup_table_items [i], TRUE, TRUE, 0);
            }

#if GTK_CHECK_VERSION(3, 2, 0)
            separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
#else
            separator = gtk_hseparator_new ();
#endif
            gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 0);

#if GTK_CHECK_VERSION(3, 2, 0)
            hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
#else
            hbox = gtk_hbox_new (FALSE, 0);
#endif
            gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

            //New down button
            image = ui_create_down_icon ();
            _lookup_table_down_button = gtk_button_new ();
            gtk_container_add (GTK_CONTAINER (_lookup_table_down_button), image);
            gtk_box_pack_end (GTK_BOX (hbox), _lookup_table_down_button, FALSE, FALSE, 0);
            g_signal_connect (G_OBJECT (_lookup_table_down_button), "clicked",
                                G_CALLBACK (ui_lookup_table_down_button_click_cb),
                                image);

            //New up button
            image = ui_create_up_icon ();
            _lookup_table_up_button = gtk_button_new ();
            gtk_container_add (GTK_CONTAINER (_lookup_table_up_button), image);
            gtk_box_pack_end (GTK_BOX (hbox), _lookup_table_up_button, FALSE, FALSE, 0);
            g_signal_connect (G_OBJECT (_lookup_table_up_button), "clicked",
                                G_CALLBACK (ui_lookup_table_up_button_click_cb),
                                image);

        } else {
#if GTK_CHECK_VERSION(3, 2, 0)
            hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
#else
            hbox = gtk_hbox_new (FALSE, 0);
#endif
            gtk_container_add (GTK_CONTAINER (lookup_table_parent), hbox);

            _lookup_table_items [0] = scim_string_view_new ();
            if (_default_font_desc)
#if GTK_CHECK_VERSION(3, 0, 0)
                gtk_widget_override_font (_lookup_table_items [0], _default_font_desc);
#else
                gtk_widget_modify_font (_lookup_table_items [0], _default_font_desc);
#endif

#if GTK_CHECK_VERSION(3, 0, 0)
            gtk_widget_override_background_color (_lookup_table_items [0], GTK_STATE_FLAG_NORMAL, &_normal_bg);
            gtk_widget_override_background_color (_lookup_table_items [0], GTK_STATE_FLAG_ACTIVE, &_active_bg);
            gtk_widget_override_color (_lookup_table_items [0], GTK_STATE_FLAG_NORMAL, &_normal_text);
            gtk_widget_override_color (_lookup_table_items [0], GTK_STATE_FLAG_ACTIVE, &_active_text);
#else
            gtk_widget_modify_base (_lookup_table_items [0], GTK_STATE_NORMAL, &_normal_bg);
            gtk_widget_modify_base (_lookup_table_items [0], GTK_STATE_ACTIVE, &_active_bg);
            gtk_widget_modify_text (_lookup_table_items [0], GTK_STATE_NORMAL, &_normal_text);
            gtk_widget_modify_text (_lookup_table_items [0], GTK_STATE_ACTIVE, &_active_text);
#endif
            scim_string_view_set_forward_event (SCIM_STRING_VIEW (_lookup_table_items [0]), TRUE);
            scim_string_view_set_auto_resize (SCIM_STRING_VIEW (_lookup_table_items [0]), TRUE);
            scim_string_view_set_has_frame (SCIM_STRING_VIEW (_lookup_table_items [0]), FALSE);
            scim_string_view_set_draw_cursor (SCIM_STRING_VIEW (_lookup_table_items [0]), FALSE);
            scim_string_view_set_auto_move_cursor (SCIM_STRING_VIEW (_lookup_table_items [0]), FALSE);
            g_signal_connect (G_OBJECT (_lookup_table_items [0]), "move_cursor",
                            G_CALLBACK (ui_lookup_table_horizontal_click_cb),
                            0);
            gtk_box_pack_start (GTK_BOX (hbox), _lookup_table_items [0], TRUE, TRUE, 0);

#if GTK_CHECK_VERSION(3, 2, 0)
            separator = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
#else
            separator = gtk_vseparator_new ();
#endif
            gtk_box_pack_start (GTK_BOX (hbox), separator, FALSE, FALSE, 0);

            //New left button
            image = ui_create_left_icon ();
            _lookup_table_up_button = gtk_button_new ();
            gtk_container_add (GTK_CONTAINER (_lookup_table_up_button), image);

            gtk_box_pack_start (GTK_BOX (hbox), _lookup_table_up_button, FALSE, FALSE, 0);
            g_signal_connect (G_OBJECT (_lookup_table_up_button), "clicked",
                                G_CALLBACK (ui_lookup_table_up_button_click_cb),
                                image);

            //New right button
            image = ui_create_right_icon ();
            _lookup_table_down_button = gtk_button_new ();
            gtk_container_add (GTK_CONTAINER (_lookup_table_down_button), image);

            gtk_box_pack_start (GTK_BOX (hbox), _lookup_table_down_button, FALSE, FALSE, 0);

            g_signal_connect (G_OBJECT (_lookup_table_down_button), "clicked",
                                G_CALLBACK (ui_lookup_table_down_button_click_cb),
                                image);
        }

        gtk_button_set_relief (GTK_BUTTON (_lookup_table_up_button), GTK_RELIEF_NONE);
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_override_background_color (_lookup_table_up_button, GTK_STATE_FLAG_ACTIVE, &_normal_bg);
        gtk_widget_override_background_color (_lookup_table_up_button, GTK_STATE_FLAG_INSENSITIVE, &_normal_bg);
        gtk_widget_override_background_color (_lookup_table_up_button, GTK_STATE_FLAG_PRELIGHT, &_normal_bg);
#else
        gtk_widget_modify_bg (_lookup_table_up_button, GTK_STATE_ACTIVE, &_normal_bg);
        gtk_widget_modify_bg (_lookup_table_up_button, GTK_STATE_INSENSITIVE, &_normal_bg);
        gtk_widget_modify_bg (_lookup_table_up_button, GTK_STATE_PRELIGHT, &_normal_bg);
#endif

        gtk_button_set_relief (GTK_BUTTON (_lookup_table_down_button), GTK_RELIEF_NONE);
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_override_background_color (_lookup_table_down_button, GTK_STATE_FLAG_ACTIVE, &_normal_bg);
        gtk_widget_override_background_color (_lookup_table_down_button, GTK_STATE_FLAG_INSENSITIVE, &_normal_bg);
        gtk_widget_override_background_color (_lookup_table_down_button, GTK_STATE_FLAG_PRELIGHT, &_normal_bg);
#else
        gtk_widget_modify_bg (_lookup_table_down_button, GTK_STATE_ACTIVE, &_normal_bg);
        gtk_widget_modify_bg (_lookup_table_down_button, GTK_STATE_INSENSITIVE, &_normal_bg);
        gtk_widget_modify_bg (_lookup_table_down_button, GTK_STATE_PRELIGHT, &_normal_bg);
#endif

        if (!_lookup_table_embedded)
            gtk_window_move (GTK_WINDOW (_lookup_table_window), ui_screen_width (), ui_screen_height ());

        gtk_widget_show_all (_lookup_table_window);
        gtk_widget_hide (_lookup_table_window);
    }

    //Create toolbar window
    {
        GtkWidget *hbox;
        GtkWidget *frame;
        GtkWidget *image;

        _toolbar_window = gtk_window_new (GTK_WINDOW_POPUP);
        gtk_window_set_resizable (GTK_WINDOW (_toolbar_window), FALSE);
        gtk_widget_add_events (_toolbar_window,GDK_BUTTON_PRESS_MASK);
        gtk_widget_add_events (_toolbar_window,GDK_BUTTON_RELEASE_MASK);
        gtk_widget_add_events (_toolbar_window,GDK_POINTER_MOTION_MASK);
        g_signal_connect (G_OBJECT (_toolbar_window), "button-press-event",
                          G_CALLBACK (ui_toolbar_window_click_cb),
                          GINT_TO_POINTER (0));
        g_signal_connect (G_OBJECT (_toolbar_window), "button-release-event",
                          G_CALLBACK (ui_toolbar_window_click_cb),
                          GINT_TO_POINTER (1));

        frame = gtk_frame_new (0);
        gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
        gtk_container_add (GTK_CONTAINER (_toolbar_window), frame);

#if GTK_CHECK_VERSION(3, 2, 0)
        hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
#else
        hbox = gtk_hbox_new (FALSE, 0);
#endif
        gtk_container_add (GTK_CONTAINER (frame), hbox);

        //New trademark pixmap
        image = ui_create_trademark_icon ();
        gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

        //New stick button
        if (_toolbar_show_stick_icon) {
            image = ui_create_stick_icon (_window_sticked);
            _window_stick_button = gtk_button_new ();
            gtk_button_set_relief (GTK_BUTTON (_window_stick_button), GTK_RELIEF_NONE);
            gtk_container_add (GTK_CONTAINER (_window_stick_button), image);
            gtk_box_pack_start (GTK_BOX (hbox), _window_stick_button, TRUE, TRUE, 0);
            g_signal_connect (G_OBJECT (_window_stick_button), "clicked",
                              G_CALLBACK (ui_window_stick_button_click_cb),
                              0);
        }

        //New factory button
        if (_toolbar_show_factory_icon || _toolbar_show_factory_name) {
            _factory_button = gtk_button_new ();
            gtk_button_set_relief (GTK_BUTTON (_factory_button), GTK_RELIEF_NONE);
            gtk_box_pack_start (GTK_BOX (hbox), _factory_button, TRUE, TRUE, 0);
            g_signal_connect (G_OBJECT (_factory_button), "button-release-event",
                              G_CALLBACK (ui_factory_button_click_cb),
                              0);
        }

        // Put all properties here
#if GTK_CHECK_VERSION(3, 2, 0)
        _client_properties_area = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
#else
        _client_properties_area = gtk_hbox_new (FALSE, 0);
#endif
        gtk_box_pack_start (GTK_BOX (hbox), _client_properties_area, TRUE, TRUE, 0);
        gtk_widget_show (_client_properties_area);

        //New menu button
        if (_toolbar_show_menu_icon) {
            image = ui_create_menu_icon ();
            _menu_button = gtk_button_new ();
            gtk_button_set_relief (GTK_BUTTON (_menu_button), GTK_RELIEF_NONE);
            gtk_container_add (GTK_CONTAINER (_menu_button), image);
            gtk_box_pack_start (GTK_BOX (hbox), _menu_button, TRUE, TRUE, 0);
            g_signal_connect (G_OBJECT (_menu_button), "clicked",
                              G_CALLBACK (ui_menu_button_click_cb),
                              image);
        }

        //New help button
        if (_toolbar_show_help_icon) {
            image = ui_create_help_icon ();
            _help_button = gtk_button_new ();
            gtk_button_set_relief (GTK_BUTTON (_help_button), GTK_RELIEF_NONE);
            gtk_container_add (GTK_CONTAINER (_help_button), image);
            gtk_box_pack_start (GTK_BOX (hbox), _help_button, TRUE, TRUE, 0);
            g_signal_connect (G_OBJECT (_help_button), "clicked",
                              G_CALLBACK (ui_help_button_click_cb),
                              image);
        }

        gtk_window_move (GTK_WINDOW (_toolbar_window), ui_screen_width (), ui_screen_height ());

        gtk_widget_show_all (_toolbar_window);
        gtk_widget_hide (_toolbar_window);

	ui_settle_toolbar_window ();
    }

    // Create help window
    {
        _help_dialog = gtk_dialog_new_with_buttons (_("SCIM Help"),
                                NULL,
                                GtkDialogFlags (0),
                                _("_OK"),
                                GTK_RESPONSE_OK,
                                NULL);

#if GTK_CHECK_VERSION(3, 0, 0)
        g_signal_connect_swapped (GTK_WIDGET (_help_dialog),
                                  "response",
                                  G_CALLBACK (gtk_widget_hide),
                                  GTK_WIDGET (_help_dialog));
#else
        g_signal_connect_swapped (GTK_OBJECT (_help_dialog),
                                  "response",
                                  G_CALLBACK (gtk_widget_hide),
                                  GTK_OBJECT (_help_dialog));
#endif

#if GTK_CHECK_VERSION(3, 0, 0)
        g_signal_connect_swapped (GTK_WIDGET (_help_dialog),
                                  "delete_event",
                                  G_CALLBACK (gtk_widget_hide_on_delete),
                                  GTK_WIDGET (_help_dialog));
#else
        g_signal_connect_swapped (GTK_OBJECT (_help_dialog),
                                  "delete_event",
                                  G_CALLBACK (gtk_widget_hide_on_delete),
                                  GTK_OBJECT (_help_dialog));
#endif

        _help_scroll = gtk_scrolled_window_new (NULL, NULL);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (_help_scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
        gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (_help_dialog))), _help_scroll, TRUE, TRUE, 0);
        gtk_widget_show (_help_scroll);

        _help_area = gtk_label_new ("");
        gtk_label_set_justify (GTK_LABEL (_help_area), GTK_JUSTIFY_LEFT);
#if GTK_CHECK_VERSION(3, 8, 0)
        gtk_container_add (GTK_CONTAINER (_help_scroll), _help_area);
#else
        gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (_help_scroll), _help_area);
#endif
        gtk_widget_show (_help_area);
    }

#if ENABLE_TRAY_ICON
    // Create Tray Icon
    {
        if (_show_tray_icon)
            ui_create_tray_icon_when_idle (0);
    }
#endif

    //Settle input/lookup windows to default position
    {
        uint32 spot_x, spot_y;

        spot_x = ui_screen_width () / 2 - 64;
        spot_y = ui_screen_height () * 3 / 4;
        gtk_window_move (GTK_WINDOW (_input_window), spot_x, spot_y);

        if (!_lookup_table_embedded)
            gtk_window_move (GTK_WINDOW (_lookup_table_window), spot_x, spot_y + 32);
    }

    //Init timeout callback
    if (_toolbar_hide_timeout != 0) {
        g_source_remove (_toolbar_hide_timeout);
        _toolbar_hide_timeout = 0;

    }
    if (_toolbar_always_show && _toolbar_hide_timeout_max > 0) {
        _toolbar_hide_timeout =
#if GTK_CHECK_VERSION(2, 12, 0)
        gdk_threads_add_timeout
#else
        g_timeout_add
#endif
            (1000, ui_hide_window_timeout_cb, NULL);
        g_signal_connect (G_OBJECT (_toolbar_window), "enter-notify-event",
                          G_CALLBACK (ui_toolbar_window_crossing_cb),
                          GINT_TO_POINTER (0));
        g_signal_connect (G_OBJECT (_toolbar_window), "leave-notify-event",
                          G_CALLBACK (ui_toolbar_window_crossing_cb),
                          GINT_TO_POINTER (1));
    }

    // Init the tooltips
    {
#if GTK_CHECK_VERSION(2, 12, 0)
#else
        _tooltips = gtk_tooltips_new ();

        gtk_tooltips_set_delay (_tooltips, 1000);
#endif

        if (_window_stick_button)
#if GTK_CHECK_VERSION(2, 12, 0)
            gtk_widget_set_tooltip_text (_window_stick_button,
                                  _("Stick/unstick the input window and the toolbar."));
#else
            gtk_tooltips_set_tip (_tooltips, _window_stick_button,
                                  _("Stick/unstick the input window and the toolbar."),
                                  NULL);
#endif

        if (_help_button)
#if GTK_CHECK_VERSION(2, 12, 0)
            gtk_widget_set_tooltip_text (_help_button,
                                  _("Show a brief help about SCIM and the current input method."));
#else
            gtk_tooltips_set_tip (_tooltips, _help_button,
                                  _("Show a brief help about SCIM and the current input method."),
#endif

        if (_menu_button)
#if GTK_CHECK_VERSION(2, 12, 0)
            gtk_widget_set_tooltip_text (_menu_button,
                                  _("Show command menu."));
#else
            gtk_tooltips_set_tip (_tooltips, _menu_button,
                                  _("Show command menu."),
                                  NULL);
#endif
    }

    /*
#ifdef GDK_WINDOWING_X11
    // Add an event filter function to observe X root window's properties.
    GdkWindow *root_window = gdk_get_default_root_window ();
    GdkEventMask event_mask;
#if GTK_CHECK_VERSION(2, 2, 0)
    if (_current_screen)
        root_window = gdk_screen_get_root_window (_current_screen);
#endif
    event_mask = (GdkEventMask) (gdk_window_get_events (root_window) | GDK_PROPERTY_NOTIFY);
    gdk_window_set_events (root_window, event_mask);
    gdk_window_add_filter (root_window, ui_event_filter, NULL);
#endif
    */

    _ui_initialized = true;
}

static void
ui_settle_input_window (bool relative, bool force)
{
    SCIM_DEBUG_MAIN (2) << " Settle input window...\n";

    if (_window_sticked) {
        if (force)
            gtk_window_move (GTK_WINDOW (_input_window), _input_window_x, _input_window_y);
        return;
    }

    GtkRequisition ws;
    gint spot_x, spot_y;

#if GTK_CHECK_VERSION(3, 0, 0)
    gtk_widget_get_preferred_size (_input_window, &ws, NULL);
#else
    gtk_widget_size_request (_input_window, &ws);
#endif

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
        gtk_window_move (GTK_WINDOW (_input_window), spot_x, spot_y);
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
            gtk_window_move (GTK_WINDOW (_lookup_table_window), _lookup_table_window_x, _lookup_table_window_y);
        return;
    }

    gint pos_x, pos_y;

    GtkRequisition iws;
    GtkRequisition ws;

#if GTK_CHECK_VERSION(3, 0, 0)
    gtk_widget_get_preferred_size (_input_window, &iws, NULL);
    gtk_widget_get_preferred_size (_lookup_table_window, &ws, NULL);
#else
    gtk_widget_size_request (_input_window, &iws);
    gtk_widget_size_request (_lookup_table_window, &ws);
#endif

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
        gtk_window_move (GTK_WINDOW (_lookup_table_window), pos_x, pos_y);
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
            gtk_window_move (GTK_WINDOW (_toolbar_window), _toolbar_window_x, _toolbar_window_y);
        return;
    }

    gint workarea_x, workarea_y, workarea_width, workarea_height;
    ui_get_workarea (workarea_x, workarea_y, workarea_width, workarea_height);

    GtkRequisition ws;
    gint pos_x, pos_y;

#if GTK_CHECK_VERSION(3, 0, 0)
    gtk_widget_get_preferred_size (_toolbar_window, &ws, NULL);
#else
    gtk_widget_size_request (_toolbar_window, &ws);
#endif

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
        gtk_window_move (GTK_WINDOW (_toolbar_window), pos_x, pos_y);
        _toolbar_window_x = pos_x;
        _toolbar_window_y = pos_y;
    }
}

static bool
ui_get_screen_rect (GdkRectangle &rect)
{
#if GTK_CHECK_VERSION(2, 2, 0)
	GdkWindow * active_window;
    int index;

    if (_current_screen)
    {
        if ( gdk_screen_get_n_monitors (_current_screen) > 1)
        {
            _multi_monitors = true;
            active_window = gdk_screen_get_active_window(_current_screen);
            index = gdk_screen_get_monitor_at_window(_current_screen, active_window);
            gdk_screen_get_monitor_geometry(_current_screen, index, &rect);
            return TRUE;
        }
    }
#endif
    return FALSE;
}

static int
ui_multi_monitor_width ()
{
#if GTK_CHECK_VERSION(2, 2, 0)
	GdkRectangle rect;

    if (_current_screen)
    {
        if ( ui_get_screen_rect (rect) )
        {
            return rect.x + rect.width;
        }

        return gdk_screen_get_width (_current_screen);
    }
#endif
    return 0;
}

static int
ui_multi_monitor_height ()
{
#if GTK_CHECK_VERSION(2, 2, 0)
	GdkRectangle rect;

    if (_current_screen)
    {
        if ( ui_get_screen_rect (rect) )
        {
            return rect.y + rect.height;
        }

        return gdk_screen_get_height (_current_screen);
    }
#endif
    return 0;
}

static int
ui_screen_width (void)
{
#if GTK_CHECK_VERSION(2, 2, 0)
    if (_current_screen)
        return ui_multi_monitor_width ();
#endif
    return gdk_screen_width ();
}

static int
ui_screen_height (void)
{
#if GTK_CHECK_VERSION(2, 2, 0)
    if (_current_screen)
        return ui_multi_monitor_height ();
#endif
    return gdk_screen_height ();
}

static void
ui_get_workarea (int &x, int &y, int &width, int &height)
{
    static GdkAtom net_current_desktop_atom = gdk_atom_intern ("_NET_CURRENT_DESKTOP", TRUE);;
    static GdkAtom net_workarea_atom = gdk_atom_intern ("_NET_WORKAREA", TRUE);
    GdkWindow *root_window = gdk_get_default_root_window ();
    GdkAtom atom_ret;
    gint format, length, current_desktop = 0;
    guchar *data;

#if GTK_CHECK_VERSION(2, 2, 0)
    if (_current_screen)
        root_window = gdk_screen_get_root_window (_current_screen);
#endif

    x = 0;
    y = 0;
    width = ui_screen_width ();
    height = ui_screen_height ();

    if (net_current_desktop_atom != GDK_NONE) {
        gboolean found = gdk_property_get (root_window,
                                           net_current_desktop_atom, GDK_NONE, 0, G_MAXLONG, FALSE,
                                           &atom_ret, &format, &length, &data);
        if (found && format == 32 && length / sizeof(glong) > 0)
            current_desktop = ((glong*)data)[0];
        if (found)
            g_free (data);
    }

    if (net_workarea_atom != GDK_NONE) {
        gboolean found = gdk_property_get (root_window,
                                           net_workarea_atom, GDK_NONE, 0, G_MAXLONG, FALSE,
                                           &atom_ret, &format, &length, &data);
        if (found && format == 32 && length / sizeof(glong) >= (current_desktop + 1) * 4) {
            x      = ((glong*)data)[current_desktop * 4];
            y      = ((glong*)data)[current_desktop * 4 + 1];
            width  = ((glong*)data)[current_desktop * 4 + 2];
            height = ((glong*)data)[current_desktop * 4 + 3];
        }
        if (found)
            g_free (data);
    }
}

#if GTK_CHECK_VERSION(2, 2, 0)
static void
ui_switch_screen (GdkScreen *screen)
{
    if (screen) {
        if (_input_window) {
            gtk_window_set_screen (GTK_WINDOW (_input_window), screen);

            _input_window_x = ui_screen_width ();
            _input_window_y = ui_screen_height ();

            gtk_window_move (GTK_WINDOW (_input_window), _input_window_x, _input_window_y);
        }

        if (_toolbar_window) {
            gtk_window_set_screen (GTK_WINDOW (_toolbar_window), screen);
	    ui_settle_toolbar_window ();
        }

        if (!_lookup_table_embedded && _lookup_table_window) {
            gtk_window_set_screen (GTK_WINDOW (_lookup_table_window), screen);

            _lookup_table_window_x = ui_screen_width ();
            _lookup_table_window_y = ui_screen_height ();

            gtk_window_move (GTK_WINDOW (_lookup_table_window), _lookup_table_window_x, _lookup_table_window_y);
        }

#if ENABLE_TRAY_ICON
        // if (_tray_icon) {
        //     gtk_window_set_screen (GTK_WINDOW (_tray_icon), screen);
        // }
#endif

        if (_help_dialog) {
            gtk_window_set_screen (GTK_WINDOW (_help_dialog), screen);
        }

        /*
#ifdef GDK_WINDOWING_X11
        GdkWindow *root_window = gdk_get_default_root_window ();
        GdkEventMask event_mask;
        if (_current_screen)
            root_window = gdk_screen_get_root_window (_current_screen);
        event_mask = (GdkEventMask) (gdk_window_get_events (root_window) | GDK_PROPERTY_NOTIFY);
        gdk_window_set_events (root_window, event_mask);
        gdk_window_add_filter (root_window, ui_event_filter, NULL);
#endif
        */

        ui_settle_input_window ();
        ui_settle_lookup_table_window ();
        ui_settle_toolbar_window ();
    }
}
#endif

#if ENABLE_TRAY_ICON
// static gboolean
// ui_tray_icon_expose_event_cb (GtkWidget *widget, GdkEventExpose *event)
// {
//     gdk_window_clear_area (widget->window, event->area.x, event->area.y,
//                            event->area.width, event->area.height);
//     return FALSE;
// }
//
// static void
// ui_tray_icon_style_set_cb (GtkWidget *widget, GtkStyle *previous_style)
// {
//     gdk_window_set_back_pixmap (widget->window, NULL, TRUE);
// }
//
// static void
// ui_tray_icon_realize_cb (GtkWidget *widget)
// {
// #if GTK_CHECK_VERSION(2, 18, 0)
//     if (!gtk_widget_get_has_window (widget) || gtk_widget_get_app_paintable (widget))
// #else
//     if (GTK_WIDGET_NO_WINDOW (widget) || GTK_WIDGET_APP_PAINTABLE (widget))
// #endif
//         return;
//
//     gtk_widget_set_app_paintable (widget, TRUE);
//     gtk_widget_set_double_buffered (widget, FALSE);
//     gdk_window_set_back_pixmap (widget->window, NULL, TRUE);
//     g_signal_connect (widget, "expose_event",
//                       G_CALLBACK (ui_tray_icon_expose_event_cb), NULL);
//     g_signal_connect_after (widget, "style_set",
//                             G_CALLBACK (ui_tray_icon_style_set_cb), NULL);
// }

static gboolean
ui_create_tray_icon_when_idle (gpointer data)
{
    GtkWidget *image;

    // TODO use GtkNotification?
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    _tray_icon = gtk_status_icon_new_from_file (SCIM_KEYBOARD_ICON_FILE);
    G_GNUC_END_IGNORE_DEPRECATIONS
    // g_signal_connect (G_OBJECT (_tray_icon), "realize",
    //                   G_CALLBACK (ui_tray_icon_realize_cb), NULL);

    // _tray_icon_destroy_signal_id =
    // g_signal_connect (G_OBJECT (_tray_icon), "destroy",
    //                   G_CALLBACK (ui_tray_icon_destroy_cb),
    //                   0);

    // image = ui_create_icon (SCIM_KEYBOARD_ICON_FILE,
    //                         NULL,
    //                         TRAY_ICON_SIZE,
    //                         TRAY_ICON_SIZE,
    //                         true);

    // _tray_icon_factory_button = gtk_event_box_new ();
    // g_signal_connect (G_OBJECT (_tray_icon_factory_button), "realize",
    //                   G_CALLBACK (ui_tray_icon_realize_cb), NULL);
    // gtk_container_add (GTK_CONTAINER (_tray_icon_factory_button), image);
    // gtk_container_add (GTK_CONTAINER (_tray_icon), _tray_icon_factory_button);


    // g_signal_connect (G_OBJECT (_tray_icon_factory_button), "button-release-event",
    //                   G_CALLBACK (ui_factory_button_click_cb),
    //                   0);

    g_signal_connect (G_OBJECT (_tray_icon), "popup-menu",
                      G_CALLBACK (ui_tray_icon_popup_menu_cb),
                      0);

    g_signal_connect (G_OBJECT (_tray_icon), "activate",
                      G_CALLBACK (ui_tray_icon_activate_cb),
                      0);

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gtk_status_icon_set_visible (_tray_icon, TRUE);
    G_GNUC_END_IGNORE_DEPRECATIONS

    return FALSE;
}
#endif

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
ui_create_label (const String   &name,
                 const String   &iconfile,
                 const char    **xpm,
                 bool            show_icon_only,
                 bool            force_icon)
{
#if GTK_CHECK_VERSION(3, 2, 0)
    GtkWidget * hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
#else
    GtkWidget * hbox = gtk_hbox_new (FALSE, 0);
#endif
    GtkWidget * label = gtk_label_new (name.c_str ());

    gint width, height;

    if (_default_font_desc)
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_override_font (label, _default_font_desc);
#else
        gtk_widget_modify_font (label, _default_font_desc);
#endif

    gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &width, &height);

    GtkWidget *icon = ui_create_icon (iconfile,
                                      xpm,
                                      width,
                                      height,
                                      force_icon);

    if (icon) {
        gtk_box_pack_start (GTK_BOX (hbox), icon, FALSE, FALSE, 0);
        if (!show_icon_only)
            gtk_box_set_spacing (GTK_BOX (hbox), 4);
    }

    if (!show_icon_only || !icon)
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    else
        gtk_widget_destroy (label);

    gtk_widget_show_all (hbox);

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

        GtkWidget *icon = gtk_image_new_from_pixbuf (pixbuf);
        gtk_widget_show (icon);

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

static GtkWidget*
ui_create_factory_menu_entry (const PanelFactoryInfo &info,
                              gint                   id,
                              GtkMenuShell           *menu,
                              bool                   show_lang,
                              bool                   show_name)
{
    gint width, height;
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

    menu_item =
#if GTK_CHECK_VERSION(3, 10, 0)
        gtk_menu_item_new_with_label
#else
        gtk_image_menu_item_new_with_label
#endif
            (text.c_str ());
    gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &width, &height);
#if !GTK_CHECK_VERSION(3, 10, 0)
    icon_image = ui_create_icon (info.icon, NULL, width, height, false);
    if (icon_image)
        gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon_image);
#endif

    g_signal_connect (G_OBJECT (menu_item), "activate",
                      G_CALLBACK (ui_factory_menu_activate_cb),
                      GINT_TO_POINTER ((gint)id));
    gtk_widget_show (menu_item);

    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

    if (tooltip != "")
#if GTK_CHECK_VERSION(2, 12, 0)
        gtk_widget_set_tooltip_text (menu_item, tooltip.c_str ());
#else
        gtk_tooltips_set_tip (_tooltips, menu_item, tooltip.c_str (), NULL);
#endif

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

#if GTK_CHECK_VERSION(2, 18, 0)
    if (gtk_widget_get_visible (_help_dialog)) {
#else
    if (GTK_WIDGET_VISIBLE (_help_dialog)) {
#endif
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

static gboolean
ui_factory_button_click_cb (GtkWidget *button,
                            GdkEvent  *event,
                            gpointer   user_data)
{
    SCIM_DEBUG_MAIN (3) << "  ui_factory_button_click_cb...\n";

    GdkEventButton *bevent = (GdkEventButton *) event;

    struct timeval cur_time;
    gettimeofday (&cur_time, 0);

    if (cur_time.tv_sec < _last_menu_deactivate_time.tv_sec ||
        (cur_time.tv_sec == _last_menu_deactivate_time.tv_sec &&
         cur_time.tv_usec < _last_menu_deactivate_time.tv_usec + 200000))
        return FALSE;

    if (bevent->button <= 1)
        _panel_agent->request_factory_menu ();
    else
        action_show_command_menu ();

    return FALSE;
}

static void
ui_tray_icon_popup_menu_cb (GtkStatusIcon *status_icon, guint button,
    guint activate_time, gpointer user_data)
{
    _tray_icon_clicked = true;
    _tray_icon_clicked_time = activate_time;
    action_show_command_menu ();
}

static void
ui_tray_icon_activate_cb (GtkStatusIcon *status_icon, gpointer user_data)
{
    _tray_icon_clicked = true;
    _tray_icon_clicked_time = gtk_get_current_event_time ();
    _panel_agent->request_factory_menu ();
}

static void
ui_factory_menu_activate_cb (GtkMenuItem *item,
                             gpointer     user_data)
{
    int id = GPOINTER_TO_INT (user_data);

    if (id >= 0 && id < (int) _factory_menu_uuids.size ())
        _panel_agent->change_factory (_factory_menu_uuids [id]);
    else
        _panel_agent->change_factory ("");
}

static void
ui_factory_menu_deactivate_cb (GtkMenuItem *item,
                               gpointer     user_data)
{
    _factory_menu_activated = false;
    gettimeofday (&_last_menu_deactivate_time, 0);
}

static gboolean
ui_lookup_table_vertical_click_cb (GtkWidget      *item,
                                   GdkEventButton *event,
                                   gpointer        user_data)
{
    SCIM_DEBUG_MAIN (3) << "  ui_lookup_table_vertical_click_cb...\n";

    _panel_agent->select_candidate ((uint32)GPOINTER_TO_INT (user_data));

    return TRUE;
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

static gboolean
ui_input_window_motion_cb (GtkWidget *window,
                           GdkEventMotion *event,
                           gpointer user_data)
{
    gint pos_x, pos_y;

    if ((event->state & (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK | GDK_BUTTON3_MASK)) != 0 &&
        _input_window_draging) {
        gtk_window_get_position (GTK_WINDOW (window), &pos_x, &pos_y);
        gtk_window_move (GTK_WINDOW (window),
            pos_x + ((gint) event->x_root - _input_window_drag_x),
            pos_y + ((gint) event->y_root - _input_window_drag_y));

        _input_window_drag_x = (gint) event->x_root;
        _input_window_drag_y = (gint) event->y_root;

        return TRUE;
    }
    return FALSE;
}

static gboolean
ui_input_window_click_cb (GtkWidget *window,
                          GdkEventButton *event,
                          gpointer user_data)
{
    int click_type = GPOINTER_TO_INT (user_data);
    static gulong motion_handler;
    GdkCursor *cursor;

#if GTK_CHECK_VERSION(3, 0, 0)
    GdkDisplay    *display;
    GdkDevice     *pointer;
    GdkDeviceManager *device_manager;
    display = gdk_window_get_display (gtk_widget_get_window (window));
    device_manager = gdk_display_get_device_manager (display);
    pointer = gdk_device_manager_get_client_pointer (device_manager);
#endif

    if (click_type == 0) {
        if (_input_window_draging)
            return FALSE;

        // Connection pointer motion handler to this window.
        motion_handler = g_signal_connect (G_OBJECT (window), "motion-notify-event",
                                           G_CALLBACK (ui_input_window_motion_cb),
                                           NULL);

        _input_window_draging = TRUE;
        _input_window_drag_x = (gint) event->x_root;
        _input_window_drag_y = (gint) event->y_root;

        cursor = gdk_cursor_new (GDK_TOP_LEFT_ARROW);

        // Grab the cursor to prevent losing events.
#if GTK_CHECK_VERSION(3, 0, 0)

        /* FIXME Not sure if report to GDK_OWNERSHIP_WINDOW
           or GDK_OWNERSHIP_APPLICATION */
        gdk_device_grab (pointer, gtk_widget_get_window (window),
                         GDK_OWNERSHIP_WINDOW, TRUE,
                         GdkEventMask (GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK),
                         cursor, event->time);
        g_object_unref (cursor);
#else
        gdk_pointer_grab (window->window, TRUE,
                          (GdkEventMask) (GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK),
                          NULL, cursor, event->time);
        gdk_cursor_unref (cursor);
#endif
        return TRUE;
    } else if (click_type == 1) {
        if (!_input_window_draging)
            return FALSE;

        g_signal_handler_disconnect (G_OBJECT (window), motion_handler);
#if GTK_CHECK_VERSION(3, 0, 0)
        gdk_device_ungrab (pointer, event->time);
#else
        gdk_pointer_ungrab (event->time);
#endif
        _input_window_draging = FALSE;

        gtk_window_get_position (GTK_WINDOW (window), &_input_window_x, &_input_window_y);

        return TRUE;
    }

    return FALSE;
}

static gboolean
ui_toolbar_window_crossing_cb (GtkWidget        *window,
                               GdkEventCrossing *event,
                               gpointer          user_data)
{
    if (!_toolbar_always_show || _panel_is_on || _toolbar_window_draging)
        return FALSE;

    int crossing_type = GPOINTER_TO_INT (user_data);

    // 0 == enter, otherwise leave
    if (crossing_type == 0) {
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
    } else {
        _toolbar_should_hide = true;
    }

    return FALSE;
}

static gboolean
ui_toolbar_window_motion_cb (GtkWidget *window,
                             GdkEventMotion *event,
                             gpointer user_data)
{
    gint pos_x, pos_y;
    if ((event->state & (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK | GDK_BUTTON3_MASK)) != 0 &&
        _toolbar_window_draging) {
        gtk_window_get_position (GTK_WINDOW (window), &pos_x, &pos_y);
        gtk_window_move (GTK_WINDOW (window),
            pos_x + ((gint) event->x_root - _toolbar_window_drag_x),
            pos_y + ((gint) event->y_root - _toolbar_window_drag_y));

        _toolbar_window_drag_x = (gint) event->x_root;
        _toolbar_window_drag_y = (gint) event->y_root;

        return TRUE;
    }
    return FALSE;
}

static gboolean
ui_toolbar_window_click_cb (GtkWidget *window,
                            GdkEventButton *event,
                            gpointer user_data)
{
    int click_type = GPOINTER_TO_INT (user_data);
    static gulong motion_handler;
    GdkCursor *cursor;

#if GTK_CHECK_VERSION(3, 0, 0)
    GdkDisplay    *display;
    GdkDevice     *pointer;
    GdkDeviceManager *device_manager;
    display = gdk_window_get_display (gtk_widget_get_window (window));
    device_manager = gdk_display_get_device_manager (display);
    pointer = gdk_device_manager_get_client_pointer (device_manager);
#endif

    if (click_type == 0 && event->button <= 1) {
        if (_toolbar_window_draging)
            return FALSE;

        // Connection pointer motion handler to this window.
        motion_handler = g_signal_connect (G_OBJECT (window), "motion-notify-event",
                                           G_CALLBACK (ui_toolbar_window_motion_cb),
                                           NULL);

        _toolbar_window_draging = TRUE;
        _toolbar_window_drag_x = (gint) event->x_root;
        _toolbar_window_drag_y = (gint) event->y_root;

        cursor = gdk_cursor_new (GDK_TOP_LEFT_ARROW);

        // Grab the cursor to prevent losing events.
#if GTK_CHECK_VERSION(3, 0, 0)

        /* FIXME Not sure if report to GDK_OWNERSHIP_WINDOW
           or GDK_OWNERSHIP_APPLICATION */
        gdk_device_grab (pointer, gtk_widget_get_window (window),
                         GDK_OWNERSHIP_WINDOW, TRUE,
                         GdkEventMask (GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK),
                         cursor, event->time);
        g_object_unref (cursor);
#else
        gdk_pointer_grab (window->window, TRUE,
                          (GdkEventMask) (GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK),
                          NULL, cursor, event->time);
        gdk_cursor_unref (cursor);
#endif
        return TRUE;
    } else if (click_type == 1 && event->button <= 1) {
        if (!_toolbar_window_draging)
            return FALSE;

        g_signal_handler_disconnect (G_OBJECT (window), motion_handler);
#if GTK_CHECK_VERSION(3, 0, 0)
        gdk_device_ungrab (pointer, event->time);
#else
        gdk_pointer_ungrab (event->time);
#endif
        _toolbar_window_draging = FALSE;

        gint pos_x, pos_y;

        gtk_window_get_position (GTK_WINDOW (window), &pos_x, &pos_y);

        if (!_config.null () &&
            (_toolbar_window_x != pos_x || _toolbar_window_y != pos_y)) {
            if (_multi_monitors) {
               pos_x = -1;
               pos_y = -1;
            }
            _config->write (
                SCIM_CONFIG_PANEL_GTK_TOOLBAR_POS_X, pos_x);
            _config->write (
                SCIM_CONFIG_PANEL_GTK_TOOLBAR_POS_Y, pos_y);

        }
        _toolbar_window_x = pos_x;
        _toolbar_window_y = pos_y;

        return TRUE;
    } else if (click_type == 1 && event->button > 1) {
        action_show_command_menu ();
        return TRUE;
    }
    return FALSE;
}

static gboolean
ui_lookup_table_window_motion_cb (GtkWidget      *window,
                                  GdkEventMotion *event,
                                  gpointer       user_data)
{
    gint pos_x, pos_y;
    if ((event->state & (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK | GDK_BUTTON3_MASK)) != 0 &&
        _lookup_table_window_draging) {
        gtk_window_get_position (GTK_WINDOW (window), &pos_x, &pos_y);
        gtk_window_move (GTK_WINDOW (window),
            pos_x + ((gint) event->x_root - _lookup_table_window_drag_x),
            pos_y + ((gint) event->y_root - _lookup_table_window_drag_y));

        _lookup_table_window_drag_x = (gint) event->x_root;
        _lookup_table_window_drag_y = (gint) event->y_root;

        return TRUE;
    }
    return FALSE;
}

static gboolean
ui_lookup_table_window_click_cb (GtkWidget *window,
                                 GdkEventButton *event,
                                 gpointer user_data)
{
    int click_type = GPOINTER_TO_INT (user_data);
    static gulong motion_handler;
    GdkCursor *cursor;

#if GTK_CHECK_VERSION(3, 0, 0)
    GdkDisplay    *display;
    GdkDevice     *pointer;
    GdkDeviceManager *device_manager;
    display = gdk_window_get_display (gtk_widget_get_window (window));
    device_manager = gdk_display_get_device_manager (display);
    pointer = gdk_device_manager_get_client_pointer (device_manager);
#endif

    if (click_type == 0) {
        if (_lookup_table_window_draging)
            return FALSE;

        // Connection pointer motion handler to this window.
        motion_handler = g_signal_connect (G_OBJECT (window), "motion-notify-event",
                                           G_CALLBACK (ui_lookup_table_window_motion_cb),
                                           NULL);

        _lookup_table_window_draging = TRUE;
        _lookup_table_window_drag_x = (gint) event->x_root;
        _lookup_table_window_drag_y = (gint) event->y_root;

        cursor = gdk_cursor_new (GDK_TOP_LEFT_ARROW);

        // Grab the cursor to prevent losing events.
#if GTK_CHECK_VERSION(3, 0, 0)

        /* FIXME Not sure if report to GDK_OWNERSHIP_WINDOW
           or GDK_OWNERSHIP_APPLICATION */
        gdk_device_grab (pointer, gtk_widget_get_window (window),
                         GDK_OWNERSHIP_WINDOW, TRUE,
                         GdkEventMask (GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK),
                         cursor, event->time);
        g_object_unref (cursor);
#else
        gdk_pointer_grab (window->window, TRUE,
                          (GdkEventMask) (GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK),
                          NULL, cursor, event->time);
        gdk_cursor_unref (cursor);
#endif
        return TRUE;
    } else if (click_type == 1) {
        if (!_lookup_table_window_draging)
            return FALSE;

        g_signal_handler_disconnect (G_OBJECT (window), motion_handler);
#if GTK_CHECK_VERSION(3, 0, 0)
        gdk_device_ungrab (pointer, event->time);
#else
        gdk_pointer_ungrab (event->time);
#endif
        _lookup_table_window_draging = FALSE;

        gtk_window_get_position (GTK_WINDOW (window), &_lookup_table_window_x, &_lookup_table_window_y);

        return TRUE;
    }

    return FALSE;
}

static gboolean
ui_hide_window_timeout_cb (gpointer data)
{
#if !GTK_CHECK_VERSION(2, 12, 0)
    gdk_threads_enter ();
#endif

    if (!_toolbar_always_show) {
#if !GTK_CHECK_VERSION(2, 12, 0)
        gdk_threads_leave ();
#endif
        return TRUE;
    }

    if (!_toolbar_should_hide || _panel_is_on ||
        _toolbar_window_draging || _toolbar_hidden ||
        ui_any_menu_activated ()) {
        _toolbar_hide_timeout_count = 0;
#if !GTK_CHECK_VERSION(2, 12, 0)
        gdk_threads_leave ();
#endif
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

#if !GTK_CHECK_VERSION(2, 12, 0)
    gdk_threads_leave ();
#endif
    return TRUE;
}

static bool
ui_can_hide_input_window (void)
{
    if (!_panel_is_on) return true;

#if GTK_CHECK_VERSION(2, 18, 0)
    if (gtk_widget_get_visible (_preedit_area) ||
        gtk_widget_get_visible (_aux_area) ||
        (_lookup_table_embedded && gtk_widget_get_visible (_lookup_table_window)))
#else
    if (GTK_WIDGET_VISIBLE (_preedit_area) ||
        GTK_WIDGET_VISIBLE (_aux_area) ||
        (_lookup_table_embedded && GTK_WIDGET_VISIBLE (_lookup_table_window)))
#endif
        return false;
    return true;
}

static bool
ui_any_menu_activated (void)
{
    return _factory_menu_activated || _command_menu_activated || _property_menu_activated;
}

static void
ui_show_help (const String &help)
{
    if (!help.length () || !_help_dialog || !_help_scroll || !_help_area)
        return;

    GtkRequisition size;

    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (_help_scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    gtk_label_set_text (GTK_LABEL (_help_area), help.c_str ());

#if GTK_CHECK_VERSION(3, 0, 0)
    gtk_widget_get_preferred_size (_help_area, &size, NULL);
#else
    gtk_widget_size_request (_help_area, &size);
#endif

    if (size.width > ui_screen_width ()/2) {
        size.width = ui_screen_width ()/2;
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (_help_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    }

    if (size.height > ui_screen_height ()/2)
        size.height = ui_screen_height ()/2;

    if (size.height < size.width/2)
        size.height = size.width/2;

    gtk_widget_set_size_request (_help_scroll, size.width, size.height);

    gtk_window_set_position (GTK_WINDOW (_help_dialog), GTK_WIN_POS_CENTER_ALWAYS);
    gtk_widget_show (_help_dialog);
}

static PangoAttrList *
create_pango_attrlist (const String        &mbs,
                       const AttributeList &attrs)
{
    PangoAttrList  *attrlist = pango_attr_list_new ();
    PangoAttribute *attr;

    guint start_index, end_index;
    guint wlen = g_utf8_strlen (mbs.c_str (), mbs.length ());

#if GTK_CHECK_VERSION(3, 0, 0)
    guint16 _normal_bg_rgb[] = { 65536*_normal_bg.red, 65536*_normal_bg.green, 65536*_normal_bg.blue };
    guint16 _active_bg_rgb[] = { 65536*_active_bg.red, 65536*_active_bg.green, 65536*_active_bg.blue };
    guint16 _normal_text_rgb[] = { 65536*_normal_text.red, 65536*_normal_text.green, 65536*_normal_text.blue };
    guint16 _active_text_rgb[] = { 65536*_active_text.red, 65536*_active_text.green, 65536*_active_text.blue };
#else
    guint16 _normal_bg_rgb[] = { _normal_bg.red, _normal_bg.green, _normal_bg.blue };
    guint16 _active_bg_rgb[] = { _active_bg.red, _active_bg.green, _active_bg.blue };
    guint16 _normal_text_rgb[] = { _normal_text.red, _normal_text.green, _normal_text.blue };
    guint16 _active_text_rgb[] = { _active_text.red, _active_text.green, _active_text.blue };
#endif

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
ui_command_menu_exit_activate_cb (GtkMenuItem *item,
                                  gpointer     user_data)
{
    _panel_agent->exit ();
}

static void
ui_command_menu_reload_activate_cb (GtkMenuItem *item,
                                    gpointer     user_data)
{
    _panel_agent->reload_config ();

    if (!_config.null ()) _config->reload ();
}

static void
ui_command_menu_stick_activate_cb (GtkMenuItem *item,
                                   gpointer     user_data)
{
    action_toggle_window_stick ();
}

static void
ui_command_menu_hide_toolbar_toggled_cb (GtkMenuItem *item,
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
ui_command_menu_help_activate_cb (GtkMenuItem *item,
                                  gpointer     user_data)
{
#if GTK_CHECK_VERSION(2, 18, 0)
    if (gtk_widget_get_visible (_help_dialog)) {
#else
    if (GTK_WIDGET_VISIBLE (_help_dialog)) {
#endif
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

#if ENABLE_TRAY_ICON
// static void
// ui_tray_icon_destroy_cb (GtkObject      *object,
//                          gpointer        user_data)
// {
//     SCIM_DEBUG_MAIN (1) << "Tray Icon destroyed!\n";
//
//     gtk_widget_destroy (GTK_WIDGET (object));
//
//     _tray_icon = 0;
//     _tray_icon_factory_button = 0;
//
//     g_idle_add (ui_create_tray_icon_when_idle, NULL);
// }
#endif

static void
ui_property_activate_cb (GtkWidget      *widget,
                         gpointer        user_data)
{
    GtkWidget *submenu = (GtkWidget *) g_object_get_data (G_OBJECT (widget), "property_submenu");

    if (submenu) {
#if GTK_CHECK_VERSION(2, 2, 0)
        if (_current_screen)
            gtk_menu_set_screen (GTK_MENU (submenu), _current_screen);
#endif
        guint32 activate_time = gtk_get_current_event_time ();
        _property_menu_activated = true;
        gtk_menu_popup (GTK_MENU (submenu), 0, 0, 0, 0, 1, activate_time);
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
        image = gtk_bin_get_child (GTK_BIN (_window_stick_button));
        gtk_container_remove (GTK_CONTAINER (_window_stick_button), image);

        image = ui_create_stick_icon (_window_sticked);
        gtk_container_add (GTK_CONTAINER (_window_stick_button), image);
    }
}

static void
action_show_command_menu (void)
{
    if (_command_menu_activated)
        return;

    _command_menu_activated = true;

    guint32 activate_time = gtk_get_current_event_time ();

    if (_command_menu) {
        gtk_widget_destroy (_command_menu);
        _command_menu = 0;
    }

    _command_menu = gtk_menu_new ();

#if GTK_CHECK_VERSION(2, 2, 0)
    if (_current_screen)
        gtk_menu_set_screen (GTK_MENU (_command_menu), _current_screen);
#endif

    GtkWidget *menu_item;
    GtkWidget *icon;

    gint width, height;

    gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &width, &height);

    // Add Helper object items.
    for (size_t i = 0; i < _helper_list.size (); ++i) {
        if ((_helper_list [i].option & SCIM_HELPER_STAND_ALONE) != 0 &&
            (_helper_list [i].option & SCIM_HELPER_AUTO_START) == 0) {
            menu_item =
#if GTK_CHECK_VERSION(3, 10, 0)
                gtk_menu_item_new_with_label
#else
                gtk_image_menu_item_new_with_label
#endif
                    (_helper_list [i].name.c_str ());
#if GTK_CHECK_VERSION(2, 12, 0)
            gtk_widget_set_tooltip_text (menu_item, _helper_list [i].description.c_str ());
#else
            gtk_tooltips_set_tip (_tooltips, menu_item, _helper_list [i].description.c_str (), NULL);
#endif

#if !GTK_CHECK_VERSION(3, 10, 0)
            icon = ui_create_icon (_helper_list [i].icon, NULL, width, height, false);
            if (icon)
                gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
#endif

            gtk_menu_shell_append (GTK_MENU_SHELL (_command_menu), menu_item);

            g_signal_connect (G_OBJECT (menu_item), "activate",
                              G_CALLBACK (ui_command_menu_helper_activate_cb),
                              GINT_TO_POINTER ((int)i));

            gtk_widget_show (menu_item);
        }
    }

    if (_helper_list.size ()) {
        menu_item = gtk_separator_menu_item_new ();
        gtk_menu_shell_append (GTK_MENU_SHELL (_command_menu), menu_item);
        gtk_widget_show (menu_item);
    }

    //Reload Configuration.
    menu_item =
#if GTK_CHECK_VERSION(3, 10, 0)
        gtk_menu_item_new_with_label
#else
        gtk_image_menu_item_new_with_label
#endif
            (_("Reload Configuration"));

#if !GTK_CHECK_VERSION(3, 10, 0)
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
                                   gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU));
#endif
    gtk_menu_shell_append (GTK_MENU_SHELL (_command_menu), menu_item);
    g_signal_connect (G_OBJECT (menu_item), "activate",
                      G_CALLBACK (ui_command_menu_reload_activate_cb),
                      0);
    gtk_widget_show_all (menu_item);

    //Stick
    menu_item = gtk_check_menu_item_new_with_label (_("Stick Windows"));
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), _window_sticked);
    gtk_menu_shell_append (GTK_MENU_SHELL (_command_menu), menu_item);
    g_signal_connect (G_OBJECT (menu_item), "activate",
                      G_CALLBACK (ui_command_menu_stick_activate_cb),
                      0);
    gtk_widget_show_all (menu_item);

    //Toolbar
    menu_item = gtk_check_menu_item_new_with_label (_("Hide Toolbar"));
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), _toolbar_always_hidden);
    gtk_menu_shell_append (GTK_MENU_SHELL (_command_menu), menu_item);
    g_signal_connect (G_OBJECT (menu_item), "toggled",
                      G_CALLBACK (ui_command_menu_hide_toolbar_toggled_cb),
                      0);
    gtk_widget_show_all (menu_item);

    //Help
    menu_item =
#if GTK_CHECK_VERSION(3, 10, 0)
        gtk_menu_item_new_with_label
#else
        gtk_image_menu_item_new_with_label
#endif
             (_("Help ..."));

#if !GTK_CHECK_VERSION(3, 10, 0)
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
                                   gtk_image_new_from_stock (GTK_STOCK_HELP, GTK_ICON_SIZE_MENU));
#endif
    gtk_menu_shell_append (GTK_MENU_SHELL (_command_menu), menu_item);
    g_signal_connect (G_OBJECT (menu_item), "activate",
                      G_CALLBACK (ui_command_menu_help_activate_cb),
                      0);
    gtk_widget_show_all (menu_item);

    g_signal_connect (G_OBJECT (_command_menu), "deactivate",
                      G_CALLBACK (ui_command_menu_deactivate_cb),
                      NULL);

    menu_item = gtk_separator_menu_item_new ();
    gtk_menu_shell_append (GTK_MENU_SHELL (_command_menu), menu_item);
    gtk_widget_show (menu_item);

    //Clients exit.
    menu_item =
#if GTK_CHECK_VERSION(3, 10, 0)
        gtk_menu_item_new_with_label
#else
        gtk_image_menu_item_new_with_label
#endif
             (_("Exit"));

#if !GTK_CHECK_VERSION(3, 10, 0)
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
                                   gtk_image_new_from_stock (GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU));
#endif
    gtk_menu_shell_append (GTK_MENU_SHELL (_command_menu), menu_item);
    g_signal_connect (G_OBJECT (menu_item), "activate",
                      G_CALLBACK (ui_command_menu_exit_activate_cb),
                      0);
    gtk_widget_show_all (menu_item);
    if (_tray_icon_clicked && _tray_icon) {
        G_GNUC_BEGIN_IGNORE_DEPRECATIONS
        gtk_menu_popup (GTK_MENU (_command_menu), 0, 0, gtk_status_icon_position_menu, _tray_icon, 2, _tray_icon_clicked_time);
        G_GNUC_END_IGNORE_DEPRECATIONS
    }
    else
        gtk_menu_popup (GTK_MENU (_command_menu), 0, 0, 0, 0, 2, activate_time);
    _tray_icon_clicked = false;
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
#if GTK_CHECK_VERSION(2, 32, 0)
        _panel_agent_thread = g_thread_new ("panel_agent", panel_agent_thread_func, NULL);
#else
        _panel_agent_thread = g_thread_create (panel_agent_thread_func, NULL, TRUE, NULL);
#endif
    }

    return (_panel_agent_thread != NULL);
}

static gpointer
panel_agent_thread_func (gpointer data)
{
    SCIM_DEBUG_MAIN(1) << "panel_agent_thread_func ()\n";

    if (!_panel_agent->run ())
        std::cerr << "Failed to run Panel.\n";
/*
    G_LOCK (_global_resource_lock);
    _should_exit = true;
    G_UNLOCK (_global_resource_lock);
*/
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gdk_threads_enter ();
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_main_quit ();
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gdk_threads_leave ();
    g_thread_exit (NULL);
    G_GNUC_END_IGNORE_DEPRECATIONS
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

static void
slot_transaction_start (void)
{
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gdk_threads_enter ();
    G_GNUC_END_IGNORE_DEPRECATIONS
}

static void
slot_transaction_end (void)
{
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gdk_threads_leave ();
    G_GNUC_END_IGNORE_DEPRECATIONS
}

static void
slot_reload_config (void)
{
    if (!_config.null ()) _config->reload ();
}

static void
slot_turn_on (void)
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
slot_turn_off (void)
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
slot_update_screen (int num)
{
#if GTK_CHECK_VERSION(2, 2, 0)
    gint n_screens =
#if GTK_CHECK_VERSION(3, 10, 0)
        1
#else
        gdk_display_get_n_screens (gdk_display_get_default ())
#endif
        ;
    if (n_screens > num) {

        GdkScreen *screen = gdk_display_get_screen (gdk_display_get_default (), num);

        if (screen) {
            /*
#ifdef GDK_WINDOWING_X11
            GdkWindow *root_window = gdk_get_default_root_window ();
            if (_current_screen)
                root_window = gdk_screen_get_root_window (_current_screen);
            gdk_window_remove_filter (root_window, ui_event_filter, NULL);
#endif
            */

            _current_screen = screen;
            ui_switch_screen (screen);
        }
    }
#endif
}

static void
slot_update_factory_info (const PanelFactoryInfo &info)
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
            if (_default_font_desc)
#if GTK_CHECK_VERSION(3, 0, 0)
                gtk_widget_override_font (newlabel, _default_font_desc);
#else
                gtk_widget_modify_font (newlabel, _default_font_desc);
#endif
            gtk_widget_show (newlabel);
        }

        if (newlabel) {
            GtkWidget * old = gtk_bin_get_child (GTK_BIN (_factory_button));
            if (old)
                gtk_container_remove (GTK_CONTAINER (_factory_button), old);
            gtk_container_add (GTK_CONTAINER (_factory_button), newlabel);
        }

#if GTK_CHECK_VERSION(2, 18, 0)
        if (!gtk_widget_get_visible (_factory_button) && !_toolbar_hidden)
#else
        if (!GTK_WIDGET_VISIBLE (_factory_button) && !_toolbar_hidden)
#endif
            gtk_widget_show (_factory_button);

#if GTK_CHECK_VERSION(2, 12, 0)
        gtk_widget_set_tooltip_text (_factory_button, info.name.c_str ());
#else
        if (_tooltips)
            gtk_tooltips_set_tip (_tooltips, _factory_button, info.name.c_str (), NULL);
#endif

        ui_settle_toolbar_window ();
    }

#if ENABLE_TRAY_ICON
    // if (_tray_icon_factory_button) {
    //     GtkWidget *icon = gtk_bin_get_child (GTK_BIN (_tray_icon_factory_button));

    //     if (icon)
    //         gtk_container_remove (GTK_CONTAINER (_tray_icon_factory_button), icon);

    //     icon = ui_create_icon (info.icon, NULL, TRAY_ICON_SIZE, TRAY_ICON_SIZE, true);

    //     gtk_container_add (GTK_CONTAINER (_tray_icon_factory_button), icon);

    //     gtk_widget_set_tooltip_text (_tray_icon_factory_button, info.name.c_str ());
    // }
    if (_tray_icon) {
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
        gtk_status_icon_set_from_file (_tray_icon, info.icon.c_str());
    G_GNUC_END_IGNORE_DEPRECATIONS
    }
#endif

    if (info.uuid != "") {
        _recent_factory_uuids.remove(info.uuid);
        _recent_factory_uuids.push_front(info.uuid);
        if (_recent_factory_uuids.size () > 5)
            _recent_factory_uuids.pop_back ();
    }
}

static void
slot_show_help (const String &help)
{
    ui_show_help (help);
}

static void
slot_show_factory_menu (const std::vector <PanelFactoryInfo> &factories)
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
            gtk_widget_destroy (_factory_menu);
            _factory_menu = 0;
        }

        _factory_menu = gtk_menu_new ();

#if GTK_CHECK_VERSION(2, 2, 0)
    if (_current_screen)
        gtk_menu_set_screen (GTK_MENU (_factory_menu), _current_screen);
#endif

        GtkWidget *submenu;
        GtkWidget *menu_item;
        guint id;
        PanelFactoryInfo info;

        // recently used factories
        if (show_recent && recents.size ()) {
            for (std::list<String>::iterator it = _recent_factory_uuids.begin (); it != _recent_factory_uuids.end (); ++it) {

                id = recents [*it];
                info = factories [id];

                ui_create_factory_menu_entry (info, id, GTK_MENU_SHELL (_factory_menu), true, (langs [info.lang] > 1));

                if (use_submenus) {
                    MapStringVectorSizeT::iterator g = groups.find (info.lang);
                    if (g != groups.end () && g->second.size () >= 1) {
                        g->second.push_back (id);
                    }
                }
            }

            menu_item = gtk_separator_menu_item_new ();
            gtk_menu_shell_append (GTK_MENU_SHELL (_factory_menu), menu_item);
            gtk_widget_show (menu_item);
        }

        for (MapStringVectorSizeT::iterator it = groups.begin (); it != groups.end (); ++ it) {
            if (use_submenus && it->second.size () > 1) {
                String lang = it->first;
                menu_item = gtk_menu_item_new_with_label (scim_get_language_name (lang).c_str ());
                submenu = gtk_menu_new ();
            } else {
                menu_item = 0;
                submenu = 0;
            }

            for (i = 0; i < it->second.size (); ++i) {
                id = it->second [i];
                info = factories [id];
                ui_create_factory_menu_entry (info, id, GTK_MENU_SHELL (submenu ? submenu : _factory_menu), !submenu, (langs [info.lang] > 1));
            }

            if (menu_item && submenu) {
                gtk_menu_shell_append (GTK_MENU_SHELL (_factory_menu), menu_item);
                gtk_widget_show (menu_item);
                gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), submenu);
                gtk_widget_show (submenu);
            }
        }

        //Append an entry for forward mode.
        info = PanelFactoryInfo (String (""), String (_("English/Keyboard")), String ("C"), String (SCIM_KEYBOARD_ICON_FILE));
        ui_create_factory_menu_entry (info, -1, GTK_MENU_SHELL (_factory_menu), false, true);

        g_signal_connect (G_OBJECT (_factory_menu), "deactivate",
                          G_CALLBACK (ui_factory_menu_deactivate_cb),
                          NULL);

        if (_tray_icon_clicked && _tray_icon) {
            while (gtk_main_iteration_do (FALSE));
            G_GNUC_BEGIN_IGNORE_DEPRECATIONS
            gtk_menu_popup (GTK_MENU (_factory_menu), 0, 0, gtk_status_icon_position_menu, _tray_icon, 1, _tray_icon_clicked_time);
            G_GNUC_END_IGNORE_DEPRECATIONS
        }
        else {
            gtk_menu_popup (GTK_MENU (_factory_menu), 0, 0, 0, 0, 1, gtk_get_current_event_time ());
        }

        _tray_icon_clicked = false;

    }
}

static void
slot_update_spot_location (int x, int y)
{
    if (x > 0 && x < ui_screen_width () && y > 0 && y < ui_screen_height ()) {
        _spot_location_x = x;
        _spot_location_y = y;

        ui_settle_input_window ();
        ui_settle_lookup_table_window ();
    }
}

static void
slot_show_preedit_string (void)
{
    gtk_widget_show (_preedit_area);

#if GTK_CHECK_VERSION(2, 18, 0)
    if (_panel_is_on && !gtk_widget_get_visible (_input_window))
#else
    if (_panel_is_on && !GTK_WIDGET_VISIBLE (_input_window))
#endif
        gtk_widget_show (_input_window);

    ui_settle_input_window (true, true);
    ui_settle_lookup_table_window ();
}

static void
slot_show_aux_string (void)
{
    gtk_widget_show (_aux_area);

#if GTK_CHECK_VERSION(2, 18, 0)
    if (_panel_is_on && !gtk_widget_get_visible (_input_window))
#else
    if (_panel_is_on && !gtk_widget_get_visible (_input_window))
#endif
        gtk_widget_show (_input_window);

    ui_settle_input_window (true, true);
    ui_settle_lookup_table_window ();
}

static void
slot_show_lookup_table (void)
{
    gtk_widget_show (_lookup_table_window);

#if GTK_CHECK_VERSION(2, 18, 0)
    if (_panel_is_on && _lookup_table_embedded && !gtk_widget_get_visible (_input_window)) {
#else
    if (_panel_is_on && _lookup_table_embedded && !GTK_WIDGET_VISIBLE (_input_window)) {
#endif
        gtk_widget_show (_input_window);
        ui_settle_input_window (true, true);
    }

    ui_settle_lookup_table_window (true);
}

static void
slot_hide_preedit_string (void)
{
    gtk_widget_hide (_preedit_area);
    scim_string_view_set_text (SCIM_STRING_VIEW (_preedit_area), "");

    if (ui_can_hide_input_window ())
        gtk_widget_hide (_input_window);

    ui_settle_lookup_table_window ();
}

static void
slot_hide_aux_string (void)
{
    gtk_widget_hide (_aux_area);
    scim_string_view_set_text (SCIM_STRING_VIEW (_aux_area), "");

    if (ui_can_hide_input_window ())
        gtk_widget_hide (_input_window);

    ui_settle_lookup_table_window ();
}

static void
slot_hide_lookup_table (void)
{
    gtk_widget_hide (_lookup_table_window);

    if (_lookup_table_embedded && ui_can_hide_input_window ())
        gtk_widget_hide (_input_window);
}

static void
slot_update_preedit_string (const String &str, const AttributeList &attrs)
{
    PangoAttrList  *attrlist = create_pango_attrlist (str, attrs);

    scim_string_view_set_attributes (SCIM_STRING_VIEW (_preedit_area), attrlist);
    scim_string_view_set_text (SCIM_STRING_VIEW (_preedit_area), str.c_str ());

    pango_attr_list_unref (attrlist);

    ui_settle_input_window (true);

    ui_settle_lookup_table_window ();
}

static void
slot_update_preedit_caret (int caret)
{
    scim_string_view_set_position (SCIM_STRING_VIEW (_preedit_area), caret);
}

static void
slot_update_aux_string (const String &str, const AttributeList &attrs)
{
    PangoAttrList  *attrlist = create_pango_attrlist (str, attrs);

    scim_string_view_set_attributes (SCIM_STRING_VIEW (_aux_area), attrlist);
    scim_string_view_set_text (SCIM_STRING_VIEW (_aux_area), str.c_str ());

    pango_attr_list_unref (attrlist);

    ui_settle_input_window (true);

    ui_settle_lookup_table_window ();
}

static void
slot_update_lookup_table (const LookupTable &table)
{
    size_t i;
    size_t item_num = table.get_current_page_size ();

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

                wcs = table.get_candidate_in_current_page (i);

                label = table.get_candidate_label (i);

                if (label.length ()) {
                    label += utf8_mbstowcs (". ");
                } else {
                    label = utf8_mbstowcs (" ");
                }

                mbs = utf8_wcstombs (label+wcs);

                scim_string_view_set_text (SCIM_STRING_VIEW (_lookup_table_items [i]),
                                           mbs.c_str ());

                // Update attributes;
                attrs = table.get_attributes_in_current_page (i);

                if (attrs.size ()) {
                    for (AttributeList::iterator ait = attrs.begin (); ait != attrs.end (); ++ait)
                        ait->set_start (ait->get_start () + label.length ());

                    attrlist = create_pango_attrlist (mbs, attrs);
                    scim_string_view_set_attributes (SCIM_STRING_VIEW (_lookup_table_items [i]), attrlist);
                    pango_attr_list_unref (attrlist);
                } else {
                    scim_string_view_set_attributes (SCIM_STRING_VIEW (_lookup_table_items [i]), 0);
                }

                if (i == table.get_cursor_pos_in_current_page () && table.is_cursor_visible ())
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
                AttributeList item_attrs = table.get_attributes_in_current_page (i);
                size_t attr_start, attr_end;

                label = table.get_candidate_label (i);

                if (label.length ()) {
                    label += utf8_mbstowcs (".");
                }

                wcs += label;

                attr_start = wcs.length ();

                wcs += table.get_candidate_in_current_page (i);

                attr_end = wcs.length ();

                wcs.push_back (0x20);

                _lookup_table_index [i+1] = wcs.length ();

                mbs = utf8_wcstombs (wcs);

                scim_string_view_set_text (SCIM_STRING_VIEW (_lookup_table_items [0]),
                                           mbs.c_str ());

#if GTK_CHECK_VERSION(3, 0, 0)
                gtk_widget_get_preferred_size (_lookup_table_window, &size, NULL);
#else
                gtk_widget_size_request (_lookup_table_window, &size);
#endif

                if (size.width >= ui_screen_width () / 3 && !table.is_page_size_fixed ()) {
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

        if (table.is_cursor_visible ()) {
            int start = _lookup_table_index [table.get_cursor_pos_in_current_page ()];
            int end = _lookup_table_index [table.get_cursor_pos_in_current_page ()+1] - 1;
            scim_string_view_set_highlight (SCIM_STRING_VIEW (_lookup_table_items [0]), start, end);
        } else {
            scim_string_view_set_highlight (SCIM_STRING_VIEW (_lookup_table_items [0]), -1, -1);
        }
    }

    if (table.get_current_page_start ())
        gtk_widget_set_sensitive (_lookup_table_up_button, TRUE);
    else
        gtk_widget_set_sensitive (_lookup_table_up_button, FALSE);

    if (table.get_current_page_start () + item_num < table.number_of_candidates ())
        gtk_widget_set_sensitive (_lookup_table_down_button, TRUE);
    else
        gtk_widget_set_sensitive (_lookup_table_down_button, FALSE);

    if (item_num < table.get_current_page_size ())
        _panel_agent->update_lookup_table_page_size (item_num);

    if (_lookup_table_embedded)
        ui_settle_input_window (true);
    else
        ui_settle_lookup_table_window ();
}

static void
slot_register_properties (const PropertyList &props)
{
    register_frontend_properties (props);
}

static void
slot_update_property (const Property &prop)
{
    update_frontend_property (prop);
}

static void
slot_register_helper_properties (int id, const PropertyList &props)
{
    register_helper_properties (id, props);
}

static void
slot_update_helper_property (int id, const Property &prop)
{
    update_helper_property (id, prop);
}

static void
slot_register_helper (int id, const HelperInfo &helper)
{
}

static void
slot_remove_helper (int id)
{
    HelperPropertyRepository::iterator it = _helper_property_repository.find (id);

    if (it != _helper_property_repository.end () && it->second.holder)
        gtk_widget_destroy (it->second.holder);

    _helper_property_repository.erase (id);
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

    // If the level is zero, then create the this node as button, otherwise create as a menu item.
    if (!level) {
        GtkWidget * label = ui_create_label (begin->get_label (),
                                             begin->get_icon (),
                                             0,
                                             !_toolbar_show_property_label,
                                             false);

        node = gtk_button_new ();
        gtk_container_add (GTK_CONTAINER (node), label);
        gtk_button_set_relief (GTK_BUTTON (node), GTK_RELIEF_NONE);
    } else {
        node =
#if GTK_CHECK_VERSION(3, 10, 0)
            gtk_menu_item_new_with_label
#else
            gtk_image_menu_item_new_with_label
#endif
                (begin->get_label ().c_str ());
#if !GTK_CHECK_VERSION(3, 10, 0)
        gint width, height;
        gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &width, &height);
        GtkWidget * icon = ui_create_icon (begin->get_icon (), NULL, width, height, false);
        if (icon)
            gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (node), icon);
#endif
    }

    if (begin->visible ())
        gtk_widget_show (node);
    else
        gtk_widget_hide (node);

    gtk_widget_set_sensitive (node, begin->active ());

#if GTK_CHECK_VERSION(2, 12, 0)
    if (begin->get_tip ().length ())
        gtk_widget_set_tooltip_text (node, begin->get_tip ().c_str ());
#else
    if (_tooltips && begin->get_tip ().length ())
        gtk_tooltips_set_tip (_tooltips, node, begin->get_tip ().c_str (), NULL);
#endif

    g_object_set_data_full (G_OBJECT (node), "property_key", g_strdup (begin->get_key ().c_str ()), g_free);

    info.property = *begin;
    info.widget = node;

    repository.push_back (info);

    it = begin + 1;

    if (it != end) {
        GtkWidget * submenu = gtk_menu_new ();
        GtkWidget * child;
        int menu_item_idx = 0;

        // Create all leafs of the first child.
        while (it != end) {
            // Find all leafs of the first child.
            for (next = it + 1; next != end; ++ next)
                if (!next->is_a_leaf_of (*it)) break;

            child = create_properties_node (repository, it, next, client, level + 1);
            if (child) {
                gtk_menu_shell_append (GTK_MENU_SHELL (submenu), child);
                g_object_set_data (G_OBJECT (child), "menu_item_idx", GINT_TO_POINTER (menu_item_idx));
                ++ menu_item_idx;
            }

            it = next;
        }

        // The node is a button, so attach the submenu as its data.
        if (!level) {
            g_object_set_data_full (G_OBJECT (node), "property_submenu", submenu, (void (*)(void*)) gtk_widget_destroy);

            g_signal_connect (G_OBJECT (submenu), "deactivate",
                              G_CALLBACK (ui_property_menu_deactivate_cb),
                              NULL);
        } else // The node is a menu item, so attach the submenu directly.
            gtk_menu_item_set_submenu (GTK_MENU_ITEM (node), submenu);

        leaf = false;
    }

    if (leaf || level == 0) {
        g_signal_connect (G_OBJECT (node),
                          ((level > 0) ? "activate" : "clicked"),
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

    GtkWidget *root;

    int menu_item_idx = 0;

    if (begin == end) return;

#if GTK_CHECK_VERSION(3, 2, 0)
    root = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
#else
    root = gtk_hbox_new (FALSE, 0);
#endif

    it = begin;
    next = begin + 1;

    while (it != end) {
        if (next == end || !next->is_a_leaf_of (*it)) {
            GtkWidget * node = create_properties_node (repository, it, next, client, level);

            if (node) {
                // The container is a hbox.
                if (!level)
                    gtk_box_pack_start (GTK_BOX (container), node, TRUE, TRUE, 0);
                // The container is a menu.
                else {
                    gtk_menu_shell_append (GTK_MENU_SHELL (container), node);
                    g_object_set_data (G_OBJECT (node), "menu_item_idx", GINT_TO_POINTER (menu_item_idx));
                    ++ menu_item_idx;
                }
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
            gtk_widget_destroy (_frontend_properties_area);

        _frontend_properties_area = 0;

        _frontend_property_repository.clear ();

        if (properties.size ()) {
#if GTK_CHECK_VERSION(3, 2, 0)
            _frontend_properties_area = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
#else
            _frontend_properties_area = gtk_hbox_new (FALSE, 0);
#endif

            create_properties (_frontend_properties_area,
                               _frontend_property_repository,
                               properties,
                               -1,
                               0);

            gtk_widget_show (_frontend_properties_area);

            gtk_box_pack_start (GTK_BOX (_client_properties_area), _frontend_properties_area, TRUE,TRUE, 0);
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
        gtk_widget_destroy (it->second.holder);

    it->second.holder = 0;

    if (properties.size ()) {
#if GTK_CHECK_VERSION(3, 2, 0)
        it->second.holder = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
#else
        it->second.holder = gtk_hbox_new (FALSE, 0);
#endif

        create_properties (it->second.holder,
                           it->second.repository,
                           properties,
                           client,
                           0);

        gtk_widget_show (it->second.holder);
        gtk_box_pack_end (GTK_BOX (_client_properties_area), it->second.holder, TRUE,TRUE, 0);
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
                    GtkWidget *old = gtk_bin_get_child (GTK_BIN (it->widget));
                    gtk_container_remove (GTK_CONTAINER (it->widget), old);
                    gtk_container_add (GTK_CONTAINER (it->widget), label);
                } else if (GTK_IS_MENU_ITEM (it->widget)) {
                    gint width, height;
                    gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &width, &height);

                    GtkWidget * menu = gtk_widget_get_parent (it->widget);
                    int menu_item_idx = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (it->widget), "menu_item_idx"));

                    GtkWidget * icon = ui_create_icon (property.get_icon (), NULL, width, height, false);
                    GtkWidget * new_item =
#if GTK_CHECK_VERSION(3, 10, 0)
                        gtk_menu_item_new_with_label
#else
                        gtk_image_menu_item_new_with_label
#endif
                            (property.get_label ().c_str ());

#if !GTK_CHECK_VERSION(3, 10, 0)
                    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (new_item), icon);
#endif

                    GtkWidget * submenu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (it->widget));

                    gtk_menu_item_set_submenu (GTK_MENU_ITEM (new_item), submenu);

                    g_object_set_data_full (G_OBJECT (new_item), "property_key", g_strdup (property.get_key ().c_str ()), g_free);
                    g_object_set_data (G_OBJECT (new_item), "menu_item_idx", GINT_TO_POINTER (menu_item_idx));

                    gtk_widget_destroy (it->widget);

                    it->widget = new_item;
                    gtk_menu_shell_insert (GTK_MENU_SHELL (menu), new_item, menu_item_idx);
                }
            }

            if (property.visible ())
                gtk_widget_show (it->widget);
            else
                gtk_widget_hide (it->widget);

            gtk_widget_set_sensitive (it->widget, property.active ());

#if GTK_CHECK_VERSION(2, 12, 0)
            if (property.get_tip ().length ())
                gtk_widget_set_tooltip_text (it->widget, property.get_tip ().c_str ());
#else
            if (_tooltips && property.get_tip ().length ())
                gtk_tooltips_set_tip (_tooltips, it->widget, property.get_tip ().c_str (), NULL);
#endif

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
        G_GNUC_BEGIN_IGNORE_DEPRECATIONS
        gdk_threads_enter ();
        G_GNUC_END_IGNORE_DEPRECATIONS
        gtk_main_quit ();
        G_GNUC_BEGIN_IGNORE_DEPRECATIONS
        gdk_threads_leave ();
        G_GNUC_END_IGNORE_DEPRECATIONS
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

    int    new_argc = 0;
    char **new_argv = new char * [40];

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

    new_argv [new_argc ++] = argv [0];

    // Store the rest argvs into new_argv.
    for (++i; i < argc && new_argc < 40; ++i) {
        new_argv [new_argc ++] = argv [i];
    }

    // Make up DISPLAY env.
    if (display_name.length ()) {
        new_argv [new_argc ++] = const_cast <char*> ("--display");
        new_argv [new_argc ++] = const_cast <char*> (display_name.c_str ());

        setenv ("DISPLAY", display_name.c_str (), 1);
    }

    new_argv [new_argc] = 0;

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

#if !GTK_CHECK_VERSION(2, 32, 0)
    /* init threads */
    g_thread_init (NULL);
#endif
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gdk_threads_init ();
    G_GNUC_END_IGNORE_DEPRECATIONS

    signal(SIGQUIT, signalhandler);
    signal(SIGTERM, signalhandler);
    signal(SIGINT,  signalhandler);
    signal(SIGHUP,  signalhandler);

    gtk_init (&new_argc, &new_argv);

    ui_initialize ();

    // get current display.
    {
#if GTK_CHECK_VERSION(2, 2, 0)
        const char *p = gdk_display_get_name (gdk_display_get_default ());
#else
        const char *p = getenv ("DISPLAY");
#endif
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

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gdk_threads_enter ();
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_main ();
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gdk_threads_leave ();
    G_GNUC_END_IGNORE_DEPRECATIONS

    // Exiting...
    g_thread_join (_panel_agent_thread);
    _config.reset ();

    std::cerr << "Successfully exited.\n";

    return 0;
}

/*
vi:ts=4:nowrap:expandtab
*/
