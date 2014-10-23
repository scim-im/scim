/** @file scim_aaa_frontend_setup.cpp
 * implementation of Setup Module for FrontEnd Hotkeys configuration.
 */

/*
 * Smart Common Input Method
 *
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
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
 * $Id: scim_frontend_setup.cpp,v 1.6 2005/06/29 08:19:17 suzhe Exp $
 *
 */

#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_EVENT

#include <iostream>

#include <gtk/gtk.h>

#include "scim_private.h"
#include "scim.h"
#include "scimkeyselection.h"

using namespace scim;

#define scim_module_init aaa_frontend_setup_LTX_scim_module_init
#define scim_module_exit aaa_frontend_setup_LTX_scim_module_exit

#define scim_setup_module_create_ui       aaa_frontend_setup_LTX_scim_setup_module_create_ui
#define scim_setup_module_get_category    aaa_frontend_setup_LTX_scim_setup_module_get_category
#define scim_setup_module_get_name        aaa_frontend_setup_LTX_scim_setup_module_get_name
#define scim_setup_module_get_description aaa_frontend_setup_LTX_scim_setup_module_get_description
#define scim_setup_module_load_config     aaa_frontend_setup_LTX_scim_setup_module_load_config
#define scim_setup_module_save_config     aaa_frontend_setup_LTX_scim_setup_module_save_config
#define scim_setup_module_query_changed   aaa_frontend_setup_LTX_scim_setup_module_query_changed

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
        return String ("FrontEnd");
    }

    String scim_setup_module_get_name (void)
    {
        return String (_("Global Setup"));
    }

    String scim_setup_module_get_description (void)
    {
        return String (_("Setup the global options used by All FrontEnd modules, including X11 FrontEnd, GTK IMModule, QT IMModule etc."));
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

// Internal data structure
struct KeyboardConfigData
{
    const char *key;
    const char *label;
    const char *title;
    const char *tooltip;
    GtkWidget  *entry;
    GtkWidget  *button;
    String      data;
};

// Internal data declaration.

static bool           __config_on_the_spot       = true;

#if GTK_CHECK_VERSION(2, 12, 0)
#else
static GtkTooltips   * __widget_tooltips         = 0;
#endif

static bool           __config_shared_input_method = false;

static KeyboardLayout __config_keyboard_layout   = SCIM_KEYBOARD_Unknown;

static bool           __have_changed             = false;


static GtkWidget     * __widget_on_the_spot      = 0;

static GtkWidget     * __widget_keyboard_layout  = NULL;

static GtkWidget     * __widget_shared_input_method = NULL;

static KeyboardConfigData __config_keyboards [] =
{
    {
        // key
        SCIM_CONFIG_HOTKEYS_FRONTEND_TRIGGER,
        // label
        N_("_Trigger:"),
        // title
        N_("Select the trigger keys"),
        // tooltip
        N_("The key events to turn on/off SCIM input method. "
           "Click on the button on the right to edit it."),
        // entry
        NULL,
        // button
        NULL,
        // data
        "Control+space"
    },
    {
        // key
        SCIM_CONFIG_HOTKEYS_FRONTEND_ON,
        // label
        N_("Turn _On:"),
        // title
        N_("Select the Turn On keys"),
        // tooltip
        N_("The key events to turn on SCIM input method. "
           "Click on the button on the right to edit it."),
        // entry
        NULL,
        // button
        NULL,
        // data
        ""
    },
    {
        // key
        SCIM_CONFIG_HOTKEYS_FRONTEND_OFF,
        // label
        N_("Turn O_ff:"),
        // title
        N_("Select the Turn Off keys"),
        // tooltip
        N_("The key events to turn off SCIM input method. "
           "Click on the button on the right to edit it."),
        // entry
        NULL,
        // button
        NULL,
        // data
        ""
    },
    {
        // key
        SCIM_CONFIG_HOTKEYS_FRONTEND_NEXT_FACTORY,
        // label
        N_("_Next input method:"),
        // title
        N_("Select the next input method keys"),
        // tooltip
        N_("The key events to switch to the next input method. "
           "Click on the button on the right to edit it."),
        // entry
        NULL,
        // button
        NULL,
        // data
        "Control+Alt+Down,"
        "Control+Shift_R,"
        "Control+Shift_L"
    },
    {
        // key
        SCIM_CONFIG_HOTKEYS_FRONTEND_PREVIOUS_FACTORY,
        // label
        N_("_Previous input method:"),
        // title
        N_("Select the previous input method keys"),
        // tooltip
        N_("The key events to switch to the previous input method. "
           "Click on the button on the right to edit it."),
        // entry
        NULL,
        // button
        NULL,
        // data
        "Control+Alt+Up,"
        "Shift+Control_R,"
        "Shift+Control_L"
    },
    {
        // key
        SCIM_CONFIG_HOTKEYS_FRONTEND_SHOW_FACTORY_MENU,
        // label
        N_("Show input method _menu:"),
        // title
        N_("Select the show input method menu keys"),
        // tooltip
        N_("The key events to show the input method menu. "
           "Click on the button on the right to edit it."),
        // entry
        NULL,
        // button
        NULL,
        // data
        "Control+Alt+l,"
        "Control+Alt+m,"
        "Control+Alt+s,"
        "Control+Alt+Right,"
    },
    {
        // key
        NULL,
        // label
        NULL,
        // title
        NULL,
        // tooltip
        NULL,
        // entry
        NULL,
        // button
        NULL,
        // data
        ""
    },
};

// Declaration of internal functions.
static void
on_default_editable_changed          (GtkEditable     *editable,
                                      gpointer         user_data);

static void
on_default_key_selection_clicked     (GtkButton       *button,
                                      gpointer         user_data);

static void
on_keyboard_layout_changed           (GtkComboBox     *combobox,
                                      gpointer         user_data);

static void
on_default_toggle_button_toggled     (GtkToggleButton *togglebutton,
                                      gpointer         user_data);

static void
setup_widget_value ();

// Function implementations.
static GtkWidget *
create_setup_window ()
{
    static GtkWidget *window = 0;

    if (!window) {
        GtkWidget *table;
        GtkWidget *frame;
        GtkWidget *hbox;
        GtkWidget *vbox;
        GtkWidget *label;
        int i;

#if GTK_CHECK_VERSION(2, 12, 0)
#else
        __widget_tooltips = gtk_tooltips_new ();
#endif

        // Create the toplevel box.
#if GTK_CHECK_VERSION(3, 2, 0)
        window = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
#else
        window = gtk_vbox_new (FALSE, 0);
#endif
        gtk_widget_show (window);

        frame = gtk_frame_new (_("Options"));
        gtk_widget_show (frame);
        gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
        gtk_box_pack_start (GTK_BOX (window), frame, FALSE, FALSE, 0);

#if GTK_CHECK_VERSION(3, 2, 0)
        vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
#else
        vbox = gtk_vbox_new (FALSE, 4);
#endif
        gtk_widget_show (vbox);
        gtk_container_set_border_width (GTK_CONTAINER (vbox), 4);
        gtk_container_add (GTK_CONTAINER (frame), vbox);

        // Keyboard Layout.
#if GTK_CHECK_VERSION(3, 2, 0)
        hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
#else
        hbox = gtk_hbox_new (FALSE, 4);
#endif
        gtk_widget_show (hbox);
        gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

        label = gtk_label_new_with_mnemonic (_("_Keyboard Layout:"));
        gtk_widget_show (label);
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

#if GTK_CHECK_VERSION(2,24,0)
        __widget_keyboard_layout = gtk_combo_box_text_new ();
#else
        __widget_keyboard_layout = gtk_combo_box_new_text ();
#endif
        gtk_widget_show (__widget_keyboard_layout);

        gtk_label_set_mnemonic_widget (GTK_LABEL (label), __widget_keyboard_layout);

        for (size_t i = 0; i < SCIM_KEYBOARD_NUM_LAYOUTS; ++i) {
#if GTK_CHECK_VERSION(2,24,0)
            gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (__widget_keyboard_layout),
#else
            gtk_combo_box_append_text (GTK_COMBO_BOX (__widget_keyboard_layout),
#endif
                scim_keyboard_layout_get_display_name (static_cast<KeyboardLayout> (i)).c_str ());
        }

        g_signal_connect (G_OBJECT (__widget_keyboard_layout), "changed",
                          G_CALLBACK (on_keyboard_layout_changed),
                          NULL);

        gtk_box_pack_start (GTK_BOX (hbox), __widget_keyboard_layout, TRUE, TRUE, 0);

#if GTK_CHECK_VERSION(2, 12, 0)
        gtk_widget_set_tooltip_text (__widget_keyboard_layout,
                              _("You should choose your currently used keyboard layout here "
                                "so that input methods, who care about keyboard layout, could work correctly."));
#else
        gtk_tooltips_set_tip (__widget_tooltips, __widget_keyboard_layout,
                              _("You should choose your currently used keyboard layout here "
                                "so that input methods, who care about keyboard layout, could work correctly."), NULL);
#endif

        // On The Spot.
        __widget_on_the_spot = gtk_check_button_new_with_mnemonic (_("_Embed Preedit String into client window"));
        gtk_widget_show (__widget_on_the_spot);
        gtk_box_pack_start (GTK_BOX (vbox), __widget_on_the_spot, FALSE, FALSE, 0);

#if GTK_CHECK_VERSION(2, 12, 0)
        gtk_widget_set_tooltip_text (__widget_on_the_spot,
                              _("If this option is checked, "
                                "the preedit string will be displayed directly in the client input window, "
                                "rather than in a independent float window."));
#else
        gtk_tooltips_set_tip (__widget_tooltips, __widget_on_the_spot,
                              _("If this option is checked, "
                                "the preedit string will be displayed directly in the client input window, "
                                "rather than in a independent float window."), NULL);
#endif

        g_signal_connect ((gpointer) __widget_on_the_spot, "toggled",
                          G_CALLBACK (on_default_toggle_button_toggled),
                          &__config_on_the_spot);

        // Shared input method.
        __widget_shared_input_method = gtk_check_button_new_with_mnemonic (_("_Share the same input method among all applications"));
        gtk_widget_show (__widget_shared_input_method);
        gtk_box_pack_start (GTK_BOX (vbox), __widget_shared_input_method, FALSE, FALSE, 0);

#if GTK_CHECK_VERSION(2, 12, 0)
        gtk_widget_set_tooltip_text (__widget_shared_input_method,
                              _("If this option is checked, "
                                "then only one input method could be used by all applications at the same time."
                                "Otherwise different input method could be used by each application."));
#else
        gtk_tooltips_set_tip (__widget_tooltips, __widget_shared_input_method,
                              _("If this option is checked, "
                                "then only one input method could be used by all applications at the same time."
                                "Otherwise different input method could be used by each application."), NULL);
#endif

        g_signal_connect ((gpointer) __widget_shared_input_method, "toggled",
                          G_CALLBACK (on_default_toggle_button_toggled),
                          &__config_shared_input_method);

        frame = gtk_frame_new (_("Hotkeys"));
        gtk_widget_show (frame);
        gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
        gtk_box_pack_start (GTK_BOX (window), frame, TRUE, TRUE, 0);

#if GTK_CHECK_VERSION(3, 4, 0)
        table = gtk_grid_new();
#else
        table = gtk_table_new (3, 3, FALSE);
#endif
        gtk_widget_show (table);
        gtk_container_add (GTK_CONTAINER (frame), table);
#if GTK_CHECK_VERSION(3, 4, 0)
        gtk_grid_set_row_spacing (GTK_GRID (table), 0);
        gtk_grid_set_column_spacing (GTK_GRID (table), 8);
#else
        gtk_table_set_row_spacings (GTK_TABLE (table), 0);
        gtk_table_set_col_spacings (GTK_TABLE (table), 8);
#endif

        for (i = 0; __config_keyboards [i].key; ++ i) {
            label = gtk_label_new (NULL);
            gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _(__config_keyboards[i].label));
            gtk_widget_show (label);
#if GTK_CHECK_VERSION(3, 14, 0)
            gtk_widget_set_halign (label, GTK_ALIGN_END);
            gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
            gtk_widget_set_margin_start (label, 4);
            gtk_widget_set_margin_end (label, 4);
#else
            gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
            gtk_misc_set_padding (GTK_MISC (label), 4, 0);
#endif
#if GTK_CHECK_VERSION(3, 4, 0)
            gtk_widget_set_halign (label, GTK_ALIGN_FILL);
            gtk_widget_set_valign (label, GTK_ALIGN_FILL);
            gtk_grid_attach (GTK_GRID (table), label, 0, i, 1, 1);
#else
            gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
                              (GtkAttachOptions) (GTK_FILL),
                              (GtkAttachOptions) (GTK_FILL), 4, 2);
#endif

            __config_keyboards [i].entry = gtk_entry_new ();
            gtk_widget_show (__config_keyboards [i].entry);
#if GTK_CHECK_VERSION(3, 4, 0)
            gtk_widget_set_halign (__config_keyboards [i].entry, GTK_ALIGN_FILL);
            gtk_widget_set_valign (__config_keyboards [i].entry, GTK_ALIGN_FILL);
            gtk_grid_attach (GTK_GRID (table), __config_keyboards [i].entry,
                              1, i, 1, 1);
#else
            gtk_table_attach (GTK_TABLE (table), __config_keyboards [i].entry, 1, 2, i, i+1,
                              (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
                              (GtkAttachOptions) (GTK_FILL), 4, 2);
#endif
            gtk_editable_set_editable (GTK_EDITABLE (__config_keyboards[i].entry), FALSE);

            __config_keyboards[i].button = gtk_button_new_with_label ("...");
            gtk_widget_show (__config_keyboards[i].button);
#if GTK_CHECK_VERSION(3, 4, 0)
            gtk_widget_set_halign (__config_keyboards [i].button, GTK_ALIGN_FILL);
            gtk_widget_set_valign (__config_keyboards [i].button, GTK_ALIGN_FILL);
            gtk_grid_attach (GTK_GRID (table), __config_keyboards[i].button, 2, i, 1, 1);
#else
            gtk_table_attach (GTK_TABLE (table), __config_keyboards[i].button, 2, 3, i, i+1,
                              (GtkAttachOptions) (GTK_FILL),
                              (GtkAttachOptions) (GTK_FILL), 4, 2);
#endif
            gtk_label_set_mnemonic_widget (GTK_LABEL (label), __config_keyboards[i].button);
        }

        for (i = 0; __config_keyboards [i].key; ++ i) {
            g_signal_connect ((gpointer) __config_keyboards [i].button, "clicked",
                              G_CALLBACK (on_default_key_selection_clicked),
                              &(__config_keyboards [i]));
            g_signal_connect ((gpointer) __config_keyboards [i].entry, "changed",
                              G_CALLBACK (on_default_editable_changed),
                              &(__config_keyboards [i].data));
        }

        for (i = 0; __config_keyboards [i].key; ++ i) {
#if GTK_CHECK_VERSION(2, 12, 0)
            gtk_widget_set_tooltip_text (__config_keyboards [i].entry,
                                  _(__config_keyboards [i].tooltip));
#else
            gtk_tooltips_set_tip (__widget_tooltips, __config_keyboards [i].entry,
                                  _(__config_keyboards [i].tooltip), NULL);
#endif
        }

        setup_widget_value ();
    }

    return window;
}

