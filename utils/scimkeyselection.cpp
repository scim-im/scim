#include <string.h>
#include <stdio.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#define Uses_SCIM_EVENT

#include "scim_private.h"
#include "scim.h"
#include "scimkeyselection.h"

#ifdef GDK_WINDOWING_X11
#include <X11/Xlib.h>
#include <gdk/gdkx.h>
#include "scim_x11_utils.h"
#endif

using namespace scim;

enum {
    KEY_SELECTION_CHANGED,
    LAST_SIGNAL
};

struct KeyGrabData {
    KeyEvent key;
};

/* GObject, GtkObject methods
 */

static void scim_key_selection_class_init              (ScimKeySelectionClass *klass);
static void scim_key_selection_init                    (ScimKeySelection      *keyselection);
static void scim_key_selection_finalize                (GObject               *object);

static void scim_key_selection_add_key_button_callback (GtkButton             *button,
                                                        ScimKeySelection      *keyselection);

static void scim_key_selection_del_key_button_callback (GtkButton             *button,
                                                        ScimKeySelection      *keyselection);

static void scim_key_selection_list_changed_callback   (GtkTreeSelection      *selection,
                                                        ScimKeySelection      *keyselection);

static void scim_key_grab_button_callback              (GtkButton             *button,
                                                        ScimKeySelection      *keyselection);

static gboolean scim_key_grab_press_callback           (GtkDialog             *dialog,
                                                        GdkEventKey           *event,
                                                        KeyGrabData           *data);

static gboolean scim_key_grab_release_callback         (GtkDialog             *dialog,
                                                        GdkEventKey           *event,
                                                        KeyGrabData           *data);

static void scim_key_selection_set_key_event           (ScimKeySelection      *keyselection,
                                                        KeyEvent               event);

static GtkWidgetClass *parent_class = NULL;

static gint key_selection_signals[LAST_SIGNAL] = { 0 };

static GType key_selection_type = 0;

void
scim_key_selection_register_type (GTypeModule *type_module)
{
    static const GTypeInfo key_selection_info =
    {
        sizeof (ScimKeySelectionClass),
        NULL,
        NULL,
        (GClassInitFunc) scim_key_selection_class_init,
        NULL,
        NULL,
        sizeof (ScimKeySelection),
        0,
        (GInstanceInitFunc) scim_key_selection_init,
        0
    };

    if (!key_selection_type) {
        if (type_module)
            key_selection_type = g_type_module_register_type (
                                    type_module,
#if GTK_CHECK_VERSION(3, 0, 0)
                                    GTK_TYPE_BOX,
#else
                                    GTK_TYPE_VBOX,
#endif
                                    "SCIM_ScimKeySelection",
                                    &key_selection_info,
                                    (GTypeFlags) 0);
        else
            key_selection_type = g_type_register_static (
#if GTK_CHECK_VERSION(3, 0, 0)
                                    GTK_TYPE_BOX,
#else
                                    GTK_TYPE_VBOX,
#endif
                                    "SCIM_ScimKeySelection",
                                    &key_selection_info,
                                    (GTypeFlags) 0);
    }
}

GType
scim_key_selection_get_type (void)
{
    if (!key_selection_type)
        scim_key_selection_register_type (NULL);

    return key_selection_type;
}

static void
scim_key_selection_class_init (ScimKeySelectionClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
#if GTK_CHECK_VERSION(3, 0, 0)
#else
    GtkObjectClass *object_class;
    GtkWidgetClass *widget_class;

    object_class = (GtkObjectClass*) klass;
    widget_class = (GtkWidgetClass*) klass;
#endif
    parent_class = (GtkWidgetClass*) g_type_class_peek_parent (klass);

    gobject_class->finalize = scim_key_selection_finalize;

    key_selection_signals[KEY_SELECTION_CHANGED] =
        g_signal_new ("key-selection-changed",
#if GTK_CHECK_VERSION(3, 0, 0)
                    G_TYPE_FROM_CLASS (gobject_class),
#else
                    G_TYPE_FROM_CLASS (object_class),
#endif
                    G_SIGNAL_RUN_FIRST,
                    G_STRUCT_OFFSET (ScimKeySelectionClass, changed),
                    NULL,
                    NULL,
                    g_cclosure_marshal_VOID__VOID,
                    G_TYPE_NONE, 0);

    klass->changed = NULL;
}

