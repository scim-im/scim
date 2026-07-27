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
 */

#include <cstring>
#include <cstdio>

#define Uses_SCIM_COMPOSE_KEY
#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_MODULE
#define Uses_SCIM_IMENGINE_MODULE
#define Uses_SCIM_HELPER

#include "scim_private.h"
#include "scim.h"
#include "scim_setup_module.h"
#include "scim_setup_ui.h"

#define SCIM_SETUP_GTK3_HELPER   (SCIM_LIBEXECDIR "/scim-setup-gtk3-host")

const gchar * scim_setup_module_categories [] =
{
    N_("SCIM"),
    N_("FrontEnd"),
    N_("IMEngine"),
    N_("Panel"),
    N_("Extra"),
    NULL
};

enum
{
    MODULE_LIST_LABEL = 0,
    MODULE_LIST_CATEGORY,
    MODULE_LIST_MODULE,
    MODULE_LIST_WIDGET,
    MODULE_LIST_NUM_COLUMNS
};

SetupUI::SetupUI (const ConfigPointer &config, const String &display, const HelperInfo &helper_info)
    : m_main_window (0),
      m_work_area (0),
      m_apply_button (0),
      m_restore_button (0),
      m_status_label (0),
      m_module_list_view (0),
      m_module_list_selection (0),
      m_module_list_model (0),
      m_current_widget (0),
      m_current_module (0),
      m_config (config),
      m_query_changed_timeout (0),
      m_changes_applied (false),
      m_loop (0)
{
    if (display.length ())
        setenv ("DISPLAY", display.c_str (), 1);

    gtk_init ();

    m_loop = g_main_loop_new (NULL, FALSE);

    create_main_ui ();
    create_module_list_model ();

    m_query_changed_timeout = g_timeout_add (200, query_changed_timeout_cb, this);

    m_helper_agent.open_connection (helper_info);
}

SetupUI::~SetupUI ()
{
    if (m_query_changed_timeout)
        g_source_remove (m_query_changed_timeout);
    if (m_main_window)
        gtk_window_destroy (GTK_WINDOW (m_main_window));
    if (m_loop)
        g_main_loop_unref (m_loop);
    m_helper_agent.close_connection ();
}

gboolean
SetupUI::find_category (const char *category, GtkTreeIter *parent_out)
{
    GtkTreeIter parent;

    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (m_module_list_model), &parent)) {
        do {
            gchar *cat = 0;
            gtk_tree_model_get (GTK_TREE_MODEL (m_module_list_model), &parent,
                                MODULE_LIST_CATEGORY, &cat, -1);
            if (cat && !strcmp (cat, category)) {
                g_free (cat);
                *parent_out = parent;
                return TRUE;
            }
            g_free (cat);
        } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (m_module_list_model), &parent));
    }

    return FALSE;
}

GtkTreeIter
SetupUI::create_category (const char *category, const char *label)
{
    GtkWidget  *cover = create_setup_cover (label ? label : category);
    GtkTreeIter parent;

    gtk_box_append (GTK_BOX (m_work_area), cover);
    gtk_widget_set_hexpand (cover, TRUE);
    gtk_widget_set_vexpand (cover, TRUE);

    gtk_tree_store_append (m_module_list_model, &parent, NULL);
    gtk_tree_store_set (
        m_module_list_model, &parent,
        MODULE_LIST_LABEL,    _(category),
        MODULE_LIST_CATEGORY, category,
        MODULE_LIST_MODULE,   NULL,
        MODULE_LIST_WIDGET,   cover,
        -1);

    return parent;
}

void
SetupUI::append_module_row (GtkTreeIter *parent, const char *label,
                            SetupModule *module, GtkWidget *widget)
{
    GtkTreeIter iter;

    gtk_tree_store_append (m_module_list_model, &iter, parent);
    gtk_tree_store_set (
        m_module_list_model, &iter,
        MODULE_LIST_LABEL,    label,
        MODULE_LIST_CATEGORY, NULL,
        MODULE_LIST_MODULE,   module,
        MODULE_LIST_WIDGET,   widget,
        -1);

    gtk_tree_view_expand_all (GTK_TREE_VIEW (m_module_list_view));
}

