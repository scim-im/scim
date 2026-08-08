/** @file scim_setup_gtk3_host.cpp
 *  @brief Frozen GTK3 legacy setup helper.
 *
 *  A standalone GTK3 program that hosts third-party GTK3 SetupUI plugins which
 *  cannot be embedded in the modern GTK4 setup host. It enumerates the SetupUI
 *  modules, keeps only the ones linked against GTK3 (decided from DT_NEEDED),
 *  loads them in-process, and shows them in a notebook window.
 *
 *  This source is intentionally frozen at GTK3: it is bugfix-only and gains no
 *  new features. New work happens in the GTK4 host.
 */

/*
 * Smart Common Input Method
 *
 * Copyright (c) 2026 SCIM developers
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
 */

#include <cstdlib>
#include <vector>

#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_MODULE
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_GLOBAL_CONFIG
#define Uses_SCIM_MODULE

#include <gtk/gtk.h>

#include "scim_private.h"
#include "scim.h"
#include "scim_setup_module.h"
#include "scim_setup_classify.h"

using namespace scim;

static std::vector<SetupModule *> _modules;
static ConfigPointer              _config;
static bool                       _changes_applied = false;

static GtkWidget                 *_window   = 0;

static void
save_all_changed ()
{
    if (_config.null ())
        return;

    bool any = false;
    for (size_t i = 0; i < _modules.size (); ++i) {
        if (_modules [i]->query_changed ()) {
            _modules [i]->save_config (_config);
            _changes_applied = true;
            any = true;
        }
    }

    if (any)
        _config->flush ();
}

// Kept for the day a setting turns up that genuinely cannot be applied live.
// Everything the setup pages write today -- engine enable/disable, filters,
// hotkeys, frontend and panel options -- is picked up by the running daemon,
// frontends and panel through the config reload, so nothing gates this and it
// has no caller. To bring it back: uncomment, decide which key is not
// reloadable, snapshot that key around save_all_changed () and call this from
// ok_button_clicked_cb () when it changed.
//
// static void
// show_restart_hint ()
// {
//     GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (_window),
//                             GTK_DIALOG_MODAL,
//                             GTK_MESSAGE_INFO,
//                             GTK_BUTTONS_OK,
//                             _("Not all configuration can be reloaded on the fly. "
//                               "Don't forget to restart SCIM in order to let all of "
//                               "the new configuration take effect."));
//
//     gtk_dialog_run (GTK_DIALOG (dialog));
//     gtk_widget_destroy (dialog);
// }

static void
apply_button_clicked_cb (GtkButton *, gpointer)
{
    save_all_changed ();
}

static void
ok_button_clicked_cb (GtkButton *, gpointer)
{
    save_all_changed ();
    gtk_main_quit ();
}

static void
cancel_button_clicked_cb (GtkButton *, gpointer)
{
    gtk_main_quit ();
}

static gboolean
window_delete_cb (GtkWidget *, GdkEvent *, gpointer)
{
    gtk_main_quit ();
    return TRUE;
}

int
main (int argc, char *argv [])
{
    String config_name;
    String display;

    const char *p = getenv ("DISPLAY");
    if (p) display = String (p);

    config_name = scim_global_config_read (SCIM_GLOBAL_CONFIG_DEFAULT_CONFIG_MODULE,
                                            String ("simple"));

    for (int i = 1; i < argc; ++i) {
        if (String ("-c") == argv [i] || String ("--config") == argv [i]) {
            if (++i >= argc) { std::cerr << "No argument for " << argv [i-1] << "\n"; return -1; }
            config_name = argv [i];
        } else if (String ("--display") == argv [i]) {
            if (++i >= argc) { std::cerr << "No argument for " << argv [i-1] << "\n"; return -1; }
            display = argv [i];
        } else if (String ("-h") == argv [i] || String ("--help") == argv [i]) {
            std::cout << "Usage: " << argv [0] << " [--config name] [--display name]\n";
            return 0;
        }
    }

    if (display.length ())
        setenv ("DISPLAY", display.c_str (), 1);

    // Connect to the same config store the GTK4 host uses.
    ConfigModule *config_module = new ConfigModule (config_name);
    if (config_module && config_module->valid ())
        _config = config_module->create_config ();
    if (_config.null ())
        _config = new DummyConfig ();

    gtk_init (&argc, &argv);

    // Enumerate SetupUI plugins and keep only the GTK3 ones.
    std::vector<String> setup_list;
    scim_get_setup_module_list (setup_list);

    GtkWidget *notebook = gtk_notebook_new ();

    for (size_t i = 0; i < setup_list.size (); ++i) {
        if (scim_setup_classify_module (setup_list [i]) != SETUP_TOOLKIT_GTK3)
            continue;

        SetupModule *module = new SetupModule (setup_list [i]);
        if (!module || !module->valid ()) {
            delete module;
            continue;
        }

        GtkWidget *ui       = module->create_ui ();
        String     name     = module->get_name ();
        if (!ui || !name.length ()) {
            delete module;
            continue;
        }

        if (!_config.null ())
            module->load_config (_config);

        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), ui,
                                  gtk_label_new (name.c_str ()));
        gtk_widget_set_visible (ui, TRUE);
        _modules.push_back (module);
    }

    // Build the window.
    _window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (_window), _("SCIM Legacy Setup (GTK3)"));
    gtk_window_set_default_size (GTK_WINDOW (_window), 640, 480);

    GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add (GTK_CONTAINER (_window), vbox);

    if (_modules.empty ()) {
        GtkWidget *empty = gtk_label_new (
            _("No GTK3 legacy setup modules were found."));
        gtk_box_pack_start (GTK_BOX (vbox), empty, TRUE, TRUE, 0);
        gtk_widget_destroy (notebook);
    } else {
        gtk_box_pack_start (GTK_BOX (vbox), notebook, TRUE, TRUE, 0);
    }

    GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_set_halign (hbox, GTK_ALIGN_END);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 8);
    gtk_box_pack_end (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

    GtkWidget *apply_button  = gtk_button_new_with_mnemonic (_("_Apply"));
    GtkWidget *cancel_button = gtk_button_new_with_mnemonic (_("_Cancel"));
    GtkWidget *ok_button     = gtk_button_new_with_mnemonic (_("_OK"));
    gtk_box_pack_start (GTK_BOX (hbox), apply_button, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), cancel_button, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), ok_button, FALSE, FALSE, 0);

    g_signal_connect (apply_button, "clicked", G_CALLBACK (apply_button_clicked_cb), 0);
    g_signal_connect (cancel_button, "clicked", G_CALLBACK (cancel_button_clicked_cb), 0);
    g_signal_connect (ok_button, "clicked", G_CALLBACK (ok_button_clicked_cb), 0);
    g_signal_connect (_window, "delete-event", G_CALLBACK (window_delete_cb), 0);

    gtk_widget_show_all (_window);

    gtk_main ();

    // Best effort: persist and let the user know a restart is needed. There is
    // no live reload signal here -- the legacy helper is a standalone process.
    gtk_widget_destroy (_window);

    for (size_t i = 0; i < _modules.size (); ++i)
        delete _modules [i];
    _modules.clear ();

    _config.reset ();
    delete config_module;

    return 0;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
