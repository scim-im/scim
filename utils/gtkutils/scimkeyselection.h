#ifndef __SCIM_KEY_SELECTION_H__
#define __SCIM_KEY_SELECTION_H__

#include <gtk/gtk.h>
#include <scim.h>

G_BEGIN_DECLS

#define SCIM_TYPE_KEY_SELECTION            (scim_key_selection_get_type ())
#define SCIM_KEY_SELECTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SCIM_TYPE_KEY_SELECTION, ScimKeySelection))
#define SCIM_KEY_SELECTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SCIM_TYPE_KEY_SELECTION, ScimKeySelectionClass))
#define SCIM_IS_KEY_SELECTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SCIM_TYPE_KEY_SELECTION))
#define SCIM_IS_KEY_SELECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SCIM_TYPE_KEY_SELECTION))
#define SCIM_KEY_SELECTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SCIM_TYPE_KEY_SELECTION, ScimKeySelectionClass))

#define SCIM_TYPE_KEY_SELECTION_DIALOG            (scim_key_selection_dialog_get_type ())
#define SCIM_KEY_SELECTION_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SCIM_TYPE_KEY_SELECTION_DIALOG, ScimKeySelectionDialog))
#define SCIM_KEY_SELECTION_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SCIM_TYPE_KEY_SELECTION_DIALOG, ScimKeySelectionDialogClass))
#define SCIM_IS_KEY_SELECTION_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SCIM_TYPE_KEY_SELECTION_DIALOG))
#define SCIM_IS_KEY_SELECTION_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SCIM_TYPE_KEY_SELECTION_DIALOG))
#define SCIM_KEY_SELECTION_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SCIM_TYPE_KEY_SELECTION_DIALOG, ScimKeySelectionDialogClass))

typedef struct _ScimKeySelection       ScimKeySelection;
typedef struct _ScimKeySelectionClass  ScimKeySelectionClass;

typedef struct _ScimKeySelectionDialog       ScimKeySelectionDialog;
typedef struct _ScimKeySelectionDialogClass  ScimKeySelectionDialogClass;

struct _ScimKeySelection
{
    GtkBox  vbox;

    GtkWidget *toggle_ctrl;
    GtkWidget *toggle_alt;
    GtkWidget *toggle_shift;
    GtkWidget *toggle_meta;
    GtkWidget *toggle_super;
    GtkWidget *toggle_hyper;
    GtkWidget *toggle_capslock;
    GtkWidget *toggle_release;
    GtkWidget *key_code;

    GtkWidget          *list_view;
    // Owned by list_view, which holds the selection, which holds the model.
    GtkSingleSelection *list_selection;
    GtkStringList      *list_model;

    gchar            *keys;
};

struct _ScimKeySelectionClass
{
    GtkBoxClass parent_class;

    void (*changed) (ScimKeySelection *keyselection);
};

// What the dialog reports through its "response" signal. Its own values, not
// GtkResponseType: that enum lives in gtk/deprecated/gtkdialog.h, which is
// exactly what this widget stopped deriving from.
enum
{
    SCIM_KEY_SELECTION_RESPONSE_CANCEL = 0,
    SCIM_KEY_SELECTION_RESPONSE_OK     = 1
};

struct _ScimKeySelectionDialog
{
    GtkWindow parent_instance;

    GtkWidget *keysel;

    GtkWidget *content_area;

    GtkWidget *ok_button;
    GtkWidget *cancel_button;
};

struct _ScimKeySelectionDialogClass
{
    GtkWindowClass parent_class;

    void (*response) (ScimKeySelectionDialog *dialog, gint response_id);

    /* Padding for future expansion */
    void (*_gtk_reserved1) (void);
    void (*_gtk_reserved2) (void);
    void (*_gtk_reserved3) (void);
    void (*_gtk_reserved4) (void);
};

GType                 scim_key_selection_get_type        (void) G_GNUC_CONST;

GtkWidget*            scim_key_selection_new             (void);

void                  scim_key_selection_set_keys        (ScimKeySelection       *keyselection,
                                                          const gchar            *keys);

void                  scim_key_selection_append_keys     (ScimKeySelection       *keyselection,
                                                          const gchar            *keys);


const gchar* scim_key_selection_get_keys        (ScimKeySelection       *keyselection);


GType                 scim_key_selection_dialog_get_type (void) G_GNUC_CONST;
GtkWidget*            scim_key_selection_dialog_new      (const gchar            *title);
void                  scim_key_selection_dialog_set_keys (ScimKeySelectionDialog *ksd,
                                                          const gchar            *keys);
const gchar* scim_key_selection_dialog_get_keys (ScimKeySelectionDialog *ksd);

G_END_DECLS

#endif /* __GTK_KEY_SELECTION_H__ */

/*
vi:ts=4:nowrap:ai:expandtab
*/
