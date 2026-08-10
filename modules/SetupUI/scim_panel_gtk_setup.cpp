/** @file scim_panel_gtk_setup.cpp
 * implementation of Setup Module of scim-panel-gtk.
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
 * $Id: scim_panel_gtk_setup.cpp,v 1.15 2005/12/10 14:04:54 suzhe Exp $
 *
 */

#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_PANEL

#include <gtk/gtk.h>
#include "scim_private.h"
#include "scim.h"

using namespace scim;

#define scim_module_init panel_gtk_setup_LTX_scim_module_init
#define scim_module_exit panel_gtk_setup_LTX_scim_module_exit

#define scim_setup_module_create_ui       panel_gtk_setup_LTX_scim_setup_module_create_ui
#define scim_setup_module_get_category    panel_gtk_setup_LTX_scim_setup_module_get_category
#define scim_setup_module_get_name        panel_gtk_setup_LTX_scim_setup_module_get_name
#define scim_setup_module_get_description panel_gtk_setup_LTX_scim_setup_module_get_description
#define scim_setup_module_load_config     panel_gtk_setup_LTX_scim_setup_module_load_config
#define scim_setup_module_save_config     panel_gtk_setup_LTX_scim_setup_module_save_config
#define scim_setup_module_query_changed   panel_gtk_setup_LTX_scim_setup_module_query_changed

#define SCIM_CONFIG_PANEL_GTK_FONT                      "/Panel/Gtk/Font"
#define SCIM_CONFIG_PANEL_GTK_COLOR_NORMAL_BG           "/Panel/Gtk/Color/NormalBackground"
#define SCIM_CONFIG_PANEL_GTK_COLOR_NORMAL_TEXT         "/Panel/Gtk/Color/NormalText"
#define SCIM_CONFIG_PANEL_GTK_COLOR_ACTIVE_BG           "/Panel/Gtk/Color/ActiveBackground"
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
#define SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_MENU_ICON    "/Panel/Gtk/ToolBar/ShowMenuIcon"
#define SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_HELP_ICON    "/Panel/Gtk/ToolBar/ShowHelpIcon"
#define SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_PROPERTY_LABEL "/Panel/Gtk/ToolBar/ShowPropertyLabel"
#define SCIM_CONFIG_PANEL_GTK_DEFAULT_STICKED           "/Panel/Gtk/DefaultSticked"
#define SCIM_CONFIG_PANEL_GTK_SHOW_TRAY_ICON            "/Panel/Gtk/ShowTrayIcon"

static GtkWidget * create_setup_window ();
static void        load_config (const ConfigPointer &config);
static void        save_config (const ConfigPointer &config);
static bool        query_changed ();

// Module Interface.
extern "C" {
    void scim_module_init (void)
    {
    }

    void scim_module_exit (void)
    {
    }

    GtkWidget * scim_setup_module_create_ui (void)
    {
        return create_setup_window ();
    }

    String scim_setup_module_get_category (void)
    {
        return String ("Panel");
    }

    String scim_setup_module_get_name (void)
    {
        return String (_("GTK"));
    }

    String scim_setup_module_get_description (void)
    {
        return String (_("A panel daemon based on the GTK library."));
    }

    void scim_setup_module_load_config (const ConfigPointer &config)
    {
        load_config (config);
    }

    void scim_setup_module_save_config (const ConfigPointer &config)
    {
        save_config (config);
    }

    bool scim_setup_module_query_changed ()
    {
        return query_changed ();
    }
} // extern "C"

// Internal data declaration.
static bool   __config_toolbar_always_show       = false;
static bool   __config_toolbar_always_hidden     = false;
static bool   __config_toolbar_auto_snap         = false;
static int    __config_toolbar_hide_timeout      = 2;
static bool   __config_toolbar_show_factory_icon = true;
static bool   __config_toolbar_show_factory_name = true;
static bool   __config_toolbar_show_stick_icon   = false;
static bool   __config_toolbar_show_menu_icon    = true;
static bool   __config_toolbar_show_help_icon    = false;
static bool   __config_toolbar_show_property_label = true;
static bool   __config_default_sticked           = false;
static bool   __config_show_tray_icon            = true;

static String __config_font                      = "default";

// Panel colors.  The candidate window's own colors live in the Candidates setup
// page; it still falls back to these when it has none of its own.
static String __config_panel_normal_bg               = "gray92";
static String __config_panel_normal_text             = "black";
static String __config_panel_active_bg               = "light blue";
static String __config_panel_active_text             = "black";

