/*
 * Smart Common Input Method
 *
 * Copyright (c) 2026 SCIM developers
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
 */

#define Uses_SCIM_LOOKUP_TABLE
#define Uses_SCIM_ATTRIBUTE
#define Uses_SCIM_UTILITY
#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_PATH
#include "scim_private.h"
#include <scim.h>

#include "scim_candidates.h"

#include <vector>
#include <pango/pangocairo.h>

namespace scim {

/* ------------------------------------------------------------------ */
/* Theme presets                                                       */
/* ------------------------------------------------------------------ */

CandidatesTheme
CandidatesTheme::light ()
{
    CandidatesTheme t;
    t.font         = "Sans 12";
    t.bg           = { 0.98, 0.98, 0.98, 1.0 };
    t.fg           = { 0.10, 0.10, 0.10, 1.0 };
    t.label        = { 0.45, 0.45, 0.45, 1.0 };
    t.highlight_bg = { 0.20, 0.50, 0.90, 1.0 };
    t.highlight_fg = { 1.00, 1.00, 1.00, 1.0 };
    t.border       = { 0.60, 0.60, 0.60, 1.0 };
    t.border_width = 1;
    t.padding      = 6;
    t.spacing      = 6;
    return t;
}

CandidatesTheme
CandidatesTheme::dark ()
{
    CandidatesTheme t;
    t.font         = "Sans 12";
    t.bg           = { 0.16, 0.16, 0.16, 1.0 };
    t.fg           = { 0.92, 0.92, 0.92, 1.0 };
    t.label        = { 0.60, 0.60, 0.60, 1.0 };
    t.highlight_bg = { 0.24, 0.52, 0.90, 1.0 };
    t.highlight_fg = { 1.00, 1.00, 1.00, 1.0 };
    t.border       = { 0.40, 0.40, 0.40, 1.0 };
    t.border_width = 1;
    t.padding      = 6;
    t.spacing      = 6;
    return t;
}

// Parse a CSS/X11 color string ("gray92", "light blue", "#rrggbb") into an
// RGBA color, using Pango (no GDK dependency). Alpha defaults to 1.
static CandidatesColor
parse_color (const String &s, const CandidatesColor &fallback)
{
    PangoColor pc;
    if (s.length () && pango_color_parse (&pc, s.c_str ())) {
        CandidatesColor c = { pc.red / 65535.0, pc.green / 65535.0, pc.blue / 65535.0, 1.0 };
        return c;
    }
    return fallback;
}

// Read an appearance key with a layered fallback:
//   /Candidates/<renderer>/<key>  ->  /Candidates/Default/<key>  ->
//   /Panel/Gtk/<key> (legacy, shared with the panel)  ->  "" (unset)
// so a per-renderer override wins, otherwise the shared Candidates/Default
// value, otherwise the historical panel value (backward compatible). When
// skip_default is set, the sentinel "default" is treated as unset at each
// level, so writing "default" in setup does not shadow a lower level.
static String
read_layered_key (const ConfigPointer &config, const String &renderer,
                  const String &key, bool skip_default = false)
{
    const String levels[] = {
        String ("/Candidates/") + renderer + "/" + key,
        renderer == String ("Default") ? String () : String ("/Candidates/Default/") + key,
        String ("/Panel/Gtk/") + key,
    };
    for (const String &path : levels) {
        if (path.empty ())
            continue;
        String v = config->read (path, String ());
        if (v.length () && (!skip_default || v != String ("default")))
            return v;
    }
    return String ();
}

CandidatesTheme
scim_candidates_theme_from_config (const ConfigPointer &config, const String &renderer)
{
    CandidatesTheme t = CandidatesTheme::light ();
    if (config.null ())
        return t;

    String font = read_layered_key (config, renderer, String ("Font"), true);
    if (font.length ())
        t.font = font;

    // parse_color () keeps the light() field when its string is empty/unparsable,
    // so pass the historical default string when every level is unset.
    auto color = [&] (const char *key, const char *deflt) -> String {
        String v = read_layered_key (config, renderer, String (key));
        return v.length () ? v : String (deflt);
    };
    t.bg           = parse_color (color ("Color/NormalBackground", "gray92"),     t.bg);
    t.fg           = parse_color (color ("Color/NormalText",       "black"),      t.fg);
    t.highlight_bg = parse_color (color ("Color/ActiveBackground", "light blue"), t.highlight_bg);
    t.highlight_fg = parse_color (color ("Color/ActiveText",       "black"),      t.highlight_fg);
    // label/border have no config keys yet; keep the light() defaults.
    return t;
}

/* ------------------------------------------------------------------ */
/* Implementation                                                      */
/* ------------------------------------------------------------------ */

namespace {

// A laid-out run of text ready to be painted.
struct TextItem {
    String         utf8;         // the text
    PangoAttrList *attrs;        // owned; may be null
    int            x, y;         // top-left, panel-relative px
    int            w, h;         // pixel extents