static void
scim_key_selection_init (ScimKeySelection *keyselection)
{
    GtkWidget *table;
    GtkWidget *hbox;
    GtkWidget *frame;
    GtkWidget *label;

    GtkWidget *scrolledwindow;
    GtkWidget *button;

    GtkCellRenderer *list_cell;
    GtkTreeViewColumn *list_column;

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (keyselection), frame, TRUE, TRUE, 0);

    label = gtk_label_new (NULL);
    gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _("Selected _Keys:"));
    gtk_widget_show (label);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    // Create keys list view
    scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (scrolledwindow);
    gtk_container_add (GTK_CONTAINER(frame), scrolledwindow);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow),
                                         GTK_SHADOW_ETCHED_IN);

    keyselection->list_view = gtk_tree_view_new ();
    gtk_widget_show (keyselection->list_view);
    gtk_container_add (GTK_CONTAINER (scrolledwindow), keyselection->list_view);
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (keyselection->list_view), FALSE);

    gtk_label_set_mnemonic_widget (GTK_LABEL (label), keyselection->list_view);

    keyselection->list_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (keyselection->list_view));
    gtk_tree_selection_set_mode (keyselection->list_selection, GTK_SELECTION_BROWSE);

    g_signal_connect (G_OBJECT (keyselection->list_selection), "changed",
                      G_CALLBACK (scim_key_selection_list_changed_callback),
                      keyselection);

    // Create key list column.
    list_cell = gtk_cell_renderer_text_new ();
    list_column = gtk_tree_view_column_new_with_attributes (
                            NULL, list_cell, "text", 0, NULL);

    gtk_tree_view_append_column (GTK_TREE_VIEW (keyselection->list_view), list_column);

    // Create key list model
    keyselection->list_model = gtk_list_store_new (1, G_TYPE_STRING);

    gtk_tree_view_set_model (GTK_TREE_VIEW (keyselection->list_view),
                             GTK_TREE_MODEL (keyselection->list_model));

#if GTK_CHECK_VERSION(3, 4, 0)
    table = gtk_grid_new ();
    gtk_grid_set_row_spacing (GTK_GRID(table), 4);
    gtk_grid_set_column_spacing (GTK_GRID(table), 4);
#else
    table = gtk_table_new (2, 3, FALSE);
#endif
    gtk_widget_show (table);
    gtk_box_pack_start (GTK_BOX (keyselection), table, FALSE, FALSE, 0);

    label = gtk_label_new (_("Key Code:"));
    gtk_widget_show (label);
#if GTK_CHECK_VERSION(3, 4, 0)
    gtk_widget_set_halign (label, GTK_ALIGN_FILL);
    gtk_grid_attach (GTK_GRID (table), label, 0, 1, 1, 1);
#else
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 4, 4);
#endif
#if GTK_CHECK_VERSION(3, 14, 0)
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
#endif

#if GTK_CHECK_VERSION(3, 2, 0)
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
#else
    hbox = gtk_hbox_new (FALSE, 0);
#endif
    gtk_widget_show (hbox);
#if GTK_CHECK_VERSION(3, 4, 0)
    gtk_widget_set_halign (hbox, GTK_ALIGN_FILL);
    gtk_grid_attach (GTK_GRID (table), hbox, 1, 0, 1, 1);
#else
    gtk_table_attach (GTK_TABLE (table), hbox, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
                      (GtkAttachOptions) (0), 4, 4);
#endif

    keyselection->key_code = gtk_entry_new ();
    gtk_widget_show (keyselection->key_code);
    gtk_box_pack_start (GTK_BOX (hbox), keyselection->key_code, TRUE, TRUE, 2);
    gtk_editable_set_editable (GTK_EDITABLE (keyselection->key_code), FALSE);

    button = gtk_button_new_with_label (_("..."));
    gtk_widget_show (button);
    gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 2);
    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (scim_key_grab_button_callback), keyselection);

    label = gtk_label_new (_("Modifiers:"));
    gtk_widget_show (label);
