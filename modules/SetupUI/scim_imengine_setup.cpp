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
#include <functional>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

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
// One row of the engine list, which is a two-level tree: a language, its engines
// beneath it. GtkColumnView takes a GListModel of objects rather than a
// GtkTreeStore, and a GtkTreeExpander in the name column does the nesting -- a
// real widget, so the arrow's clickable area is its own allocation instead of a
// rectangle the view hit-tests by hand.
//
// Whatever the UI writes back is a property, so a binding keeps cell and row in
// step both ways; the rest is read once when a row is bound.
#define FACTORY_TYPE_ITEM (factory_item_get_type ())
G_DECLARE_FINAL_TYPE (FactoryItem, factory_item, FACTORY, ITEM, GObject)

struct _FactoryItem
{
    GObject      parent_instance;

    gboolean     enable;
    gboolean     inconsistent;
    gchar       *hotkeys;
    gchar       *filter_names;
    gchar       *filter_uuids;

    gchar       *name;
    gchar       *uuid;          // null on a language row
    GdkTexture  *icon;
    GListStore  *children;      // null on an engine row
};

G_DEFINE_TYPE (FactoryItem, factory_item, G_TYPE_OBJECT)

enum
{
    FACTORY_ITEM_PROP_0,
    FACTORY_ITEM_PROP_ENABLE,
    FACTORY_ITEM_PROP_INCONSISTENT,
    FACTORY_ITEM_PROP_HOTKEYS,
    FACTORY_ITEM_PROP_FILTER_NAMES,
    FACTORY_ITEM_PROP_FILTER_UUIDS,
    FACTORY_ITEM_N_PROPS
};

static GParamSpec *__factory_item_props [FACTORY_ITEM_N_PROPS];

static void
factory_item_get_property (GObject *object, guint id, GValue *value, GParamSpec *pspec)
{
    FactoryItem *self = FACTORY_ITEM (object);

    switch (id) {
    case FACTORY_ITEM_PROP_ENABLE:        g_value_set_boolean (value, self->enable); break;
    case FACTORY_ITEM_PROP_INCONSISTENT:  g_value_set_boolean (value, self->inconsistent); break;
    case FACTORY_ITEM_PROP_HOTKEYS:       g_value_set_string (value, self->hotkeys); break;
    case FACTORY_ITEM_PROP_FILTER_NAMES:  g_value_set_string (value, self->filter_names); break;
    case FACTORY_ITEM_PROP_FILTER_UUIDS:  g_value_set_string (value, self->filter_uuids); break;
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, id, pspec);
    }
}

static void
factory_item_set_property (GObject *object, guint id, const GValue *value, GParamSpec *pspec)
{
    FactoryItem *self = FACTORY_ITEM (object);

    switch (id) {
    case FACTORY_ITEM_PROP_ENABLE:        self->enable = g_value_get_boolean (value); break;
    case FACTORY_ITEM_PROP_INCONSISTENT:  self->inconsistent = g_value_get_boolean (value); break;
    case FACTORY_ITEM_PROP_HOTKEYS:
        g_free (self->hotkeys);       self->hotkeys = g_value_dup_string (value); break;
    case FACTORY_ITEM_PROP_FILTER_NAMES:
        g_free (self->filter_names);  self->filter_names = g_value_dup_string (value); break;
    case FACTORY_ITEM_PROP_FILTER_UUIDS:
        g_free (self->filter_uuids);  self->filter_uuids = g_value_dup_string (value); break;
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, id, pspec);
    }
}

static void
factory_item_finalize (GObject *object)
{
    FactoryItem *self = FACTORY_ITEM (object);

    g_free (self->hotkeys);
    g_free (self->filter_names);
    g_free (self->filter_uuids);
    g_free (self->name);
    g_free (self->uuid);
    g_clear_object (&self->icon);
    g_clear_object (&self->children);

    G_OBJECT_CLASS (factory_item_parent_class)->finalize (object);
}

static void
factory_item_class_init (FactoryItemClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = factory_item_get_property;
    object_class->set_property = factory_item_set_property;
    object_class->finalize     = factory_item_finalize;

    __factory_item_props [FACTORY_ITEM_PROP_ENABLE] =
        g_param_spec_boolean ("enable", NULL, NULL, TRUE, G_PARAM_READWRITE);
    __factory_item_props [FACTORY_ITEM_PROP_INCONSISTENT] =
        g_param_spec_boolean ("inconsistent", NULL, NULL, FALSE, G_PARAM_READWRITE);
    __factory_item_props [FACTORY_ITEM_PROP_HOTKEYS] =
        g_param_spec_string ("hotkeys", NULL, NULL, NULL, G_PARAM_READWRITE);
    __factory_item_props [FACTORY_ITEM_PROP_FILTER_NAMES] =
        g_param_spec_string ("filter-names", NULL, NULL, NULL, G_PARAM_READWRITE);
    __factory_item_props [FACTORY_ITEM_PROP_FILTER_UUIDS] =
        g_param_spec_string ("filter-uuids", NULL, NULL, NULL, G_PARAM_READWRITE);

    g_object_class_install_properties (object_class, FACTORY_ITEM_N_PROPS, __factory_item_props);
}

static void
factory_item_init (FactoryItem *self)
{
    self->enable = TRUE;
}

static void
factory_item_enable_notify (GObject *object, GParamSpec *pspec, gpointer data);

static FactoryItem *
factory_item_new (const char *name, const char *uuid, GdkTexture *icon, gboolean is_group)
{
    FactoryItem *item = FACTORY_ITEM (g_object_new (FACTORY_TYPE_ITEM, NULL));

    item->name     = g_strdup (name);
    item->uuid     = uuid ? g_strdup (uuid) : NULL;
    item->icon     = icon;                                      // takes the reference
    item->children = is_group ? g_list_store_new (FACTORY_TYPE_ITEM) : NULL;

    // The checkbox writes into the row, not the other way round, so the row is
    // where the parent/child bookkeeping has to hang.
    g_signal_connect (item, "notify::enable",
                      G_CALLBACK (factory_item_enable_notify), NULL);

    return item;
}

// A row of the filter list, which is flat: no expander, and order matters --
// the filters run in the sequence shown, which the Up/Down buttons rearrange.
// Only "enable" is written back from the cell, so only it needs to be a
// property.
#define FILTER_TYPE_ITEM (filter_item_get_type ())
G_DECLARE_FINAL_TYPE (FilterItem, filter_item, FILTER, ITEM, GObject)

struct _FilterItem
{
    GObject      parent_instance;

    gboolean     enable;

    gchar       *uuid;
    gchar       *name;
    gchar       *langs;
    gchar       *desc;
    GdkTexture  *icon;
};

