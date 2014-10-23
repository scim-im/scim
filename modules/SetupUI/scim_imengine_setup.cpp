/** @file scim_frontend_hotkeys_setup.cpp
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
 * $Id: scim_imengine_setup.cpp,v 1.9.2.2 2006/09/24 16:00:51 suzhe Exp $
 *
 */

#define Uses_SCIM_COMPOSE_KEY
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_IMENGINE_MODULE
#define Uses_SCIM_HOTKEY
#define Uses_SCIM_FILTER_MANAGER

#include <iostream>

#include <gtk/gtk.h>

#include "scim_private.h"
#include "scim.h"
#include "scimkeyselection.h"
#include "scim_stl_map.h"

using namespace scim;

#define scim_module_init aaa_imengine_setup_LTX_scim_module_init
#define scim_module_exit aaa_imengine_setup_LTX_scim_module_exit

#define scim_setup_module_create_ui       aaa_imengine_setup_LTX_scim_setup_module_create_ui
#define scim_setup_module_get_category    aaa_imengine_setup_LTX_scim_setup_module_get_category
#define scim_setup_module_get_name        aaa_imengine_setup_LTX_scim_setup_module_get_name
#define scim_setup_module_get_description aaa_imengine_setup_LTX_scim_setup_module_get_description
#define scim_setup_module_load_config     aaa_imengine_setup_LTX_scim_setup_module_load_config
#define scim_setup_module_save_config     aaa_imengine_setup_LTX_scim_setup_module_save_config
#define scim_setup_module_query_changed   aaa_imengine_setup_LTX_scim_setup_module_query_changed