#if GTK_CHECK_VERSION(3, 4, 0)
    gtk_widget_set_halign (label, GTK_ALIGN_FILL);
    gtk_grid_attach (GTK_GRID (table), label, 0, 1, 1, 1);
#else
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 4, 4);
#endif
#if GTK_CHECK_VERSION(3, 14, 0)
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
#endif

#if GTK_CHECK_VERSION(3, 2, 0)
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
#else
    hbox = gtk_hbox_new (FALSE, 0);
#endif
    gtk_widget_show (hbox);
#if GTK_CHECK_VERSION(3, 2, 0)
    gtk_widget_set_halign (hbox, GTK_ALIGN_FILL);
    gtk_grid_attach (GTK_GRID (table), hbox, 1, 1, 1, 1);
#else
    gtk_table_attach (GTK_TABLE (table), hbox, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
                      (GtkAttachOptions) (0), 4, 4);
#endif

    keyselection->toggle_ctrl = gtk_check_button_new_with_mnemonic (_("_Ctrl"));
    gtk_widget_show (keyselection->toggle_ctrl);
    gtk_box_pack_start (GTK_BOX (hbox), keyselection->toggle_ctrl, TRUE, TRUE, 2);

    keyselection->toggle_alt = gtk_check_button_new_with_mnemonic (_("A_lt"));
    gtk_widget_show (keyselection->toggle_alt);
    gtk_box_pack_start (GTK_BOX (hbox), keyselection->toggle_alt, TRUE, TRUE, 2);

    keyselection->toggle_shift = gtk_check_button_new_with_mnemonic (_("_Shift"));
    gtk_widget_show (keyselection->toggle_shift);
    gtk_box_pack_start (GTK_BOX (hbox), keyselection->toggle_shift, TRUE, TRUE, 2);

    keyselection->toggle_release = gtk_check_button_new_with_mnemonic (_("_Release"));
    gtk_widget_show (keyselection->toggle_release);
    gtk_box_pack_start (GTK_BOX (hbox), keyselection->toggle_release, TRUE, TRUE, 2);

#if GTK_CHECK_VERSION(3, 2, 0)
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
#else
    hbox = gtk_hbox_new (FALSE, 0);
#endif
    gtk_widget_show (hbox);
#if GTK_CHECK_VERSION(3, 4, 0)
    gtk_widget_set_halign (hbox, GTK_ALIGN_FILL);
    gtk_grid_attach (GTK_GRID (table), hbox, 1, 2, 1, 1);
#else
    gtk_table_attach (GTK_TABLE (table), hbox, 1, 2, 2, 3,
                      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
                      (GtkAttachOptions) (0), 4, 4);
#endif

    keyselection->toggle_meta = gtk_check_button_new_with_mnemonic (_("_Meta"));
    gtk_widget_show (keyselection->toggle_meta);
    gtk_box_pack_start (GTK_BOX (hbox), keyselection->toggle_meta, TRUE, TRUE, 2);

    keyselection->toggle_super = gtk_check_button_new_with_mnemonic (_("S_uper"));
    gtk_widget_show (keyselection->toggle_super);
    gtk_box_pack_start (GTK_BOX (hbox), keyselection->toggle_super, TRUE, TRUE, 2);

    keyselection->toggle_hyper = gtk_check_button_new_with_mnemonic (_("_Hyper"));
    gtk_widget_show (keyselection->toggle_hyper);
    gtk_box_pack_start (GTK_BOX (hbox), keyselection->toggle_hyper, TRUE, TRUE, 2);


    keyselection->toggle_capslock = gtk_check_button_new_with_mnemonic (_("Ca_psLock"));
    gtk_widget_show (keyselection->toggle_capslock);
    gtk_box_pack_start (GTK_BOX (hbox), keyselection->toggle_capslock, TRUE, TRUE, 2);