G_DEFINE_TYPE (FilterItem, filter_item, G_TYPE_OBJECT)

enum
{
    FILTER_ITEM_PROP_0,
    FILTER_ITEM_PROP_ENABLE,
    FILTER_ITEM_N_PROPS
};

static GParamSpec *__filter_item_props [FILTER_ITEM_N_PROPS];

static void
filter_item_get_property (GObject *object, guint id, GValue *value, GParamSpec *pspec)
{
    FilterItem *self = FILTER_ITEM (object);

    switch (id) {
    case FILTER_ITEM_PROP_ENABLE: g_value_set_boolean (value, self->enable); break;
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, id, pspec);
    }
}

static void
filter_item_set_property (GObject *object, guint id, const GValue *value, GParamSpec *pspec)
{
    FilterItem *self = FILTER_ITEM (object);

    switch (id) {
    case FILTER_ITEM_PROP_ENABLE: self->enable = g_value_get_boolean (value); break;
    default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, id, pspec);
    }
}

static void
filter_item_finalize (GObject *object)
{
    FilterItem *self = FILTER_ITEM (object);

    g_free (self->uuid);
    g_free (self->name);
    g_free (self->langs);
    g_free (self->desc);
    g_clear_object (&self->icon);

    G_OBJECT_CLASS (filter_item_parent_class)->finalize (object);
}

static void
filter_item_class_init (FilterItemClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = filter_item_get_property;
    object_class->set_property = filter_item_set_property;
    object_class->finalize     = filter_item_finalize;

    __filter_item_props [FILTER_ITEM_PROP_ENABLE] =
        g_param_spec_boolean ("enable", NULL, NULL, FALSE, G_PARAM_READWRITE);

    g_object_class_install_properties (object_class, FILTER_ITEM_N_PROPS, __filter_item_props);
}

static void
filter_item_init (FilterItem * /*self*/)
{
}

typedef scim_map <String, std::vector <size_t> >                          MapStringVectorSizeT;
typedef scim_map <String, KeyEventList>                                   MapStringKeyEventList;
typedef scim_map <String, std::vector <FilterInfo> >                       MapStringVectorFilterInfo;

// Internal data declaration.
static bool           __have_changed         = false;

#if GTK_CHECK_VERSION(2, 12, 0)
#else
static GtkTooltips   *__widget_tooltips      = 0;
#endif
static GListStore       *__factory_list_model = 0;  // the language rows
static GtkTreeListModel *__factory_tree       = 0;  // those, flattened as they expand
static GtkWidget        *__hotkey_button      = 0;
static GtkWidget        *__filter_button      = 0;

static FactoryItem      *__selected_factory   = 0;  // borrowed from the selection

// Set while the code, rather than the user, is moving checkboxes about, so the
// notify handler neither recurses nor reports a change the user did not make.
static bool              __updating_factories = false;

static std::vector <FilterInfo> __filter_infos;

// Internal functions declaration.
static GdkPixbuf *
scale_pixbuf (GdkPixbuf **pixbuf, int width, int height);

static void
factory_list_update_inconsistent (void);

// Walk every engine row, skipping the language rows above them. The old code
// used gtk_tree_model_foreach () for this and had to test each row for a uuid to
// tell the two apart; here the nesting is explicit.
static void
factory_list_foreach_engine (const std::function <void (FactoryItem *)> &fn)
{
    if (!__factory_list_model) return;

    guint groups = g_list_model_get_n_items (G_LIST_MODEL (__factory_list_model));

    for (guint g = 0; g < groups; ++g) {
        FactoryItem *group =
            FACTORY_ITEM (g_list_model_get_item (G_LIST_MODEL (__factory_list_model), g));
        if (group->children) {
            guint n = g_list_model_get_n_items (G_LIST_MODEL (group->children));
            for (guint i = 0; i < n; ++i) {
                FactoryItem *item =
                    FACTORY_ITEM (g_list_model_get_item (G_LIST_MODEL (group->children), i));
                fn (item);
                g_object_unref (item);
            }
        }
        g_object_unref (group);
    }
}

static void
factory_list_foreach_group (const std::function <void (FactoryItem *)> &fn)
{
    if (!__factory_list_model) return;

    guint groups = g_list_model_get_n_items (G_LIST_MODEL (__factory_list_model));

    for (guint g = 0; g < groups; ++g) {
        FactoryItem *group =
            FACTORY_ITEM (g_list_model_get_item (G_LIST_MODEL (__factory_list_model), g));
        fn (group);
        g_object_unref (group);
    }
}

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
set_filter_list_view_content (GtkWidget *view, const std::vector <FilterInfo> & infos, const std::vector <String> &enabled_filters);

static void
get_filter_list_view_result (GtkWidget *view, std::vector <String> &result, std::vector <String> &names);

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
factory_list_selection_changed_callback (GObject *selection, GParamSpec *pspec, gpointer user_data);

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

        // Create the toplevel box.
        window = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

        label = gtk_label_new (_("The installed input method services:"));
        gtk_widget_set_halign (label, GTK_ALIGN_START);
        gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
        gtk_box_append (GTK_BOX (window), label);

        scrolledwindow = gtk_scrolled_window_new ();
        gtk_widget_set_vexpand (scrolledwindow, TRUE);
        gtk_box_append (GTK_BOX (window), scrolledwindow);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
                                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

        // Create hotkey and filter button before factory list view, because
        // factory_list_selection_changed_callback may access these two buttons.
        __hotkey_button = gtk_button_new_with_mnemonic (_("Edit _Hotkeys"));
        gtk_widget_set_sensitive (__hotkey_button, FALSE);

        g_signal_connect ((gpointer) __hotkey_button, "clicked",
                          G_CALLBACK (on_hotkey_button_clicked),
                          NULL);

        gtk_widget_set_tooltip_text (__hotkey_button,
                              _("Edit Hotkeys associated with the selected input method."));

        __filter_button = gtk_button_new_with_mnemonic (_("Select _Filters"));
        gtk_widget_set_sensitive (__filter_button, FALSE);

        g_signal_connect ((gpointer) __filter_button, "clicked",
                          G_CALLBACK (on_filter_button_clicked),
                          NULL);

        gtk_widget_set_tooltip_text (__filter_button,
                              _("Select the Filters which will be attached to this input method."));

        view = create_factory_list_view ();
        gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolledwindow), view);

        sep = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
        gtk_box_append (GTK_BOX (window), sep);

        hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_box_append (GTK_BOX (window), hbox);

        gtk_widget_set_hexpand (__hotkey_button, TRUE);
        gtk_widget_set_halign (__hotkey_button, GTK_ALIGN_END);
        gtk_box_append (GTK_BOX (hbox), __hotkey_button);
        gtk_box_append (GTK_BOX (hbox), __filter_button);

        button = gtk_button_new_with_mnemonic (_("_Expand"));
        gtk_box_append (GTK_BOX (hbox), button);

        g_signal_connect ((gpointer) button, "clicked",
                          G_CALLBACK (on_expand_button_clicked),
                          (gpointer) view);

        gtk_widget_set_tooltip_text (button,
                              _("Expand all language categories."));

        button = gtk_button_new_with_mnemonic (_("_Collapse"));
        gtk_box_append (GTK_BOX (hbox), button);

        g_signal_connect ((gpointer) button, "clicked",
                          G_CALLBACK (on_collapse_button_clicked),
                          (gpointer) view);

        gtk_widget_set_tooltip_text (button,
                              _("Collapse all language categories."));

        button = gtk_button_new_with_mnemonic (_("E_nable All"));
        gtk_box_append (GTK_BOX (hbox), button);

        g_signal_connect ((gpointer) button, "clicked",
                          G_CALLBACK (on_toggle_all_button_clicked),
                          (gpointer) (1));

        gtk_widget_set_tooltip_text (button,
                              _("Enable all input methods."));

        button = gtk_button_new_with_mnemonic (_("_Disable All"));
        gtk_box_append (GTK_BOX (hbox), button);

        g_signal_connect ((gpointer) button, "clicked",
                          G_CALLBACK (on_toggle_all_button_clicked),
                          (gpointer) (0));

        gtk_widget_set_tooltip_text (button,
                              _("Disable all input methods."));
    }
    return window;
}

