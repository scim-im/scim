/** @file scim_candidates_setup.cpp
 * implementation of the Setup Module for the candidate window's appearance.
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

#define Uses_SCIM_CONFIG_BASE

#include <gtk/gtk.h>
#include "scim_private.h"
#include "scim.h"
#include "scim_candidates.h"

using namespace scim;

#define scim_module_init candidates_setup_LTX_scim_module_init
#define scim_module_exit candidates_setup_LTX_scim_module_exit

#define scim_setup_module_create_ui       candidates_setup_LTX_scim_setup_module_create_ui
#define scim_setup_module_get_category    candidates_setup_LTX_scim_setup_module_get_category
#define scim_setup_module_get_name        candidates_setup_LTX_scim_setup_module_get_name
#define scim_setup_module_get_description candidates_setup_LTX_scim_setup_module_get_description
#define scim_setup_module_load_config     candidates_setup_LTX_scim_setup_module_load_config
#define scim_setup_module_save_config     candidates_setup_LTX_scim_setup_module_save_config
#define scim_setup_module_query_changed   candidates_setup_LTX_scim_setup_module_query_changed

// The renderer reads these through a layered lookup (see
// scim_candidates_theme_from_config); this page writes the
// /Candidates/Default/ level, which a per-renderer /Candidates/<renderer>/
// value can still override.
#define SCIM_CONFIG_CANDIDATES_COLOR_SCHEME        "/Candidates/Default/ColorScheme"
#define SCIM_CONFIG_CANDIDATES_FONT                "/Candidates/Default/Font"
#define SCIM_CONFIG_CANDIDATES_PREEDIT_FONT        "/Candidates/Default/PreeditFont"
#define SCIM_CONFIG_CANDIDATES_COLOR_NORMAL_BG     "/Candidates/Default/Color/NormalBackground"
#define SCIM_CONFIG_CANDIDATES_COLOR_NORMAL_TEXT   "/Candidates/Default/Color/NormalText"
#define SCIM_CONFIG_CANDIDATES_COLOR_PREEDIT_TEXT  "/Candidates/Default/Color/PreeditText"
#define SCIM_CONFIG_CANDIDATES_COLOR_ACTIVE_BG     "/Candidates/Default/Color/ActiveBackground"
#define SCIM_CONFIG_CANDIDATES_COLOR_ACTIVE_TEXT   "/Candidates/Default/Color/ActiveText"
#define SCIM_CONFIG_CANDIDATES_COLOR_LABEL         "/Candidates/Default/Color/Label"
#define SCIM_CONFIG_CANDIDATES_COLOR_BORDER        "/Candidates/Default/Color/Border"
#define SCIM_CONFIG_CANDIDATES_BORDER_WIDTH        "/Candidates/Default/BorderWidth"
#define SCIM_CONFIG_CANDIDATES_CORNER_RADIUS       "/Candidates/Default/CornerRadius"
#define SCIM_CONFIG_CANDIDATES_PADDING             "/Candidates/Default/Padding"
#define SCIM_CONFIG_CANDIDATES_SPACING             "/Candidates/Default/Spacing"

static GtkWidget * create_setup_window ();
static void        load_config (const ConfigPointer &config);
static void        save_config (const ConfigPointer &config);
static bool        query_changed ();

// Module Interface.
extern "C" {
    void scim_module_init (void)
    {
    }

    void scim_module_exit (void)
    {
    }

    GtkWidget * scim_setup_module_create_ui (void)
    {
        return create_setup_window ();
    }

    String scim_setup_module_get_category (void)
    {
        return String ("Candidates");
    }

    String scim_setup_module_get_name (void)
    {
        return String (_("Appearance"));
    }

    String scim_setup_module_get_description (void)
    {
        return String (_("How the candidate window drawn at the cursor looks."));
    }

    void scim_setup_module_load_config (const ConfigPointer &config)
    {
        load_config (config);
    }

    void scim_setup_module_save_config (const ConfigPointer &config)
    {
        save_config (config);
    }

    bool scim_setup_module_query_changed ()
    {
        return query_changed ();
    }
} // extern "C"

// Internal data declaration.
//
// "default" (fonts) and "" (colors) mean the key stays unset, so the renderer
// keeps its own fallback. Only what the user actually picks is pinned down here.

// "", "light" or "dark". Empty is "auto", i.e. follow the desktop where the host
// can tell us; see scim_candidates_set_dark_hint ().
static String __config_color_scheme          = "";

static String __config_font                  = "default";
static String __config_preedit_font          = "default";

static String __config_color_normal_bg       = "";
static String __config_color_normal_text     = "";
static String __config_color_preedit_text    = "";
static String __config_color_active_bg       = "";
static String __config_color_active_text     = "";
static String __config_color_label           = "";
static String __config_color_border          = "";

// Seeded from CandidatesTheme::light () in load_config (), never hardcoded here:
// a copy that drifted from the renderer's default would show the wrong number and
// let Apply write it back as though the user had chosen it.
static int    __config_border_width          = 0;
static int    __config_corner_radius         = 0;
static int    __config_padding               = 0;
static int    __config_spacing               = 0;

// What an unset color resolves to, so the pickers show the color actually in
// effect. Set from the preset in load_config (); nothing is inherited from the
// panel's own /Panel/Gtk/ keys any more.
static String __unset_color_normal_bg;
static String __unset_color_normal_text;
static String __unset_color_active_bg;
static String __unset_color_active_text;

static bool   __have_changed                 = false;

static GtkWidget * __widget_color_scheme      = 0;
static GtkWidget * __widget_font              = 0;
static GtkWidget * __widget_preedit_font      = 0;
static GtkWidget * __widget_color_normal_bg   = 0;
static GtkWidget * __widget_color_normal_text = 0;
static GtkWidget * __widget_color_preedit_text = 0;
static GtkWidget * __widget_color_active_bg   = 0;
static GtkWidget * __widget_color_active_text = 0;
static GtkWidget * __widget_color_label       = 0;
static GtkWidget * __widget_color_border      = 0;
static GtkWidget * __widget_border_width      = 0;
static GtkWidget * __widget_corner_radius     = 0;
static GtkWidget * __widget_padding           = 0;
static GtkWidget * __widget_spacing           = 0;
static GtkWidget * __widget_reset             = 0;

// Declaration of internal functions.
static void setup_widget_value ();

static void
on_default_spin_button_changed (GtkSpinButton *spinbutton, gpointer user_data);

static void
on_color_set                   (GtkColorButton *button, gpointer user_data);

static void
on_font_clicked                (GtkButton *button, gpointer user_data);

static void
set_color_button               (GtkWidget *button, const String &color);

static void
on_reset_clicked               (GtkButton *button, gpointer user_data);

static CandidatesTheme
__preset                       ();

// The dropdown's rows, in order. Index 0 is "auto", stored as an unset key.
static const char *__color_scheme_values[] = { "", "light", "dark" };

static guint
color_scheme_to_index (const String &value)
{
    for (guint i = 1; i < G_N_ELEMENTS (__color_scheme_values); ++i)
        if (value == __color_scheme_values[i])
            return i;
    return 0;
}

static void
on_color_scheme_changed (GObject *object, GParamSpec * /*pspec*/,
                         gpointer /*user_data*/)
{
    guint i = gtk_drop_down_get_selected (GTK_DROP_DOWN (object));
    if (i >= G_N_ELEMENTS (__color_scheme_values))
        i = 0;
    __config_color_scheme = String (__color_scheme_values[i]);
    __have_changed = true;

    // The pickers show what an unset color resolves to, and that just changed.
    CandidatesTheme p = __preset ();
    __unset_color_normal_bg   = scim_candidates_format_color (p.bg);
    __unset_color_normal_text = scim_candidates_format_color (p.fg);
    __unset_color_active_bg   = scim_candidates_format_color (p.highlight_bg);
    __unset_color_active_text = scim_candidates_format_color (p.highlight_fg);
    setup_widget_value ();
}