#define LIST_ICON_SIZE 20

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
        return String ("IMEngine");
    }

    String scim_setup_module_get_name (void)
    {
        return String (_("Global Setup"));
    }

    String scim_setup_module_get_description (void)
    {
        return String (_("You can enable/disable input methods and set hotkeys for input methods here."));
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

// Internal data type.
enum
{
    FACTORY_LIST_ENABLE = 0,
    FACTORY_LIST_INCONSISTENT,
    FACTORY_LIST_ICON,
    FACTORY_LIST_NAME,
    FACTORY_LIST_UUID,
    FACTORY_LIST_HOTKEYS,
    FACTORY_LIST_FILTER_NAMES,
    FACTORY_LIST_FILTER_UUIDS,
    FACTORY_LIST_NUM_COLUMNS
};

enum
{
    FILTER_LIST_ENABLE = 0,
    FILTER_LIST_UUID,
    FILTER_LIST_NAME,
    FILTER_LIST_ICON,
    FILTER_LIST_LANGS,
    FILTER_LIST_DESC,
    FILTER_LIST_NUM_COLUMNS
};

#if SCIM_USE_STL_EXT_HASH_MAP
typedef __gnu_cxx::hash_map <String, std::vector <size_t>, scim_hash_string>        MapStringVectorSizeT;
typedef __gnu_cxx::hash_map <String, KeyEventList, scim_hash_string>                MapStringKeyEventList;
typedef __gnu_cxx::hash_map <String, std::vector <FilterInfo>, scim_hash_string>    MapStringVectorFilterInfo;
#elif SCIM_USE_STL_HASH_MAP
typedef std::hash_map <String, std::vector <size_t>, scim_hash_string>              MapStringVectorSizeT;
typedef std::hash_map <String, KeyEventList, scim_hash_string>                      MapStringKeyEventList;
typedef std::hash_map <String, std::vector <FilterInfo>, scim_hash_string>          MapStringVectorFilterInfo;
#else
typedef std::map <String, std::vector <size_t> >                                    MapStringVectorSizeT;
typedef std::map <String, KeyEventList>                                             MapStringKeyEventList;
typedef std::map <String, std::vector <FilterInfo> >                                MapStringVectorFilterInfo;
#endif

// Internal data declaration.
static bool           __have_changed         = false;

#if GTK_CHECK_VERSION(2, 12, 0)
#else
static GtkTooltips   *__widget_tooltips      = 0;
#endif
static GtkTreeStore  *__factory_list_model   = 0;
static GtkWidget     *__hotkey_button        = 0;
static GtkWidget     *__filter_button        = 0;

static GtkTreeIter    __selected_factory;

static std::vector <FilterInfo> __filter_infos;

// Internal functions declaration.
static GdkPixbuf *
scale_pixbuf (GdkPixbuf **pixbuf, int width, int height);

static void
get_factory_list (const ConfigPointer &config,
                  std::vector <String> &uuids,
                  std::vector <String> &names,
                  std::vector <String> &langs,
                  std::vector <String> &icons);

static GtkWidget *
create_factory_list_view ();

static GtkWidget *
create_filter_list_view ();

static void
set_filter_list_view_content (GtkTreeView *view, const std::vector <FilterInfo> & infos, const std::vector <String> &enabled_filters);

static void
get_filter_list_view_result (GtkTreeView *view, std::vector <String> &result, std::vector <String> &names);

static void
load_factory_list (const ConfigPointer &config);

static void
load_hotkey_settings (const ConfigPointer &config);

static void
save_hotkey_settings (const ConfigPointer &config);

static void
load_filter_settings (const ConfigPointer &config);

static void
save_filter_settings (const ConfigPointer &config);

static void
factory_list_update_inconsistent(void);

static void
on_hotkey_button_clicked (GtkButton *button, gpointer user_data);

static void
on_expand_button_clicked (GtkButton *button, gpointer user_data);

static void
on_collapse_button_clicked (GtkButton *button, gpointer user_data);

static void
on_toggle_all_button_clicked (GtkButton *button, gpointer user_data);

static void
on_filter_button_clicked (GtkButton *button, gpointer user_data);

static void
on_filter_move_up_button_clicked (GtkButton *button, gpointer user_data);

static void
on_filter_move_down_button_clicked (GtkButton *button, gpointer user_data);

static void
on_factory_enable_box_clicked (GtkCellRendererToggle *cell, gchar *arg1, gpointer data);

static void
on_filter_enable_box_clicked (GtkCellRendererToggle *cell, gchar *arg1, gpointer data);

static gboolean
factory_list_set_disabled_func (GtkTreeModel *model,
                                GtkTreePath  *path,
                                GtkTreeIter  *iter,
                                gpointer      data);

static gboolean
factory_list_get_disabled_func (GtkTreeModel *model,
                                GtkTreePath  *path,
                                GtkTreeIter  *iter,
                                gpointer      data);

static gboolean
factory_list_set_hotkeys_func (GtkTreeModel *model,
                               GtkTreePath  *path,
                               GtkTreeIter  *iter,
                               gpointer      data);

static gboolean
factory_list_get_hotkeys_func (GtkTreeModel *model,
                               GtkTreePath  *path,
                               GtkTreeIter  *iter,
                               gpointer      data);

static gboolean
factory_list_set_filters_func (GtkTreeModel *model,
                               GtkTreePath  *path,
                               GtkTreeIter  *iter,
                               gpointer      data);

static gboolean
factory_list_get_filters_func (GtkTreeModel *model,
                               GtkTreePath  *path,
                               GtkTreeIter  *iter,
                               gpointer      data);

static gboolean
factory_list_toggle_all_func  (GtkTreeModel *model,
                               GtkTreePath  *path,
                               GtkTreeIter  *iter,
                               gpointer      data);

static void
factory_list_selection_changed_callback (GtkTreeSelection *selection, gpointer user_data);

// Function implementations.
static GtkWidget *
create_setup_window ()
{
    static GtkWidget *window = 0;

    if (!window) {
        GtkWidget *view;
        GtkWidget *hbox;
        GtkWidget *label;
        GtkWidget *sep;
        GtkWidget *scrolledwindow;
        GtkWidget *button;

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

        label = gtk_label_new (_("The installed input method services:"));
#if GTK_CHECK_VERSION(3, 14, 0)
        gtk_widget_set_halign (label, GTK_ALIGN_START);
        gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
        gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
#endif
        gtk_widget_show (label);
        gtk_box_pack_start (GTK_BOX (window), label, FALSE, FALSE, 0);

        scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
        gtk_widget_show (scrolledwindow);
        gtk_box_pack_start (GTK_BOX (window), scrolledwindow, TRUE, TRUE, 4);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
                                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow),
                                             GTK_SHADOW_NONE);

        // Create hotkey and filter button before factory list view, because
        // factory_list_selection_changed_callback may access these two buttons.
        __hotkey_button = gtk_button_new_with_mnemonic (_("Edit _Hotkeys"));
        gtk_widget_show (__hotkey_button);
        gtk_widget_set_sensitive (__hotkey_button, FALSE);

        g_signal_connect ((gpointer) __hotkey_button, "clicked",
                          G_CALLBACK (on_hotkey_button_clicked),
                          NULL);

#if GTK_CHECK_VERSION(2, 12, 0)
        gtk_widget_set_tooltip_text (__hotkey_button,
                              _("Edit Hotkeys associated with the selected input method."));
#else
        gtk_tooltips_set_tip (__widget_tooltips, __hotkey_button,
                              _("Edit Hotkeys associated with the selected input method."), NULL);
#endif

        __filter_button = gtk_button_new_with_mnemonic (_("Select _Filters"));
        gtk_widget_show (__filter_button);
        gtk_widget_set_sensitive (__filter_button, FALSE);

        g_signal_connect ((gpointer) __filter_button, "clicked",
                          G_CALLBACK (on_filter_button_clicked),
                          NULL);

#if GTK_CHECK_VERSION(2, 12, 0)
        gtk_widget_set_tooltip_text (__filter_button,
                              _("Select the Filters which will be attached to this input method."));
#else
        gtk_tooltips_set_tip (__widget_tooltips, __filter_button,
                              _("Select the Filters which will be attached to this input method."), NULL);
#endif

        view = create_factory_list_view ();
        gtk_widget_show (view);
        gtk_container_add (GTK_CONTAINER (scrolledwindow), view);

#if GTK_CHECK_VERSION(3, 2, 0)
        sep = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
#else
        sep = gtk_hseparator_new ();
#endif
        gtk_widget_show (sep);
        gtk_box_pack_start (GTK_BOX (window), sep, FALSE, FALSE, 2);

#if GTK_CHECK_VERSION(3, 2, 0)
        hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
#else
        hbox = gtk_hbox_new (FALSE, 0);
#endif
        gtk_widget_show (hbox);
        gtk_box_pack_start (GTK_BOX (window), hbox, FALSE, FALSE, 2);

        gtk_box_pack_end (GTK_BOX (hbox), __hotkey_button, FALSE, FALSE, 4);
        gtk_box_pack_end (GTK_BOX (hbox), __filter_button, FALSE, FALSE, 4);

        button = gtk_button_new_with_mnemonic (_("_Expand"));
        gtk_widget_show (button);
        gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 4);

        g_signal_connect ((gpointer) button, "clicked",
                          G_CALLBACK (on_expand_button_clicked),
                          (gpointer) view);

#if GTK_CHECK_VERSION(2, 12, 0)
        gtk_widget_set_tooltip_text (button,
                              _("Expand all language categories."));
#else
        gtk_tooltips_set_tip (__widget_tooltips, button,
                              _("Expand all language categories."), NULL);
#endif

        button = gtk_button_new_with_mnemonic (_("_Collapse"));
        gtk_widget_show (button);
        gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 4);

        g_signal_connect ((gpointer) button, "clicked",
                          G_CALLBACK (on_collapse_button_clicked),
                          (gpointer) view);

#if GTK_CHECK_VERSION(2, 12, 0)
        gtk_widget_set_tooltip_text (button,
                              _("Collapse all language categories."));
#else
        gtk_tooltips_set_tip (__widget_tooltips, button,
                              _("Collapse all language categories."), NULL);
#endif

        button = gtk_button_new_with_mnemonic (_("E_nable All"));
        gtk_widget_show (button);
        gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 4);

        g_signal_connect ((gpointer) button, "clicked",
                          G_CALLBACK (on_toggle_all_button_clicked),
                          (gpointer) (1));

#if GTK_CHECK_VERSION(2, 12, 0)
        gtk_widget_set_tooltip_text (button,
                              _("Enable all input methods."));
#else
        gtk_tooltips_set_tip (__widget_tooltips, button,
                              _("Enable all input methods."), NULL);