// Cell factories for the engine list. Each builds its widget once in "setup"
// and, in "bind", points it at the row being shown -- property bindings for the
// values the user can change, so cell and row follow one another without the
// view having to be told to refresh.

static GListModel *
factory_item_child_model (gpointer item, gpointer /*data*/)
{
    FactoryItem *self = FACTORY_ITEM (item);

    // A null return marks a leaf; anything else is what the row expands into.
    return self->children ? G_LIST_MODEL (g_object_ref (self->children)) : NULL;
}

// The arrow alone is a small target. Let the whole name cell work too, so a
// language expands wherever it is clicked; the row comes from the expander
// itself, which is always the one currently bound.
static void
factory_name_clicked (GtkGestureClick *gesture, gint /*n_press*/,
                      gdouble /*x*/, gdouble /*y*/, gpointer /*data*/)
{
    GtkWidget *box      = gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (gesture));
    GtkWidget *expander = gtk_widget_get_parent (box);

    if (!GTK_IS_TREE_EXPANDER (expander))
        return;

    GtkTreeListRow *row = gtk_tree_expander_get_list_row (GTK_TREE_EXPANDER (expander));

    // Engine rows have nothing to expand; leave their clicks to the selection.
    if (row && gtk_tree_list_row_is_expandable (row))
        gtk_tree_list_row_set_expanded (row, !gtk_tree_list_row_get_expanded (row));
}

static void
factory_name_setup (GtkSignalListItemFactory * /*f*/, GtkListItem *cell, gpointer /*d*/)
{
    GtkWidget *expander = gtk_tree_expander_new ();
    GtkWidget *box      = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *image    = gtk_image_new ();
    GtkWidget *label    = gtk_label_new (NULL);

    gtk_image_set_pixel_size (GTK_IMAGE (image), LIST_ICON_SIZE);
    gtk_label_set_xalign (GTK_LABEL (label), 0.0);

    gtk_box_append (GTK_BOX (box), image);
    gtk_box_append (GTK_BOX (box), label);
    gtk_tree_expander_set_child (GTK_TREE_EXPANDER (expander), box);
    gtk_list_item_set_child (cell, expander);

    GtkGesture *click = gtk_gesture_click_new ();

    g_signal_connect (click, "released", G_CALLBACK (factory_name_clicked), NULL);
    gtk_widget_add_controller (box, GTK_EVENT_CONTROLLER (click));
}

static void
factory_name_bind (GtkSignalListItemFactory * /*f*/, GtkListItem *cell, gpointer /*d*/)
{
    GtkTreeListRow *row      = GTK_TREE_LIST_ROW (gtk_list_item_get_item (cell));
    FactoryItem    *item     = FACTORY_ITEM (gtk_tree_list_row_get_item (row));
    GtkWidget      *expander = gtk_list_item_get_child (cell);
    GtkWidget      *box      = gtk_tree_expander_get_child (GTK_TREE_EXPANDER (expander));
    GtkWidget      *image    = gtk_widget_get_first_child (box);
    GtkWidget      *label    = gtk_widget_get_last_child (box);

    // This is what makes the arrow behave: the expander is handed the row and
    // draws and hit-tests itself, indentation included.
    gtk_tree_expander_set_list_row (GTK_TREE_EXPANDER (expander), row);

    gtk_image_set_from_paintable (GTK_IMAGE (image), GDK_PAINTABLE (item->icon));
    gtk_widget_set_visible (image, item->icon != NULL);
    gtk_label_set_text (GTK_LABEL (label), item->name ? item->name : "");

    g_object_unref (item);
}

// Bindings a cell made in "bind" have to go when it is recycled, or the row it
// used to show keeps driving it.
static void
factory_cell_drop_bindings (GtkListItem *cell)
{
    static const char * const keys [] = { "scim-binding-1", "scim-binding-2" };

    for (size_t i = 0; i < G_N_ELEMENTS (keys); ++i) {
        GBinding *binding =
            static_cast <GBinding *> (g_object_get_data (G_OBJECT (cell), keys [i]));
        if (binding) {
            g_binding_unbind (binding);
            g_object_set_data (G_OBJECT (cell), keys [i], NULL);
        }
    }
}

static void
factory_cell_unbind (GtkSignalListItemFactory * /*f*/, GtkListItem *cell, gpointer /*d*/)
{
    factory_cell_drop_bindings (cell);
}

static void
factory_enable_setup (GtkSignalListItemFactory * /*f*/, GtkListItem *cell, gpointer /*d*/)
{
    GtkWidget *check = gtk_check_button_new ();

    gtk_widget_set_halign (check, GTK_ALIGN_CENTER);
    gtk_list_item_set_child (cell, check);
}