bool
SetupUI::add_module (SetupModule *module)
{
    if (!module || !module->valid ()) return false;

    GtkWidget *module_widget   = module->create_ui ();
    String     module_label    = module->get_name ();
    String     module_category = module->get_category ();

    if (!module_widget || !module_label.length () || !module_category.length ())
        return false;

    if (!m_config.null ())
        module->load_config (m_config);

    gtk_box_append (GTK_BOX (m_work_area), module_widget);
    gtk_widget_set_hexpand (module_widget, TRUE);
    gtk_widget_set_vexpand (module_widget, TRUE);
    gtk_widget_set_visible (module_widget, FALSE);

    GtkTreeIter parent;
    if (!find_category (module_category.c_str (), &parent))
        parent = create_category (module_category.c_str (), module_category.c_str ());

    append_module_row (&parent, module_label.c_str (), module, module_widget);

    return true;
}

void
SetupUI::add_legacy_gtk3_entry (const std::vector<String> &names,
                                const String &config_name,
                                const String &display)
{
    if (names.empty ())
        return;

    m_legacy_config_name = config_name;
    m_legacy_display     = display;

    // Build the launcher page.
    GtkWidget *page = gtk_box_new (GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_top (page, 16);
    gtk_widget_set_margin_bottom (page, 16);
    gtk_widget_set_margin_start (page, 16);
    gtk_widget_set_margin_end (page, 16);
    gtk_widget_set_hexpand (page, TRUE);
    gtk_widget_set_vexpand (page, TRUE);

    String text;
    text  = "<span size=\"large\">";
    text += _("Legacy setup panels (GTK3)");
    text += "</span>\n\n";
    text += _("The following setup modules are built for GTK3 and cannot be "
              "shown inside this GTK4 window. Open them in a separate legacy "
              "settings window instead:");
    text += "\n";
    for (size_t i = 0; i < names.size (); ++i) {
        text += "\n    • ";
        text += names [i];
    }

    GtkWidget *label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), text.c_str ());
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_label_set_wrap (GTK_LABEL (label), TRUE);
    gtk_label_set_xalign (GTK_LABEL (label), 0.0);
    gtk_box_append (GTK_BOX (page), label);

    GtkWidget *button = gtk_button_new_with_mnemonic (_("_Open legacy settings..."));
    gtk_widget_set_halign (button, GTK_ALIGN_START);
    g_signal_connect (button, "clicked",
                      G_CALLBACK (SetupUI::legacy_launch_clicked_callback), this);
    gtk_box_append (GTK_BOX (page), button);

    gtk_box_append (GTK_BOX (m_work_area), page);
    gtk_widget_set_visible (page, FALSE);

    GtkTreeIter parent;
    if (!find_category ("__legacy__", &parent))
        parent = create_category ("__legacy__", _("Legacy"));

    append_module_row (&parent, _("Legacy settings (GTK3)"), NULL, page);
}

void
SetupUI::add_unsupported_notice (const std::vector<String> &names)
{
    if (names.empty ())
        return;

    GtkWidget *page = gtk_box_new (GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_top (page, 16);
    gtk_widget_set_margin_bottom (page, 16);
    gtk_widget_set_margin_start (page, 16);
    gtk_widget_set_margin_end (page, 16);
    gtk_widget_set_hexpand (page, TRUE);
    gtk_widget_set_vexpand (page, TRUE);

    String text;
    text  = "<span size=\"large\">";
    text += _("Unsupported setup panels");
    text += "</span>\n\n";
    text += _("The following setup modules are built for a GUI toolkit that is "
              "no longer supported and cannot be shown. Rebuild them against "
              "GTK4 (or GTK3 for the legacy window) to use their settings:");
    text += "\n";
    for (size_t i = 0; i < names.size (); ++i) {
        text += "\n    • ";
        text += names [i];
    }

    GtkWidget *label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), text.c_str ());
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_label_set_wrap (GTK_LABEL (label), TRUE);
    gtk_label_set_xalign (GTK_LABEL (label), 0.0);
    gtk_box_append (GTK_BOX (page), label);

    gtk_box_append (GTK_BOX (m_work_area), page);
    gtk_widget_set_visible (page, FALSE);

    GtkTreeIter parent;
    if (!find_category ("__unsupported__", &parent))
        parent = create_category ("__unsupported__", _("Unsupported"));

    append_module_row (&parent, _("Unsupported modules"), NULL, page);
}

