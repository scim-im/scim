/*
 * ScimStringView - a single-line, non-editable styled text display used by
 * the SCIM panel to render the client string / lookup text with an optional
 * blinking cursor, a highlight range and horizontal scrolling.
 *
 * This is a lean reimplementation for GTK4: the widget draws itself through
 * the snapshot/measure model (GTK4 has no per-widget GdkWindow, expose, or
 * gtk_paint machinery).  The public API and behavior match the original.
 */

#include <string.h>
#include <stdio.h>

#include <gtk/gtk.h>
#include <pango/pango.h>

#include "scimstringview.h"

#define MIN_STRING_VIEW_WIDTH  64
#define MAX_STRING_VIEW_WIDTH  400
#define INNER_BORDER           2
#define MIN_SIZE               16
#define MAX_SIZE               G_MAXUSHORT

#define CURSOR_ON_MULTIPLIER   0.66
#define CURSOR_OFF_MULTIPLIER  0.34

enum {
  PROP_0,
  PROP_DRAW_CURSOR,
  PROP_CURSOR_POSITION,
  PROP_AUTO_MOVE_CURSOR,
  PROP_FORWARD_EVENT,
  PROP_AUTO_RESIZE,
  PROP_MAX_LENGTH,
  PROP_MAX_WIDTH,
  PROP_HAS_FRAME,
  PROP_WIDTH_CHARS,
  PROP_SCROLL_OFFSET,
  PROP_TEXT
};

enum {
  MOVE_CURSOR,
  LAST_SIGNAL
};

static void   scim_string_view_class_init    (gpointer klass_ptr, gpointer klass_data);
static void   scim_string_view_init          (GTypeInstance *instance, gpointer klass);
static void   scim_string_view_finalize      (GObject             *object);
static void   scim_string_view_set_property  (GObject *object, guint prop_id,
                                              const GValue *value, GParamSpec *pspec);
static void   scim_string_view_get_property  (GObject *object, guint prop_id,
                                              GValue *value, GParamSpec *pspec);

static void   scim_string_view_measure       (GtkWidget *widget, GtkOrientation orientation,
                                              int for_size, int *minimum, int *natural,
                                              int *minimum_baseline, int *natural_baseline);
static void   scim_string_view_size_allocate (GtkWidget *widget, int width, int height, int baseline);
static void   scim_string_view_snapshot      (GtkWidget *widget, GtkSnapshot *snapshot);
static void   scim_string_view_map           (GtkWidget *widget);
static void   scim_string_view_unmap         (GtkWidget *widget);

static void   scim_string_view_pressed_cb    (GtkGestureClick *gesture, int n_press,
                                              double x, double y, gpointer data);

static PangoLayout *scim_string_view_ensure_layout       (ScimStringView *string_view);
static void         scim_string_view_reset_layout        (ScimStringView *string_view);
static void         scim_string_view_recompute           (ScimStringView *string_view);
static gint         scim_string_view_find_position       (ScimStringView *string_view, gint widget_x);
static gint         scim_string_view_get_cursor_locations (ScimStringView *string_view,
                                                          gint *strong_x, gint *weak_x);
static void         scim_string_view_adjust_scroll       (ScimStringView *string_view);
static void         scim_string_view_move_cursor         (ScimStringView *string_view, guint position);
static void         scim_string_view_check_cursor_blink  (ScimStringView *string_view);
static void         scim_string_view_ensure_metrics      (ScimStringView *string_view);

static GtkWidgetClass *parent_class = NULL;
static gint string_view_signals[LAST_SIGNAL] = { 0 };
static GType string_view_type = 0;

void
scim_string_view_register_type (GTypeModule *type_module)
{
  static const GTypeInfo string_view_info =
  {
    sizeof (ScimStringViewClass),
    NULL,
    NULL,
    scim_string_view_class_init,
    NULL,
    NULL,
    sizeof (ScimStringView),
    0,
    scim_string_view_init,
    0
  };

  if (!string_view_type) {
    if (type_module)
      string_view_type = g_type_module_register_type (
                            type_module, GTK_TYPE_WIDGET,
                            "SCIM_ScimStringView", &string_view_info, (GTypeFlags) 0);
    else
      string_view_type = g_type_register_static (
                            GTK_TYPE_WIDGET,
                            "SCIM_ScimStringView", &string_view_info, (GTypeFlags) 0);
  }
}

GType
scim_string_view_get_type (void)
{
  if (!string_view_type)
    scim_string_view_register_type (NULL);

  return string_view_type;
}