    TextItem () : attrs (0), x (0), y (0), w (0), h (0) { }
};

// A candidate cell + the geometry needed to hit-test / highlight it.
struct CandidateCell {
    TextItem item;
    int      index;              // index within the current page
    bool     highlighted;

    CandidateCell () : index (0), highlighted (false) { }
};

// Map a character index into a WideString to a byte offset in its utf8 form.
// offsets[i] is the byte offset of the i-th character; offsets has
// wstr.length()+1 entries (the last is the total byte length).
void
build_byte_offsets (const WideString &wstr, std::vector<int> &offsets)
{
    offsets.clear ();
    offsets.reserve (wstr.length () + 1);
    int bytes = 0;
    offsets.push_back (0);
    for (size_t i = 0; i < wstr.length (); ++i) {
        String one = utf8_wcstombs (wstr.substr (i, 1));
        bytes += static_cast<int> (one.length ());
        offsets.push_back (bytes);
    }
}

// Translate a SCIM AttributeList over a WideString into a PangoAttrList over
// its utf8 rendering. Returns null when there is nothing to apply.
PangoAttrList *
make_pango_attrs (const WideString &wstr, const AttributeList &attrs,
                  const CandidatesColor &fg)
{
    if (attrs.empty ())
        return 0;

    std::vector<int> off;
    build_byte_offsets (wstr, off);
    const int nchars = static_cast<int> (wstr.length ());

    PangoAttrList *plist = pango_attr_list_new ();

    for (size_t i = 0; i < attrs.size (); ++i) {
        const Attribute &a = attrs[i];
        int start = static_cast<int> (a.get_start ());
        int end   = start + static_cast<int> (a.get_length ());
        if (start < 0) start = 0;
        if (end > nchars) end = nchars;
        if (end <= start) continue;

        guint bstart = static_cast<guint> (off[start]);
        guint bend   = static_cast<guint> (off[end]);

        PangoAttribute *pa = 0;
        switch (a.get_type ()) {
        case SCIM_ATTR_DECORATE:
            switch (a.get_value ()) {
            case SCIM_ATTR_DECORATE_UNDERLINE:
                pa = pango_attr_underline_new (PANGO_UNDERLINE_SINGLE);
                break;
            case SCIM_ATTR_DECORATE_HIGHLIGHT:
            case SCIM_ATTR_DECORATE_REVERSE:
                // Reverse/highlight: paint text in the theme bg over an fg
                // background so it reads as a selection swatch.
                pa = pango_attr_background_new (
                        (guint16)(fg.r * 65535), (guint16)(fg.g * 65535),
                        (guint16)(fg.b * 65535));
                pa->start_index = bstart;
                pa->end_index   = bend;
                pango_attr_list_insert (plist, pa);
                pa = pango_attr_foreground_new (0xffff, 0xffff, 0xffff);
                break;
            default:
                break;
            }
            break;
        case SCIM_ATTR_FOREGROUND: {
            unsigned int c = a.get_value ();
            pa = pango_attr_foreground_new (
                    (guint16)(SCIM_RGB_COLOR_RED (c)   * 257),
                    (guint16)(SCIM_RGB_COLOR_GREEN (c) * 257),
                    (guint16)(SCIM_RGB_COLOR_BLUE (c)  * 257));
            break;
        }
        case SCIM_ATTR_BACKGROUND: {
            unsigned int c = a.get_value ();
            pa = pango_attr_background_new (
                    (guint16)(SCIM_RGB_COLOR_RED (c)   * 257),
                    (guint16)(SCIM_RGB_COLOR_GREEN (c) * 257),
                    (guint16)(SCIM_RGB_COLOR_BLUE (c)  * 257));
            break;
        }
        default:
            break;
        }

        if (pa) {
            pa->start_index = bstart;
            pa->end_index   = bend;
            pango_attr_list_insert (plist, pa);
        }
    }

    return plist;
}

} // anonymous namespace

class CandidatesUI::CandidatesUIImpl
{
public:
    CandidatesTheme  m_theme;

