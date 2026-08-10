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
        // data -- keep in step with __scim_frontend_hotkey_defaults in
        // src/scim_hotkey.cpp, which this shadows as the config fallback.
        "Control+Shift+Shift_L+KeyRelease,"
        "Control+Shift+Shift_R+KeyRelease"
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
        "Control+Shift+Control_L+KeyRelease,"
        "Control+Shift+Control_R+KeyRelease"
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
        // data -- unbound by default: the panel offers this menu from its
        // tray icon, and in tray mode the hotkey draws nothing anyway.
        ""
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
on_default_check_button_toggled      (GtkCheckButton  *checkbutton,
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

        // Create the toplevel box.
        window = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

        frame = gtk_frame_new (_("Options"));
        gtk_widget_set_margin_start (frame, 4);
        gtk_widget_set_margin_end (frame, 4);
        gtk_widget_set_margin_top (frame, 4);
        gtk_widget_set_margin_bottom (frame, 4);
        gtk_box_append (GTK_BOX (window), frame);

        vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
        gtk_widget_set_margin_start (vbox, 4);
        gtk_widget_set_margin_end (vbox, 4);
        gtk_widget_set_margin_top (vbox, 4);
        gtk_widget_set_margin_bottom (vbox, 4);
        gtk_frame_set_child (GTK_FRAME (frame), vbox);

        // Keyboard Layout.
        hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
        gtk_box_append (GTK_BOX (vbox), hbox);

        label = gtk_label_new_with_mnemonic (_("_Keyboard Layout:"));
        gtk_box_append (GTK_BOX (hbox), label);

        __widget_keyboard_layout = gtk_combo_box_text_new ();

        gtk_label_set_mnemonic_widget (GTK_LABEL (label), __widget_keyboard_layout);

        for (size_t i = 0; i < SCIM_KEYBOARD_NUM_LAYOUTS; ++i) {
            gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (__widget_keyboard_layout),
                scim_keyboard_layout_get_display_name (static_cast<KeyboardLayout> (i)).c_str ());
        }

        g_signal_connect (G_OBJECT (__widget_keyboard_layout), "changed",
                          G_CALLBACK (on_keyboard_layout_changed),
                          NULL);

        gtk_widget_set_hexpand (__widget_keyboard_layout, TRUE);
        gtk_box_append (GTK_BOX (hbox), __widget_keyboard_layout);

        gtk_widget_set_tooltip_text (__widget_keyboard_layout,
                              _("You should choose your currently used keyboard layout here "
                                "so that input methods, who care about keyboard layout, could work correctly."));

        // On The Spot.
        __widget_on_the_spot = gtk_check_button_new_with_mnemonic (_("_Embed Preedit String into client window"));
        gtk_box_append (GTK_BOX (vbox), __widget_on_the_spot);

        gtk_widget_set_tooltip_text (__widget_on_the_spot,
                              _("If this option is checked, "
                                "the preedit string will be displayed directly in the client input window, "
                                "rather than in a independent float window."));

        g_signal_connect ((gpointer) __widget_on_the_spot, "toggled",
                          G_CALLBACK (on_default_check_button_toggled),
                          &__config_on_the_spot);

        // Shared input method.
        __widget_shared_input_method = gtk_check_button_new_with_mnemonic (_("_Share the same input method among all applications"));
        gtk_box_append (GTK_BOX (vbox), __widget_shared_input_method);

        gtk_widget_set_tooltip_text (__widget_shared_input_method,
                              _("If this option is checked, "
                                "then only one input method could be used by all applications at the same time."
                                "Otherwise different input method could be used by each application."));

        g_signal_connect ((gpointer) __widget_shared_input_method, "toggled",
                          G_CALLBACK (on_default_check_button_toggled),
                          &__config_shared_input_method);

        frame = gtk_frame_new (_("Hotkeys"));
        gtk_widget_set_margin_start (frame, 4);
        gtk_widget_set_margin_end (frame, 4);
        gtk_widget_set_margin_top (frame, 4);
        gtk_widget_set_margin_bottom (frame, 4);
        gtk_widget_set_vexpand (frame, TRUE);
        gtk_box_append (GTK_BOX (window), frame);

        table = gtk_grid_new();
        gtk_frame_set_child (GTK_FRAME (frame), table);
        gtk_grid_set_row_spacing (GTK_GRID (table), 0);
        gtk_grid_set_column_spacing (GTK_GRID (table), 8);

        for (i = 0; __config_keyboards [i].key; ++ i) {
            label = gtk_label_new (NULL);
            gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _(__config_keyboards[i].label));
            gtk_widget_set_halign (label, GTK_ALIGN_END);
            gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
            gtk_widget_set_margin_start (label, 4);
            gtk_widget_set_margin_end (label, 4);
            gtk_grid_attach (GTK_GRID (table), label, 0, i, 1, 1);

            __config_keyboards [i].entry = gtk_entry_new ();
            gtk_widget_set_hexpand (__config_keyboards [i].entry, TRUE);
            gtk_grid_attach (GTK_GRID (table), __config_keyboards [i].entry,
                              1, i, 1, 1);
            gtk_editable_set_editable (GTK_EDITABLE (__config_keyboards[i].entry), FALSE);

            __config_keyboards[i].button = gtk_button_new_with_label ("...");
            gtk_grid_attach (GTK_GRID (table), __config_keyboards[i].button, 2, i, 1, 1);
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
            gtk_widget_set_tooltip_text (__config_keyboards [i].entry,
                                  _(__config_keyboards [i].tooltip));
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
            gtk_editable_set_text (
                GTK_EDITABLE (__config_keyboards [i].entry),
                __config_keyboards [i].data.c_str ());
        }
    }

    if (__widget_on_the_spot) {
        gtk_check_button_set_active (
            GTK_CHECK_BUTTON (__widget_on_the_spot),
            __config_on_the_spot);
    }

    if (__widget_shared_input_method) {
        gtk_check_button_set_active (
            GTK_CHECK_BUTTON (__widget_shared_input_method),
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
        *str = String (gtk_editable_get_text (editable));
        __have_changed = true;
    }
}