static bool   __have_changed                     = false;

static GtkWidget * __widget_toolbar_show_behavior    = 0;
static GtkWidget * __widget_toolbar_auto_snap         = 0;
static GtkWidget * __widget_toolbar_hide_timeout      = 0;
static GtkWidget * __widget_toolbar_show_factory_icon  = 0;
static GtkWidget * __widget_toolbar_show_factory_name  = 0;
static GtkWidget * __widget_toolbar_show_stick_icon   = 0;
static GtkWidget * __widget_toolbar_show_menu_icon   = 0;
static GtkWidget * __widget_toolbar_show_help_icon    = 0;
static GtkWidget * __widget_toolbar_show_property_label = 0;
static GtkWidget * __widget_default_sticked           = 0;
static GtkWidget * __widget_show_tray_icon            = 0;
static GtkWidget * __widget_font                      = 0;
static GtkWidget * __widget_panel_normal_bg           = 0;
static GtkWidget * __widget_panel_normal_text         = 0;
static GtkWidget * __widget_panel_active_bg           = 0;
static GtkWidget * __widget_panel_active_text         = 0;

enum ToolbarShowFlavourType {
    SCIM_TOOLBAR_SHOW_ALWAYS,
    SCIM_TOOLBAR_SHOW_ON_DEMAND,
    SCIM_TOOLBAR_SHOW_NEVER
};

static const char * __toolbar_show_behavior_text[] = {
    N_("Always"),
    N_("On demand"),
    N_("Never")
};

// Declaration of internal functions.
static void
on_default_check_button_toggled      (GtkCheckButton  *checkbutton,
                                      gpointer         user_data);

static void
on_default_spin_button_changed       (GtkSpinButton   *spinbutton,
                                      gpointer         user_data);

static void
on_toolbar_show_behavior_changed      (GtkComboBox     *combobox,
                                      gpointer         user_data);

static void
on_font_selection_clicked            (GtkButton       *button,
                                      gpointer         user_data);

static void
on_panel_color_set                   (GtkColorButton  *button,
                                      gpointer         user_data);

static void
set_color_button                     (GtkWidget       *button,
                                      const String    &color);

static void
setup_widget_value ();