#if GTK_CHECK_VERSION(3, 2, 0)
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
#else
    hbox = gtk_hbox_new (TRUE, 0);
#endif
    gtk_widget_show (hbox);
    gtk_box_pack_start (GTK_BOX (keyselection), hbox, FALSE, FALSE, 4);

#if GTK_CHECK_VERSION(3, 14, 0)
    button = gtk_button_new_from_icon_name ("list-add", GTK_ICON_SIZE_BUTTON);
#else
    button = gtk_button_new_from_stock ("gtk-add");
#endif
    gtk_widget_show (button);
    gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 4);
    g_signal_connect ((gpointer) button, "clicked",
                      G_CALLBACK (scim_key_selection_add_key_button_callback),
                      keyselection);

#if GTK_CHECK_VERSION(3, 14, 0)
    button = gtk_button_new_from_icon_name ("list-delete", GTK_ICON_SIZE_BUTTON);
#else
    button = gtk_button_new_from_stock ("gtk-delete");
#endif
    gtk_widget_show (button);
    gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 4);
    g_signal_connect ((gpointer) button, "clicked",
                      G_CALLBACK (scim_key_selection_del_key_button_callback),
                      keyselection);

    // Create key names list
    keyselection->keys = NULL;
}

static void
scim_key_selection_finalize (GObject *object)
{
    ScimKeySelection *keyselection = SCIM_KEY_SELECTION (object);
    if (keyselection->keys)
        g_free (keyselection->keys);

    G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
scim_key_selection_add_key_button_callback (GtkButton        *button,
                                            ScimKeySelection *keyselection)
{
    GtkTreeIter iter;
    String key;
    String key_code;

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (keyselection->toggle_ctrl)))
        key += String ("Control+");
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (keyselection->toggle_alt)))
        key += String ("Alt+");
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (keyselection->toggle_shift)))
        key += String ("Shift+");
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (keyselection->toggle_meta)))
        key += String ("Meta+");
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (keyselection->toggle_super)))
        key += String ("Super+");
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (keyselection->toggle_hyper)))
        key += String ("Hyper+");
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (keyselection->toggle_capslock)))
        key += String ("CapsLock+");

    key_code = String (gtk_entry_get_text (GTK_ENTRY (keyselection->key_code)));
    if (!key_code.length ()){
      // empty key code is not allowed.
      GtkWidget *dialog;
      dialog = gtk_message_dialog_new (NULL,
                                       GTK_DIALOG_MODAL,
                                       GTK_MESSAGE_ERROR,
                                       GTK_BUTTONS_CLOSE,
                                       _("Please enter a Key Code first."));
      gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (dialog);
      return;
    }
    key += key_code;

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (keyselection->toggle_release)))
        key += String ("+KeyRelease");

    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (keyselection->list_model), &iter)) {
        gchar * keystr;

        do {
            gtk_tree_model_get (GTK_TREE_MODEL (keyselection->list_model), &iter,
                                0, &keystr, -1);

            if (keystr && String (keystr) == key)
                return;

        } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (keyselection->list_model), &iter));
    }

    gtk_list_store_append (keyselection->list_model, &iter);
    gtk_list_store_set (keyselection->list_model, &iter,
                        0, key.c_str (), -1);

    g_signal_emit_by_name (keyselection, "key-selection-changed");
}

static void
scim_key_selection_del_key_button_callback (GtkButton       *button,
                                           ScimKeySelection *keyselection)
{
    GtkTreeIter iter;
    GtkTreeModel *model;

    if (gtk_tree_selection_get_selected (keyselection->list_selection, &model, &iter)) {
        gtk_list_store_remove (keyselection->list_model, &iter);
        g_signal_emit_by_name (keyselection, "key-selection-changed");
    }
}

