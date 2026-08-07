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
    t.font          = "Sans 12";
    t.preedit_font  = "";
    t.bg            = { 0.98, 0.98, 0.98, 0.8 };
    t.fg            = { 0.10, 0.10, 0.10, 1.0 };
    t.preedit_fg    = { 0.10, 0.10, 0.10, 1.0 };
    t.label         = { 0.45, 0.45, 0.45, 1.0 };
    t.highlight_bg  = { 0.20, 0.50, 0.90, 1.0 };
    t.highlight_fg  = { 1.00, 1.00, 1.00, 1.0 };
    t.border        = { 0.60, 0.60, 0.60, 1.0 };
    t.border_width  = 1;
    t.corner_radius = 4;
    t.padding       = 6;
    t.spacing       = 6;
    t.vertical      = false;
    return t;
}

CandidatesTheme
CandidatesTheme::dark ()
{
    CandidatesTheme t;
    t.font          = "Sans 12";
    t.preedit_font  = "";
    t.bg            = { 0.16, 0.16, 0.16, 0.8 };
    t.fg            = { 0.92, 0.92, 0.92, 1.0 };
    t.preedit_fg    = { 0.92, 0.92, 0.92, 1.0 };
    t.label         = { 0.60, 0.60, 0.60, 1.0 };
    t.highlight_bg  = { 0.24, 0.52, 0.90, 1.0 };
    t.highlight_fg  = { 1.00, 1.00, 1.00, 1.0 };
    t.border        = { 0.40, 0.40, 0.40, 1.0 };
    t.border_width  = 1;
    t.corner_radius = 4;
    t.padding       = 6;
    t.spacing       = 6;
    t.vertical      = false;
    return t;
}

/* ------------------------------------------------------------------ */
/* Light / dark                                                        */
/* ------------------------------------------------------------------ */

static bool __dark_hint = false;

void
scim_candidates_set_dark_hint (bool dark)
{
    __dark_hint = dark;
}

bool
scim_candidates_dark_hint (void)
{
    return __dark_hint;
}

// Parse "rgb(r,g,b)" / "rgba(r,g,b,a)" -- the form a GTK color chooser writes
// through gdk_rgba_to_string (), and the only CSS syntax that can carry an
// alpha besides "#rrggbbaa". Components are 0-255 or percentages, alpha is a
// float in [0,1]. Locale-independent on purpose: the config file is not
// localized, so a comma decimal separator must never be accepted here.
static bool
parse_rgba_function (const String &s, CandidatesColor &out)
{
    const char *p = s.c_str ();
    while (g_ascii_isspace (*p)) ++p;

    bool has_alpha;
    if (g_ascii_strncasecmp (p, "rgba", 4) == 0) { has_alpha = true;  p += 4; }
    else if (g_ascii_strncasecmp (p, "rgb", 3) == 0) { has_alpha = false; p += 3; }
    else return false;

    while (g_ascii_isspace (*p)) ++p;
    if (*p != '(') return false;
    ++p;

    double v[4] = { 0.0, 0.0, 0.0, 1.0 };
    const int n = has_alpha ? 4 : 3;
    for (int i = 0; i < n; ++i) {
        while (g_ascii_isspace (*p)) ++p;
        char *end = 0;
        double d = g_ascii_strtod (p, &end);
        if (end == p) return false;
        p = end;
        if (*p == '%') {
            d /= 100.0;
            ++p;
        } else if (i < 3) {
            d /= 255.0;      // plain rgb components are 0-255
        }
        v[i] = d < 0.0 ? 0.0 : (d > 1.0 ? 1.0 : d);

        while (g_ascii_isspace (*p)) ++p;
        if (i + 1 < n) {
            // A comma is conventional but CSS also allows bare spaces.
            if (*p == ',') ++p;
        }
    }

    while (g_ascii_isspace (*p)) ++p;
    if (*p != ')') return false;

    out.r = v[0]; out.g = v[1]; out.b = v[2]; out.a = v[3];
    return true;
}

