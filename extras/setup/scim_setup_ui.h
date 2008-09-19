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
 *
 * $Id: scim_setup_ui.h,v 1.23 2005/01/10 08:30:45 suzhe Exp $
 */

#if !defined (__SCIM_SETUP_UI_H)
#define __SCIM_SETUP_UI_H

#include <gtk/gtk.h>

using namespace scim;

class SetupUI
{
    GtkWidget        *m_main_window;
    GtkWidget        *m_work_area;
    GtkWidget        *m_apply_button;
    GtkWidget        *m_restore_button;
    GtkWidget        *m_status_bar;

    GtkWidget        *m_module_list_view;
    GtkTreeSelection *m_module_list_selection;
    GtkTreeStore     *m_module_list_model;

    GtkWidget        *m_current_widget;
    SetupModule      *m_current_module;

    ConfigPointer     m_config;

    guint             m_query_changed_timeout;

    bool              m_changes_applied;

    HelperAgent       m_helper_agent;

    SetupUI (const SetupUI &);
    SetupUI & operator= (const SetupUI &);

public:
    SetupUI (const ConfigPointer &config, const String &display, const HelperInfo &helper_info);
    ~SetupUI ();

    bool add_module (SetupModule *module);

    // Return true if the changes have been applied.
    void run ();
private:
    void create_main_ui ();
    void create_module_list_model ();

    GtkWidget * create_splash_view ();
    GtkWidget * create_setup_cover (const char *category);

    void show_restart_hint () const;

    static void module_list_selection_changed_callback (GtkTreeSelection *selection, gpointer user_data);

    static void apply_button_clicked_callback (GtkButton *button, gpointer user_data);
    static void restore_button_clicked_callback (GtkButton *button, gpointer user_data);
    static void ok_button_clicked_callback (GtkButton *button, gpointer user_data);
    static void exit_button_clicked_callback (GtkButton *button, gpointer user_data);
    static void config_changed_callback (GtkEditable *editable, gpointer user_data);

    static gboolean main_window_delete_callback (GtkWidget *widget, GdkEvent *event, gpointer user_data);

    static gboolean query_changed_timeout_cb (gpointer data);

    static gboolean module_list_hide_widget_iter_func (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data); 

    static gboolean module_list_save_config_iter_func (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data); 

    static gboolean module_list_load_config_iter_func (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data); 
};

#endif // __SCIM_SETUP_UI_H

/*
vi:ts=4:ai:nowrap:expandtab
*/
