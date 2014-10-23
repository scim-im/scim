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
#define SCIM_CONFIG_PANEL_GTK_LOOKUP_TABLE_EMBEDDED     "/Panel/Gtk/LookupTableEmbedded"
#define SCIM_CONFIG_PANEL_GTK_LOOKUP_TABLE_VERTICAL     "/Panel/Gtk/LookupTableVertical"
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
        return String (_("A panel daemon based on GTK+-2.x library."));
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
static bool   __config_lookup_table_embedded     = true;
static bool   __config_lookup_table_vertical     = false;
static bool   __config_default_sticked           = false;
static bool   __config_show_tray_icon            = true;

static String __config_font                      = "default";

static bool   __have_changed                     = false;

static GtkWidget * __widget_toolbar_show_behaviour    = 0;
static GtkWidget * __widget_toolbar_auto_snap         = 0;
static GtkWidget * __widget_toolbar_hide_timeout      = 0;
static GtkWidget * __widget_toolbar_show_factory_icon  = 0;
static GtkWidget * __widget_toolbar_show_factory_name  = 0;
static GtkWidget * __widget_toolbar_show_stick_icon   = 0;
static GtkWidget * __widget_toolbar_show_menu_icon   = 0;
static GtkWidget * __widget_toolbar_show_help_icon    = 0;
static GtkWidget * __widget_toolbar_show_property_label = 0;
static GtkWidget * __widget_lookup_table_embedded     = 0;
static GtkWidget * __widget_lookup_table_vertical     = 0;
static GtkWidget * __widget_default_sticked           = 0;
static GtkWidget * __widget_show_tray_icon            = 0;
static GtkWidget * __widget_font                      = 0;

#if GTK_CHECK_VERSION(2, 12, 0)
#else
static GtkTooltips * __widget_tooltips                = 0;
#endif

enum ToolbarShowFlavourType {
    SCIM_TOOLBAR_SHOW_ALWAYS,
    SCIM_TOOLBAR_SHOW_ON_DEMAND,
    SCIM_TOOLBAR_SHOW_NEVER
};

static const char * __toolbar_show_behaviour_text[] = {
    N_("Always"),
    N_("On demand"),
    N_("Never")
};

// Declaration of internal functions.
static void
on_default_toggle_button_toggled     (GtkToggleButton *togglebutton,
                                      gpointer         user_data);

static void
on_default_spin_button_changed       (GtkSpinButton   *spinbutton,
                                      gpointer         user_data);

static void
on_toolbar_show_behaviour_changed      (GtkComboBox     *combobox,
                                      gpointer         user_data);

static void
on_font_selection_clicked            (GtkButton       *button,
                                      gpointer         user_data);

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

#if GTK_CHECK_VERSION(2, 12, 0)
#else
        __widget_tooltips = gtk_tooltips_new ();
#endif

        // Create the vbox for the first page.
#if GTK_CHECK_VERSION(3, 0, 0)
        page = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
#else
        page = gtk_vbox_new (FALSE, 0);
#endif
        gtk_widget_show (page);

        vbox = page;

        // Create the ToolBar setup block.
        frame = gtk_frame_new (_("ToolBar"));
        gtk_widget_show (frame);
        gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (frame), 4);

#if GTK_CHECK_VERSION(3, 4, 0)
        table = gtk_grid_new();
        gtk_grid_set_row_spacing (GTK_GRID (table), 4);
        gtk_grid_set_column_spacing (GTK_GRID (table), 8);
#else
        table = gtk_table_new (4, 2, FALSE);
        gtk_table_set_row_spacings (GTK_TABLE (table), 4);
        gtk_table_set_col_spacings (GTK_TABLE (table), 8);
#endif
        gtk_widget_show (table);
        gtk_container_add (GTK_CONTAINER (frame), table);

#if GTK_CHECK_VERSION(3, 0, 0)
        hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
#else
        hbox = gtk_hbox_new (FALSE, 0);
#endif
        gtk_widget_show (hbox);

