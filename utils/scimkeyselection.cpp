#include <string.h>
#include <stdio.h>

#include <glib.h>
#include <gtk/gtk.h>

#define Uses_SCIM_EVENT

#include "scim_private.h"
#include "scim.h"
#include "scimkeyselection.h"

using namespace scim;

enum {
    KEY_SELECTION_CHANGED,
    LAST_SIGNAL
};

/* Data carried through the (asynchronous) key-grab dialog.  GTK4 removed
 * gtk_dialog_run(), so the grab result is applied from the release callback
 * instead of a blocking return value. */
struct KeyGrabData {
    ScimKeySelection *keyselection;
    GtkWidget        *dialog;
    KeyEvent          key;
    gboolean          got_press;
};

/* GObject methods
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
                                    GTK_TYPE_BOX,
                                    "SCIM_ScimKeySelection",
                                    &key_selection_info,
                                    (GTypeFlags) 0);
        else
            key_selection_type = g_type_register_static (
                                    GTK_TYPE_BOX,
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

    parent_class = (GtkWidgetClass*) g_type_class_peek_parent (klass);

    gobject_class->finalize = scim_key_selection_finalize;

    key_selection_signals[KEY_SELECTION_CHANGED] =
        g_signal_new ("key-selection-changed",
                    G_TYPE_FROM_CLASS (gobject_class),
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
    GtkWidget *grid;
    GtkWidget *hbox;
    GtkWidget *frame;
    GtkWidget *label;

    GtkWidget *scrolledwindow;
    GtkWidget *button;

    GtkCellRenderer *list_cell;
    GtkTreeViewColumn *list_column;

    frame = gtk_frame_new (NULL);
    gtk_widget_set_vexpand (frame, TRUE);
    gtk_box_append (GTK_BOX (keyselection), frame);

    label = gtk_label_new (NULL);
    gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _("Selected _Keys:"));
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);

    // Create keys list view
    scrolledwindow = gtk_scrolled_window_new ();
    gtk_frame_set_child (GTK_FRAME (frame), scrolledwindow);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_has_frame (GTK_SCROLLED_WINDOW (scrolledwindow), TRUE);

    keyselection->list_view = gtk_tree_view_new ();
    gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolledwindow), keyselection->list_view);
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

    grid = gtk_grid_new ();
    gtk_grid_set_row_spacing (GTK_GRID (grid), 4);
    gtk_grid_set_column_spacing (GTK_GRID (grid), 4);
    gtk_box_append (GTK_BOX (keyselection), grid);

    // Key Code row
    label = gtk_label_new (_("Key Code:"));
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
    gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand (hbox, TRUE);
    gtk_grid_attach (GTK_GRID (grid), hbox, 1, 0, 1, 1);

    keyselection->key_code = gtk_entry_new ();
    gtk_widget_set_hexpand (keyselection->key_code, TRUE);
    gtk_editable_set_editable (GTK_EDITABLE (keyselection->key_code), FALSE);
    gtk_box_append (GTK_BOX (hbox), keyselection->key_code);

    button = gtk_button_new_with_label (_("..."));
    gtk_box_append (GTK_BOX (hbox), button);
    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (scim_key_grab_button_callback), keyselection);

    // Modifiers label
    label = gtk_label_new (_("Modifiers:"));
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
    gtk_grid_attach (GTK_GRID (grid), label, 0, 1, 1, 1);

    // First modifier row
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand (hbox, TRUE);
    gtk_grid_attach (GTK_GRID (grid), hbox, 1, 1, 1, 1);

    keyselection->toggle_ctrl = gtk_check_button_new_with_mnemonic (_("_Ctrl"));
    gtk_box_append (GTK_BOX (hbox), keyselection->toggle_ctrl);

    keyselection->toggle_alt = gtk_check_button_new_with_mnemonic (_("A_lt"));
    gtk_box_append (GTK_BOX (hbox), keyselection->toggle_alt);

    keyselection->toggle_shift = gtk_check_button_new_with_mnemonic (_("_Shift"));
    gtk_box_append (GTK_BOX (hbox), keyselection->toggle_shift);

    keyselection->toggle_release = gtk_check_button_new_with_mnemonic (_("_Release"));
    gtk_box_append (GTK_BOX (hbox), keyselection->toggle_release);

    // Second modifier row
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand (hbox, TRUE);
    gtk_grid_attach (GTK_GRID (grid), hbox, 1, 2, 1, 1);

    keyselection->toggle_meta = gtk_check_button_new_with_mnemonic (_("_Meta"));
    gtk_box_append (GTK_BOX (hbox), keyselection->toggle_meta);

    keyselection->toggle_super = gtk_check_button_new_with_mnemonic (_("S_uper"));
    gtk_box_append (GTK_BOX (hbox), keyselection->toggle_super);

    keyselection->toggle_hyper = gtk_check_button_new_with_mnemonic (_("_Hyper"));
    gtk_box_append (GTK_BOX (hbox), keyselection->toggle_hyper);

    keyselection->toggle_capslock = gtk_check_button_new_with_mnemonic (_("Ca_psLock"));
    gtk_box_append (GTK_BOX (hbox), keyselection->toggle_capslock);

    // Add/Delete buttons row
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_box_set_homogeneous (GTK_BOX (hbox), TRUE);
    gtk_box_append (GTK_BOX (keyselection), hbox);

    button = gtk_button_new_from_icon_name ("list-add");
    gtk_widget_set_hexpand (button, TRUE);
    gtk_box_append (GTK_BOX (hbox), button);
    g_signal_connect ((gpointer) button, "clicked",
                      G_CALLBACK (scim_key_selection_add_key_button_callback),
                      keyselection);

    button = gtk_button_new_from_icon_name ("list-remove");
    gtk_widget_set_hexpand (button, TRUE);
    gtk_box_append (GTK_BOX (hbox), button);
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

    if (gtk_check_button_get_active (GTK_CHECK_BUTTON (keyselection->toggle_ctrl)))
        key += String ("Control+");
    if (gtk_check_button_get_active (GTK_CHECK_BUTTON (keyselection->toggle_alt)))
        key += String ("Alt+");
    if (gtk_check_button_get_active (GTK_CHECK_BUTTON (keyselection->toggle_shift)))
        key += String ("Shift+");
    if (gtk_check_button_get_active (GTK_CHECK_BUTTON (keyselection->toggle_meta)))
        key += String ("Meta+");
    if (gtk_check_button_get_active (GTK_CHECK_BUTTON (keyselection->toggle_super)))
        key += String ("Super+");
    if (gtk_check_button_get_active (GTK_CHECK_BUTTON (keyselection->toggle_hyper)))
        key += String ("Hyper+");
    if (gtk_check_button_get_active (GTK_CHECK_BUTTON (keyselection->toggle_capslock)))
        key += String ("CapsLock+");

    key_code = String (gtk_editable_get_text (GTK_EDITABLE (keyselection->key_code)));
    if (!key_code.length ()){
      // empty key code is not allowed.
      GtkAlertDialog *alert = gtk_alert_dialog_new ("%s", _("Please enter a Key Code first."));
      gtk_alert_dialog_show (alert, GTK_WINDOW (gtk_widget_get_root (GTK_WIDGET (keyselection))));
      g_object_unref (alert);
      return;
    }
    key += key_code;

    if (gtk_check_button_get_active (GTK_CHECK_BUTTON (keyselection->toggle_release)))
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

static KeyEvent
keyevent_gtk_to_scim (guint keyval, GdkModifierType state, gboolean release)
{
    KeyEvent key;

    // Use the key symbol provided by gtk.
    key.code = keyval;

    if (state & GDK_SHIFT_MASK)   key.mask |= SCIM_KEY_ShiftMask;
    if (state & GDK_LOCK_MASK)    key.mask |= SCIM_KEY_CapsLockMask;
    if (state & GDK_CONTROL_MASK) key.mask |= SCIM_KEY_ControlMask;
    if (state & GDK_ALT_MASK)     key.mask |= SCIM_KEY_AltMask;
    if (state & GDK_SUPER_MASK)   key.mask |= SCIM_KEY_SuperMask;
    if (state & GDK_HYPER_MASK)   key.mask |= SCIM_KEY_HyperMask;
    if (state & GDK_META_MASK)    key.mask |= SCIM_KEY_MetaMask;

    if (release) key.mask |= SCIM_KEY_ReleaseMask;

    return key;
}

static gboolean
scim_key_grab_key_pressed (GtkEventControllerKey *controller,
                           guint keyval, guint keycode, GdkModifierType state,
                           KeyGrabData *data)
{
    data->key = keyevent_gtk_to_scim (keyval, state, FALSE);
    data->got_press = TRUE;

    return TRUE;
}

static void
scim_key_grab_key_released (GtkEventControllerKey *controller,
                            guint keyval, guint keycode, GdkModifierType state,
                            KeyGrabData *data)
{
    KeyEvent key = keyevent_gtk_to_scim (keyval, state, FALSE);

    if (data->got_press && key.code == data->key.code) {
        data->key.mask = key.mask;
        if (data->key.code >= SCIM_KEY_Shift_L && data->key.code <= SCIM_KEY_Hyper_R)
            data->key.mask |= SCIM_KEY_ReleaseMask;
        else
            data->key.mask &= (~ SCIM_KEY_ReleaseMask);

        scim_key_selection_set_key_event (data->keyselection, data->key);
    }

    gtk_window_destroy (GTK_WINDOW (data->dialog));
}

static void
scim_key_grab_data_free (gpointer data, GObject * /*where_the_object_was*/)
{
    g_free (data);
}