// Function implementations.
GtkWidget *
create_setup_window ()
{
    static GtkWidget *window = 0;

    if (!window) {
        GtkWidget *page;
        GtkWidget *table;
        GtkWidget *frame;
        GtkWidget *vbox;
        GtkWidget *label;
        GtkWidget *hbox;

        // Create the vbox for the first page.
        page = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

        vbox = page;

        // Create the ToolBar setup block.
        frame = gtk_frame_new (_("ToolBar"));
        gtk_widget_set_margin_start (frame, 4);
        gtk_widget_set_margin_end (frame, 4);
        gtk_widget_set_margin_top (frame, 4);
        gtk_widget_set_margin_bottom (frame, 4);
        gtk_box_append (GTK_BOX (vbox), frame);

        table = gtk_grid_new();
        gtk_grid_set_row_spacing (GTK_GRID (table), 4);
        gtk_grid_set_column_spacing (GTK_GRID (table), 8);
        gtk_frame_set_child (GTK_FRAME (frame), table);

        hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_hexpand (hbox, TRUE);
        gtk_grid_attach (GTK_GRID (table), hbox, 0, 0, 1, 1);

        label = gtk_label_new_with_mnemonic (_("_Show:"));
        gtk_widget_set_margin_start (label, 4);
        gtk_widget_set_margin_end (label, 4);
        gtk_box_append (GTK_BOX (hbox), label);

        __widget_toolbar_show_behavior = gtk_combo_box_text_new ();
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (__widget_toolbar_show_behavior),
                                   _(__toolbar_show_behavior_text[SCIM_TOOLBAR_SHOW_ALWAYS]));
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (__widget_toolbar_show_behavior),
                                   _(__toolbar_show_behavior_text[SCIM_TOOLBAR_SHOW_ON_DEMAND]));
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (__widget_toolbar_show_behavior),
                                   _(__toolbar_show_behavior_text[SCIM_TOOLBAR_SHOW_NEVER]));
        gtk_box_append (GTK_BOX (hbox), __widget_toolbar_show_behavior);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), __widget_toolbar_show_behavior);

        __widget_toolbar_auto_snap = gtk_check_button_new_with_mnemonic (_("Auto s_nap"));
        __widget_toolbar_show_factory_icon = gtk_check_button_new_with_mnemonic (_("Show _input method icon"));
        __widget_toolbar_show_factory_name = gtk_check_button_new_with_mnemonic (_("Show inp_ut method name"));

        gtk_widget_set_halign (__widget_toolbar_auto_snap, GTK_ALIGN_FILL);
        gtk_grid_attach (GTK_GRID (table), __widget_toolbar_auto_snap, 0, 1, 1, 1);

        gtk_widget_set_halign (__widget_toolbar_show_factory_icon, GTK_ALIGN_FILL);
        gtk_grid_attach (GTK_GRID (table), __widget_toolbar_show_factory_icon, 0, 2, 1, 1);

        gtk_widget_set_halign (__widget_toolbar_show_factory_name, GTK_ALIGN_FILL);
        gtk_grid_attach (GTK_GRID (table), __widget_toolbar_show_factory_name, 0, 3, 1, 1);

        hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_hexpand (hbox, TRUE);
        gtk_grid_attach (GTK_GRID (table), hbox, 1, 0, 1, 1);

        label = gtk_label_new_with_mnemonic (_("Hide time_out:"));
        gtk_widget_set_margin_start (label, 4);
        gtk_widget_set_margin_end (label, 4);
        gtk_box_append (GTK_BOX (hbox), label);

        __widget_toolbar_hide_timeout = gtk_spin_button_new_with_range (0, 60, 1);
        gtk_box_append (GTK_BOX (hbox), __widget_toolbar_hide_timeout);
        gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (__widget_toolbar_hide_timeout), TRUE);
        gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (__widget_toolbar_hide_timeout), TRUE);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON (__widget_toolbar_hide_timeout), 0);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), __widget_toolbar_hide_timeout);

        __widget_toolbar_show_stick_icon = gtk_check_button_new_with_mnemonic (_("Show s_tick icon"));
        __widget_toolbar_show_menu_icon = gtk_check_button_new_with_mnemonic (_("Show m_enu icon"));
        __widget_toolbar_show_help_icon = gtk_check_button_new_with_mnemonic (_("Show _help icon"));
        __widget_toolbar_show_property_label = gtk_check_button_new_with_mnemonic (_("Show _property label"));

        gtk_widget_set_halign (__widget_toolbar_show_stick_icon, GTK_ALIGN_FILL);
        gtk_grid_attach (GTK_GRID (table), __widget_toolbar_show_stick_icon, 1, 1, 1, 1);

        gtk_widget_set_halign (__widget_toolbar_show_menu_icon, GTK_ALIGN_FILL);
        gtk_grid_attach (GTK_GRID (table), __widget_toolbar_show_menu_icon, 1, 2, 1, 1);

        gtk_widget_set_halign (__widget_toolbar_show_help_icon, GTK_ALIGN_FILL);
        gtk_grid_attach (GTK_GRID (table), __widget_toolbar_show_help_icon, 1, 3, 1, 1);

        gtk_widget_set_halign (__widget_toolbar_show_property_label, GTK_ALIGN_FILL);
        gtk_grid_attach (GTK_GRID (table), __widget_toolbar_show_property_label, 0, 4, 1, 1);

        hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
        gtk_box_append (GTK_BOX (vbox), hbox);

        // No "Input window" block here any more. The panel stopped drawing the
        // preedit and the lookup table -- every transport renders its own now --
        // so its two checkboxes ("embedded" and "vertical" lookup table) had
        // nothing left to act on and only wrote keys nobody reads. Orientation
        // moved to the candidates page, where the renderer that honours it lives.

        frame = gtk_frame_new (_("Misc"));
        gtk_widget_set_margin_start (frame, 4);
        gtk_widget_set_margin_end (frame, 4);
        gtk_widget_set_margin_top (frame, 4);
        gtk_widget_set_margin_bottom (frame, 4);
        gtk_widget_set_hexpand (frame, TRUE);
        gtk_box_append (GTK_BOX (hbox), frame);

        vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
        gtk_frame_set_child (GTK_FRAME (frame), vbox);

        __widget_show_tray_icon = gtk_check_button_new_with_mnemonic (_("Show tra_y icon"));
        gtk_box_append (GTK_BOX (vbox), __widget_show_tray_icon);

        __widget_default_sticked = gtk_check_button_new_with_mnemonic (_("Stick _windows"));
        gtk_box_append (GTK_BOX (vbox), __widget_default_sticked);

        hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_box_append (GTK_BOX (vbox), hbox);

        label = gtk_label_new_with_mnemonic (_("_Font:"));
        gtk_widget_set_margin_start (label, 4);
        gtk_widget_set_margin_end (label, 4);
        gtk_box_append (GTK_BOX (hbox), label);

        __widget_font = gtk_button_new_with_label ("default");
        gtk_box_append (GTK_BOX (hbox), __widget_font);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), __widget_font);

        // Panel colors (written under /Panel/Gtk/Color/*).
        frame = gtk_frame_new (_("Panel colors"));
        gtk_widget_set_margin_start (frame, 4);
        gtk_widget_set_margin_end (frame, 4);
        gtk_widget_set_margin_top (frame, 4);
        gtk_widget_set_margin_bottom (frame, 4);
        gtk_widget_set_hexpand (frame, TRUE);
        gtk_box_append (GTK_BOX (page), frame);

        table = gtk_grid_new ();
        gtk_grid_set_row_spacing (GTK_GRID (table), 4);
        gtk_grid_set_column_spacing (GTK_GRID (table), 8);
        gtk_widget_set_margin_start (table, 4);
        gtk_widget_set_margin_end (table, 4);
        gtk_widget_set_margin_top (table, 4);
        gtk_widget_set_margin_bottom (table, 4);
        gtk_frame_set_child (GTK_FRAME (frame), table);

        {
            struct { const char *label; GtkWidget **widget; String *cfg; } rows[] = {
                { _("N_ormal background:"),   &__widget_panel_normal_bg,   &__config_panel_normal_bg   },
                { _("No_rmal text:"),         &__widget_panel_normal_text, &__config_panel_normal_text },
                { _("Se_lected background:"), &__widget_panel_active_bg,   &__config_panel_active_bg   },
                { _("Selec_ted text:"),       &__widget_panel_active_text, &__config_panel_active_text },
            };
            for (int i = 0; i < 4; ++i) {
                label = gtk_label_new_with_mnemonic (rows[i].label);
                gtk_widget_set_halign (label, GTK_ALIGN_START);
                gtk_grid_attach (GTK_GRID (table), label, 0, i, 1, 1);

                *rows[i].widget = gtk_color_button_new ();
                gtk_grid_attach (GTK_GRID (table), *rows[i].widget, 1, i, 1, 1);
                gtk_label_set_mnemonic_widget (GTK_LABEL (label), *rows[i].widget);

                g_signal_connect ((gpointer) *rows[i].widget, "color-set",
                                  G_CALLBACK (on_panel_color_set),
                                  rows[i].cfg);
            }
        }

        // Connect all signals.
        g_signal_connect ((gpointer) __widget_toolbar_show_behavior, "changed",
                          G_CALLBACK (on_toolbar_show_behavior_changed),
                          NULL);

        g_signal_connect ((gpointer) __widget_toolbar_auto_snap, "toggled",
                          G_CALLBACK (on_default_check_button_toggled),
                          &__config_toolbar_auto_snap);

        g_signal_connect ((gpointer) __widget_toolbar_hide_timeout, "value_changed",
                          G_CALLBACK (on_default_spin_button_changed),
                          &__config_toolbar_hide_timeout);

        g_signal_connect ((gpointer) __widget_toolbar_show_factory_icon, "toggled",
                          G_CALLBACK (on_default_check_button_toggled),
                          &__config_toolbar_show_factory_icon);

        g_signal_connect ((gpointer) __widget_toolbar_show_factory_name, "toggled",
                          G_CALLBACK (on_default_check_button_toggled),
                          &__config_toolbar_show_factory_name);

        g_signal_connect ((gpointer) __widget_toolbar_show_stick_icon, "toggled",
                          G_CALLBACK (on_default_check_button_toggled),
                          &__config_toolbar_show_stick_icon);

        g_signal_connect ((gpointer) __widget_toolbar_show_menu_icon, "toggled",
                          G_CALLBACK (on_default_check_button_toggled),
                          &__config_toolbar_show_menu_icon);

        g_signal_connect ((gpointer) __widget_toolbar_show_help_icon, "toggled",
                          G_CALLBACK (on_default_check_button_toggled),
                          &__config_toolbar_show_help_icon);

        g_signal_connect ((gpointer) __widget_toolbar_show_property_label, "toggled",
                          G_CALLBACK (on_default_check_button_toggled),
                          &__config_toolbar_show_property_label);

        g_signal_connect ((gpointer) __widget_default_sticked, "toggled",
                          G_CALLBACK (on_default_check_button_toggled),
                          &__config_default_sticked);

        g_signal_connect ((gpointer) __widget_show_tray_icon, "toggled",
                          G_CALLBACK (on_default_check_button_toggled),
                          &__config_show_tray_icon);

        g_signal_connect ((gpointer) __widget_font, "clicked",
                          G_CALLBACK (on_font_selection_clicked),
                          NULL);

        // Set all tooltips.
        gtk_widget_set_tooltip_text (__widget_toolbar_show_behavior,
                              _("If option \"Always\" is selected, "
                                "the toolbar will always be shown on the screen. "
                                "If option \"On demand\" is selected, it will only be shown when SCIM "
                                "is activated. "
                                "If option \"Never\" is selected, it will never be shown."));

        gtk_widget_set_tooltip_text (__widget_toolbar_auto_snap,
                              _("If this option is checked, "
                                "the toolbar will be snapped to "
                                "the screen border."));


        gtk_widget_set_tooltip_text (__widget_toolbar_hide_timeout,
                              _("The toolbar will be hidden out after "
                                "this timeout is elapsed. "
                                "This option is only valid when "
                                "\"Always show\" is selected. "
                                "Set to zero to disable this behavior."));

        gtk_widget_set_tooltip_text (__widget_toolbar_show_factory_icon,
                              _("If this option is checked, "
                                "the input method icon will be showed on the toolbar."));

        gtk_widget_set_tooltip_text (__widget_toolbar_show_factory_name,
                              _("If this option is checked, "
                                "the input method name will be showed on the toolbar."));

        gtk_widget_set_tooltip_text (__widget_toolbar_show_stick_icon,
                              _("If this option is checked, "
                                "the stick icon will be showed on the toolbar."));

        gtk_widget_set_tooltip_text (__widget_toolbar_show_menu_icon,
                              _("If this option is checked, "
                                "the menu icon will be showed on the toolbar."));

        gtk_widget_set_tooltip_text (__widget_toolbar_show_help_icon,
                              _("If this option is checked, "
                                "the help icon will be showed on the toolbar."));

        gtk_widget_set_tooltip_text (__widget_toolbar_show_property_label,
                              _("If this option is checked, "
                                "the text label of input method properties will be showed on the toolbar."));

        gtk_widget_set_tooltip_text (__widget_show_tray_icon,
                              _("If this option is checked, "
                                "the tray icon will be showed on the desktop's taskbar."));

        gtk_widget_set_tooltip_text (__widget_default_sticked,
                              _("If this option is checked, "
                                "the toolbar, input and lookup table "
                                "windows will be sticked to "
                                "its original position."));

        gtk_widget_set_tooltip_text (__widget_font,
                              _("The font used by the panel windows."));

        window = page;

        setup_widget_value ();
    }
    return window;
}