#endif

        button = gtk_button_new_with_mnemonic (_("_Disable All"));
        gtk_widget_show (button);
        gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 4);

        g_signal_connect ((gpointer) button, "clicked",
                          G_CALLBACK (on_toggle_all_button_clicked),
                          (gpointer) (0));

#if GTK_CHECK_VERSION(2, 12, 0)
        gtk_widget_set_tooltip_text (button,
                              _("Disable all input methods."));
#else
        gtk_tooltips_set_tip (__widget_tooltips, button,
                              _("Disable all input methods."), NULL);
#endif
    }
    return window;
}

static GtkWidget *
create_factory_list_view ()
{
    GtkWidget *view;
    GtkTreeSelection *selection;

    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    view = gtk_tree_view_new ();
    gtk_widget_show (view);
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (view), TRUE);
#if !GTK_CHECK_VERSION(3, 14, 0)
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (view), TRUE);
#endif

    // Name column
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_resizable (column, TRUE);

    gtk_tree_view_column_set_title (column, _("Name"));

    renderer = gtk_cell_renderer_pixbuf_new ();
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_set_attributes (column, renderer,
                                         "pixbuf", FACTORY_LIST_ICON, NULL);

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, renderer,
                                         "text", FACTORY_LIST_NAME, NULL);

    gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);

    // Enable column
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_title (column, _("Enable"));

    renderer = gtk_cell_renderer_toggle_new ();
    gtk_cell_renderer_toggle_set_radio (GTK_CELL_RENDERER_TOGGLE (renderer), FALSE);
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_set_attributes (column, renderer,
                                         "active", FACTORY_LIST_ENABLE,
                                         "inconsistent", FACTORY_LIST_INCONSISTENT,
                                         NULL);

    g_signal_connect (G_OBJECT (renderer), "toggled",
                      G_CALLBACK (on_factory_enable_box_clicked),
                      NULL);

    gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);

    // Hotkey column
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_resizable (column, TRUE);

    gtk_tree_view_column_set_title (column, _("Hotkeys"));

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, renderer,
                                         "text", FACTORY_LIST_HOTKEYS, NULL);

    gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);

    // Filter column
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_resizable (column, TRUE);

    gtk_tree_view_column_set_title (column, _("Filters"));

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, renderer,
                                         "text", FACTORY_LIST_FILTER_NAMES, NULL);

    gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);

    // selection
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

    g_signal_connect (G_OBJECT (selection), "changed",
                      G_CALLBACK (factory_list_selection_changed_callback),
                      NULL);

    // Create model.
    __factory_list_model = gtk_tree_store_new (FACTORY_LIST_NUM_COLUMNS,
                                               G_TYPE_BOOLEAN,
                                               G_TYPE_BOOLEAN,
                                               GDK_TYPE_PIXBUF,
                                               G_TYPE_STRING,
                                               G_TYPE_STRING,
                                               G_TYPE_STRING,
                                               G_TYPE_STRING,
                                               G_TYPE_STRING);

    gtk_tree_view_set_model (GTK_TREE_VIEW (view),
                             GTK_TREE_MODEL (__factory_list_model));

    gtk_tree_view_collapse_all (GTK_TREE_VIEW (view));

    return view;
}

static GtkWidget *
create_filter_list_view ()
{
    GtkWidget *view;

    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    GtkListStore  *filter_list_model;

    view = gtk_tree_view_new ();
    gtk_widget_show (view);
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (view), TRUE);
#if !GTK_CHECK_VERSION(3, 14, 0)
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (view), TRUE);
#endif

    // Enable column
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_resizable (column, FALSE);
    gtk_tree_view_column_set_title (column, _("Enable"));

    renderer = gtk_cell_renderer_toggle_new ();
    gtk_cell_renderer_toggle_set_radio (GTK_CELL_RENDERER_TOGGLE (renderer), FALSE);
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_set_attributes (column, renderer,
                                         "active", FILTER_LIST_ENABLE,
                                         NULL);

    g_signal_connect (G_OBJECT (renderer), "toggled",
                      G_CALLBACK (on_filter_enable_box_clicked),
                      (gpointer) view);

    gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);

    // Icon and column
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_title (column, _("Name"));

    renderer = gtk_cell_renderer_pixbuf_new ();
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_set_attributes (column, renderer,
                                         "pixbuf", FILTER_LIST_ICON, NULL);

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, renderer,
                                         "text", FILTER_LIST_NAME, NULL);

    gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);

    // Languages column
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_resizable (column, TRUE);

    gtk_tree_view_column_set_title (column, _("Languages"));

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, renderer,
                                         "text", FILTER_LIST_LANGS, NULL);

    gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);

    // Description column
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_resizable (column, TRUE);

    gtk_tree_view_column_set_title (column, _("Description"));

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, renderer,
                                         "text", FILTER_LIST_DESC, NULL);

    gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);

    // Create model.
    filter_list_model = gtk_list_store_new (FILTER_LIST_NUM_COLUMNS,
                                            G_TYPE_BOOLEAN,
                                            G_TYPE_STRING,
                                            G_TYPE_STRING,
                                            GDK_TYPE_PIXBUF,
                                            G_TYPE_STRING,
                                            G_TYPE_STRING);

    gtk_tree_view_set_model (GTK_TREE_VIEW (view), GTK_TREE_MODEL (filter_list_model));

    return view;
}

static GdkPixbuf *
scale_pixbuf (GdkPixbuf **pixbuf, int width, int height)
{
    if (pixbuf && *pixbuf) {
        if (gdk_pixbuf_get_width (*pixbuf) != width ||
            gdk_pixbuf_get_height (*pixbuf) != height) {
            GdkPixbuf *dest = gdk_pixbuf_scale_simple (*pixbuf, width, height, GDK_INTERP_BILINEAR);
            g_object_unref (*pixbuf);
            *pixbuf = dest;
        }
        return *pixbuf;
    }
    return 0;
}