#if GTK_CHECK_VERSION(3, 4, 0)
        gtk_widget_set_halign (hbox, GTK_ALIGN_FILL);
        gtk_grid_attach(GTK_GRID (table), hbox, 0, 0, 1, 1);
#else
        gtk_table_attach (GTK_TABLE (table), hbox, 0, 1, 0, 1,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (GTK_EXPAND), 4, 0);
#endif

        label = gtk_label_new_with_mnemonic (_("_Show:"));
        gtk_widget_show (label);
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
#if GTK_CHECK_VERSION(3, 14, 0)
        gtk_widget_set_margin_start (label, 4);
        gtk_widget_set_margin_end (label, 4);
#else
        gtk_misc_set_padding (GTK_MISC (label), 4, 0);
#endif
 
#if GTK_CHECK_VERSION(2, 24, 0)
        __widget_toolbar_show_behaviour = gtk_combo_box_text_new ();
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (__widget_toolbar_show_behaviour), 
                                   _(__toolbar_show_behaviour_text[SCIM_TOOLBAR_SHOW_ALWAYS]));
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (__widget_toolbar_show_behaviour),
                                   _(__toolbar_show_behaviour_text[SCIM_TOOLBAR_SHOW_ON_DEMAND]));
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (__widget_toolbar_show_behaviour),
                                   _(__toolbar_show_behaviour_text[SCIM_TOOLBAR_SHOW_NEVER]));
#else
        __widget_toolbar_show_behaviour = gtk_combo_box_new_text ();
        gtk_combo_box_append_text (GTK_COMBO_BOX (__widget_toolbar_show_behaviour), 
                                   _(__toolbar_show_behaviour_text[SCIM_TOOLBAR_SHOW_ALWAYS]));
        gtk_combo_box_append_text (GTK_COMBO_BOX (__widget_toolbar_show_behaviour),
                                   _(__toolbar_show_behaviour_text[SCIM_TOOLBAR_SHOW_ON_DEMAND]));
        gtk_combo_box_append_text (GTK_COMBO_BOX (__widget_toolbar_show_behaviour),
                                   _(__toolbar_show_behaviour_text[SCIM_TOOLBAR_SHOW_NEVER]));
#endif
        gtk_widget_show (__widget_toolbar_show_behaviour);
        gtk_box_pack_start (GTK_BOX (hbox), __widget_toolbar_show_behaviour, FALSE, FALSE, 0);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), __widget_toolbar_show_behaviour);

        __widget_toolbar_auto_snap = gtk_check_button_new_with_mnemonic (_("Auto s_nap"));
        gtk_widget_show (__widget_toolbar_auto_snap);

        __widget_toolbar_show_factory_icon = gtk_check_button_new_with_mnemonic (_("Show _input method icon"));
        gtk_widget_show (__widget_toolbar_show_factory_icon);

        __widget_toolbar_show_factory_name = gtk_check_button_new_with_mnemonic (_("Show inp_ut method name"));
        gtk_widget_show (__widget_toolbar_show_factory_name);

#if GTK_CHECK_VERSION(3, 4, 0)
        gtk_widget_set_halign (__widget_toolbar_auto_snap, GTK_ALIGN_FILL);
        gtk_grid_attach(GTK_GRID (table), __widget_toolbar_auto_snap, 0, 1, 1, 1);

        gtk_widget_set_halign (__widget_toolbar_show_factory_icon, GTK_ALIGN_FILL);
        gtk_grid_attach(GTK_GRID (table), __widget_toolbar_show_factory_icon, 0, 2, 1, 1);

        gtk_widget_set_halign (__widget_toolbar_show_factory_name, GTK_ALIGN_FILL);
        gtk_grid_attach(GTK_GRID (table), __widget_toolbar_show_factory_name, 0, 3, 1, 1);
#else
        gtk_table_attach (GTK_TABLE (table), __widget_toolbar_auto_snap, 0, 1, 1, 2,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (GTK_EXPAND), 4, 0);
        gtk_table_attach (GTK_TABLE (table), __widget_toolbar_show_factory_icon, 0, 1, 2, 3,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (GTK_EXPAND), 4, 0);
        gtk_table_attach (GTK_TABLE (table), __widget_toolbar_show_factory_name, 0, 1, 3, 4,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (GTK_EXPAND), 4, 0);
