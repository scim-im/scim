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
 * $Id: scim_setup_ui.cpp,v 1.52 2005/06/23 05:12:58 suzhe Exp $
 *
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

#define LIST_ICON_SIZE 20

#define SCIM_TRADEMARK_ICON_FILE    (SCIM_ICONDIR "/trademark.png")

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
      m_status_bar (0),
      m_module_list_view (0),
      m_module_list_selection (0),
      m_module_list_model (0),
      m_current_widget (0),
      m_current_module (0),
      m_config (config),
      m_query_changed_timeout (0),
      m_changes_applied (false)
{
    char **argv = new char * [4];
    int    argc = 1;

    argv [0] = const_cast<char*>("scim-setup");
    argv [1] = 0;

    if (display.length ()) {
        argv [argc ++] = const_cast <char*> ("--display");
        argv [argc ++] = const_cast <char *> (display.c_str ());
        argv [argc] = 0;
        setenv ("DISPLAY", display.c_str (), 1);
    }

    gtk_init (&argc, &argv);

    create_main_ui ();
    create_module_list_model ();

    m_query_changed_timeout = g_timeout_add (200, query_changed_timeout_cb, this);

    m_helper_agent.open_connection (helper_info, display);

    delete [] argv;
}

SetupUI::~SetupUI ()
{
    g_source_remove (m_query_changed_timeout);
    gtk_widget_destroy (m_main_window);
    m_helper_agent.close_connection ();
}

bool
SetupUI::add_module (SetupModule *module)
{
    if (!module || !module->valid ()) return false;

    GtkTreeIter iter, parent;

    GtkWidget *module_widget   = module->create_ui ();
    String     module_label    = module->get_name ();
    String     module_category = module->get_category ();

    if (!module_widget || !module_label.length () || !module_category.length ())
        return false;

    if (!m_config.null ())
        module->load_config (m_config);

    gtk_box_pack_start (GTK_BOX (m_work_area), module_widget, TRUE, TRUE, 0);
    gtk_widget_hide (module_widget);

    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (m_module_list_model), &parent)) {
        gchar *category;

        do {
            gtk_tree_model_get (GTK_TREE_MODEL (m_module_list_model), &parent,
                                MODULE_LIST_CATEGORY, &category, -1);

            // Find the right category and append the module.
            if (category && !strcmp (category, module_category.c_str ())) {
                gtk_tree_store_append (m_module_list_model, &iter, &parent);
                gtk_tree_store_set (
                    m_module_list_model, &iter,
                    MODULE_LIST_LABEL,    module_label.c_str (),
                    MODULE_LIST_CATEGORY, NULL,
                    MODULE_LIST_MODULE,   module,
                    MODULE_LIST_WIDGET,   module_widget,
                    -1);

                g_free (category);

                gtk_tree_view_expand_all (GTK_TREE_VIEW (m_module_list_view));

                return true;
            }

            g_free (category);

        } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (m_module_list_model), &parent));
    }

    GtkWidget *cover;

    cover = create_setup_cover (module_category.c_str ());

    gtk_box_pack_start (GTK_BOX (m_work_area), cover, TRUE, TRUE, 0);

    // No suitable category available, add one.
    gtk_tree_store_append (m_module_list_model, &parent, NULL);
    gtk_tree_store_set (
        m_module_list_model, &parent,
        MODULE_LIST_LABEL,    _(module_category.c_str ()),
        MODULE_LIST_CATEGORY, module_category.c_str (),
        MODULE_LIST_MODULE,   NULL,
        MODULE_LIST_WIDGET,   cover,
        -1);

    gtk_tree_store_append (m_module_list_model, &iter, &parent);
    gtk_tree_store_set (
        m_module_list_model, &iter,
        MODULE_LIST_LABEL,    module_label.c_str (),
        MODULE_LIST_CATEGORY, NULL,
        MODULE_LIST_MODULE,   module,
        MODULE_LIST_WIDGET,   module_widget,
        -1);

    gtk_tree_view_expand_all (GTK_TREE_VIEW (m_module_list_view));

    return true;
}