static void
scim_string_view_class_init (gpointer klass_ptr,
                             gpointer /* klass_data */)
{
  ScimStringViewClass *klass = (ScimStringViewClass *) klass_ptr;
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = scim_string_view_finalize;
  gobject_class->set_property = scim_string_view_set_property;
  gobject_class->get_property = scim_string_view_get_property;

  widget_class->measure = scim_string_view_measure;
  widget_class->size_allocate = scim_string_view_size_allocate;
  widget_class->snapshot = scim_string_view_snapshot;
  widget_class->map = scim_string_view_map;
  widget_class->unmap = scim_string_view_unmap;

  klass->move_cursor = scim_string_view_move_cursor;

  g_object_class_install_property (gobject_class, PROP_CURSOR_POSITION,
      g_param_spec_int ("cursor_position", "Cursor Position",
                        "The current position of the insertion cursor in chars.",
                        0, MAX_SIZE, 0, G_PARAM_READABLE));
  g_object_class_install_property (gobject_class, PROP_MAX_LENGTH,
      g_param_spec_int ("max_length", "Maximum length",
                        "Maximum number of characters for this string view. Zero if no maximum.",
                        0, MAX_SIZE, 0, (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE)));
  g_object_class_install_property (gobject_class, PROP_MAX_WIDTH,
      g_param_spec_int ("max_width", "Maximum width",
                        "Maximum width of this string view.",
                        0, MAX_STRING_VIEW_WIDTH, 0, (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE)));
  g_object_class_install_property (gobject_class, PROP_HAS_FRAME,
      g_param_spec_boolean ("has_frame", "Has Frame",
                            "FALSE removes outside bevel from string view.",
                            TRUE, (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE)));
  g_object_class_install_property (gobject_class, PROP_DRAW_CURSOR,
      g_param_spec_boolean ("draw_cursor", "Draw cursor",
                            "TRUE draw blinking cursor.",
                            TRUE, (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE)));
  g_object_class_install_property (gobject_class, PROP_AUTO_MOVE_CURSOR,
      g_param_spec_boolean ("auto_move_cursor", "Auto move cursor",
                            "TRUE auto move cursor position when mouse clicking.",
                            FALSE, (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE)));
  g_object_class_install_property (gobject_class, PROP_FORWARD_EVENT,
      g_param_spec_boolean ("forward_event", "Forward button press event",
                            "TRUE forward button press event to user program.",
                            FALSE, (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE)));
  g_object_class_install_property (gobject_class, PROP_AUTO_RESIZE,
      g_param_spec_boolean ("auto_resize", "Auto resize the widget to fit the string",
                            "TRUE Auto resize on.",
                            FALSE, (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE)));
  g_object_class_install_property (gobject_class, PROP_WIDTH_CHARS,
      g_param_spec_int ("width_chars", "Width in chars",
                        "Number of characters to leave space for in the string view.",
                        -1, G_MAXINT, -1, (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE)));
  g_object_class_install_property (gobject_class, PROP_SCROLL_OFFSET,
      g_param_spec_int ("scroll_offset", "Scroll offset",
                        "Number of pixels of the string view scrolled off the screen to the left",
                        0, G_MAXINT, 0, G_PARAM_READABLE));
  g_object_class_install_property (gobject_class, PROP_TEXT,
      g_param_spec_string ("text", "Text", "The contents of the string view",
                           "", (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE)));

  string_view_signals[MOVE_CURSOR] =
      g_signal_new ("move_cursor", G_TYPE_FROM_CLASS (gobject_class),
                    G_SIGNAL_RUN_FIRST,
                    G_STRUCT_OFFSET (ScimStringViewClass, move_cursor),
                    NULL, NULL, g_cclosure_marshal_VOID__UINT,
                    G_TYPE_NONE, 1, G_TYPE_UINT);
}

static void
scim_string_view_init (GTypeInstance *instance,
                       gpointer /* klass */)
{
  ScimStringView *string_view = (ScimStringView *) instance;
  GtkGesture *click;

  string_view->text_size = MIN_SIZE;
  string_view->text = g_malloc (string_view->text_size);
  string_view->text[0] = '\0';

  string_view->width_chars = -1;
  string_view->has_frame = TRUE;
  string_view->draw_cursor = TRUE;
  string_view->cursor_visible = TRUE;
  string_view->auto_move_cursor = FALSE;
  string_view->forward_event = FALSE;
  string_view->max_width = -1;
  string_view->highlight_start = -1;
  string_view->highlight_end = -1;

  click = gtk_gesture_click_new ();
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (click), GDK_BUTTON_PRIMARY);
  g_signal_connect (click, "pressed", G_CALLBACK (scim_string_view_pressed_cb), string_view);
  gtk_widget_add_controller (GTK_WIDGET (string_view), GTK_EVENT_CONTROLLER (click));
}