static void
scim_key_grab_button_callback (GtkButton        *button,
                               ScimKeySelection *keyselection)
{
    KeyGrabData key_grab_data;
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new (NULL,
                                     GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_INFO,
                                     GTK_BUTTONS_CANCEL,
                                     _("Press a key (or a key combination).\n"
                                       "This dialog will be closed when the key is released."));

    gtk_window_set_title (GTK_WINDOW (dialog), _("Grabbing a key."));

    g_signal_connect (G_OBJECT (dialog), "key-press-event",
                      G_CALLBACK (scim_key_grab_press_callback), &key_grab_data);

    g_signal_connect (G_OBJECT (dialog), "key-release-event",
                      G_CALLBACK (scim_key_grab_release_callback), &key_grab_data);

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK) {
        scim_key_selection_set_key_event (keyselection, key_grab_data.key);
    }

    gtk_widget_destroy (dialog);
}

static KeyEvent
keyevent_gdk_to_scim (const GdkEventKey &gdkevent)
{
    KeyEvent key;

    // Use Key Symbole provided by gtk.
    key.code = gdkevent.keyval;

#ifdef GDK_WINDOWING_X11
    key.mask = scim_x11_keymask_x11_to_scim (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()), gdkevent.state);
#else
    if (gdkevent.state & GDK_SHIFT_MASK) key.mask |=SCIM_KEY_ShiftMask;
    if (gdkevent.state & GDK_LOCK_MASK) key.mask |=SCIM_KEY_CapsLockMask;
    if (gdkevent.state & GDK_CONTROL_MASK) key.mask |=SCIM_KEY_ControlMask;
    if (gdkevent.state & GDK_MOD1_MASK) key.mask |=SCIM_KEY_AltMask;
    if (gdkevent.state & GDK_MOD2_MASK) key.mask |=SCIM_KEY_NumLockMask;
#endif

    if (gdkevent.type == GDK_KEY_RELEASE) key.mask |= SCIM_KEY_ReleaseMask;

    return key;
}

static gboolean
scim_key_grab_press_callback (GtkDialog   *dialog,
                              GdkEventKey *event,
                              KeyGrabData *data)
{
    data->key = keyevent_gdk_to_scim (*event);

    return TRUE;
}

static gboolean
scim_key_grab_release_callback (GtkDialog   *dialog,
                                GdkEventKey *event,
                                KeyGrabData *data)
{
    KeyEvent key = keyevent_gdk_to_scim (*event);

    if (key.code == data->key.code) {
        data->key.mask = key.mask;
        if (data->key.code >= SCIM_KEY_Shift_L && data->key.code <= SCIM_KEY_Hyper_R)
            data->key.mask |= SCIM_KEY_ReleaseMask;
        else
            data->key.mask &= (~ SCIM_KEY_ReleaseMask);

        gtk_dialog_response (dialog, GTK_RESPONSE_OK);
    } else {
        gtk_dialog_response (dialog, GTK_RESPONSE_CANCEL);
    }

    return TRUE;
}

/* public api
 */
GtkWidget*
scim_key_selection_new (void)
{
  return GTK_WIDGET (g_object_new (SCIM_TYPE_KEY_SELECTION,
#if GTK_CHECK_VERSION(3, 0, 0)
                                   "orientation", GTK_ORIENTATION_VERTICAL,
#endif
                                   NULL));
}

void
scim_key_selection_append_keys (ScimKeySelection *keyselection,
                               const gchar     *keys)
{
    g_return_if_fail (SCIM_IS_KEY_SELECTION (keyselection));
    g_return_if_fail (keys != NULL);

    KeyEventList keylist;

    if (!scim_string_to_key_list (keylist, keys))
        return;

    GtkTreeIter iter;
    GtkTreeModel *model;

    String str;

    for (size_t i = 0; i < keylist.size (); ++ i) {
        if (scim_key_to_string (str, keylist [i])) {
            gtk_list_store_append (keyselection->list_model, &iter);
            gtk_list_store_set (keyselection->list_model, &iter,
                                0, str.c_str (), -1);
        }
    }
}

void
scim_key_selection_set_keys (ScimKeySelection *keyselection,
                             const gchar      *keys)
{
    g_return_if_fail (SCIM_IS_KEY_SELECTION (keyselection));

    gtk_list_store_clear (keyselection->list_model);
    scim_key_selection_append_keys (keyselection, keys);
}