// The color a picker should show: what the user chose, or what the renderer
// would fall back to if the key is left unset.
static String
effective_color (const String &chosen, const String &fallback)
{
    return chosen.length () ? chosen : fallback;
}

// Everything this page owns, back to "unset". Fonts and colors spell that with
// the empty string, which every reader already treats as "fall back"; the shape
// values have no such spelling, so they take the renderer's own numbers rather
// than a second copy kept here.
static void
adopt_builtin_defaults ()
{
    CandidatesTheme t = CandidatesTheme::light ();

    __config_color_scheme       = String ();   // auto
    __config_font               = String ();
    __config_preedit_font       = String ();

    __config_color_normal_bg    = String ();
    __config_color_normal_text  = String ();
    __config_color_preedit_text = String ();
    __config_color_active_bg    = String ();
    __config_color_active_text  = String ();
    __config_color_label        = String ();
    __config_color_border       = String ();

    __unset_color_normal_bg     = scim_candidates_format_color (t.bg);
    __unset_color_normal_text   = scim_candidates_format_color (t.fg);
    __unset_color_active_bg     = scim_candidates_format_color (t.highlight_bg);
    __unset_color_active_text   = scim_candidates_format_color (t.highlight_fg);

    __config_border_width       = t.border_width;
    __config_corner_radius      = t.corner_radius;
    __config_padding            = t.padding;
    __config_spacing            = t.spacing;
}