static void
get_factory_list (const ConfigPointer &config,
                  std::vector <String> &uuids,
                  std::vector <String> &names,
                  std::vector <String> &langs,
                  std::vector <String> &icons)
{
    std::vector<String>    module_list;
    IMEngineFactoryPointer factory;
    IMEngineModule         module;

    scim_get_imengine_module_list (module_list);

    uuids.clear ();
    names.clear ();
    langs.clear ();
    icons.clear ();

    // Add "English/European" factory first.
    factory = new ComposeKeyFactory ();
    uuids.push_back (factory->get_uuid ());
    names.push_back (utf8_wcstombs (factory->get_name ()));
    langs.push_back (scim_get_normalized_language (factory->get_language ()));
    icons.push_back (factory->get_icon_file ());

    for (size_t i = 0; i < module_list.size (); ++ i) {

        module.load (module_list [i], config);

        if (module.valid ()) {
            for (size_t j = 0; j < module.number_of_factories (); ++j) {
                try {
                    factory = module.create_factory (j);
                } catch (...) {
                    factory.reset ();
                }

                if (!factory.null ()) {
                    if (std::find (uuids.begin (), uuids.end (), factory->get_uuid ()) == uuids.end ()) {
                        uuids.push_back (factory->get_uuid ());
                        names.push_back (utf8_wcstombs (factory->get_name ()));
                        langs.push_back (scim_get_normalized_language (factory->get_language ()));
                        icons.push_back (factory->get_icon_file ());
                    }
                    factory.reset ();
                }
            }
            module.unload ();
        }
    }
}

static void
load_factory_list (const ConfigPointer &config)
{
    if (!__factory_list_model) return;

    std::vector <String> uuids;
    std::vector <String> names;
    std::vector <String> langs;
    std::vector <String> icons;

    MapStringVectorSizeT groups;

    GtkTreeIter iter;
    GtkTreeIter parent;

    String      lang_name;
    GdkPixbuf   *pixbuf;
    size_t i;

    get_factory_list (config, uuids, names, langs, icons);

    for (i = 0; i < uuids.size (); ++i) {
        groups [langs [i]].push_back (i);
    }

    gtk_tree_store_clear (GTK_TREE_STORE (__factory_list_model));

    // Add language group
    for (MapStringVectorSizeT::iterator it = groups.begin ();
         it != groups.end (); ++ it) {
        lang_name = scim_get_language_name (it->first);
        gtk_tree_store_append (__factory_list_model, &parent, NULL);
        gtk_tree_store_set (__factory_list_model, &parent,
                            FACTORY_LIST_ENABLE, true,
                            FACTORY_LIST_INCONSISTENT, FALSE,
                            FACTORY_LIST_ICON, NULL,
                            FACTORY_LIST_NAME, lang_name.c_str (),
                            FACTORY_LIST_UUID, NULL,
                            FACTORY_LIST_HOTKEYS, NULL,
                            -1);

        // Add factories for this group
        for (i = 0; i < it->second.size (); ++i) {
            pixbuf = gdk_pixbuf_new_from_file (icons [it->second [i]].c_str (), NULL);
            scale_pixbuf (&pixbuf, LIST_ICON_SIZE, LIST_ICON_SIZE);
            gtk_tree_store_append (__factory_list_model, &iter, &parent);
            gtk_tree_store_set (__factory_list_model, &iter,
                                FACTORY_LIST_ENABLE, true,
                                FACTORY_LIST_INCONSISTENT, FALSE,
                                FACTORY_LIST_ICON, pixbuf,
                                FACTORY_LIST_NAME, names [it->second [i]].c_str (),
                                FACTORY_LIST_UUID, uuids [it->second [i]].c_str (),
                                FACTORY_LIST_HOTKEYS, NULL,
                                -1);
            if (pixbuf)
                g_object_unref (pixbuf);
        }
    }
}

static void
load_hotkey_settings (const ConfigPointer &config)
{
    // Load Hotkeys.
    IMEngineHotkeyMatcher hotkey_matcher;

    hotkey_matcher.load_hotkeys (config);
    KeyEventList keys;
    std::vector <String> uuids;

    MapStringKeyEventList hotkey_map;

    if (hotkey_matcher.get_all_hotkeys (keys, uuids) > 0) {
        for (size_t i = 0; i < keys.size (); ++i)
            hotkey_map [uuids[i]].push_back (keys [i]);
    }

    gtk_tree_model_foreach (GTK_TREE_MODEL (__factory_list_model),
                            factory_list_set_hotkeys_func,
                            static_cast <gpointer> (&hotkey_map));
}

static void
save_hotkey_settings (const ConfigPointer &config)
{
    // Save Hotkeys.
    IMEngineHotkeyMatcher hotkey_matcher;
    MapStringKeyEventList hotkey_map;

    gtk_tree_model_foreach (GTK_TREE_MODEL (__factory_list_model),
                            factory_list_get_hotkeys_func,
                            static_cast <gpointer> (&hotkey_map));

    for (MapStringKeyEventList::iterator it = hotkey_map.begin (); it != hotkey_map.end (); ++it)
        hotkey_matcher.add_hotkeys (it->second, it->first);

    hotkey_matcher.save_hotkeys (config);
}

static void
load_filter_settings (const ConfigPointer &config)
{
    FilterManager m_manager (config);

    unsigned int nfilters = m_manager.number_of_filters ();

    __filter_infos.clear ();

    if (!nfilters) return;

    FilterInfo info;

    // Get information of all filters.
    for (unsigned int i = 0; i < nfilters; ++i) {
        if (m_manager.get_filter_info (i, info))
            __filter_infos.push_back (info);
    }

    // Load Filter infos.
    std::vector <String> filtered_imes;

    MapStringVectorFilterInfo filter_map;

    if (m_manager.get_filtered_imengines (filtered_imes) > 0) {
        std::vector <String> filter_uuids;
        for (size_t i = 0; i < filtered_imes.size (); ++i) {
            if (m_manager.get_filters_for_imengine (filtered_imes [i], filter_uuids)) {
                for (size_t j = 0; j < filter_uuids.size (); ++j) {
                    if (m_manager.get_filter_info (filter_uuids [j], info))
                        filter_map [filtered_imes [i]].push_back (info);
                }
            }
        }

    }

    gtk_tree_model_foreach (GTK_TREE_MODEL (__factory_list_model),
                            factory_list_set_filters_func,
                            static_cast <gpointer> (&filter_map));
}

