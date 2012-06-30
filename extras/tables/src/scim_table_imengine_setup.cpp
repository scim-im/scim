/** @file scim_table_imengine_setup.cpp
 * implementation of Setup Module of table imengine module.
 */

/*
 * Smart Common Input Method
 * 
 * Copyright (c) 2002-2005 James Su <suzhe@tsinghua.org.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: scim_table_imengine_setup.cpp,v 1.4 2005/10/26 07:53:53 suzhe Exp $
 *
 */

#define Uses_SCIM_CONFIG_BASE

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <gtk/scimkeyselection.h>
#include <scim.h>
#include "scim_generic_table.h"
#include "scim_table_private.h"

using namespace scim;

#define scim_module_init table_imengine_setup_LTX_scim_module_init
#define scim_module_exit table_imengine_setup_LTX_scim_module_exit

#define scim_setup_module_create_ui       table_imengine_setup_LTX_scim_setup_module_create_ui
#define scim_setup_module_get_category    table_imengine_setup_LTX_scim_setup_module_get_category
#define scim_setup_module_get_name        table_imengine_setup_LTX_scim_setup_module_get_name
#define scim_setup_module_get_description table_imengine_setup_LTX_scim_setup_module_get_description
#define scim_setup_module_load_config     table_imengine_setup_LTX_scim_setup_module_load_config
#define scim_setup_module_save_config     table_imengine_setup_LTX_scim_setup_module_save_config
#define scim_setup_module_query_changed   table_imengine_setup_LTX_scim_setup_module_query_changed


#define SCIM_CONFIG_IMENGINE_TABLE_FULL_WIDTH_PUNCT_KEY    "/IMEngine/Table/FullWidthPunctKey"
#define SCIM_CONFIG_IMENGINE_TABLE_FULL_WIDTH_LETTER_KEY   "/IMEngine/Table/FullWidthLetterKey"
#define SCIM_CONFIG_IMENGINE_TABLE_MODE_SWITCH_KEY         "/IMEngine/Table/ModeSwitchKey"
#define SCIM_CONFIG_IMENGINE_TABLE_ADD_PHRASE_KEY          "/IMEngine/Table/AddPhraseKey"
#define SCIM_CONFIG_IMENGINE_TABLE_DEL_PHRASE_KEY          "/IMEngine/Table/DeletePhraseKey"
#define SCIM_CONFIG_IMENGINE_TABLE_SHOW_PROMPT             "/IMEngine/Table/ShowPrompt"
#define SCIM_CONFIG_IMENGINE_TABLE_SHOW_KEY_HINT           "/IMEngine/Table/ShowKeyHint"
#define SCIM_CONFIG_IMENGINE_TABLE_USER_TABLE_BINARY       "/IMEngine/Table/UserTableBinary"
#define SCIM_CONFIG_IMENGINE_TABLE_USER_PHRASE_FIRST       "/IMEngine/Table/UserPhraseFirst"
#define SCIM_CONFIG_IMENGINE_TABLE_LONG_PHRASE_FIRST       "/IMEngine/Table/LongPhraseFirst"

#define SCIM_TABLE_ICON_FILE                              (SCIM_ICONDIR "/table.png")

#define LIST_ICON_SIZE 20 

static GtkWidget * create_setup_window ();
static void        load_config (const ConfigPointer &config);
static void        save_config (const ConfigPointer &config);
static bool        query_changed ();

static void        destroy_all_tables ();

// Module Interface.
extern "C" {
    void scim_module_init (void)
    {
        bindtextdomain (GETTEXT_PACKAGE, SCIM_TABLE_LOCALEDIR);
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    }

    void scim_module_exit (void)
    {
        destroy_all_tables ();
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
        return String (_("Generic Table"));
    }

    String scim_setup_module_get_description (void)
    {
        return String (_("An IMEngine Module which uses generic table input method file."));
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

enum
{
    TABLE_COLUMN_ICON = 0,
    TABLE_COLUMN_NAME,
    TABLE_COLUMN_LANG,
    TABLE_COLUMN_FILE,
    TABLE_COLUMN_TYPE,
    TABLE_COLUMN_LIBRARY,
    TABLE_COLUMN_IS_USER,
    TABLE_NUM_COLUMNS
};

struct TablePropertiesData
{
    String name;
    String author;
    String uuid;
    String serial;
    String icon;
    String languages;
    String status_prompt;
    String valid_input_chars;
    String multi_wildcard_chars;
    String single_wildcard_chars;
    String split_keys;
    String commit_keys;
    String forward_keys;
    String select_keys;
    String page_up_keys;
    String page_down_keys;
    int    max_key_length;
    bool   show_key_prompt;
    bool   auto_select;
    bool   auto_fill;
    bool   auto_wildcard;
    bool   auto_commit;
    bool   auto_split;
    bool   discard_invalid_key;
    bool   dynamic_adjust;
    bool   always_show_lookup;
    bool   def_full_width_punct;
    bool   def_full_width_letter;
};

// Internal data declaration.
static bool __config_show_prompt           = false;
static bool __config_show_key_hint         = false;
static bool __config_user_table_binary     = false;
static bool __config_user_phrase_first     = false;
static bool __config_long_phrase_first     = false;

static bool __have_changed                 = false;

static GtkWidget    * __widget_show_prompt           = 0;
static GtkWidget    * __widget_show_key_hint         = 0;
static GtkWidget    * __widget_user_table_binary     = 0;
static GtkWidget    * __widget_user_phrase_first     = 0;
static GtkWidget    * __widget_long_phrase_first     = 0;
#if GTK_CHECK_VERSION(3, 0, 0)
#else
static GtkTooltips  * __widget_tooltips              = 0;
#endif

static GtkWidget    * __widget_table_list_view       = 0;
static GtkListStore * __widget_table_list_model      = 0;

static GtkWidget    * __widget_table_install_button    = 0;
static GtkWidget    * __widget_table_delete_button     = 0;
static GtkWidget    * __widget_table_properties_button = 0;

static KeyboardConfigData __config_keyboards [] =
{
    {
        // key
        SCIM_CONFIG_IMENGINE_TABLE_FULL_WIDTH_PUNCT_KEY,
        // label
        N_("Full width _punctuation:"),
        // title
        N_("Select full width puncutation keys"),
        // tooltip
        N_("The key events to switch full/half width punctuation input mode. "
           "Click on the button on the right to edit it."),
        // entry
        NULL,
        // button
        NULL,
        // data
        "Control+period"
    },
    {
        // key
        SCIM_CONFIG_IMENGINE_TABLE_FULL_WIDTH_LETTER_KEY,
        // label
        N_("Full width _letter:"),
        // title
        N_("Select full width letter keys"),
        // tooltip
        N_("The key events to switch full/half width letter input mode. "
           "Click on the button on the right to edit it."),
        // entry
        NULL,
        // button
        NULL,
        // data
        "Shift+space"
    },
    {
        // key
        SCIM_CONFIG_IMENGINE_TABLE_MODE_SWITCH_KEY,
        // label
        N_("_Mode switch:"),
        // title
        N_("Select mode switch keys"),
        // tooltip
        N_("The key events to change current input mode. "
           "Click on the button on the right to edit it."),
        // entry
        NULL,
        // button
        NULL,
        // data
        "Alt+Shift_L+KeyRelease,"
        "Alt+Shift_R+KeyRelease,"
        "Shift+Shift_L+KeyRelease,"
        "Shift+Shift_R+KeyRelease"
    },
    {
        // key
        SCIM_CONFIG_IMENGINE_TABLE_ADD_PHRASE_KEY,
        // label
        N_("_Add phrase:"),
        // title
        N_("Select add phrase keys."),
        // tooltip
        N_("The key events to add a new user defined phrase. "
           "Click on the button on the right to edit it."),
        // entry
        NULL,
        // button
        NULL,
        // data
        "Control+a,"
        "Control+equal"
    },
    {
        // key
        SCIM_CONFIG_IMENGINE_TABLE_DEL_PHRASE_KEY,
        // label
        N_("_Delete phrase:"),
        // title
        N_("Select delete phrase keys."),
        // tooltip
        N_("The key events to delete a selected phrase. "
           "Click on the button on the right to edit it."),
        // entry
        NULL,
        // button
        NULL,
        // data
        "Control+d,"
        "Control+minus"
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
on_default_toggle_button_toggled     (GtkToggleButton *togglebutton,
                                      gpointer         user_data);

static void
on_default_key_selection_clicked     (GtkButton       *button,
                                      gpointer         user_data);

static void
on_icon_file_selection_clicked       (GtkButton       *button,
                                      gpointer         user_data);

static void
on_table_list_selection_changed      (GtkTreeSelection *selection,
                                      gpointer          user_data);

static void
on_table_install_clicked             (GtkButton       *button,
                                      gpointer         user_data);

static void
on_table_delete_clicked              (GtkButton       *button,
                                      gpointer         user_data);

static void
on_table_properties_clicked          (GtkButton       *button,
                                      gpointer         user_data);

static void
on_toggle_button_toggled             (GtkToggleButton *button,
                                      gpointer         user_data);

static gint
run_table_properties_dialog          (GenericTableLibrary *lib,
                                      TablePropertiesData &data,
                                      bool                 editable);

static bool
validate_table_properties_data       (const GenericTableLibrary *lib,
                                      const TablePropertiesData &data);

static GdkPixbuf *
scale_pixbuf                         (GdkPixbuf      **pixbuf,
                                      int              width,
                                      int              height);

static void
setup_widget_value ();

static GtkWidget *
create_generic_page ();

static GtkWidget *
create_keyboard_page ();

static GtkWidget *
create_table_management_page ();

static GtkListStore *
create_table_list_model ();

static void
get_table_list (std::vector<String> &table_list, const String &path);

static GenericTableLibrary *
load_table_file (const String &file);

static void
add_table_to_list (GenericTableLibrary *table, const String &dir, const String &file, bool user);

static void
delete_table_from_list (GtkTreeModel *model, GtkTreeIter *iter);

static void
load_all_tables ();

static void
save_all_tables ();

static bool
test_file_modify (const String &file);

static bool
test_file_unlink (const String &file);

// Function implementations.
static GtkWidget *
create_generic_page ()
{
    GtkWidget *vbox;

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);

    __widget_show_prompt = gtk_check_button_new_with_mnemonic (_("Show _prompt"));
    gtk_widget_show (__widget_show_prompt);
    gtk_box_pack_start (GTK_BOX (vbox), __widget_show_prompt, FALSE, FALSE, 4);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_show_prompt), 4);

    __widget_show_key_hint = gtk_check_button_new_with_mnemonic (_("Show key _hint"));
    gtk_widget_show (__widget_show_key_hint);
    gtk_box_pack_start (GTK_BOX (vbox), __widget_show_key_hint, FALSE, FALSE, 4);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_show_key_hint), 4);

    __widget_user_table_binary = gtk_check_button_new_with_mnemonic (_("Save _user table in binary format"));
    gtk_widget_show (__widget_user_table_binary);
    gtk_box_pack_start (GTK_BOX (vbox), __widget_user_table_binary, FALSE, FALSE, 4);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_user_table_binary), 4);

    __widget_user_phrase_first = gtk_check_button_new_with_mnemonic (_("Show the u_ser defined phrases first"));
    gtk_widget_show (__widget_user_phrase_first);
    gtk_box_pack_start (GTK_BOX (vbox), __widget_user_phrase_first, FALSE, FALSE, 4);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_user_phrase_first), 4);

    __widget_long_phrase_first = gtk_check_button_new_with_mnemonic (_("Show the _longer phrases first"));
    gtk_widget_show (__widget_long_phrase_first);
    gtk_box_pack_start (GTK_BOX (vbox), __widget_long_phrase_first, FALSE, FALSE, 4);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_long_phrase_first), 4);

    // Connect all signals.
    g_signal_connect ((gpointer) __widget_show_prompt, "toggled",
                      G_CALLBACK (on_default_toggle_button_toggled),
                      &__config_show_prompt);
    g_signal_connect ((gpointer) __widget_show_key_hint, "toggled",
                      G_CALLBACK (on_default_toggle_button_toggled),
                      &__config_show_key_hint);
    g_signal_connect ((gpointer) __widget_user_table_binary, "toggled",
                      G_CALLBACK (on_default_toggle_button_toggled),
                      &__config_user_table_binary);
    g_signal_connect ((gpointer) __widget_user_phrase_first, "toggled",
                      G_CALLBACK (on_default_toggle_button_toggled),
                      &__config_user_phrase_first);
    g_signal_connect ((gpointer) __widget_long_phrase_first, "toggled",
                      G_CALLBACK (on_default_toggle_button_toggled),
                      &__config_long_phrase_first);

    // Set all tooltips.