void
SetupUI::run ()
{
    SCIM_DEBUG_MAIN(1) << "SetupUI::run ()\n";

    if (m_main_window) {
        gtk_widget_show (m_main_window);

        gtk_main ();
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
    GtkWidget *hpaned1;
    GtkWidget *scrolledwindow1;
    GtkWidget *vbox1;
    GtkWidget *vbox2;
    GtkWidget *frame1;
    GtkWidget *hbox1;
    GtkWidget *ok_button;
    GtkWidget *exit_button;
    GtkWidget *vseparator1;
    GdkPixbuf *icon;

    GtkCellRenderer *module_list_cell;
    GtkTreeViewColumn *module_list_column;

    // Create main window.
    m_main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (m_main_window), _("SCIM Input Method Setup"));
    gtk_window_set_position (GTK_WINDOW (m_main_window), GTK_WIN_POS_CENTER);
    gtk_window_set_modal (GTK_WINDOW (m_main_window), TRUE);
    gtk_window_set_destroy_with_parent (GTK_WINDOW (m_main_window), TRUE);
    gtk_window_set_resizable (GTK_WINDOW (m_main_window), TRUE);

    // Set the window icon
    icon = gdk_pixbuf_new_from_file (SCIM_TRADEMARK_ICON_FILE, NULL);
    if (icon) {
        gtk_window_set_icon (GTK_WINDOW (m_main_window), icon);
        g_object_unref (icon);
    }

#if GTK_CHECK_VERSION(3, 2, 0)
    vbox1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
#else
    vbox1 = gtk_vbox_new (FALSE, 0);
#endif
    gtk_widget_show (vbox1);
    gtk_container_add (GTK_CONTAINER (m_main_window), vbox1);

    // Create paned window.
#if GTK_CHECK_VERSION(3, 2, 0)
    hpaned1 = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
#else
    hpaned1 = gtk_hpaned_new ();
#endif
    gtk_widget_show (hpaned1);
    gtk_box_pack_start (GTK_BOX (vbox1), hpaned1, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hpaned1), 4);

    // Create statusbar.
    m_status_bar = gtk_statusbar_new ();
#if GTK_CHECK_VERSION(3, 14, 0)
    // resize_grip is removed in gtk+-3.14.0
#elif GTK_CHECK_VERSION(3, 0, 0)
    gtk_window_set_has_resize_grip (GTK_WINDOW(m_main_window), TRUE);
#else
    gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (m_status_bar), TRUE);
#endif
    gtk_widget_show (m_status_bar);
    gtk_box_pack_start (GTK_BOX (vbox1), m_status_bar, FALSE, FALSE, 0);

    // Create scrollwindow for module list.
    scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (scrolledwindow1);
    gtk_paned_pack1 (GTK_PANED (hpaned1), scrolledwindow1, FALSE, FALSE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1),
                                    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow1),
                                         GTK_SHADOW_ETCHED_IN);

    // Create module list view.
    m_module_list_view = gtk_tree_view_new ();
    gtk_widget_show (m_module_list_view);
    gtk_container_add (GTK_CONTAINER (scrolledwindow1), m_module_list_view);
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (m_module_list_view), FALSE);
    gtk_tree_view_set_enable_search (GTK_TREE_VIEW (m_module_list_view), FALSE);

    // Get module list selection.
    m_module_list_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (m_module_list_view));
    gtk_tree_selection_set_mode (m_module_list_selection, GTK_SELECTION_BROWSE);

    // Create module list column.
    module_list_cell = gtk_cell_renderer_text_new ();
    module_list_column = gtk_tree_view_column_new_with_attributes (
                            NULL, module_list_cell, "text", MODULE_LIST_LABEL, NULL);

    gtk_tree_view_append_column (GTK_TREE_VIEW (m_module_list_view), module_list_column);

    // Create vbox for work area and button area.
#if GTK_CHECK_VERSION(3, 2, 0)
    vbox2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