void
setup_widget_value ()
{
    if (__widget_toolbar_show_behavior) {
        if (__config_toolbar_always_hidden) {
            gtk_combo_box_set_active (
                GTK_COMBO_BOX (__widget_toolbar_show_behavior),
                SCIM_TOOLBAR_SHOW_NEVER);
        } else if (__config_toolbar_always_show) {
            gtk_combo_box_set_active (
                GTK_COMBO_BOX (__widget_toolbar_show_behavior),
                SCIM_TOOLBAR_SHOW_ALWAYS);
        } else {
            gtk_combo_box_set_active (
                GTK_COMBO_BOX (__widget_toolbar_show_behavior),
                SCIM_TOOLBAR_SHOW_ON_DEMAND);
        }
    }

    if (__widget_toolbar_auto_snap) {
        gtk_check_button_set_active (
            GTK_CHECK_BUTTON (__widget_toolbar_auto_snap),
            __config_toolbar_auto_snap);
    }

    if (__widget_toolbar_hide_timeout) {
        gtk_spin_button_set_value (
            GTK_SPIN_BUTTON (__widget_toolbar_hide_timeout),
            __config_toolbar_hide_timeout);

        gtk_widget_set_sensitive (
            __widget_toolbar_hide_timeout,
            __config_toolbar_always_show);
    }

    if (__widget_toolbar_show_factory_icon) {
        gtk_check_button_set_active (
            GTK_CHECK_BUTTON (__widget_toolbar_show_factory_icon),
            __config_toolbar_show_factory_icon);
    }

    if (__widget_toolbar_show_factory_name) {
        gtk_check_button_set_active (
            GTK_CHECK_BUTTON (__widget_toolbar_show_factory_name),
            __config_toolbar_show_factory_name);
    }

    if (__widget_toolbar_show_stick_icon) {
        gtk_check_button_set_active (
            GTK_CHECK_BUTTON (__widget_toolbar_show_stick_icon),
            __config_toolbar_show_stick_icon);
    }

    if (__widget_toolbar_show_menu_icon) {
        gtk_check_button_set_active (
            GTK_CHECK_BUTTON (__widget_toolbar_show_menu_icon),
            __config_toolbar_show_menu_icon);
    }

    if (__widget_toolbar_show_help_icon) {
        gtk_check_button_set_active (
            GTK_CHECK_BUTTON (__widget_toolbar_show_help_icon),
            __config_toolbar_show_help_icon);
    }

    if (__widget_toolbar_show_property_label) {
        gtk_check_button_set_active (
            GTK_CHECK_BUTTON (__widget_toolbar_show_property_label),
            __config_toolbar_show_property_label);
    }

    if (__widget_default_sticked) {
        gtk_check_button_set_active (
            GTK_CHECK_BUTTON (__widget_default_sticked),
            __config_default_sticked);
    }

    if (__widget_show_tray_icon) {
        gtk_check_button_set_active (
            GTK_CHECK_BUTTON (__widget_show_tray_icon),
            __config_show_tray_icon);
    }

    if (__widget_font) {
        gtk_button_set_label (
            GTK_BUTTON (__widget_font),
            __config_font.c_str ());
    }

    if (__widget_panel_normal_bg)   set_color_button (__widget_panel_normal_bg,   __config_panel_normal_bg);
    if (__widget_panel_normal_text) set_color_button (__widget_panel_normal_text, __config_panel_normal_text);
    if (__widget_panel_active_bg)   set_color_button (__widget_panel_active_bg,   __config_panel_active_bg);
    if (__widget_panel_active_text) set_color_button (__widget_panel_active_text, __config_panel_active_text);
}