#if GTK_CHECK_VERSION(3, 0, 0)
    gtk_widget_set_tooltip_text (__widget_show_prompt,
                          _("If this option is checked, "
                            "the key prompt of the currently selected phrase "
                            "will be shown."));

    gtk_widget_set_tooltip_text (__widget_show_key_hint,
                          _("If this option is checked, "
                           "the remaining keystrokes of the phrases "
                            "will be shown on the lookup table."));

    gtk_widget_set_tooltip_text (__widget_user_table_binary,
                          _("If this option is checked, "
                            "the user table will be stored with binary format, "
                            "this will increase the loading speed."));

    gtk_widget_set_tooltip_text (__widget_user_phrase_first,
                          _("If this option is checked, "
                            "the user defined phrases will be shown "
                            "in front of others. "));

    gtk_widget_set_tooltip_text (__widget_long_phrase_first,
                          _("If this option is checked, "
                            "the longer phrase will be shown "
                            "in front of others. "));
#else
    gtk_tooltips_set_tip (__widget_tooltips, __widget_show_prompt,
                          _("If this option is checked, "
                            "the key prompt of the currently selected phrase "
                            "will be shown."), NULL);

    gtk_tooltips_set_tip (__widget_tooltips, __widget_show_key_hint,
                          _("If this option is checked, "
                           "the remaining keystrokes of the phrases "
                            "will be shown on the lookup table."), NULL);

    gtk_tooltips_set_tip (__widget_tooltips, __widget_user_table_binary,
                          _("If this option is checked, "
                            "the user table will be stored with binary format, "
                            "this will increase the loading speed."), NULL);

    gtk_tooltips_set_tip (__widget_tooltips, __widget_user_phrase_first,
                          _("If this option is checked, "
                            "the user defined phrases will be shown "
                            "in front of others. "), NULL);

    gtk_tooltips_set_tip (__widget_tooltips, __widget_long_phrase_first,
                          _("If this option is checked, "
                            "the longer phrase will be shown "
                            "in front of others. "), NULL);
#endif
    return vbox;
}

static GtkWidget *
create_keyboard_page ()
{
    GtkWidget *table;
    GtkWidget *label;

    int i;

    table = gtk_table_new (3, 3, FALSE);
    gtk_widget_show (table);

    // Create keyboard setting.
    for (i = 0; __config_keyboards [i].key; ++ i) {
        label = gtk_label_new (NULL);
        gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _(__config_keyboards[i].label));
        gtk_widget_show (label);
        gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
        gtk_misc_set_padding (GTK_MISC (label), 4, 0);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (GTK_FILL), 4, 4);

        __config_keyboards [i].entry = gtk_entry_new ();
        gtk_widget_show (__config_keyboards [i].entry);
        gtk_table_attach (GTK_TABLE (table), __config_keyboards [i].entry, 1, 2, i, i+1,
                          (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
                          (GtkAttachOptions) (GTK_FILL), 4, 4);
#if GTK_CHECK_VERSION(3, 0, 0)
        g_object_set (G_OBJECT (__config_keyboards[i].entry), "editable", FALSE, NULL);
#else
        gtk_entry_set_editable (GTK_ENTRY (__config_keyboards[i].entry), FALSE);
#endif

        __config_keyboards[i].button = gtk_button_new_with_label ("...");
        gtk_widget_show (__config_keyboards[i].button);
        gtk_table_attach (GTK_TABLE (table), __config_keyboards[i].button, 2, 3, i, i+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (GTK_FILL), 4, 4);
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
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_tooltip_text (__config_keyboards [i].entry,
                              _(__config_keyboards [i].tooltip));
#else
        gtk_tooltips_set_tip (__widget_tooltips, __config_keyboards [i].entry,
                              _(__config_keyboards [i].tooltip), NULL);
#endif
    }

    return table;
}

static GtkListStore *
create_table_list_model ()
{
    GtkListStore *model;

    model = gtk_list_store_new (TABLE_NUM_COLUMNS,
                                GDK_TYPE_PIXBUF,
                                G_TYPE_STRING,
                                G_TYPE_STRING,
                                G_TYPE_STRING,
                                G_TYPE_STRING,
                                G_TYPE_POINTER,
                                G_TYPE_BOOLEAN);

    return model;
}

static GtkWidget *
create_table_management_page ()
{
    GtkWidget *page;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *scrolledwindow;
    GtkWidget *treeview;
    GtkWidget *hbox;
    GtkWidget *button;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkTreeSelection  *selection;

    page = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (page);

    label = gtk_label_new (_("The installed tables:"));
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (page), label, FALSE, FALSE, 2);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_misc_set_padding (GTK_MISC (label), 2, 2);

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox);
    gtk_box_pack_start (GTK_BOX (page), hbox, TRUE, TRUE, 0);

    scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (scrolledwindow);
    gtk_box_pack_start (GTK_BOX (hbox), scrolledwindow, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_SHADOW_ETCHED_IN);

    // Create table list view
    __widget_table_list_model = create_table_list_model ();
    __widget_table_list_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (__widget_table_list_model));
    gtk_widget_show (__widget_table_list_view);
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (__widget_table_list_view), TRUE);
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (__widget_table_list_view), TRUE);
    gtk_container_add (GTK_CONTAINER (scrolledwindow), __widget_table_list_view);

    // Create name column
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_reorderable (column, TRUE);
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_GROW_ONLY);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_sort_column_id (column, TABLE_COLUMN_NAME);

    gtk_tree_view_column_set_title (column, _("Name"));

    renderer = gtk_cell_renderer_pixbuf_new ();
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_set_attributes (column, renderer,
                                         "pixbuf", TABLE_COLUMN_ICON, NULL);

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, renderer,
                                         "text", TABLE_COLUMN_NAME, NULL);

    gtk_tree_view_append_column (GTK_TREE_VIEW (__widget_table_list_view), column);

    // Create lang column
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_reorderable (column, TRUE);
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_GROW_ONLY);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_sort_column_id (column, TABLE_COLUMN_LANG);

    gtk_tree_view_column_set_title (column, _("Language"));

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, renderer,
                                         "text", TABLE_COLUMN_LANG, NULL);

    gtk_tree_view_append_column (GTK_TREE_VIEW (__widget_table_list_view), column);

    // Create type column.
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_reorderable (column, TRUE);
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_GROW_ONLY);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_sort_column_id (column, TABLE_COLUMN_TYPE);

    gtk_tree_view_column_set_title (column, _("Type"));

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, renderer,
                                         "text", TABLE_COLUMN_TYPE, NULL);

    gtk_tree_view_append_column (GTK_TREE_VIEW (__widget_table_list_view), column);

    // Create file column.
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_reorderable (column, TRUE);
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_GROW_ONLY);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_sort_column_id (column, TABLE_COLUMN_FILE);

    gtk_tree_view_column_set_title (column, _("File"));

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, renderer,
                                         "text", TABLE_COLUMN_FILE, NULL);

    gtk_tree_view_append_column (GTK_TREE_VIEW (__widget_table_list_view), column);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (__widget_table_list_view));
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

    g_signal_connect (G_OBJECT (selection), "changed",
                      G_CALLBACK (on_table_list_selection_changed),
                      0);

    // Create buttons.
 
    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, TRUE, 4);

    button = gtk_button_new_with_mnemonic (_("_Install"));
    gtk_widget_show (button);
    gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (button), 2);
#if GTK_CHECK_VERSION(3, 0, 0)
    gtk_widget_set_tooltip_text (button, _("Install a new table."));
#else
    gtk_tooltips_set_tip (__widget_tooltips, button, _("Install a new table."), NULL);
#endif
    g_signal_connect (G_OBJECT (button), "clicked",
                      G_CALLBACK (on_table_install_clicked),
                      0);
    __widget_table_install_button = button;

    button = gtk_button_new_with_mnemonic (_("_Delete"));
    gtk_widget_show (button);
    gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (button), 2);
#if GTK_CHECK_VERSION(3, 0, 0)
    gtk_widget_set_tooltip_text (button, _("Delete the selected table."));
#else
    gtk_tooltips_set_tip (__widget_tooltips, button, _("Delete the selected table."), NULL);
#endif
    g_signal_connect (G_OBJECT (button), "clicked",
                      G_CALLBACK (on_table_delete_clicked),
                      0);
    __widget_table_delete_button = button;

    button = gtk_button_new_with_mnemonic (_("_Properties"));
    gtk_widget_show (button);
    gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (button), 2);
#if GTK_CHECK_VERSION(3, 0, 0)
    gtk_widget_set_tooltip_text (button, _("Edit the properties of the selected table."));
#else
    gtk_tooltips_set_tip (__widget_tooltips, button, _("Edit the properties of the selected table."), NULL);
#endif
    g_signal_connect (G_OBJECT (button), "clicked",
                      G_CALLBACK (on_table_properties_clicked),
                      0);
    __widget_table_properties_button = button;

    return page;
}