void
SetupUI::run ()
{
    SCIM_DEBUG_MAIN(1) << "SetupUI::run ()\n";

    if (m_main_window) {
        gtk_widget_set_visible (m_main_window, TRUE);
        g_main_loop_run (m_loop);
    }

    if (m_changes_applied) {
        // Flush the global config before sending reload config event.
        scim_global_config_flush ();
        m_helper_agent.reload_config ();
    }

    SCIM_DEBUG_MAIN(1) << "exit SetupUI::run ()\n";
}

void
SetupUI::create_main_ui ()
{
    // Create main window.
    m_main_window = gtk_window_new ();
    gtk_window_set_title (GTK_WINDOW (m_main_window), _("SCIM Input Method Setup"));
    gtk_window_set_default_size (GTK_WINDOW (m_main_window), 640, 480);
    gtk_window_set_resizable (GTK_WINDOW (m_main_window), TRUE);

    GtkWidget *vbox1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child (GTK_WINDOW (m_main_window), vbox1);

    // Create paned window.
    GtkWidget *hpaned1 = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_margin_top (hpaned1, 4);
    gtk_widget_set_margin_bottom (hpaned1, 4);
    gtk_widget_set_margin_start (hpaned1, 4);
    gtk_widget_set_margin_end (hpaned1, 4);
    gtk_widget_set_vexpand (hpaned1, TRUE);
    gtk_box_append (GTK_BOX (vbox1), hpaned1);

    // Create status line.
    m_status_label = gtk_label_new ("");
    gtk_label_set_xalign (GTK_LABEL (m_status_label), 0.0);
    gtk_label_set_ellipsize (GTK_LABEL (m_status_label), PANGO_ELLIPSIZE_END);
    gtk_widget_set_margin_start (m_status_label, 6);
    gtk_widget_set_margin_end (m_status_label, 6);
    gtk_widget_set_margin_bottom (m_status_label, 4);
    gtk_box_append (GTK_BOX (vbox1), m_status_label);

    // Create scrollwindow for module list.
    GtkWidget *scrolledwindow1 = gtk_scrolled_window_new ();
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1),
                                    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_paned_set_start_child (GTK_PANED (hpaned1), scrolledwindow1);
    gtk_paned_set_resize_start_child (GTK_PANED (hpaned1), FALSE);
    gtk_paned_set_shrink_start_child (GTK_PANED (hpaned1), FALSE);

    // Create module list view.
    m_module_list_view = gtk_tree_view_new ();
    gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolledwindow1), m_module_list_view);
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (m_module_list_view), FALSE);
    gtk_tree_view_set_enable_search (GTK_TREE_VIEW (m_module_list_view), FALSE);

    // Get module list selection.
    m_module_list_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (m_module_list_view));
    gtk_tree_selection_set_mode (m_module_list_selection, GTK_SELECTION_BROWSE);

    // Create module list column.
    GtkCellRenderer *module_list_cell = gtk_cell_renderer_text_new ();
    GtkTreeViewColumn *module_list_column = gtk_tree_view_column_new_with_attributes (
                            NULL, module_list_cell, "text", MODULE_LIST_LABEL, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (m_module_list_view), module_list_column);

    // Create vbox for work area and button area.
    GtkWidget *vbox2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_paned_set_end_child (GTK_PANED (hpaned1), vbox2);
    gtk_paned_set_resize_end_child (GTK_PANED (hpaned1), TRUE);
    gtk_paned_set_shrink_end_child (GTK_PANED (hpaned1), FALSE);

    // Create frame for work area.
    GtkWidget *frame1 = gtk_frame_new (NULL);
    gtk_widget_set_hexpand (frame1, TRUE);
    gtk_widget_set_vexpand (frame1, TRUE);
    gtk_box_append (GTK_BOX (vbox2), frame1);

    m_work_area = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_frame_set_child (GTK_FRAME (frame1), m_work_area);

    // Create hbox for button area.
    GtkWidget *hbox1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_set_halign (hbox1, GTK_ALIGN_END);
    gtk_widget_set_margin_top (hbox1, 8);
    gtk_widget_set_margin_bottom (hbox1, 8);
    gtk_widget_set_margin_start (hbox1, 8);
    gtk_widget_set_margin_end (hbox1, 8);
    gtk_box_append (GTK_BOX (vbox2), hbox1);

    m_restore_button = gtk_button_new_with_mnemonic (_("_Revert"));
    gtk_widget_set_sensitive (m_restore_button, FALSE);
    gtk_box_append (GTK_BOX (hbox1), m_restore_button);

    m_apply_button = gtk_button_new_with_mnemonic (_("_Apply"));
    gtk_widget_set_sensitive (m_apply_button, FALSE);
    gtk_box_append (GTK_BOX (hbox1), m_apply_button);

    GtkWidget *vseparator1 = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
    gtk_box_append (GTK_BOX (hbox1), vseparator1);

    GtkWidget *exit_button = gtk_button_new_with_mnemonic (_("_Cancel"));
    gtk_box_append (GTK_BOX (hbox1), exit_button);

    GtkWidget *ok_button = gtk_button_new_with_mnemonic (_("_OK"));
    gtk_box_append (GTK_BOX (hbox1), ok_button);

    g_signal_connect (ok_button, "clicked",
                      G_CALLBACK (SetupUI::ok_button_clicked_callback), this);
    g_signal_connect (exit_button, "clicked",
                      G_CALLBACK (SetupUI::exit_button_clicked_callback), this);
    g_signal_connect (m_apply_button, "clicked",
                      G_CALLBACK (SetupUI::apply_button_clicked_callback), this);
    g_signal_connect (m_restore_button, "clicked",
                      G_CALLBACK (SetupUI::restore_button_clicked_callback), this);
    g_signal_connect (m_main_window, "close-request",
                      G_CALLBACK (SetupUI::main_window_close_request_callback), this);
    g_signal_connect (m_module_list_selection, "changed",
                      G_CALLBACK (SetupUI::module_list_selection_changed_callback), this);

    gtk_window_set_default_widget (GTK_WINDOW (m_main_window), ok_button);
}