const gchar*
scim_key_selection_get_keys (ScimKeySelection *keyselection)
{
    g_return_val_if_fail (SCIM_IS_KEY_SELECTION (keyselection), NULL);

    if (keyselection->keys)
        g_free (keyselection->keys);

    keyselection->keys = NULL;

    GtkTreeIter iter;

    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (keyselection->list_model), &iter)) {
        std::vector <String> keylist;
        gchar * keystr;

        do {
            gtk_tree_model_get (GTK_TREE_MODEL (keyselection->list_model), &iter,
                                0, &keystr, -1);

            if (keystr) keylist.push_back (keystr);
        } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (keyselection->list_model), &iter));

        if (keylist.size ())
            keyselection->keys = g_strdup (scim_combine_string_list (keylist).c_str ());
    }

    return keyselection->keys;
}

static void
scim_key_selection_list_changed_callback (GtkTreeSelection *selection,
                                          ScimKeySelection *keyselection)
{
    GtkTreeModel *model;
    GtkTreeIter   iter;
    gchar        *keystr;

    KeyEvent      keyevent;

    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
        gtk_tree_model_get (model, &iter, 0, &keystr, -1);

        if (scim_string_to_key (keyevent, String (keystr)))
            scim_key_selection_set_key_event (keyselection, keyevent);
    }
}

static void
scim_key_selection_set_key_event (ScimKeySelection *keyselection,
                                  KeyEvent          event)
{
    g_return_if_fail (SCIM_IS_KEY_SELECTION (keyselection));

    gtk_toggle_button_set_active (
        GTK_TOGGLE_BUTTON (keyselection->toggle_ctrl),
        event.is_control_down ());
    gtk_toggle_button_set_active (
        GTK_TOGGLE_BUTTON (keyselection->toggle_alt),
        event.is_alt_down ());
    gtk_toggle_button_set_active (
        GTK_TOGGLE_BUTTON (keyselection->toggle_shift),
        event.is_shift_down ());
    gtk_toggle_button_set_active (
        GTK_TOGGLE_BUTTON (keyselection->toggle_meta),
        event.is_meta_down ());
    gtk_toggle_button_set_active (
        GTK_TOGGLE_BUTTON (keyselection->toggle_super),
        event.is_super_down ());
    gtk_toggle_button_set_active (
        GTK_TOGGLE_BUTTON (keyselection->toggle_hyper),
        event.is_hyper_down ());
    gtk_toggle_button_set_active (
        GTK_TOGGLE_BUTTON (keyselection->toggle_capslock),
        event.is_caps_lock_down ());
    gtk_toggle_button_set_active (
        GTK_TOGGLE_BUTTON (keyselection->toggle_release),
        event.is_key_release ());

    event.mask = 0;
    String str;

    if (scim_key_to_string (str, event)) {
        gtk_entry_set_text (GTK_ENTRY (keyselection->key_code),
                            str.c_str ());
    }
}

/*****************************************************************************
 * ScimKeySelectionDialog
 *****************************************************************************/
static GtkWidgetClass *dialog_parent_class = NULL;

static GType key_selection_dialog_type = 0;

static void scim_key_selection_dialog_class_init (ScimKeySelectionDialogClass *klass);
static void scim_key_selection_dialog_init (ScimKeySelectionDialog *keyseldialog);

void
scim_key_selection_dialog_register_type (GTypeModule *type_module)
{
    static const GTypeInfo key_selection_dialog_info =
    {
        sizeof (ScimKeySelectionDialogClass),
        NULL,
        NULL,
        (GClassInitFunc) scim_key_selection_dialog_class_init,
        NULL,
        NULL,
        sizeof (ScimKeySelectionDialog),
        0,
        (GInstanceInitFunc) scim_key_selection_dialog_init,
        0
    };

    if (!key_selection_dialog_type) {
        if (type_module)
            key_selection_dialog_type = g_type_module_register_type (
                                    type_module,
                                    GTK_TYPE_DIALOG,
                                    "SCIM_ScimKeySelectionDialog",
                                    &key_selection_dialog_info,
                                    (GTypeFlags) 0);
        else
            key_selection_dialog_type = g_type_register_static (
                                    GTK_TYPE_DIALOG,
                                    "SCIM_ScimKeySelectionDialog",
                                    &key_selection_dialog_info,
                                    (GTypeFlags) 0);
    }
}