#else
    vbox2 = gtk_vbox_new (FALSE, 0);
#endif
    gtk_widget_show (vbox2);
    gtk_paned_pack2 (GTK_PANED (hpaned1), vbox2, TRUE, FALSE);

    // Create frame for work area.
    frame1 = gtk_frame_new (NULL);
    gtk_widget_show (frame1);
    gtk_box_pack_start (GTK_BOX (vbox2), frame1, TRUE, TRUE, 0);

#if GTK_CHECK_VERSION(3, 2, 0)
    m_work_area = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
#else
    m_work_area = gtk_vbox_new (FALSE, 0);
#endif
    gtk_widget_show (m_work_area);
    gtk_container_add (GTK_CONTAINER (frame1), m_work_area);

    // Create hbox for button area.
#if GTK_CHECK_VERSION(3, 2, 0)
    hbox1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
#else
    hbox1 = gtk_hbox_new (FALSE, 0);
#endif
    gtk_widget_show (hbox1);
    gtk_box_pack_end (GTK_BOX (vbox2), hbox1, FALSE, FALSE, 8);

#if GTK_CHECK_VERSION(3, 14, 0)
    ok_button = gtk_button_new_from_icon_name ("gtk-ok", GTK_ICON_SIZE_BUTTON);
#else
    ok_button = gtk_button_new_from_stock ("gtk-ok");
#endif
    gtk_widget_show (ok_button);
    gtk_box_pack_end (GTK_BOX (hbox1), ok_button, FALSE, FALSE, 4);

#if GTK_CHECK_VERSION(3, 14, 0)
    exit_button = gtk_button_new_from_icon_name ("gtk-cancel", GTK_ICON_SIZE_BUTTON);
#else
    exit_button = gtk_button_new_from_stock ("gtk-cancel");
#endif
    gtk_widget_show (exit_button);
    gtk_box_pack_end (GTK_BOX (hbox1), exit_button, FALSE, FALSE, 4);

#if GTK_CHECK_VERSION(3, 2, 0)
    vseparator1 = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
#else
    vseparator1 = gtk_vseparator_new ();
#endif
    gtk_widget_show (vseparator1);
    gtk_box_pack_end (GTK_BOX (hbox1), vseparator1, FALSE, FALSE, 4);

#if GTK_CHECK_VERSION(3, 14, 0)
    m_apply_button = gtk_button_new_from_icon_name ("gtk-apply", GTK_ICON_SIZE_BUTTON);
#else
    m_apply_button = gtk_button_new_from_stock ("gtk-apply");
#endif
    gtk_widget_show (m_apply_button);
    gtk_box_pack_end (GTK_BOX (hbox1), m_apply_button, FALSE, FALSE, 4);
#if GTK_CHECK_VERSION(2, 18, 0)
    gtk_widget_set_can_default (m_apply_button, TRUE);
#else
    GTK_WIDGET_SET_FLAGS (m_apply_button, GTK_CAN_DEFAULT);
#endif
    gtk_widget_set_sensitive (m_apply_button, FALSE);

#if GTK_CHECK_VERSION(3, 14, 0)
    m_restore_button = gtk_button_new_from_icon_name ("gtk-revert-to-saved", GTK_ICON_SIZE_BUTTON);
#else
    m_restore_button = gtk_button_new_from_stock ("gtk-revert-to-saved");
#endif
    gtk_widget_show (m_restore_button);
    gtk_box_pack_end (GTK_BOX (hbox1), m_restore_button, FALSE, FALSE, 4);
    gtk_widget_set_sensitive (m_restore_button, FALSE);

    g_signal_connect ((gpointer) ok_button, "clicked",
                      G_CALLBACK (SetupUI::ok_button_clicked_callback),
                      this);
    g_signal_connect ((gpointer) exit_button, "clicked",
                      G_CALLBACK (SetupUI::exit_button_clicked_callback),
                      this);
    g_signal_connect ((gpointer) m_apply_button, "clicked",
                      G_CALLBACK (SetupUI::apply_button_clicked_callback),
                      this);
    g_signal_connect ((gpointer) m_restore_button, "clicked",
                      G_CALLBACK (SetupUI::restore_button_clicked_callback),
                      this);
    g_signal_connect (G_OBJECT (m_main_window), "delete_event",
                      G_CALLBACK (main_window_delete_callback),
                      this);

    g_signal_connect (G_OBJECT (m_module_list_selection), "changed",
                      G_CALLBACK (module_list_selection_changed_callback),
                      this);

    gtk_widget_grab_default (m_apply_button);
}