    // Raw state.
    bool          m_preedit_visible;
    WideString    m_preedit_str;
    AttributeList m_preedit_attrs;
    int           m_preedit_caret;

    bool          m_aux_visible;
    WideString    m_aux_str;
    AttributeList m_aux_attrs;

    bool          m_lookup_visible;
    bool          m_lookup_vertical;
    std::vector<WideString> m_cand_text;
    std::vector<WideString> m_cand_label;
    int           m_cursor_in_page;      // -1 if none / hidden
    bool          m_has_prev_page;
    bool          m_has_next_page;

    // Laid-out geometry (rebuilt lazily).
    bool          m_dirty;
    int           m_width;
    int           m_height;
    TextItem      m_preedit_item;
    bool          m_has_preedit_item;
    int           m_caret_x, m_caret_y, m_caret_h;
    bool          m_has_caret;
    TextItem      m_aux_item;
    bool          m_has_aux_item;
    std::vector<CandidateCell> m_cells;

    // A scratch cairo context for measuring without a live surface.
    cairo_surface_t *m_scratch_surface;
    cairo_t         *m_scratch_cr;

    CandidatesUIImpl ()
        : m_theme (CandidatesTheme::light ()),
          m_preedit_visible (false), m_preedit_caret (0),
          m_aux_visible (false),
          m_lookup_visible (false), m_lookup_vertical (false),
          m_cursor_in_page (-1), m_has_prev_page (false), m_has_next_page (false),
          m_dirty (true), m_width (0), m_height (0),
          m_has_preedit_item (false),
          m_caret_x (0), m_caret_y (0), m_caret_h (0), m_has_caret (false),
          m_has_aux_item (false),
          m_scratch_surface (0), m_scratch_cr (0)
    {
    }

    ~CandidatesUIImpl ()
    {
        clear_layout ();
        if (m_scratch_cr) cairo_destroy (m_scratch_cr);
        if (m_scratch_surface) cairo_surface_destroy (m_scratch_surface);
    }

    cairo_t * scratch ()
    {
        if (!m_scratch_cr) {
            m_scratch_surface =
                cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1, 1);
            m_scratch_cr = cairo_create (m_scratch_surface);
        }
        return m_scratch_cr;
    }

    void free_item_attrs (TextItem &it)
    {
        if (it.attrs) {
            pango_attr_list_unref (it.attrs);
            it.attrs = 0;
        }
    }

    void clear_layout ()
    {
        free_item_attrs (m_preedit_item);
        free_item_attrs (m_aux_item);
        for (size_t i = 0; i < m_cells.size (); ++i)
            free_item_attrs (m_cells[i].item);
        m_cells.clear ();
        m_has_preedit_item = false;
        m_has_aux_item = false;
        m_has_caret = false;
    }

    // Build a PangoLayout on cr for utf8 with attrs (font from theme).
    PangoLayout * make_layout (cairo_t *cr, const String &utf8,
                               PangoAttrList *attrs)
    {
        PangoLayout *layout = pango_cairo_create_layout (cr);
        PangoFontDescription *desc =
            pango_font_description_from_string (m_theme.font.c_str ());
        pango_layout_set_font_description (layout, desc);
        pango_font_description_free (desc);
        pango_layout_set_text (layout, utf8.c_str (),
                               static_cast<int> (utf8.length ()));
        if (attrs)
            pango_layout_set_attributes (layout, attrs);
        return layout;
    }

    void measure_item (cairo_t *cr, TextItem &it)
    {
        PangoLayout *layout = make_layout (cr, it.utf8, it.attrs);
        int w = 0, h = 0;
        pango_layout_get_pixel_size (layout, &w, &h);
        it.w = w;
        it.h = h;
        g_object_unref (layout);
    }