static GtkWidget *
create_setup_window ()
{
    static GtkWidget *window = 0;

    if (!window) {
        GtkWidget *notebook;
        GtkWidget *label;
        GtkWidget *page;

#if GTK_CHECK_VERSION(3, 0, 0)
#else
        __widget_tooltips = gtk_tooltips_new ();
#endif

        // Create the Notebook.
        notebook = gtk_notebook_new ();
        gtk_widget_show (notebook);

        // Create the first page.
        page = create_generic_page ();
        gtk_container_add (GTK_CONTAINER (notebook), page);

        // Create the label for this note page.
        label = gtk_label_new (_("Generic"));
        gtk_widget_show (label);
        gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 0), label);

        // Create the second page.
        page = create_keyboard_page ();

        // Create the label for this note page.
        label = gtk_label_new (_("Keyboard"));
        gtk_widget_show (label);

        // Append this page.
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

        // Create the third page.
        page = create_table_management_page ();

        // Create the label for this note page.
        label = gtk_label_new (_("Table Management"));
        gtk_widget_show (label);

        // Append this page.
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

        window = notebook;

        setup_widget_value ();
    }

    return window;
}

void
setup_widget_value ()
{
    if (__widget_show_prompt) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_show_prompt),
            __config_show_prompt);
    }

    if (__widget_show_key_hint) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_show_key_hint),
            __config_show_key_hint);
    }

    if (__widget_user_table_binary) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_user_table_binary),
            __config_user_table_binary);
    }

    if (__widget_user_phrase_first) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_user_phrase_first),
            __config_user_phrase_first);
    }

    if (__widget_long_phrase_first) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_long_phrase_first),
            __config_long_phrase_first);
    }

    for (int i = 0; __config_keyboards [i].key; ++ i) {
        if (__config_keyboards [i].entry) {
            gtk_entry_set_text (
                GTK_ENTRY (__config_keyboards [i].entry),
                __config_keyboards [i].data.c_str ());
        }
    }

}

void
load_config (const ConfigPointer &config)
{
    if (!config.null ()) {
        __config_show_prompt =
            config->read (String (SCIM_CONFIG_IMENGINE_TABLE_SHOW_PROMPT),
                          __config_show_prompt);
        __config_show_key_hint =
            config->read (String (SCIM_CONFIG_IMENGINE_TABLE_SHOW_KEY_HINT),
                          __config_show_key_hint);
        __config_user_table_binary =
            config->read (String (SCIM_CONFIG_IMENGINE_TABLE_USER_TABLE_BINARY),
                          __config_user_table_binary);
        __config_user_phrase_first =
            config->read (String (SCIM_CONFIG_IMENGINE_TABLE_USER_PHRASE_FIRST),
                          __config_user_phrase_first);
        __config_long_phrase_first =
            config->read (String (SCIM_CONFIG_IMENGINE_TABLE_LONG_PHRASE_FIRST),
                          __config_long_phrase_first);

        for (int i = 0; __config_keyboards [i].key; ++ i) {
            __config_keyboards [i].data =
                config->read (String (__config_keyboards [i].key),
                              __config_keyboards [i].data);
        }

        setup_widget_value ();

        load_all_tables ();

        __have_changed = false;
    }
}

void
save_config (const ConfigPointer &config)
{
    if (!config.null ()) {
        config->write (String (SCIM_CONFIG_IMENGINE_TABLE_SHOW_PROMPT),
                        __config_show_prompt);
        config->write (String (SCIM_CONFIG_IMENGINE_TABLE_SHOW_KEY_HINT),
                        __config_show_key_hint);
        config->write (String (SCIM_CONFIG_IMENGINE_TABLE_USER_TABLE_BINARY),
                       __config_user_table_binary);
        config->write (String (SCIM_CONFIG_IMENGINE_TABLE_USER_PHRASE_FIRST),
                       __config_user_phrase_first);
        config->write (String (SCIM_CONFIG_IMENGINE_TABLE_LONG_PHRASE_FIRST),
                       __config_long_phrase_first);

        for (int i = 0; __config_keyboards [i].key; ++ i) {
            config->write (String (__config_keyboards [i].key),
                          __config_keyboards [i].data);
        }

        save_all_tables ();

        __have_changed = false;
    }
}

bool
query_changed ()
{
    if (__have_changed)
        return true;

    GtkTreeIter iter;
    if (__widget_table_list_model &&
        gtk_tree_model_get_iter_first (GTK_TREE_MODEL (__widget_table_list_model), &iter)) {

        GenericTableLibrary *lib;

        do {
            gtk_tree_model_get (GTK_TREE_MODEL (__widget_table_list_model), &iter,
                                TABLE_COLUMN_LIBRARY, &lib,
                                -1);
            if (lib->updated ())
                return true;

        } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (__widget_table_list_model), &iter));
    }
    return false;
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
on_default_key_selection_clicked (GtkButton *button,
                                  gpointer   user_data)
{
    KeyboardConfigData *data = static_cast <KeyboardConfigData *> (user_data);

    if (data) {
        GtkWidget *dialog = scim_key_selection_dialog_new (_(data->title));
        gint result;

        scim_key_selection_dialog_set_keys (
            SCIM_KEY_SELECTION_DIALOG (dialog),
            gtk_entry_get_text (GTK_ENTRY (data->entry)));

        result = gtk_dialog_run (GTK_DIALOG (dialog));

        if (result == GTK_RESPONSE_OK) {
            const gchar *keys = scim_key_selection_dialog_get_keys (
                            SCIM_KEY_SELECTION_DIALOG (dialog));

            if (!keys) keys = "";

            if (strcmp (keys, gtk_entry_get_text (GTK_ENTRY (data->entry))) != 0)
                gtk_entry_set_text (GTK_ENTRY (data->entry), keys);
        }

        gtk_widget_destroy (dialog);
    }
}

static void
on_icon_file_selection_clicked (GtkButton *button,
                                gpointer   user_data)
{
    GtkEntry *entry = static_cast <GtkEntry*> (user_data);

    if (entry) {
#if GTK_CHECK_VERSION(3, 0, 0)
        GtkWidget *file_selection = gtk_file_chooser_dialog_new (
                                    _("Select an icon file"),
                                    NULL,
                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                    GTK_STOCK_OPEN, GTK_RESPONSE_OK,
                                    NULL);
#else
        GtkWidget *file_selection = gtk_file_selection_new (_("Select an icon file"));
        gtk_file_selection_set_filename (GTK_FILE_SELECTION (file_selection),
                                         gtk_entry_get_text (entry));
        gtk_file_selection_hide_fileop_buttons (GTK_FILE_SELECTION (file_selection));
#endif

        gint result = gtk_dialog_run (GTK_DIALOG (file_selection));

        if (result == GTK_RESPONSE_OK)
            gtk_entry_set_text (entry,
#if GTK_CHECK_VERSION(3, 0, 0)
                 gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_selection))
#else
                 gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selection))
#endif
                 );

        gtk_widget_destroy (file_selection);
    }
}

// Table manager related functions.
static bool
test_file_modify (const String &file)
{
    if (access (file.c_str (), W_OK) != 0 && errno != ENOENT)
        return false;

    return true;
}

static bool
test_file_unlink (const String &file)
{
    String path;
    String::size_type pos = file.rfind (SCIM_PATH_DELIM);

    if (pos != String::npos) path = file.substr (0, pos);

    if (!path.length ()) path = SCIM_PATH_DELIM_STRING;

    if (access (path.c_str (), W_OK) != 0)
        return false;

    return true;
}

static void
get_table_list (std::vector<String> &table_list, const String &path)
{
    table_list.clear ();

    DIR *dir = opendir (path.c_str ());
    if (dir != NULL) {
        struct dirent *file = readdir (dir);
        while (file != NULL) {
            struct stat filestat;
            String absfn = path + SCIM_PATH_DELIM_STRING + file->d_name;
            stat (absfn.c_str (), &filestat);

            if (S_ISREG (filestat.st_mode))
                table_list.push_back (absfn);

            file = readdir (dir);
        }
        closedir (dir);        
    }
}

static GdkPixbuf *
scale_pixbuf (GdkPixbuf **pixbuf,
               int         width,
               int         height)
{
    if (pixbuf && *pixbuf) {
        if (gdk_pixbuf_get_width (*pixbuf) != width ||
            gdk_pixbuf_get_height (*pixbuf) != height) {
            GdkPixbuf *dest = gdk_pixbuf_scale_simple (*pixbuf, width, height, GDK_INTERP_BILINEAR);
            gdk_pixbuf_unref (*pixbuf);
            *pixbuf = dest;
        }
        return *pixbuf;
    }
    return 0;
}

static GenericTableLibrary *
load_table_file (const String &file)
{
    GenericTableLibrary *library = 0;

    if (file.length ()) {
        library = new GenericTableLibrary ();
        if (!library->init (file, "", "", true)) {
            delete library;
            library = 0;
        }
    }

    return library;
}

static void
add_table_to_list (GenericTableLibrary *table, const String &dir, const String &file, bool user)
{
    if (!table || !table->valid () || !__widget_table_list_model) return;

    GtkTreeIter iter;
    GdkPixbuf   *pixbuf;
    String      name;
    String      lang;

    pixbuf = gdk_pixbuf_new_from_file (table->get_icon_file ().c_str (), NULL);

    if (!pixbuf) {
        pixbuf = gdk_pixbuf_new_from_file (SCIM_TABLE_ICON_FILE, NULL);
    }

    scale_pixbuf (&pixbuf, LIST_ICON_SIZE, LIST_ICON_SIZE);

    name = utf8_wcstombs (table->get_name (scim_get_current_locale ()));
    lang = scim_get_language_name (table->get_language ());

    gtk_list_store_append (__widget_table_list_model, &iter);

    gtk_list_store_set (__widget_table_list_model, &iter,
                        TABLE_COLUMN_ICON, pixbuf,
                        TABLE_COLUMN_NAME, name.c_str (),
                        TABLE_COLUMN_LANG, lang.c_str (),
                        TABLE_COLUMN_FILE, file.c_str (),
                        TABLE_COLUMN_TYPE, user ? _("User") : _("System"),
                        TABLE_COLUMN_LIBRARY, table,
                        TABLE_COLUMN_IS_USER, user,
                        -1);

    if (pixbuf)
        g_object_unref (pixbuf);
}

static void
load_all_tables ()
{
    if (!__widget_table_list_model) return;

    std::vector<String> usr_tables;
    std::vector<String> sys_tables;
    std::vector<String>::iterator it;
    GenericTableLibrary *library;

    String sys_dir (SCIM_TABLE_SYSTEM_TABLE_DIR);
    String usr_dir (scim_get_home_dir () + SCIM_TABLE_USER_TABLE_DIR);

    destroy_all_tables ();

    get_table_list (sys_tables, sys_dir);
    get_table_list (usr_tables, usr_dir);

    for (it = sys_tables.begin (); it != sys_tables.end (); ++it) {
        if ((library = load_table_file (*it)) != 0)
            add_table_to_list (library, sys_dir, *it, false);
    }

    for (it = usr_tables.begin (); it != usr_tables.end (); ++it) {
        if ((library = load_table_file (*it)) != 0)
            add_table_to_list (library, usr_dir, *it, true);
    }
}

static gboolean
table_list_destroy_iter_func (GtkTreeModel *model,
                              GtkTreePath  *path,
                              GtkTreeIter  *iter,
                              gpointer      data)
{
    GenericTableLibrary *library;
    gtk_tree_model_get (model, iter, TABLE_COLUMN_LIBRARY, &library, -1);

    if (library) {
        delete library;
        gtk_list_store_set (GTK_LIST_STORE (model), iter, TABLE_COLUMN_LIBRARY, NULL, -1);
    }

    return FALSE;
}

