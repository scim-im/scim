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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define Uses_SCIM_DEBUG
#include "scim_private.h"
#include <scim.h>

#include "scim_candidates_wayland.h"

#include <wayland-client.h>
#include "input-method-unstable-v1-client-protocol.h"
#include "input-method-unstable-v2-client-protocol.h"

#include <cairo.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <linux/input-event-codes.h>

namespace scim {

namespace {

// A throwaway shm buffer: freed when the compositor releases it.
struct ShmBuffer {
    struct wl_buffer *buffer;
    void             *data;
    size_t            size;
    int               width;
    int               height;
    int               stride;
};

const struct wl_buffer_listener buffer_listener = {
    // release
    [] (void *data, struct wl_buffer *) {
        ShmBuffer *b = static_cast<ShmBuffer *> (data);
        if (b->data) munmap (b->data, b->size);
        if (b->buffer) wl_buffer_destroy (b->buffer);
        delete b;
    }
};

} // anonymous namespace

class CandidatesWaylandImpl
{
public:
    CandidatesUI          m_ui;

    struct wl_display    *m_display;
    struct wl_compositor *m_compositor;
    struct wl_shm        *m_shm;
    struct zwp_input_method_v2       *m_input_method;
    struct wl_seat       *m_seat;

    struct wl_surface    *m_surface;
    // Exactly one role is set, depending on which init() was used.
    struct zwp_input_popup_surface_v2 *m_popup;
    struct zwp_input_panel_surface_v1 *m_panel_surface;
    struct wl_pointer    *m_pointer;

    // Set from the input-popup surface's text_input_rectangle: true when the
    // compositor placed us above the text rather than below it.
    bool     m_text_below;

    // Whether the compositor has activated the input method, i.e. whether there
    // is a focused text input for this popup to be positioned against. Nothing is
    // committed to the surface until it is.
    bool     m_active;
    // Whether a buffer is currently attached, so an unmap is committed once
    // rather than on every update that finds nothing to show.
    bool     m_mapped;

    // Pointer state.
    bool     m_pointer_on_surface;
    double   m_ptr_x, m_ptr_y;
    uint32_t m_enter_serial;

    CandidatesWayland::CandidateSlot m_candidate_slot;
    CandidatesWayland::PageSlot      m_page_up_slot;
    CandidatesWayland::PageSlot      m_page_down_slot;

    CandidatesWaylandImpl ()
        : m_display (0), m_compositor (0), m_shm (0), m_input_method (0),
          m_seat (0), m_surface (0), m_popup (0), m_panel_surface (0), m_pointer (0),
          m_text_below (false),
          m_active (false), m_mapped (false),
          m_pointer_on_surface (false), m_ptr_x (0), m_ptr_y (0),
          m_enter_serial (0)
    {
    }

    ~CandidatesWaylandImpl ()
    {
        finish ();
    }

    void finish ()
    {
        if (m_pointer) { wl_pointer_destroy (m_pointer); m_pointer = 0; }
        if (m_popup)   { zwp_input_popup_surface_v2_destroy (m_popup); m_popup = 0; }
        if (m_panel_surface) {
            zwp_input_panel_surface_v1_destroy (m_panel_surface);
            m_panel_surface = 0;
        }
        if (m_surface) { wl_surface_destroy (m_surface); m_surface = 0; }
    }

    ShmBuffer * create_buffer (int w, int h)
    {
        if (w < 1) w = 1;
        if (h < 1) h = 1;
        int stride = cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, w);
        size_t size = static_cast<size_t> (stride) * h;

        int fd = memfd_create ("scim-panel", MFD_CLOEXEC);
        if (fd < 0)
            return 0;
        if (ftruncate (fd, size) < 0) {
            close (fd);
            return 0;
        }

        void *data = mmap (0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (data == MAP_FAILED) {
            close (fd);
            return 0;
        }

        struct wl_shm_pool *pool = wl_shm_create_pool (m_shm, fd, size);
        struct wl_buffer *buffer = wl_shm_pool_create_buffer (
            pool, 0, w, h, stride, WL_SHM_FORMAT_ARGB8888);
        wl_shm_pool_destroy (pool);
        close (fd);

        ShmBuffer *b = new ShmBuffer;
        b->buffer = buffer;
        b->data   = data;
        b->size   = size;
        b->width  = w;
        b->height = h;
        b->stride = stride;
        wl_buffer_add_listener (buffer, &buffer_listener, b);
        return b;
    }