void
load_config (const ConfigPointer &config)
{
    if (!config.null ()) {
        __config_toolbar_always_hidden =
            config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_ALWAYS_HIDDEN),
                          __config_toolbar_always_hidden);
        __config_toolbar_always_show =
            config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_ALWAYS_SHOW),
                          __config_toolbar_always_show);
        __config_toolbar_auto_snap =
            config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_AUTO_SNAP),
                          __config_toolbar_auto_snap);
        __config_toolbar_hide_timeout =
            config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_HIDE_TIMEOUT),
                          __config_toolbar_hide_timeout);
        __config_toolbar_show_factory_icon =
            config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_FACTORY_ICON),
                          __config_toolbar_show_factory_icon);
        __config_toolbar_show_factory_name =
            config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_FACTORY_NAME),
                          __config_toolbar_show_factory_name);
        __config_toolbar_show_stick_icon =
            config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_STICK_ICON),
                          __config_toolbar_show_stick_icon);
        __config_toolbar_show_menu_icon =
            config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_MENU_ICON),
                          __config_toolbar_show_menu_icon);
        __config_toolbar_show_help_icon =
            config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_HELP_ICON),
                          __config_toolbar_show_help_icon);
        __config_toolbar_show_property_label =
            config->read (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_PROPERTY_LABEL),
                          __config_toolbar_show_property_label);
        __config_default_sticked =
            config->read (String (SCIM_CONFIG_PANEL_GTK_DEFAULT_STICKED),
                          __config_default_sticked);
        __config_show_tray_icon =
            config->read (String (SCIM_CONFIG_PANEL_GTK_SHOW_TRAY_ICON),
                          __config_show_tray_icon);
        __config_font =
            config->read (String (SCIM_CONFIG_PANEL_GTK_FONT),
                          __config_font);
        __config_panel_normal_bg =
            config->read (String (SCIM_CONFIG_PANEL_GTK_COLOR_NORMAL_BG),   __config_panel_normal_bg);
        __config_panel_normal_text =
            config->read (String (SCIM_CONFIG_PANEL_GTK_COLOR_NORMAL_TEXT), __config_panel_normal_text);
        __config_panel_active_bg =
            config->read (String (SCIM_CONFIG_PANEL_GTK_COLOR_ACTIVE_BG),   __config_panel_active_bg);
        __config_panel_active_text =
            config->read (String (SCIM_CONFIG_PANEL_GTK_COLOR_ACTIVE_TEXT), __config_panel_active_text);

        setup_widget_value ();

        __have_changed = false;
    }
}

