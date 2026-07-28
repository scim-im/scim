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
#include <cairo.h>
#include <cairo-xlib.h>

namespace scim {

class CandidatesUIX11::CandidatesUIX11Impl
{
public:
    CandidatesUI          m_ui;

    Display         *m_display;
    int              m_screen;
    Window           m_root;
    Window           m_window;
    cairo_surface_t *m_surface;

    int              m_win_w, m_win_h;
    int              m_spot_x, m_spot_y;
    bool             m_mapped;

    CandidateSlot    m_candidate_slot;
    PageSlot         m_page_up_slot;
    PageSlot         m_page_down_slot;

    CandidatesUIX11Impl ()
        : m_display (0), m_screen (0), m_root (0), m_window (0), m_surface (0),
          m_win_w (1), m_win_h (1), m_spot_x (0), m_spot_y (0), m_mapped (false)
    {
    }

    ~CandidatesUIX11Impl ()
    {
        destroy ();
    }

    bool create (const String &display_name)
    {
        m_display = XOpenDisplay (display_name.length () ? display_name.c_str () : 0);
        if (!m_display)
            return false;

        m_screen = DefaultScreen (m_display);
        m_root   = RootWindow (m_display, m_screen);

        XSetWindowAttributes attrs;
        attrs.override_redirect = True;
        attrs.save_under        = True;
        attrs.background_pixel   = WhitePixel (m_display, m_screen);
        attrs.border_pixel       = BlackPixel (m_display, m_screen);
        attrs.event_mask =
            ExposureMask | ButtonPressMask | StructureNotifyMask;

        m_window = XCreateWindow (
            m_display, m_root,
            0, 0, m_win_w, m_win_h, 0,
            DefaultDepth (m_display, m_screen),
            InputOutput,
            DefaultVisual (m_display, m_screen),
            CWOverrideRedirect | CWSaveUnder | CWBackPixel | CWBorderPixel |
            CWEventMask,
            &attrs);

        if (!m_window) {
            XCloseDisplay (m_display);
            m_display = 0;
            return false;
        }

        m_surface = cairo_xlib_surface_create (
            m_display, m_window,
            DefaultVisual (m_display, m_screen),
            m_win_w, m_win_h);

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
        resize (w, h);
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
            CandidatesUI::HitType hit = m_ui.hit_test (ev.x, ev.y, idx);
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
CandidatesUIX11::signal_connect_candidate_selected (CandidateSlot slot)
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

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