GtkWidget *
SetupUI::create_splash_view ()
{
    GtkRequisition size;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *view;
    GtkWidget *combo;
    GtkWidget *label;
    GtkWidget *separator;

#if GTK_CHECK_VERSION(3, 2, 0)
    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
#else
    vbox = gtk_vbox_new (FALSE, 0);
#endif
    gtk_widget_show (vbox);

    view = gtk_label_new (NULL);
    gtk_widget_show (view);
    gtk_label_set_justify (GTK_LABEL (view), GTK_JUSTIFY_CENTER);
    gtk_label_set_markup (GTK_LABEL (view), _(
                " <span size=\"20000\">Smart Common Input Method platform</span> \n\n"
                "<span size=\"16000\" style=\"italic\">GUI Setup Utility</span>\n\n\n\n"
                "<span size=\"12000\">Copyright 2002-2004, James Su &lt;suzhe@tsinghua.org.cn&gt;</span>"));
    gtk_box_pack_start (GTK_BOX (vbox), view, TRUE, TRUE, 4);

#if GTK_CHECK_VERSION(3, 0, 0)
    gtk_widget_get_preferred_size(vbox, &size, NULL);
#else
    gtk_widget_size_request (vbox, &size);
#endif

    if (size.width  < 320) size.width = 320;
    if (size.height < 240) size.height = 240;

    gtk_widget_set_size_request (vbox, size.width, size.height);

    gtk_widget_hide (vbox);

    return vbox;
}

GtkWidget *
SetupUI::create_setup_cover (const char *category)
{
    GtkRequisition size;
    GtkWidget *cover;
    char buf [128];

    snprintf (buf, 127,
              _("<span size=\"x-large\">The Setup for %s modules.</span>"),
              _(category));

    cover = gtk_label_new (NULL);

    gtk_label_set_markup (GTK_LABEL (cover), buf);
    gtk_label_set_justify (GTK_LABEL (cover), GTK_JUSTIFY_CENTER);

    gtk_widget_show (cover);

#if GTK_CHECK_VERSION(3, 0, 0)
    gtk_widget_get_preferred_size(cover, &size, NULL);
#else
    gtk_widget_size_request (cover, &size);
#endif

    if (size.width  < 320) size.width = 320;
    if (size.height < 240) size.height = 240;

    gtk_widget_set_size_request (cover, size.width, size.height);

    gtk_widget_hide (cover);

    return cover;
}