#endif



#if GTK_CHECK_VERSION(3, 0, 0)
        hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
#else
        hbox = gtk_hbox_new (FALSE, 0);
#endif
        gtk_widget_show (hbox);

#if GTK_CHECK_VERSION(3, 4, 0)
        gtk_widget_set_halign (hbox, GTK_ALIGN_FILL);
        gtk_grid_attach (GTK_GRID (table), hbox, 1, 0, 1, 1);
#else
        gtk_table_attach (GTK_TABLE (table), hbox, 1, 2, 0, 1,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (GTK_EXPAND), 4, 0);
#endif

        label = gtk_label_new_with_mnemonic (_("Hide time_out:"));
        gtk_widget_show (label);
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
#if GTK_CHECK_VERSION(3, 14, 0)
        gtk_widget_set_margin_start (label, 4);
        gtk_widget_set_margin_end (label, 4);
#else
        gtk_misc_set_padding (GTK_MISC (label), 4, 0);
#endif

        __widget_toolbar_hide_timeout = gtk_spin_button_new_with_range (0, 60, 1);
        gtk_widget_show (__widget_toolbar_hide_timeout);
        gtk_box_pack_start (GTK_BOX (hbox), __widget_toolbar_hide_timeout, FALSE, FALSE, 0);
        gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (__widget_toolbar_hide_timeout), TRUE);
        gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (__widget_toolbar_hide_timeout), TRUE);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON (__widget_toolbar_hide_timeout), 0);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), __widget_toolbar_hide_timeout);

        __widget_toolbar_show_stick_icon = gtk_check_button_new_with_mnemonic (_("Show s_tick icon"));
        gtk_widget_show (__widget_toolbar_show_stick_icon);

        __widget_toolbar_show_menu_icon = gtk_check_button_new_with_mnemonic (_("Show m_enu icon"));
        gtk_widget_show (__widget_toolbar_show_menu_icon);

        __widget_toolbar_show_help_icon = gtk_check_button_new_with_mnemonic (_("Show _help icon"));
        gtk_widget_show (__widget_toolbar_show_help_icon);

        __widget_toolbar_show_property_label = gtk_check_button_new_with_mnemonic (_("Show _property label"));
        gtk_widget_show (__widget_toolbar_show_property_label);
#if GTK_CHECK_VERSION(3, 4, 0)
        gtk_widget_set_halign (__widget_toolbar_show_stick_icon, GTK_ALIGN_FILL);
        gtk_grid_attach (GTK_GRID (table), __widget_toolbar_show_stick_icon, 1, 1, 1, 1);

        gtk_widget_set_halign (__widget_toolbar_show_menu_icon, GTK_ALIGN_FILL);
        gtk_grid_attach (GTK_GRID (table), __widget_toolbar_show_menu_icon, 1, 2, 1, 1);

        gtk_widget_set_halign (__widget_toolbar_show_help_icon, GTK_ALIGN_FILL);
        gtk_grid_attach (GTK_GRID (table), __widget_toolbar_show_help_icon, 1, 3, 1, 1);

        gtk_widget_set_halign (__widget_toolbar_show_property_label, GTK_ALIGN_FILL);
        gtk_grid_attach (GTK_GRID (table), __widget_toolbar_show_property_label, 0, 4, 1, 1);
#else
        gtk_table_attach (GTK_TABLE (table), __widget_toolbar_show_stick_icon, 1, 2, 1, 2,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (GTK_EXPAND), 4, 0);
        gtk_table_attach (GTK_TABLE (table), __widget_toolbar_show_menu_icon, 1, 2, 2, 3,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (GTK_EXPAND), 4, 0);
        gtk_table_attach (GTK_TABLE (table), __widget_toolbar_show_help_icon, 1, 2, 3, 4,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (GTK_EXPAND), 4, 0);
        gtk_table_attach (GTK_TABLE (table), __widget_toolbar_show_property_label, 0, 1, 4, 5,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (GTK_EXPAND), 4, 0);