// The preset the page is currently showing. Which one it is depends on the
// color-scheme choice, so "unset" means what the renderer will really draw.
static CandidatesTheme
__preset ()
{
    return __config_color_scheme == String ("dark") ? CandidatesTheme::dark ()
                                                   : CandidatesTheme::light ();
}

static String
default_label_color ()
{
    return scim_candidates_format_color (CandidatesTheme::light ().label);
}

static String
default_border_color ()
{
    return scim_candidates_format_color (CandidatesTheme::light ().border);
}

// Function implementations.
static GtkWidget *
create_frame (GtkWidget *parent, const char *title)
{
    GtkWidget *frame = gtk_frame_new (title);
    gtk_widget_set_margin_start (frame, 4);
    gtk_widget_set_margin_end (frame, 4);
    gtk_widget_set_margin_top (frame, 4);
    gtk_widget_set_margin_bottom (frame, 4);
    gtk_widget_set_hexpand (frame, TRUE);
    gtk_box_append (GTK_BOX (parent), frame);
    return frame;
}

static GtkWidget *
create_grid (GtkWidget *frame)
{
    GtkWidget *grid = gtk_grid_new ();
    gtk_grid_set_row_spacing (GTK_GRID (grid), 4);
    gtk_grid_set_column_spacing (GTK_GRID (grid), 8);
    gtk_widget_set_margin_start (grid, 4);
    gtk_widget_set_margin_end (grid, 4);
    gtk_widget_set_margin_top (grid, 4);
    gtk_widget_set_margin_bottom (grid, 4);
    gtk_frame_set_child (GTK_FRAME (frame), grid);
    return grid;
}