    void blank ()
    {
        if (!m_surface || !m_mapped) return;
        wl_surface_attach (m_surface, 0, 0, 0);
        wl_surface_commit (m_surface);
        m_mapped = false;
        if (m_display) wl_display_flush (m_display);
    }

    // Called when the compositor activates or deactivates the input method.
    void set_active (bool active)
    {
        if (m_active == active)
            return;
        m_active = active;
        if (m_active) update ();   // there may already be something to show
        else          blank ();
    }

    void update ()
    {
        if (!m_surface || !m_shm)
            return;

        // Commit nothing until the input method is active. An input popup is
        // positioned relative to the focused text input's surface, so without one
        // there is nowhere for this to go -- and sway dereferences the focused
        // surface's scene node when it processes the commit, which segfaults it
        // when no such surface is current (input_popup_update () ->
        // wlr_scene_node_coords ()). Staying unmapped until then is also simply
        // the truth: there is nothing to draw candidates next to.
        if (!m_active) {
            blank ();
            return;
        }

        if (!m_ui.is_visible ()) {
            blank ();
            return;
        }

        int w = 0, h = 0;
        m_ui.measure (w, h);

        ShmBuffer *b = create_buffer (w, h);
        if (!b) {
            blank ();
            return;
        }

        cairo_surface_t *cs = cairo_image_surface_create_for_data (
            static_cast<unsigned char *> (b->data), CAIRO_FORMAT_ARGB32,
            b->width, b->height, b->stride);
        cairo_t *cr = cairo_create (cs);
        m_ui.draw (cr);
        cairo_destroy (cr);
        cairo_surface_destroy (cs);

        // TODO(5c): output scale / HiDPI. Read the preferred buffer scale
        // (wl_surface enter -> wl_output, or fractional-scale-v1) and render
        // + wl_surface_set_buffer_scale accordingly; buffers are 1x for now.
        // TODO(5c): a per-frame throwaway shm buffer is simple but allocates
        // each update; reuse a buffer pool once the update rate matters.
        wl_surface_attach (m_surface, b->buffer, 0, 0);
        wl_surface_damage_buffer (m_surface, 0, 0, b->width, b->height);
        wl_surface_commit (m_surface);
        m_mapped = true;
        if (m_display) wl_display_flush (m_display);
    }

    void handle_button (uint32_t button, uint32_t state)
    {
        if (state != WL_POINTER_BUTTON_STATE_PRESSED)
            return;
        if (button != BTN_LEFT)
            return;
        int idx = -1;
        CandidatesUI::HitType hit =
            m_ui.hit_test (static_cast<int> (m_ptr_x),
                           static_cast<int> (m_ptr_y), idx);
        if (hit == CandidatesUI::HIT_CANDIDATE && idx >= 0) {
            if (m_candidate_slot) m_candidate_slot (idx);
        } else if (hit == CandidatesUI::HIT_PREV_PAGE) {
            if (m_page_up_slot) m_page_up_slot ();
        } else if (hit == CandidatesUI::HIT_NEXT_PAGE) {
            if (m_page_down_slot) m_page_down_slot ();
        }
    }