static void
factory_enable_bind (GtkSignalListItemFactory * /*f*/, GtkListItem *cell, gpointer /*d*/)
{
    GtkTreeListRow *row   = GTK_TREE_LIST_ROW (gtk_list_item_get_item (cell));
    FactoryItem    *item  = FACTORY_ITEM (gtk_tree_list_row_get_item (row));
    GtkWidget      *check = gtk_list_item_get_child (cell);

    factory_cell_drop_bindings (cell);

    // Bidirectional: a click writes straight back into the row, and the notify
    // handler turns that into the parent/child bookkeeping.
    g_object_set_data (G_OBJECT (cell), "scim-binding-1",
                       g_object_bind_property (item, "enable", check, "active",
                                               (GBindingFlags) (G_BINDING_BIDIRECTIONAL |
                                                                G_BINDING_SYNC_CREATE)));
    g_object_set_data (G_OBJECT (cell), "scim-binding-2",
                       g_object_bind_property (item, "inconsistent", check, "inconsistent",
                                               G_BINDING_SYNC_CREATE));

    g_object_unref (item);
}

static void
factory_text_setup (GtkSignalListItemFactory * /*f*/, GtkListItem *cell, gpointer /*d*/)
{
    GtkWidget *label = gtk_label_new (NULL);

    gtk_label_set_xalign (GTK_LABEL (label), 0.0);
    gtk_list_item_set_child (cell, label);
}

static void
factory_text_bind (GtkSignalListItemFactory * /*f*/, GtkListItem *cell, gpointer data)
{
    GtkTreeListRow *row   = GTK_TREE_LIST_ROW (gtk_list_item_get_item (cell));
    FactoryItem    *item  = FACTORY_ITEM (gtk_tree_list_row_get_item (row));
    GtkWidget      *label = gtk_list_item_get_child (cell);

    factory_cell_drop_bindings (cell);

    g_object_set_data (G_OBJECT (cell), "scim-binding-1",
                       g_object_bind_property (item, (const char *) data, label, "label",
                                               G_BINDING_SYNC_CREATE));

    g_object_unref (item);
}

static GtkColumnViewColumn *
factory_append_column (GtkColumnView *view, const char *title,
                       GCallback setup, GCallback bind, gpointer data,
                       gboolean expand)
{
    GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();

    g_signal_connect (factory, "setup", setup, data);
    g_signal_connect (factory, "bind", bind, data);
    g_signal_connect (factory, "unbind", G_CALLBACK (factory_cell_unbind), NULL);

    GtkColumnViewColumn *column = gtk_column_view_column_new (title, factory);

    gtk_column_view_column_set_resizable (column, TRUE);
    gtk_column_view_column_set_expand (column, expand);
    gtk_column_view_append_column (view, column);
    g_object_unref (column);

    return column;      // the view holds it now
}

static GtkWidget *
create_factory_list_view ()
{
    // Language rows at the root, each holding its own store of engines.
    __factory_list_model = g_list_store_new (FACTORY_TYPE_ITEM);

    __factory_tree = gtk_tree_list_model_new (G_LIST_MODEL (__factory_list_model),
                                              FALSE,   // rows are GtkTreeListRow, not the items
                                              FALSE,   // start collapsed, as the old view did
                                              factory_item_child_model, NULL, NULL);

    GtkSingleSelection *selection =
        gtk_single_selection_new (G_LIST_MODEL (__factory_tree));

    gtk_single_selection_set_autoselect (selection, FALSE);
    gtk_single_selection_set_can_unselect (selection, TRUE);

    GtkWidget *view = gtk_column_view_new (GTK_SELECTION_MODEL (selection));

    factory_append_column (GTK_COLUMN_VIEW (view), _("Name"),
                           G_CALLBACK (factory_name_setup),
                           G_CALLBACK (factory_name_bind), NULL, TRUE);
    factory_append_column (GTK_COLUMN_VIEW (view), _("Enable"),
                           G_CALLBACK (factory_enable_setup),
                           G_CALLBACK (factory_enable_bind), NULL, FALSE);
    factory_append_column (GTK_COLUMN_VIEW (view), _("Hotkeys"),
                           G_CALLBACK (factory_text_setup),
                           G_CALLBACK (factory_text_bind),
                           (gpointer) "hotkeys", FALSE);
    factory_append_column (GTK_COLUMN_VIEW (view), _("Filters"),
                           G_CALLBACK (factory_text_setup),
                           G_CALLBACK (factory_text_bind),
                           (gpointer) "filter-names", FALSE);

    g_signal_connect (selection, "notify::selected-item",
                      G_CALLBACK (factory_list_selection_changed_callback), NULL);

    return view;
}

// The filter list is flat, so it needs no expander -- but its text can be long
// (a description is a sentence, and a filter can name a dozen languages), so the
// cells wrap rather than run off the edge and rows take whatever height they
// need. Anything beside wrapped text aligns to the top, to sit with the first
// line rather than float against the middle.
static void
filter_wrap_label (GtkWidget *label)
{
    gtk_label_set_xalign (GTK_LABEL (label), 0.0);
    gtk_label_set_yalign (GTK_LABEL (label), 0.0);
    gtk_label_set_wrap (GTK_LABEL (label), TRUE);
    gtk_label_set_wrap_mode (GTK_LABEL (label), PANGO_WRAP_WORD_CHAR);

    gtk_label_set_natural_wrap_mode (GTK_LABEL (label), GTK_NATURAL_WRAP_WORD);

    // What actually makes the text wrap is the column's width, set where the
    // columns are built. A GtkColumnView sizes a column from the natural width
    // of its cells, and a label's natural width is its full unwrapped length --
    // so left alone the column grows to fit the longest line and nothing wraps,
    // whatever wrap mode the label is in.
    gtk_widget_set_hexpand (label, TRUE);
}