GtkWidget *
create_setup_window ()
{
    static GtkWidget *window = 0;

    if (!window) {
        GtkWidget *page = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
        GtkWidget *frame;
        GtkWidget *grid;
        GtkWidget *label;

        // --- Fonts ---
        frame = create_frame (page, _("Fonts"));
        grid  = create_grid (frame);

        {
            struct { const char *label; GtkWidget **widget; } rows[] = {
                { _("_Font:"),         &__widget_font         },
                { _("_Preedit font:"), &__widget_preedit_font },
            };
            for (int i = 0; i < 2; ++i) {
                label = gtk_label_new_with_mnemonic (rows[i].label);
                gtk_widget_set_halign (label, GTK_ALIGN_START);
                gtk_grid_attach (GTK_GRID (grid), label, 0, i, 1, 1);

                *rows[i].widget = gtk_button_new_with_label ("default");
                gtk_grid_attach (GTK_GRID (grid), *rows[i].widget, 1, i, 1, 1);
                gtk_label_set_mnemonic_widget (GTK_LABEL (label), *rows[i].widget);
            }
        }

        // --- Colors ---
        frame = create_frame (page, _("Colors"));
        grid  = create_grid (frame);

        // Which preset the colors below fall back to when they are left unset.
        // First, because it changes what every one of them means.
        {
            label = gtk_label_new_with_mnemonic (_("Color _scheme:"));
            gtk_widget_set_halign (label, GTK_ALIGN_START);
            gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);

            const char *choices[] = { _("Follow the desktop"), _("Light"), _("Dark"), 0 };
            __widget_color_scheme = gtk_drop_down_new_from_strings (choices);
            gtk_grid_attach (GTK_GRID (grid), __widget_color_scheme, 1, 0, 1, 1);
            gtk_label_set_mnemonic_widget (GTK_LABEL (label), __widget_color_scheme);

            gtk_widget_set_tooltip_text (__widget_color_scheme,
                _("Which built-in appearance the colors below fall back to. "
                  "\"Follow the desktop\" works in GTK applications, which can "
                  "report their theme; elsewhere it means light, so choose Dark "
                  "explicitly for a dark candidate window."));

            g_signal_connect ((gpointer) __widget_color_scheme, "notify::selected",
                              G_CALLBACK (on_color_scheme_changed), 0);
        }

        {
            struct { const char *label; GtkWidget **widget; String *cfg; } rows[] = {
                { _("_Background:"),          &__widget_color_normal_bg,    &__config_color_normal_bg    },
                { _("_Text:"),                &__widget_color_normal_text,  &__config_color_normal_text  },
                { _("P_reedit text:"),        &__widget_color_preedit_text, &__config_color_preedit_text },
                { _("_Label:"),               &__widget_color_label,        &__config_color_label        },
                { _("Selected bac_kground:"), &__widget_color_active_bg,    &__config_color_active_bg    },
                { _("Selected te_xt:"),       &__widget_color_active_text,  &__config_color_active_text  },
                { _("B_order:"),              &__widget_color_border,       &__config_color_border       },
            };
            const int n = (int) (sizeof rows / sizeof rows[0]);
            for (int i = 0; i < n; ++i) {
                // + 1: the color-scheme row above occupies row 0.
                label = gtk_label_new_with_mnemonic (rows[i].label);
                gtk_widget_set_halign (label, GTK_ALIGN_START);
                gtk_grid_attach (GTK_GRID (grid), label, 0, i + 1, 1, 1);

                *rows[i].widget = gtk_color_button_new ();
                // Alpha is how transparency is configured: the renderer honors
                // it on the background and the border.
                gtk_color_chooser_set_use_alpha (
                    GTK_COLOR_CHOOSER (*rows[i].widget), TRUE);
                gtk_grid_attach (GTK_GRID (grid), *rows[i].widget, 1, i + 1, 1, 1);
                gtk_label_set_mnemonic_widget (GTK_LABEL (label), *rows[i].widget);

                g_signal_connect ((gpointer) *rows[i].widget, "color-set",
                                  G_CALLBACK (on_color_set), rows[i].cfg);
            }
        }

        // --- Shape ---
        frame = create_frame (page, _("Shape"));
        grid  = create_grid (frame);

        {
            struct { const char *label; GtkWidget **widget; int *cfg; int max; } rows[] = {
                { _("Border _width:"),   &__widget_border_width,  &__config_border_width,  16 },
                { _("Corner ra_dius:"),  &__widget_corner_radius, &__config_corner_radius, 64 },
                { _("Pa_dding:"),        &__widget_padding,       &__config_padding,       64 },
                { _("S_pacing:"),        &__widget_spacing,       &__config_spacing,       64 },
            };
            const int n = (int) (sizeof rows / sizeof rows[0]);
            for (int i = 0; i < n; ++i) {
                label = gtk_label_new_with_mnemonic (rows[i].label);
                gtk_widget_set_halign (label, GTK_ALIGN_START);
                gtk_grid_attach (GTK_GRID (grid), label, 0, i, 1, 1);

                *rows[i].widget =
                    gtk_spin_button_new_with_range (0.0, (double) rows[i].max, 1.0);
                gtk_grid_attach (GTK_GRID (grid), *rows[i].widget, 1, i, 1, 1);
                gtk_label_set_mnemonic_widget (GTK_LABEL (label), *rows[i].widget);

                g_signal_connect ((gpointer) *rows[i].widget, "value_changed",
                                  G_CALLBACK (on_default_spin_button_changed),
                                  rows[i].cfg);
            }
        }

        // --- Reset ---
        //
        // Without this there is no way back to the defaults for the shape values:
        // a spin button always holds a number, so Apply always wrote one, and
        // nothing the user could type meant "unset". Takes effect on Apply, like
        // every other control on the page.
        {
            GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
            gtk_widget_set_margin_start (box, 8);
            gtk_widget_set_margin_end (box, 8);
            gtk_widget_set_margin_top (box, 4);
            gtk_widget_set_margin_bottom (box, 4);
            gtk_box_append (GTK_BOX (page), box);

            __widget_reset = gtk_button_new_with_mnemonic (_("_Reset to defaults"));
            gtk_widget_set_halign (__widget_reset, GTK_ALIGN_END);
            gtk_widget_set_hexpand (__widget_reset, TRUE);
            gtk_box_append (GTK_BOX (box), __widget_reset);

            gtk_widget_set_tooltip_text (__widget_reset,
                _("Discard every setting on this page, so the candidate window "
                  "follows the built-in appearance again -- including any later "
                  "change to it. Applies when you press Apply."));

            g_signal_connect ((gpointer) __widget_reset, "clicked",
                              G_CALLBACK (on_reset_clicked), 0);
        }

        // Connect the font buttons; the payload says which setting to edit.
        g_signal_connect ((gpointer) __widget_font, "clicked",
                          G_CALLBACK (on_font_clicked), &__config_font);
        g_signal_connect ((gpointer) __widget_preedit_font, "clicked",
                          G_CALLBACK (on_font_clicked), &__config_preedit_font);

        // Tooltips.
        gtk_widget_set_tooltip_text (__widget_font,
                              _("The font used for the candidate list and the "
                                "auxiliary text."));

        gtk_widget_set_tooltip_text (__widget_preedit_font,
                              _("The font used for the string being composed.  "
                                "Leave as \"default\" to use the font above."));

        gtk_widget_set_tooltip_text (__widget_color_normal_bg,
                              _("The candidate window's background.  Lower the "
                                "opacity to see through it; that needs a "
                                "compositor on X11."));

        gtk_widget_set_tooltip_text (__widget_color_preedit_text,
                              _("The color of the string being composed."));

        gtk_widget_set_tooltip_text (__widget_color_label,
                              _("The color of the number in front of each "
                                "candidate."));

        gtk_widget_set_tooltip_text (__widget_border_width,
                              _("Thickness of the line around the candidate "
                                "window.  Zero draws no border."));

        gtk_widget_set_tooltip_text (__widget_corner_radius,
                              _("How far the corners are rounded.  Zero draws a "
                                "plain rectangle.  Rounded corners need a "
                                "compositor on X11."));

        gtk_widget_set_tooltip_text (__widget_padding,
                              _("Space between the window's border and its "
                                "contents."));

        gtk_widget_set_tooltip_text (__widget_spacing,
                              _("Space between the preedit, the auxiliary text "
                                "and the candidates."));

        window = page;

        setup_widget_value ();
    }
    return window;
}