static void
setup_widget_value ()
{
    for (int i = 0; __config_keyboards [i].key; ++ i) {
        if (__config_keyboards [i].entry) {
            gtk_entry_set_text (
                GTK_ENTRY (__config_keyboards [i].entry),
                __config_keyboards [i].data.c_str ());
        }
    }

    if (__widget_on_the_spot) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_on_the_spot),
            __config_on_the_spot);
    }

    if (__widget_shared_input_method) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_shared_input_method),
            __config_shared_input_method);
    }

    gtk_combo_box_set_active (GTK_COMBO_BOX (__widget_keyboard_layout), (gint) __config_keyboard_layout);
}

static void
load_config (const ConfigPointer &config)
{
    if (!config.null ()) {
        for (int i = 0; __config_keyboards [i].key; ++ i) {
            __config_keyboards [i].data =
                config->read (String (__config_keyboards [i].key),
                              __config_keyboards [i].data);
        }

        __config_on_the_spot =
            config->read (String (SCIM_CONFIG_FRONTEND_ON_THE_SPOT),
                          __config_on_the_spot);

        __config_shared_input_method =
            config->read (String (SCIM_CONFIG_FRONTEND_SHARED_INPUT_METHOD),
                          __config_shared_input_method);

        __config_keyboard_layout = scim_get_default_keyboard_layout ();

        setup_widget_value ();

        __have_changed = false;
    }
}