    // Rebuild geometry from raw state.
    void layout ()
    {
        if (!m_dirty)
            return;

        clear_layout ();

        cairo_t *cr = scratch ();
        const int pad     = m_theme.padding;
        const int spacing = m_theme.spacing;
        const int bw      = m_theme.border_width;

        int origin = bw + pad;
        int cur_y  = origin;
        int max_x  = origin;   // right edge of content

        // Preedit line (+ caret).
        if (m_preedit_visible && m_preedit_str.length ()) {
            m_preedit_item.utf8  = utf8_wcstombs (m_preedit_str);
            m_preedit_item.attrs =
                make_pango_attrs (m_preedit_str, m_preedit_attrs, m_theme.fg);
            m_preedit_item.x = origin;
            m_preedit_item.y = cur_y;
            measure_item (cr, m_preedit_item);
            m_has_preedit_item = true;

            // Caret x within the preedit layout.
            std::vector<int> off;
            build_byte_offsets (m_preedit_str, off);
            int caret = m_preedit_caret;
            if (caret < 0) caret = 0;
            if (caret > (int) m_preedit_str.length ())
                caret = (int) m_preedit_str.length ();
            PangoLayout *pl =
                make_layout (cr, m_preedit_item.utf8, m_preedit_item.attrs);
            PangoRectangle strong;
            pango_layout_get_cursor_pos (pl, off[caret], &strong, 0);
            m_caret_x = m_preedit_item.x + strong.x / PANGO_SCALE;
            m_caret_y = m_preedit_item.y;
            m_caret_h = m_preedit_item.h;
            m_has_caret = true;
            g_object_unref (pl);

            cur_y += m_preedit_item.h + spacing;
            if (m_preedit_item.x + m_preedit_item.w > max_x)
                max_x = m_preedit_item.x + m_preedit_item.w;
        }

        // Aux line.
        if (m_aux_visible && m_aux_str.length ()) {
            m_aux_item.utf8  = utf8_wcstombs (m_aux_str);
            m_aux_item.attrs = make_pango_attrs (m_aux_str, m_aux_attrs, m_theme.fg);
            m_aux_item.x = origin;
            m_aux_item.y = cur_y;
            measure_item (cr, m_aux_item);
            m_has_aux_item = true;

            cur_y += m_aux_item.h + spacing;
            if (m_aux_item.x + m_aux_item.w > max_x)
                max_x = m_aux_item.x + m_aux_item.w;
        }

        // Lookup table.
        if (m_lookup_visible && !m_cand_text.empty ()) {
            const int n = static_cast<int> (m_cand_text.size ());

            if (m_lookup_vertical) {
                for (int i = 0; i < n; ++i) {
                    CandidateCell cell;
                    cell.index = i;
                    cell.highlighted = (i == m_cursor_in_page);
                    cell.item.utf8 = cell_text (i);
                    cell.item.attrs = make_cell_attrs (i);
                    cell.item.x = origin;
                    cell.item.y = cur_y;
                    measure_item (cr, cell.item);
                    cur_y += cell.item.h + spacing;
                    if (cell.item.x + cell.item.w > max_x)
                        max_x = cell.item.x + cell.item.w;
                    m_cells.push_back (cell);
                }
            } else {
                int x = origin;
                int rowh = 0;
                for (int i = 0; i < n; ++i) {
                    CandidateCell cell;
                    cell.index = i;
                    cell.highlighted = (i == m_cursor_in_page);
                    cell.item.utf8 = cell_text (i);
                    cell.item.attrs = make_cell_attrs (i);
                    cell.item.x = x;
                    cell.item.y = cur_y;
                    measure_item (cr, cell.item);
                    x += cell.item.w + spacing * 2;
                    if (cell.item.h > rowh) rowh = cell.item.h;
                    m_cells.push_back (cell);
                }
                if (x - spacing * 2 > max_x)
                    max_x = x - spacing * 2;
                cur_y += rowh + spacing;
            }
        }

        m_width  = max_x + pad + bw;
        // Trailing spacing after the last block is folded into the padding.
        m_height = (cur_y - spacing) + pad + bw;
        if (m_height < origin + pad)
            m_height = origin + pad;

        m_dirty = false;
    }

    // "label. text" for candidate i.
    String cell_text (int i)
    {
        WideString w = m_cand_label[i];
        w += utf8_mbstowcs (". ");
        w += m_cand_text[i];
        return utf8_wcstombs (w);
    }