// Parse "#rrggbbaa". Pango handles every other hex length, but not the one that
// carries an alpha, so only the 8-digit form is claimed here.
static bool
parse_hex_rgba (const String &s, CandidatesColor &out)
{
    if (s.length () != 9 || s[0] != '#')
        return false;

    unsigned int v[4];
    for (int i = 0; i < 4; ++i) {
        int hi = g_ascii_xdigit_value (s[1 + i * 2]);
        int lo = g_ascii_xdigit_value (s[2 + i * 2]);
        if (hi < 0 || lo < 0)
            return false;
        v[i] = (unsigned int) (hi * 16 + lo);
    }

    out.r = v[0] / 255.0; out.g = v[1] / 255.0;
    out.b = v[2] / 255.0; out.a = v[3] / 255.0;
    return true;
}

// Parse a CSS/X11 color string ("gray92", "light blue", "#rrggbb", "#rrggbbaa",
// "rgb()", "rgba()") into an RGBA color. Pango covers the names and the shorter
// hex forms (no GDK dependency); the alpha-carrying forms are handled above.
// Alpha defaults to 1 when the syntax cannot express one.
static CandidatesColor
parse_color (const String &s, const CandidatesColor &fallback)
{
    if (!s.length ())
        return fallback;

    CandidatesColor c;
    if (parse_hex_rgba (s, c) || parse_rgba_function (s, c))
        return c;

    PangoColor pc;
    if (pango_color_parse (&pc, s.c_str ())) {
        CandidatesColor p = { pc.red / 65535.0, pc.green / 65535.0, pc.blue / 65535.0, 1.0 };
        return p;
    }
    return fallback;
}

bool
scim_candidates_parse_color (const String &str, CandidatesColor &out)
{
    if (!str.length ())
        return false;

    if (parse_hex_rgba (str, out) || parse_rgba_function (str, out))
        return true;

    PangoColor pc;
    if (pango_color_parse (&pc, str.c_str ())) {
        out.r = pc.red / 65535.0;
        out.g = pc.green / 65535.0;
        out.b = pc.blue / 65535.0;
        out.a = 1.0;
        return true;
    }
    return false;
}

String
scim_candidates_format_color (const CandidatesColor &color)
{
    auto byte = [] (double v) -> int {
        double d = v * 255.0 + 0.5;
        if (d < 0.0)   d = 0.0;
        if (d > 255.0) d = 255.0;
        return (int) d;
    };

    const int r = byte (color.r), g = byte (color.g), b = byte (color.b);
    const int a = byte (color.a);

    gchar *s = (a >= 255)
        ? g_strdup_printf ("#%02x%02x%02x", r, g, b)
        : g_strdup_printf ("#%02x%02x%02x%02x", r, g, b, a);
    String out (s);
    g_free (s);
    return out;
}

// Append a rectangle to the current path, with the corners rounded to radius r.
// r <= 0 gives a plain rectangle; a radius larger than half the shorter side is
// clamped, which turns a small panel into a stadium rather than a tangle.
static void
rounded_rect (cairo_t *cr, double x, double y, double w, double h, double r)
{
    if (r <= 0.0 || w <= 0.0 || h <= 0.0) {
        cairo_rectangle (cr, x, y, w, h);
        return;
    }

    const double max = (w < h ? w : h) / 2.0;
    if (r > max) r = max;

    const double pi = 3.14159265358979323846;
    cairo_new_sub_path (cr);
    cairo_arc (cr, x + w - r, y + r,     r, -pi / 2.0, 0.0);
    cairo_arc (cr, x + w - r, y + h - r, r, 0.0,        pi / 2.0);
    cairo_arc (cr, x + r,     y + h - r, r, pi / 2.0,   pi);
    cairo_arc (cr, x + r,     y + r,     r, pi,         3.0 * pi / 2.0);
    cairo_close_path (cr);
}