GtkWidget *
SetupUI::create_splash_view ()
{
    GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *view = gtk_label_new (NULL);
    gtk_label_set_justify (GTK_LABEL (view), GTK_JUSTIFY_CENTER);
    gtk_label_set_markup (GTK_LABEL (view), _(
                " <span size=\"20000\">Smart Common Input Method platform</span> \n\n"
                "<span size=\"16000\" style=\"italic\">GUI Setup Utility</span>\n\n\n\n"
                "<span size=\"12000\">Copyright 2002-2004, James Su &lt;suzhe@tsinghua.org.cn&gt;</span>"));
    gtk_widget_set_hexpand (view, TRUE);
    gtk_widget_set_vexpand (view, TRUE);
    gtk_box_append (GTK_BOX (vbox), view);

    gtk_widget_set_size_request (vbox, 320, 240);
    gtk_widget_set_visible (vbox, FALSE);

    return vbox;
}

GtkWidget *
SetupUI::create_setup_cover (const char *category)
{
    char buf [256];

    snprintf (buf, sizeof (buf) - 1,
              _("<span size=\"x-large\">The Setup for %s modules.</span>"),
              _(category));

    GtkWidget *cover = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (cover), buf);
    gtk_label_set_justify (GTK_LABEL (cover), GTK_JUSTIFY_CENTER);

    gtk_widget_set_size_request (cover, 320, 240);
    gtk_widget_set_visible (cover, FALSE);

    return cover;
}

void
SetupUI::create_module_list_model ()
{
    GtkTreeIter iter;

    GtkWidget *widget = create_splash_view ();
    gtk_box_append (GTK_BOX (m_work_area), widget);
    gtk_widget_set_hexpand (widget, TRUE);
    gtk_widget_set_vexpand (widget, TRUE);

    m_module_list_model = gtk_tree_store_new (
                            MODULE_LIST_NUM_COLUMNS,
                            G_TYPE_STRING,
                            G_TYPE_STRING,
                            G_TYPE_POINTER,
                            GTK_TYPE_WIDGET);

    gtk_tree_store_append (m_module_list_model, &iter, NULL);
    gtk_tree_store_set (m_module_list_model, &iter,
                        MODULE_LIST_LABEL,    _(scim_setup_module_categories [0]),
                        MODULE_LIST_CATEGORY, scim_setup_module_categories [0],
                        MODULE_LIST_MODULE,   NULL,
                        MODULE_LIST_WIDGET,   widget,
                        -1);

    gtk_tree_view_set_model (GTK_TREE_VIEW (m_module_list_view),
                             GTK_TREE_MODEL (m_module_list_model));
}

