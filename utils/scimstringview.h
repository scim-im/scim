#ifndef __SCIM_STRING_VIEW_H__
#define __SCIM_STRING_VIEW_H__


#include <gdk/gdk.h>
#include <pango/pango.h>

G_BEGIN_DECLS

#define SCIM_TYPE_STRING_VIEW            (scim_string_view_get_type ())
#define SCIM_STRING_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SCIM_TYPE_STRING_VIEW, ScimStringView))
#define SCIM_STRING_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SCIM_TYPE_STRING_VIEW, ScimStringViewClass))
#define SCIM_IS_STRING_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SCIM_TYPE_STRING_VIEW))
#define SCIM_IS_STRING_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SCIM_TYPE_STRING_VIEW))
#define SCIM_STRING_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SCIM_TYPE_STRING_VIEW, ScimStringViewClass))


typedef struct _ScimStringView       ScimStringView;
typedef struct _ScimStringViewClass  ScimStringViewClass;

struct _ScimStringView
{
  GtkWidget    widget;

  gchar       *text;

  guint16      text_length;        /* length in use, in chars */
  guint16      text_max_length;

  /*< private >*/
  GdkWindow   *text_area;
  
  gint         current_pos;
  
  PangoLayout *cached_layout;
  PangoAttrList *attrs;
  PangoAttrList *effective_attrs;

  guint        has_frame : 1;
  guint        draw_cursor : 1;
  guint        cursor_visible : 1;
  guint        auto_move_cursor : 1;
  guint        forward_event : 1;
  guint        auto_resize : 1;

  guint        blink_timeout;
  guint        recompute_idle;

  gint         scroll_offset;
  gint         ascent;        /* font ascent, in pango units  */
  gint         descent;        /* font descent, in pango units  */
  gint         max_width;

  gint         highlight_start;
  gint         highlight_end;
  
  guint16      text_size;        /* allocated size, in bytes */
  guint16      n_bytes;        /* length in use, in bytes */

  gint         width_chars;
};

struct _ScimStringViewClass
{
  GtkWidgetClass parent_class;

  void (* move_cursor) (ScimStringView *string_view, guint position);
};

void       scim_string_view_register_type        (GTypeModule    *type_module);
GType      scim_string_view_get_type             (void) G_GNUC_CONST;
GtkWidget* scim_string_view_new                  (void);
void       scim_string_view_set_has_frame        (ScimStringView *entry,
                                                  gboolean        setting);
gboolean   scim_string_view_get_has_frame        (ScimStringView *entry);

/* text is truncated if needed */
void       scim_string_view_set_max_length       (ScimStringView *entry,
                                                  gint            max);
gint       scim_string_view_get_max_length       (ScimStringView *entry);

void       scim_string_view_set_width_chars      (ScimStringView *entry,
                                                  gint            n_chars);
gint       scim_string_view_get_width_chars      (ScimStringView *entry);

/* Somewhat more convenient than the GtkEditable generic functions
 */
void       scim_string_view_set_text             (ScimStringView *entry,
                                                  const gchar    *text);
/* returns a reference to the text */
const gchar* scim_string_view_get_text  (ScimStringView *entry);

void       scim_string_view_set_position         (ScimStringView *entry,
                                                  gint            position);
gint       scim_string_view_get_position         (ScimStringView *entry);

void       scim_string_view_set_max_width        (ScimStringView *entry,
                                                  gint            width);
gint       scim_string_view_get_max_width        (ScimStringView *entry);

void       scim_string_view_set_draw_cursor      (ScimStringView *entry,
                                                  gboolean        width);
gboolean   scim_string_view_get_draw_cursor      (ScimStringView *entry);

void       scim_string_view_set_auto_move_cursor (ScimStringView *entry,
                                                  gboolean        setting);
gboolean   scim_string_view_get_auto_move_cursor (ScimStringView *entry);

void       scim_string_view_set_forward_event    (ScimStringView *entry,
                                                  gboolean        setting);
gboolean   scim_string_view_get_forward_event    (ScimStringView *entry);

void       scim_string_view_set_auto_resize      (ScimStringView *entry,
                                                  gboolean        setting);
gboolean   scim_string_view_get_auto_resize      (ScimStringView *entry);

PangoLayout* scim_string_view_get_layout         (ScimStringView *entry);
void         scim_string_view_get_layout_offsets (ScimStringView *entry,
                                                  gint           *x,
                                                  gint           *y);

void       scim_string_view_set_highlight        (ScimStringView *entry,
                                                  gint            start,
                                                  gint            end);

void       scim_string_view_get_highlight        (ScimStringView *entry,
                                                  gint           *start,
                                                  gint           *end);

void           scim_string_view_set_attributes   (ScimStringView *entry,
                                                  PangoAttrList  *attrs);

PangoAttrList *scim_string_view_get_attributes   (ScimStringView *entry);

G_END_DECLS

#endif /* __scim_string_view_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