    void handle_axis (uint32_t axis, wl_fixed_t value)
    {
        if (axis != WL_POINTER_AXIS_VERTICAL_SCROLL)
            return;
        double v = wl_fixed_to_double (value);
        if (v < 0) {
            if (m_page_up_slot) m_page_up_slot ();
        } else if (v > 0) {
            if (m_page_down_slot) m_page_down_slot ();
        }
    }
};

/* ------------------------------------------------------------------ */
/* Pointer listener                                                    */
/* ------------------------------------------------------------------ */

namespace {

void ptr_enter (void *data, struct wl_pointer *, uint32_t serial,
                struct wl_surface *surface, wl_fixed_t sx, wl_fixed_t sy)
{
    CandidatesWaylandImpl *d =
        static_cast<CandidatesWaylandImpl *> (data);
    if (surface == d->m_surface) {
        d->m_pointer_on_surface = true;
        d->m_enter_serial = serial;
        d->m_ptr_x = wl_fixed_to_double (sx);
        d->m_ptr_y = wl_fixed_to_double (sy);
    }
}

void ptr_leave (void *data, struct wl_pointer *, uint32_t,
                struct wl_surface *surface)
{
    CandidatesWaylandImpl *d =
        static_cast<CandidatesWaylandImpl *> (data);
    if (surface == d->m_surface)
        d->m_pointer_on_surface = false;
}

void ptr_motion (void *data, struct wl_pointer *, uint32_t,
                 wl_fixed_t sx, wl_fixed_t sy)
{
    CandidatesWaylandImpl *d =
        static_cast<CandidatesWaylandImpl *> (data);
    if (d->m_pointer_on_surface) {
        d->m_ptr_x = wl_fixed_to_double (sx);
        d->m_ptr_y = wl_fixed_to_double (sy);
    }
}

void ptr_button (void *data, struct wl_pointer *, uint32_t, uint32_t,
                 uint32_t button, uint32_t state)
{
    CandidatesWaylandImpl *d =
        static_cast<CandidatesWaylandImpl *> (data);
    if (d->m_pointer_on_surface)
        d->handle_button (button, state);
}

void ptr_axis (void *data, struct wl_pointer *, uint32_t, uint32_t axis,
               wl_fixed_t value)
{
    CandidatesWaylandImpl *d =
        static_cast<CandidatesWaylandImpl *> (data);
    if (d->m_pointer_on_surface)
        d->handle_axis (axis, value);
}

void ptr_frame (void *, struct wl_pointer *) {}
void ptr_axis_source (void *, struct wl_pointer *, uint32_t) {}
void ptr_axis_stop (void *, struct wl_pointer *, uint32_t, uint32_t) {}
void ptr_axis_discrete (void *, struct wl_pointer *, uint32_t, int32_t) {}
void ptr_axis_value120 (void *, struct wl_pointer *, uint32_t, int32_t) {}
void ptr_axis_relative_direction (void *, struct wl_pointer *, uint32_t, uint32_t) {}

const struct wl_pointer_listener pointer_listener = {
    ptr_enter,
    ptr_leave,
    ptr_motion,
    ptr_button,
    ptr_axis,
    ptr_frame,
    ptr_axis_source,
    ptr_axis_stop,
    ptr_axis_discrete,
    ptr_axis_value120,
    ptr_axis_relative_direction,
};

// input-popup-surface: the compositor reports the text input area as a rectangle
// in *our* surface coordinates. It cannot be used to position the panel -- the
// compositor owns placement, and the surface is sized exactly to the content, so
// there is nothing to align within and nothing to move. What it does tell us is
// which side of the panel the text sits on: a positive y means the text is below
// us, i.e. the compositor put the panel above the text (typically near the
// bottom of the screen). In that case mirror the section order so the block
// nearest the text is the one closest to it.
//
// input-method-v1 has no equivalent event, so this refinement is v2 only; there
// the section order simply stays as laid out.
void popup_text_input_rectangle (void *data, struct zwp_input_popup_surface_v2 *,
                                 int32_t, int32_t y, int32_t, int32_t)
{
    CandidatesWaylandImpl *d = static_cast<CandidatesWaylandImpl *> (data);
    bool text_below = (y > 0);
    if (d->m_text_below == text_below)
        return;
    d->m_text_below = text_below;
    d->m_ui.set_sections_reversed (text_below);
    // Re-render only when something is already on screen; otherwise the next
    // update () picks the new order up.
    if (d->m_ui.is_visible ())
        d->update ();
}

const struct zwp_input_popup_surface_v2_listener popup_listener = {
    popup_text_input_rectangle,
};

} // anonymous namespace

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

CandidatesWayland::CandidatesWayland ()
    : m_impl (new CandidatesWaylandImpl ())
{
}

CandidatesWayland::~CandidatesWayland ()
{
    delete m_impl;
}

bool
CandidatesWayland::init (struct wl_display *display,
                      struct wl_compositor *compositor,
                      struct wl_shm *shm,
                      struct zwp_input_method_v2 *input_method,
                      struct wl_seat *seat)
{
    CandidatesWaylandImpl *d = m_impl;
    if (!display || !compositor || !shm || !input_method)
        return false;

    d->m_display      = display;
    d->m_compositor   = compositor;
    d->m_shm          = shm;
    d->m_input_method = input_method;
    d->m_seat         = seat;

    d->m_surface = wl_compositor_create_surface (compositor);
    if (!d->m_surface)
        return false;

    d->m_popup = zwp_input_method_v2_get_input_popup_surface (
        input_method, d->m_surface);
    if (!d->m_popup) {
        wl_surface_destroy (d->m_surface);
        d->m_surface = 0;
        return false;
    }
    zwp_input_popup_surface_v2_add_listener (d->m_popup, &popup_listener, d);

    if (seat) {
        d->m_pointer = wl_seat_get_pointer (seat);
        if (d->m_pointer)
            wl_pointer_add_listener (d->m_pointer, &pointer_listener, d);
    }