static void
scim_key_grab_button_callback (GtkButton        *button,
                               ScimKeySelection *keyselection)
{
    KeyGrabData *data = g_new0 (KeyGrabData, 1);
    data->keyselection = keyselection;

    GtkWidget *dialog = gtk_window_new ();
    data->dialog = dialog;

    gtk_window_set_title (GTK_WINDOW (dialog), _("Grabbing a key."));
    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

    GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (keyselection));
    if (root && GTK_IS_WINDOW (root))
        gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (root));

    GtkWidget *box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_start (box, 12);
    gtk_widget_set_margin_end (box, 12);
    gtk_widget_set_margin_top (box, 12);
    gtk_widget_set_margin_bottom (box, 12);
    gtk_window_set_child (GTK_WINDOW (dialog), box);

    GtkWidget *label = gtk_label_new (_("Press a key (or a key combination).\n"
                                        "This dialog will be closed when the key is released."));
    gtk_box_append (GTK_BOX (box), label);

    GtkWidget *cancel = gtk_button_new_with_mnemonic (_("_Cancel"));
    gtk_box_append (GTK_BOX (box), cancel);
    g_signal_connect_swapped (cancel, "clicked", G_CALLBACK (gtk_window_destroy), dialog);

    GtkEventController *controller = gtk_event_controller_key_new ();
    g_signal_connect (controller, "key-pressed", G_CALLBACK (scim_key_grab_key_pressed), data);
    g_signal_connect (controller, "key-released", G_CALLBACK (scim_key_grab_key_released), data);
    gtk_widget_add_controller (dialog, controller);

    g_object_weak_ref (G_OBJECT (dialog), scim_key_grab_data_free, data);

    gtk_window_present (GTK_WINDOW (dialog));
}

