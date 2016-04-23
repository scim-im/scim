#include <string.h>
#include <stdio.h>

#include <pango/pango.h>

#include <glib.h>
#include <gtk/gtk.h>
#if GTK_CHECK_VERSION(2, 22, 0)
#else
#include <gtk/gtkmain.h>
#include <gtk/gtksettings.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkgc.h>
#include <gtk/gtkstyle.h>
#endif

#include "scim_private.h"
#include "scimstringview.h"

#define MIN_STRING_VIEW_WIDTH  64
#define MAX_STRING_VIEW_WIDTH  400
#define DRAW_TIMEOUT     20
#define INNER_BORDER     2

/* Initial size of buffer, in bytes */
#define MIN_SIZE 16

/* Maximum size of text buffer, in bytes */
#define MAX_SIZE G_MAXUSHORT

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

/* GObject, GtkObject methods
 */
static void   scim_string_view_class_init      (ScimStringViewClass  *klass);
static void   scim_string_view_init            (ScimStringView       *string_view);
static void   scim_string_view_set_property    (GObject              *object,
                                                guint                 prop_id,
                                                const GValue         *value,
                                                GParamSpec           *pspec);
static void   scim_string_view_get_property    (GObject              *object,
                                                guint                 prop_id,
                                                GValue               *value,
                                                GParamSpec           *pspec);
static void   scim_string_view_finalize        (GObject              *object);

/* GtkWidget methods
 */
static void   scim_string_view_realize         (GtkWidget        *widget);
static void   scim_string_view_unrealize       (GtkWidget        *widget);
static void   scim_string_view_size_request    (GtkWidget        *widget,
                                                GtkRequisition   *requisition);
#if GTK_CHECK_VERSION(3, 0, 0)
static void   scim_widget_get_preferred_width  (GtkWidget        *widget,
                                                gint             *minimal_width,
                                                gint             *natural_width);
static void   scim_widget_get_preferred_height (GtkWidget        *widget,
                                                gint             *minimal_height,
                                                gint             *natural_height);
#endif
static void   scim_string_view_size_allocate   (GtkWidget        *widget,
                                                GtkAllocation    *allocation);
#if GTK_CHECK_VERSION(3, 0, 0)
static void   scim_string_view_draw_frame      (GtkWidget        *widget,
                                                cairo_t          *cr);
static gint   scim_string_view_draw            (GtkWidget        *widget,
                                                cairo_t          *cr);
#else
static void   scim_string_view_draw_frame      (GtkWidget        *widget);
static gint   scim_string_view_expose          (GtkWidget        *widget,
                                                GdkEventExpose   *event);
#endif
static gint   scim_string_view_button_press    (GtkWidget        *widget,
                                                GdkEventButton   *event);
static gint   scim_string_view_focus_in        (GtkWidget        *widget,
                                                GdkEventFocus    *event);
static gint   scim_string_view_focus_out       (GtkWidget        *widget,
                                                GdkEventFocus    *event);
static void   scim_string_view_style_set       (GtkWidget        *widget,
                                                GtkStyle         *previous_style);
static void   scim_string_view_direction_changed (GtkWidget      *widget,
                                                  GtkTextDirection  previous_dir);
static void   scim_string_view_state_changed   (GtkWidget        *widget,
                                                GtkStateType      previous_state);

/* Internal routines
 */
static void         scim_string_view_draw_text                (ScimStringView       *view);
static void         scim_string_view_draw_cursor              (ScimStringView       *view);
static PangoLayout *scim_string_view_ensure_layout            (ScimStringView       *view);
static void         scim_string_view_queue_draw               (ScimStringView       *view);
static void         scim_string_view_recompute                (ScimStringView       *view);
static gint         scim_string_view_find_position            (ScimStringView       *view,
                                                               gint                  x);
static gint         scim_string_view_get_cursor_locations     (ScimStringView       *view,
                                                               gint                 *strong_x,
                                                               gint                 *weak_x);
static void         scim_string_view_adjust_scroll            (ScimStringView       *view);
static void         get_text_area_size                        (ScimStringView       *string_view,
                                                               gint                 *x,
                                                               gint                 *y,
                                                               gint                 *width,
                                                               gint                 *height);
static void         get_widget_window_size                    (ScimStringView       *string_view,
                                                               gint                 *x,
                                                               gint                 *y,
                                                               gint                 *width,
                                                               gint                 *height);

static void         scim_string_view_keymap_direction_changed (GdkKeymap            *keymap,
                                                               ScimStringView       *string_view);