static void
filter_enable_setup (GtkSignalListItemFactory * /*f*/, GtkListItem *cell, gpointer /*d*/)
{
    GtkWidget *check = gtk_check_button_new ();

    gtk_widget_set_halign (check, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (check, GTK_ALIGN_START);
    gtk_list_item_set_child (cell, check);
}

static void
filter_enable_bind (GtkSignalListItemFactory * /*f*/, GtkListItem *cell, gpointer /*d*/)
{
    FilterItem *item  = FILTER_ITEM (gtk_list_item_get_item (cell));
    GtkWidget  *check = gtk_list_item_get_child (cell);

    factory_cell_drop_bindings (cell);

    // Bidirectional, so the click lands in the row; the dialog reads the rows
    // back when it is accepted.
    g_object_set_data (G_OBJECT (cell), "scim-binding-1",
                       g_object_bind_property (item, "enable", check, "active",
                                               (GBindingFlags) (G_BINDING_BIDIRECTIONAL |
                                                                G_BINDING_SYNC_CREATE)));
}

static void
filter_name_setup (GtkSignalListItemFactory * /*f*/, GtkListItem *cell, gpointer /*d*/)
{
    GtkWidget *box   = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *image = gtk_image_new ();
    GtkWidget *label = gtk_label_new (NULL);

    gtk_image_set_pixel_size (GTK_IMAGE (image), LIST_ICON_SIZE);
    gtk_widget_set_valign (image, GTK_ALIGN_START);
    filter_wrap_label (label);

    gtk_box_append (GTK_BOX (box), image);
    gtk_box_append (GTK_BOX (box), label);
    gtk_list_item_set_child (cell, box);
}

static void
filter_name_bind (GtkSignalListItemFactory * /*f*/, GtkListItem *cell, gpointer /*d*/)
{
    FilterItem *item  = FILTER_ITEM (gtk_list_item_get_item (cell));
    GtkWidget  *box   = gtk_list_item_get_child (cell);
    GtkWidget  *image = gtk_widget_get_first_child (box);
    GtkWidget  *label = gtk_widget_get_last_child (box);

    gtk_image_set_from_paintable (GTK_IMAGE (image), GDK_PAINTABLE (item->icon));
    gtk_widget_set_visible (image, item->icon != NULL);
    gtk_label_set_text (GTK_LABEL (label), item->name ? item->name : "");
}

static void
filter_text_setup (GtkSignalListItemFactory * /*f*/, GtkListItem *cell, gpointer /*d*/)
{
    GtkWidget *label = gtk_label_new (NULL);

    filter_wrap_label (label);
    gtk_list_item_set_child (cell, label);
}

// data names the field to show; neither is a property, so there is nothing to
// bind and the text is set outright.
static void
filter_text_bind (GtkSignalListItemFactory * /*f*/, GtkListItem *cell, gpointer data)
{
    FilterItem *item  = FILTER_ITEM (gtk_list_item_get_item (cell));
    GtkWidget  *label = gtk_list_item_get_child (cell);
    const char *which = (const char *) data;
    const char *text  = g_str_equal (which, "langs") ? item->langs : item->desc;

    gtk_label_set_text (GTK_LABEL (label), text ? text : "");
}

static GtkWidget *
create_filter_list_view ()
{
    GListStore *model = g_list_store_new (FILTER_TYPE_ITEM);

    // Single selection: the Up/Down buttons act on one row.
    GtkSingleSelection *selection = gtk_single_selection_new (G_LIST_MODEL (model));

    gtk_single_selection_set_autoselect (selection, FALSE);
    gtk_single_selection_set_can_unselect (selection, TRUE);

    GtkWidget *view = gtk_column_view_new (GTK_SELECTION_MODEL (selection));

    factory_append_column (GTK_COLUMN_VIEW (view), _("Enable"),
                           G_CALLBACK (filter_enable_setup),
                           G_CALLBACK (filter_enable_bind), NULL, FALSE);

    // The three text columns share what the 640-wide dialog has once the window
    // margins and the Enable column are taken off -- roughly 190 each. Set that
    // as their width so the cells are allocated it and wrap inside; expand hands
    // out whatever is left over, and more again if the dialog is resized.
    static const int FILTER_COLUMN_WIDTH = 190;

    GtkColumnViewColumn *column;

    column = factory_append_column (GTK_COLUMN_VIEW (view), _("Name"),
                                    G_CALLBACK (filter_name_setup),
                                    G_CALLBACK (filter_name_bind), NULL, TRUE);
    gtk_column_view_column_set_fixed_width (column, FILTER_COLUMN_WIDTH);

    column = factory_append_column (GTK_COLUMN_VIEW (view), _("Languages"),
                                    G_CALLBACK (filter_text_setup),
                                    G_CALLBACK (filter_text_bind), (gpointer) "langs", TRUE);
    gtk_column_view_column_set_fixed_width (column, FILTER_COLUMN_WIDTH);

    column = factory_append_column (GTK_COLUMN_VIEW (view), _("Description"),
                                    G_CALLBACK (filter_text_setup),
                                    G_CALLBACK (filter_text_bind), (gpointer) "desc", TRUE);
    gtk_column_view_column_set_fixed_width (column, FILTER_COLUMN_WIDTH);

    return view;
}

// The store behind a filter view, for the code that fills and reads it.
static GListStore *
filter_list_store (GtkWidget *view)
{
    GtkSelectionModel *selection = gtk_column_view_get_model (GTK_COLUMN_VIEW (view));

    return G_LIST_STORE (gtk_single_selection_get_model (GTK_SINGLE_SELECTION (selection)));
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

    get_factory_list (config, uuids, names, langs, icons);

    for (size_t i = 0; i < uuids.size (); ++i)
        groups [langs [i]].push_back (i);

    g_list_store_remove_all (__factory_list_model);

    // Put the languages in the order they read in. Iterating groups sorts by
    // language id ("zh_CN"), not by the name on screen -- and not at all if this
    // was built with --enable-unordered-map. Collate on the displayed name,
    // which is translated, so the order follows the user's locale rather than
    // raw UTF-8 bytes.
    std::vector <std::pair <String, String> > ordered;   // (collation key, language id)

    for (MapStringVectorSizeT::iterator it = groups.begin ();
         it != groups.end (); ++ it) {
        String  lang_name = scim_get_language_name (it->first);
        gchar  *key       = g_utf8_collate_key (lang_name.c_str (), -1);

        ordered.push_back (std::make_pair (String (key), it->first));
        g_free (key);
    }

    std::sort (ordered.begin (), ordered.end ());

    // Add language group
    for (size_t g = 0; g < ordered.size (); ++g) {
        const std::vector <size_t> &members = groups [ordered [g].second];

        String       lang_name = scim_get_language_name (ordered [g].second);
        FactoryItem *group     = factory_item_new (lang_name.c_str (), NULL, NULL, TRUE);

        // Add factories for this group
        for (size_t i = 0; i < members.size (); ++i) {
            // Turn the engine's icon file into a texture once, here: a row is
            // bound afresh every time it scrolls back into view.
            GdkTexture *texture = 0;
            GdkPixbuf  *pixbuf  =
                gdk_pixbuf_new_from_file (icons [members [i]].c_str (), NULL);

            if (pixbuf) {
                scale_pixbuf (&pixbuf, LIST_ICON_SIZE, LIST_ICON_SIZE);
                texture = gdk_texture_new_for_pixbuf (pixbuf);
                g_object_unref (pixbuf);
            }

            FactoryItem *item = factory_item_new (names [members [i]].c_str (),
                                                  uuids [members [i]].c_str (),
                                                  texture, FALSE);
            g_list_store_append (group->children, item);
            g_object_unref (item);
        }

        g_list_store_append (__factory_list_model, group);
        g_object_unref (group);
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

    factory_list_foreach_engine ([&] (FactoryItem *item) {
        MapStringKeyEventList::iterator it = hotkey_map.find (String (item->uuid));
        if (it != hotkey_map.end ()) {
            String str;
            scim_key_list_to_string (str, it->second);
            g_object_set (item, "hotkeys", str.c_str (), NULL);
        } else {
            g_object_set (item, "hotkeys", NULL, NULL);
        }
    });
}

static void
save_hotkey_settings (const ConfigPointer &config)
{
    // Save Hotkeys.
    IMEngineHotkeyMatcher hotkey_matcher;
    MapStringKeyEventList hotkey_map;

    factory_list_foreach_engine ([&] (FactoryItem *item) {
        if (!item->hotkeys) return;
        KeyEventList keylist;
        if (scim_string_to_key_list (keylist, String (item->hotkeys)))
            hotkey_map.insert (std::make_pair (String (item->uuid), keylist));
    });

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

    factory_list_foreach_engine ([&] (FactoryItem *item) {
        MapStringVectorFilterInfo::iterator it = filter_map.find (String (item->uuid));
        if (it != filter_map.end ()) {
            std::vector <String> fnames;
            std::vector <String> fuuids;
            for (size_t i = 0; i < it->second.size (); ++i) {
                fnames.push_back (it->second [i].name);
                fuuids.push_back (it->second [i].uuid);
            }
            g_object_set (item,
                          "filter-names", scim_combine_string_list (fnames).c_str (),
                          "filter-uuids", scim_combine_string_list (fuuids).c_str (),
                          NULL);
        } else {
            g_object_set (item, "filter-names", NULL, "filter-uuids", NULL, NULL);
        }
    });
}

static void
save_filter_settings (const ConfigPointer &config)
{
    FilterManager m_manager (config);

    MapStringVectorFilterInfo filter_map;

    factory_list_foreach_engine ([&] (FactoryItem *item) {
        if (!item->filter_uuids) return;
        std::vector <String> strvec;
        scim_split_string_list (strvec, String (item->filter_uuids));

        std::vector <FilterInfo> infovec;
        for (size_t i = 0; i < strvec.size (); ++i)
            infovec.push_back (FilterInfo (strvec [i]));

        if (infovec.size ())
            filter_map.insert (std::make_pair (String (item->uuid), infovec));
    });

    m_manager.clear_all_filter_settings ();

    for (MapStringVectorFilterInfo::iterator it = filter_map.begin (); it != filter_map.end (); ++it) {
        std::vector <String> filters;

        for (size_t i = 0; i < it->second.size (); ++i)
            filters.push_back (it->second [i].uuid);

        m_manager.set_filters_for_imengine (it->first, filters);
    }
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

        // The config names what is off; everything else is on.
        __updating_factories = true;
        factory_list_foreach_engine ([&] (FactoryItem *item) {
            gboolean off = std::binary_search (disabled.begin (), disabled.end (),
                                               String (item->uuid));
            g_object_set (item, "enable", off ? FALSE : TRUE, NULL);
        });
        __updating_factories = false;

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

        factory_list_foreach_engine ([&] (FactoryItem *item) {
            if (!item->enable)
                disabled.push_back (String (item->uuid));
        });

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

// A language row reflects its engines: on when most are on, tri-state while they
// disagree.
static void
factory_list_update_inconsistent (void)
{
    // Writing a group's state must not cascade back down onto its engines: that
    // is what the notify handler does for a click, and doing it here would
    // replace each engine's own setting with the group's majority.
    bool was = __updating_factories;
    __updating_factories = true;

    factory_list_foreach_group ([] (FactoryItem *group) {
        if (!group->children) return;

        guint total   = g_list_model_get_n_items (G_LIST_MODEL (group->children));
        guint enabled = 0;

        for (guint i = 0; i < total; ++i) {
            FactoryItem *child =
                FACTORY_ITEM (g_list_model_get_item (G_LIST_MODEL (group->children), i));
            if (child->enable) ++ enabled;
            g_object_unref (child);
        }

        g_object_set (group,
                      "enable",       (enabled && enabled >= ((total + 1) >> 1)) ? TRUE : FALSE,
                      "inconsistent", (enabled && enabled < total) ? TRUE : FALSE,
                      NULL);
    });

    __updating_factories = was;
}

// Every checkbox writes into its row through a bidirectional binding, so this is
// where a click lands. A language row drags its engines with it; an engine row
// makes its language recompute.
static void
factory_item_enable_notify (GObject *object, GParamSpec * /*pspec*/, gpointer /*data*/)
{
    if (__updating_factories) return;

    FactoryItem *item = FACTORY_ITEM (object);

    if (item->children) {
        // A language row carries every engine under it along.
        __updating_factories = true;
        guint n = g_list_model_get_n_items (G_LIST_MODEL (item->children));
        for (guint i = 0; i < n; ++i) {
            FactoryItem *child =
                FACTORY_ITEM (g_list_model_get_item (G_LIST_MODEL (item->children), i));
            g_object_set (child, "enable", item->enable, NULL);
            g_object_unref (child);
        }
        g_object_set (item, "inconsistent", FALSE, NULL);
        __updating_factories = false;
    } else {
        // An engine row: let its language work out its own state.
        factory_list_update_inconsistent ();
    }

    __have_changed = true;
}

struct HotkeyDialogData {
    gchar *old_hotkeys;
};

static void
hotkey_dialog_response_cb (ScimKeySelectionDialog *dialog, gint response,
                           gpointer user_data)
{
    HotkeyDialogData *d = static_cast<HotkeyDialogData *> (user_data);

    if (response == SCIM_KEY_SELECTION_RESPONSE_OK) {
        const gchar *newkeys = scim_key_selection_dialog_get_keys (SCIM_KEY_SELECTION_DIALOG (dialog));
        const gchar *hotkeys = d->old_hotkeys;

        if ((newkeys && hotkeys && String (newkeys) != String (hotkeys)) || (newkeys || hotkeys)) {
            g_object_set (__selected_factory, "hotkeys", newkeys, NULL);
            __have_changed = true;
        }
    }

    g_free (d->old_hotkeys);
    delete d;
    gtk_window_destroy (GTK_WINDOW (dialog));
}

static void
on_hotkey_button_clicked (GtkButton *button, gpointer /* user_data */)
{
    if (!__selected_factory) return;

    const gchar *uuid    = __selected_factory->uuid;
    const gchar *hotkeys = __selected_factory->hotkeys;
    const gchar *name    = __selected_factory->name;

    if (uuid) {
        char buf [256];
        snprintf (buf, 256, _("Edit Hotkeys for %s"), name);

        GtkWidget *dialog = scim_key_selection_dialog_new (buf);
        GtkRoot   *root = gtk_widget_get_root (GTK_WIDGET (button));

        if (hotkeys) {
            scim_key_selection_dialog_set_keys (
                SCIM_KEY_SELECTION_DIALOG (dialog),
                hotkeys);
        }

        HotkeyDialogData *d = new HotkeyDialogData;
        d->old_hotkeys = hotkeys ? g_strdup (hotkeys) : NULL;

        if (root && GTK_IS_WINDOW (root))
            gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (root));
        gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

        g_signal_connect (dialog, "response", G_CALLBACK (hotkey_dialog_response_cb), d);

        gtk_window_present (GTK_WINDOW (dialog));
    }

    // uuid/hotkeys/name are the row's own strings, not copies: nothing to free.
}

// Expanding inserts rows just after the one expanded, so re-reading the count
// each time round is enough to reach every language row exactly once.
static void
factory_list_set_all_expanded (gboolean expanded)
{
    if (!__factory_tree) return;

    for (guint i = 0; i < g_list_model_get_n_items (G_LIST_MODEL (__factory_tree)); ++i) {
        GtkTreeListRow *row =
            GTK_TREE_LIST_ROW (g_list_model_get_item (G_LIST_MODEL (__factory_tree), i));
        if (gtk_tree_list_row_get_depth (row) == 0)
            gtk_tree_list_row_set_expanded (row, expanded);
        g_object_unref (row);
    }
}

static void
on_expand_button_clicked (GtkButton * /*button*/, gpointer /*user_data*/)
{
    factory_list_set_all_expanded (TRUE);
}

static void
on_collapse_button_clicked (GtkButton * /*button*/, gpointer /*user_data*/)
{
    factory_list_set_all_expanded (FALSE);
}

static void
factory_list_selection_changed_callback (GObject *selection, GParamSpec * /*pspec*/,
                                         gpointer /*user_data*/)
{
    gpointer selected =
        gtk_single_selection_get_selected_item (GTK_SINGLE_SELECTION (selection));

    // The selection hands out the tree row; the engine is inside it. Borrowed,
    // so nothing to release: the selection holds it for as long as it is current.
    __selected_factory = 0;
    if (selected) {
        FactoryItem *item = FACTORY_ITEM (gtk_tree_list_row_get_item (GTK_TREE_LIST_ROW (selected)));
        __selected_factory = item;
        g_object_unref (item);
    }

    // Hotkeys and filters belong to an engine; a language row has neither.
    gboolean engine = __selected_factory && !__selected_factory->children;

    gtk_widget_set_sensitive (__hotkey_button, engine);
    gtk_widget_set_sensitive (__filter_button, engine && __filter_infos.size () != 0);
}

static void
on_toggle_all_button_clicked (GtkButton */* button */, gpointer user_data)
{
    gboolean enable = (user_data != 0);

    // One sweep over both levels, so no language row is left tri-state.
    __updating_factories = true;
    factory_list_foreach_engine ([&] (FactoryItem *item) {
        g_object_set (item, "enable", enable, NULL);
    });
    factory_list_foreach_group ([&] (FactoryItem *group) {
        g_object_set (group, "enable", enable, "inconsistent", FALSE, NULL);
    });
    __updating_factories = false;

    __have_changed = true;
}

// Build one row per filter, the enabled ones first and in the order they are
// applied; the rest follow, switched off.
static FilterItem *
filter_item_new (const FilterInfo &info, gboolean enable)
{
    FilterItem *item = FILTER_ITEM (g_object_new (FILTER_TYPE_ITEM, NULL));

    std::vector <String> lang_ids;
    std::vector <String> lang_names;

    scim_split_string_list (lang_ids, info.langs);

    for (std::vector <String>::const_iterator sit = lang_ids.begin ();
         sit != lang_ids.end (); ++sit) {
        String name = scim_get_language_name (*sit);
        if (std::find (lang_names.begin (), lang_names.end (), name) == lang_names.end ())
            lang_names.push_back (name);
    }

    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (info.icon.c_str (), NULL);

    if (pixbuf) {
        scale_pixbuf (&pixbuf, LIST_ICON_SIZE, LIST_ICON_SIZE);
        item->icon = gdk_texture_new_for_pixbuf (pixbuf);
        g_object_unref (pixbuf);
    }

    item->enable = enable;
    item->uuid   = g_strdup (info.uuid.c_str ());
    item->name   = g_strdup (info.name.c_str ());
    item->langs  = g_strdup (scim_combine_string_list (lang_names).c_str ());
    item->desc   = g_strdup (info.desc.c_str ());

    return item;
}

static void
set_filter_list_view_content (GtkWidget *view, const std::vector <FilterInfo> & infos, const std::vector <String> &enabled_filters)
{
    GListStore *model = filter_list_store (view);

    std::vector <FilterInfo> disabled_infos = infos;
    std::vector <FilterInfo> enabled_infos;

    std::vector <FilterInfo>::iterator fiit;
    std::vector <String>::const_iterator sit;

    g_list_store_remove_all (model);

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
        FilterItem *item = filter_item_new (*fiit, TRUE);
        g_list_store_append (model, item);
        g_object_unref (item);
    }

    for (fiit = disabled_infos.begin (); fiit != disabled_infos.end (); ++fiit) {
        FilterItem *item = filter_item_new (*fiit, FALSE);
        g_list_store_append (model, item);
        g_object_unref (item);
    }
}

static void
get_filter_list_view_result (GtkWidget *view, std::vector <String> &result, std::vector <String> &names)
{
    GListStore *model = filter_list_store (view);
    guint       n     = g_list_model_get_n_items (G_LIST_MODEL (model));

    result.clear ();
    names.clear ();

    // Row order is the order the filters run in, so read it straight through.
    for (guint i = 0; i < n; ++i) {
        FilterItem *item = FILTER_ITEM (g_list_model_get_item (G_LIST_MODEL (model), i));

        if (item->enable && item->uuid) {
            result.push_back (String (item->uuid));
            names.push_back (String (item->name ? item->name : ""));
        }

        g_object_unref (item);
    }
}

struct FilterDialogData {
    GtkWidget          *view;
    String              filter_uuids_old;
    std::vector<String> enabled_filters;
};

// GtkDialog's response protocol went with the widget; the two buttons say
// which they are through this flag instead.
static void
filter_dialog_finish (GtkWidget *dialog, gboolean accepted, gpointer user_data)
{
    FilterDialogData *d = static_cast<FilterDialogData *> (user_data);

    if (accepted) {
        std::vector <String> filter_names;

        get_filter_list_view_result (d->view, d->enabled_filters, filter_names);

        String str = scim_combine_string_list (d->enabled_filters);

        if (d->filter_uuids_old != str) {
            g_object_set (__selected_factory,
                          "filter-names", scim_combine_string_list (filter_names).c_str (),
                          "filter-uuids", str.c_str (),
                          NULL);
            __have_changed = true;
        }
    }

    // The dialog owns d; destroying it frees it, on this path and on the one
    // where the window is closed without touching either button.
    gtk_window_destroy (GTK_WINDOW (dialog));
}

static void
filter_dialog_data_free (gpointer data)
{
    delete static_cast<FilterDialogData *> (data);
}

static void
filter_dialog_ok_cb (GtkButton *button, gpointer user_data)
{
    filter_dialog_finish (GTK_WIDGET (gtk_widget_get_root (GTK_WIDGET (button))),
                          TRUE, user_data);
}

static void
filter_dialog_cancel_cb (GtkButton *button, gpointer user_data)
{
    filter_dialog_finish (GTK_WIDGET (gtk_widget_get_root (GTK_WIDGET (button))),
                          FALSE, user_data);
}

static void
on_filter_button_clicked (GtkButton */* button */, gpointer /* user_data */)
{
    if (!__selected_factory) return;

    const gchar *uuid         = __selected_factory->uuid;
    const gchar *filter_uuids = __selected_factory->filter_uuids;
    const gchar *name         = __selected_factory->name;

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

        dialog = gtk_window_new ();
        gtk_window_set_title (GTK_WINDOW (dialog), buf);
        gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

        // GtkWindow takes a single child, so the content and the buttons go in
        // a box of our own -- the shape GtkDialog used to supply.
        GtkWidget *content_area = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
        gtk_window_set_child (GTK_WINDOW (dialog), content_area);

        scrolledwindow = gtk_scrolled_window_new ();
        gtk_widget_set_vexpand (scrolledwindow, TRUE);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
                                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_box_append (GTK_BOX (content_area), scrolledwindow);

        view = create_filter_list_view ();
        set_filter_list_view_content (view, __filter_infos, enabled_filters);

        gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolledwindow), view);

        separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
        gtk_box_append (GTK_BOX (content_area), separator);

        hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
        gtk_box_append (GTK_BOX (content_area), hbox);

        button = gtk_button_new_with_mnemonic (_("Move _Up"));
        gtk_widget_set_hexpand (button, TRUE);
        gtk_widget_set_halign (button, GTK_ALIGN_END);
        gtk_box_append (GTK_BOX (hbox), button);
        g_signal_connect ((gpointer) button, "clicked",
                          G_CALLBACK (on_filter_move_up_button_clicked),
                          (gpointer) view);

        button = gtk_button_new_with_mnemonic (_("Move _Down"));
        gtk_box_append (GTK_BOX (hbox), button);
        g_signal_connect ((gpointer) button, "clicked",
                          G_CALLBACK (on_filter_move_down_button_clicked),
                          (gpointer) view);

        gtk_window_set_default_size (GTK_WINDOW (dialog), 640, 400);

        GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (__filter_button));
        if (root && GTK_IS_WINDOW (root))
            gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (root));

        FilterDialogData *d = new FilterDialogData;
        d->view             = view;
        d->filter_uuids_old = String (filter_uuids ? filter_uuids : "");
        d->enabled_filters  = enabled_filters;

        // Hang it on the dialog rather than on the buttons: the window can also
        // be closed without pressing either, and this way that frees it too.
        g_object_set_data_full (G_OBJECT (dialog), "scim-filter-dialog-data",
                                d, filter_dialog_data_free);

        // Escape dismisses it, as it did while this was a GtkDialog.
        GtkEventController *shortcuts = gtk_shortcut_controller_new ();
        gtk_shortcut_controller_add_shortcut (
            GTK_SHORTCUT_CONTROLLER (shortcuts),
            gtk_shortcut_new (gtk_keyval_trigger_new (GDK_KEY_Escape,
                                                      (GdkModifierType) 0),
                              gtk_named_action_new ("window.close")));
        gtk_widget_add_controller (dialog, shortcuts);

        // The action area GtkDialog used to add.
        GtkWidget *action_area = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
        gtk_widget_set_halign (action_area, GTK_ALIGN_END);
        gtk_widget_set_margin_start (action_area, 4);
        gtk_widget_set_margin_end (action_area, 4);
        gtk_widget_set_margin_top (action_area, 4);
        gtk_widget_set_margin_bottom (action_area, 4);
        gtk_box_append (GTK_BOX (content_area), action_area);

        GtkWidget *cancel = gtk_button_new_with_mnemonic (_("_Cancel"));
        gtk_box_append (GTK_BOX (action_area), cancel);
        g_signal_connect (cancel, "clicked", G_CALLBACK (filter_dialog_cancel_cb), d);

        GtkWidget *ok = gtk_button_new_with_mnemonic (_("_OK"));
        gtk_box_append (GTK_BOX (action_area), ok);
        g_signal_connect (ok, "clicked", G_CALLBACK (filter_dialog_ok_cb), d);

        gtk_window_set_default_widget (GTK_WINDOW (dialog), ok);

        gtk_window_present (GTK_WINDOW (dialog));
    }

    // uuid/filter_uuids/name are the row's own strings, not copies.
}