static void
save_filter_settings (const ConfigPointer &config)
{
    FilterManager m_manager (config);

    MapStringVectorFilterInfo filter_map;

    gtk_tree_model_foreach (GTK_TREE_MODEL (__factory_list_model),
                            factory_list_get_filters_func,
                            static_cast <gpointer> (&filter_map));

    m_manager.clear_all_filter_settings ();

    for (MapStringVectorFilterInfo::iterator it = filter_map.begin (); it != filter_map.end (); ++it) {
        std::vector <String> filters;

        for (size_t i = 0; i < it->second.size (); ++i)
            filters.push_back (it->second [i].uuid);

        m_manager.set_filters_for_imengine (it->first, filters);
    }
}

static gboolean
factory_list_set_disabled_func (GtkTreeModel *model,
                                GtkTreePath  *path,
                                GtkTreeIter  *iter,
                                gpointer      data)
{
    gchar *uuid;
    std::vector <String> *disabled = static_cast <std::vector<String> *> (data);

    gtk_tree_model_get (model, iter,
                        FACTORY_LIST_UUID, &uuid,
                        -1);
    if (uuid && std::binary_search (disabled->begin (), disabled->end (), String (uuid))) {
        gtk_tree_store_set (GTK_TREE_STORE (model), iter,
                            FACTORY_LIST_ENABLE, FALSE,
                            -1);
    } else {
        gtk_tree_store_set (GTK_TREE_STORE (model), iter,
                            FACTORY_LIST_ENABLE, TRUE,
                            -1);
    }

    if (uuid) g_free (uuid);

    return FALSE;
}

static gboolean
factory_list_get_disabled_func (GtkTreeModel *model,
                                GtkTreePath  *path,
                                GtkTreeIter  *iter,
                                gpointer      data)
{
    gchar *uuid;
    gboolean enable;
    std::vector <String> *disabled = static_cast <std::vector<String> *> (data);

    gtk_tree_model_get (model, iter,
                        FACTORY_LIST_UUID, &uuid,
                        FACTORY_LIST_ENABLE, &enable,
                        -1);

    if (!enable && uuid)
    disabled->push_back (String (uuid));

    if (uuid) g_free (uuid);

    return FALSE;
}

static gboolean
factory_list_set_hotkeys_func (GtkTreeModel *model,
                               GtkTreePath  *path,
                               GtkTreeIter  *iter,
                               gpointer      data)
{
    gchar *uuid;
    MapStringKeyEventList::iterator it;

    MapStringKeyEventList *hotkey_map = static_cast <MapStringKeyEventList *> (data);

    gtk_tree_model_get (model, iter,
                        FACTORY_LIST_UUID, &uuid,
                        -1);

    if (uuid && (it = hotkey_map->find (String (uuid))) != hotkey_map->end ()) {
        String str;
        scim_key_list_to_string (str, it->second);
        gtk_tree_store_set (GTK_TREE_STORE (model), iter,
                            FACTORY_LIST_HOTKEYS, str.c_str (),
                            -1);
    } else {
        gtk_tree_store_set (GTK_TREE_STORE (model), iter,
                            FACTORY_LIST_HOTKEYS, NULL,
                            -1);
    }

    if (uuid) g_free (uuid);

    return FALSE;
}

static gboolean
factory_list_get_hotkeys_func (GtkTreeModel *model,
                               GtkTreePath  *path,
                               GtkTreeIter  *iter,
                               gpointer      data)
{
    gchar *uuid;
    gchar *hotkeys;

    MapStringKeyEventList *hotkey_map = static_cast <MapStringKeyEventList *> (data);

    gtk_tree_model_get (model, iter,
                        FACTORY_LIST_UUID, &uuid,
                        FACTORY_LIST_HOTKEYS, &hotkeys,
                        -1);

    if (hotkeys && uuid) {
        KeyEventList keylist;
        if (scim_string_to_key_list (keylist, String (hotkeys)))
            hotkey_map->insert (std::make_pair (String (uuid), keylist));
    }

    if (uuid) g_free (uuid);
    if (hotkeys) g_free (hotkeys);

    return FALSE;
}

static gboolean
factory_list_set_filters_func (GtkTreeModel *model,
                               GtkTreePath  *path,
                               GtkTreeIter  *iter,
                               gpointer      data)
{
    gchar *uuid;
    MapStringVectorFilterInfo::iterator it;
    MapStringVectorFilterInfo *filter_map = static_cast <MapStringVectorFilterInfo*> (data);

    gtk_tree_model_get (model, iter,
                        FACTORY_LIST_UUID, &uuid,
                        -1);

    if (uuid && (it = filter_map->find (String (uuid))) != filter_map->end ()) {
        std::vector <String> names;
        std::vector <String> uuids;
        for (size_t i = 0; i < it->second.size (); ++i) {
            names.push_back (it->second [i].name);
            uuids.push_back (it->second [i].uuid);
        }
        gtk_tree_store_set (GTK_TREE_STORE (model), iter,
                            FACTORY_LIST_FILTER_NAMES, scim_combine_string_list (names).c_str (),
                            FACTORY_LIST_FILTER_UUIDS, scim_combine_string_list (uuids).c_str (),
                            -1);
    } else {
        gtk_tree_store_set (GTK_TREE_STORE (model), iter,
                            FACTORY_LIST_FILTER_NAMES, NULL,
                            FACTORY_LIST_FILTER_UUIDS, NULL,
                            -1);
    }

    if (uuid) g_free (uuid);

    return FALSE;
}

static gboolean
factory_list_get_filters_func (GtkTreeModel *model,
                               GtkTreePath  *path,
                               GtkTreeIter  *iter,
                               gpointer      data)
{
    gchar *uuid;
    gchar *filter_uuids;

    MapStringVectorFilterInfo *filter_map = static_cast <MapStringVectorFilterInfo *> (data);

    gtk_tree_model_get (model, iter,
                        FACTORY_LIST_UUID, &uuid,
                        FACTORY_LIST_FILTER_UUIDS, &filter_uuids,
                        -1);

    if (filter_uuids && uuid) {
        std::vector <String> strvec;
        scim_split_string_list (strvec, String (filter_uuids));

        std::vector <FilterInfo> infovec;

        for (size_t i = 0; i < strvec.size (); ++i)
            infovec.push_back (FilterInfo (strvec [i]));

        if (infovec.size ())
            filter_map->insert (std::make_pair (String (uuid), infovec));
    }

    if (uuid) g_free (uuid);
    if (filter_uuids) g_free (filter_uuids);

    return FALSE;
}