#endif


#if GTK_CHECK_VERSION(3, 0, 0)
        hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
#else
        hbox = gtk_hbox_new (FALSE, 8);
#endif
        gtk_widget_show (hbox);
        gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

        // Create the Input Window setup block
        frame = gtk_frame_new (_("Input window"));
        gtk_widget_show (frame);
        gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
        gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);

#if GTK_CHECK_VERSION(3, 0, 0)
        vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
#else
        vbox = gtk_vbox_new (FALSE, 4);
#endif
        gtk_widget_show (vbox);
        gtk_container_add (GTK_CONTAINER (frame), vbox);

        __widget_lookup_table_embedded = gtk_check_button_new_with_mnemonic (_("E_mbedded lookup table"));
        gtk_widget_show (__widget_lookup_table_embedded);
        gtk_box_pack_start (GTK_BOX (vbox), __widget_lookup_table_embedded, FALSE, FALSE, 0);

        __widget_lookup_table_vertical = gtk_check_button_new_with_mnemonic (_("_Vertical lookup table"));
        gtk_widget_show (__widget_lookup_table_vertical);
        gtk_box_pack_start (GTK_BOX (vbox), __widget_lookup_table_vertical, FALSE, FALSE, 0);

        frame = gtk_frame_new (_("Misc"));
        gtk_widget_show (frame);
        gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
        gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);

#if GTK_CHECK_VERSION(3, 0, 0)
        vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
#else
        vbox = gtk_vbox_new (FALSE, 4);
#endif
        gtk_widget_show (vbox);
        gtk_container_add (GTK_CONTAINER (frame), vbox);

        __widget_show_tray_icon = gtk_check_button_new_with_mnemonic (_("Show tra_y icon"));
        gtk_widget_show (__widget_show_tray_icon);
        gtk_box_pack_start (GTK_BOX (vbox), __widget_show_tray_icon, FALSE, FALSE, 0);

        __widget_default_sticked = gtk_check_button_new_with_mnemonic (_("Stick _windows"));
        gtk_widget_show (__widget_default_sticked);
        gtk_box_pack_start (GTK_BOX (vbox), __widget_default_sticked, FALSE, FALSE, 0);

#if GTK_CHECK_VERSION(3, 0, 0)
        hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
#else
        hbox = gtk_hbox_new (FALSE, 0);
#endif
        gtk_widget_show (hbox);
        gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

        label = gtk_label_new_with_mnemonic (_("_Font:"));
        gtk_widget_show (label);
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
#if GTK_CHECK_VERSION(3, 14, 0)
        gtk_widget_set_margin_start (label, 4);
        gtk_widget_set_margin_end (label, 4);
#else
        gtk_misc_set_padding (GTK_MISC (label), 4, 0);