// Which level of the layered lookup answered. Worth telling apart because the
// Read an appearance key with a layered fallback:
//   /Candidates/<renderer>/<key>  ->  /Candidates/Default/<key>  ->  "" (unset)
// so a per-renderer override wins, otherwise the shared Candidates/Default value,
// otherwise the preset supplies it. When skip_default is set, the sentinel
// "default" is treated as unset at each level, so writing "default" in setup does
// not shadow a lower level.
static String
read_layered_key (const ConfigPointer &config, const String &renderer,
                  const String &key, bool skip_default = false)
{
    const String levels[] = {
        String ("/Candidates/") + renderer + "/" + key,
        renderer == String ("Default") ? String () : String ("/Candidates/Default/") + key,
    };
    for (size_t i = 0; i < sizeof levels / sizeof levels[0]; ++i) {
        if (levels[i].empty ())
            continue;
        String v = config->read (levels[i], String ());
        if (v.length () && (!skip_default || v != String ("default")))
            return v;
    }
    return String ();
}

// The int counterpart of read_layered_key (): same levels, but "unset" has to be
// told apart from a meaningful 0 (no border, square corners), so this uses the
// ConfigBase overload that reports whether the key existed rather than a
// sentinel default.
static bool
read_layered_int (const ConfigPointer &config, const String &renderer,
                  const String &key, int &out)
{
    const String levels[] = {
        String ("/Candidates/") + renderer + "/" + key,
        renderer == String ("Default") ? String () : String ("/Candidates/Default/") + key,
    };
    for (const String &path : levels) {
        if (path.empty ())
            continue;
        int v = 0;
        if (config->read (path, &v)) {
            out = v;
            return true;
        }
    }
    return false;
}