static gboolean
factory_list_toggle_all_func  (GtkTreeModel *model,
                               GtkTreePath  *path,
                               GtkTreeIter  *iter,
                               gpointer      data)
{
    gboolean enable = (data != 0);

    gtk_tree_store_set (GTK_TREE_STORE (model), iter,
                        FACTORY_LIST_ENABLE, enable,
                        FACTORY_LIST_INCONSISTENT, FALSE,
                        -1);
    return FALSE;
}

static void
load_config (const ConfigPointer &config)
{
    if (__factory_list_model) {
        load_factory_list (config);

        // Load disabled IMEngines list.
        std::vector <String> disabled;

        disabled = scim_global_config_read (String (SCIM_GLOBAL_CONFIG_DISABLED_IMENGINE_FACTORIES), disabled);

        std::sort (disabled.begin (), disabled.end ());

        gtk_tree_model_foreach (GTK_TREE_MODEL (__factory_list_model),
                                factory_list_set_disabled_func,
                                static_cast <gpointer> (&disabled));

        factory_list_update_inconsistent ();

        load_hotkey_settings (config);

        load_filter_settings (config);
    }

    __have_changed = false;
}

static void
save_config (const ConfigPointer &config)
{

    if (__factory_list_model && __have_changed) {
        // Save disabled IMEngines list.
        std::vector <String> disabled;

        gtk_tree_model_foreach (GTK_TREE_MODEL (__factory_list_model),
                                factory_list_get_disabled_func,
                                static_cast <gpointer> (&disabled));

        scim_global_config_write (String (SCIM_GLOBAL_CONFIG_DISABLED_IMENGINE_FACTORIES), disabled);

        save_hotkey_settings (config);

        save_filter_settings (config);
    }

    __have_changed = false;
}

static bool
query_changed ()
{
    return __have_changed;
}

static void
factory_list_update_inconsistent(void)
{
    GtkTreeIter iter;
    GtkTreeIter childiter;
    gboolean    enable;
    gboolean    inconsistent;

    if (!__factory_list_model || !gtk_tree_model_get_iter_first (GTK_TREE_MODEL (__factory_list_model), &iter))
        return;

    do {
        gtk_tree_model_get (GTK_TREE_MODEL (__factory_list_model), &iter,
                            FACTORY_LIST_ENABLE, &enable,
                            FACTORY_LIST_INCONSISTENT, &inconsistent,
                            -1);
        if (gtk_tree_model_iter_children (GTK_TREE_MODEL (__factory_list_model), &childiter, &iter)) {
            gint enable_count = 0;
            gint total_count = gtk_tree_model_iter_n_children(GTK_TREE_MODEL (__factory_list_model), &iter);
            do {
                gboolean child_enable;
                gtk_tree_model_get (GTK_TREE_MODEL (__factory_list_model), &childiter,
                                    FACTORY_LIST_ENABLE, &child_enable,
                                    -1);
                if (child_enable) ++ enable_count;
            } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (__factory_list_model), &childiter));
            enable = (enable_count && enable_count >= ((total_count+1) >> 1));
            inconsistent = (enable_count && enable_count < total_count);
        }
        gtk_tree_store_set (GTK_TREE_STORE (__factory_list_model), &iter,
                            FACTORY_LIST_ENABLE, enable,
                            FACTORY_LIST_INCONSISTENT, inconsistent,
                            -1);
    } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (__factory_list_model), &iter));
}

static void
on_factory_enable_box_clicked (GtkCellRendererToggle *cell, gchar *arg1, gpointer data)
{
    GtkTreePath *path;
    GtkTreeIter iter, childiter;
    gboolean enable;

    path = gtk_tree_path_new_from_string (arg1);
    if (gtk_tree_model_get_iter (GTK_TREE_MODEL (__factory_list_model), &iter, path)) {
        if (gtk_tree_model_iter_children(GTK_TREE_MODEL (__factory_list_model), &childiter, &iter)) {
            gtk_tree_model_get (GTK_TREE_MODEL (__factory_list_model), &iter,
                                FACTORY_LIST_ENABLE, &enable,
                                -1);
            enable = !enable;
            gtk_tree_store_set (__factory_list_model, &iter,
                                FACTORY_LIST_ENABLE, enable,
                                FACTORY_LIST_INCONSISTENT, FALSE,
                                -1);
            do {
                gtk_tree_store_set (__factory_list_model, &childiter,
                                    FACTORY_LIST_ENABLE, enable,
                                    -1);
            } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (__factory_list_model), &childiter));
        } else {
            gtk_tree_model_get (GTK_TREE_MODEL (__factory_list_model), &iter,
                                FACTORY_LIST_ENABLE, &enable,
                                -1);
            gtk_tree_store_set (__factory_list_model, &iter,
                                FACTORY_LIST_ENABLE, !enable,
                                -1);
            factory_list_update_inconsistent ();
        }
    }
    gtk_tree_path_free (path);

    __have_changed = true;
}

static void
on_filter_enable_box_clicked (GtkCellRendererToggle *cell, gchar *arg1, gpointer data)
{
    GtkTreePath  *path;
    GtkTreeIter   iter;
    gboolean      enable;
    GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (data));

    path = gtk_tree_path_new_from_string (arg1);
    if (gtk_tree_model_get_iter (model, &iter, path)) {
        gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
                            FILTER_LIST_ENABLE, &enable,
                            -1);
        enable = !enable;
        gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                            FILTER_LIST_ENABLE, enable,
                            -1);
    }
    gtk_tree_path_free (path);
}