void
SetupUI::create_module_list_model ()
{
    GtkTreeIter iter;
    GtkWidget  *widget;

    widget = create_splash_view ();

    gtk_box_pack_start (GTK_BOX (m_work_area), widget, TRUE, TRUE, 0);

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
SetupUI::module_list_hide_widget_iter_func  (GtkTreeModel *model,
                                             GtkTreePath *path,
                                             GtkTreeIter *iter,
                                             gpointer data)
{
    GtkWidget    *widget;
    gtk_tree_model_get (model, iter, MODULE_LIST_WIDGET, &widget, -1);

    if (widget)
        gtk_widget_hide (widget);

    g_object_unref (widget);

    return FALSE;
}

void
SetupUI::module_list_selection_changed_callback (GtkTreeSelection *selection, gpointer user_data)
{
    GtkTreeModel *model;
    GtkTreeIter   iter;
    GtkWidget    *widget;
    SetupModule  *module;
    gchar        *label;
    gchar        *category;

    SetupUI * ui = (SetupUI *) user_data;

    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
        gtk_tree_model_get (model, &iter,
                            MODULE_LIST_LABEL,    &label,
                            MODULE_LIST_CATEGORY, &category,
                            MODULE_LIST_MODULE,   &module,
                            MODULE_LIST_WIDGET,   &widget,
                            -1);

        if (widget != ui->m_current_widget) {
            //Hide all other widgets
            gtk_tree_model_foreach (model, module_list_hide_widget_iter_func, NULL);
            gtk_widget_show (widget);
            ui->m_current_widget = widget;
        }

        if (module != ui->m_current_module || !module) {
            gtk_statusbar_pop (GTK_STATUSBAR (ui->m_status_bar), 1);
            gtk_widget_set_sensitive (ui->m_apply_button, FALSE);
            gtk_widget_set_sensitive (ui->m_restore_button, FALSE);

            if (module) {
                String desc = module->get_description ();
                if (desc.length ())
                    gtk_statusbar_push (GTK_STATUSBAR (ui->m_status_bar), 1, desc.c_str ());

                if (module->query_changed () && !ui->m_config.null ()) {
                    gtk_widget_set_sensitive (ui->m_apply_button, TRUE);
                    gtk_widget_set_sensitive (ui->m_restore_button, TRUE);
                }
            }

            ui->m_current_module = module;
        }

        g_free (label);
        if (category) g_free (category);
        g_object_unref (widget);
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
SetupUI::module_list_save_config_iter_func  (GtkTreeModel *model,
                                             GtkTreePath *path,
                                             GtkTreeIter *iter,
                                             gpointer data)
{
    SetupModule *module;

    SetupUI *ui = (SetupUI *) data;

    gtk_tree_model_get (model, iter, MODULE_LIST_MODULE, &module, -1);

    if (module && module->query_changed () && ui && !ui->m_config.null ()) {
        module->save_config (ui->m_config);
        ui->m_changes_applied = true;
    }

    return FALSE;
}

gboolean
SetupUI::module_list_load_config_iter_func  (GtkTreeModel *model,
                                             GtkTreePath *path,
                                             GtkTreeIter *iter,
                                             gpointer data)
{
    SetupModule *module;

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
        if (ui->m_changes_applied)
            ui->show_restart_hint ();
    }

    gtk_main_quit ();
}

void
SetupUI::exit_button_clicked_callback (GtkButton *button, gpointer user_data)
{
    SetupUI *ui = (SetupUI*) user_data;

    if (ui->m_changes_applied)
        ui->show_restart_hint ();
    gtk_main_quit ();
}

gboolean
SetupUI::main_window_delete_callback (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    SetupUI *ui = (SetupUI*) user_data;

    if (ui->m_changes_applied)
        ui->show_restart_hint ();

    gtk_main_quit ();

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

#if GTK_CHECK_VERSION(2, 18, 0)
    if (gtk_widget_get_sensitive (ui->m_apply_button) != modified)
#else
    if (GTK_WIDGET_SENSITIVE (ui->m_apply_button) != modified)
#endif
        gtk_widget_set_sensitive (ui->m_apply_button, modified);

#if GTK_CHECK_VERSION(2, 18, 0)
    if (gtk_widget_get_sensitive (ui->m_restore_button) != modified)
#else
    if (GTK_WIDGET_SENSITIVE (ui->m_restore_button) != modified)
#endif
        gtk_widget_set_sensitive (ui->m_restore_button, modified);


    return TRUE;
}

void
SetupUI::show_restart_hint () const
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new (GTK_WINDOW (m_main_window),
                            GTK_DIALOG_MODAL,
                            GTK_MESSAGE_INFO,
                            GTK_BUTTONS_OK,
                            _("Not all configuration can be reloaded on the fly. "
                              "Don't forget to restart SCIM in order to let all of "
                              "the new configuration take effect."));

    gtk_dialog_run (GTK_DIALOG (dialog));

    gtk_widget_destroy (dialog);
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