CandidatesTheme
scim_candidates_theme_from_config (const ConfigPointer &config, const String &renderer)
{
    // Which preset the unset keys fall back to. "auto" follows the desktop
    // through whatever the host told us (see scim_candidates_set_dark_hint);
    // "light" and "dark" pin it, which is what to use where nothing can find the
    // preference out -- a bare compositor with no portal running, say.
    //
    // Only the base changes: every key below still overrides it, so a config that
    // names its own colours is unaffected by this.
    bool want_dark = scim_candidates_dark_hint ();
    if (!config.null ()) {
        String scheme = read_layered_key (config, renderer, String ("ColorScheme"), true);
        if (scheme == "dark")       want_dark = true;
        else if (scheme == "light") want_dark = false;
    }

    CandidatesTheme t = want_dark ? CandidatesTheme::dark ()
                                  : CandidatesTheme::light ();
    if (config.null ())
        return t;

    String font = read_layered_key (config, renderer, String ("Font"), true);
    if (font.length ())
        t.font = font;

    // Empty keeps the preedit on the shared font, which is the default.
    t.preedit_font = read_layered_key (config, renderer, String ("PreeditFont"), true);

    // parse_color () keeps the preset's field when its string is empty or
    // unparsable, so an unset key needs no default string here: the preset is the
    // default. Nothing is inherited from /Panel/Gtk/ any more -- those keys style
    // the panel's own lookup table and describe a single light palette, so letting
    // them reach here overrode the presets (making a new default invisible) and
    // could never be right for the dark one.
    auto color = [&] (const char *key) -> String {
        return read_layered_key (config, renderer, String (key));
    };
    // The panel is slightly translucent by default (see light ()). A background
    // set here is taken exactly as written, which is how to ask for a fully opaque
    // panel -- "#rrggbbff", or any name/#rrggbb, since those parse as opaque.
    t.bg           = parse_color (color ("Color/NormalBackground"), t.bg);
    t.fg           = parse_color (color ("Color/NormalText"),       t.fg);
    t.highlight_bg = parse_color (color ("Color/ActiveBackground"), t.highlight_bg);
    t.highlight_fg = parse_color (color ("Color/ActiveText"),       t.highlight_fg);
    t.label        = parse_color (color ("Color/Label"),             t.label);
    t.border       = parse_color (color ("Color/Border"),            t.border);
    // The preedit shares the normal text color unless it is given its own, so
    // that setting only NormalText still colors the whole panel consistently.
    t.preedit_fg   = parse_color (color ("Color/PreeditText"),       t.fg);

    // "vertical" or "horizontal"; anything else leaves the preset alone.
    String orientation = read_layered_key (config, renderer, String ("Orientation"), true);
    if (orientation == "vertical")        t.vertical = true;
    else if (orientation == "horizontal") t.vertical = false;

    read_layered_int (config, renderer, String ("BorderWidth"),  t.border_width);
    read_layered_int (config, renderer, String ("CornerRadius"), t.corner_radius);
    read_layered_int (config, renderer, String ("Padding"),      t.padding);
    read_layered_int (config, renderer, String ("Spacing"),      t.spacing);

    // Nonsense values would corrupt the layout arithmetic rather than merely
    // look wrong, so clamp instead of trusting the config.
    if (t.border_width  < 0) t.border_width  = 0;
    if (t.corner_radius < 0) t.corner_radius = 0;
    if (t.padding       < 0) t.padding       = 0;
    if (t.spacing       < 0) t.spacing       = 0;

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
    String         font;         // pango font description; empty = the theme font
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
                  const CandidatesColor &fg,
                  const CandidatesColor &sel_bg, const CandidatesColor &sel_fg)
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
                // Both mean "this part is the active one", so both get the
                // theme's selection colors -- the same ones the selected
                // candidate uses, so the composing text reads in the same visual
                // language as the row below it.
                //
                // Taking REVERSE literally is what made the preedit an inverted
                // slab: swapping in the foreground color gave a near-black
                // background under a pale window, with the glyphs hardcoded
                // white. That could not follow a theme and was illegible against
                // the dark preset. Engines use the two interchangeably in
                // practice, so one consistent look beats the literal reading.
                pa = pango_attr_background_new (
                        (guint16)(sel_bg.r * 65535), (guint16)(sel_bg.g * 65535),
                        (guint16)(sel_bg.b * 65535));
                pa->start_index = bstart;
                pa->end_index   = bend;
                pango_attr_list_insert (plist, pa);
                pa = pango_attr_foreground_new (
                        (guint16)(sel_fg.r * 65535), (guint16)(sel_fg.g * 65535),
                        (guint16)(sel_fg.b * 65535));
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
    bool          m_sections_reversed;
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

    /**
     * @brief A page-flip chevron: where it is and whether it leads anywhere.
     *
     * Laid out in the same logical pixels as everything else, so hit_test ()
     * compares against these directly. Both are placed whenever the list runs to
     * more than one page, the unreachable direction merely drawn dimmed -- hiding
     * one would change the panel's width as the user reaches the last page,
     * which is a jump right under the caret.
     */
    struct PageArrow {
        int  x, y, w, h;
        bool enabled;
    };
    bool      m_has_arrows;
    PageArrow m_prev_arrow;
    PageArrow m_next_arrow;

    // A scratch cairo context for measuring without a live surface.
    cairo_surface_t *m_scratch_surface;
    cairo_t         *m_scratch_cr;

    CandidatesUIImpl ()
        : m_theme (CandidatesTheme::light ()),
          m_preedit_visible (false), m_preedit_caret (0),
          m_aux_visible (false),
          m_lookup_visible (false),
          m_sections_reversed (false),
          m_cursor_in_page (-1), m_has_prev_page (false), m_has_next_page (false),
          m_dirty (true), m_width (0), m_height (0),
          m_has_preedit_item (false),
          m_caret_x (0), m_caret_y (0), m_caret_h (0), m_has_caret (false),
          m_has_aux_item (false),
          m_has_arrows (false),
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

    // Chevrons are sized from the candidate row so they track the font rather
    // than a fixed pixel count. Narrower than they are tall, which is the shape
    // of the glyph; the floor keeps a tiny font from leaving nothing to click.
    static int arrow_width (int row_height)
    {
        int w = row_height / 2;
        return w < 8 ? 8 : w;
    }

    /**
     * @brief Stroke one page-flip chevron.
     *
     * A path rather than an icon: it costs no assets, it is sharp at whatever
     * scale the host renders at, and being drawn in the theme's own text color
     * it follows a light or dark panel without a second set of images. A glyph
     * would do as well, but not every CJK font carries the arrow characters.
     *
     * Points along the list: left/right beside a horizontal row, up/down beside
     * a vertical one.
     */
    void draw_chevron (cairo_t *cr, const PageArrow &a, bool prev)
    {
        const double cx = a.x + a.w / 2.0;
        const double cy = a.y + a.h / 2.0;
        const double d  = (a.w < a.h ? a.w : a.h) * 0.25;

        cairo_save (cr);
        // The direction that cannot be taken is dimmed rather than dropped, so
        // the panel keeps its size while the user pages through the list.
        cairo_set_source_rgba (cr, m_theme.fg.r, m_theme.fg.g, m_theme.fg.b,
                               m_theme.fg.a * (a.enabled ? 1.0 : 0.3));
        double lw = a.h * 0.08;
        if (lw < 1.5) lw = 1.5;
        cairo_set_line_width (cr, lw);
        cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
        cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);

        if (m_theme.vertical) {
            if (prev) {                       // up
                cairo_move_to (cr, cx - d, cy + d * 0.6);
                cairo_line_to (cr, cx,     cy - d * 0.6);
                cairo_line_to (cr, cx + d, cy + d * 0.6);
            } else {                          // down
                cairo_move_to (cr, cx - d, cy - d * 0.6);
                cairo_line_to (cr, cx,     cy + d * 0.6);
                cairo_line_to (cr, cx + d, cy - d * 0.6);
            }
        } else {
            if (prev) {                       // left
                cairo_move_to (cr, cx + d * 0.6, cy - d);
                cairo_line_to (cr, cx - d * 0.6, cy);
                cairo_line_to (cr, cx + d * 0.6, cy + d);
            } else {                          // right
                cairo_move_to (cr, cx - d * 0.6, cy - d);
                cairo_line_to (cr, cx + d * 0.6, cy);
                cairo_line_to (cr, cx - d * 0.6, cy + d);
            }
        }

        cairo_stroke (cr);
        cairo_restore (cr);
    }

    void clear_layout ()
    {
        free_item_attrs (m_preedit_item);
        free_item_attrs (m_aux_item);
        for (size_t i = 0; i < m_cells.size (); ++i)
            free_item_attrs (m_cells[i].item);
        m_cells.clear ();
        m_has_arrows = false;
        m_has_preedit_item = false;
        m_has_aux_item = false;
        m_has_caret = false;
    }

    // Build a PangoLayout on cr for utf8 with attrs. An empty font falls back to
    // the theme font, so only the sections that were given their own carry one.
    PangoLayout * make_layout (cairo_t *cr, const String &utf8,
                               PangoAttrList *attrs, const String &font)
    {
        PangoLayout *layout = pango_cairo_create_layout (cr);
        PangoFontDescription *desc = pango_font_description_from_string (
            font.length () ? font.c_str () : m_theme.font.c_str ());
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
        PangoLayout *layout = make_layout (cr, it.utf8, it.attrs, it.font);
        int w = 0, h = 0;
        pango_layout_get_pixel_size (layout, &w, &h);
        it.w = w;
        it.h = h;
        g_object_unref (layout);
    }

    // Rebuild geometry from raw state.
    // Mirror the order of the sections after they have been laid out top-down.
    // Whole blocks are translated, never re-laid-out, so candidate rows keep
    // their order within the table and the caret keeps its offset in the
    // preedit line. Total height is unchanged, since the same blocks and gaps
    // are simply visited in the opposite order.
    void reverse_sections (int origin, int spacing)
    {
        struct Block { int kind; int y, h; };   // 0 preedit, 1 aux, 2 candidates
        std::vector<Block> blocks;

        if (m_has_preedit_item)
            blocks.push_back (Block { 0, m_preedit_item.y, m_preedit_item.h });
        if (m_has_aux_item)
            blocks.push_back (Block { 1, m_aux_item.y, m_aux_item.h });
        if (!m_cells.empty ()) {
            int top = m_cells[0].item.y;
            int bot = top;
            for (size_t i = 0; i < m_cells.size (); ++i) {
                const TextItem &it = m_cells[i].item;
                if (it.y < top)          top = it.y;
                if (it.y + it.h > bot)   bot = it.y + it.h;
            }
            // The chevrons belong to this block and move with it. In a vertical
            // list they sit on their own row below the candidates, so leaving
            // them out would stretch the block past them and strand them.
            if (m_has_arrows) {
                if (m_prev_arrow.y < top) top = m_prev_arrow.y;
                if (m_prev_arrow.y + m_prev_arrow.h > bot)
                    bot = m_prev_arrow.y + m_prev_arrow.h;
                if (m_next_arrow.y + m_next_arrow.h > bot)
                    bot = m_next_arrow.y + m_next_arrow.h;
            }
            blocks.push_back (Block { 2, top, bot - top });
        }

        if (blocks.size () < 2)
            return;

        int y = origin;
        for (int i = (int) blocks.size () - 1; i >= 0; --i) {
            int dy = y - blocks[i].y;
            switch (blocks[i].kind) {
            case 0:
                m_preedit_item.y += dy;
                m_caret_y       += dy;
                break;
            case 1:
                m_aux_item.y += dy;
                break;
            default:
                for (size_t c = 0; c < m_cells.size (); ++c)
                    m_cells[c].item.y += dy;
                if (m_has_arrows) {
                    m_prev_arrow.y += dy;
                    m_next_arrow.y += dy;
                }
                break;
            }
            y += blocks[i].h + spacing;
        }
    }

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
                make_pango_attrs (m_preedit_str, m_preedit_attrs, m_theme.preedit_fg,
                                  m_theme.highlight_bg, m_theme.highlight_fg);
            m_preedit_item.font = m_theme.preedit_font;
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
                make_layout (cr, m_preedit_item.utf8, m_preedit_item.attrs,
                             m_preedit_item.font);
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
            m_aux_item.attrs = make_pango_attrs (m_aux_str, m_aux_attrs, m_theme.fg,
                                     m_theme.highlight_bg, m_theme.highlight_fg);
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

            if (m_theme.vertical) {
                int cellh = 0;
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
                    if (cell.item.h > cellh) cellh = cell.item.h;
                    if (cell.item.x + cell.item.w > max_x)
                        max_x = cell.item.x + cell.item.w;
                    m_cells.push_back (cell);
                }

                // Side by side on a row of their own beneath the list, since
                // stacking them would make an already tall panel taller.
                if (m_has_prev_page || m_has_next_page) {
                    const int aw = arrow_width (cellh);
                    m_prev_arrow = PageArrow { origin, cur_y, aw, cellh, m_has_prev_page };
                    m_next_arrow = PageArrow { origin + aw + spacing * 2, cur_y,
                                               aw, cellh, m_has_next_page };
                    cur_y += cellh + spacing;
                    if (m_next_arrow.x + aw > max_x)
                        max_x = m_next_arrow.x + aw;
                    m_has_arrows = true;
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
                int right = x - spacing * 2;

                // Chevrons at the end of the row, where the eye lands after the
                // last candidate and where the panel is already widest.
                if (m_has_prev_page || m_has_next_page) {
                    const int aw = arrow_width (rowh);
                    int ax = right + spacing * 2;
                    m_prev_arrow = PageArrow { ax, cur_y, aw, rowh, m_has_prev_page };
                    ax += aw + spacing;
                    m_next_arrow = PageArrow { ax, cur_y, aw, rowh, m_has_next_page };
                    right = ax + aw;
                    m_has_arrows = true;
                }

                if (right > max_x)
                    max_x = right;
                cur_y += rowh + spacing;
            }
        }

        if (m_sections_reversed)
            reverse_sections (origin, spacing);

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
        PangoLayout *layout = make_layout (cr, it.utf8, it.attrs, it.font);
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