static void
key_selection_response_cb (GtkDialog *dialog,
                           gint       response,
                           gpointer   user_data)
{
    KeyboardConfigData *data = static_cast <KeyboardConfigData *> (user_data);

    if (response == GTK_RESPONSE_OK && data) {
        const gchar *keys = scim_key_selection_dialog_get_keys (
                        SCIM_KEY_SELECTION_DIALOG (dialog));

        if (!keys) keys = "";

        if (String (keys) != data->data)
            gtk_editable_set_text (GTK_EDITABLE (data->entry), keys);
    }

    gtk_window_destroy (GTK_WINDOW (dialog));
}

static void
on_default_key_selection_clicked (GtkButton *button,
                                  gpointer   user_data)
{
    KeyboardConfigData *data = static_cast <KeyboardConfigData *> (user_data);

    if (data) {
        GtkWidget *dialog = scim_key_selection_dialog_new (_(data->title));
        GtkRoot   *root = gtk_widget_get_root (GTK_WIDGET (button));

        scim_key_selection_dialog_set_keys (
            SCIM_KEY_SELECTION_DIALOG (dialog),
            data->data.c_str ());

        if (root && GTK_IS_WINDOW (root))
            gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (root));
        gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

        g_signal_connect (dialog, "response", G_CALLBACK (key_selection_response_cb), data);

        gtk_window_present (GTK_WINDOW (dialog));
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
on_keyboard_layout_changed (GtkComboBox */* combobox */,
                            gpointer     /* user_data */)
{
    __have_changed = true;
}


/*
vi:ts=4:nowrap:expandtab
*/
