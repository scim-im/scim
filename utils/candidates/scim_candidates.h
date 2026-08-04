/**
 * @file scim_candidates.h
 * @brief Backend-agnostic Cairo+Pango renderer for the cursor-relative
 *        input panel (preedit + aux + candidate lookup table).
 *
 * This is the "own-Cairo" candidate UI core. It lays out and draws the input
 * panel onto a caller-supplied cairo_t and answers hit-tests, but knows nothing
 * about windows, X11 or Wayland. A surface shim (e.g. CandidatesUIX11) owns the
 * actual toplevel/surface, drives measure()/draw() and routes pointer events
 * back through hit_test().
 */

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

#pragma once

#ifndef Uses_SCIM_ATTRIBUTE
#define Uses_SCIM_ATTRIBUTE
#endif
#ifndef Uses_SCIM_LOOKUP_TABLE
#define Uses_SCIM_LOOKUP_TABLE
#endif
#ifndef Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_BASE
#endif
#include <scim.h>
#include <cairo.h>

namespace scim {

/**
 * @brief An RGBA color, components in [0,1].
 */
struct CandidatesColor {
    double r;
    double g;
    double b;
    double a;
};

/**
 * @brief Config-driven theme for the Cairo input panel.
 *
 * SCIM historically config-drives the panel font and colors, so a Cairo theme
 * reading those same values is consistent with the GTK panel, not a regression.
 */
struct CandidatesTheme {
    String       font;          ///< Pango font description, e.g. "Sans 12".
    /**
     * @brief Pango font description for the preedit line.
     *
     * Empty means "same as font", which is the default. Set this to give the
     * string being composed its own family or size.
     */
    String       preedit_font;
    CandidatesColor bg;            ///< Window background; its alpha is honored
                                   ///< (0.8 by default, i.e. slightly see-through).
    CandidatesColor fg;            ///< Normal text.
    CandidatesColor preedit_fg;    ///< Preedit text.
    CandidatesColor label;         ///< Candidate label ("1." "2." ...).
    CandidatesColor highlight_bg;  ///< Selected-candidate background.
    CandidatesColor highlight_fg;  ///< Selected-candidate text.
    CandidatesColor border;        ///< Window border.
    int          border_width;  ///< Border thickness in px; 0 draws none.
    /**
     * @brief Corner radius in px; 0 draws a plain rectangle. Defaults to 4.
     *
     * Rounding cuts the corners out of the surface, so it only looks right
     * where the surface carries an alpha channel -- always on Wayland, on X11
     * only under a compositor (see CandidatesUIX11). Without one the renderer
     * falls back to a square, opaque panel by itself.
     */
    int          corner_radius;
    int          padding;       ///< Inner padding in px.
    int          spacing;       ///< Gap between rows / candidates in px.