GType
scim_key_selection_dialog_get_type (void)
{
    if (!key_selection_dialog_type)
        scim_key_selection_dialog_register_type (NULL);

    return key_selection_dialog_type;
}

static void
scim_key_selection_dialog_class_init (ScimKeySelectionDialogClass *klass)
{
    dialog_parent_class = (GtkWidgetClass*) g_type_class_peek_parent (klass);
}

static void
scim_key_selection_dialog_init (ScimKeySelectionDialog *keyseldialog)
{
    GtkDialog *dialog;

#if !GTK_CHECK_VERSION(3, 10, 0)
    gtk_widget_push_composite_child ();
#endif

    dialog = GTK_DIALOG (keyseldialog);

    gtk_container_set_border_width (GTK_CONTAINER (keyseldialog), 4);
    gtk_window_set_resizable (GTK_WINDOW (keyseldialog), TRUE);

#if GTK_CHECK_VERSION(3, 0, 0)
    keyseldialog->content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
#else
    keyseldialog->main_vbox = dialog->vbox;
#endif

    keyseldialog->keysel = scim_key_selection_new ();
    gtk_container_set_border_width (GTK_CONTAINER (keyseldialog->keysel), 4);
    gtk_widget_show (keyseldialog->keysel);
#if GTK_CHECK_VERSION(3, 0, 0)
    gtk_box_pack_start (GTK_BOX (keyseldialog->content_area),
#else
    gtk_box_pack_start (GTK_BOX (keyseldialog->main_vbox),
#endif
                        keyseldialog->keysel, TRUE, TRUE, 0);

    /* Create the action area */
#if GTK_CHECK_VERSION(3, 0, 0)
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    keyseldialog->action_area = gtk_dialog_get_action_area (GTK_DIALOG (dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS
#else
    keyseldialog->action_area = dialog->action_area;
#endif

    keyseldialog->cancel_button = gtk_dialog_add_button (dialog,
#if GTK_CHECK_VERSION(3, 10, 0)
                                                        _("_Cancel"),
#else
                                                        GTK_STOCK_CANCEL,
#endif
                                                        GTK_RESPONSE_CANCEL);

    keyseldialog->ok_button = gtk_dialog_add_button (dialog,
#if GTK_CHECK_VERSION(3, 10, 0)
                                                    _("_OK"),
#else
                                                    GTK_STOCK_OK,
#endif
                                                    GTK_RESPONSE_OK);
    gtk_widget_grab_default (keyseldialog->ok_button);

    gtk_window_set_title (GTK_WINDOW (keyseldialog),
                          _("Key Selection"));

#if GTK_CHECK_VERSION(2, 22, 0)
#else
    gtk_dialog_set_has_separator (GTK_DIALOG (dialog), TRUE);
#endif

#if !GTK_CHECK_VERSION(3, 10, 0)
    gtk_widget_pop_composite_child ();
#endif
}

GtkWidget*
scim_key_selection_dialog_new (const gchar *title)
{
    ScimKeySelectionDialog *keyseldialog;

    keyseldialog= (ScimKeySelectionDialog *) g_object_new (SCIM_TYPE_KEY_SELECTION_DIALOG, NULL);

    if (title)
        gtk_window_set_title (GTK_WINDOW (keyseldialog), title);

    return GTK_WIDGET (keyseldialog);
}

const gchar*
scim_key_selection_dialog_get_keys (ScimKeySelectionDialog *ksd)
{
    return scim_key_selection_get_keys (SCIM_KEY_SELECTION (ksd->keysel));
}

void
scim_key_selection_dialog_set_keys (ScimKeySelectionDialog *ksd,
                                    const gchar            *keys)
{
    return scim_key_selection_set_keys (SCIM_KEY_SELECTION (ksd->keysel), keys);
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