static void
delete_table_from_list (GtkTreeModel *model, GtkTreeIter *iter)
{
    if (model && iter) {
        table_list_destroy_iter_func (model, 0, iter, 0);
        gtk_list_store_remove (GTK_LIST_STORE (model), iter);
    }
}

static void
destroy_all_tables ()
{
    if (__widget_table_list_model) {
        gtk_tree_model_foreach (GTK_TREE_MODEL (__widget_table_list_model),
                                table_list_destroy_iter_func,
                                0);
        gtk_list_store_clear (__widget_table_list_model);
    }
}

static void
on_table_list_selection_changed (GtkTreeSelection *selection,
                                 gpointer          user_data)
{
    GtkTreeModel *model;
    GtkTreeIter   iter;
    gchar        *file = 0;
    bool          can_unlink;

    if (__widget_table_delete_button) {
        if (gtk_tree_selection_get_selected (selection, &model, &iter))
            gtk_tree_model_get (model, &iter,
                                TABLE_COLUMN_FILE, &file,
                                -1);

        if (file) {
            can_unlink = test_file_unlink (file);
            g_free (file);
        } else {
            can_unlink = false;
        }

        gtk_widget_set_sensitive (__widget_table_delete_button, can_unlink);
    }
}

static bool find_table_in_list_by_file (const String &file, GtkTreeIter *iter_found)
{
    GtkTreeIter iter;

    if (__widget_table_list_model &&
        gtk_tree_model_get_iter_first (GTK_TREE_MODEL (__widget_table_list_model), &iter)) {
        do {
            gchar *fn;
            gtk_tree_model_get (GTK_TREE_MODEL (__widget_table_list_model), &iter,
                                TABLE_COLUMN_FILE, &fn,
                                -1);
            if (String (fn) == file) {
                g_free (fn);

                if (iter_found)
                    *iter_found = iter;

                return true;
            }
            g_free (fn);
        } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (__widget_table_list_model), &iter));
    }
    return false;
}

static bool find_table_in_list_by_library (GenericTableLibrary *library, GtkTreeIter *iter_found)
{
    GtkTreeIter iter;

    if (__widget_table_list_model && library &&
        gtk_tree_model_get_iter_first (GTK_TREE_MODEL (__widget_table_list_model), &iter)) {
        do {
            GenericTableLibrary *lib;

            gtk_tree_model_get (GTK_TREE_MODEL (__widget_table_list_model), &iter,
                                TABLE_COLUMN_LIBRARY, &lib,
                                -1);

            if (lib && lib->get_uuid () == library->get_uuid ()) {
                if (iter_found)
                    *iter_found = iter;
                return true;
            }
        } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (__widget_table_list_model), &iter));
    }
    return false;
}

static void
on_table_install_clicked (GtkButton *button,
                          gpointer   user_data)
{
    GtkWidget *file_selection;
    GtkWidget *msg;
    GtkTreeIter iter;
    String file;
    String new_file;
    String path;
    gint result;
    GenericTableLibrary *library;
    String::size_type pos;
    bool user_table = true;

    String sys_dir (SCIM_TABLE_SYSTEM_TABLE_DIR);
    String usr_dir (scim_get_home_dir () + SCIM_TABLE_USER_TABLE_DIR);

    // Select the table file.
#if GTK_CHECK_VERSION(3, 0, 0)
    file_selection = gtk_file_chooser_dialog_new (
                                _("Please select the table file to be installed."),
                                NULL,
                                GTK_FILE_CHOOSER_ACTION_OPEN,
                                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                GTK_STOCK_OPEN, GTK_RESPONSE_OK,
                                NULL);
#else
    file_selection = gtk_file_selection_new (_("Please select the table file to be installed."));
    gtk_file_selection_hide_fileop_buttons (GTK_FILE_SELECTION (file_selection));
#endif

    result = gtk_dialog_run (GTK_DIALOG (file_selection));

    if (result != GTK_RESPONSE_OK) {
        gtk_widget_destroy (file_selection);
        return;
    }

#if GTK_CHECK_VERSION(3, 0, 0)
    file = String (gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_selection)));
#else
    file = String (gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selection)));
#endif

    gtk_widget_destroy (file_selection);

    pos = file.rfind (SCIM_PATH_DELIM);

    new_file = usr_dir + SCIM_PATH_DELIM_STRING;

    // Check if the file is already in the table directories.
    if (pos != String::npos) {
        path = file.substr (0, pos);
        if (!path.length ()) path = SCIM_PATH_DELIM_STRING;

        if (path == sys_dir || path == usr_dir) {
            msg = gtk_message_dialog_new (0,
                                          GTK_DIALOG_MODAL,
                                          GTK_MESSAGE_ERROR,
                                          GTK_BUTTONS_CLOSE,
                                          _("Failed to install the table! "
                                            "It's already in table file directory."));
            gtk_dialog_run (GTK_DIALOG (msg));
            gtk_widget_destroy (msg);
            return;
        }
        new_file += file.substr (pos + 1);
    } else {
        new_file += file;
    }

    path = usr_dir;

    // Load the table into memory.
    if ((library = load_table_file (file)) == 0) {
        msg = gtk_message_dialog_new (0,
                                      GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_ERROR,
                                      GTK_BUTTONS_CLOSE,
                                      _("Failed to load the table file!"));
        gtk_dialog_run (GTK_DIALOG (msg));
        gtk_widget_destroy (msg);
        return;
    }

    // Find if there is a table with same uuid was already installed.
    if (find_table_in_list_by_library (library, &iter)) {
        gchar *fn;

        gtk_tree_model_get (GTK_TREE_MODEL (__widget_table_list_model), &iter,
                            TABLE_COLUMN_FILE, &fn, -1);
        new_file = String (fn);
        g_free (fn);

        if (!test_file_modify (new_file)) {
            msg = gtk_message_dialog_new (0,
                                          GTK_DIALOG_MODAL,
                                          GTK_MESSAGE_ERROR,
                                          GTK_BUTTONS_CLOSE,
                                          _("Failed to install the table! "
                                            "Another version of this table was already installed."));

            gtk_dialog_run (GTK_DIALOG (msg));
            gtk_widget_destroy (msg);

            delete library;
            return;
        }

        msg = gtk_message_dialog_new (0,
                                      GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_QUESTION,
                                      GTK_BUTTONS_OK_CANCEL,
                                      _("Another version of this table was already installed. "
                                        "Do you want to replace it with the new one?"));
        result = gtk_dialog_run (GTK_DIALOG (msg));
        gtk_widget_destroy (msg);

        if (result != GTK_RESPONSE_OK) {
            delete library;
            return;
        }

        delete_table_from_list (GTK_TREE_MODEL (__widget_table_list_model), &iter);

        pos = new_file.rfind (SCIM_PATH_DELIM);
        if (pos != String::npos && pos != 0) path = new_file.substr (0, pos);
        else path = SCIM_PATH_DELIM_STRING;

        if (path == sys_dir) user_table = false;
    }

    // Find if the file is already existed.
    if (find_table_in_list_by_file (new_file, &iter)) {
        if (!test_file_modify (new_file)) {
            msg = gtk_message_dialog_new (0,
                                          GTK_DIALOG_MODAL,
                                          GTK_MESSAGE_ERROR,
                                          GTK_BUTTONS_CLOSE,
                                          _("Failed to install the table! "
                                            "A table with the same file name was already installed."));

            gtk_dialog_run (GTK_DIALOG (msg));
            gtk_widget_destroy (msg);

            delete library;
            return;
        }

        msg = gtk_message_dialog_new (0,
                                      GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_QUESTION,
                                      GTK_BUTTONS_OK_CANCEL,
                                      _("A table with the same file name was already installed. "
                                        "Do you want to overwrite it?"));
        result = gtk_dialog_run (GTK_DIALOG (msg));
        gtk_widget_destroy (msg);

        if (result != GTK_RESPONSE_OK) {
            delete library;
            return;
        }

        delete_table_from_list (GTK_TREE_MODEL (__widget_table_list_model), &iter);
    }

    if (!scim_make_dir (path) ||
        !library->save (new_file, "", "", __config_user_table_binary)) {
        msg = gtk_message_dialog_new (0,
                                      GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_ERROR,
                                      GTK_BUTTONS_CLOSE,
                                      _("Failed to install the table to %s!"),
                                      new_file.c_str ());
        gtk_dialog_run (GTK_DIALOG (msg));
        gtk_widget_destroy (msg);

        delete library;
        return;
    }

    add_table_to_list (library, path, new_file, user_table);
}

static void
on_table_delete_clicked (GtkButton *button,
                         gpointer   user_data)
{
    GtkTreeIter  iter;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkWidget *msg;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (__widget_table_list_view));

    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
        gchar *fn;
        String file;

        gtk_tree_model_get (model, &iter,
                            TABLE_COLUMN_FILE, &fn, -1);
        file = String (fn);
        g_free (fn);

        if (!test_file_unlink (file)) {
            msg = gtk_message_dialog_new (0,
                                          GTK_DIALOG_MODAL,
                                          GTK_MESSAGE_ERROR,
                                          GTK_BUTTONS_CLOSE,
                                          _("Can not delete the file %s!"),
                                          file.c_str ());
            gtk_dialog_run (GTK_DIALOG (msg));
            gtk_widget_destroy (msg);
            return;
        }

        msg = gtk_message_dialog_new (0,
                                      GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_QUESTION,
                                      GTK_BUTTONS_OK_CANCEL,
                                      _("Are you sure to delete this table file?"));

        gint result = gtk_dialog_run (GTK_DIALOG (msg));
        gtk_widget_destroy (msg);

        if (result != GTK_RESPONSE_OK)
            return;

        if (unlink (file.c_str ()) != 0) {
            msg = gtk_message_dialog_new (0,
                                          GTK_DIALOG_MODAL,
                                          GTK_MESSAGE_ERROR,
                                          GTK_BUTTONS_CLOSE,
                                          _("Failed to delete the table file!"));
            gtk_dialog_run (GTK_DIALOG (msg));
            gtk_widget_destroy (msg);
            return;
        }

        delete_table_from_list (model, &iter);
    }
}

static void
on_toggle_button_toggled (GtkToggleButton *button,
                          gpointer         user_data)
{
    if (gtk_toggle_button_get_active (button))
        gtk_button_set_label (GTK_BUTTON (button), _("True"));
    else
        gtk_button_set_label (GTK_BUTTON (button), _("False"));
}