void
setup_widget_value ()
{
    if (__widget_color_scheme)
        gtk_drop_down_set_selected (GTK_DROP_DOWN (__widget_color_scheme),
                                    color_scheme_to_index (__config_color_scheme));
    if (__widget_font)
        gtk_button_set_label (GTK_BUTTON (__widget_font), __config_font.c_str ());
    if (__widget_preedit_font)
        gtk_button_set_label (GTK_BUTTON (__widget_preedit_font),
                              __config_preedit_font.c_str ());

    const String normal_text =
        effective_color (__config_color_normal_text, __unset_color_normal_text);

    if (__widget_color_normal_bg)
        set_color_button (__widget_color_normal_bg,
            effective_color (__config_color_normal_bg, __unset_color_normal_bg));
    if (__widget_color_normal_text)
        set_color_button (__widget_color_normal_text, normal_text);
    // An unset preedit color follows the normal text color, not the panel's.
    if (__widget_color_preedit_text)
        set_color_button (__widget_color_preedit_text,
            effective_color (__config_color_preedit_text, normal_text));
    if (__widget_color_active_bg)
        set_color_button (__widget_color_active_bg,
            effective_color (__config_color_active_bg, __unset_color_active_bg));
    if (__widget_color_active_text)
        set_color_button (__widget_color_active_text,
            effective_color (__config_color_active_text, __unset_color_active_text));
    if (__widget_color_label)
        set_color_button (__widget_color_label,
            effective_color (__config_color_label, default_label_color ()));
    if (__widget_color_border)
        set_color_button (__widget_color_border,
            effective_color (__config_color_border, default_border_color ()));

    if (__widget_border_width)
        gtk_spin_button_set_value (GTK_SPIN_BUTTON (__widget_border_width),
                                   __config_border_width);
    if (__widget_corner_radius)
        gtk_spin_button_set_value (GTK_SPIN_BUTTON (__widget_corner_radius),
                                   __config_corner_radius);
    if (__widget_padding)
        gtk_spin_button_set_value (GTK_SPIN_BUTTON (__widget_padding),
                                   __config_padding);
    if (__widget_spacing)
        gtk_spin_button_set_value (GTK_SPIN_BUTTON (__widget_spacing),
                                   __config_spacing);
}