/* public api
 */
GtkWidget*
scim_key_selection_new (void)
{
  return GTK_WIDGET (g_object_new (SCIM_TYPE_KEY_SELECTION,
                                   "orientation", GTK_ORIENTATION_VERTICAL,
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

    gtk_check_button_set_active (
        GTK_CHECK_BUTTON (keyselection->toggle_ctrl),
        event.is_control_down ());
    gtk_check_button_set_active (
        GTK_CHECK_BUTTON (keyselection->toggle_alt),
        event.is_alt_down ());
    gtk_check_button_set_active (
        GTK_CHECK_BUTTON (keyselection->toggle_shift),
        event.is_shift_down ());
    gtk_check_button_set_active (
        GTK_CHECK_BUTTON (keyselection->toggle_meta),
        event.is_meta_down ());
    gtk_check_button_set_active (
        GTK_CHECK_BUTTON (keyselection->toggle_super),
        event.is_super_down ());
    gtk_check_button_set_active (
        GTK_CHECK_BUTTON (keyselection->toggle_hyper),
        event.is_hyper_down ());
    gtk_check_button_set_active (
        GTK_CHECK_BUTTON (keyselection->toggle_capslock),
        event.is_caps_lock_down ());
    gtk_check_button_set_active (
        GTK_CHECK_BUTTON (keyselection->toggle_release),
        event.is_key_release ());

    event.mask = 0;
    String str;

    if (scim_key_to_string (str, event)) {
        gtk_editable_set_text (GTK_EDITABLE (keyselection->key_code),
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
    GtkDialog *dialog = GTK_DIALOG (keyseldialog);

    gtk_window_set_resizable (GTK_WINDOW (keyseldialog), TRUE);

    keyseldialog->content_area = gtk_dialog_get_content_area (dialog);

    keyseldialog->keysel = scim_key_selection_new ();
    gtk_widget_set_margin_start (keyseldialog->keysel, 4);
    gtk_widget_set_margin_end (keyseldialog->keysel, 4);
    gtk_widget_set_margin_top (keyseldialog->keysel, 4);
    gtk_widget_set_margin_bottom (keyseldialog->keysel, 4);
    gtk_widget_set_vexpand (keyseldialog->keysel, TRUE);
    gtk_box_append (GTK_BOX (keyseldialog->content_area), keyseldialog->keysel);

    keyseldialog->cancel_button = gtk_dialog_add_button (dialog,
                                                        _("_Cancel"),
                                                        GTK_RESPONSE_CANCEL);

    keyseldialog->ok_button = gtk_dialog_add_button (dialog,
                                                    _("_OK"),
                                                    GTK_RESPONSE_OK);
    gtk_window_set_default_widget (GTK_WINDOW (dialog), keyseldialog->ok_button);

    gtk_window_set_title (GTK_WINDOW (keyseldialog),
                          _("Key Selection"));
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