static gint
run_table_properties_dialog (GenericTableLibrary *lib, TablePropertiesData &data, bool editable)
{
    GtkWidget *dialog;
    GtkWidget *dialog_vbox;
    GtkWidget *dialog_action_area;
    GtkWidget *scrolledwindow;
    GtkWidget *viewport;
    GtkWidget *table;
    GtkWidget *label;
    GtkWidget *hbox;
    GtkWidget *entry_name;
    GtkWidget *entry_author;
    GtkWidget *entry_uuid;
    GtkWidget *entry_serial;
    GtkWidget *entry_icon;
    GtkWidget *button_icon;
    GtkWidget *entry_languages;
    GtkWidget *entry_status_prompt;
    GtkWidget *entry_valid_input_chars;
    GtkWidget *entry_multi_wildcard_chars;
    GtkWidget *entry_single_wildcard_chars;
    GtkWidget *spin_max_key_length;
    GtkWidget *toggle_dynamic_adjust;
    GtkWidget *toggle_always_show_lookup;
    GtkWidget *toggle_show_key_prompt;
    GtkWidget *toggle_auto_select;
    GtkWidget *toggle_auto_fill;
    GtkWidget *toggle_auto_wildcard;
    GtkWidget *toggle_auto_commit;
    GtkWidget *toggle_auto_split;
    GtkWidget *toggle_discard_invalid_key;
    GtkWidget *toggle_def_full_width_punct;
    GtkWidget *toggle_def_full_width_letter;
    GtkWidget *cancelbutton;
    GtkWidget *okbutton;

#if GTK_CHECK_VERSION(3, 0, 0)
#else
    GtkTooltips *tooltips;
#endif

    KeyboardConfigData split_keys  = {NULL,
                                      _("Split Keys:"),
                                      _("Split Keys:"),
                                      _("The key strokes to split inputted string."),
                                      NULL,
                                      NULL,
                                      ""};

    KeyboardConfigData commit_keys = {NULL,
                                      _("Commit Keys:"),
                                      _("Commit Keys:"),
                                      _("The key strokes to commit converted result to client."),
                                      NULL,
                                      NULL,
                                      ""};

    KeyboardConfigData forward_keys= {NULL,
                                      _("Forward Keys:"),
                                      _("Forward Keys:"),
                                      _("The key strokes to forward inputted string to client."),
                                      NULL,
                                      NULL,
                                      ""};

    KeyboardConfigData select_keys = {NULL,
                                      _("Select Keys:"),
                                      _("Select Keys:"),
                                      _("The key strokes to select candidate phrases in lookup table."),
                                      NULL,
                                      NULL,
                                      ""};

    KeyboardConfigData page_up_keys = {NULL,
                                      _("Page Up Keys:"),
                                      _("Page Up Keys:"),
                                      _("The lookup table page up keys"),
                                      NULL,
                                      NULL,
                                      ""};

    KeyboardConfigData page_down_keys = {NULL,
                                      _("Page Down Keys:"),
                                      _("Page Down Keys:"),
                                      _("The lookup table page down keys"),
                                      NULL,
                                      NULL,
                                      ""};

    KeyboardConfigData *all_keys [] = {
                                       &split_keys,
                                       &commit_keys,
                                       &forward_keys,
                                       &select_keys,
                                       &page_up_keys,
                                       &page_down_keys,
                                       NULL
                                      };

    gint row = 0;
    gint result = GTK_RESPONSE_CANCEL;

    {// Create dialog.
#if GTK_CHECK_VERSION(3, 0, 0)
#else
        tooltips = gtk_tooltips_new ();
#endif

        dialog = gtk_dialog_new ();
        gtk_container_set_border_width (GTK_CONTAINER (dialog), 2);
        gtk_window_set_title (GTK_WINDOW (dialog), _("Table Properties"));
        gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
        gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
#if GTK_CHECK_VERSION(3, 0, 0)
        g_object_set (G_OBJECT (dialog), "has-separator", FALSE, NULL);
#else
        gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
#endif
  
#if GTK_CHECK_VERSION(3, 0, 0)
        dialog_vbox = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
#else
        dialog_vbox = GTK_DIALOG (dialog)->vbox;
#endif
        gtk_widget_show (dialog_vbox);
  
        scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
        gtk_widget_show (scrolledwindow);
        gtk_box_pack_start (GTK_BOX (dialog_vbox), scrolledwindow, TRUE, TRUE, 0);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
  
        viewport = gtk_viewport_new (NULL, NULL);
        gtk_widget_show (viewport);
        gtk_container_add (GTK_CONTAINER (scrolledwindow), viewport);
  
        table = gtk_table_new (24, 2, FALSE);
        gtk_widget_show (table);
        gtk_container_add (GTK_CONTAINER (viewport), table);
        gtk_table_set_row_spacings (GTK_TABLE (table), 2);
        gtk_table_set_col_spacings (GTK_TABLE (table), 2);
  
        // Name
        label = gtk_label_new (_("Name:"));
        gtk_widget_show (label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
  
        entry_name = gtk_entry_new ();
        gtk_widget_show (entry_name);
        gtk_table_attach (GTK_TABLE (table), entry_name, 1, 2, row, row+1,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
 
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_tooltip_text (entry_name, _("The name of this table."));
#else
        gtk_tooltips_set_tip (tooltips, entry_name,
                              _("The name of this table."), NULL);
#endif

        ++ row;

        // Author
        label = gtk_label_new (_("Author:"));
        gtk_widget_show (label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
  
        entry_author = gtk_entry_new ();
        gtk_widget_show (entry_author);
        gtk_table_attach (GTK_TABLE (table), entry_author, 1, 2, row, row+1,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
  
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_tooltip_text (entry_author,
                              _("The author of this table."));
#else
        gtk_tooltips_set_tip (tooltips, entry_author,
                              _("The author of this table."), NULL);
#endif

        ++ row;

        // UUID
        label = gtk_label_new (_("UUID:"));
        gtk_widget_show (label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
  
        entry_uuid = gtk_entry_new ();
        gtk_widget_show (entry_uuid);
        gtk_table_attach (GTK_TABLE (table), entry_uuid, 1, 2, row, row+1,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
  
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_tooltip_text (entry_uuid,
                              _("The unique ID of this table."));
#else
        gtk_tooltips_set_tip (tooltips, entry_uuid,
                              _("The unique ID of this table."), NULL);
#endif

        ++ row;

        // Serial Number
        label = gtk_label_new (_("Serial Number:"));
        gtk_widget_show (label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
  
        entry_serial = gtk_entry_new ();
        gtk_widget_show (entry_serial);
        gtk_table_attach (GTK_TABLE (table), entry_serial, 1, 2, row, row+1,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
  
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_tooltip_text (entry_serial,
                              _("The serial number of this table."));
#else
        gtk_tooltips_set_tip (tooltips, entry_serial,
                              _("The serial number of this table."), NULL);
#endif

        ++ row;

        // Icon file
        label = gtk_label_new (_("Icon File:"));
        gtk_widget_show (label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
  
        hbox = gtk_hbox_new (FALSE, 0);
        gtk_widget_show (hbox);
        gtk_table_attach (GTK_TABLE (table), hbox, 1, 2, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (GTK_FILL), 0, 0);
  
        entry_icon = gtk_entry_new ();
        gtk_widget_show (entry_icon);
        gtk_box_pack_start (GTK_BOX (hbox), entry_icon, TRUE, TRUE, 0);
  
        button_icon = gtk_button_new_with_mnemonic (_("Browse"));
        gtk_widget_show (button_icon);
        gtk_box_pack_start (GTK_BOX (hbox), button_icon, FALSE, FALSE, 0);
 
        g_signal_connect (G_OBJECT (button_icon), "clicked",
                          G_CALLBACK (on_icon_file_selection_clicked),
                          entry_icon);

#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_tooltip_text (entry_icon,
                              _("The icon file of this table."));
#else
        gtk_tooltips_set_tip (tooltips, entry_icon,
                              _("The icon file of this table."), NULL);
#endif
  
        ++ row;

        // Supported Languages
        label = gtk_label_new (_("Supported Languages:"));
        gtk_widget_show (label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
  
        entry_languages = gtk_entry_new ();
        gtk_widget_show (entry_languages);
        gtk_table_attach (GTK_TABLE (table), entry_languages, 1, 2, row, row+1,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);

#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_tooltip_text (entry_languages,
                              _("The languages supported by this table."));
#else
        gtk_tooltips_set_tip (tooltips, entry_languages,
                              _("The languages supported by this table."), NULL);
#endif

        ++ row;

        // Status Prompts
        label = gtk_label_new (_("Status Prompt:"));
        gtk_widget_show (label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
  
        entry_status_prompt = gtk_entry_new ();
        gtk_widget_show (entry_status_prompt);
        gtk_table_attach (GTK_TABLE (table), entry_status_prompt, 1, 2, row, row+1,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
 
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_tooltip_text (entry_status_prompt,
                              _("A prompt string to be shown in status area."));
#else
        gtk_tooltips_set_tip (tooltips, entry_status_prompt,
                              _("A prompt string to be shown in status area."), NULL);
#endif

        ++ row;

        // Valid Input Chars
        label = gtk_label_new (_("Valid Input Chars:"));
        gtk_widget_show (label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
  
        entry_valid_input_chars = gtk_entry_new ();
        gtk_widget_show (entry_valid_input_chars);
        gtk_table_attach (GTK_TABLE (table), entry_valid_input_chars, 1, 2, row, row+1,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
  
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_tooltip_text (entry_valid_input_chars,
                              _("The valid input chars of this table."));
#else
        gtk_tooltips_set_tip (tooltips, entry_valid_input_chars,
                              _("The valid input chars of this table."), NULL);
#endif

        ++ row;

        // Mulit Wildcard Char
        label = gtk_label_new (_("Multi Wildcard Char:"));
        gtk_widget_show (label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
  
        entry_multi_wildcard_chars = gtk_entry_new ();
        gtk_widget_show (entry_multi_wildcard_chars);
        gtk_table_attach (GTK_TABLE (table), entry_multi_wildcard_chars, 1, 2, row, row+1,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);

#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_tooltip_text (entry_multi_wildcard_chars,
                              _("The multi wildcard chars of this table. "
                                "These chars can be used to match one or more arbitrary chars."));
#else
        gtk_tooltips_set_tip (tooltips, entry_multi_wildcard_chars,
                              _("The multi wildcard chars of this table. "
                                "These chars can be used to match one or more arbitrary chars."), NULL);
#endif
 
        ++ row;

        // Single Wildcard Char
        label = gtk_label_new (_("Single Wildcard Char:"));
        gtk_widget_show (label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
  
        entry_single_wildcard_chars = gtk_entry_new ();
        gtk_widget_show (entry_single_wildcard_chars);
        gtk_table_attach (GTK_TABLE (table), entry_single_wildcard_chars, 1, 2, row, row+1,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
 
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_tooltip_text (entry_single_wildcard_chars,
                              _("The single wildcard chars of this table."
                                "These chars can be used to match one arbitrary char."));
#else
        gtk_tooltips_set_tip (tooltips, entry_single_wildcard_chars,
                              _("The single wildcard chars of this table."
                                "These chars can be used to match one arbitrary char."), NULL);
#endif

        ++ row;

        // All keyboard settings
        for (int i = 0; all_keys [i]; ++i) {
            label = gtk_label_new (all_keys [i]->label);
            gtk_widget_show (label);
            gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
                              (GtkAttachOptions) (GTK_FILL),
                              (GtkAttachOptions) (0), 0, 0);
            gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);

            hbox = gtk_hbox_new (FALSE, 0);
            gtk_widget_show (hbox);
            gtk_table_attach (GTK_TABLE (table), hbox, 1, 2, row, row+1,
                              (GtkAttachOptions) (GTK_FILL),
                              (GtkAttachOptions) (GTK_FILL), 0, 0);

            all_keys [i]->entry = gtk_entry_new ();
            gtk_widget_show (all_keys [i]->entry);
            gtk_box_pack_start (GTK_BOX (hbox), all_keys [i]->entry, TRUE, TRUE, 0);

            all_keys [i]->button = gtk_button_new_with_label (_("..."));
            gtk_widget_show (all_keys [i]->button);
            gtk_box_pack_start (GTK_BOX (hbox), all_keys [i]->button, FALSE, FALSE, 0);
 
            g_signal_connect ((gpointer) all_keys [i]->button, "clicked",
                              G_CALLBACK (on_default_key_selection_clicked),
                              all_keys [i]);

#if GTK_CHECK_VERSION(3, 0, 0)
            gtk_widget_set_tooltip_text (all_keys [i]->entry, all_keys [i]->tooltip);
#else
            gtk_tooltips_set_tip (tooltips, all_keys [i]->entry, all_keys [i]->tooltip, NULL);
#endif

            ++ row;
        }

        // Max key length
        label = gtk_label_new (_("Max Key Length:"));
        gtk_widget_show (label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
  
        spin_max_key_length = gtk_spin_button_new_with_range (1, SCIM_GT_MAX_KEY_LENGTH, 1);
        gtk_spin_button_set_digits (GTK_SPIN_BUTTON (spin_max_key_length), 0);
        gtk_widget_show (spin_max_key_length);
        gtk_table_attach (GTK_TABLE (table), spin_max_key_length, 1, 2, row, row+1,
                          (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
  
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_tooltip_text (spin_max_key_length,
                              _("The maxmium length of key strings."));
#else
        gtk_tooltips_set_tip (tooltips, spin_max_key_length,
                              _("The maxmium length of key strings."), NULL);
#endif

        ++ row;

        // Show key prompt. 
        label = gtk_label_new (_("Show Key Prompt:"));
        gtk_widget_show (label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
  
        toggle_show_key_prompt = gtk_toggle_button_new_with_label (_("True"));
        gtk_widget_show (toggle_show_key_prompt);
        gtk_table_attach (GTK_TABLE (table), toggle_show_key_prompt, 1, 2, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_show_key_prompt), TRUE);
        g_signal_connect (G_OBJECT (toggle_show_key_prompt), "toggled", 
                          G_CALLBACK (on_toggle_button_toggled),
                          0);
  
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_tooltip_text (toggle_show_key_prompt,
                              _("If true then the key prompts will be shown "
                                "instead of the raw keys."));
#else
        gtk_tooltips_set_tip (tooltips, toggle_show_key_prompt,
                              _("If true then the key prompts will be shown "
                                "instead of the raw keys."), NULL);
#endif

        ++ row;
        // Auto Select
        label = gtk_label_new (_("Auto Select:"));
        gtk_widget_show (label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
  
        toggle_auto_select = gtk_toggle_button_new_with_label (_("True"));
        gtk_widget_show (toggle_auto_select);
        gtk_table_attach (GTK_TABLE (table), toggle_auto_select, 1, 2, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_auto_select), TRUE);
        g_signal_connect (G_OBJECT (toggle_auto_select), "toggled", 
                          G_CALLBACK (on_toggle_button_toggled),
                          0);
  
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_tooltip_text (toggle_auto_select,
                              _("If true then the first candidate phrase will be "
                                "selected automatically when inputing the next key."));
#else
        gtk_tooltips_set_tip (tooltips, toggle_auto_select,
                              _("If true then the first candidate phrase will be "
                                "selected automatically when inputing the next key."), NULL);
#endif

        ++ row;

        // Auto Wildcard
        label = gtk_label_new (_("Auto Wildcard:"));
        gtk_widget_show (label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
  
        toggle_auto_wildcard = gtk_toggle_button_new_with_label (_("True"));
        gtk_widget_show (toggle_auto_wildcard);
        gtk_table_attach (GTK_TABLE (table), toggle_auto_wildcard, 1, 2, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_auto_wildcard), TRUE);
        g_signal_connect (G_OBJECT (toggle_auto_wildcard), "toggled", 
                          G_CALLBACK (on_toggle_button_toggled),
                          0);
 
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_tooltip_text (toggle_auto_wildcard,
                             _("If true then a multi wildcard char will be appended to "
                                "the end of the inputted key string when searching phrases."));
#else
        gtk_tooltips_set_tip (tooltips, toggle_auto_wildcard,
                             _("If true then a multi wildcard char will be appended to "
                                "the end of the inputted key string when searching phrases."), NULL);
#endif

        ++ row;

        // Auto Commit
        label = gtk_label_new (_("Auto Commit:"));
        gtk_widget_show (label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
  
        toggle_auto_commit = gtk_toggle_button_new_with_label (_("True"));
        gtk_widget_show (toggle_auto_commit);
        gtk_table_attach (GTK_TABLE (table), toggle_auto_commit, 1, 2, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_auto_commit), TRUE);
        g_signal_connect (G_OBJECT (toggle_auto_commit), "toggled", 
                          G_CALLBACK (on_toggle_button_toggled),
                          0);
 
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_tooltip_text (toggle_auto_commit,
                              _("If true then the converted result string will "
                                "be committed to client automatically."));
#else
        gtk_tooltips_set_tip (tooltips, toggle_auto_commit,
                              _("If true then the converted result string will "
                                "be committed to client automatically."), NULL);
#endif

        ++ row;

        // Auto Split
        label = gtk_label_new (_("Auto Split:"));
        gtk_widget_show (label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
  
        toggle_auto_split = gtk_toggle_button_new_with_label (_("True"));
        gtk_widget_show (toggle_auto_split);
        gtk_table_attach (GTK_TABLE (table), toggle_auto_split, 1, 2, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_auto_split), TRUE);
        g_signal_connect (G_OBJECT (toggle_auto_split), "toggled", 
                          G_CALLBACK (on_toggle_button_toggled),
                          0);
 
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_tooltip_text (toggle_auto_split,
                              _("If true then the inputted key string will be "
                                "split automatically when necessary."));
#else
        gtk_tooltips_set_tip (tooltips, toggle_auto_split,
                              _("If true then the inputted key string will be "
                                "split automatically when necessary."), NULL);
#endif

        ++ row;

        // Discard Invalid Key
        label = gtk_label_new (_("Discard Invalid Key:"));
        gtk_widget_show (label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
  
        toggle_discard_invalid_key = gtk_toggle_button_new_with_label (_("True"));
        gtk_widget_show (toggle_discard_invalid_key);
        gtk_table_attach (GTK_TABLE (table), toggle_discard_invalid_key, 1, 2, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_discard_invalid_key), TRUE);
        g_signal_connect (G_OBJECT (toggle_discard_invalid_key), "toggled", 
                          G_CALLBACK (on_toggle_button_toggled),
                          0);
 
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_tooltip_text (toggle_discard_invalid_key,
                              _("If true then the invalid key will be discarded automatically."
                                "This option is only valid when Auto Select and Auto Commit is true."));
#else
        gtk_tooltips_set_tip (tooltips, toggle_discard_invalid_key,
                              _("If true then the invalid key will be discarded automatically."
                                "This option is only valid when Auto Select and Auto Commit is true."), NULL);
#endif

        ++ row;

        // Dynamic Adjust
        label = gtk_label_new (_("Dynamic Adjust:"));
        gtk_widget_show (label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
 
        toggle_dynamic_adjust = gtk_toggle_button_new_with_label (_("True"));
        gtk_widget_show (toggle_dynamic_adjust);
        gtk_table_attach (GTK_TABLE (table), toggle_dynamic_adjust, 1, 2, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_dynamic_adjust), TRUE);
        g_signal_connect (G_OBJECT (toggle_dynamic_adjust), "toggled", 
                          G_CALLBACK (on_toggle_button_toggled),
                          0);
  
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_tooltip_text (toggle_dynamic_adjust,
                              _("If true then the phrases' frequencies "
                                "will be adjusted dynamically."));
#else
        gtk_tooltips_set_tip (tooltips, toggle_dynamic_adjust,
                              _("If true then the phrases' frequencies "
                                "will be adjusted dynamically."), NULL);
#endif

        ++ row;

        // Auto Fill Preedit String
        label = gtk_label_new (_("Auto Fill Preedit Area:"));
        gtk_widget_show (label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
  
        toggle_auto_fill = gtk_toggle_button_new_with_label (_("True"));
        gtk_widget_show (toggle_auto_fill);
        gtk_table_attach (GTK_TABLE (table), toggle_auto_fill, 1, 2, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_auto_fill), TRUE);
        g_signal_connect (G_OBJECT (toggle_auto_fill), "toggled", 
                          G_CALLBACK (on_toggle_button_toggled),
                          0);
  
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_tooltip_text (toggle_auto_fill,
                              _("If true then the preedit string will be filled up with the "
                                "current candiate phrase automatically."
                                "This option is only valid when Auto Select is TRUE."));
#else
        gtk_tooltips_set_tip (tooltips, toggle_auto_fill,
                              _("If true then the preedit string will be filled up with the "
                                "current candiate phrase automatically."
                                "This option is only valid when Auto Select is TRUE."), NULL);
#endif

        ++ row;

        // Always Show Lookup
        label = gtk_label_new (_("Always Show Lookup Table:"));
        gtk_widget_show (label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
 
        toggle_always_show_lookup = gtk_toggle_button_new_with_label (_("True"));
        gtk_widget_show (toggle_always_show_lookup);
        gtk_table_attach (GTK_TABLE (table), toggle_always_show_lookup, 1, 2, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_always_show_lookup), TRUE);
        g_signal_connect (G_OBJECT (toggle_always_show_lookup), "toggled", 
                          G_CALLBACK (on_toggle_button_toggled),
                          0);
  
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_tooltip_text (toggle_always_show_lookup,
                              _("If true then the lookup table will always be shown "
                                "when any candidate phrase is available. Otherwise "
                                "the lookup table will only be shown when necessary.\n"
                                "If Auto Fill is false, then this option will be no effect, "
                                "and always be true."));
#else
        gtk_tooltips_set_tip (tooltips, toggle_always_show_lookup,
                              _("If true then the lookup table will always be shown "
                                "when any candidate phrase is available. Otherwise "
                                "the lookup table will only be shown when necessary.\n"
                                "If Auto Fill is false, then this option will be no effect, "
                                "and always be true."), NULL);
#endif

        ++ row;

        // Default full width punctuation
        label = gtk_label_new (_("Default Full Width Punct:"));
        gtk_widget_show (label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
 
        toggle_def_full_width_punct = gtk_toggle_button_new_with_label (_("True"));
        gtk_widget_show (toggle_def_full_width_punct);
        gtk_table_attach (GTK_TABLE (table), toggle_def_full_width_punct, 1, 2, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_def_full_width_punct), TRUE);
        g_signal_connect (G_OBJECT (toggle_def_full_width_punct), "toggled", 
                          G_CALLBACK (on_toggle_button_toggled),
                          0);
  
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_tooltip_text (toggle_def_full_width_punct,
                              _("If true then full width punctuations will be inputted by default."));
#else
        gtk_tooltips_set_tip (tooltips, toggle_def_full_width_punct,
                              _("If true then full width punctuations will be inputted by default."), NULL);
#endif

        ++ row;

        // Default full width letter 
        label = gtk_label_new (_("Default Full Width Letter:"));
        gtk_widget_show (label);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
 
        toggle_def_full_width_letter = gtk_toggle_button_new_with_label (_("True"));
        gtk_widget_show (toggle_def_full_width_letter);
        gtk_table_attach (GTK_TABLE (table), toggle_def_full_width_letter, 1, 2, row, row+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_def_full_width_letter), TRUE);
        g_signal_connect (G_OBJECT (toggle_def_full_width_letter), "toggled", 
                          G_CALLBACK (on_toggle_button_toggled),
                          0);
  
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_tooltip_text (toggle_def_full_width_letter,
                              _("If true then full width letters will be inputted by default."));
#else
        gtk_tooltips_set_tip (tooltips, toggle_def_full_width_letter,
                              _("If true then full width letters will be inputted by default."), NULL);
#endif

        // action buttons
#if GTK_CHECK_VERSION(3, 0, 0)
        dialog_action_area = gtk_dialog_get_action_area (GTK_DIALOG (dialog));
#else
        dialog_action_area = GTK_DIALOG (dialog)->action_area;
#endif
        gtk_widget_show (dialog_action_area);
        gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area), GTK_BUTTONBOX_END);
  
        cancelbutton = gtk_button_new_from_stock ("gtk-cancel");
        gtk_widget_show (cancelbutton);
        gtk_dialog_add_action_widget (GTK_DIALOG (dialog), cancelbutton, GTK_RESPONSE_CANCEL);
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_can_default (cancelbutton, TRUE);
#else
        GTK_WIDGET_SET_FLAGS (cancelbutton, GTK_CAN_DEFAULT);
#endif

  
        okbutton = gtk_button_new_from_stock ("gtk-ok");
        gtk_widget_show (okbutton);
        gtk_dialog_add_action_widget (GTK_DIALOG (dialog), okbutton, GTK_RESPONSE_OK);
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_widget_set_can_default (okbutton, TRUE);
#else
        GTK_WIDGET_SET_FLAGS (okbutton, GTK_CAN_DEFAULT);
#endif
    } 

    {// Set initial data and the widgets status.

#if GTK_CHECK_VERSION(3, 0, 0)
        g_object_set (G_OBJECT (entry_name), "editable", FALSE, NULL);
        g_object_set (G_OBJECT (entry_author), "editable", FALSE, NULL);
        g_object_set (G_OBJECT (entry_uuid), "editable", FALSE, NULL);
        g_object_set (G_OBJECT (entry_serial), "editable", FALSE, NULL);
        g_object_set (G_OBJECT (entry_icon), "editable", FALSE, NULL);
        g_object_set (G_OBJECT (entry_valid_input_chars), "editable", FALSE, NULL);
        g_object_set (G_OBJECT (split_keys.entry), "editable", FALSE, NULL);
        g_object_set (G_OBJECT (commit_keys.entry), "editable", FALSE, NULL);
        g_object_set (G_OBJECT (forward_keys.entry), "editable", FALSE, NULL);
        g_object_set (G_OBJECT (select_keys.entry), "editable", FALSE, NULL);
        g_object_set (G_OBJECT (page_up_keys.entry), "editable", FALSE, NULL);
        g_object_set (G_OBJECT (page_down_keys.entry), "editable", FALSE, NULL);
#else
        gtk_entry_set_editable (GTK_ENTRY (entry_name), FALSE);
        gtk_entry_set_editable (GTK_ENTRY (entry_author), FALSE);
        gtk_entry_set_editable (GTK_ENTRY (entry_uuid), FALSE);
        gtk_entry_set_editable (GTK_ENTRY (entry_serial), FALSE);
        gtk_entry_set_editable (GTK_ENTRY (entry_icon), FALSE);
        gtk_entry_set_editable (GTK_ENTRY (entry_valid_input_chars), FALSE);
        gtk_entry_set_editable (GTK_ENTRY (split_keys.entry), FALSE);
        gtk_entry_set_editable (GTK_ENTRY (commit_keys.entry), FALSE);
        gtk_entry_set_editable (GTK_ENTRY (forward_keys.entry), FALSE);
        gtk_entry_set_editable (GTK_ENTRY (select_keys.entry), FALSE);
        gtk_entry_set_editable (GTK_ENTRY (page_up_keys.entry), FALSE);
        gtk_entry_set_editable (GTK_ENTRY (page_down_keys.entry), FALSE);
#endif

        if (!editable) {
#if GTK_CHECK_VERSION(3, 0, 0)
            g_object_set (G_OBJECT (entry_status_prompt), "editable", FALSE, NULL);
            g_object_set (G_OBJECT (entry_languages), "editable", FALSE, NULL);
            g_object_set (G_OBJECT (entry_multi_wildcard_chars), "editable", FALSE, NULL);
            g_object_set (G_OBJECT (entry_single_wildcard_chars), "editable", FALSE, NULL);
#else
            gtk_entry_set_editable (GTK_ENTRY (entry_status_prompt), FALSE);
            gtk_entry_set_editable (GTK_ENTRY (entry_languages), FALSE);
            gtk_entry_set_editable (GTK_ENTRY (entry_multi_wildcard_chars), FALSE);
            gtk_entry_set_editable (GTK_ENTRY (entry_single_wildcard_chars), FALSE);
#endif
            gtk_widget_set_sensitive (spin_max_key_length, FALSE);
            gtk_widget_set_sensitive (toggle_show_key_prompt, FALSE);
            gtk_widget_set_sensitive (toggle_auto_select, FALSE);
            gtk_widget_set_sensitive (toggle_auto_fill, FALSE);
            gtk_widget_set_sensitive (toggle_auto_wildcard, FALSE);
            gtk_widget_set_sensitive (toggle_auto_commit, FALSE);
            gtk_widget_set_sensitive (toggle_auto_split, FALSE);
            gtk_widget_set_sensitive (toggle_discard_invalid_key, FALSE);
            gtk_widget_set_sensitive (toggle_dynamic_adjust, FALSE);
            gtk_widget_set_sensitive (toggle_always_show_lookup, FALSE);
            gtk_widget_set_sensitive (toggle_def_full_width_punct, FALSE);
            gtk_widget_set_sensitive (toggle_def_full_width_letter, FALSE);
            gtk_widget_set_sensitive (button_icon, FALSE);
            gtk_widget_set_sensitive (split_keys.button, FALSE);
            gtk_widget_set_sensitive (commit_keys.button, FALSE);
            gtk_widget_set_sensitive (forward_keys.button, FALSE);
            gtk_widget_set_sensitive (select_keys.button, FALSE);
            gtk_widget_set_sensitive (page_up_keys.button, FALSE);
            gtk_widget_set_sensitive (page_down_keys.button, FALSE);
        }

        gtk_entry_set_text     (GTK_ENTRY (entry_name), data.name.c_str ());
        gtk_entry_set_text     (GTK_ENTRY (entry_author), data.author.c_str ());
        gtk_entry_set_text     (GTK_ENTRY (entry_uuid), data.uuid.c_str ());
        gtk_entry_set_text     (GTK_ENTRY (entry_serial), data.serial.c_str ());
        gtk_entry_set_text     (GTK_ENTRY (entry_icon), data.icon.c_str ());
        gtk_entry_set_text     (GTK_ENTRY (entry_languages), data.languages.c_str ());
        gtk_entry_set_text     (GTK_ENTRY (entry_status_prompt), data.status_prompt.c_str ());
        gtk_entry_set_text     (GTK_ENTRY (entry_valid_input_chars), data.valid_input_chars.c_str ());
        gtk_entry_set_text     (GTK_ENTRY (entry_multi_wildcard_chars), data.multi_wildcard_chars.c_str ());
        gtk_entry_set_text     (GTK_ENTRY (entry_single_wildcard_chars), data.single_wildcard_chars.c_str ());

        split_keys.data = data.split_keys;
        commit_keys.data = data.commit_keys;
        forward_keys.data = data.forward_keys;
        select_keys.data = data.select_keys;
        page_up_keys.data = data.page_up_keys;
        page_down_keys.data = data.page_down_keys;

        gtk_entry_set_text     (GTK_ENTRY (split_keys.entry), data.split_keys.c_str ());
        gtk_entry_set_text     (GTK_ENTRY (commit_keys.entry), data.commit_keys.c_str ());
        gtk_entry_set_text     (GTK_ENTRY (forward_keys.entry), data.forward_keys.c_str ());
        gtk_entry_set_text     (GTK_ENTRY (select_keys.entry), data.select_keys.c_str ());
        gtk_entry_set_text     (GTK_ENTRY (page_up_keys.entry), data.page_up_keys.c_str ());
        gtk_entry_set_text     (GTK_ENTRY (page_down_keys.entry), data.page_down_keys.c_str ());

        gtk_spin_button_set_range (GTK_SPIN_BUTTON (spin_max_key_length), data.max_key_length, SCIM_GT_MAX_KEY_LENGTH);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_show_key_prompt), data.show_key_prompt);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_auto_select), data.auto_select);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_auto_fill), data.auto_fill);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_auto_wildcard), data.auto_wildcard);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_auto_commit), data.auto_commit);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_auto_split), data.auto_split);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_discard_invalid_key), data.discard_invalid_key);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_dynamic_adjust), data.dynamic_adjust);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_always_show_lookup), data.always_show_lookup);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_def_full_width_punct), data.def_full_width_punct);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_def_full_width_letter), data.def_full_width_letter);
    }

    {// Run the dialog and return the result;
        gtk_window_set_default_size (GTK_WINDOW (dialog), 560, 400);

        while (1) {
            result = gtk_dialog_run (GTK_DIALOG (dialog));

            if (result != GTK_RESPONSE_OK) break;

            data.icon = String (gtk_entry_get_text (GTK_ENTRY (entry_icon)));
            data.languages = String (gtk_entry_get_text (GTK_ENTRY (entry_languages)));
            data.status_prompt = String (gtk_entry_get_text (GTK_ENTRY (entry_status_prompt)));
            data.multi_wildcard_chars  = String (gtk_entry_get_text (GTK_ENTRY (entry_multi_wildcard_chars)));
            data.single_wildcard_chars = String (gtk_entry_get_text (GTK_ENTRY (entry_single_wildcard_chars)));
            data.split_keys = String (gtk_entry_get_text (GTK_ENTRY (split_keys.entry)));
            data.commit_keys = String (gtk_entry_get_text (GTK_ENTRY (commit_keys.entry)));
            data.forward_keys = String (gtk_entry_get_text (GTK_ENTRY (forward_keys.entry)));
            data.select_keys = String (gtk_entry_get_text (GTK_ENTRY (select_keys.entry)));
            data.page_up_keys = String (gtk_entry_get_text (GTK_ENTRY (page_up_keys.entry)));
            data.page_down_keys = String (gtk_entry_get_text (GTK_ENTRY (page_down_keys.entry)));

            data.max_key_length = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spin_max_key_length));
            data.show_key_prompt = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle_show_key_prompt));
            data.auto_select = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle_auto_select));
            data.auto_fill = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle_auto_fill));
            data.auto_wildcard = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle_auto_wildcard));
            data.auto_commit = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle_auto_commit));
            data.auto_split = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle_auto_split));
            data.discard_invalid_key = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle_discard_invalid_key));
            data.dynamic_adjust = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle_dynamic_adjust));
            data.always_show_lookup = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle_always_show_lookup));
            data.def_full_width_punct = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle_def_full_width_punct));
            data.def_full_width_letter = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle_def_full_width_letter));

            if (validate_table_properties_data (lib, data))
                break;
        }

        gtk_widget_destroy (dialog);