static void
save_config (const ConfigPointer &config)
{
    if (!config.null ()) {
        for (int i = 0; __config_keyboards [i].key; ++ i) {
            config->write (String (__config_keyboards [i].key),
                          __config_keyboards [i].data);
        }

        gint act = gtk_combo_box_get_active (GTK_COMBO_BOX (__widget_keyboard_layout));

        if (act >= 0 && act < SCIM_KEYBOARD_NUM_LAYOUTS)
            __config_keyboard_layout = static_cast<KeyboardLayout> (act);
        else
            __config_keyboard_layout = SCIM_KEYBOARD_Unknown;

        if (__config_keyboard_layout != scim_get_default_keyboard_layout ())
            scim_set_default_keyboard_layout (__config_keyboard_layout);

        config->write (String (SCIM_CONFIG_FRONTEND_ON_THE_SPOT),
                       __config_on_the_spot);

        config->write (String (SCIM_CONFIG_FRONTEND_SHARED_INPUT_METHOD),
                       __config_shared_input_method);

        __have_changed = false;
    }
}

static bool
query_changed ()
{
    return __have_changed;
}

static void
on_default_editable_changed (GtkEditable *editable,
                             gpointer     user_data)
{
    String *str = static_cast <String *> (user_data);

    if (str) {
        *str = String (gtk_entry_get_text (GTK_ENTRY (editable)));
        __have_changed = true;
    }
}

static void
on_default_key_selection_clicked (GtkButton *button,
                                  gpointer   user_data)
{
    KeyboardConfigData *data = static_cast <KeyboardConfigData *> (user_data);

    if (data) {
        GtkWidget *dialog = scim_key_selection_dialog_new (_(data->title));
        gint result;

        scim_key_selection_dialog_set_keys (
            SCIM_KEY_SELECTION_DIALOG (dialog),
            data->data.c_str ());

        result = gtk_dialog_run (GTK_DIALOG (dialog));

        if (result == GTK_RESPONSE_OK) {
            const gchar *keys = scim_key_selection_dialog_get_keys (
                            SCIM_KEY_SELECTION_DIALOG (dialog));

            if (!keys) keys = "";

            if (String (keys) != data->data)
                gtk_entry_set_text (GTK_ENTRY (data->entry), keys);
        }

        gtk_widget_destroy (dialog);
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
on_keyboard_layout_changed (GtkComboBox *combobox,
                            gpointer     user_data)
{
    __have_changed = true;
}


/*
vi:ts=4:nowrap:expandtab
*/