    // Color the label portion; on the highlighted cell paint fg white.
    PangoAttrList * make_cell_attrs (int i)
    {
        PangoAttrList *plist = pango_attr_list_new ();

        // Byte length of the "label. " prefix.
        WideString prefix = m_cand_label[i];
        prefix += utf8_mbstowcs (". ");
        String prefix_utf8 = utf8_wcstombs (prefix);
        guint plen = static_cast<guint> (prefix_utf8.length ());

        const bool hi = (i == m_cursor_in_page);
        const CandidatesColor &lab = hi ? m_theme.highlight_fg : m_theme.label;
        const CandidatesColor &txt = hi ? m_theme.highlight_fg : m_theme.fg;

        PangoAttribute *pa;
        pa = pango_attr_foreground_new ((guint16)(lab.r * 65535),
                                        (guint16)(lab.g * 65535),
                                        (guint16)(lab.b * 65535));
        pa->start_index = 0;
        pa->end_index   = plen;
        pango_attr_list_insert (plist, pa);

        pa = pango_attr_foreground_new ((guint16)(txt.r * 65535),
                                        (guint16)(txt.g * 65535),
                                        (guint16)(txt.b * 65535));
        pa->start_index = plen;
        pa->end_index   = G_MAXUINT;
        pango_attr_list_insert (plist, pa);

        return plist;
    }

    void paint_item (cairo_t *cr, const TextItem &it, const CandidatesColor &fg)
    {
        PangoLayout *layout = make_layout (cr, it.utf8, it.attrs);
        cairo_move_to (cr, it.x, it.y);
        cairo_set_source_rgba (cr, fg.r, fg.g, fg.b, fg.a);
        pango_cairo_show_layout (cr, layout);
        g_object_unref (layout);
    }
};

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

CandidatesUI::CandidatesUI ()
    : m_impl (new CandidatesUIImpl ())
{
}

CandidatesUI::~CandidatesUI ()
{
    delete m_impl;
}

void
CandidatesUI::set_theme (const CandidatesTheme &theme)
{
    m_impl->m_theme = theme;
    m_impl->m_dirty = true;
}

const CandidatesTheme &
CandidatesUI::get_theme () const
{
    return m_impl->m_theme;
}

void
CandidatesUI::set_font (const String &font_desc)
{
    m_impl->m_theme.font = font_desc;
    m_impl->m_dirty = true;
}

void
CandidatesUI::update_preedit_string (const WideString &str, const AttributeList &attrs)
{
    m_impl->m_preedit_str   = str;
    m_impl->m_preedit_attrs = attrs;
    m_impl->m_dirty = true;
}

void
CandidatesUI::update_preedit_caret (int caret)
{
    m_impl->m_preedit_caret = caret;
    m_impl->m_dirty = true;
}

void
CandidatesUI::show_preedit_string ()
{
    m_impl->m_preedit_visible = true;
    m_impl->m_dirty = true;
}

void
CandidatesUI::hide_preedit_string ()
{
    m_impl->m_preedit_visible = false;
    m_impl->m_dirty = true;
}

void
CandidatesUI::update_aux_string (const WideString &str, const AttributeList &attrs)
{
    m_impl->m_aux_str   = str;
    m_impl->m_aux_attrs = attrs;
    m_impl->m_dirty = true;
}

void
CandidatesUI::show_aux_string ()
{
    m_impl->m_aux_visible = true;
    m_impl->m_dirty = true;
}

void
CandidatesUI::hide_aux_string ()
{
    m_impl->m_aux_visible = false;
    m_impl->m_dirty = true;
}

void
CandidatesUI::update_lookup_table (const LookupTable &table)
{
    CandidatesUIImpl *d = m_impl;
    d->m_cand_text.clear ();
    d->m_cand_label.clear ();

    int page = table.get_current_page_size ();
    for (int i = 0; i < page; ++i) {
        d->m_cand_text.push_back (table.get_candidate_in_current_page (i));
        d->m_cand_label.push_back (table.get_candidate_label (i));
    }

    // TODO(5a): vertical orientation. LookupTable exposes no orientation getter
    // at this layer; wire from config/engine hint. Horizontal for now.
    d->m_lookup_vertical = false;
    d->m_cursor_in_page =
        table.is_cursor_visible () ? table.get_cursor_pos_in_current_page () : -1;

    int start = table.get_current_page_start ();
    d->m_has_prev_page = (start > 0);
    d->m_has_next_page =
        (start + page < static_cast<int> (table.number_of_candidates ()));

    d->m_dirty = true;
}