static void
on_hotkey_button_clicked (GtkButton *button, gpointer user_data)
{
    gchar *uuid;
    gchar *hotkeys;
    gchar *name;

    gtk_tree_model_get (GTK_TREE_MODEL (__factory_list_model), &__selected_factory,
                        FACTORY_LIST_UUID, &uuid,
                        FACTORY_LIST_HOTKEYS, &hotkeys,
                        FACTORY_LIST_NAME, &name,
                        -1);

    if (uuid) {
        char buf [256];
        snprintf (buf, 256, _("Edit Hotkeys for %s"), name);

        GtkWidget *dialog = scim_key_selection_dialog_new (buf);
        gint result;

        if (hotkeys) {
            scim_key_selection_dialog_set_keys (
                SCIM_KEY_SELECTION_DIALOG (dialog),
                hotkeys);
        }

        result = gtk_dialog_run (GTK_DIALOG (dialog));

        if (result == GTK_RESPONSE_OK) {
            const gchar *newkeys = scim_key_selection_dialog_get_keys (SCIM_KEY_SELECTION_DIALOG (dialog));

            if ((newkeys && hotkeys && String (newkeys) != String (hotkeys)) || (newkeys || hotkeys)) {
                gtk_tree_store_set (__factory_list_model, &__selected_factory,
                                    FACTORY_LIST_HOTKEYS, newkeys,
                                    -1);
                __have_changed = true;
            }
        }

        gtk_widget_destroy (dialog);
    }

    if (uuid) g_free (uuid);
    if (hotkeys) g_free (hotkeys);
    if (name) g_free (name);
}

static void
on_expand_button_clicked (GtkButton *button, gpointer user_data)
{
    GtkWidget *view = static_cast<GtkWidget *>(user_data);

    if (view)
        gtk_tree_view_expand_all (GTK_TREE_VIEW (view));
}

static void
on_collapse_button_clicked (GtkButton *button, gpointer user_data)
{
    GtkWidget *view = static_cast<GtkWidget *>(user_data);

    if (view)
        gtk_tree_view_collapse_all (GTK_TREE_VIEW (view));
}

static void
factory_list_selection_changed_callback (GtkTreeSelection *selection, gpointer user_data)
{
    GtkTreeModel *model;

    if (gtk_tree_selection_get_selected (selection, &model, &__selected_factory)) {
        if (gtk_tree_model_iter_has_child (model, &__selected_factory)) {
            gtk_widget_set_sensitive (__hotkey_button, FALSE);
            gtk_widget_set_sensitive (__filter_button, FALSE);
        } else {
            gtk_widget_set_sensitive (__hotkey_button, TRUE);
            if (__filter_infos.size ())
                gtk_widget_set_sensitive (__filter_button, TRUE);
        }
    } else {
        gtk_widget_set_sensitive (__hotkey_button, FALSE);
        gtk_widget_set_sensitive (__filter_button, FALSE);
    }
}

static void
on_toggle_all_button_clicked (GtkButton *button, gpointer user_data)
{
    gtk_tree_model_foreach (GTK_TREE_MODEL (__factory_list_model),
                            factory_list_toggle_all_func,
                            user_data);

    __have_changed = true;
}

static void
set_filter_list_view_content (GtkTreeView *view, const std::vector <FilterInfo> & infos, const std::vector <String> &enabled_filters)
{
    std::vector <String> lang_ids;
    std::vector <String> lang_names;

    std::vector <FilterInfo> disabled_infos = infos;
    std::vector <FilterInfo> enabled_infos;

    std::vector <FilterInfo>::iterator fiit;

    std::vector <String>::const_iterator sit;

    GdkPixbuf           *pixbuf;
    GtkTreeIter          iter;
    GtkTreeModel        *model = gtk_tree_view_get_model (view);

    gtk_list_store_clear (GTK_LIST_STORE (model));

    // Put the enabled filters in the front.
    for (sit = enabled_filters.begin (); sit != enabled_filters.end (); ++sit) {
        for (fiit = disabled_infos.begin (); fiit != disabled_infos.end (); ++fiit) {
            if (fiit->uuid == *sit) {
                enabled_infos.push_back (*fiit);
                disabled_infos.erase (fiit);
                break;
            }
        }
    }

    for (fiit = enabled_infos.begin (); fiit != enabled_infos.end (); ++fiit) {
        pixbuf = gdk_pixbuf_new_from_file (fiit->icon.c_str (), NULL);
        scale_pixbuf (&pixbuf, LIST_ICON_SIZE, LIST_ICON_SIZE);
        scim_split_string_list (lang_ids, fiit->langs);
        lang_names.clear ();

        for (sit = lang_ids.begin (); sit != lang_ids.end (); ++sit) {
            String name = scim_get_language_name (*sit);
            if (std::find (lang_names.begin (), lang_names.end (), name) == lang_names.end ())
                lang_names.push_back (name);
        }

        String langs = scim_combine_string_list (lang_names);
        if (!langs.length ()) langs = "";

        gtk_list_store_append (GTK_LIST_STORE (model), &iter);
        gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                            FILTER_LIST_ENABLE, TRUE,
                            FILTER_LIST_ICON, pixbuf,
                            FILTER_LIST_UUID, fiit->uuid.c_str (),
                            FILTER_LIST_NAME, fiit->name.c_str (),
                            FILTER_LIST_LANGS, langs.c_str (),
                            FILTER_LIST_DESC, fiit->desc.c_str (),
                            -1);

        if (pixbuf)
            g_object_unref (pixbuf);
    }

    for (fiit = disabled_infos.begin (); fiit != disabled_infos.end (); ++fiit) {
        pixbuf = gdk_pixbuf_new_from_file (fiit->icon.c_str (), NULL);
        scale_pixbuf (&pixbuf, LIST_ICON_SIZE, LIST_ICON_SIZE);
        scim_split_string_list (lang_ids, fiit->langs);
        lang_names.clear ();

        for (sit = lang_ids.begin (); sit != lang_ids.end (); ++sit) {
            String name = scim_get_language_name (*sit);
            if (std::find (lang_names.begin (), lang_names.end (), name) == lang_names.end ())
                lang_names.push_back (name);
        }

        String langs = scim_combine_string_list (lang_names);
        if (!langs.length ()) langs = "";

        gtk_list_store_append (GTK_LIST_STORE (model), &iter);
        gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                            FILTER_LIST_ENABLE, FALSE,
                            FILTER_LIST_ICON, pixbuf,
                            FILTER_LIST_UUID, fiit->uuid.c_str (),
                            FILTER_LIST_NAME, fiit->name.c_str (),
                            FILTER_LIST_LANGS, langs.c_str (),
                            FILTER_LIST_DESC, fiit->desc.c_str (),
                            -1);

        if (pixbuf)
            g_object_unref (pixbuf);
    }
}