void
CandidatesUI::set_sections_reversed (bool reversed)
{
    if (m_impl->m_sections_reversed == reversed)
        return;
    m_impl->m_sections_reversed = reversed;
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

    // Orientation is deliberately not touched here: it comes from the theme
    // (/Candidates/<renderer>/Orientation), and this runs on every keystroke,
    // so setting it from the table would undo the configured value each time.
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

    // Rounding and translucency both need somewhere for the missing pixels to
    // go. On a surface with no alpha channel there is nowhere: a cut corner
    // comes out black rather than showing the window behind, and a translucent
    // background composites over black and just looks muddy. That happens on
    // X11 with no compositor running, so rather than make the config responsible
    // for knowing which surface it landed on, ask the surface and fall back to
    // an opaque rectangle.
    const bool alpha_ok =
        cairo_surface_get_content (cairo_get_target (cr)) != CAIRO_CONTENT_COLOR;
    const double radius = alpha_ok ? t.corner_radius : 0.0;
    const double bg_a   = alpha_ok ? t.bg.a : 1.0;

    // Background: clear first, then fill only the rounded shape, so the corners
    // really are absent from the surface instead of being filled with the
    // background color.
    cairo_save (cr);
    cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint (cr);
    cairo_restore (cr);

    cairo_save (cr);
    rounded_rect (cr, 0.0, 0.0, d->m_width, d->m_height, radius);
    cairo_set_source_rgba (cr, t.bg.r, t.bg.g, t.bg.b, bg_a);
    cairo_fill_preserve (cr);
    // Keep the content inside the rounded outline: a highlight on the first or
    // last candidate reaches the panel edge and would otherwise square off the
    // corner it sits in.
    cairo_clip (cr);

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
        d->paint_item (cr, d->m_preedit_item, t.preedit_fg);

    if (d->m_has_caret) {
        cairo_set_source_rgba (cr, t.preedit_fg.r, t.preedit_fg.g,
                               t.preedit_fg.b, t.preedit_fg.a);
        cairo_set_line_width (cr, 1.0);
        cairo_move_to (cr, d->m_caret_x + 0.5, d->m_caret_y);
        cairo_line_to (cr, d->m_caret_x + 0.5, d->m_caret_y + d->m_caret_h);
        cairo_stroke (cr);
    }

    if (d->m_has_aux_item)
        d->paint_item (cr, d->m_aux_item, t.fg);

    for (size_t i = 0; i < d->m_cells.size (); ++i)
        d->paint_item (cr, d->m_cells[i].item, t.fg);

    if (d->m_has_arrows) {
        d->draw_chevron (cr, d->m_prev_arrow, true);
        d->draw_chevron (cr, d->m_next_arrow, false);
    }

    // Border. Stroked on a path inset by half the line width so the whole line
    // lands inside the panel, with the radius pulled in by the same amount to
    // stay concentric with the background's curve.
    if (t.border_width > 0) {
        double half = t.border_width / 2.0;
        double r    = radius - half;
        if (r < 0.0) r = 0.0;
        cairo_set_source_rgba (cr, t.border.r, t.border.g, t.border.b, t.border.a);
        cairo_set_line_width (cr, t.border_width);
        rounded_rect (cr, half, half,
                      d->m_width - t.border_width,
                      d->m_height - t.border_width, r);
        cairo_stroke (cr);
    }

    cairo_restore (cr);
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

    // Only the direction that leads somewhere answers; a click on the dimmed
    // one falls through to HIT_NONE rather than asking for a page that is not
    // there.
    if (d->m_has_arrows) {
        const CandidatesUIImpl::PageArrow &p = d->m_prev_arrow;
        const CandidatesUIImpl::PageArrow &n = d->m_next_arrow;
        if (p.enabled &&
            x >= p.x && x <= p.x + p.w && y >= p.y && y <= p.y + p.h)
            return HIT_PREV_PAGE;
        if (n.enabled &&
            x >= n.x && x <= n.x + n.w && y >= n.y && y <= n.y + n.h)
            return HIT_NEXT_PAGE;
    }

    return HIT_NONE;
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
