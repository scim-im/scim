/** @file scim_setup_ui.h
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
 */

#pragma once

#include <gtk/gtk.h>
#include <vector>

using namespace scim;

class SetupUI
{
    GtkWidget        *m_main_window;
    GtkWidget        *m_work_area;
    GtkWidget        *m_apply_button;
    GtkWidget        *m_restore_button;
    GtkWidget        *m_status_label;

    GtkWidget        *m_module_list_view;
    GtkTreeSelection *m_module_list_selection;
    GtkTreeStore     *m_module_list_model;

    GtkWidget        *m_current_widget;
    SetupModule      *m_current_module;

    ConfigPointer     m_config;

    guint             m_query_changed_timeout;

    bool              m_changes_applied;

    HelperAgent       m_helper_agent;

    GMainLoop        *m_loop;

    // How to launch the frozen GTK3 legacy setup helper.
    String            m_legacy_config_name;
    String            m_legacy_display;

    SetupUI (const SetupUI &);
    SetupUI & operator= (const SetupUI &);

public:
    SetupUI (const ConfigPointer &config, const String &display, const HelperInfo &helper_info);
    ~SetupUI ();

    // Add a GTK4 setup plugin, embedded in-process as a page.
    bool add_module (SetupModule *module);

    // Add a launcher entry that spawns the frozen GTK3 legacy setup helper for
    // the given GTK3 plugins. @config_name is the config module name to hand to
    // the helper so it talks to the same config store.
    void add_legacy_gtk3_entry (const std::vector<String> &names,
                                const String &config_name,
                                const String &display);

    // Add an informational entry listing setup plugins that cannot be shown
    // (GTK2 / unrecognized toolkit) with a rebuild hint.
    void add_unsupported_notice (const std::vector<String> &names);

    void run ();

private:
    void create_main_ui ();
    void create_module_list_model ();

    GtkWidget * create_splash_view ();
    GtkWidget * create_setup_cover (const char *category);

    gboolean find_category (const char *category, GtkTreeIter *parent_out);
    void     append_module_row (GtkTreeIter *parent, const char *label,
                                SetupModule *module, GtkWidget *widget);
    GtkTreeIter create_category (const char *category, const char *label);

    void request_quit ();

    // Kept for a future setting that cannot be applied live; see the commented
    // definitions in scim_setup_ui.cpp. Nothing calls these today.
    // void show_restart_hint_then_quit ();
    // static void restart_hint_response_cb (GObject *source, GAsyncResult *res, gpointer user_data);

    static void module_list_selection_changed_callback (GtkTreeSelection *selection, gpointer user_data);

    static void apply_button_clicked_callback (GtkButton *button, gpointer user_data);
    static void restore_button_clicked_callback (GtkButton *button, gpointer user_data);
    static void ok_button_clicked_callback (GtkButton *button, gpointer user_data);
    static void exit_button_clicked_callback (GtkButton *button, gpointer user_data);
    static void legacy_launch_clicked_callback (GtkButton *button, gpointer user_data);

    static gboolean main_window_close_request_callback (GtkWindow *window, gpointer user_data);

    static gboolean query_changed_timeout_cb (gpointer data);


    static gboolean module_list_hide_widget_iter_func (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data);

    static gboolean module_list_save_config_iter_func (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data);

    static gboolean module_list_load_config_iter_func (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data);
};

/*
vi:ts=4:ai:nowrap:expandtab
*/