static void
get_filter_list_view_result (GtkTreeView *view, std::vector <String> &result, std::vector <String> &names)
{
    gchar               *uuid;
    gchar               *name;
    gboolean             enable;
    GtkTreeIter          iter;
    GtkTreeModel        *model = gtk_tree_view_get_model (view);

    result.clear ();
    names.clear ();

    if (!gtk_tree_model_get_iter_first (model, &iter)) return;

    do {
        gtk_tree_model_get (model, &iter,
                            FILTER_LIST_UUID, &uuid,
                            FILTER_LIST_NAME, &name,
                            FILTER_LIST_ENABLE, &enable,
                            -1);

        if (enable && uuid) {
            result.push_back (String (uuid));
            names.push_back (String (name));
        }

        if (uuid) g_free (uuid);
        if (name) g_free (name);
    } while (gtk_tree_model_iter_next (model, &iter));
}

static void
on_filter_button_clicked (GtkButton *button, gpointer user_data)
{
    gchar *uuid;
    gchar *filter_uuids;
    gchar *name;

    gtk_tree_model_get (GTK_TREE_MODEL (__factory_list_model), &__selected_factory,
                        FACTORY_LIST_UUID, &uuid,
                        FACTORY_LIST_FILTER_UUIDS, &filter_uuids,
                        FACTORY_LIST_NAME, &name,
                        -1);

    if (uuid) {
        GtkWidget *dialog;
        GtkWidget *view;
        GtkWidget *scrolledwindow;
        GtkWidget *separator;
        GtkWidget *hbox;
        GtkWidget *button;

        std::vector <String> enabled_filters;

        if (filter_uuids)
            scim_split_string_list (enabled_filters, filter_uuids);

        char buf [256];
        snprintf (buf, 256, _("Select Filters for %s"), name);

        dialog = gtk_dialog_new_with_buttons (buf,
                                              NULL,
                                              GTK_DIALOG_MODAL,
                                              _("_OK"), GTK_RESPONSE_OK,
                                              _("_Cancel"), GTK_RESPONSE_CANCEL,
                                              NULL);

#if GTK_CHECK_VERSION(2, 22, 0)
#else
        gtk_dialog_set_has_separator (GTK_DIALOG (dialog), TRUE);
#endif

        scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
        gtk_widget_show (scrolledwindow);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
                                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow),
                                             GTK_SHADOW_NONE);

#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), scrolledwindow, TRUE, TRUE, 2);
#else
        gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox), scrolledwindow, TRUE, TRUE, 2);
#endif

        view = create_filter_list_view ();
        set_filter_list_view_content (GTK_TREE_VIEW (view), __filter_infos, enabled_filters);

        gtk_container_add (GTK_CONTAINER (scrolledwindow), view);

#if GTK_CHECK_VERSION(3, 2, 0)
        separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
#else
        separator = gtk_hseparator_new ();
#endif
        gtk_widget_show (separator);
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), separator, FALSE, FALSE, 2);
#else
        gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox), separator, FALSE, FALSE, 2);
#endif

#if GTK_CHECK_VERSION(3, 2, 0)
        hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
#else
        hbox = gtk_hbox_new (FALSE, 4);
#endif
        gtk_widget_show (hbox);
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), hbox, FALSE, FALSE, 2);
#else
        gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox), hbox, FALSE, FALSE, 2);
#endif

        button = gtk_button_new_with_mnemonic (_("Move _Up"));
        gtk_widget_show (button);
        gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 4);
        g_signal_connect ((gpointer) button, "clicked",
                          G_CALLBACK (on_filter_move_up_button_clicked),
                          (gpointer) view);

        button = gtk_button_new_with_mnemonic (_("Move _Down"));
        gtk_widget_show (button);
        gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 4);
        g_signal_connect ((gpointer) button, "clicked",
                          G_CALLBACK (on_filter_move_down_button_clicked),
                          (gpointer) view);

        gtk_window_set_default_size (GTK_WINDOW (dialog), 320, 240);

        gint result = gtk_dialog_run (GTK_DIALOG (dialog));

        if (result == GTK_RESPONSE_OK) {
            std::vector <String>  filter_names;

            get_filter_list_view_result (GTK_TREE_VIEW (view), enabled_filters, filter_names);

            String str = scim_combine_string_list (enabled_filters);

            if (String (filter_uuids ? filter_uuids : "") != str) {
                gtk_tree_store_set (GTK_TREE_STORE (__factory_list_model), &__selected_factory,
                                    FACTORY_LIST_FILTER_NAMES, scim_combine_string_list (filter_names).c_str (),
                                    FACTORY_LIST_FILTER_UUIDS, str.c_str (),
                                    -1);
                __have_changed = true;
            }
        }

        gtk_widget_destroy (dialog);
    }

    if (uuid) g_free (uuid);
    if (filter_uuids) g_free (filter_uuids);
    if (name) g_free (name);
}

static void
on_filter_move_up_button_clicked (GtkButton *button, gpointer user_data)
{
    GtkTreeView *view = GTK_TREE_VIEW (user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection (view);
    GtkTreeModel *model;
    GtkTreeIter cur_iter;
    GtkTreeIter prev_iter;
    GtkTreePath *path;

    if (gtk_tree_selection_get_selected (selection, &model, &cur_iter)) {
        path = gtk_tree_model_get_path (model, &cur_iter);
        if (gtk_tree_path_prev (path) && gtk_tree_model_get_iter (model, &prev_iter, path)) {
            gtk_list_store_swap (GTK_LIST_STORE (model), &cur_iter, &prev_iter);
        }
        gtk_tree_path_free (path);
    }
}

static void
on_filter_move_down_button_clicked (GtkButton *button, gpointer user_data)
{
    GtkTreeView *view = GTK_TREE_VIEW (user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection (view);
    GtkTreeModel *model;
    GtkTreeIter cur_iter;
    GtkTreeIter next_iter;

    if (gtk_tree_selection_get_selected (selection, &model, &cur_iter)) {
        next_iter = cur_iter;
        if (gtk_tree_model_iter_next (model, &next_iter)) {
            gtk_list_store_swap (GTK_LIST_STORE (model), &cur_iter, &next_iter);
        }
    }
}
/*
vi:ts=4:nowrap:expandtab
*/