    return true;
}

bool
CandidatesWayland::init_input_panel (struct wl_display *display,
                                     struct wl_compositor *compositor,
                                     struct wl_shm *shm,
                                     struct zwp_input_panel_v1 *panel,
                                     struct wl_seat *seat)
{
    CandidatesWaylandImpl *d = m_impl;
    if (!display || !compositor || !shm || !panel)
        return false;

    d->m_display    = display;
    d->m_compositor = compositor;
    d->m_shm        = shm;
    d->m_seat       = seat;

    d->m_surface = wl_compositor_create_surface (compositor);
    if (!d->m_surface)
        return false;

    d->m_panel_surface =
        zwp_input_panel_v1_get_input_panel_surface (panel, d->m_surface);
    if (!d->m_panel_surface) {
        wl_surface_destroy (d->m_surface);
        d->m_surface = 0;
        return false;
    }

    // Overlay panel rather than set_toplevel (): a candidate window follows the
    // text being typed, so it should float over the app instead of being docked
    // as a full-width on-screen keyboard. zwp_input_panel_surface_v1 has no
    // listener -- unlike the v2 popup there is no cursor-rectangle event, so the
    // compositor's placement is all we get.
    zwp_input_panel_surface_v1_set_overlay_panel (d->m_panel_surface);

    if (seat) {
        d->m_pointer = wl_seat_get_pointer (seat);
        if (d->m_pointer)
            wl_pointer_add_listener (d->m_pointer, &pointer_listener, d);
    }

    return true;
}

void
CandidatesWayland::finish ()
{
    m_impl->finish ();
}

bool
CandidatesWayland::is_ready () const
{
    return m_impl->m_popup != 0 || m_impl->m_panel_surface != 0;
}

CandidatesUI &
CandidatesWayland::ui ()
{
    return m_impl->m_ui;
}

void
CandidatesWayland::update ()
{
    m_impl->update ();
}

void
CandidatesWayland::show ()
{
    m_impl->update ();
}

void
CandidatesWayland::hide ()
{
    m_impl->blank ();
}

void
CandidatesWayland::set_active (bool active)
{
    m_impl->set_active (active);
}

void
CandidatesWayland::signal_connect_select_candidate (CandidateSlot slot)
{
    m_impl->m_candidate_slot = slot;
}

void
CandidatesWayland::signal_connect_page_up (PageSlot slot)
{
    m_impl->m_page_up_slot = slot;
}

void
CandidatesWayland::signal_connect_page_down (PageSlot slot)
{
    m_impl->m_page_down_slot = slot;
}

/* ------------------------------------------------------------------ */
/* CandidatesSink                                                      */
/* ------------------------------------------------------------------ */

void
CandidatesWayland::enable (bool enabled)
{
    if (!is_ready () || enabled)
        return;
    m_impl->m_ui.hide_preedit_string ();
    m_impl->m_ui.hide_aux_string ();
    m_impl->m_ui.hide_lookup_table ();
    hide ();
}

void
CandidatesWayland::update_preedit_string (const WideString &str,
                                          const AttributeList &attrs)
{
    if (!is_ready ()) return;
    m_impl->m_ui.update_preedit_string (str, attrs);
    update ();
}

void
CandidatesWayland::update_preedit_caret (int caret)
{
    if (!is_ready ()) return;
    m_impl->m_ui.update_preedit_caret (caret);
    update ();
}

void
CandidatesWayland::show_preedit_string (bool visible)
{
    if (!is_ready ()) return;
    if (visible) m_impl->m_ui.show_preedit_string ();
    else         m_impl->m_ui.hide_preedit_string ();
    update ();
}

void
CandidatesWayland::update_aux_string (const WideString &str,
                                      const AttributeList &attrs)
{
    if (!is_ready ()) return;
    m_impl->m_ui.update_aux_string (str, attrs);
    update ();
}

void
CandidatesWayland::show_aux_string (bool visible)
{
    if (!is_ready ()) return;
    if (visible) m_impl->m_ui.show_aux_string ();
    else         m_impl->m_ui.hide_aux_string ();
    update ();
}

void
CandidatesWayland::update_lookup_table (const LookupTable &table)
{
    if (!is_ready ()) return;
    m_impl->m_ui.update_lookup_table (table);
    update ();
}

void
CandidatesWayland::show_lookup_table (bool visible)
{
    if (!is_ready ()) return;
    if (visible) m_impl->m_ui.show_lookup_table ();
    else         m_impl->m_ui.hide_lookup_table ();
    update ();
}

void
CandidatesWayland::update_spot_location (int x, int y)
{
    (void) x;
    (void) y;
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