#if GTK_CHECK_VERSION(3, 0, 0)
#else
        gtk_object_destroy (GTK_OBJECT (tooltips));
#endif
    }

    return result;
}

static bool
validate_table_properties_data (const GenericTableLibrary *lib, const TablePropertiesData &data)
{
    bool ok = true;
    String err;

    if (ok && !data.icon.length () && access (data.icon.c_str (), R_OK) != 0) {
        ok = false;
        err = _("Invalid icon file.");
    }

    if (ok && !data.languages.length ()) {
        ok = false;
        err = _("Invalid languages.");
    }

    if (ok && !data.status_prompt.length ()) {
        ok = false;
        err = _("Invalid status prompt.");
    }

    if (ok && data.multi_wildcard_chars.length ()) {
        for (String::const_iterator i = data.multi_wildcard_chars.begin ();
             i != data.multi_wildcard_chars.end (); ++i) {
            if (lib->is_valid_input_char (*i)) {
                ok = false;
                err = _("Invalid multi wildcard chars.");
                break;
            }
        }
    }

    if (ok && data.single_wildcard_chars.length ()) {
        for (String::const_iterator i = data.single_wildcard_chars.begin ();
             i != data.single_wildcard_chars.end (); ++i) {
            if (lib->is_valid_input_char (*i) ||
                data.multi_wildcard_chars.find (*i) != String::npos) {
                ok = false;
                err = _("Invalid single wildcard chars.");
                break;
            }
        }
    }

    if (ok && !data.commit_keys.length ()) {
        ok = false;
        err = _("Invalid commit keys.");
    }

    if (ok && !data.select_keys.length ()) {
        ok = false;
        err = _("Invalid select keys.");
    }

    if (ok && !data.page_up_keys.length ()) {
        ok = false;
        err = _("Invalid page up keys.");
    }

    if (ok && !data.page_down_keys.length ()) {
        ok = false;
        err = _("Invalid page down keys.");
    }

    if (ok && (data.max_key_length < lib->get_max_key_length () ||
               data.max_key_length > SCIM_GT_MAX_KEY_LENGTH)) {
        ok = false;
        err = _("Invalid max key length.");
    }

    if (!ok) {
        GtkWidget *msg = gtk_message_dialog_new (0,
                                          GTK_DIALOG_MODAL,
                                          GTK_MESSAGE_ERROR,
                                          GTK_BUTTONS_CLOSE,
                                          err.c_str ());
        gtk_dialog_run (GTK_DIALOG (msg));
        gtk_widget_destroy (msg);
    }

    return ok;
}

