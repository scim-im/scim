#include <string.h>
#include <stdio.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

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

static void scim_key_selection_class_init              (gpointer klass_ptr, gpointer klass_data);
static void scim_key_selection_init                    (GTypeInstance *instance, gpointer klass);
static void scim_key_selection_finalize                (GObject               *object);

static void scim_key_selection_add_key_button_callback (GtkButton             *button,
                                                        ScimKeySelection      *keyselection);

static void scim_key_selection_del_key_button_callback (GtkButton             *button,
                                                        ScimKeySelection      *keyselection);

static void scim_key_selection_list_changed_callback   (GtkSingleSelection    *selection,
                                                        GParamSpec            *pspec,
                                                        ScimKeySelection      *keyselection);

static void scim_key_selection_list_setup_callback     (GtkSignalListItemFactory *factory,
                                                        GtkListItem           *item,
                                                        gpointer               data);

static void scim_key_selection_list_bind_callback      (GtkSignalListItemFactory *factory,
                                                        GtkListItem           *item,
                                                        gpointer               data);

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
        scim_key_selection_class_init,
        NULL,
        NULL,
        sizeof (ScimKeySelection),
        0,
        scim_key_selection_init,
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
scim_key_selection_class_init (gpointer klass_ptr,
                               gpointer /* klass_data */)
{
    ScimKeySelectionClass *klass = (ScimKeySelectionClass *) klass_ptr;
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
scim_key_selection_init (GTypeInstance *instance,
                         gpointer /* klass */)
{
    ScimKeySelection *keyselection = (ScimKeySelection *) instance;
    GtkWidget *grid;
    GtkWidget *hbox;
    GtkWidget *frame;
    GtkWidget *label;

    GtkWidget *scrolledwindow;
    GtkWidget *button;

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

    // A flat list of key strings: GtkStringList is the model for exactly that,
    // and GtkSingleSelection gives the always-one-selected behaviour that
    // GTK_SELECTION_BROWSE used to. Each takes a reference to the one below,
    // so only the view has to be kept.
    keyselection->list_model = gtk_string_list_new (NULL);
    keyselection->list_selection =
        gtk_single_selection_new (G_LIST_MODEL (keyselection->list_model));

    GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
    g_signal_connect (factory, "setup",
                      G_CALLBACK (scim_key_selection_list_setup_callback), NULL);
    g_signal_connect (factory, "bind",
                      G_CALLBACK (scim_key_selection_list_bind_callback), NULL);

    keyselection->list_view =
        gtk_list_view_new (GTK_SELECTION_MODEL (keyselection->list_selection), factory);
    gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolledwindow), keyselection->list_view);

    gtk_label_set_mnemonic_widget (GTK_LABEL (label), keyselection->list_view);

    g_signal_connect (G_OBJECT (keyselection->list_selection), "notify::selected",
                      G_CALLBACK (scim_key_selection_list_changed_callback),
                      keyselection);

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
scim_key_selection_add_key_button_callback (GtkButton        */* button */,
                                            ScimKeySelection *keyselection)
{
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

    const guint n = g_list_model_get_n_items (G_LIST_MODEL (keyselection->list_model));
    for (guint i = 0; i < n; ++ i) {
        const char *keystr = gtk_string_list_get_string (keyselection->list_model, i);
        if (keystr && String (keystr) == key)
            return;
    }

    gtk_string_list_append (keyselection->list_model, key.c_str ());

    g_signal_emit_by_name (keyselection, "key-selection-changed");
}