void
load_config (const ConfigPointer &config)
{
    if (!config.null ()) {
        // Every read below falls back to what is already in these variables, so
        // seed them with the renderer's defaults first: that is what an unset key
        // resolves to, and it keeps this page from inventing its own answer.
        adopt_builtin_defaults ();

        __config_color_scheme =
            config->read (String (SCIM_CONFIG_CANDIDATES_COLOR_SCHEME),
                          __config_color_scheme);
        __config_font =
            config->read (String (SCIM_CONFIG_CANDIDATES_FONT), __config_font);
        __config_preedit_font =
            config->read (String (SCIM_CONFIG_CANDIDATES_PREEDIT_FONT),
                          __config_preedit_font);

        __config_color_normal_bg =
            config->read (String (SCIM_CONFIG_CANDIDATES_COLOR_NORMAL_BG), String ());
        __config_color_normal_text =
            config->read (String (SCIM_CONFIG_CANDIDATES_COLOR_NORMAL_TEXT), String ());
        __config_color_preedit_text =
            config->read (String (SCIM_CONFIG_CANDIDATES_COLOR_PREEDIT_TEXT), String ());
        __config_color_active_bg =
            config->read (String (SCIM_CONFIG_CANDIDATES_COLOR_ACTIVE_BG), String ());
        __config_color_active_text =
            config->read (String (SCIM_CONFIG_CANDIDATES_COLOR_ACTIVE_TEXT), String ());
        __config_color_label =
            config->read (String (SCIM_CONFIG_CANDIDATES_COLOR_LABEL), String ());
        __config_color_border =
            config->read (String (SCIM_CONFIG_CANDIDATES_COLOR_BORDER), String ());

        __config_border_width =
            config->read (String (SCIM_CONFIG_CANDIDATES_BORDER_WIDTH),
                          __config_border_width);
        __config_corner_radius =
            config->read (String (SCIM_CONFIG_CANDIDATES_CORNER_RADIUS),
                          __config_corner_radius);
        __config_padding =
            config->read (String (SCIM_CONFIG_CANDIDATES_PADDING),
                          __config_padding);
        __config_spacing =
            config->read (String (SCIM_CONFIG_CANDIDATES_SPACING),
                          __config_spacing);

        // Now that the scheme is known, show the colors that an unset key really
        // resolves to: choosing Dark changes what "unset" looks like.
        {
            CandidatesTheme p = __preset ();
            __unset_color_normal_bg   = scim_candidates_format_color (p.bg);
            __unset_color_normal_text = scim_candidates_format_color (p.fg);
            __unset_color_active_bg   = scim_candidates_format_color (p.highlight_bg);
            __unset_color_active_text = scim_candidates_format_color (p.highlight_fg);
        }

        setup_widget_value ();

        __have_changed = false;
    }
}

void
save_config (const ConfigPointer &config)
{
    if (config.null ())
        return;

    CandidatesTheme t = CandidatesTheme::light ();

    // An unset key is erased rather than written, so a setting the user never
    // chose keeps following the renderer -- including when its default changes in
    // a later version. Writing them all back unconditionally is what froze a
    // page of appearance at whatever the defaults happened to be on the day
    // someone first opened it.
    struct { const char *key; const String *value; } strings[] = {
        { SCIM_CONFIG_CANDIDATES_COLOR_SCHEME,       &__config_color_scheme       },
        { SCIM_CONFIG_CANDIDATES_FONT,               &__config_font               },
        { SCIM_CONFIG_CANDIDATES_PREEDIT_FONT,       &__config_preedit_font       },
        { SCIM_CONFIG_CANDIDATES_COLOR_NORMAL_BG,    &__config_color_normal_bg    },
        { SCIM_CONFIG_CANDIDATES_COLOR_NORMAL_TEXT,  &__config_color_normal_text  },
        { SCIM_CONFIG_CANDIDATES_COLOR_PREEDIT_TEXT, &__config_color_preedit_text },
        { SCIM_CONFIG_CANDIDATES_COLOR_ACTIVE_BG,    &__config_color_active_bg    },
        { SCIM_CONFIG_CANDIDATES_COLOR_ACTIVE_TEXT,  &__config_color_active_text  },
        { SCIM_CONFIG_CANDIDATES_COLOR_LABEL,        &__config_color_label        },
        { SCIM_CONFIG_CANDIDATES_COLOR_BORDER,       &__config_color_border       },
    };
    for (size_t i = 0; i < sizeof strings / sizeof strings[0]; ++i) {
        if (strings[i].value->length ())
            config->write (String (strings[i].key), *strings[i].value);
        else
            config->erase (String (strings[i].key));
    }

    // A shape value equal to the default is erased too. It looks identical today
    // and keeps following the default tomorrow, which is what choosing the
    // default means -- and for a number there is no other way to say "unset".
    struct { const char *key; int value; int deflt; } ints[] = {
        { SCIM_CONFIG_CANDIDATES_BORDER_WIDTH,  __config_border_width,  t.border_width  },
        { SCIM_CONFIG_CANDIDATES_CORNER_RADIUS, __config_corner_radius, t.corner_radius },
        { SCIM_CONFIG_CANDIDATES_PADDING,       __config_padding,       t.padding       },
        { SCIM_CONFIG_CANDIDATES_SPACING,       __config_spacing,       t.spacing       },
    };
    for (size_t i = 0; i < sizeof ints / sizeof ints[0]; ++i) {
        if (ints[i].value != ints[i].deflt)
            config->write (String (ints[i].key), ints[i].value);
        else
            config->erase (String (ints[i].key));
    }

    __have_changed = false;
}