static void
scim_string_view_finalize (GObject *object)
{
  ScimStringView *view = SCIM_STRING_VIEW (object);

  if (view->cached_layout)
    g_object_unref (G_OBJECT (view->cached_layout));

  if (view->blink_timeout)
    g_source_remove (view->blink_timeout);

  if (view->recompute_idle)
    g_source_remove (view->recompute_idle);

  view->text_size = 0;

  if (view->text)
    g_free (view->text);
  view->text = NULL;

  if (view->attrs)
    pango_attr_list_unref (view->attrs);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
scim_string_view_set_property (GObject *object, guint prop_id,
                               const GValue *value, GParamSpec *pspec)
{
  ScimStringView *view = SCIM_STRING_VIEW (object);

  switch (prop_id)
    {
    case PROP_MAX_LENGTH:      scim_string_view_set_max_length (view, g_value_get_int (value)); break;
    case PROP_MAX_WIDTH:       scim_string_view_set_max_width (view, g_value_get_int (value)); break;
    case PROP_HAS_FRAME:       scim_string_view_set_has_frame (view, g_value_get_boolean (value)); break;
    case PROP_DRAW_CURSOR:     scim_string_view_set_draw_cursor (view, g_value_get_boolean (value)); break;
    case PROP_AUTO_MOVE_CURSOR:scim_string_view_set_auto_move_cursor (view, g_value_get_boolean (value)); break;
    case PROP_FORWARD_EVENT:   scim_string_view_set_forward_event (view, g_value_get_boolean (value)); break;
    case PROP_AUTO_RESIZE:     scim_string_view_set_auto_resize (view, g_value_get_boolean (value)); break;
    case PROP_WIDTH_CHARS:     scim_string_view_set_width_chars (view, g_value_get_int (value)); break;
    case PROP_TEXT:            scim_string_view_set_text (view, g_value_get_string (value)); break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
scim_string_view_get_property (GObject *object, guint prop_id,
                               GValue *value, GParamSpec *pspec)
{
  ScimStringView *view = SCIM_STRING_VIEW (object);

  switch (prop_id)
    {
    case PROP_CURSOR_POSITION:  g_value_set_int (value, view->current_pos); break;
    case PROP_MAX_LENGTH:       g_value_set_int (value, view->text_max_length); break;
    case PROP_MAX_WIDTH:        g_value_set_int (value, view->max_width); break;
    case PROP_HAS_FRAME:        g_value_set_boolean (value, view->has_frame); break;
    case PROP_DRAW_CURSOR:      g_value_set_boolean (value, view->draw_cursor); break;
    case PROP_AUTO_MOVE_CURSOR: g_value_set_boolean (value, view->auto_move_cursor); break;
    case PROP_FORWARD_EVENT:    g_value_set_boolean (value, view->forward_event); break;
    case PROP_AUTO_RESIZE:      g_value_set_boolean (value, view->auto_resize); break;
    case PROP_WIDTH_CHARS:      g_value_set_int (value, view->width_chars); break;
    case PROP_SCROLL_OFFSET:    g_value_set_int (value, view->scroll_offset); break;
    case PROP_TEXT:             g_value_set_string (value, view->text); break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

/* --- layout helpers --- */

static void
scim_string_view_get_borders (ScimStringView *string_view, gint *xborder, gint *yborder)
{
  if (string_view->has_frame)
    { *xborder = 1; *yborder = 1; }
  else
    { *xborder = 0; *yborder = 0; }
}

static void
scim_string_view_ensure_metrics (ScimStringView *string_view)
{
  PangoContext *context = gtk_widget_get_pango_context (GTK_WIDGET (string_view));
  PangoFontMetrics *metrics =
      pango_context_get_metrics (context,
                                 pango_context_get_font_description (context),
                                 pango_context_get_language (context));

  string_view->ascent = pango_font_metrics_get_ascent (metrics);
  string_view->descent = pango_font_metrics_get_descent (metrics);

  pango_font_metrics_unref (metrics);
}

static void
scim_string_view_reset_layout (ScimStringView *string_view)
{
  if (string_view->cached_layout)
    {
      g_object_unref (G_OBJECT (string_view->cached_layout));
      string_view->cached_layout = NULL;
    }
}

static PangoLayout *
scim_string_view_ensure_layout (ScimStringView *string_view)
{
  if (!string_view->cached_layout)
    {
      PangoLayout *layout = gtk_widget_create_pango_layout (GTK_WIDGET (string_view), NULL);

      if (!string_view->attrs)
        string_view->attrs = pango_attr_list_new ();

      pango_layout_set_single_paragraph_mode (layout, TRUE);
      pango_layout_set_text (layout, string_view->text, string_view->n_bytes);
      pango_layout_set_attributes (layout, string_view->attrs);

      string_view->cached_layout = layout;
    }

  return string_view->cached_layout;
}

/* Inset from the widget edge to the text area (frame + inner border). */
static void
scim_string_view_get_inset (ScimStringView *string_view, gint *ix, gint *iy)
{
  gint xb, yb;
  scim_string_view_get_borders (string_view, &xb, &yb);
  if (ix) *ix = xb + INNER_BORDER;
  if (iy) *iy = yb + INNER_BORDER;
}

static void
get_layout_position (ScimStringView *string_view, gint *x, gint *y)
{
  PangoLayout *layout = scim_string_view_ensure_layout (string_view);
  PangoRectangle logical_rect;
  PangoLayoutLine *line;
  gint inset_x, inset_y;
  gint content_height, area_height;
  gint y_pos;

  scim_string_view_ensure_metrics (string_view);
  scim_string_view_get_inset (string_view, &inset_x, &inset_y);

  content_height = gtk_widget_get_height (GTK_WIDGET (string_view)) - 2 * inset_y;
  if (content_height < 0)
    content_height = 0;
  area_height = PANGO_SCALE * content_height;

  line = pango_layout_get_lines_readonly (layout)->data;
  pango_layout_line_get_extents (line, NULL, &logical_rect);

  y_pos = ((area_height - string_view->ascent - string_view->descent) / 2 +
           string_view->ascent + logical_rect.y);

  if (logical_rect.height > area_height)
    y_pos = (area_height - logical_rect.height) / 2;
  else if (y_pos < 0)
    y_pos = 0;
  else if (y_pos + logical_rect.height > area_height)
    y_pos = area_height - logical_rect.height;

  y_pos = inset_y + y_pos / PANGO_SCALE;

  if (x) *x = inset_x - string_view->scroll_offset;
  if (y) *y = y_pos;
}

static gint
scim_string_view_find_position (ScimStringView *string_view, gint widget_x)
{
  PangoLayout *layout;
  PangoLayoutLine *line;
  gint index, pos, inset_x;
  gboolean trailing;

  scim_string_view_get_inset (string_view, &inset_x, NULL);

  layout = scim_string_view_ensure_layout (string_view);
  line = pango_layout_get_lines_readonly (layout)->data;

  gint layout_x = widget_x - inset_x + string_view->scroll_offset;

  pango_layout_line_x_to_index (line, layout_x * PANGO_SCALE, &index, &trailing);

  pos = g_utf8_pointer_to_offset (string_view->text, string_view->text + index);
  pos += trailing;

  return pos;
}

static gint
scim_string_view_get_cursor_locations (ScimStringView *string_view, gint *strong_x, gint *weak_x)
{
  PangoLayout *layout = scim_string_view_ensure_layout (string_view);
  const gchar *text;
  PangoRectangle strong_pos, weak_pos;
  gint index;

  text = pango_layout_get_text (layout);
  index = g_utf8_offset_to_pointer (text, string_view->current_pos) - text;

  pango_layout_get_cursor_pos (layout, index, &strong_pos, &weak_pos);

  if (strong_x) *strong_x = strong_pos.x / PANGO_SCALE;
  if (weak_x)   *weak_x   = weak_pos.x / PANGO_SCALE;

  return index;
}

static void
scim_string_view_adjust_scroll (ScimStringView *string_view)
{
  gint min_offset, max_offset;
  gint text_area_width, inset_x;
  gint strong_x, weak_x;
  gint strong_xoffset, weak_xoffset;
  PangoLayout *layout;
  PangoLayoutLine *line;
  PangoRectangle logical_rect;

  if (!gtk_widget_get_mapped (GTK_WIDGET (string_view)))
    return;

  scim_string_view_get_inset (string_view, &inset_x, NULL);
  text_area_width = gtk_widget_get_width (GTK_WIDGET (string_view)) - 2 * inset_x;
  if (text_area_width < 0)
    text_area_width = 0;

  layout = scim_string_view_ensure_layout (string_view);
  line = pango_layout_get_lines_readonly (layout)->data;
  pango_layout_line_get_extents (line, NULL, &logical_rect);

  if (gtk_widget_get_direction (GTK_WIDGET (string_view)) == GTK_TEXT_DIR_LTR)
    {
      min_offset = 0;
      max_offset = MAX (min_offset, logical_rect.width / PANGO_SCALE - text_area_width);
    }
  else
    {
      max_offset = logical_rect.width / PANGO_SCALE - text_area_width;
      min_offset = MIN (0, max_offset);
    }

  string_view->scroll_offset = CLAMP (string_view->scroll_offset, min_offset, max_offset);

  scim_string_view_get_cursor_locations (string_view, &strong_x, &weak_x);

  strong_xoffset = strong_x - string_view->scroll_offset;

  if (strong_xoffset < 0)
    {
      string_view->scroll_offset += strong_xoffset;
      strong_xoffset = 0;
    }
  else if (strong_xoffset > text_area_width)
    {
      string_view->scroll_offset += strong_xoffset - text_area_width;
      strong_xoffset = text_area_width;
    }

  weak_xoffset = weak_x - string_view->scroll_offset;

  if (weak_xoffset < 0 && strong_xoffset - weak_xoffset <= text_area_width)
    string_view->scroll_offset += weak_xoffset;
  else if (weak_xoffset > text_area_width &&
           strong_xoffset - (weak_xoffset - text_area_width) >= 0)
    string_view->scroll_offset += weak_xoffset - text_area_width;

  g_object_notify (G_OBJECT (string_view), "scroll_offset");
}

static gboolean
recompute_idle_func (gpointer data)
{
  ScimStringView *string_view = SCIM_STRING_VIEW (data);

  scim_string_view_adjust_scroll (string_view);
  gtk_widget_queue_draw (GTK_WIDGET (string_view));

  string_view->recompute_idle = 0;

  return FALSE;
}

static void
scim_string_view_recompute (ScimStringView *string_view)
{
  scim_string_view_reset_layout (string_view);
  scim_string_view_check_cursor_blink (string_view);

  if (!string_view->recompute_idle)
    string_view->recompute_idle =
        g_idle_add_full (G_PRIORITY_HIGH_IDLE + 15,
                         recompute_idle_func, string_view, NULL);
}

/* --- cursor blink --- */

static gboolean
cursor_blinks (ScimStringView *string_view)
{
  GtkSettings *settings = gtk_widget_get_settings (GTK_WIDGET (string_view));
  gboolean blink;
  g_object_get (G_OBJECT (settings), "gtk-cursor-blink", &blink, NULL);
  return blink;
}

static gint
get_cursor_time (ScimStringView *string_view)
{
  GtkSettings *settings = gtk_widget_get_settings (GTK_WIDGET (string_view));
  gint time;
  g_object_get (G_OBJECT (settings), "gtk-cursor-blink-time", &time, NULL);
  return time;
}

static gboolean
blink_cb (gpointer data)
{
  ScimStringView *string_view = SCIM_STRING_VIEW (data);

  string_view->cursor_visible = !string_view->cursor_visible;
  gtk_widget_queue_draw (GTK_WIDGET (string_view));

  return G_SOURCE_CONTINUE;
}

static void
scim_string_view_check_cursor_blink (ScimStringView *string_view)
{
  if (string_view->draw_cursor &&
      gtk_widget_get_mapped (GTK_WIDGET (string_view)) &&
      cursor_blinks (string_view))
    {
      if (!string_view->blink_timeout)
        {
          string_view->cursor_visible = TRUE;
          string_view->blink_timeout =
              g_timeout_add (get_cursor_time (string_view) * CURSOR_ON_MULTIPLIER,
                             blink_cb, string_view);
        }
    }
  else
    {
      if (string_view->blink_timeout)
        {
          g_source_remove (string_view->blink_timeout);
          string_view->blink_timeout = 0;
        }
      string_view->cursor_visible = string_view->draw_cursor;
      gtk_widget_queue_draw (GTK_WIDGET (string_view));
    }
}

/* --- widget vfuncs --- */

static void
scim_string_view_measure (GtkWidget *widget, GtkOrientation orientation,
                          int /* for_size */, int *minimum, int *natural,
                          int *minimum_baseline, int *natural_baseline)
{
  ScimStringView *string_view = SCIM_STRING_VIEW (widget);
  gint inset_x, inset_y;
  gint size;

  scim_string_view_ensure_metrics (string_view);
  scim_string_view_get_inset (string_view, &inset_x, &inset_y);

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      gint width;

      if (string_view->auto_resize)
        {
          PangoLayout *layout = scim_string_view_ensure_layout (string_view);
          int w, h;
          pango_layout_get_pixel_size (layout, &w, &h);
          width = ((w < MIN_STRING_VIEW_WIDTH) ? MIN_STRING_VIEW_WIDTH : w) + 2;
        }
      else if (string_view->width_chars < 0)
        {
          width = MIN_STRING_VIEW_WIDTH;
        }
      else
        {
          PangoContext *context = gtk_widget_get_pango_context (widget);
          PangoFontMetrics *metrics =
              pango_context_get_metrics (context,
                                         pango_context_get_font_description (context),
                                         pango_context_get_language (context));
          gint char_width = pango_font_metrics_get_approximate_char_width (metrics);
          width = PANGO_PIXELS (char_width) * string_view->width_chars;
          pango_font_metrics_unref (metrics);
        }

      if (string_view->max_width > 0 && width > string_view->max_width)
        width = string_view->max_width;

      size = width + inset_x * 2;
    }
  else
    {
      size = PANGO_PIXELS (string_view->ascent + string_view->descent) + inset_y * 2;
    }

  if (minimum) *minimum = size;
  if (natural) *natural = size;
  if (minimum_baseline) *minimum_baseline = -1;
  if (natural_baseline) *natural_baseline = -1;
}

static void
scim_string_view_size_allocate (GtkWidget *widget, int width, int height, int baseline)
{
  ScimStringView *string_view = SCIM_STRING_VIEW (widget);

  if (parent_class->size_allocate)
    parent_class->size_allocate (widget, width, height, baseline);

  scim_string_view_adjust_scroll (string_view);
}

static void
scim_string_view_map (GtkWidget *widget)
{
  parent_class->map (widget);
  scim_string_view_check_cursor_blink (SCIM_STRING_VIEW (widget));
}

static void
scim_string_view_unmap (GtkWidget *widget)
{
  ScimStringView *string_view = SCIM_STRING_VIEW (widget);

  if (string_view->blink_timeout)
    {
      g_source_remove (string_view->blink_timeout);
      string_view->blink_timeout = 0;
    }

  parent_class->unmap (widget);
}

static void
scim_string_view_snapshot (GtkWidget *widget, GtkSnapshot *snapshot)
{
  ScimStringView *string_view = SCIM_STRING_VIEW (widget);
  PangoLayout *layout;
  GdkRGBA text_color;
  gint inset_x, inset_y;
  gint x, y;
  int W = gtk_widget_get_width (widget);
  int H = gtk_widget_get_height (widget);
  graphene_rect_t clip;
  graphene_point_t origin;

  scim_string_view_get_inset (string_view, &inset_x, &inset_y);
  gtk_widget_get_color (widget, &text_color);

  /* Optional frame */
  if (string_view->has_frame)
    {
      GskRoundedRect frame;
      graphene_rect_t rect = GRAPHENE_RECT_INIT (0, 0, W, H);
      float widths[4] = { 1, 1, 1, 1 };
      GdkRGBA colors[4] = { text_color, text_color, text_color, text_color };
      gsk_rounded_rect_init_from_rect (&frame, &rect, 0);
      gtk_snapshot_append_border (snapshot, &frame, widths, colors);
    }

  layout = scim_string_view_ensure_layout (string_view);
  get_layout_position (string_view, &x, &y);

  clip = GRAPHENE_RECT_INIT (inset_x, inset_y,
                             MAX (0, W - 2 * inset_x), MAX (0, H - 2 * inset_y));
  gtk_snapshot_push_clip (snapshot, &clip);

  /* Base text */
  origin = GRAPHENE_POINT_INIT (x, y);
  gtk_snapshot_save (snapshot);
  gtk_snapshot_translate (snapshot, &origin);
  gtk_snapshot_append_layout (snapshot, layout, &text_color);
  gtk_snapshot_restore (snapshot);

  /* Highlight range */
  if (string_view->highlight_start >= 0 &&
      string_view->highlight_start < string_view->highlight_end &&
      string_view->highlight_start < string_view->text_length)
    {
      gint *ranges;
      gint n_ranges, i;
      gint start_pos = string_view->highlight_start;
      gint end_pos = (string_view->text_length > string_view->highlight_end) ?
                      string_view->highlight_end : string_view->text_length;
      PangoRectangle logical_rect;
      const gchar *text = pango_layout_get_text (layout);
      gint start_index = g_utf8_offset_to_pointer (text, start_pos) - text;
      gint end_index = g_utf8_offset_to_pointer (text, end_pos) - text;
      PangoLayoutLine *line = pango_layout_get_lines_readonly (layout)->data;
      GdkRGBA sel_bg, sel_fg;

      /* Deprecated in GTK 4.10 with nothing put in their place: GTK4 offers no
       * way to read a theme's named colours, and gtk_widget_get_color() answers
       * only for the widget's own state, which is never "selected" here -- the
       * highlight is a range inside one unfocusable view, not a widget state.
       * The hardcoded fallbacks below already cover a theme that defines
       * neither, so the worst this can decay to is those. Scoped, so a real
       * deprecation elsewhere in the file is still reported. */
      G_GNUC_BEGIN_IGNORE_DEPRECATIONS
      GtkStyleContext *ctx = gtk_widget_get_style_context (widget);

      if (!gtk_style_context_lookup_color (ctx, "theme_selected_bg_color", &sel_bg))
        sel_bg = (GdkRGBA){ 0.2, 0.4, 0.85, 1.0 };
      if (!gtk_style_context_lookup_color (ctx, "theme_selected_fg_color", &sel_fg))
        sel_fg = (GdkRGBA){ 1.0, 1.0, 1.0, 1.0 };
      G_GNUC_END_IGNORE_DEPRECATIONS

      pango_layout_line_get_x_ranges (line, start_index, end_index, &ranges, &n_ranges);
      pango_layout_get_extents (layout, NULL, &logical_rect);

      for (i = 0; i < n_ranges; ++i)
        {
          graphene_rect_t rect =
              GRAPHENE_RECT_INIT (x + ranges[2 * i] / PANGO_SCALE,
                                  y,
                                  (ranges[2 * i + 1] - ranges[2 * i]) / PANGO_SCALE,
                                  logical_rect.height / PANGO_SCALE);

          gtk_snapshot_append_color (snapshot, &sel_bg, &rect);

          gtk_snapshot_push_clip (snapshot, &rect);
          gtk_snapshot_save (snapshot);
          gtk_snapshot_translate (snapshot, &origin);
          gtk_snapshot_append_layout (snapshot, layout, &sel_fg);
          gtk_snapshot_restore (snapshot);
          gtk_snapshot_pop (snapshot);
        }

      g_free (ranges);
    }

  /* Insertion cursor */
  if (string_view->draw_cursor && string_view->cursor_visible)
    {
      gint strong_x, weak_x;
      graphene_rect_t cursor;

      scim_string_view_get_cursor_locations (string_view, &strong_x, &weak_x);

      cursor = GRAPHENE_RECT_INIT (x + strong_x, inset_y, 1, MAX (0, H - 2 * inset_y));
      gtk_snapshot_append_color (snapshot, &text_color, &cursor);
    }

  gtk_snapshot_pop (snapshot);
}

static void
scim_string_view_pressed_cb (GtkGestureClick *gesture, int /* n_press */,
                             double x, double /* y */, gpointer data)
{
  ScimStringView *string_view = SCIM_STRING_VIEW (data);
  gint pos = scim_string_view_find_position (string_view, (gint) x);

  g_signal_emit (G_OBJECT (string_view), string_view_signals[MOVE_CURSOR], 0, pos);

  if (!string_view->forward_event)
    gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_CLAIMED);
}

static void
scim_string_view_move_cursor (ScimStringView *string_view, guint position)
{
  if (string_view->auto_move_cursor)
    scim_string_view_set_position (string_view, position);
}

/* Public API
 */

GtkWidget*
scim_string_view_new (void)
{
  return GTK_WIDGET (g_object_new (SCIM_TYPE_STRING_VIEW, NULL));
}

void
scim_string_view_set_position (ScimStringView *string_view, gint position)
{
  gboolean changed = FALSE;

  g_return_if_fail (SCIM_IS_STRING_VIEW (string_view));

  g_object_freeze_notify (G_OBJECT (string_view));

  if (position != -1 && string_view->current_pos != position)
    {
      string_view->current_pos =
         (position > string_view->text_length) ? (string_view->text_length) : position;
      changed = TRUE;
      g_object_notify (G_OBJECT (string_view), "cursor_position");
    }

  g_object_thaw_notify (G_OBJECT (string_view));

  if (changed)
    scim_string_view_recompute (string_view);
}

gint
scim_string_view_get_position (ScimStringView *string_view)
{
  g_return_val_if_fail (SCIM_IS_STRING_VIEW (string_view), FALSE);
  return string_view->current_pos;
}

void
scim_string_view_set_max_width (ScimStringView *string_view, gint width)
{
  g_return_if_fail (SCIM_IS_STRING_VIEW (string_view));

  if (string_view->max_width == width)
    return;

  if (width > 0 && width < MIN_STRING_VIEW_WIDTH) width = MIN_STRING_VIEW_WIDTH;

  string_view->max_width = width;

  gtk_widget_queue_resize (GTK_WIDGET (string_view));
  scim_string_view_recompute (string_view);
}

gint
scim_string_view_get_max_width (ScimStringView *string_view)
{
  g_return_val_if_fail (SCIM_IS_STRING_VIEW (string_view), FALSE);
  return string_view->max_width;
}

void
scim_string_view_set_draw_cursor (ScimStringView *string_view, gboolean setting)
{
  g_return_if_fail (SCIM_IS_STRING_VIEW (string_view));

  setting = (setting != FALSE);
  if (string_view->draw_cursor == setting)
    return;

  string_view->draw_cursor = setting;
  scim_string_view_check_cursor_blink (string_view);
  gtk_widget_queue_draw (GTK_WIDGET (string_view));
  g_object_notify (G_OBJECT (string_view), "draw_cursor");
}

gboolean
scim_string_view_get_draw_cursor (ScimStringView *string_view)
{
  g_return_val_if_fail (SCIM_IS_STRING_VIEW (string_view), FALSE);
  return string_view->draw_cursor;
}

void
scim_string_view_set_auto_move_cursor (ScimStringView *string_view, gboolean setting)
{
  g_return_if_fail (SCIM_IS_STRING_VIEW (string_view));

  setting = (setting != FALSE);
  if (string_view->auto_move_cursor == setting)
    return;

  string_view->auto_move_cursor = setting;
  g_object_notify (G_OBJECT (string_view), "auto_move_cursor");
}

gboolean
scim_string_view_get_auto_move_cursor (ScimStringView *string_view)
{
  g_return_val_if_fail (SCIM_IS_STRING_VIEW (string_view), FALSE);
  return string_view->auto_move_cursor;
}

void
scim_string_view_set_forward_event (ScimStringView *string_view, gboolean setting)
{
  g_return_if_fail (SCIM_IS_STRING_VIEW (string_view));

  setting = (setting != FALSE);
  if (string_view->forward_event == setting)
    return;

  string_view->forward_event = setting;
  g_object_notify (G_OBJECT (string_view), "forward_event");
}

gboolean
scim_string_view_get_forward_event (ScimStringView *string_view)
{
  g_return_val_if_fail (SCIM_IS_STRING_VIEW (string_view), FALSE);
  return string_view->forward_event;
}

void
scim_string_view_set_auto_resize (ScimStringView *string_view, gboolean setting)
{
  g_return_if_fail (SCIM_IS_STRING_VIEW (string_view));

  setting = (setting != FALSE);
  if (string_view->auto_resize == setting)
    return;

  string_view->auto_resize = setting;
  g_object_notify (G_OBJECT (string_view), "auto_resize");
}

gboolean
scim_string_view_get_auto_resize (ScimStringView *string_view)
{
  g_return_val_if_fail (SCIM_IS_STRING_VIEW (string_view), FALSE);
  return string_view->auto_resize;
}

const gchar*
scim_string_view_get_text (ScimStringView *string_view)
{
  g_return_val_if_fail (SCIM_IS_STRING_VIEW (string_view), NULL);
  return string_view->text;
}

void
scim_string_view_set_text (ScimStringView *string_view, const gchar *text)
{
  gint new_text_length;
  gint new_nbytes;

  g_return_if_fail (SCIM_IS_STRING_VIEW (string_view));
  g_return_if_fail (text != NULL);

  if (strcmp (string_view->text, text) == 0)
    return;

  new_nbytes = strlen (text);
  new_text_length = g_utf8_strlen (text, new_nbytes);

  if (string_view->text_max_length > 0 && new_text_length > string_view->text_max_length)
    {
      gtk_widget_error_bell (GTK_WIDGET (string_view));
      new_text_length = string_view->text_max_length;
      new_nbytes = g_utf8_offset_to_pointer (text, new_text_length) - text;
    }

  if (new_nbytes >= string_view->text_size)
    {
      string_view->text = g_realloc (string_view->text, new_nbytes + 1);
      string_view->text_size = new_nbytes + 1;
    }

  memcpy (string_view->text, text, new_nbytes);

  string_view->n_bytes = new_nbytes;
  string_view->text_length = new_text_length;
  string_view->text[string_view->n_bytes] = '\0';

  if (string_view->current_pos > string_view->text_length)
    string_view->current_pos = string_view->text_length;

  if (string_view->auto_resize)
    gtk_widget_queue_resize (GTK_WIDGET (string_view));

  scim_string_view_recompute (string_view);
  g_object_notify (G_OBJECT (string_view), "text");
}

void
scim_string_view_set_max_length (ScimStringView *string_view, gint max)
{
  gint nbytes;

  g_return_if_fail (SCIM_IS_STRING_VIEW (string_view));

  max = CLAMP (max, 0, MAX_SIZE);

  if (max > 0 && string_view->text_length > max)
    {
      nbytes = g_utf8_offset_to_pointer (string_view->text, max) - string_view->text;

      string_view->text_size = nbytes + 1;
      string_view->text = g_realloc (string_view->text, string_view->text_size);
      string_view->text[nbytes] = '\0';
      string_view->n_bytes = nbytes;
      string_view->text_length = max;

      if (string_view->current_pos > string_view->text_length)
        string_view->current_pos = string_view->text_length;

      if (string_view->auto_resize)
        gtk_widget_queue_resize (GTK_WIDGET (string_view));

      scim_string_view_recompute (string_view);
    }

  string_view->text_max_length = max;
  g_object_notify (G_OBJECT (string_view), "max_length");
}

gint
scim_string_view_get_max_length (ScimStringView *string_view)
{
  g_return_val_if_fail (SCIM_IS_STRING_VIEW (string_view), 0);
  return string_view->text_max_length;
}

void
scim_string_view_set_width_chars (ScimStringView *string_view, gint n_chars)
{
  g_return_if_fail (SCIM_IS_STRING_VIEW (string_view));

  if (string_view->width_chars != n_chars)
    {
      string_view->width_chars = n_chars;
      g_object_notify (G_OBJECT (string_view), "width_chars");
      gtk_widget_queue_resize (GTK_WIDGET (string_view));
    }
}

gint
scim_string_view_get_width_chars (ScimStringView *string_view)
{
  g_return_val_if_fail (SCIM_IS_STRING_VIEW (string_view), 0);
  return string_view->width_chars;
}

void
scim_string_view_set_has_frame (ScimStringView *string_view, gboolean setting)
{
  g_return_if_fail (SCIM_IS_STRING_VIEW (string_view));

  setting = (setting != FALSE);
  if (string_view->has_frame == setting)
    return;

  string_view->has_frame = setting;
  gtk_widget_queue_resize (GTK_WIDGET (string_view));
  g_object_notify (G_OBJECT (string_view), "has_frame");
}

gboolean
scim_string_view_get_has_frame (ScimStringView *string_view)
{
  g_return_val_if_fail (SCIM_IS_STRING_VIEW (string_view), FALSE);
  return string_view->has_frame;
}

PangoLayout*
scim_string_view_get_layout (ScimStringView *string_view)
{
  g_return_val_if_fail (SCIM_IS_STRING_VIEW (string_view), NULL);
  return scim_string_view_ensure_layout (string_view);
}

void
scim_string_view_get_layout_offsets (ScimStringView *string_view, gint *x, gint *y)
{
  g_return_if_fail (SCIM_IS_STRING_VIEW (string_view));
  get_layout_position (string_view, x, y);
}

void
scim_string_view_set_highlight (ScimStringView *string_view, gint start, gint end)
{
  g_return_if_fail (SCIM_IS_STRING_VIEW (string_view));

  string_view->highlight_start = start;
  string_view->highlight_end = end;

  gtk_widget_queue_draw (GTK_WIDGET (string_view));
}

void
scim_string_view_get_highlight (ScimStringView *string_view, gint *start, gint *end)
{
  g_return_if_fail (SCIM_IS_STRING_VIEW (string_view));

  if (start) *start = string_view->highlight_start;
  if (end)   *end   = string_view->highlight_end;
}

void
scim_string_view_set_attributes (ScimStringView *entry, PangoAttrList *attrs)
{
  g_return_if_fail (SCIM_IS_STRING_VIEW (entry));

  if (attrs)
    pango_attr_list_ref (attrs);

  if (entry->attrs)
    pango_attr_list_unref (entry->attrs);

  entry->attrs = attrs;

  scim_string_view_recompute (entry);
}

PangoAttrList *
scim_string_view_get_attributes (ScimStringView *entry)
{
  g_return_val_if_fail (SCIM_IS_STRING_VIEW (entry), NULL);
  return entry->attrs;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