static void
scim_key_selection_del_key_button_callback (GtkButton       */* button */,
                                           ScimKeySelection *keyselection)
{
    const guint pos = gtk_single_selection_get_selected (keyselection->list_selection);

    if (pos != GTK_INVALID_LIST_POSITION) {
        gtk_string_list_remove (keyselection->list_model, pos);
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
scim_key_grab_key_pressed (GtkEventControllerKey */* controller */,
                           guint keyval, guint /* keycode */, GdkModifierType state,
                           KeyGrabData *data)
{
    data->key = keyevent_gtk_to_scim (keyval, state, FALSE);
    data->got_press = TRUE;

    return TRUE;
}

static void
scim_key_grab_key_released (GtkEventControllerKey */* controller */,
                            guint keyval, guint /* keycode */, GdkModifierType state,
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
scim_key_grab_button_callback (GtkButton        */* button */,
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

    String str;

    for (size_t i = 0; i < keylist.size (); ++ i) {
        if (scim_key_to_string (str, keylist [i]))
            gtk_string_list_append (keyselection->list_model, str.c_str ());
    }
}

void
scim_key_selection_set_keys (ScimKeySelection *keyselection,
                             const gchar      *keys)
{
    g_return_if_fail (SCIM_IS_KEY_SELECTION (keyselection));

    gtk_string_list_splice (keyselection->list_model, 0,
                            g_list_model_get_n_items (G_LIST_MODEL (keyselection->list_model)),
                            NULL);
    scim_key_selection_append_keys (keyselection, keys);
}

const gchar*
scim_key_selection_get_keys (ScimKeySelection *keyselection)
{
    g_return_val_if_fail (SCIM_IS_KEY_SELECTION (keyselection), NULL);

    if (keyselection->keys)
        g_free (keyselection->keys);

    keyselection->keys = NULL;

    const guint n = g_list_model_get_n_items (G_LIST_MODEL (keyselection->list_model));
    std::vector <String> keylist;

    for (guint i = 0; i < n; ++ i) {
        const char *keystr = gtk_string_list_get_string (keyselection->list_model, i);
        if (keystr) keylist.push_back (String (keystr));
    }

    if (keylist.size ())
        keyselection->keys = g_strdup (scim_combine_string_list (keylist).c_str ());

    return keyselection->keys;
}

static void
scim_key_selection_list_changed_callback (GtkSingleSelection *selection,
                                          GParamSpec         */* pspec */,
                                          ScimKeySelection   *keyselection)
{
    const guint pos = gtk_single_selection_get_selected (selection);

    if (pos == GTK_INVALID_LIST_POSITION)
        return;

    GtkStringList *model = GTK_STRING_LIST (gtk_single_selection_get_model (selection));
    const char *keystr = gtk_string_list_get_string (model, pos);
    KeyEvent keyevent;

    if (keystr && scim_string_to_key (keyevent, String (keystr)))
        scim_key_selection_set_key_event (keyselection, keyevent);
}

// One label per row; the factory builds it once and refills it as rows scroll.
static void
scim_key_selection_list_setup_callback (GtkSignalListItemFactory */* factory */,
                                        GtkListItem              *item,
                                        gpointer                  /* data */)
{
    GtkWidget *label = gtk_label_new (NULL);
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_list_item_set_child (item, label);
}

static void
scim_key_selection_list_bind_callback (GtkSignalListItemFactory */* factory */,
                                       GtkListItem              *item,
                                       gpointer                  /* data */)
{
    GtkWidget *label = gtk_list_item_get_child (item);
    GtkStringObject *obj = GTK_STRING_OBJECT (gtk_list_item_get_item (item));

    if (label && obj)
        gtk_label_set_text (GTK_LABEL (label), gtk_string_object_get_string (obj));
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

enum { DIALOG_RESPONSE, DIALOG_LAST_SIGNAL };
static guint dialog_signals [DIALOG_LAST_SIGNAL] = { 0 };

static GType key_selection_dialog_type = 0;

static void scim_key_selection_dialog_class_init (gpointer klass_ptr, gpointer klass_data);
static void scim_key_selection_dialog_init (GTypeInstance *instance, gpointer klass);
static gboolean scim_key_selection_dialog_close_request (GtkWindow *window);

void
scim_key_selection_dialog_register_type (GTypeModule *type_module)
{
    static const GTypeInfo key_selection_dialog_info =
    {
        sizeof (ScimKeySelectionDialogClass),
        NULL,
        NULL,
        scim_key_selection_dialog_class_init,
        NULL,
        NULL,
        sizeof (ScimKeySelectionDialog),
        0,
        scim_key_selection_dialog_init,
        0
    };

    if (!key_selection_dialog_type) {
        if (type_module)
            key_selection_dialog_type = g_type_module_register_type (
                                    type_module,
                                    GTK_TYPE_WINDOW,
                                    "SCIM_ScimKeySelectionDialog",
                                    &key_selection_dialog_info,
                                    (GTypeFlags) 0);
        else
            key_selection_dialog_type = g_type_register_static (
                                    GTK_TYPE_WINDOW,
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
scim_key_selection_dialog_class_init (gpointer klass_ptr,
                                      gpointer /* klass_data */)
{
    ScimKeySelectionDialogClass *klass = (ScimKeySelectionDialogClass *) klass_ptr;
    dialog_parent_class = (GtkWidgetClass*) g_type_class_peek_parent (klass);

    // GtkDialog used to provide this; the widget carries its own now.
    dialog_signals [DIALOG_RESPONSE] =
        g_signal_new ("response",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (ScimKeySelectionDialogClass, response),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__INT,
                      G_TYPE_NONE, 1, G_TYPE_INT);

    // The other two things GtkDialog gave a dialog: Escape dismisses it, and
    // closing it any other way is reported like a button, so a caller hears
    // exactly once however the dialog ends.
    GTK_WINDOW_CLASS (klass)->close_request = scim_key_selection_dialog_close_request;

    gtk_widget_class_add_binding_action (GTK_WIDGET_CLASS (klass),
                                         GDK_KEY_Escape, (GdkModifierType) 0,
                                         "window.close", NULL);
}

// Closing from the window manager, or with Escape, is a cancel. The window is
// left to the response handler to destroy, as it is on either button, so a
// caller has one place to clean up whatever it attached to the dialog.
static gboolean
scim_key_selection_dialog_close_request (GtkWindow *window)
{
    g_signal_emit (window, dialog_signals [DIALOG_RESPONSE], 0,
                   SCIM_KEY_SELECTION_RESPONSE_CANCEL);

    return GTK_WINDOW_CLASS (dialog_parent_class)->close_request (window);
}

static void
scim_key_selection_dialog_button_cb (GtkButton *button, gpointer user_data)
{
    ScimKeySelectionDialog *dialog = (ScimKeySelectionDialog *) user_data;
    const gint id = (button == GTK_BUTTON (dialog->ok_button))
                    ? SCIM_KEY_SELECTION_RESPONSE_OK
                    : SCIM_KEY_SELECTION_RESPONSE_CANCEL;

    g_signal_emit (dialog, dialog_signals [DIALOG_RESPONSE], 0, id);
}

static void
scim_key_selection_dialog_init (GTypeInstance *instance,
                                gpointer /* klass */)
{
    ScimKeySelectionDialog *keyseldialog = (ScimKeySelectionDialog *) instance;
    gtk_window_set_resizable (GTK_WINDOW (keyseldialog), TRUE);

    // GtkDialog laid out a content area with an action area under it; build the
    // same shape by hand, since GtkWindow has only the one child.
    GtkWidget *root_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child (GTK_WINDOW (keyseldialog), root_box);

    keyseldialog->content_area = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_vexpand (keyseldialog->content_area, TRUE);
    gtk_box_append (GTK_BOX (root_box), keyseldialog->content_area);

    keyseldialog->keysel = scim_key_selection_new ();
    gtk_widget_set_margin_start (keyseldialog->keysel, 4);
    gtk_widget_set_margin_end (keyseldialog->keysel, 4);
    gtk_widget_set_margin_top (keyseldialog->keysel, 4);
    gtk_widget_set_margin_bottom (keyseldialog->keysel, 4);
    gtk_widget_set_vexpand (keyseldialog->keysel, TRUE);
    gtk_box_append (GTK_BOX (keyseldialog->content_area), keyseldialog->keysel);

    GtkWidget *action_area = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_halign (action_area, GTK_ALIGN_END);
    gtk_widget_set_margin_start (action_area, 4);
    gtk_widget_set_margin_end (action_area, 4);
    gtk_widget_set_margin_top (action_area, 4);
    gtk_widget_set_margin_bottom (action_area, 4);
    gtk_box_append (GTK_BOX (root_box), action_area);

    keyseldialog->cancel_button = gtk_button_new_with_mnemonic (_("_Cancel"));
    gtk_box_append (GTK_BOX (action_area), keyseldialog->cancel_button);
    g_signal_connect (keyseldialog->cancel_button, "clicked",
                      G_CALLBACK (scim_key_selection_dialog_button_cb), keyseldialog);

    keyseldialog->ok_button = gtk_button_new_with_mnemonic (_("_OK"));
    gtk_box_append (GTK_BOX (action_area), keyseldialog->ok_button);
    g_signal_connect (keyseldialog->ok_button, "clicked",
                      G_CALLBACK (scim_key_selection_dialog_button_cb), keyseldialog);

    gtk_window_set_default_widget (GTK_WINDOW (keyseldialog), keyseldialog->ok_button);

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