void
save_config (const ConfigPointer &config)
{
    if (!config.null ()) {
        config->write (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_ALWAYS_HIDDEN),
                       __config_toolbar_always_hidden);
        config->write (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_ALWAYS_SHOW),
                       __config_toolbar_always_show);
        config->write (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_AUTO_SNAP),
                       __config_toolbar_auto_snap);
        config->write (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_HIDE_TIMEOUT),
                       __config_toolbar_hide_timeout);
        config->write (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_FACTORY_ICON),
                       __config_toolbar_show_factory_icon);
        config->write (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_FACTORY_NAME),
                       __config_toolbar_show_factory_name);
        config->write (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_STICK_ICON),
                       __config_toolbar_show_stick_icon);
        config->write (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_MENU_ICON),
                       __config_toolbar_show_menu_icon);
        config->write (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_HELP_ICON),
                       __config_toolbar_show_help_icon);
        config->write (String (SCIM_CONFIG_PANEL_GTK_TOOLBAR_SHOW_PROPERTY_LABEL),
                       __config_toolbar_show_property_label);
        config->write (String (SCIM_CONFIG_PANEL_GTK_SHOW_TRAY_ICON),
                       __config_show_tray_icon);
        config->write (String (SCIM_CONFIG_PANEL_GTK_DEFAULT_STICKED),
                       __config_default_sticked);
        config->write (String (SCIM_CONFIG_PANEL_GTK_FONT),
                       __config_font);
        config->write (String (SCIM_CONFIG_PANEL_GTK_COLOR_NORMAL_BG),   __config_panel_normal_bg);
        config->write (String (SCIM_CONFIG_PANEL_GTK_COLOR_NORMAL_TEXT), __config_panel_normal_text);
        config->write (String (SCIM_CONFIG_PANEL_GTK_COLOR_ACTIVE_BG),   __config_panel_active_bg);
        config->write (String (SCIM_CONFIG_PANEL_GTK_COLOR_ACTIVE_TEXT), __config_panel_active_text);

        __have_changed = false;
    }
}