#endif

        __widget_font = gtk_button_new_with_label ("default");
        gtk_widget_show (__widget_font);
        gtk_container_set_border_width (GTK_CONTAINER (__widget_font), 4);
        gtk_box_pack_start (GTK_BOX (hbox), __widget_font, FALSE, FALSE, 0);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), __widget_font);

        // Connect all signals.
        g_signal_connect ((gpointer) __widget_toolbar_show_behaviour, "changed",
                          G_CALLBACK (on_toolbar_show_behaviour_changed),
                          NULL);

        g_signal_connect ((gpointer) __widget_toolbar_auto_snap, "toggled",
                          G_CALLBACK (on_default_toggle_button_toggled),
                          &__config_toolbar_auto_snap);

        g_signal_connect ((gpointer) __widget_toolbar_hide_timeout, "value_changed",
                          G_CALLBACK (on_default_spin_button_changed),
                          &__config_toolbar_hide_timeout);

        g_signal_connect ((gpointer) __widget_toolbar_show_factory_icon, "toggled",
                          G_CALLBACK (on_default_toggle_button_toggled),
                          &__config_toolbar_show_factory_icon);

        g_signal_connect ((gpointer) __widget_toolbar_show_factory_name, "toggled",
                          G_CALLBACK (on_default_toggle_button_toggled),
                          &__config_toolbar_show_factory_name);

        g_signal_connect ((gpointer) __widget_toolbar_show_stick_icon, "toggled",
                          G_CALLBACK (on_default_toggle_button_toggled),
                          &__config_toolbar_show_stick_icon);

        g_signal_connect ((gpointer) __widget_toolbar_show_menu_icon, "toggled",
                          G_CALLBACK (on_default_toggle_button_toggled),
                          &__config_toolbar_show_menu_icon);

        g_signal_connect ((gpointer) __widget_toolbar_show_help_icon, "toggled",
                          G_CALLBACK (on_default_toggle_button_toggled),
                          &__config_toolbar_show_help_icon);

        g_signal_connect ((gpointer) __widget_toolbar_show_property_label, "toggled",
                          G_CALLBACK (on_default_toggle_button_toggled),
                          &__config_toolbar_show_property_label);

        g_signal_connect ((gpointer) __widget_lookup_table_embedded, "toggled",
                          G_CALLBACK (on_default_toggle_button_toggled),
                          &__config_lookup_table_embedded);

        g_signal_connect ((gpointer) __widget_lookup_table_vertical, "toggled",
                          G_CALLBACK (on_default_toggle_button_toggled),
                          &__config_lookup_table_vertical);

        g_signal_connect ((gpointer) __widget_default_sticked, "toggled",
                          G_CALLBACK (on_default_toggle_button_toggled),
                          &__config_default_sticked);

        g_signal_connect ((gpointer) __widget_show_tray_icon, "toggled",
                          G_CALLBACK (on_default_toggle_button_toggled),
                          &__config_show_tray_icon);

        g_signal_connect ((gpointer) __widget_font, "clicked",
                          G_CALLBACK (on_font_selection_clicked),
                          NULL);

        // Set all tooltips.