bool
query_changed ()
{
    return __have_changed;
}

static void
on_default_spin_button_changed (GtkSpinButton *spinbutton,
                                gpointer       user_data)
{
    int *value = static_cast<int *> (user_data);

    if (value) {
        *value = gtk_spin_button_get_value_as_int (spinbutton);
        __have_changed = true;
    }
}

// Show a color string in a picker, parsing it exactly as the renderer will.
static void
set_color_button (GtkWidget *button, const String &color)
{
    CandidatesColor c = { 0.5, 0.5, 0.5, 1.0 };
    scim_candidates_parse_color (color, c);

    GdkRGBA rgba;
    rgba.red   = c.r;
    rgba.green = c.g;
    rgba.blue  = c.b;
    rgba.alpha = c.a;

    gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (button), &rgba);
}

static void
on_reset_clicked (GtkButton * /*button*/, gpointer /*user_data*/)
{
    adopt_builtin_defaults ();
    setup_widget_value ();      // show what the defaults actually resolve to
    __have_changed = true;      // save_config () erases the keys on Apply
}

static void
on_color_set (GtkColorButton *button, gpointer user_data)
{
    String *cfg = static_cast<String *> (user_data);
    if (!cfg)
        return;

    GdkRGBA rgba;
    gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (button), &rgba);

    CandidatesColor c = { rgba.red, rgba.green, rgba.blue, rgba.alpha };
    *cfg = scim_candidates_format_color (c);
    __have_changed = true;

    // The preedit color follows the normal text color while it is unset, so it
    // has to track a change to that one (set_rgba does not re-emit "color-set").
    if (cfg == &__config_color_normal_text)
        setup_widget_value ();
}

static void
font_dialog_response_cb (GtkDialog *dialog,
                         gint       response,
                         gpointer   user_data)
{
    String    *cfg    = static_cast<String *> (user_data);
    GtkWidget *button = (cfg == &__config_preedit_font)
                        ? __widget_preedit_font : __widget_font;

    if (response == GTK_RESPONSE_OK && cfg) {
        gchar *fontname = gtk_font_chooser_get_font (GTK_FONT_CHOOSER (dialog));

        if (fontname) {
            *cfg = String (fontname);
            g_free (fontname);

            if (button)
                gtk_button_set_label (GTK_BUTTON (button), cfg->c_str ());

            __have_changed = true;
        }
    }

    gtk_window_destroy (GTK_WINDOW (dialog));
}

static void
on_font_clicked (GtkButton *button, gpointer user_data)
{
    String *cfg = static_cast<String *> (user_data);
    if (!cfg)
        return;

    const bool preedit = (cfg == &__config_preedit_font);
    GtkWidget *dialog = gtk_font_chooser_dialog_new (
        preedit ? _("Select Preedit Font") : _("Select Candidate Font"), NULL);
    GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (button));

    if (*cfg != String ("default"))
        gtk_font_chooser_set_font (GTK_FONT_CHOOSER (dialog), cfg->c_str ());

    if (root && GTK_IS_WINDOW (root))
        gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (root));
    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

    g_signal_connect (dialog, "response",
                      G_CALLBACK (font_dialog_response_cb), cfg);

    gtk_window_present (GTK_WINDOW (dialog));
}

/*
vi:ts=4:nowrap:expandtab
*/