bool
query_changed ()
{
    return __have_changed;
}

static void
on_default_spin_button_changed (GtkSpinButton *spinbutton,
                                gpointer       user_data)
{
	int *value = static_cast <int *> (user_data);

	if (value) {
		*value = gtk_spin_button_get_value_as_int (spinbutton);
		__have_changed = true;
	}
}

static void
on_default_check_button_toggled (GtkCheckButton *checkbutton,
                                 gpointer        user_data)
{
    bool *toggle = static_cast<bool*> (user_data);

    if (toggle) {
        *toggle = gtk_check_button_get_active (checkbutton);
        __have_changed = true;
    }
}

static void
on_toolbar_show_behavior_changed (GtkComboBox *combobox,
                                 gpointer     /* user_data */)
{
    gint active;
    active  = gtk_combo_box_get_active (combobox);

    switch (active) {
        case SCIM_TOOLBAR_SHOW_ALWAYS:
            __config_toolbar_always_show   = true;
            __config_toolbar_always_hidden = false;
            break;
        case SCIM_TOOLBAR_SHOW_ON_DEMAND:
            __config_toolbar_always_show   = false;
            __config_toolbar_always_hidden = false;
            break;
        case SCIM_TOOLBAR_SHOW_NEVER:
            __config_toolbar_always_show   = false;
            __config_toolbar_always_hidden = true;
            break;
        default:
            __config_toolbar_always_show   = true;
            __config_toolbar_always_hidden = false;
            break;
    }

    if (__widget_toolbar_hide_timeout) {
        gtk_widget_set_sensitive (
            __widget_toolbar_hide_timeout,
            !__config_toolbar_always_hidden &&
            __config_toolbar_always_show);
    }

    if (__widget_toolbar_auto_snap) {
        gtk_widget_set_sensitive (
            __widget_toolbar_auto_snap,
            !__config_toolbar_always_hidden);
    }

    if (__widget_toolbar_show_factory_icon) {
        gtk_widget_set_sensitive (
            __widget_toolbar_show_factory_icon,
            !__config_toolbar_always_hidden);
    }

    if (__widget_toolbar_show_factory_name) {
        gtk_widget_set_sensitive (
            __widget_toolbar_show_factory_name,
            !__config_toolbar_always_hidden);
    }

    if (__widget_toolbar_show_stick_icon) {
        gtk_widget_set_sensitive (
            __widget_toolbar_show_stick_icon,
            !__config_toolbar_always_hidden);
    }

    if (__widget_toolbar_show_menu_icon) {
        gtk_widget_set_sensitive (
            __widget_toolbar_show_menu_icon,
            !__config_toolbar_always_hidden);
    }

    if (__widget_toolbar_show_help_icon) {
        gtk_widget_set_sensitive (
            __widget_toolbar_show_help_icon,
            !__config_toolbar_always_hidden);
    }

    if (__widget_toolbar_show_property_label) {
        gtk_widget_set_sensitive (
            __widget_toolbar_show_property_label,
            !__config_toolbar_always_hidden);
    }

    __have_changed = true;
}