static void
on_table_properties_clicked (GtkButton *button,
                             gpointer   user_data)
{
    GtkTreeIter  iter;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkWidget *msg;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (__widget_table_list_view));

    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
        GenericTableLibrary *lib;
        gchar               *file;

        gtk_tree_model_get (model, &iter,
                            TABLE_COLUMN_LIBRARY, &lib,
                            TABLE_COLUMN_FILE, &file,
                            -1);

        if (!lib || !file) {
            g_free (file);
            return;
        }

        TablePropertiesData data, olddata;
        gint result;

        data.name                  = utf8_wcstombs (lib->get_name (scim_get_current_locale ()));
        data.author                = utf8_wcstombs (lib->get_author ());
        data.uuid                  = lib->get_uuid ();
        data.serial                = lib->get_serial_number ();
        data.languages             = lib->get_languages ();
        data.icon                  = lib->get_icon_file ();
        data.status_prompt         = utf8_wcstombs (lib->get_status_prompt ());
        data.valid_input_chars     = lib->get_valid_input_chars ();
        data.multi_wildcard_chars  = lib->get_multi_wildcard_chars ();
        data.single_wildcard_chars = lib->get_single_wildcard_chars ();

        data.max_key_length        = lib->get_max_key_length ();
        data.show_key_prompt       = lib->is_show_key_prompt ();
        data.auto_select           = lib->is_auto_select ();
        data.auto_fill             = lib->is_auto_fill ();
        data.auto_wildcard         = lib->is_auto_wildcard ();
        data.auto_commit           = lib->is_auto_commit ();
        data.auto_split            = lib->is_auto_split ();
        data.discard_invalid_key   = lib->is_discard_invalid_key ();
        data.dynamic_adjust        = lib->is_dynamic_adjust ();
        data.always_show_lookup    = lib->is_always_show_lookup ();
        data.def_full_width_punct  = lib->is_def_full_width_punct ();
        data.def_full_width_letter = lib->is_def_full_width_letter ();

        scim_key_list_to_string (data.split_keys, lib->get_split_keys ());
        scim_key_list_to_string (data.commit_keys, lib->get_commit_keys ());
        scim_key_list_to_string (data.forward_keys, lib->get_forward_keys ());
        scim_key_list_to_string (data.select_keys, lib->get_select_keys ());
        scim_key_list_to_string (data.page_up_keys, lib->get_page_up_keys ());
        scim_key_list_to_string (data.page_down_keys, lib->get_page_down_keys ());

        olddata = data;

        result = run_table_properties_dialog (lib, data, test_file_modify (file));

        g_free (file);

        // Save the changes.
        if (result == GTK_RESPONSE_OK) {
            std::vector <KeyEvent> keyevents;

            if (data.icon != olddata.icon) {
                GdkPixbuf * pixbuf = gdk_pixbuf_new_from_file (data.icon.c_str (), NULL);
                scale_pixbuf (&pixbuf, LIST_ICON_SIZE, LIST_ICON_SIZE);

                gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                                    TABLE_COLUMN_ICON, pixbuf,
                                    -1);

                if (pixbuf)
                    g_object_unref (pixbuf);

                lib->set_icon_file (data.icon);
            }

            if (data.languages != olddata.languages)
                lib->set_languages (data.languages);

            if (data.status_prompt != olddata.status_prompt)
                lib->set_status_prompt (utf8_mbstowcs (data.status_prompt));

            if (data.single_wildcard_chars != olddata.single_wildcard_chars)
                lib->set_single_wildcard_chars (data.single_wildcard_chars);

            if (data.multi_wildcard_chars != olddata.multi_wildcard_chars)
                lib->set_multi_wildcard_chars (data.multi_wildcard_chars);

            if (data.max_key_length != olddata.max_key_length)
                lib->set_max_key_length (data.max_key_length);

            if (data.show_key_prompt != olddata.show_key_prompt)
                lib->set_show_key_prompt (data.show_key_prompt);

            if (data.auto_select != olddata.auto_select)
                lib->set_auto_select (data.auto_select);

            if (data.auto_fill != olddata.auto_fill)
                lib->set_auto_fill (data.auto_fill);

            if (data.auto_wildcard != olddata.auto_wildcard)
                lib->set_auto_wildcard (data.auto_wildcard);

            if (data.auto_commit != olddata.auto_commit)
                lib->set_auto_commit (data.auto_commit);

            if (data.auto_split != olddata.auto_split)
                lib->set_auto_split (data.auto_split);

            if (data.discard_invalid_key != olddata.discard_invalid_key)
                lib->set_discard_invalid_key (data.discard_invalid_key);

            if (data.dynamic_adjust != olddata.dynamic_adjust)
                lib->set_dynamic_adjust (data.dynamic_adjust);

            if (data.always_show_lookup != olddata.always_show_lookup)
                lib->set_always_show_lookup (data.always_show_lookup);

            if (data.def_full_width_punct != olddata.def_full_width_punct)
                lib->set_def_full_width_punct (data.def_full_width_punct);

            if (data.def_full_width_letter != olddata.def_full_width_letter)
                lib->set_def_full_width_letter (data.def_full_width_letter);

            if (data.split_keys != olddata.split_keys &&
                scim_string_to_key_list (keyevents, data.split_keys))
                lib->set_split_keys (keyevents);

            if (data.commit_keys != olddata.commit_keys &&
                scim_string_to_key_list (keyevents, data.commit_keys))
                lib->set_commit_keys (keyevents);

            if (data.forward_keys != olddata.forward_keys &&
                scim_string_to_key_list (keyevents, data.forward_keys))
                lib->set_forward_keys (keyevents);

            if (data.select_keys != olddata.select_keys &&
                scim_string_to_key_list (keyevents, data.select_keys))
                lib->set_select_keys (keyevents);

            if (data.page_up_keys != olddata.page_up_keys &&
                scim_string_to_key_list (keyevents, data.page_up_keys))
                lib->set_page_up_keys (keyevents);

            if (data.page_down_keys != olddata.page_down_keys &&
                scim_string_to_key_list (keyevents, data.page_down_keys))
                lib->set_page_down_keys (keyevents);
        }
    }
}