static void         scim_string_view_check_cursor_blink       (ScimStringView       *string_view);
static void         scim_string_view_move_cursor              (ScimStringView       *string_view,
                                                               guint                 position);

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
    (GClassInitFunc) scim_string_view_class_init,
    NULL,
    NULL,
    sizeof (ScimStringView),
    0,
    (GInstanceInitFunc) scim_string_view_init,
    0
  };

  if (!string_view_type)
    {
      if (type_module)
        string_view_type = g_type_module_register_type (type_module,
                                                        GTK_TYPE_WIDGET,
                                                        "SCIM_ScimStringView",
                                                        &string_view_info,
                                                        (GTypeFlags) 0);
      else
        string_view_type = g_type_register_static (GTK_TYPE_WIDGET,
                                                   "SCIM_ScimStringView",
                                                   &string_view_info,
                                                   (GTypeFlags) 0);
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
scim_string_view_class_init (ScimStringViewClass *class)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

#if GTK_CHECK_VERSION(3, 0, 0)
#else
  GtkObjectClass *object_class;
#endif
  GtkWidgetClass *widget_class;

#if GTK_CHECK_VERSION(3, 0, 0)
#else
  object_class = (GtkObjectClass*) class;
#endif
  widget_class = (GtkWidgetClass*) class;
  parent_class = g_type_class_ref (GTK_TYPE_WIDGET);

  gobject_class->finalize = scim_string_view_finalize;
  gobject_class->set_property = scim_string_view_set_property;
  gobject_class->get_property = scim_string_view_get_property;

  widget_class->realize = scim_string_view_realize;
  widget_class->unrealize = scim_string_view_unrealize;
#if GTK_CHECK_VERSION(3, 0, 0)
  widget_class->get_preferred_width = scim_widget_get_preferred_width;
  widget_class->get_preferred_height = scim_widget_get_preferred_height;
#else
  widget_class->size_request = scim_string_view_size_request;
#endif
  widget_class->size_allocate = scim_string_view_size_allocate;
#if GTK_CHECK_VERSION(3, 0, 0)
  widget_class->draw = scim_string_view_draw;
#else
  widget_class->expose_event = scim_string_view_expose;
#endif
  widget_class->button_press_event = scim_string_view_button_press;
  widget_class->focus_in_event = scim_string_view_focus_in;
  widget_class->focus_out_event = scim_string_view_focus_out;
  widget_class->style_set = scim_string_view_style_set;
  widget_class->direction_changed = scim_string_view_direction_changed;
  widget_class->state_changed = scim_string_view_state_changed;

  class->move_cursor = scim_string_view_move_cursor;

  g_object_class_install_property (gobject_class,
                                   PROP_CURSOR_POSITION,
                                   g_param_spec_int ("cursor_position",
                                                     _("Cursor Position"),
                                                     _("The current position of the insertion cursor in chars."),
                                                     0,
                                                     MAX_SIZE,
                                                     0,
                                                     G_PARAM_READABLE));

  g_object_class_install_property (gobject_class,
                                   PROP_MAX_LENGTH,
                                   g_param_spec_int ("max_length",
                                                     _("Maximum length"),
                                                     _("Maximum number of characters for this string view. Zero if no maximum."),
                                                     0,
                                                     MAX_SIZE,
                                                     0,
                                                     G_PARAM_READABLE | G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class,
                                   PROP_MAX_WIDTH,
                                   g_param_spec_int ("max_width",
                                                     _("Maximum width"),
                                                     _("Maximum width of this string view."),
                                                     0,
                                                     MAX_STRING_VIEW_WIDTH,
                                                     0,
                                                     G_PARAM_READABLE | G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class,
                                   PROP_HAS_FRAME,
                                   g_param_spec_boolean ("has_frame",
                                                         _("Has Frame"),
                                                         _("FALSE removes outside bevel from string view."),
                                                         TRUE,
                                                         G_PARAM_READABLE | G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class,
                                   PROP_DRAW_CURSOR,
                                   g_param_spec_boolean ("draw_cursor",
                                                         _("Draw cursor"),
                                                         _("TRUE draw blinking cursor."),
                                                         TRUE,
                                                         G_PARAM_READABLE | G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class,
                                   PROP_AUTO_MOVE_CURSOR,
                                   g_param_spec_boolean ("auto_move_cursor",
                                                         _("Auto move cursor"),
                                                         _("TRUE auto move cursor position when mouse clicking."),
                                                         FALSE,
                                                         G_PARAM_READABLE | G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class,
                                   PROP_FORWARD_EVENT,
                                   g_param_spec_boolean ("forward_event",
                                                         _("Forward button press event"),
                                                         _("TRUE forward button press event to user program."),
                                                         FALSE,
                                                         G_PARAM_READABLE | G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class,
                                   PROP_FORWARD_EVENT,
                                   g_param_spec_boolean ("auto_resize",
                                                         _("Auto resize the widget to fit the string"),
                                                         _("TRUE Auto resize on."),
                                                         FALSE,
                                                         G_PARAM_READABLE | G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class,
                                   PROP_WIDTH_CHARS,
                                   g_param_spec_int ("width_chars",
                                                     _("Width in chars"),
                                                     _("Number of characters to leave space for in the string view."),
                                                     -1,
                                                     G_MAXINT,
                                                     -1,
                                                     G_PARAM_READABLE | G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class,
                                   PROP_SCROLL_OFFSET,
                                   g_param_spec_int ("scroll_offset",
                                                     _("Scroll offset"),
                                                     _("Number of pixels of the string view scrolled off the screen to the left"),
                                                     0,
                                                     G_MAXINT,
                                                     0,
                                                     G_PARAM_READABLE));

  g_object_class_install_property (gobject_class,
                                   PROP_TEXT,
                                   g_param_spec_string ("text",
                                                        _("Text"),
                                                        _("The contents of the string view"),
                                                        "",
                                                        G_PARAM_READABLE | G_PARAM_WRITABLE));

  string_view_signals[MOVE_CURSOR] = g_signal_new ("move_cursor",
#if GTK_CHECK_VERSION(3, 0, 0)
                                        G_TYPE_FROM_CLASS (gobject_class),
#else
                                        G_TYPE_FROM_CLASS (object_class),
#endif
                                        G_SIGNAL_RUN_FIRST,
                                        G_STRUCT_OFFSET (ScimStringViewClass, move_cursor),
                                        NULL,
                                        NULL,
                                        g_cclosure_marshal_VOID__UINT,
                                        G_TYPE_NONE, 1, G_TYPE_UINT);

}

static void
scim_string_view_set_property (GObject         *object,
                              guint            prop_id,
                              const GValue    *value,
                              GParamSpec      *pspec)
{
  ScimStringView *view = SCIM_STRING_VIEW (object);

  switch (prop_id)
    {
    case PROP_MAX_LENGTH:
      scim_string_view_set_max_length (view, g_value_get_int (value));
      break;

    case PROP_MAX_WIDTH:
      scim_string_view_set_max_width (view, g_value_get_int (value));
      break;

    case PROP_HAS_FRAME:
      scim_string_view_set_has_frame (view, g_value_get_boolean (value));
      break;

    case PROP_DRAW_CURSOR:
      scim_string_view_set_draw_cursor (view, g_value_get_boolean (value));
      break;

    case PROP_AUTO_MOVE_CURSOR:
      scim_string_view_set_auto_move_cursor (view, g_value_get_boolean (value));
      break;

    case PROP_FORWARD_EVENT:
      scim_string_view_set_forward_event (view, g_value_get_boolean (value));
      break;

    case PROP_AUTO_RESIZE:
      scim_string_view_set_auto_resize (view, g_value_get_boolean (value));
      break;

    case PROP_WIDTH_CHARS:
      scim_string_view_set_width_chars (view, g_value_get_int (value));
      break;

    case PROP_TEXT:
      scim_string_view_set_text (view, g_value_get_string (value));
      break;

    case PROP_SCROLL_OFFSET:
    case PROP_CURSOR_POSITION:
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
scim_string_view_get_property (GObject         *object,
                              guint            prop_id,
                              GValue          *value,
                              GParamSpec      *pspec)
{
  ScimStringView *view = SCIM_STRING_VIEW (object);

  switch (prop_id)
    {
    case PROP_CURSOR_POSITION:
      g_value_set_int (value, view->current_pos);
      break;
    case PROP_MAX_LENGTH:
      g_value_set_int (value, view->text_max_length);
      break;
    case PROP_MAX_WIDTH:
      g_value_set_int (value, view->max_width);
      break;
    case PROP_HAS_FRAME:
      g_value_set_boolean (value, view->has_frame);
      break;
    case PROP_DRAW_CURSOR:
      g_value_set_boolean (value, view->draw_cursor);
      break;
    case PROP_AUTO_MOVE_CURSOR:
      g_value_set_boolean (value, view->auto_move_cursor);
      break;
    case PROP_FORWARD_EVENT:
      g_value_set_boolean (value, view->forward_event);
      break;
    case PROP_AUTO_RESIZE:
      g_value_set_boolean (value, view->auto_resize);
      break;
    case PROP_WIDTH_CHARS:
      g_value_set_int (value, view->width_chars);
      break;
    case PROP_SCROLL_OFFSET:
      g_value_set_int (value, view->scroll_offset);
      break;
    case PROP_TEXT:
      g_value_set_string (value, scim_string_view_get_text (view));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
scim_string_view_init (ScimStringView *string_view)
{
#if GTK_CHECK_VERSION(2, 18, 0)
  gtk_widget_set_can_focus (GTK_WIDGET (string_view), TRUE);
#else
  GTK_WIDGET_SET_FLAGS (string_view, GTK_CAN_FOCUS);
#endif

  string_view->text_size = MIN_SIZE;
  string_view->text = g_malloc (string_view->text_size);
  string_view->text[0] = '\0';

  string_view->width_chars = -1;
  string_view->has_frame = TRUE;
  string_view->draw_cursor = TRUE;
  string_view->auto_move_cursor = FALSE;
  string_view->forward_event = FALSE;
  string_view->max_width = -1;
  string_view->highlight_start = -1;
  string_view->highlight_end = -1;
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
scim_string_view_realize (GtkWidget *widget)
{
  ScimStringView *string_view;
  GdkWindowAttr attributes;
  gint attributes_mask;

#if GTK_CHECK_VERSION(2, 20, 0)
  gtk_widget_set_realized (widget, TRUE);
#else
  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
#endif
  string_view = SCIM_STRING_VIEW (widget);

  attributes.window_type = GDK_WINDOW_CHILD;

  get_widget_window_size (string_view, &attributes.x, &attributes.y, &attributes.width, &attributes.height);

  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual (widget);
#if GTK_CHECK_VERSION(3, 0, 0)
#else
  attributes.colormap = gtk_widget_get_colormap (widget);
#endif
  attributes.event_mask = gtk_widget_get_events (widget);
  attributes.event_mask |= (GDK_EXPOSURE_MASK |
                            GDK_BUTTON_PRESS_MASK |
                            GDK_BUTTON_RELEASE_MASK);

#if GTK_CHECK_VERSION(3, 0, 0)
  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;
#else
  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
#endif

#if GTK_CHECK_VERSION(3, 0, 0)
  gtk_widget_set_window (widget, gdk_window_new (gtk_widget_get_parent_window (widget), &attributes, attributes_mask));
#else
  widget->window = gdk_window_new (gtk_widget_get_parent_window (widget), &attributes, attributes_mask);
#endif
#if GTK_CHECK_VERSION(2, 14, 0)
  gdk_window_set_user_data (gtk_widget_get_window (widget), string_view);
#else
  gdk_window_set_user_data (widget->window, string_view);
#endif

  get_text_area_size (string_view, &attributes.x, &attributes.y, &attributes.width, &attributes.height);

  attributes.cursor = gdk_cursor_new (GDK_XTERM);
  attributes_mask |= GDK_WA_CURSOR;

#if GTK_CHECK_VERSION(2, 14, 0)
  string_view->text_area = gdk_window_new (gtk_widget_get_window (widget), &attributes, attributes_mask);
#else
  string_view->text_area = gdk_window_new (widget->window, &attributes, attributes_mask);
#endif
  gdk_window_set_user_data (string_view->text_area, string_view);

#if GTK_CHECK_VERSION(3, 0, 0)
  g_object_unref (attributes.cursor);
#else
  gdk_cursor_unref (attributes.cursor);
#endif

#if GTK_CHECK_VERSION(3, 0, 0)
  GtkStyleContext *style = gtk_widget_get_style_context (widget);
  GdkRGBA bg_color;
  gtk_style_context_get_background_color (style, gtk_widget_get_state_flags (widget), &bg_color);
  gdk_window_set_background_rgba (gtk_widget_get_window (widget), &bg_color);
  gdk_window_set_background_rgba (string_view->text_area, &bg_color);
#else
  widget->style = gtk_style_attach (widget->style, gtk_widget_get_window (widget));

#if GTK_CHECK_VERSION(2, 14, 0)
  gdk_window_set_background (gtk_widget_get_window (widget), &widget->style->base[gtk_widget_get_state (widget)]);
  gdk_window_set_background (string_view->text_area, &widget->style->base[gtk_widget_get_state (widget)]);
#else
  gdk_window_set_background (widget->window, &widget->style->base[GTK_WIDGET_STATE (widget)]);
  gdk_window_set_background (string_view->text_area, &widget->style->base[GTK_WIDGET_STATE (widget)]);
#endif
#endif

  gdk_window_show (string_view->text_area);

  scim_string_view_adjust_scroll (string_view);
}

static void
scim_string_view_unrealize (GtkWidget *widget)
{
  ScimStringView *string_view = SCIM_STRING_VIEW (widget);

  if (string_view->text_area)
    {
      gdk_window_set_user_data (string_view->text_area, NULL);
      gdk_window_destroy (string_view->text_area);
      string_view->text_area = NULL;
    }

  if (GTK_WIDGET_CLASS (parent_class)->unrealize)
    (* GTK_WIDGET_CLASS (parent_class)->unrealize) (widget);
}

static void
get_borders (ScimStringView *string_view,
             gint     *xborder,
             gint     *yborder)
{
  GtkWidget *widget = GTK_WIDGET (string_view);
  gint focus_width;
  gboolean interior_focus;

  gtk_widget_style_get (widget,
                        "interior-focus", &interior_focus,
                        "focus-line-width", &focus_width,
                        NULL);

  if (string_view->has_frame)
    {
#if GTK_CHECK_VERSION(3, 0, 0)
      GtkStyleContext *style_context = gtk_widget_get_style_context (widget);
      GtkBorder border;
      gtk_style_context_get_border (style_context, gtk_widget_get_state_flags (widget), &border);
      *xborder = border.left;
      *yborder = border.top;
#else
      *xborder = widget->style->xthickness;
      *yborder = widget->style->ythickness;
#endif
    }
  else
    {
      *xborder = 0;
      *yborder = 0;
    }

  if (!interior_focus)
    {
      *xborder += focus_width;
      *yborder += focus_width;
    }
}

static void
scim_string_view_size_request (GtkWidget      *widget,
                        GtkRequisition *requisition)
{
  ScimStringView *string_view = SCIM_STRING_VIEW (widget);
  PangoFontMetrics *metrics;
  gint xborder, yborder;
  PangoContext *context;
  PangoLayout *layout;
  int width, height;

  context = gtk_widget_get_pango_context (widget);
  metrics = pango_context_get_metrics (context,
#if GTK_CHECK_VERSION(3, 0, 0)
                                       pango_context_get_font_description (context),
#else
                                       widget->style->font_desc,
#endif
                                       pango_context_get_language (context));

  string_view->ascent = pango_font_metrics_get_ascent (metrics);
  string_view->descent = pango_font_metrics_get_descent (metrics);

  get_borders (string_view, &xborder, &yborder);

  xborder += INNER_BORDER;
  yborder += INNER_BORDER;

  if (string_view->auto_resize) {
    layout = scim_string_view_ensure_layout (string_view);
    pango_layout_get_pixel_size (layout, &width, &height);
    requisition->width = ((width<MIN_STRING_VIEW_WIDTH)? MIN_STRING_VIEW_WIDTH : width) + 2;
  } else {
    if (string_view->width_chars < 0)
      requisition->width = MIN_STRING_VIEW_WIDTH;
    else
      {
        gint char_width = pango_font_metrics_get_approximate_char_width (metrics);
        requisition->width = PANGO_PIXELS (char_width) * string_view->width_chars;
      }
  }

  if (string_view->max_width > 0 && requisition->width > string_view->max_width)
    requisition->width = string_view->max_width;

  requisition->width += xborder * 2;
  requisition->height = PANGO_PIXELS (string_view->ascent + string_view->descent) + yborder * 2;

  pango_font_metrics_unref (metrics);
}

#if GTK_CHECK_VERSION(3, 0, 0)
static void
scim_widget_get_preferred_width (GtkWidget *widget,
                                 gint      *minimal_width,
                                 gint      *natural_width)
{
    GtkRequisition requisition;
    scim_string_view_size_request (widget, &requisition);
    *minimal_width = *natural_width = requisition.width;
}

static void
scim_widget_get_preferred_height (GtkWidget *widget,
                                  gint      *minimal_height,
                                  gint      *natural_height)
{
    GtkRequisition requisition;
    scim_string_view_size_request (widget, &requisition);
    *minimal_height = *natural_height = requisition.height;
}
#endif

static void
get_text_area_size (ScimStringView *string_view,
                    gint     *x,
                    gint     *y,
                    gint     *width,
                    gint     *height)
{
  gint xborder, yborder;
  GtkRequisition requisition;
  GtkWidget *widget = GTK_WIDGET (string_view);

#if GTK_CHECK_VERSION(3, 0, 0)
  gtk_widget_get_preferred_size (widget, &requisition, NULL);
#else
  gtk_widget_get_child_requisition (widget, &requisition);
#endif

  get_borders (string_view, &xborder, &yborder);

  if (x)
    *x = xborder;

  if (y)
    *y = yborder;

  if (width)
    {
#if GTK_CHECK_VERSION(3, 0, 0)
      GtkAllocation allocation;
      gtk_widget_get_allocation (widget, &allocation);
      *width = allocation.width - xborder * 2;
#else
      *width = GTK_WIDGET (string_view)->allocation.width - xborder * 2;
#endif
    }

  if (height)
    *height = requisition.height - yborder * 2;
}

static void
get_widget_window_size (ScimStringView *string_view,
                        gint     *x,
                        gint     *y,
                        gint     *width,
                        gint     *height)
{
  GtkRequisition requisition;
  GtkWidget *widget = GTK_WIDGET (string_view);

  // FIXME
#if GTK_CHECK_VERSION(3, 0, 0)
  gtk_widget_get_preferred_size (widget, &requisition, NULL);
#else
  gtk_widget_get_child_requisition (widget, &requisition);
#endif

#if GTK_CHECK_VERSION(3, 0, 0)
  GtkAllocation allocation;
  gtk_widget_get_allocation (widget, &allocation);
#else
#endif

  if (x)
    {
#if GTK_CHECK_VERSION(3, 0, 0)
      *x = allocation.x;
#else
      *x = widget->allocation.x;
#endif
    }

  if (y)
    {
#if GTK_CHECK_VERSION(3, 0, 0)
        *y = allocation.y + (allocation.height - requisition.height) / 2;
#else
        *y = widget->allocation.y + (widget->allocation.height - requisition.height) / 2;
#endif
    }

  if (width)
#if GTK_CHECK_VERSION(3, 0, 0)
    *width = allocation.width;
#else
    *width = widget->allocation.width;
#endif

  if (height)
    *height = requisition.height;
}

static void
scim_string_view_size_allocate (GtkWidget     *widget,
                         GtkAllocation *allocation)
{
  ScimStringView *string_view = SCIM_STRING_VIEW (widget);

#if GTK_CHECK_VERSION(3, 0, 0)
  gtk_widget_set_allocation (widget, allocation);
#else
  widget->allocation = *allocation;
#endif

#if GTK_CHECK_VERSION(2, 20, 0)
  if (gtk_widget_get_realized (widget))
#else
  if (GTK_WIDGET_REALIZED (widget))
#endif
    {
      /* We call gtk_widget_get_child_requisition, since we want (for
       * backwards compatibility reasons) the realization here to
       * be affected by the usize of the string_view, if set
       */
      gint x, y, width, height;

      get_widget_window_size (string_view, &x, &y, &width, &height);

#if GTK_CHECK_VERSION(2, 14, 0)
      gdk_window_move_resize (gtk_widget_get_window (widget),
#else
      gdk_window_move_resize (widget->window,
#endif
                              x, y, width, height);

      get_text_area_size (string_view, &x, &y, &width, &height);

      gdk_window_move_resize (string_view->text_area,
                              x, y, width, height);

      scim_string_view_recompute (string_view);
    }
}

static void
#if GTK_CHECK_VERSION(3, 0, 0)
scim_string_view_draw_frame (GtkWidget *widget, cairo_t *cr)
#else
scim_string_view_draw_frame (GtkWidget *widget)
#endif
{
  gint x = 0, y = 0;
  gint width, height;
  gboolean interior_focus;
  gint focus_width;

  gtk_widget_style_get (widget,
                        "interior-focus", &interior_focus,
                        "focus-line-width", &focus_width,
                        NULL);

#if GTK_CHECK_VERSION(2, 24, 0)
  width = gdk_window_get_width(gtk_widget_get_window (widget));
  height = gdk_window_get_height(gtk_widget_get_window (widget));
#elif GTK_CHECK_VERSION(2, 14, 0)
  gdk_window_get_size (gtk_widget_get_window (widget), &width, &height);
#else
  gdk_window_get_size (widget->window, &width, &height);
#endif

#if GTK_CHECK_VERSION(2, 18, 0)
  if (gtk_widget_has_focus (widget) && !interior_focus)
#else
  if (GTK_WIDGET_HAS_FOCUS (widget) && !interior_focus)
#endif
    {
      x += focus_width;
      y += focus_width;
      width -= 2 * focus_width;
      height -= 2 * focus_width;
    }

#if GTK_CHECK_VERSION(3, 0, 0)
  GtkStyleContext *style = gtk_widget_get_style_context(widget);
  gtk_style_context_save (style);
  gtk_style_context_set_state (style, GTK_STATE_FLAG_NORMAL);
  gtk_render_frame (style, cr, x, y, width, height);
#else
#if GTK_CHECK_VERSION(2, 14, 0)
  gtk_paint_shadow (widget->style, gtk_widget_get_window (widget),
#else
  gtk_paint_shadow (widget->style, widget->window,
#endif
                    GTK_STATE_NORMAL, GTK_SHADOW_IN,
                    NULL, widget, "entry",
                    x, y, width, height);
#endif

#if GTK_CHECK_VERSION(2, 18, 0)
  if (gtk_widget_has_focus (widget) && !interior_focus)
#else
  if (GTK_WIDGET_HAS_FOCUS (widget) && !interior_focus)
#endif
    {
      x -= focus_width;
      y -= focus_width;
      width += 2 * focus_width;
      height += 2 * focus_width;

#if GTK_CHECK_VERSION(3, 0, 0)
      GtkStyleContext *style = gtk_widget_get_style_context(widget);
      gtk_style_context_save (style);
      gtk_style_context_set_state (style, gtk_widget_get_state_flags (widget));
      gtk_render_focus (style, cr, 0, 0, width, height);
#else
#if GTK_CHECK_VERSION(2, 18, 0)
      gtk_paint_focus (widget->style, gtk_widget_get_window (widget), gtk_widget_get_state (widget),
#elif GTK_CHECK_VERSION(2, 14, 0)
      gtk_paint_focus (widget->style, gtk_widget_get_window (widget), GTK_WIDGET_STATE (widget),
#else
      gtk_paint_focus (widget->style, widget->window, GTK_WIDGET_STATE (widget),
#endif
                       NULL, widget, "entry",
                       0, 0, width, height);
#endif
    }
}

static gint
#if GTK_CHECK_VERSION(3, 0, 0)
scim_string_view_draw (GtkWidget      *widget,
                       cairo_t        *cr)
#else
scim_string_view_expose (GtkWidget      *widget,
                         GdkEventExpose *event)
#endif
{
  ScimStringView *string_view = SCIM_STRING_VIEW (widget);

#if GTK_CHECK_VERSION(3, 0, 0)
  if (gtk_cairo_should_draw_window (cr, gtk_widget_get_window (widget)))
    scim_string_view_draw_frame (widget, cr);
  if (gtk_cairo_should_draw_window (cr, string_view->text_area))
#else
#if GTK_CHECK_VERSION(2, 14, 0)
  if (gtk_widget_get_window (widget) == event->window)
#else
  if (widget->window == event->window)
#endif
    scim_string_view_draw_frame (widget);
  else if (string_view->text_area == event->window)
#endif
    {
      gint area_width, area_height;

      get_text_area_size (string_view, NULL, NULL, &area_width, &area_height);

#if GTK_CHECK_VERSION(3, 0, 0)
      GtkStyleContext *style = gtk_widget_get_style_context(widget);
      gtk_style_context_save (style);
      gtk_style_context_set_state (style, gtk_widget_get_state_flags (widget));
      gtk_render_frame (style, cr, 0, 0, area_width, area_height);
#else
      gtk_paint_flat_box (widget->style, string_view->text_area,
#if GTK_CHECK_VERSION(2, 18, 0)
                          gtk_widget_get_state (widget), GTK_SHADOW_NONE,
#else
                          GTK_WIDGET_STATE(widget), GTK_SHADOW_NONE,
#endif
                          NULL, widget, "entry_bg",
                          0, 0, area_width, area_height);
#endif

      scim_string_view_draw_text (SCIM_STRING_VIEW (widget));

      if (string_view->cursor_visible && string_view->draw_cursor)
        scim_string_view_draw_cursor (SCIM_STRING_VIEW (widget));

    }

  return FALSE;
}

static gint
scim_string_view_button_press (GtkWidget      *widget,
                        GdkEventButton *event)
{
  ScimStringView *string_view = SCIM_STRING_VIEW (widget);

  gint tmp_pos;

  if (event->window != string_view->text_area)
    return FALSE;

  tmp_pos = scim_string_view_find_position (string_view, event->x + string_view->scroll_offset);

  if (event->button == 1)
    {
      g_signal_emit (G_OBJECT (widget),
                     string_view_signals [MOVE_CURSOR],
                     0,
                     tmp_pos);
      return !string_view->forward_event;
    }

  return FALSE;
}

static gint
scim_string_view_focus_in (GtkWidget     *widget,
                    GdkEventFocus *event)
{
  ScimStringView *string_view = SCIM_STRING_VIEW (widget);

  gtk_widget_queue_draw (widget);

  g_signal_connect (gdk_keymap_get_default (),
                    "direction_changed",
                    G_CALLBACK (scim_string_view_keymap_direction_changed), string_view);

  scim_string_view_check_cursor_blink (string_view);

  return FALSE;
}

static gint
scim_string_view_focus_out (GtkWidget     *widget,
                     GdkEventFocus *event)
{
  ScimStringView *string_view = SCIM_STRING_VIEW (widget);

  gtk_widget_queue_draw (widget);

  scim_string_view_check_cursor_blink (string_view);

  g_signal_handlers_disconnect_by_func (gdk_keymap_get_default (),
                                        (gpointer) scim_string_view_keymap_direction_changed,
                                        string_view);

  return FALSE;
}

static void
scim_string_view_direction_changed (GtkWidget        *widget,
                             GtkTextDirection  previous_dir)
{
  ScimStringView *string_view = SCIM_STRING_VIEW (widget);

  scim_string_view_recompute (string_view);

  GTK_WIDGET_CLASS (parent_class)->direction_changed (widget, previous_dir);
}

static void
scim_string_view_state_changed (GtkWidget      *widget,
                         GtkStateType    previous_state)
{
  ScimStringView *string_view = SCIM_STRING_VIEW (widget);

#if GTK_CHECK_VERSION(2, 20, 0)
  if (gtk_widget_get_realized (widget))
#else
  if (GTK_WIDGET_REALIZED (widget))
#endif
    {
#if GTK_CHECK_VERSION(3, 0, 0)
      GtkStyleContext *style = gtk_widget_get_style_context (widget);
      GdkRGBA bg_color;
      gtk_style_context_get_background_color (style, gtk_widget_get_state_flags (widget), &bg_color);
      gdk_window_set_background_rgba (gtk_widget_get_window (widget), &bg_color);
      gdk_window_set_background_rgba (string_view->text_area, &bg_color);
#else
#if GTK_CHECK_VERSION(2, 18, 0)
      gdk_window_set_background (gtk_widget_get_window (widget), &widget->style->base[gtk_widget_get_state (widget)]);
      gdk_window_set_background (string_view->text_area, &widget->style->base[gtk_widget_get_state (widget)]);
#else
      gdk_window_set_background (widget->window, &widget->style->base[GTK_WIDGET_STATE (widget)]);
      gdk_window_set_background (string_view->text_area, &widget->style->base[GTK_WIDGET_STATE (widget)]);
#endif
#endif
    }

  gtk_widget_queue_draw (widget);
}


static void
scim_string_view_style_set        (GtkWidget      *widget,
                         GtkStyle       *previous_style)
{
  ScimStringView *string_view = SCIM_STRING_VIEW (widget);

#if GTK_CHECK_VERSION(2, 20, 0)
  if (previous_style && gtk_widget_get_realized (widget))
#else
  if (previous_style && GTK_WIDGET_REALIZED (widget))
#endif
    {
      scim_string_view_recompute (string_view);

#if GTK_CHECK_VERSION(3, 0, 0)
      GtkStyleContext *style = gtk_widget_get_style_context (widget);
      GdkRGBA bg_color;
      gtk_style_context_get_background_color (style, gtk_widget_get_state_flags (widget), &bg_color);
      gdk_window_set_background_rgba (gtk_widget_get_window (widget), &bg_color);
      gdk_window_set_background_rgba (string_view->text_area, &bg_color);
#else
#if GTK_CHECK_VERSION(2, 18, 0)
      gdk_window_set_background (gtk_widget_get_window (widget), &widget->style->base[gtk_widget_get_state (widget)]);
      gdk_window_set_background (string_view->text_area, &widget->style->base[gtk_widget_get_state (widget)]);
#else
      gdk_window_set_background (widget->window, &widget->style->base[GTK_WIDGET_STATE (widget)]);
      gdk_window_set_background (string_view->text_area, &widget->style->base[GTK_WIDGET_STATE (widget)]);
#endif
#endif
    }
}

/* Default signal handlers
 */

static void
scim_string_view_keymap_direction_changed (GdkKeymap *keymap,
                                    ScimStringView  *string_view)
{
  scim_string_view_queue_draw (string_view);
}



/* Internal functions
 */


static void
scim_string_view_reset_layout (ScimStringView *string_view)
{
  if (string_view->cached_layout)
    {
      g_object_unref (G_OBJECT (string_view->cached_layout));
      string_view->cached_layout = NULL;
    }
}

static gboolean
recompute_idle_func (gpointer data)
{
  ScimStringView *string_view;

#if !GTK_CHECK_VERSION(2, 12, 0)
  GDK_THREADS_ENTER ();
#endif

  string_view = SCIM_STRING_VIEW (data);

  scim_string_view_adjust_scroll (string_view);
  scim_string_view_queue_draw (string_view);

  string_view->recompute_idle = 0;

#if !GTK_CHECK_VERSION(2, 12, 0)
  GDK_THREADS_LEAVE ();
#endif

  return FALSE;
}

static void
scim_string_view_recompute (ScimStringView *string_view)
{
  scim_string_view_reset_layout (string_view);
  scim_string_view_check_cursor_blink (string_view);

  if (!string_view->recompute_idle)
    {
      string_view->recompute_idle =
#if GTK_CHECK_VERSION(2, 12, 0)
        gdk_threads_add_idle_full
#else
        g_idle_add_full
#endif
          (G_PRIORITY_HIGH_IDLE + 15, /* between resize and redraw */
          recompute_idle_func, string_view, NULL);
    }
}

static PangoLayout *
scim_string_view_create_layout (ScimStringView *string_view)
{
  PangoLayout *layout = gtk_widget_create_pango_layout (GTK_WIDGET (string_view), NULL);

  if (!string_view->attrs)
    string_view->attrs = pango_attr_list_new ();

  pango_layout_set_single_paragraph_mode (layout, TRUE);

  pango_layout_set_text (layout, string_view->text, string_view->n_bytes);

  pango_layout_set_attributes (layout, string_view->attrs);

  return layout;
}

static PangoLayout *
scim_string_view_ensure_layout (ScimStringView *string_view)
{
  if (!string_view->cached_layout)
    {
      string_view->cached_layout = scim_string_view_create_layout (string_view);
    }

  return string_view->cached_layout;
}

static void
get_layout_position (ScimStringView *string_view,
                     gint     *x,
                     gint     *y)
{
  PangoLayout *layout;
  PangoRectangle logical_rect;
  gint area_width, area_height;
  gint y_pos;
  PangoLayoutLine *line;

  layout = scim_string_view_ensure_layout (string_view);

  get_text_area_size (string_view, NULL, NULL, &area_width, &area_height);

  area_height = PANGO_SCALE * (area_height - 2 * INNER_BORDER);

  line = pango_layout_get_lines (layout)->data;
  pango_layout_line_get_extents (line, NULL, &logical_rect);

  /* Align primarily for locale's ascent/descent */
  y_pos = ((area_height - string_view->ascent - string_view->descent) / 2 +
           string_view->ascent + logical_rect.y);

  /* Now see if we need to adjust to fit in actual drawn string */
  if (logical_rect.height > area_height)
    y_pos = (area_height - logical_rect.height) / 2;
  else if (y_pos < 0)
    y_pos = 0;
  else if (y_pos + logical_rect.height > area_height)
    y_pos = area_height - logical_rect.height;

  y_pos = INNER_BORDER + y_pos / PANGO_SCALE;

  if (x)
    *x = INNER_BORDER - string_view->scroll_offset;

  if (y)
    *y = y_pos;
}

static void
scim_string_view_draw_text (ScimStringView *string_view)
{
  GtkWidget *widget;

#if GTK_CHECK_VERSION(2, 18, 0)
  if (gtk_widget_is_drawable (GTK_WIDGET (string_view)))
#else
  if (GTK_WIDGET_DRAWABLE (string_view))
#endif
    {
      PangoLayout *layout = scim_string_view_ensure_layout (string_view);
      gint x, y;

      widget = GTK_WIDGET (string_view);

      get_layout_position (string_view, &x, &y);

#if GTK_CHECK_VERSION(3, 0, 0)
      cairo_t *cr = gdk_cairo_create (string_view->text_area);
      GdkRGBA rgba;
      gtk_style_context_get_color (gtk_widget_get_style_context (widget),
                                   gtk_widget_get_state_flags (widget),
                                   &rgba);
      gdk_cairo_set_source_rgba (cr, &rgba);
      cairo_move_to (cr, x, y);
      pango_cairo_show_layout (cr, layout);
      cairo_destroy (cr);
#else
      gdk_draw_layout (string_view->text_area, widget->style->text_gc [widget->state],
                       x, y,
                       layout);
#endif

      if (string_view->highlight_start >=0 &&
          string_view->highlight_start < string_view->highlight_end &&
          string_view->highlight_start < string_view->text_length)
        {
          gint *ranges;
          gint n_ranges, i;
          gint start_pos = string_view->highlight_start;
          gint end_pos = (string_view->text_length > string_view->highlight_end)? string_view->highlight_end : string_view->text_length;
          PangoRectangle logical_rect;
          const gchar *text = pango_layout_get_text (layout);
          gint start_index = g_utf8_offset_to_pointer (text, start_pos) - text;
          gint end_index = g_utf8_offset_to_pointer (text, end_pos) - text;
#if GTK_CHECK_VERSION(3, 0, 0)
          cairo_region_t *clip_region = cairo_region_create ();
          GdkRGBA selection_rgba;
          GdkRGBA text_rgba;
#else
          GdkRegion *clip_region = gdk_region_new ();
          GdkGC *text_gc;
          GdkGC *selection_gc;
#endif
          PangoLayoutLine *line;

          line = pango_layout_get_lines (layout)->data;

          pango_layout_line_get_x_ranges (line, start_index, end_index, &ranges, &n_ranges);

          pango_layout_get_extents (layout, NULL, &logical_rect);

#if GTK_CHECK_VERSION(3, 0, 0)
          gtk_style_context_get_background_color (
              gtk_widget_get_style_context (widget),
              GTK_STATE_FLAG_ACTIVE,
              &selection_rgba);
          gtk_style_context_get_color (
              gtk_widget_get_style_context (widget),
              GTK_STATE_ACTIVE,
              &text_rgba);
#else
          selection_gc = widget->style->base_gc [GTK_STATE_ACTIVE];
          text_gc = widget->style->text_gc [GTK_STATE_ACTIVE];
#endif

          for (i=0; i < n_ranges; ++i)
            {
#if GTK_CHECK_VERSION(3, 0, 0)
              cairo_rectangle_int_t rect;
#else
              GdkRectangle rect;
#endif

              rect.x = INNER_BORDER - string_view->scroll_offset + ranges[2*i] / PANGO_SCALE;
              rect.y = y;
              rect.width = (ranges[2*i + 1] - ranges[2*i]) / PANGO_SCALE;
              rect.height = logical_rect.height / PANGO_SCALE;

#if GTK_CHECK_VERSION(3, 0, 0)
              cairo_t *cr = gdk_cairo_create (string_view->text_area);
              cairo_set_source_surface (cr, cairo_get_target (cr), 0, 0);
              cairo_pattern_set_extend (cairo_get_source (cr), CAIRO_EXTEND_REPEAT);
              gdk_cairo_set_source_rgba (cr, &selection_rgba);
              cairo_rectangle (cr, rect.x, rect.y, rect.width, rect.height);
              cairo_fill (cr);
              cairo_destroy (cr);
              cairo_region_union_rectangle (clip_region, &rect);
#else
              gdk_draw_rectangle (string_view->text_area, selection_gc, TRUE,
                                  rect.x, rect.y, rect.width, rect.height);
              gdk_region_union_with_rect (clip_region, &rect);
#endif

            }

#if GTK_CHECK_VERSION(3, 0, 0)
          cairo_t *cr = gdk_cairo_create (string_view->text_area);
          gdk_cairo_region (cr, clip_region);
          cairo_clip (cr);
          gdk_cairo_set_source_rgba (cr, &text_rgba);
          cairo_move_to (cr, x, y);
          pango_cairo_show_layout (cr, layout);
          cairo_destroy (cr);
          cairo_region_destroy (clip_region);
#else
          gdk_gc_set_clip_region (text_gc, clip_region);
          gdk_draw_layout (string_view->text_area, text_gc,
                           x, y,
                           layout);
          gdk_gc_set_clip_region (text_gc, NULL);
          gdk_region_destroy (clip_region);
#endif

          g_free (ranges);
        }
    }
}

#if !GTK_CHECK_VERSION(2, 3, 5)
typedef struct _CursorInfo CursorInfo;

struct _CursorInfo
{
  GType for_type;
  GdkGC *primary_gc;
  GdkGC *secondary_gc;
};

static GdkGC *
make_cursor_gc (GtkWidget      *widget,
                const gchar    *property_name,
                const GdkColor *fallback)
{
  GdkGCValues gc_values;
  GdkGCValuesMask gc_values_mask;
  GdkColor *cursor_color;

  gtk_widget_style_get (widget, property_name, &cursor_color, NULL);

  gc_values_mask = GDK_GC_FOREGROUND;
  if (cursor_color)
    {
      gc_values.foreground = *cursor_color;
      gdk_color_free (cursor_color);
    }
  else
    gc_values.foreground = *fallback;

  gdk_rgb_find_color (widget->style->colormap, &gc_values.foreground);
  return gtk_gc_get (widget->style->depth, widget->style->colormap, &gc_values, gc_values_mask);
}

static GdkGC *
get_insertion_cursor_gc (GtkWidget *widget,
                         gboolean   is_primary)
{
  CursorInfo *cursor_info;

  cursor_info = g_object_get_data (G_OBJECT (widget->style), "gtk-style-cursor-info");
  if (!cursor_info)
    {
      cursor_info = g_new (CursorInfo, 1);
      g_object_set_data (G_OBJECT (widget->style), "gtk-style-cursor-info", cursor_info);
      cursor_info->primary_gc = NULL;
      cursor_info->secondary_gc = NULL;
      cursor_info->for_type = G_TYPE_INVALID;
    }

  /* We have to keep track of the type because gtk_widget_style_get()
   * can return different results when called on the same property and
   * same style but for different widgets. :-(. That is,
   * GtkEntry::cursor-color = "red" in a style will modify the cursor
   * color for entries but not for text view.
   */
  if (cursor_info->for_type != G_OBJECT_TYPE (widget))
    {
      cursor_info->for_type = G_OBJECT_TYPE (widget);
      if (cursor_info->primary_gc)
        {
          gtk_gc_release (cursor_info->primary_gc);
          cursor_info->primary_gc = NULL;
        }
      if (cursor_info->secondary_gc)
        {
          gtk_gc_release (cursor_info->secondary_gc);
          cursor_info->secondary_gc = NULL;
        }
    }

  if (is_primary)
    {
      if (!cursor_info->primary_gc)
        cursor_info->primary_gc = make_cursor_gc (widget,
                                                  "cursor-color",
                                                  &widget->style->black);

      return cursor_info->primary_gc;
    }
  else
    {
      static const GdkColor gray = { 0, 0x8888, 0x8888, 0x8888 };

      if (!cursor_info->secondary_gc)
        cursor_info->secondary_gc = make_cursor_gc (widget,
                                                    "secondary-cursor-color",
                                                    &gray);

      return cursor_info->secondary_gc;
    }
}

static void
draw_insertion_cursor (GtkWidget        *widget,
                       GdkDrawable      *drawable,
                       GdkGC            *gc,
                       GdkRectangle     *location,
                       GtkTextDirection  direction,
                       gboolean          draw_arrow)
{
  gint stem_width;
  gint arrow_width;
  gint x, y;
  gint i;
  gfloat cursor_aspect_ratio;
  gint offset;

  g_return_if_fail (direction != GTK_TEXT_DIR_NONE);

  gtk_widget_style_get (widget, "cursor-aspect-ratio", &cursor_aspect_ratio, NULL);

  stem_width = location->height * cursor_aspect_ratio + 1;
  arrow_width = stem_width + 1;

  /* put (stem_width % 2) on the proper side of the cursor */
  if (direction == GTK_TEXT_DIR_LTR)
    offset = stem_width / 2;
  else
    offset = stem_width - stem_width / 2;

  for (i = 0; i < stem_width; i++)
    gdk_draw_line (drawable, gc,
                   location->x + i - offset, location->y,
                   location->x + i - offset, location->y + location->height - 1);

  if (draw_arrow)
    {
      if (direction == GTK_TEXT_DIR_RTL)
        {
          x = location->x - offset - 1;
          y = location->y + location->height - arrow_width * 2 - arrow_width + 1;

          for (i = 0; i < arrow_width; i++)
            {
              gdk_draw_line (drawable, gc,
                             x, y + i + 1,
                             x, y + 2 * arrow_width - i - 1);
              x --;
            }
        }
      else if (direction == GTK_TEXT_DIR_LTR)
        {
          x = location->x + stem_width - offset;
          y = location->y + location->height - arrow_width * 2 - arrow_width + 1;

          for (i = 0; i < arrow_width; i++)
            {
              gdk_draw_line (drawable, gc,
                             x, y + i + 1,
                             x, y + 2 * arrow_width - i - 1);
              x++;
            }
        }
    }
}

static void
scim_string_view_draw_insertion_cursor (GtkWidget        *widget,
                                        GdkDrawable      *drawable,
                                        PangoLayout      *layout,
                                        GdkRectangle     *area,
                                        GdkRectangle     *location,
                                        gboolean          is_primary,
                                        GtkTextDirection  direction,
                                        gboolean          draw_arrow,
                                        gint              index)
{
  GdkGC *gc;

  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (GDK_IS_DRAWABLE (drawable));
  g_return_if_fail (location != NULL);
  g_return_if_fail (direction != GTK_TEXT_DIR_NONE);

  gc = get_insertion_cursor_gc (widget, is_primary);
  if (area)
    gdk_gc_set_clip_rectangle (gc, area);

  draw_insertion_cursor (widget, drawable, gc,
                         location, direction, draw_arrow);

  if (area)
    gdk_gc_set_clip_rectangle (gc, NULL);
}
#else
static void
scim_string_view_draw_insertion_cursor (GtkWidget        *widget,
#if GTK_CHECK_VERSION(3, 0, 0)
                                        GdkWindow        *drawable,
#else
                                        GdkDrawable      *drawable,
#endif
                                        PangoLayout      *layout,
                                        GdkRectangle     *area,
                                        GdkRectangle     *location,
                                        gboolean          is_primary,
                                        GtkTextDirection  direction,
                                        gboolean          draw_arrow,
                                        gint              index)
{
#if GTK_CHECK_VERSION(3, 0, 0)
    cairo_t *cr = gdk_cairo_create (drawable);
/*
#if GTK_CHECK_VERSION(3, 4, 0)
    GtkStyleContext *context = gtk_widget_get_style_context (widget);
    gtk_render_insertion_cursor (
        context, cr,
        (gdouble)location->x, (gdouble)location->y,
        layout,
        index,
        direction == GTK_TEXT_DIR_LTR ? PANGO_DIRECTION_LTR : PANGO_DIRECTION_RTL
    );
#else
*/
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gtk_draw_insertion_cursor (widget, cr, location, is_primary, direction, draw_arrow);
    G_GNUC_END_IGNORE_DEPRECATIONS
/*
#endif
*/
    cairo_destroy (cr);
#else
    gtk_draw_insertion_cursor (widget, drawable, area, location, is_primary, direction, draw_arrow);
#endif
}
#endif

static void
scim_string_view_draw_cursor (ScimStringView  *string_view)
{
  GtkTextDirection keymap_direction =
    (gdk_keymap_get_direction (gdk_keymap_get_default ()) == PANGO_DIRECTION_LTR) ?
    GTK_TEXT_DIR_LTR : GTK_TEXT_DIR_RTL;
  GtkTextDirection widget_direction = gtk_widget_get_direction (GTK_WIDGET (string_view));

#if GTK_CHECK_VERSION(2, 18, 0)
  if (gtk_widget_is_drawable (GTK_WIDGET (string_view)))
#else
  if (GTK_WIDGET_DRAWABLE (string_view))
#endif
    {
      GtkWidget *widget = GTK_WIDGET (string_view);
      GdkRectangle cursor_location;
      GdkRectangle clip_area;
      gboolean split_cursor;

      gint xoffset = INNER_BORDER - string_view->scroll_offset;
      gint strong_x, weak_x;
      gint text_area_height;
      gint text_area_width;
      GtkTextDirection dir1 = GTK_TEXT_DIR_NONE;
      GtkTextDirection dir2 = GTK_TEXT_DIR_NONE;
      gint x1 = 0;
      gint x2 = 0;

#if GTK_CHECK_VERSION(2, 24, 0)
      text_area_width = gdk_window_get_width (string_view->text_area);
      text_area_height = gdk_window_get_height (string_view->text_area);
#else
      gdk_window_get_size (string_view->text_area, &text_area_width, &text_area_height);
#endif

      gint index = scim_string_view_get_cursor_locations (string_view, &strong_x, &weak_x);

      g_object_get (gtk_widget_get_settings (widget),
                    "gtk-split-cursor", &split_cursor,
                    NULL);

      dir1 = widget_direction;

      if (split_cursor)
        {
          x1 = strong_x;

          if (weak_x != strong_x)
            {
              dir2 = (widget_direction == GTK_TEXT_DIR_LTR) ? GTK_TEXT_DIR_RTL : GTK_TEXT_DIR_LTR;
              x2 = weak_x;
            }
        }
      else
        {
          if (keymap_direction == widget_direction)
            x1 = strong_x;
          else
            x1 = weak_x;
        }

      cursor_location.x = xoffset + x1;
      cursor_location.y = INNER_BORDER;
      cursor_location.width = 0;
      cursor_location.height = text_area_height - 2 * INNER_BORDER ;

      clip_area.x = 0;
      clip_area.y = 0;
      clip_area.width = text_area_width;
      clip_area.height = text_area_height;

      scim_string_view_draw_insertion_cursor (widget,
                                              string_view->text_area,
                                              scim_string_view_ensure_layout (string_view),
                                              &clip_area,
                                              &cursor_location,
                                              TRUE,
                                              dir1,
                                              dir2 != GTK_TEXT_DIR_NONE,
                                              index);

      if (dir2 != GTK_TEXT_DIR_NONE)
        {
          cursor_location.x = xoffset + x2;
          scim_string_view_draw_insertion_cursor (widget,
                                                  string_view->text_area,
                                                  scim_string_view_ensure_layout (string_view),
                                                  &clip_area,
                                                  &cursor_location,
                                                  FALSE,
                                                  dir2,
                                                  TRUE,
                                                  index);
        }
    }
}

static void
scim_string_view_queue_draw (ScimStringView *string_view)
{
#if GTK_CHECK_VERSION(2, 20, 0)
  if (gtk_widget_get_realized (GTK_WIDGET (string_view)))
#else
  if (GTK_WIDGET_REALIZED (string_view))
#endif
    gdk_window_invalidate_rect (string_view->text_area, NULL, FALSE);
}

static gint
scim_string_view_find_position (ScimStringView *string_view, gint x)
{
  PangoLayout *layout;
  PangoLayoutLine *line;
  gint index;
  gint pos;
  gboolean trailing;

  layout = scim_string_view_ensure_layout (string_view);

  line = pango_layout_get_lines (layout)->data;
  pango_layout_line_x_to_index (line, x * PANGO_SCALE, &index, &trailing);

  pos = g_utf8_pointer_to_offset (string_view->text, string_view->text + index);
  pos += trailing;

  return pos;
}

static gint
scim_string_view_get_cursor_locations (ScimStringView   *string_view,
                                gint       *strong_x,
                                gint       *weak_x)
{
  PangoLayout *layout = scim_string_view_ensure_layout (string_view);
  const gchar *text;
  PangoRectangle strong_pos, weak_pos;
  gint index;

  text = pango_layout_get_text (layout);
  index = g_utf8_offset_to_pointer (text, string_view->current_pos) - text;

  pango_layout_get_cursor_pos (layout, index, &strong_pos, &weak_pos);

  if (strong_x)
    *strong_x = strong_pos.x / PANGO_SCALE;

  if (weak_x)
    *weak_x = weak_pos.x / PANGO_SCALE;

  return index;
}

static void
scim_string_view_adjust_scroll (ScimStringView *string_view)
{
  gint min_offset, max_offset;
  gint text_area_width;
  gint strong_x, weak_x;
  gint strong_xoffset, weak_xoffset;
  PangoLayout *layout;
  PangoLayoutLine *line;
  PangoRectangle logical_rect;

#if GTK_CHECK_VERSION(2, 20, 0)
  if (!gtk_widget_get_realized (GTK_WIDGET (string_view)))
#else
  if (!GTK_WIDGET_REALIZED (string_view))
#endif
    return;

#if GTK_CHECK_VERSION(2, 24, 0)
  text_area_width = gdk_window_get_width(string_view->text_area);
#else
  gdk_window_get_size (string_view->text_area, &text_area_width, NULL);
#endif
  text_area_width -= 2 * INNER_BORDER;

  layout = scim_string_view_ensure_layout (string_view);
  line = pango_layout_get_lines (layout)->data;

  pango_layout_line_get_extents (line, NULL, &logical_rect);

  /* Display as much text as we can */

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

  /* And make sure cursors are on screen. Note that the cursor is
   * actually drawn one pixel into the INNER_BORDER space on
   * the right, when the scroll is at the utmost right. This
   * looks better to to me than confining the cursor inside the
   * border entirely, though it means that the cursor gets one
   * pixel closer to the the edge of the widget on the right than
   * on the left. This might need changing if one changed
   * INNER_BORDER from 2 to 1, as one would do on a
   * small-screen-real-estate display.
   *
   * We always make sure that the strong cursor is on screen, and
   * put the weak cursor on screen if possible.
   */

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
    {
      string_view->scroll_offset += weak_xoffset;
    }
  else if (weak_xoffset > text_area_width &&
           strong_xoffset - (weak_xoffset - text_area_width) >= 0)
    {
      string_view->scroll_offset += weak_xoffset - text_area_width;
    }

  g_object_notify (G_OBJECT (string_view), "scroll_offset");
}

static void
scim_string_view_move_cursor (ScimStringView *string_view,
                             guint          position)
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
scim_string_view_set_position (ScimStringView *string_view,
                              gint       position)
{
  gboolean changed = FALSE;

  g_return_if_fail (SCIM_IS_STRING_VIEW (string_view));

  g_object_freeze_notify (G_OBJECT (string_view));

  if (position != -1 &&
      string_view->current_pos != position)
    {
      string_view->current_pos =
         (position > string_view->text_length)?(string_view->text_length):position;

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
scim_string_view_set_max_width (ScimStringView *string_view,
                              gint       width)
{
  g_return_if_fail (SCIM_IS_STRING_VIEW (string_view));

  if (string_view->max_width == width)
    return;

  if (width > 0 && width < MIN_STRING_VIEW_WIDTH) width = MIN_STRING_VIEW_WIDTH;

  string_view->max_width = width;

  g_object_notify (G_OBJECT (string_view), "cursor_position");
  scim_string_view_recompute (string_view);
}

gint
scim_string_view_get_max_width (ScimStringView *string_view)
{
  g_return_val_if_fail (SCIM_IS_STRING_VIEW (string_view), FALSE);

  return string_view->max_width;
}

void
scim_string_view_set_draw_cursor (ScimStringView *string_view,
                                  gboolean       setting)
{
  g_return_if_fail (SCIM_IS_STRING_VIEW (string_view));

  setting = (setting != FALSE);

  if (string_view->draw_cursor == setting)
    return;

  string_view->draw_cursor = setting;

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
scim_string_view_set_auto_move_cursor (ScimStringView *string_view,
                                      gboolean       setting)
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
scim_string_view_set_forward_event (ScimStringView *string_view,
                                      gboolean       setting)
{
  g_return_if_fail (SCIM_IS_STRING_VIEW (string_view));

  setting = (setting != FALSE);

  if (string_view->forward_event == setting)
    return;

  string_view->forward_event = setting;

  g_object_notify (G_OBJECT (string_view), "forward_event");
}

void
scim_string_view_set_auto_resize (ScimStringView *string_view,
                                      gboolean       setting)
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

gboolean
scim_string_view_get_forward_event (ScimStringView *string_view)
{
  g_return_val_if_fail (SCIM_IS_STRING_VIEW (string_view), FALSE);

  return string_view->forward_event;
}


const gchar*
scim_string_view_get_text (ScimStringView *string_view)
{
  g_return_val_if_fail (SCIM_IS_STRING_VIEW (string_view), NULL);

  return string_view->text;
}

void
scim_string_view_set_text (ScimStringView    *string_view,
                    const gchar *text)
{
  gint new_text_length;
  gint new_nbytes;

  g_return_if_fail (SCIM_IS_STRING_VIEW (string_view));
  g_return_if_fail (text != NULL);

  /* Actually setting the text will affect the cursor and selection;
   * if the contents don't actually change, this will look odd to the user.
   */
  if (strcmp (string_view->text, text) == 0)
    return;

  new_nbytes = strlen (text);
  new_text_length= g_utf8_strlen (text, new_nbytes);

  if (string_view->text_max_length > 0 && new_text_length > string_view->text_max_length)
    {
      gdk_beep ();
      new_text_length = string_view->text_max_length;
      new_nbytes= g_utf8_offset_to_pointer (text, new_text_length) - text;
    }

  if (new_nbytes >= string_view->text_size)
    {
      string_view->text = g_realloc (string_view->text, new_nbytes + 1);
      string_view->text_size = new_nbytes + 1;
    }

  memcpy (string_view->text, text, new_nbytes);

  string_view->n_bytes = new_nbytes;
  string_view->text_length = new_text_length;

  /* NUL terminate for safety and convenience */
  string_view->text[string_view->n_bytes] = '\0';

  if (string_view->current_pos > string_view->text_length)
    string_view->current_pos = string_view->text_length;

  if (string_view->auto_resize)
      gtk_widget_queue_resize (GTK_WIDGET (string_view));

  scim_string_view_recompute (string_view);

  g_object_notify (G_OBJECT (string_view), "text");
}

/**
 * scim_string_view_set_max_length:
 * @string_view: a #ScimStringView.
 * @max: the maximum length of the string_view, or 0 for no maximum.
 *   (other than the maximum length of entries.) The value passed in will
 *   be clamped to the range 0-65536.
 *
 * Sets the maximum allowed length of the contents of the widget. If
 * the current contents are longer than the given length, then they
 * will be truncated to fit.
 **/
void
scim_string_view_set_max_length (ScimStringView     *string_view,
                          gint          max)
{
  gint nbytes;

  g_return_if_fail (SCIM_IS_STRING_VIEW (string_view));

  max = CLAMP (max, 0, MAX_SIZE);

  if (max > 0 && string_view->text_length > max) {
    string_view->text_length = max;
    nbytes = g_utf8_offset_to_pointer (string_view->text, max) - string_view->text;

    string_view->text_size = nbytes + 1;
    string_view->text = g_realloc (string_view->text, string_view->text_size);
    string_view->text [nbytes] = '\0';
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

/**
 * scim_string_view_get_max_length:
 * @string_view: a #ScimStringView
 *
 * Retrieves the maximum allowed length of the text in
 * @string_view. See scim_string_view_set_max_length().
 *
 * Return value: the maximum allowed number of characters
 *               in #ScimStringView, or 0 if there is no maximum.
 **/
gint
scim_string_view_get_max_length (ScimStringView *string_view)
{
  g_return_val_if_fail (SCIM_IS_STRING_VIEW (string_view), 0);

  return string_view->text_max_length;
}


/**
 * scim_string_view_set_width_chars:
 * @string_view: a #ScimStringView
 * @n_chars: width in chars
 *
 * Changes the size request of the string_view to be about the right size
 * for @n_chars characters. Note that it changes the size
 * <emphasis>request</emphasis>, the size can still be affected by
 * how you pack the widget into containers. If @n_chars is -1, the
 * size reverts to the default string_view size.
 *
 **/
void
scim_string_view_set_width_chars (ScimStringView *string_view,
                           gint      n_chars)
{
  g_return_if_fail (SCIM_IS_STRING_VIEW (string_view));

  if (string_view->width_chars != n_chars)
    {
      string_view->width_chars = n_chars;
      g_object_notify (G_OBJECT (string_view), "width_chars");
      gtk_widget_queue_resize (GTK_WIDGET (string_view));
    }
}

/**
 * scim_string_view_get_width_chars:
 * @string_view: a #ScimStringView
 *
 * Gets the value set by scim_string_view_set_width_chars().
 *
 * Return value: number of chars to request space for, or negative if unset
 **/
gint
scim_string_view_get_width_chars (ScimStringView *string_view)
{
  g_return_val_if_fail (SCIM_IS_STRING_VIEW (string_view), 0);

  return string_view->width_chars;
}

/**
 * scim_string_view_set_has_frame:
 * @string_view: a #ScimStringView
 * @setting: new value
 *
 * Sets whether the string_view has a beveled frame around it.
 **/
void
scim_string_view_set_has_frame (ScimStringView *string_view,
                         gboolean  setting)
{
  g_return_if_fail (SCIM_IS_STRING_VIEW (string_view));

  setting = (setting != FALSE);

  if (string_view->has_frame == setting)
    return;

  gtk_widget_queue_resize (GTK_WIDGET (string_view));
  string_view->has_frame = setting;
  g_object_notify (G_OBJECT (string_view), "has_frame");
}

/**
 * scim_string_view_get_has_frame:
 * @string_view: a #ScimStringView
 *
 * Gets the value set by scim_string_view_set_has_frame().
 *
 * Return value: whether the string_view has a beveled frame
 **/
gboolean
scim_string_view_get_has_frame (ScimStringView *string_view)
{
  g_return_val_if_fail (SCIM_IS_STRING_VIEW (string_view), FALSE);

  return string_view->has_frame;
}


/**
 * scim_string_view_get_layout:
 * @string_view: a #ScimStringView
 *
 * Gets the #PangoLayout used to display the string_view.
 * The layout is useful to e.g. convert text positions to
 * pixel positions, in combination with scim_string_view_get_layout_offsets().
 * The returned layout is owned by the string_view so need not be
 * freed by the caller.
 *
 * Keep in mind that the layout text may contain a preedit string, so
 * scim_string_view_layout_index_to_text_index() and
 * scim_string_view_text_index_to_layout_index() are needed to convert byte
 * indices in the layout to byte indices in the string_view contents.
 *
 * Return value: the #PangoLayout for this string_view
 **/
PangoLayout*
scim_string_view_get_layout (ScimStringView *string_view)
{
  PangoLayout *layout;

  g_return_val_if_fail (SCIM_IS_STRING_VIEW (string_view), NULL);

  layout = scim_string_view_ensure_layout (string_view);

  return layout;
}


/**
 * scim_string_view_get_layout_offsets:
 * @string_view: a #ScimStringView
 * @x: location to store X offset of layout, or %NULL
 * @y: location to store Y offset of layout, or %NULL
 *
 *
 * Obtains the position of the #PangoLayout used to render text
 * in the string_view, in widget coordinates. Useful if you want to line
 * up the text in an string_view with some other text, e.g. when using the
 * string_view to implement editable cells in a sheet widget.
 *
 * Also useful to convert mouse events into coordinates inside the
 * #PangoLayout, e.g. to take some action if some part of the string_view text
 * is clicked.
 *
 * Note that as the user scrolls around in the string_view the offsets will
 * change; you'll need to connect to the "notify::scroll_offset"
 * signal to track this. Remember when using the #PangoLayout
 * functions you need to convert to and from pixels using
 * PANGO_PIXELS() or #PANGO_SCALE.
 *
 * Keep in mind that the layout text may contain a preedit string, so
 * scim_string_view_layout_index_to_text_index() and
 * scim_string_view_text_index_to_layout_index() are needed to convert byte
 * indices in the layout to byte indices in the string_view contents.
 *
 **/
void
scim_string_view_get_layout_offsets (ScimStringView *string_view,
                              gint     *x,
                              gint     *y)
{
  gint text_area_x, text_area_y;

  g_return_if_fail (SCIM_IS_STRING_VIEW (string_view));

  /* this gets coords relative to text area */
  get_layout_position (string_view, x, y);

  /* convert to widget coords */
  get_text_area_size (string_view, &text_area_x, &text_area_y, NULL, NULL);

  if (x)
    *x += text_area_x;

  if (y)
    *y += text_area_y;
}

void
scim_string_view_set_highlight (ScimStringView      *string_view,
                               gint                start,
                               gint                end)
{
  g_return_if_fail (SCIM_IS_STRING_VIEW (string_view));

  string_view->highlight_start = start;
  string_view->highlight_end = end;

  gtk_widget_queue_draw (GTK_WIDGET (string_view));
}

void
scim_string_view_get_highlight (ScimStringView      *string_view,
                               gint               *start,
                               gint               *end)
{
  g_return_if_fail (SCIM_IS_STRING_VIEW (string_view));

  if (start) *start = string_view->highlight_start;
  if (end)   *end   = string_view->highlight_end;
}

void
scim_string_view_set_attributes  (ScimStringView      *entry,
                                 PangoAttrList      *attrs)
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
scim_string_view_get_attributes  (ScimStringView      *entry)
{
  g_return_val_if_fail (SCIM_IS_STRING_VIEW (entry), NULL);

  return entry->attrs;
}

#define CURSOR_ON_MULTIPLIER 0.66
#define CURSOR_OFF_MULTIPLIER 0.34
#define CURSOR_PEND_MULTIPLIER 1.0

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

static void
show_cursor (ScimStringView *string_view)
{
  if (!string_view->cursor_visible)
    {
      string_view->cursor_visible = TRUE;
      gtk_widget_queue_draw (GTK_WIDGET (string_view));
    }
}

static void
hide_cursor (ScimStringView *string_view)
{
  if (string_view->cursor_visible)
    {
      string_view->cursor_visible = FALSE;
      gtk_widget_queue_draw (GTK_WIDGET (string_view));
    }
}

/*
 * Blink!
 */
static gint
blink_cb (gpointer data)
{
  ScimStringView *string_view;

#if !GTK_CHECK_VERSION(2, 12, 0)
  GDK_THREADS_ENTER ();
#endif

  string_view = SCIM_STRING_VIEW (data);

  if (string_view->cursor_visible)
    {
      hide_cursor (string_view);
      string_view->blink_timeout = g_timeout_add (get_cursor_time (string_view) * CURSOR_OFF_MULTIPLIER,
                                                    blink_cb,
                                                    string_view);
    }
  else
    {
      show_cursor (string_view);
      string_view->blink_timeout = g_timeout_add (get_cursor_time (string_view) * CURSOR_ON_MULTIPLIER,
                                                    blink_cb,
                                                    string_view);
    }

#if !GTK_CHECK_VERSION(2, 12, 0)
  GDK_THREADS_LEAVE ();
#endif

  /* Remove ourselves */
  return FALSE;
}

static void
scim_string_view_check_cursor_blink (ScimStringView *string_view)
{
  if (cursor_blinks (string_view))
    {
      if (!string_view->blink_timeout)
        {
          string_view->blink_timeout = g_timeout_add (get_cursor_time (string_view) * CURSOR_ON_MULTIPLIER,
                                                  blink_cb,
                                                  string_view);
          show_cursor (string_view);
        }
    }
  else
    {
      if (string_view->blink_timeout)
        {
          g_source_remove (string_view->blink_timeout);
          string_view->blink_timeout = 0;
        }

      string_view->cursor_visible = TRUE;
    }
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
