/**
 * @file scim_panel_ui.h
 * @brief Backend-agnostic Cairo+Pango renderer for the cursor-relative
 *        input panel (preedit + aux + candidate lookup table).
 *
 * This is the "own-Cairo" candidate UI core. It lays out and draws the input
 * panel onto a caller-supplied cairo_t and answers hit-tests, but knows nothing
 * about windows, X11 or Wayland. A surface shim (e.g. PanelUIX11) owns the
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
struct PanelUIColor {
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
struct PanelUITheme {
    String       font;          ///< Pango font description, e.g. "Sans 12".
    PanelUIColor bg;            ///< Window background.
    PanelUIColor fg;            ///< Normal text.
    PanelUIColor label;         ///< Candidate label ("1." "2." ...).
    PanelUIColor highlight_bg;  ///< Selected-candidate background.
    PanelUIColor highlight_fg;  ///< Selected-candidate text.
    PanelUIColor border;        ///< Window border.
    int          border_width;  ///< Border thickness in px.
    int          padding;       ///< Inner padding in px.
    int          spacing;       ///< Gap between rows / candidates in px.

    /** @brief A sane light-theme default. */
    static PanelUITheme light ();
    /** @brief A sane dark-theme default. */
    static PanelUITheme dark ();
};

/**
 * @brief Build a theme from the SCIM config, matching the legacy GTK panel.
 *
 * Reads /Panel/Gtk/Font and /Panel/Gtk/Color/{NormalText,NormalBackground,
 * ActiveText,ActiveBackground} (CSS/X11 color strings) into a PanelUITheme, so
 * every in-process renderer honours the user's configured font and colors the
 * same way. Falls back to PanelUITheme::light() for missing/unparsable values.
 * The PanelUI class itself stays config-agnostic; this is the shared bridge.
 */
PanelUITheme scim_panel_ui_theme_from_config (const ConfigPointer &config);

/**
 * @brief Backend-agnostic layout/draw of the input panel.
 *
 * The update_* / show_* / hide_* methods mirror the cursor-relative subset of
 * scim::PanelClient, so a frontend can drive this in-process with the same
 * calls it used to forward to scim-panel-gtk.
 */
class PanelUI
{
    class PanelUIImpl;
    PanelUIImpl *m_impl;

    PanelUI (const PanelUI &);
    const PanelUI & operator = (const PanelUI &);

public:
    /** @brief Kind of region a point falls in (see hit_test()). */
    enum HitType {
        HIT_NONE,       ///< Nothing actionable.
        HIT_CANDIDATE,  ///< A candidate cell (index returned).
        HIT_PREV_PAGE,  ///< The "previous page" affordance.
        HIT_NEXT_PAGE   ///< The "next page" affordance.
    };

    PanelUI ();
    ~PanelUI ();

    /** @name Theming @{ */
    void set_theme (const PanelUITheme &theme);
    const PanelUITheme & get_theme () const;
    /** @brief Convenience: override just the font description. */
    void set_font (const String &font_desc);
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