static void
font_dialog_response_cb (GtkDialog *dialog,
                         gint       response,
                         gpointer   /* user_data */)
{
    if (response == GTK_RESPONSE_OK) {
        gchar *fontname = gtk_font_chooser_get_font (GTK_FONT_CHOOSER (dialog));

        if (fontname) {
            __config_font = String (fontname);
            g_free (fontname);

            gtk_button_set_label (
                GTK_BUTTON (__widget_font),
                __config_font.c_str ());

            __have_changed = true;
        }
    }

    gtk_window_destroy (GTK_WINDOW (dialog));
}

static void
on_font_selection_clicked (GtkButton *button,
                           gpointer   /* user_data */)
{
    GtkWidget *font_selection = gtk_font_chooser_dialog_new (_("Select Interface Font"), NULL);
    GtkRoot   *root = gtk_widget_get_root (GTK_WIDGET (button));

    if (__config_font != "default") {
        gtk_font_chooser_set_font (
            GTK_FONT_CHOOSER (font_selection),
            __config_font.c_str ());
    }

    if (root && GTK_IS_WINDOW (root))
        gtk_window_set_transient_for (GTK_WINDOW (font_selection), GTK_WINDOW (root));
    gtk_window_set_modal (GTK_WINDOW (font_selection), TRUE);

    g_signal_connect (font_selection, "response", G_CALLBACK (font_dialog_response_cb), NULL);

    gtk_window_present (GTK_WINDOW (font_selection));
}

// Parse a color string (X11/CSS name or #rrggbb) the same way the candidate
// renderer does (Pango), and show it in the color button.
static void
set_color_button (GtkWidget *button, const String &color)
{
    GdkRGBA rgba = { 0.5, 0.5, 0.5, 1.0 };
    PangoColor pc;

    if (color.length () && pango_color_parse (&pc, color.c_str ())) {
        rgba.red   = pc.red   / 65535.0;
        rgba.green = pc.green / 65535.0;
        rgba.blue  = pc.blue  / 65535.0;
        rgba.alpha = 1.0;
    }

    gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (button), &rgba);
}

// Read a color button as #rrggbb, which the panel and the candidate renderer's
// pango_color_parse both read back.
static String
color_button_hex (GtkColorButton *button)
{
    GdkRGBA rgba;
    gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (button), &rgba);

    gchar *hex = g_strdup_printf ("#%02x%02x%02x",
        (int) (CLAMP (rgba.red,   0.0, 1.0) * 255.0 + 0.5),
        (int) (CLAMP (rgba.green, 0.0, 1.0) * 255.0 + 0.5),
        (int) (CLAMP (rgba.blue,  0.0, 1.0) * 255.0 + 0.5));
    String s (hex);
    g_free (hex);
    return s;
}

static void
on_panel_color_set (GtkColorButton *button,
                    gpointer        user_data)
{
    String *cfg = static_cast<String *> (user_data);
    if (!cfg)
        return;

    *cfg = color_button_hex (button);
    __have_changed = true;
}

/*
vi:ts=4:nowrap:expandtab
*/