// Move the selected row, keeping it selected so the buttons can be used again
// without re-picking it.
static void
filter_list_move_selected (GtkWidget *view, int delta)
{
    GtkSelectionModel  *sel   = gtk_column_view_get_model (GTK_COLUMN_VIEW (view));
    GtkSingleSelection *single = GTK_SINGLE_SELECTION (sel);
    GListStore         *model = filter_list_store (view);

    guint pos = gtk_single_selection_get_selected (single);
    if (pos == GTK_INVALID_LIST_POSITION) return;

    guint n = g_list_model_get_n_items (G_LIST_MODEL (model));
    if ((delta < 0 && pos == 0) || (delta > 0 && pos + 1 >= n)) return;

    guint target = pos + delta;

    FilterItem *item = FILTER_ITEM (g_list_model_get_item (G_LIST_MODEL (model), pos));

    g_list_store_remove (model, pos);
    g_list_store_insert (model, target, item);
    g_object_unref (item);

    gtk_single_selection_set_selected (single, target);
}

static void
on_filter_move_up_button_clicked (GtkButton * /*button*/, gpointer user_data)
{
    filter_list_move_selected (GTK_WIDGET (user_data), -1);
}

static void
on_filter_move_down_button_clicked (GtkButton * /*button*/, gpointer user_data)
{
    filter_list_move_selected (GTK_WIDGET (user_data), 1);
}
/*
vi:ts=4:nowrap:expandtab
*/