void
CandidatesUI::show_lookup_table ()
{
    m_impl->m_lookup_visible = true;
    m_impl->m_dirty = true;
}

void
CandidatesUI::hide_lookup_table ()
{
    m_impl->m_lookup_visible = false;
    m_impl->m_dirty = true;
}

bool
CandidatesUI::is_visible () const
{
    CandidatesUIImpl *d = m_impl;
    if (d->m_preedit_visible && d->m_preedit_str.length ()) return true;
    if (d->m_aux_visible && d->m_aux_str.length ()) return true;
    if (d->m_lookup_visible && !d->m_cand_text.empty ()) return true;
    return false;
}

void
CandidatesUI::measure (int &width, int &height)
{
    m_impl->layout ();
    width  = m_impl->m_width;
    height = m_impl->m_height;
}

void
CandidatesUI::draw (cairo_t *cr)
{
    CandidatesUIImpl *d = m_impl;
    d->layout ();

    const CandidatesTheme &t = d->m_theme;

    // Background.
    cairo_save (cr);
    cairo_set_source_rgba (cr, t.bg.r, t.bg.g, t.bg.b, t.bg.a);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);
    cairo_restore (cr);

    // Highlight backgrounds behind selected candidates.
    for (size_t i = 0; i < d->m_cells.size (); ++i) {
        const CandidateCell &c = d->m_cells[i];
        if (!c.highlighted) continue;
        int m = t.spacing / 2;
        cairo_set_source_rgba (cr, t.highlight_bg.r, t.highlight_bg.g,
                               t.highlight_bg.b, t.highlight_bg.a);
        cairo_rectangle (cr, c.item.x - m, c.item.y - m / 2,
                         c.item.w + 2 * m, c.item.h + m);
        cairo_fill (cr);
    }

    // Text.
    if (d->m_has_preedit_item)
        d->paint_item (cr, d->m_preedit_item, t.fg);

    if (d->m_has_caret) {
        cairo_set_source_rgba (cr, t.fg.r, t.fg.g, t.fg.b, t.fg.a);
        cairo_set_line_width (cr, 1.0);
        cairo_move_to (cr, d->m_caret_x + 0.5, d->m_caret_y);
        cairo_line_to (cr, d->m_caret_x + 0.5, d->m_caret_y + d->m_caret_h);
        cairo_stroke (cr);
    }

    if (d->m_has_aux_item)
        d->paint_item (cr, d->m_aux_item, t.fg);

    for (size_t i = 0; i < d->m_cells.size (); ++i)
        d->paint_item (cr, d->m_cells[i].item, t.fg);

    // TODO(5a): draw prev/next-page arrows when has_prev_page/has_next_page and
    // report them from hit_test() as HIT_PREV_PAGE/HIT_NEXT_PAGE (wheel paging
    // already works; this adds click targets). Also HiDPI: scale by output scale.

    // Border.
    if (t.border_width > 0) {
        cairo_set_source_rgba (cr, t.border.r, t.border.g, t.border.b, t.border.a);
        cairo_set_line_width (cr, t.border_width);
        double half = t.border_width / 2.0;
        cairo_rectangle (cr, half, half,
                         d->m_width - t.border_width,
                         d->m_height - t.border_width);
        cairo_stroke (cr);
    }
}

CandidatesUI::HitType
CandidatesUI::hit_test (int x, int y, int &candidate_index) const
{
    CandidatesUIImpl *d = m_impl;
    candidate_index = -1;
    for (size_t i = 0; i < d->m_cells.size (); ++i) {
        const CandidateCell &c = d->m_cells[i];
        int m = d->m_theme.spacing;
        if (x >= c.item.x - m && x <= c.item.x + c.item.w + m &&
            y >= c.item.y && y <= c.item.y + c.item.h) {
            candidate_index = c.index;
            return HIT_CANDIDATE;
        }
    }
    return HIT_NONE;
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