    /** @brief A sane light-theme default. */
    static CandidatesTheme light ();
    /** @brief A sane dark-theme default. */
    static CandidatesTheme dark ();
};

/**
 * @brief Tell the renderer whether the desktop is currently dark.
 *
 * Only consulted when /Candidates/<renderer>/ColorScheme is "auto", which is the
 * default; an explicit "light" or "dark" wins. Process-wide, and light until
 * something says otherwise -- this library deliberately has no way to find out
 * for itself. Finding out needs either a toolkit (GtkSettings, QPalette) or the
 * org.freedesktop.appearance portal over D-Bus, and this code is linked into
 * frontends and into GTK and Qt input-method modules alike, so requiring either
 * would put a dependency on every one of them for the sake of one colour choice.
 * The host already knows; it just has to say.
 *
 * Safe to call repeatedly, including on a theme change: the value is only read
 * when a theme is next built, so follow it with a fresh
 * scim_candidates_theme_from_config () and set_theme () to repaint.
 */
void scim_candidates_set_dark_hint (bool dark);

/** @brief What the last scim_candidates_set_dark_hint () said. */
bool scim_candidates_dark_hint (void);

/**
 * @brief Build a theme from the SCIM config for a candidate renderer.
 *
 * Reads the appearance keys -- ColorScheme, Font, PreeditFont, BorderWidth,
 * CornerRadius, Padding, Spacing and Color/{NormalText,NormalBackground,
 * PreeditText,ActiveText,ActiveBackground,Label,Border} -- with a layered
 * fallback:
 *   /Candidates/<renderer>/<key>  ->  /Candidates/Default/<key>  ->  the preset
 *   base, CandidatesTheme::light () or ::dark () (see ColorScheme below).
 * @p renderer selects the top level and defaults to "Default" (the
 * toolkit-agnostic Cairo renderer); a GTK or Qt renderer can override through
 * "/Candidates/Gtk/" or "/Candidates/Qt/".
 *
 * Nothing is inherited from the panel's own "/Panel/Gtk/" keys. Those style the
 * panel's lookup table and describe a single light palette, so reaching here they
 * overrode the presets -- a changed default was invisible to anyone whose config
 * carried them -- and no light palette can serve the dark preset.
 *
 * ColorScheme is "auto" (the default), "light" or "dark". "auto" follows
 * scim_candidates_set_dark_hint (), so it tracks the desktop only where the host
 * can say -- a frontend with no toolkit and no portal to ask stays light, and
 * wants an explicit "dark".
 *
 * Color values are CSS/X11 strings. Besides the names and "#rrggbb" that Pango
 * understands, "#rrggbbaa" and "rgb()"/"rgba()" are accepted, so a value can
 * carry an alpha -- that is how a translucent background is configured, and it
 * is the syntax a GTK color chooser with alpha enabled writes.
 *
 * A background is taken exactly as written, alpha included, so "#rrggbbff" or any
 * name/"#rrggbb" asks for a fully opaque panel; leaving it unset keeps the
 * preset's slight translucency.
 */
CandidatesTheme scim_candidates_theme_from_config (const ConfigPointer &config,
                                                   const String &renderer = String ("Default"));

/**
 * @brief Parse a color string exactly as the theme loader does.
 *
 * Accepts CSS/X11 names, "#rgb", "#rrggbb", "#rrggbbaa", "rgb()" and "rgba()".
 * Exposed so a settings UI writes only values the renderer can read back.
 *
 * @param str  the string to parse.
 * @param out  set to the parsed color on success; untouched on failure.
 * @return     true when @p str was understood.
 */
bool scim_candidates_parse_color (const String &str, CandidatesColor &out);

/**
 * @brief Format a color for the config, as the shortest form that round-trips.
 *
 * "#rrggbb" for an opaque color, "#rrggbbaa" when it carries an alpha -- so a
 * fully opaque value stays readable by anything that only understands the plain
 * hex form.
 */
String scim_candidates_format_color (const CandidatesColor &color);

/**
 * @brief Backend-agnostic layout/draw of the input panel.
 *
 * The update_* / show_* / hide_* methods mirror the cursor-relative subset of
 * scim::PanelClient, so a frontend can drive this in-process with the same
 * calls it used to forward to scim-panel-gtk.
 */
class CandidatesUI
{
    class CandidatesUIImpl;
    CandidatesUIImpl *m_impl;

    CandidatesUI (const CandidatesUI &);
    const CandidatesUI & operator = (const CandidatesUI &);

public:
    /** @brief Kind of region a point falls in (see hit_test()). */
    enum HitType {
        HIT_NONE,       ///< Nothing actionable.
        HIT_CANDIDATE,  ///< A candidate cell (index returned).
        HIT_PREV_PAGE,  ///< The "previous page" affordance.
        HIT_NEXT_PAGE   ///< The "next page" affordance.
    };

    CandidatesUI ();
    ~CandidatesUI ();

    /** @name Theming @{ */
    void set_theme (const CandidatesTheme &theme);
    const CandidatesTheme & get_theme () const;
    /** @brief Convenience: override just the font description. */
    void set_font (const String &font_desc);
    /**
     * @brief Mirror the order of the sections within the panel.
     *
     * Each section keeps its own layout; only their order top-to-bottom is
     * reversed, so candidate rows and the caret stay put relative to their
     * own block. Used when the panel sits above the text being edited, to
     * keep the section nearest the text closest to it.
     */
    void set_sections_reversed (bool reversed);
    /** @} */

    /** @name State updates (mirror PanelClient cursor-relative calls) @{ */
    void update_preedit_string (const WideString &str, const AttributeList &attrs);
    void update_preedit_caret  (int caret);
    void show_preedit_string   ();
    void hide_preedit_string   ();

    void update_aux_string     (const WideString &str, const AttributeList &attrs);
    void show_aux_string       ();
    void hide_aux_string       ();

    void update_lookup_table   (const LookupTable &table);
    void show_lookup_table     ();
    void hide_lookup_table     ();
    /** @} */

    /** @brief True if any section is visible and has content to draw. */
    bool is_visible () const;

    /**
     * @brief Compute the panel's preferred pixel size (border included).
     *
     * Safe to call without a live surface; uses an internal scratch context.
     */
    void measure (int &width, int &height);

    /**
     * @brief Draw the panel with its top-left at the cairo origin.
     *
     * The caller should have sized/cleared the surface per measure().
     */
    void draw (cairo_t *cr);

    /**
     * @brief Map a point (panel-relative px) to an actionable region.
     * @param x,y  Coordinates relative to the panel top-left.
     * @param candidate_index  Set to the candidate index when HIT_CANDIDATE.
     * @return The region kind.
     */
    HitType hit_test (int x, int y, int &candidate_index) const;
};

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