static void
save_all_tables ()
{
    GtkTreeIter iter;
    if (__widget_table_list_model &&
        gtk_tree_model_get_iter_first (GTK_TREE_MODEL (__widget_table_list_model), &iter)) {

        GenericTableLibrary *lib;
        gchar *file;
        gchar *name;
        gboolean is_user;

        do {
            gtk_tree_model_get (GTK_TREE_MODEL (__widget_table_list_model), &iter,
                                TABLE_COLUMN_LIBRARY, &lib,
                                TABLE_COLUMN_FILE, &file,
                                TABLE_COLUMN_NAME, &name,
                                TABLE_COLUMN_IS_USER, &is_user,
                                -1);
            if (lib->updated () && file) {
                if (!lib->save (file, "", "", is_user ? __config_user_table_binary : true)) {
                    GtkWidget *msg = gtk_message_dialog_new (0,
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR,
                                                  GTK_BUTTONS_CLOSE,
                                                  _("Failed to save table %s!"),
                                                  name);
                    gtk_dialog_run (GTK_DIALOG (msg));
                    gtk_widget_destroy (msg);
                }
            }
            g_free (file);
            g_free (name);
        } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (__widget_table_list_model), &iter));
    }
}


/*
vi:ts=4:nowrap:expandtab
*/