gboolean
SetupUI::module_list_hide_widget_iter_func (GtkTreeModel *model,
                                            GtkTreePath *path,
                                            GtkTreeIter *iter,
                                            gpointer data)
{
    GtkWidget *widget = 0;
    gtk_tree_model_get (model, iter, MODULE_LIST_WIDGET, &widget, -1);

    if (widget) {
        gtk_widget_set_visible (widget, FALSE);
        g_object_unref (widget);
    }

    return FALSE;
}

void
SetupUI::module_list_selection_changed_callback (GtkTreeSelection *selection, gpointer user_data)
{
    GtkTreeModel *model;
    GtkTreeIter   iter;
    GtkWidget    *widget = 0;
    SetupModule  *module = 0;
    gchar        *label = 0;
    gchar        *category = 0;

    SetupUI *ui = (SetupUI *) user_data;

    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
        gtk_tree_model_get (model, &iter,
                            MODULE_LIST_LABEL,    &label,
                            MODULE_LIST_CATEGORY, &category,
                            MODULE_LIST_MODULE,   &module,
                            MODULE_LIST_WIDGET,   &widget,
                            -1);

        if (widget != ui->m_current_widget) {
            // Hide all other widgets.
            gtk_tree_model_foreach (model, module_list_hide_widget_iter_func, NULL);
            gtk_widget_set_visible (widget, TRUE);
            ui->m_current_widget = widget;
        }

        if (module != ui->m_current_module || !module) {
            gtk_label_set_text (GTK_LABEL (ui->m_status_label), "");
            gtk_widget_set_sensitive (ui->m_apply_button, FALSE);
            gtk_widget_set_sensitive (ui->m_restore_button, FALSE);

            if (module) {
                String desc = module->get_description ();
                if (desc.length ())
                    gtk_label_set_text (GTK_LABEL (ui->m_status_label), desc.c_str ());

                if (module->query_changed () && !ui->m_config.null ()) {
                    gtk_widget_set_sensitive (ui->m_apply_button, TRUE);
                    gtk_widget_set_sensitive (ui->m_restore_button, TRUE);
                }
            }

            ui->m_current_module = module;
        }

        g_free (label);
        if (category) g_free (category);
        if (widget) g_object_unref (widget);
    }
}

void
SetupUI::restore_button_clicked_callback (GtkButton *button, gpointer user_data)
{
    SetupUI *ui = (SetupUI*) user_data;

    if (ui->m_config.null ()) return;

    if (ui->m_current_module) {
        ui->m_current_module->load_config (ui->m_config);

        gtk_widget_set_sensitive (ui->m_apply_button, FALSE);
        gtk_widget_set_sensitive (ui->m_restore_button, FALSE);
    }
}

void
SetupUI::apply_button_clicked_callback (GtkButton *button, gpointer user_data)
{
    SetupUI *ui = (SetupUI*) user_data;

    if (ui->m_config.null ()) return;

    if (ui->m_current_module) {
        ui->m_current_module->save_config (ui->m_config);

        ui->m_config->flush ();

        ui->m_changes_applied = true;

        gtk_widget_set_sensitive (ui->m_apply_button, FALSE);
        gtk_widget_set_sensitive (ui->m_restore_button, FALSE);
    }
}

gboolean
SetupUI::module_list_save_config_iter_func (GtkTreeModel *model,
                                            GtkTreePath *path,
                                            GtkTreeIter *iter,
                                            gpointer data)
{
    SetupModule *module = 0;

    SetupUI *ui = (SetupUI *) data;

    gtk_tree_model_get (model, iter, MODULE_LIST_MODULE, &module, -1);

    if (module && module->query_changed () && ui && !ui->m_config.null ()) {
        module->save_config (ui->m_config);
        ui->m_changes_applied = true;
    }

    return FALSE;
}

