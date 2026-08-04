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

#define Uses_SCIM_DEBUG
#include "scim_private.h"
#include <scim.h>

#include "scim_candidates_x11.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>       // XVisualInfo / XGetVisualInfo
#include <X11/Xresource.h>   // Xft.dpi out of the resource database
#include <cairo.h>
#include <cairo-xlib.h>
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace scim {

namespace {

/**
 * @brief The desktop's scale factor, from Xft.dpi in the resource database.
 *
 * X11 has no per-window scale the way Wayland does, so a HiDPI session is
 * expressed as a raised dpi and every toolkit multiplies by it. Do the same, or
 * the candidate panel is the one window on such a desktop still drawn at 96dpi
 * -- sharp, but half the size of everything around it.
 *
 * Unlike the Wayland buffer scale this need not be an integer: the panel is
 * drawn, not sampled, so 1.5 costs nothing and is what a 144dpi session asks
 * for.
 *
 * Read once, when the connection is opened, which is also when GTK and Qt read
 * it; changing the dpi mid-session already requires restarting applications.
 * There is deliberately no fallback to the screen's physical size -- X11
 * reports that wrongly often enough that the toolkits stopped believing it, and
 * a wrong guess would misplace the panel on an ordinary display.
 */
double parse_xft_scale (const char *rms)
{
    if (!rms || !*rms)
        return 1.0;

    // Client-side parse; no server involved, which is what keeps this testable.
    XrmDatabase db = XrmGetStringDatabase (rms);
    if (!db)
        return 1.0;

    double scale = 1.0;
    char *type = 0;
    XrmValue value;

    if (XrmGetResource (db, "Xft.dpi", "Xft.Dpi", &type, &value) &&
        value.addr && value.size) {
        // XrmValue is not promised to be terminated, so copy before reading.
        char buf[32];
        size_t n = value.size < sizeof buf ? value.size : sizeof buf - 1;
        memcpy (buf, value.addr, n);
        buf[n] = '\0';

        double dpi = strtod (buf, 0);
        if (dpi > 0.0)
            scale = dpi / 96.0;
    }

    XrmDestroyDatabase (db);

    // A nonsense resource should not produce a window that is invisible or
    // bigger than the screen.
    if (scale < 0.5) scale = 0.5;
    if (scale > 8.0) scale = 8.0;
    return scale;
}

double read_xft_scale (Display *display)
{
    return parse_xft_scale (XResourceManagerString (display));
}

} // anonymous namespace

class CandidatesUIX11::CandidatesUIX11Impl
{
public:
    CandidatesUI          m_ui;

    Display         *m_display;
    int              m_screen;
    Window           m_root;
    Window           m_window;
    Colormap         m_colormap;    // None unless we made one for an ARGB visual
    cairo_surface_t *m_surface;

    // Window size in device pixels, i.e. the logical size the renderer measured
    // multiplied by m_scale. The spot is in root coordinates, which are device
    // pixels too, so place () needs no conversion.
    int              m_win_w, m_win_h;
    int              m_spot_x, m_spot_y;
    bool             m_mapped;

    // See read_xft_scale (). 1.0 on an ordinary display.
    double           m_scale;

    CandidateSlot    m_candidate_slot;
    PageSlot         m_page_up_slot;
    PageSlot         m_page_down_slot;

    CandidatesUIX11Impl ()
        : m_display (0), m_screen (0), m_root (0), m_window (0),
          m_colormap (None), m_surface (0),
          m_win_w (1), m_win_h (1), m_spot_x (0), m_spot_y (0), m_mapped (false),
          m_scale (1.0)
    {
    }

    ~CandidatesUIX11Impl ()
    {
        destroy ();
    }

    // Is a compositing manager running on this screen? Only then is an ARGB
    // visual worth asking for: without one the X server ignores the alpha and
    // the window shows uninitialized framebuffer through every transparent
    // pixel, which looks far worse than an opaque panel.
    bool compositor_running () const
    {
        char name [32];
        snprintf (name, sizeof name, "_NET_WM_CM_S%d", m_screen);
        Atom sel = XInternAtom (m_display, name, False);
        return sel != None && XGetSelectionOwner (m_display, sel) != None;
    }

    // A depth-32 TrueColor visual, or 0 when the server offers none. Depth 32
    // with 24 bits of RGB is the alpha-capable case; checking the render format
    // instead would mean linking libXrender for no practical gain.
    Visual * find_argb_visual (int &depth_out) const
    {
        XVisualInfo tmpl;
        tmpl.screen  = m_screen;
        tmpl.depth   = 32;
        tmpl.c_class = TrueColor;

        int n = 0;
        XVisualInfo *vi = XGetVisualInfo (
            m_display, VisualScreenMask | VisualDepthMask | VisualClassMask,
            &tmpl, &n);
        if (!vi)
            return 0;

        Visual *found = 0;
        for (int i = 0; i < n && !found; ++i)
            if (vi[i].red_mask && vi[i].green_mask && vi[i].blue_mask)
                found = vi[i].visual;

        if (found)
            depth_out = 32;
        XFree (vi);
        return found;
    }

    bool create (const String &display_name)
    {
        m_display = XOpenDisplay (display_name.length () ? display_name.c_str () : 0);
        if (!m_display)
            return false;

        m_screen = DefaultScreen (m_display);
        m_root   = RootWindow (m_display, m_screen);
        m_scale  = read_xft_scale (m_display);

        int     depth  = DefaultDepth (m_display, m_screen);
        Visual *visual = DefaultVisual (m_display, m_screen);
        Colormap cmap  = None;

        if (compositor_running ()) {
            int argb_depth = 0;
            Visual *argb = find_argb_visual (argb_depth);
            if (argb) {
                visual = argb;
                depth  = argb_depth;
                cmap   = XCreateColormap (m_display, m_root, visual, AllocNone);
            }
        }

        XSetWindowAttributes attrs;
        attrs.override_redirect = True;
        attrs.save_under        = True;
        attrs.border_pixel      = 0;
        attrs.event_mask =
            ExposureMask | ButtonPressMask | StructureNotifyMask;

        unsigned long mask =
            CWOverrideRedirect | CWSaveUnder | CWBorderPixel | CWEventMask;

        if (cmap != None) {
            // No background pixel on the ARGB path: letting X prefill the window
            // would paint over the corners we are about to cut out, and flash
            // them opaque on every resize. Cairo supplies every pixel instead.
            attrs.colormap        = cmap;
            attrs.background_pixmap = None;
            mask |= CWColormap | CWBackPixmap;
        } else {
            attrs.background_pixel = WhitePixel (m_display, m_screen);
            mask |= CWBackPixel;
        }

        m_window = XCreateWindow (
            m_display, m_root,
            0, 0, m_win_w, m_win_h, 0,
            depth,
            InputOutput,
            visual,
            mask,
            &attrs);

        if (!m_window) {
            if (cmap != None)
                XFreeColormap (m_display, cmap);
            XCloseDisplay (m_display);
            m_display = 0;
            return false;
        }

        m_colormap = cmap;

        m_surface = cairo_xlib_surface_create (
            m_display, m_window, visual, m_win_w, m_win_h);

        return true;
    }

    void destroy ()
    {
        if (m_surface) {
            cairo_surface_destroy (m_surface);
            m_surface = 0;
        }
        if (m_display) {
            if (m_window) {
                XDestroyWindow (m_display, m_window);
                m_window = 0;
            }
            // After the window, which referenced it.
            if (m_colormap != None) {
                XFreeColormap (m_display, m_colormap);
                m_colormap = None;
            }
            XCloseDisplay (m_display);
            m_display = 0;
        }
        m_mapped = false;
    }

    // Compute an on-screen position for the panel anchored at the spot.
    void place ()
    {
        if (!m_display) return;

        int sw = DisplayWidth (m_display, m_screen);
        int sh = DisplayHeight (m_display, m_screen);

        int x = m_spot_x;
        int y = m_spot_y;

        // Below the cursor by default; flip above if it would clip the bottom.
        if (y + m_win_h > sh)
            y = m_spot_y - m_win_h;
        if (y < 0) y = 0;
        if (x + m_win_w > sw)
            x = sw - m_win_w;
        if (x < 0) x = 0;

        XMoveWindow (m_display, m_window, x, y);
    }

    void resize (int w, int h)
    {
        if (w < 1) w = 1;
        if (h < 1) h = 1;
        if (w == m_win_w && h == m_win_h)
            return;
        m_win_w = w;
        m_win_h = h;
        XResizeWindow (m_display, m_window, w, h);
        if (m_surface)
            cairo_xlib_surface_set_size (m_surface, w, h);
    }

    void redraw ()
    {
        if (!m_surface) return;
        cairo_t *cr = cairo_create (m_surface);
        // The renderer lays out in logical pixels; the window is that size
        // times the scale, so everything it draws is magnified to match.
        if (m_scale != 1.0)
            cairo_scale (cr, m_scale, m_scale);
        m_ui.draw (cr);
        cairo_destroy (cr);
        if (m_display)
            XFlush (m_display);
    }

    void update ()
    {
        if (!m_display) return;
        int w = 0, h = 0;
        m_ui.measure (w, h);
        // Round up, or the last pixel column and row of the panel are cut off
        // at a fractional scale.
        resize ((int) std::ceil (w * m_scale), (int) std::ceil (h * m_scale));
        place ();
        if (m_mapped)
            redraw ();
    }

    void handle_button (const XButtonEvent &ev)
    {
        // Wheel up / down -> page flip.
        if (ev.button == Button4) {
            if (m_page_up_slot) m_page_up_slot ();
            return;
        }
        if (ev.button == Button5) {
            if (m_page_down_slot) m_page_down_slot ();
            return;
        }
        if (ev.button == Button1) {
            int idx = -1;
            // Pointer coordinates are device pixels; hit_test works in the
            // logical ones the layout was measured in.
            CandidatesUI::HitType hit = m_ui.hit_test ((int) (ev.x / m_scale),
                                                       (int) (ev.y / m_scale),
                                                       idx);
            if (hit == CandidatesUI::HIT_CANDIDATE && idx >= 0) {
                if (m_candidate_slot) m_candidate_slot (idx);
            } else if (hit == CandidatesUI::HIT_PREV_PAGE) {
                if (m_page_up_slot) m_page_up_slot ();
            } else if (hit == CandidatesUI::HIT_NEXT_PAGE) {
                if (m_page_down_slot) m_page_down_slot ();
            }
        }
    }

    void process_events ()
    {
        if (!m_display) return;
        while (XPending (m_display)) {
            XEvent ev;
            XNextEvent (m_display, &ev);
            if (ev.xany.window != m_window)
                continue;
            switch (ev.type) {
            case Expose:
                if (ev.xexpose.count == 0)
                    redraw ();
                break;
            case ButtonPress:
                handle_button (ev.xbutton);
                break;
            case ConfigureNotify:
                if (m_surface &&
                    (ev.xconfigure.width != m_win_w ||
                     ev.xconfigure.height != m_win_h)) {
                    m_win_w = ev.xconfigure.width;
                    m_win_h = ev.xconfigure.height;
                    cairo_xlib_surface_set_size (m_surface, m_win_w, m_win_h);
                }
                break;
            default:
                break;
            }
        }
    }
};

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

CandidatesUIX11::CandidatesUIX11 ()
    : m_impl (new CandidatesUIX11Impl ())
{
}

CandidatesUIX11::~CandidatesUIX11 ()
{
    delete m_impl;
}

bool
CandidatesUIX11::open (const String &display_name)
{
    if (m_impl->m_display)
        return true;
    return m_impl->create (display_name);
}

void
CandidatesUIX11::close ()
{
    m_impl->destroy ();
}

bool
CandidatesUIX11::is_open () const
{
    return m_impl->m_display != 0;
}

CandidatesUI &
CandidatesUIX11::ui ()
{
    return m_impl->m_ui;
}

int
CandidatesUIX11::connection_number () const
{
    if (!m_impl->m_display)
        return -1;
    return ConnectionNumber (m_impl->m_display);
}

void
CandidatesUIX11::process_events ()
{
    m_impl->process_events ();
}

void
CandidatesUIX11::move (int x, int y)
{
    m_impl->m_spot_x = x;
    m_impl->m_spot_y = y;
    if (m_impl->m_mapped)
        m_impl->place ();
}

void
CandidatesUIX11::update ()
{
    m_impl->update ();
}

void
CandidatesUIX11::show ()
{
    CandidatesUIX11Impl *d = m_impl;
    if (!d->m_display) return;
    d->update ();
    if (!d->m_mapped) {
        XMapRaised (d->m_display, d->m_window);
        d->m_mapped = true;
    }
    d->redraw ();
    XFlush (d->m_display);
}

void
CandidatesUIX11::hide ()
{
    CandidatesUIX11Impl *d = m_impl;
    if (!d->m_display) return;
    if (d->m_mapped) {
        XUnmapWindow (d->m_display, d->m_window);
        d->m_mapped = false;
        XFlush (d->m_display);
    }
}

bool
CandidatesUIX11::is_shown () const
{
    return m_impl->m_mapped;
}

void
CandidatesUIX11::signal_connect_select_candidate (CandidateSlot slot)
{
    m_impl->m_candidate_slot = slot;
}

void
CandidatesUIX11::signal_connect_page_up (PageSlot slot)
{
    m_impl->m_page_up_slot = slot;
}

void
CandidatesUIX11::signal_connect_page_down (PageSlot slot)
{
    m_impl->m_page_down_slot = slot;
}

/* ------------------------------------------------------------------ */
/* CandidatesSink                                                      */
/*                                                                     */
/* Every state call ends in refresh (), which is what keeps the window  */
/* honest: mapped while a section has content, unmapped once none does. */
/* Doing it here rather than in each host is also the one place a       */
/* deferred flush would go, if the repaint-per-sub-update ever matters. */
/* ------------------------------------------------------------------ */

void
CandidatesUIX11::refresh ()
{
    if (!is_open ())
        return;
    if (m_impl->m_ui.is_visible ())
        show ();                 // measures, positions, maps and redraws
    else
        hide ();
}

void
CandidatesUIX11::enable (bool enabled)
{
    if (!is_open ())
        return;
    if (!enabled) {
        m_impl->m_ui.hide_preedit_string ();
        m_impl->m_ui.hide_aux_string ();
        m_impl->m_ui.hide_lookup_table ();
        hide ();
    }
}

void
CandidatesUIX11::update_preedit_string (const WideString &str,
                                        const AttributeList &attrs)
{
    if (!is_open ()) return;
    m_impl->m_ui.update_preedit_string (str, attrs);
    refresh ();
}

void
CandidatesUIX11::update_preedit_caret (int caret)
{
    if (!is_open ()) return;
    m_impl->m_ui.update_preedit_caret (caret);
    refresh ();
}

void
CandidatesUIX11::show_preedit_string (bool visible)
{
    if (!is_open ()) return;
    if (visible) m_impl->m_ui.show_preedit_string ();
    else         m_impl->m_ui.hide_preedit_string ();
    refresh ();
}

void
CandidatesUIX11::update_aux_string (const WideString &str,
                                    const AttributeList &attrs)
{
    if (!is_open ()) return;
    m_impl->m_ui.update_aux_string (str, attrs);
    refresh ();
}

void
CandidatesUIX11::show_aux_string (bool visible)
{
    if (!is_open ()) return;
    if (visible) m_impl->m_ui.show_aux_string ();
    else         m_impl->m_ui.hide_aux_string ();
    refresh ();
}

void
CandidatesUIX11::update_lookup_table (const LookupTable &table)
{
    if (!is_open ()) return;
    m_impl->m_ui.update_lookup_table (table);
    refresh ();
}

void
CandidatesUIX11::show_lookup_table (bool visible)
{
    if (!is_open ()) return;
    if (visible) m_impl->m_ui.show_lookup_table ();
    else         m_impl->m_ui.hide_lookup_table ();
    refresh ();
}

void
CandidatesUIX11::update_spot_location (int x, int y)
{
    move (x, y);
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