#if GTK_CHECK_VERSION(2, 12, 0)
        gtk_widget_set_tooltip_text (__widget_toolbar_show_behaviour,
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

        gtk_widget_set_tooltip_text (__widget_lookup_table_embedded,
                              _("If this option is checked, "
                                "the lookup table will be embedded into "
                                "the input window."));

        gtk_widget_set_tooltip_text (__widget_lookup_table_vertical,
                              _("If this option is checked, "
                                "the lookup table will be displayed "
                                "vertically."));

        gtk_widget_set_tooltip_text (__widget_show_tray_icon,
                              _("If this option is checked, "
                                "the tray icon will be showed on the desktop's taskbar."));

        gtk_widget_set_tooltip_text (__widget_default_sticked,
                              _("If this option is checked, "
                                "the toolbar, input and lookup table "
                                "windows will be sticked to "
                                "its original position."));

        gtk_widget_set_tooltip_text (__widget_font,
                              _("The font setting will be used in "
                                "the input and lookup table windows."));
#else
        gtk_tooltips_set_tip (__widget_tooltips, __widget_toolbar_show_behaviour,
                              _("If option \"Always\" is selected, "
                                "the toolbar will always be shown on the screen. "
                                "If option \"On demand\" is selected, it will only be shown when SCIM "
                                "is activated. "
                                "If option \"Never\" is selected, it will never be shown."), NULL);

        gtk_tooltip_set_tip (__widget_tooltips, __widget_toolbar_auto_snap,
                              _("If this option is checked, "
                                "the toolbar will be snapped to "
                                "the screen border.", NULL));


        gtk_tooltip_set_tip (__widget_tooltips, __widget_toolbar_hide_timeout,
                              _("The toolbar will be hidden out after "
                                "this timeout is elapsed. "
                                "This option is only valid when "
                                "\"Always show\" is selected. "
                                "Set to zero to disable this behavior.", NULL));

        gtk_tooltip_set_tip (__widget_tooltips, __widget_toolbar_show_factory_icon,
                              _("If this option is checked, "
                                "the input method icon will be showed on the toolbar.", NULL));

        gtk_tooltip_set_tip (__widget_tooltips, __widget_toolbar_show_factory_name,
                              _("If this option is checked, "
                                "the input method name will be showed on the toolbar.", NULL));

        gtk_tooltip_set_tip (__widget_tooltips, __widget_toolbar_show_stick_icon,
                              _("If this option is checked, "
                                "the stick icon will be showed on the toolbar.", NULL));

        gtk_tooltip_set_tip (__widget_tooltips, __widget_toolbar_show_menu_icon,
                              _("If this option is checked, "
                                "the menu icon will be showed on the toolbar.", NULL));

        gtk_tooltip_set_tip (__widget_tooltips, __widget_toolbar_show_help_icon,
                              _("If this option is checked, "
                                "the help icon will be showed on the toolbar.", NULL));

        gtk_tooltip_set_tip (__widget_tooltips, __widget_toolbar_show_property_label,
                              _("If this option is checked, "
                                "the text label of input method properties will be showed on the toolbar.", NULL));

        gtk_tooltip_set_tip (__widget_tooltips, __widget_lookup_table_embedded,
                              _("If this option is checked, "
                                "the lookup table will be embedded into "
                                "the input window.", NULL));

        gtk_tooltip_set_tip (__widget_tooltips, __widget_lookup_table_vertical,
                              _("If this option is checked, "
                                "the lookup table will be displayed "
                                "vertically.", NULL));

        gtk_tooltip_set_tip (__widget_tooltips, __widget_show_tray_icon,
                              _("If this option is checked, "
                                "the tray icon will be showed on the desktop's taskbar.", NULL));

        gtk_tooltip_set_tip (__widget_tooltips, __widget_default_sticked,
                              _("If this option is checked, "
                                "the toolbar, input and lookup table "
                                "windows will be sticked to "
                                "its original position.", NULL));

        gtk_tooltip_set_tip (__widget_tooltips, __widget_font,
                              _("The font setting will be used in "
                                "the input and lookup table windows.", NULL));
#endif


        window = page;

        setup_widget_value ();
    }
    return window;
}