gboolean
SetupUI::module_list_load_config_iter_func (GtkTreeModel *model,
                                            GtkTreePath *path,
                                            GtkTreeIter *iter,
                                            gpointer data)
{
    SetupModule *module = 0;

    SetupUI *ui = (SetupUI *) data;

    gtk_tree_model_get (model, iter, MODULE_LIST_MODULE, &module, -1);

    if (module && ui && !ui->m_config.null ())
        module->load_config (ui->m_config);

    return FALSE;
}

void
SetupUI::ok_button_clicked_callback (GtkButton *button, gpointer user_data)
{
    SetupUI *ui = (SetupUI *) user_data;

    if (!ui->m_config.null ()) {
        gtk_tree_model_foreach (GTK_TREE_MODEL (ui->m_module_list_model),
                                module_list_save_config_iter_func,
                                user_data);
        ui->m_config->flush ();
    }

    ui->request_quit ();
}

void
SetupUI::exit_button_clicked_callback (GtkButton *button, gpointer user_data)
{
    SetupUI *ui = (SetupUI*) user_data;
    ui->request_quit ();
}

void
SetupUI::legacy_launch_clicked_callback (GtkButton *button, gpointer user_data)
{
    SetupUI *ui = (SetupUI*) user_data;

    const char *argv [6];
    int argc = 0;

    argv [argc++] = SCIM_SETUP_GTK3_HELPER;
    if (ui->m_legacy_config_name.length ()) {
        argv [argc++] = "--config";
        argv [argc++] = ui->m_legacy_config_name.c_str ();
    }
    if (ui->m_legacy_display.length ()) {
        argv [argc++] = "--display";
        argv [argc++] = ui->m_legacy_display.c_str ();
    }
    argv [argc] = NULL;

    GError *error = 0;
    if (!g_spawn_async (NULL, (gchar **) argv, NULL,
                        G_SPAWN_DEFAULT, NULL, NULL, NULL, &error)) {
        GtkAlertDialog *dialog = gtk_alert_dialog_new (
            _("Failed to launch the legacy settings helper: %s"),
            error ? error->message : _("unknown error"));
        gtk_alert_dialog_show (dialog, GTK_WINDOW (ui->m_main_window));
        g_object_unref (dialog);
        if (error) g_error_free (error);
    }
}

gboolean
SetupUI::main_window_close_request_callback (GtkWindow *window, gpointer user_data)
{
    SetupUI *ui = (SetupUI*) user_data;
    ui->request_quit ();
    // We tear the window down ourselves in the destructor.
    return TRUE;
}

gboolean
SetupUI::query_changed_timeout_cb (gpointer data)
{
    SetupUI *ui = (SetupUI *) data;

    if (ui->m_helper_agent.has_pending_event ())
        ui->m_helper_agent.filter_event ();

    bool modified = false;

    if (!ui->m_config.null () && ui->m_config->valid () &&
        ui->m_current_module && ui->m_current_module->query_changed ())
        modified = true;

    if (gtk_widget_get_sensitive (ui->m_apply_button) != modified)
        gtk_widget_set_sensitive (ui->m_apply_button, modified);

    if (gtk_widget_get_sensitive (ui->m_restore_button) != modified)
        gtk_widget_set_sensitive (ui->m_restore_button, modified);

    return TRUE;
}

void
SetupUI::request_quit ()
{
    if (m_changes_applied)
        show_restart_hint_then_quit ();
    else
        g_main_loop_quit (m_loop);
}

void
SetupUI::show_restart_hint_then_quit ()
{
    GtkAlertDialog *dialog = gtk_alert_dialog_new ("%s",
                            _("Not all configuration can be reloaded on the fly. "
                              "Don't forget to restart SCIM in order to let all of "
                              "the new configuration take effect."));

    gtk_alert_dialog_choose (dialog, GTK_WINDOW (m_main_window), NULL,
                             SetupUI::restart_hint_response_cb, this);
    g_object_unref (dialog);
}

void
SetupUI::restart_hint_response_cb (GObject *source, GAsyncResult *res, gpointer user_data)
{
    SetupUI *ui = (SetupUI *) user_data;

    gtk_alert_dialog_choose_finish (GTK_ALERT_DIALOG (source), res, NULL);

    g_main_loop_quit (ui->m_loop);
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