void
setup_widget_value ()
{
    if (__widget_toolbar_show_behaviour) {
        if (__config_toolbar_always_hidden) {
            gtk_combo_box_set_active (
                GTK_COMBO_BOX (__widget_toolbar_show_behaviour),
                SCIM_TOOLBAR_SHOW_NEVER);
        } else if (__config_toolbar_always_show) {
            gtk_combo_box_set_active (
                GTK_COMBO_BOX (__widget_toolbar_show_behaviour),
                SCIM_TOOLBAR_SHOW_ALWAYS);
        } else {
            gtk_combo_box_set_active (
                GTK_COMBO_BOX (__widget_toolbar_show_behaviour),
                SCIM_TOOLBAR_SHOW_ON_DEMAND);
        }
    }

    if (__widget_toolbar_auto_snap) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_toolbar_auto_snap),
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
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_toolbar_show_factory_icon),
            __config_toolbar_show_factory_icon);
    }

    if (__widget_toolbar_show_factory_name) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_toolbar_show_factory_name),
            __config_toolbar_show_factory_name);
    }

    if (__widget_toolbar_show_stick_icon) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_toolbar_show_stick_icon),
            __config_toolbar_show_stick_icon);
    }

    if (__widget_toolbar_show_menu_icon) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_toolbar_show_menu_icon),
            __config_toolbar_show_menu_icon);
    }

    if (__widget_toolbar_show_help_icon) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_toolbar_show_help_icon),
            __config_toolbar_show_help_icon);
    }

    if (__widget_toolbar_show_property_label) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_toolbar_show_property_label),
            __config_toolbar_show_property_label);
    }

    if (__widget_lookup_table_embedded) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_lookup_table_embedded),
            __config_lookup_table_embedded);
    }

    if (__widget_lookup_table_vertical) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_lookup_table_vertical),
            __config_lookup_table_vertical);
    }

    if (__widget_default_sticked) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_default_sticked),
            __config_default_sticked);
    }

    if (__widget_show_tray_icon) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_show_tray_icon),
            __config_show_tray_icon);
    }

    if (__widget_font) {
        gtk_button_set_label (
            GTK_BUTTON (__widget_font),
            __config_font.c_str ());
    }
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
        __config_lookup_table_embedded =
            config->read (String (SCIM_CONFIG_PANEL_GTK_LOOKUP_TABLE_EMBEDDED),
                          __config_lookup_table_embedded);
        __config_lookup_table_vertical =
            config->read (String (SCIM_CONFIG_PANEL_GTK_LOOKUP_TABLE_VERTICAL),
                          __config_lookup_table_vertical);
        __config_default_sticked =
            config->read (String (SCIM_CONFIG_PANEL_GTK_DEFAULT_STICKED),
                          __config_default_sticked);
        __config_show_tray_icon =
            config->read (String (SCIM_CONFIG_PANEL_GTK_SHOW_TRAY_ICON),
                          __config_show_tray_icon);
        __config_font =
            config->read (String (SCIM_CONFIG_PANEL_GTK_FONT),
                          __config_font);

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
        config->write (String (SCIM_CONFIG_PANEL_GTK_LOOKUP_TABLE_EMBEDDED),
                        __config_lookup_table_embedded);
        config->write (String (SCIM_CONFIG_PANEL_GTK_LOOKUP_TABLE_VERTICAL),
                       __config_lookup_table_vertical);
        config->write (String (SCIM_CONFIG_PANEL_GTK_SHOW_TRAY_ICON),
                       __config_show_tray_icon);
        config->write (String (SCIM_CONFIG_PANEL_GTK_DEFAULT_STICKED),
                       __config_default_sticked);
        config->write (String (SCIM_CONFIG_PANEL_GTK_FONT),
                       __config_font);

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
on_default_toggle_button_toggled (GtkToggleButton *togglebutton,
                                  gpointer         user_data)
{
    bool *toggle = static_cast<bool*> (user_data);

    if (toggle) {
        *toggle = gtk_toggle_button_get_active (togglebutton);
        __have_changed = true;
    }
}

static void
on_toolbar_show_behaviour_changed (GtkComboBox *combobox,
                                 gpointer     user_data)
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
on_font_selection_clicked (GtkButton *button,
                           gpointer   user_data)
{
#if GTK_CHECK_VERSION(3, 2, 0)
    GtkWidget *font_selection = gtk_font_chooser_dialog_new (_("Select Interface Font"), NULL);
    gint result;

    if (__config_font != "default") {
        gtk_font_chooser_set_font(
            GTK_FONT_CHOOSER (font_selection),
            __config_font.c_str ());
    }

    result = gtk_dialog_run (GTK_DIALOG (font_selection));

    if (result == GTK_RESPONSE_OK) {
        gchar *fontname = gtk_font_chooser_get_font ( GTK_FONT_CHOOSER (font_selection));
        __config_font = String (fontname);
        g_free(fontname);

        gtk_button_set_label (
            GTK_BUTTON (__widget_font),
            __config_font.c_str ());

        __have_changed = true;
    }

    gtk_widget_destroy (font_selection);
#else
    GtkWidget *font_selection = gtk_font_selection_dialog_new (_("Select Interface Font"));
    gint result;

    if (__config_font != "default") {
        gtk_font_selection_dialog_set_font_name (
            GTK_FONT_SELECTION_DIALOG (font_selection),
            __config_font.c_str ());
    }

    result = gtk_dialog_run (GTK_DIALOG (font_selection));

    if (result == GTK_RESPONSE_OK) {
        __config_font = String (
                            gtk_font_selection_dialog_get_font_name (
                                GTK_FONT_SELECTION_DIALOG (font_selection)));

        gtk_button_set_label (
            GTK_BUTTON (__widget_font),
            __config_font.c_str ());

        __have_changed = true;
    }

    gtk_widget_destroy (font_selection);
#endif
}

/*
vi:ts=4:nowrap:expandtab
*/
